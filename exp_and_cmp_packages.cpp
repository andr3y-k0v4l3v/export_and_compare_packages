#include "exp_and_cmp_packages.h"

#include <fstream>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <openssl/ssl.h>
#include <rpm/rpmvercmp.h>

namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace json = boost::json;

using boost::property_tree::write_json;
using boost::asio::ip::tcp;

using namespace ExpAndCmpPackages;

ExportPackages::ExportPackages(std::string branch) : _branch(branch) {};

std::map<std::string, std::map<std::string, std::string>> ExportPackages::getPackages()
{
    json::value json_text = getResponse();
    
    for(int i = 0; i < json::value_to<int>(json_text.at("length")); i++)
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

    return json_text;}

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

CmpPackages::CmpPackages(std::string name_branch1, std::string name_branch2)
    : _name_branch1(name_branch1), _name_branch2(name_branch2)
{
    ExportPackages branch1(_name_branch1);
    ExportPackages branch2(_name_branch2);

    _b1_packages = branch1.getPackages();
    _b2_packages = branch2.getPackages();
};

void CmpPackages::generateData(std::string first_branch){
    std::map<std::string, std::map<std::string, std::string>> first_branch_packages = _b1_packages;
    std::map<std::string, std::map<std::string, std::string>> second_branch_packages = _b2_packages;
    if (first_branch == _name_branch2){
        first_branch_packages = _b2_packages;
        second_branch_packages = _b1_packages;
    }

    for (const auto& [arch, packages] : first_branch_packages){
        for (const auto& [name, version] : packages){
            std::map<std::string, std::string>::iterator it;
            it = second_branch_packages[arch].find(name);
            if (it == second_branch_packages[arch].end()){
                if(first_branch == _name_branch2) _in_b2_not_b1[arch][name] = version;
                else _in_b1_not_b2[arch][name] = version;
            }
        }
    }
}

void CmpPackages::genVerOverB1B2()
{
    for (const auto& [arch, packages] : _b1_packages){
        std::map<std::string, std::string> b1_packages = packages;
        std::map<std::string, std::string> b2_packages = _b2_packages[arch];
        for (const auto& [name, version] : b1_packages){
            std::map<std::string, std::string>::iterator it;
            it = b2_packages.find(name);
            if (it != b2_packages.end()) {
                std::string version_b1_package = b1_packages[name];
                std::string version_b2_package = it->second;
                if (rpmvercmp(version_b1_package.c_str(), version_b2_package.c_str()))
                    _ver_over_b1_b2[arch][name] = version_b1_package;
            }
        }
    }
}

ptree CmpPackages::convertDataToPtree(std::map<std::string, std::map<std::string, std::string>> data){
    ptree children;
    for (const auto& [arch, packages] : data){
        ptree tmp;
        for (const auto& [name, version] : packages)
            tmp.put(ptree::path_type{name, '\\'}, version);

        if (!tmp.empty())
            children.add_child(arch, tmp);
    }
    return children;
}

void CmpPackages::convertToJSONSaveToFile(std::string file_name)
{
    ptree pt;

    pt.add_child("packages_in_"+_name_branch1+"_not_"+_name_branch2,
                 convertDataToPtree(_in_b1_not_b2));
    pt.add_child("packages_in_"+_name_branch2+"_not_"+_name_branch1,
                 convertDataToPtree(_in_b2_not_b1));
    pt.add_child("packages_ver_over_"+_name_branch1+"_"+_name_branch2,
                 convertDataToPtree(_ver_over_b1_b2));

    std::ofstream buf;
    buf.open(file_name);
    write_json(buf, pt);
    buf.close();
}

void CmpPackages::getAllDataConvertToJSON(std::string file_name)
{
    generateData(_name_branch1);
    generateData(_name_branch2);
    genVerOverB1B2();
    convertToJSONSaveToFile(file_name);
}
