#include <export_packages.h>
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
    string _name_branch1;
    string _name_branch2;
    map<string, map<string, string>> _b2_packages;
    map<string, map<string, string>> _b1_packages;

    map<string, map<string, string>> _in_b1_not_b2;
    map<string, map<string, string>> _in_b2_not_b1;
    map<string, map<string, string>> _ver_over_b1_b2;

    public:
        explicit CmpPackages(string name_branch1="sisyphus", string name_branch2="p10");
        void getAllDataConvertToJSON();
    private:
        void generateData(string first_branch);
        void genVerOverB1B2();
        void convertToJSONSaveToFile();
        ptree convertDataToPtree(map<string, map<string, string>> data);
};

CmpPackages::CmpPackages(string name_branch1, string name_branch2)
    : _name_branch1(name_branch1), _name_branch2(name_branch2)
{
    ExportPackages::Client branch1(_name_branch1);
    ExportPackages::Client branch2(_name_branch2);

    _b1_packages = branch1.getPackages();
    _b2_packages = branch2.getPackages();
};

void CmpPackages::generateData(string first_branch){
    map<string, map<string, string>> first_branch_packages = _b1_packages;
    map<string, map<string, string>> second_branch_packages = _b2_packages;
    if (first_branch == _name_branch2){
        first_branch_packages = _b2_packages;
        second_branch_packages = _b1_packages;
    }

    for (const auto& [arch, packages] : first_branch_packages){
        for (const auto& [name, version] : packages){
            map<string, string>::iterator it;
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
        map<string, string> b1_packages = packages;
        map<string, string> b2_packages = _b2_packages[arch];
        for (const auto& [name, version] : b1_packages){
            map<string, string>::iterator it;
            it = b2_packages.find(name);
            if (it != b2_packages.end()) {
                string version_b1_package = b1_packages[name];
                string version_b2_package = it->second;
                if (rpmvercmp(version_b1_package.c_str(), version_b2_package.c_str()))
                    _ver_over_b1_b2[arch][name] = version_b1_package;
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

    pt.add_child("packages_in_"+_name_branch1+"_not_"+_name_branch2,
                 convertDataToPtree(_in_b1_not_b2));
    pt.add_child("packages_in_"+_name_branch2+"_not_"+_name_branch1,
                 convertDataToPtree(_in_b2_not_b1));
    pt.add_child("packages_ver_over_"+_name_branch1+"_"+_name_branch2,
                 convertDataToPtree(_ver_over_b1_b2));

    ofstream buf;
    buf.open("output.json");
    write_json(buf, pt);
    buf.close();
}

void CmpPackages::getAllDataConvertToJSON()
{
    generateData(_name_branch1);
    generateData(_name_branch2);
    genVerOverB1B2();
    convertToJSONSaveToFile();
}

int main()
{
    cout << "Receive packages..." << endl;
    CmpPackages cmp_packages;
    cout << "Received packages" << endl;
    cmp_packages.getAllDataConvertToJSON();

    return 0;
}
