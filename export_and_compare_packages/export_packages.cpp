#include "exp_and_cmp_packages.h"

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>

namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace json = boost::json;

using boost::asio::ip::tcp;

using namespace ExpAndCmpPackages;

ExportPackages::ExportPackages(std::string branch) : _branch(branch) {};

std::map<std::string, std::map<std::string, std::string>> ExportPackages::getPackages()
{
    // Convert JSON format to map and delete unused information from JSON
    json::value json_text = getResponse();

    for(unsigned int i = 0; i < json::value_to<unsigned int>(json_text.at("length")); i++)
    {
        json::value package = json_text.at("packages").at(i);

        std::string arch = json::value_to<std::string>(package.at("arch"));
        std::string name = json::value_to<std::string>(package.at("name"));
        std::string version_release =
            json::value_to<std::string>(package.at("version")) + "-" +
            json::value_to<std::string>(package.at("release"));

        _packages[arch][name] = version_release;
    }

    return _packages;
}

json::value ExportPackages::getResponse()
{
    beast::ssl_stream<beast::tcp_stream> stream = initContextAndSSL();
    connectToHost(stream);
    createAndSendRequest(stream);
    json::value json_text = receiveJSONResponse(stream);

    //Shutdown stream
    beast::error_code ec;
    stream.shutdown(ec);

    return json_text;
}

beast::ssl_stream<beast::tcp_stream> ExportPackages::initContextAndSSL()
{
    //Init IO context
    ssl::context context(ssl::context::tlsv13_client);
    context.set_default_verify_paths();

    //Init SSL context
    beast::ssl_stream<beast::tcp_stream> stream(_ioc, context);
    stream.set_verify_mode(ssl::verify_none);
    stream.set_verify_callback([](bool preverified, ssl::verify_context& ctx) {
        return true;
    });

    //Enable SNI
    if(!SSL_set_tlsext_host_name(stream.native_handle(), _host.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()),
                                              net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }

    return stream;
}

void ExportPackages::connectToHost(beast::ssl_stream<beast::tcp_stream> &stream)
{
    //Connect to HTTPS host
    tcp::resolver resolver(_ioc);
    get_lowest_layer(stream).connect(resolver.resolve({_host, "https"}));
    get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
}

void ExportPackages::createAndSendRequest(beast::ssl_stream<beast::tcp_stream> &stream)
{
    //Construct request
    std::string finish_target = _target + _branch;
    http::request<http::string_body> request(http::verb::get,
            finish_target, 11);

    request.set(http::field::host, _host);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    //Send request
    stream.handshake(ssl::stream_base::client);
    http::write(stream, request);
}

json::value ExportPackages::receiveJSONResponse(beast::ssl_stream<beast::tcp_stream> &stream)
{
    // Receive response
    std::string response;
    {
        boost::beast::flat_buffer buffer;
        http::response<http::string_body> res;

        http::response_parser<http::string_body> parser;
        parser.eager(true);
	parser.body_limit(std::numeric_limits<std::uint64_t>::max());

        http::read(stream, buffer, parser);
        res = parser.release();
        response = res.body();
    }

    // Parse JSON
    json::error_code err;
    json::value json_text = json::parse(response, err);

    return json_text;
}
