#include "export_packages.h"
#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <rpm/rpmvercmp.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;
using boost::property_tree::write_json;

using namespace std;

class CmpPackages {
    map<string, map<string, string>> _p10_packages;
    map<string, map<string, string>> _sisyphus_packages;

    map<string, map<string, string>> _in_sisyphus_not_p10;
    map<string, map<string, string>> _in_p10_not_sisyphus;
    map<string, map<string, string>> _ver_over_sisyphus_p10;

    public:
        explicit CmpPackages(map<string, map<string, string>> p10_packages,
                             map<string, map<string, string>> sisyphus_packages);
        void getAllDataConvertToJSON();
    private:
        void generateData(string first_branch);
        void genVerOverSisyphusP10();
        void convertToJSONSaveToFile();
        ptree convertDataToPtree(map<string, map<string, string>> data);
};

CmpPackages::CmpPackages(map<string, map<string, string>> p10_packages,
                         map<string, map<string, string>> sisyphus_packages)
    : _p10_packages(p10_packages)
    , _sisyphus_packages(sisyphus_packages) {};

void CmpPackages::generateData(string first_branch="sisyphus"){
    map<string, map<string, string>> first_branch_packages = _sisyphus_packages;
    map<string, map<string, string>> second_branch_packages = _p10_packages;
    if (first_branch == "p10"){
        first_branch_packages = _p10_packages;
        second_branch_packages = _sisyphus_packages;
    }

    for (const auto& [arch, packages] : first_branch_packages){
        for (const auto& [name, version] : packages){
            map<string, string>::iterator it;
            it = second_branch_packages[arch].find(name);
            if (it == second_branch_packages[arch].end()){
                if(first_branch == "p10") _in_p10_not_sisyphus[arch][name] = version;
                else _in_sisyphus_not_p10[arch][name] = version;
            }
        }
    }
}

void CmpPackages::genVerOverSisyphusP10()
{
    for (const auto& [arch, packages] : _sisyphus_packages){
        map<string, string> sisyphus_packages = packages;
        map<string, string> p10_packages = _p10_packages[arch];
        for (const auto& [name, version] : sisyphus_packages){
            map<string, string>::iterator it;
            it = p10_packages.find(name);
            if (it != p10_packages.end()) {
                string version_sisyphus_package = sisyphus_packages[name];
                string version_p10_package = it->second;
                if (rpmvercmp(version_sisyphus_package.c_str(), version_p10_package.c_str()))
                    _ver_over_sisyphus_p10[arch][name] = version_sisyphus_package;
            }
        }
    }
}

ptree CmpPackages::convertDataToPtree(map<string, map<string, string>> data){
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

void CmpPackages::convertToJSONSaveToFile()
{
    ptree pt;

    pt.add_child("packages_in_sisyphus_not_p10", convertDataToPtree(_in_sisyphus_not_p10));
    pt.add_child("packages_in_p10_not_sisyphus", convertDataToPtree(_in_p10_not_sisyphus));
    pt.add_child("packages_ver_over_sisyphus_p10", convertDataToPtree(_ver_over_sisyphus_p10));

    ofstream buf;
    buf.open("output.json");
    write_json(buf, pt);
    buf.close();
}

void CmpPackages::getAllDataConvertToJSON()
{
    generateData();
    generateData("p10");
    genVerOverSisyphusP10();
    convertToJSONSaveToFile();
}

int main()
{
    ExportPackages::Client p10("p10");
    ExportPackages::Client sisyphus("sisyphus");

    cout << "Receive packages..." << endl;
    CmpPackages cmp_packages(p10.getPackages(), sisyphus.getPackages());
    cout << "Received packages" << endl;
    cmp_packages.getAllDataConvertToJSON();

    return 0;
}
