#ifndef EXPORT_PACKAGES_H
#define EXPORT_PACKAGES_H

#include <string>
#include <bits/stdc++.h>

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
    std::string _archs;
    std::string _host = "rdb.altlinux.org";
    std::string _target = "/api/export/branch_binary_packages/";
    std::map< std::string, std::map<std::string, std::string> > _packages;

    boost::asio::io_context _ioc;

    public:
        explicit Client(std::string branch);
        std::map<std::string, std::map<std::string, std::string>> getPackages();
    private:
        boost::json::value getResponse();
        beast::ssl_stream<beast::tcp_stream> initContextAndSSL();
        void connectToHost(beast::ssl_stream<beast::tcp_stream> &stream);
        void createAndSendRequest(beast::ssl_stream<beast::tcp_stream> &stream);
        boost::json::value receiveJSONResponse(beast::ssl_stream<beast::tcp_stream> &stream);
    };
}

#endif
