#include "exp_and_cmp_packages.h"

#include <fstream>
#include <boost/property_tree/json_parser.hpp>
#include <rpm/rpmvercmp.h>

using boost::property_tree::write_json;

using namespace ExpAndCmpPackages;

CmpPackages::CmpPackages(const std::string name_branch1, const std::string name_branch2)
    : _name_branch1(name_branch1), _name_branch2(name_branch2) {};

void CmpPackages::setBranch1Packages(const std::map<std::string, std::map<std::string, std::string>> b1_packages)
{
    _b1_packages = b1_packages;
}

void CmpPackages::setBranch2Packages(const std::map<std::string, std::map<std::string, std::string>> b2_packages)
{
    _b2_packages = b2_packages;
}

void CmpPackages::genInB1NotB2(const std::string first_branch){
    // Return all packages that are in the first branch, but not in the second
    std::map<std::string, std::map<std::string, std::string>> first_branch_packages;
    std::map<std::string, std::map<std::string, std::string>> second_branch_packages;

    if (first_branch == _name_branch1){
        first_branch_packages = _b1_packages;
        second_branch_packages = _b2_packages;
    } else {
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
    // Return all version-release packages of which there are more in the 1st than in the 2nd
    for (const auto& [arch, packages] : _b1_packages){
        std::map<std::string, std::string> b1_packages = packages;
        std::map<std::string, std::string> b2_packages = _b2_packages[arch];
        for (const auto& [name, version] : b1_packages){
            std::map<std::string, std::string>::iterator it;
            it = b2_packages.find(name);
            if (it != b2_packages.end()) {
                std::string version_b1_package = b1_packages[name];
                std::string version_b2_package = it->second;
                if (rpmvercmp(version_b1_package.c_str(), version_b2_package.c_str()) == 1)
                    _ver_over_b1_b2[arch][name] = version_b1_package;
            }
        }
    }
}

ptree CmpPackages::convertDataToPtree(const std::map<std::string, std::map<std::string, std::string>> data){
    // Convert map to ptree format
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

void CmpPackages::convertToJSONSaveToFile(const std::string file_name)
{
    // Convert all data to ptree and save JSON file
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

void CmpPackages::getAllDataConvertToJSON(const std::string file_name)
{
    genInB1NotB2(_name_branch1);
    genInB1NotB2(_name_branch2);
    genVerOverB1B2();
    convertToJSONSaveToFile(file_name);
}
