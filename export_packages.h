#ifndef EXPORT_PACKAGES_H
#define EXPORT_PACKAGES_H

#include <string>
#include <bits/stdc++.h>

#include <boost/json.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>

using boost::property_tree::ptree;

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

    class CmpPackages {
        std::string _name_branch1;
        std::string _name_branch2;
        std::map<std::string, std::map<std::string, std::string>> _b2_packages;
        std::map<std::string, std::map<std::string, std::string>> _b1_packages;

        std::map<std::string, std::map<std::string, std::string>> _in_b1_not_b2;
        std::map<std::string, std::map<std::string, std::string>> _in_b2_not_b1;
        std::map<std::string, std::map<std::string, std::string>> _ver_over_b1_b2;

    public:
        explicit CmpPackages(std::string name_branch1="sisyphus", std::string name_branch2="p10");
        void getAllDataConvertToJSON(std::string file_name="output.json");
    private:
        void generateData(std::string first_branch);
        void genVerOverB1B2();
        void convertToJSONSaveToFile(std::string file_name);
        ptree convertDataToPtree(std::map<std::string, std::map<std::string, std::string>> data);
    };
}

#endif
