#ifndef EXPORT_PACKAGES_H
#define EXPORT_PACKAGES_H

#include <string>
#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>

namespace beast = boost::beast;

namespace ExportPackages
{
    class Client
    {
    std::string _branch;
    std::string _host = "rdb.altlinux.org";
    std::string _target = "/api/export/branch_binary_packages/";

    boost::asio::io_context _ioc;

    public:
        explicit Client(std::string branch);
        boost::json::value getResponse();
    private:
        beast::ssl_stream<beast::tcp_stream> initContextAndSSL();
        void connectToHost(beast::ssl_stream<beast::tcp_stream> &stream);
        void createAndSendRequest(beast::ssl_stream<beast::tcp_stream> &stream);
        boost::json::value receiveJSONResponse(beast::ssl_stream<beast::tcp_stream> &stream);
    };
}

#endif
