#include <exp_and_cmp_packages.h>
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 4){
        cout << "Enter the name of the first branch, the second branch, ";
        cout << "and also enter the file name where to save the final JSON file." << endl << endl;
        cout << "Example:" << endl << "cmp_packages sisyphus p10 output.json" << endl;
        return -1;
    }

    string name_branch1 = argv[1];
    string name_branch2 = argv[2];
    string file_name    = argv[3];

    ExpAndCmpPackages::ExportPackages branch1(name_branch1);
    ExpAndCmpPackages::ExportPackages branch2(name_branch2);
    ExpAndCmpPackages::CmpPackages cmp_packages(name_branch1, name_branch2);

    cout << "Receive " << name_branch1 << " packages..." << endl;
    cmp_packages.setBranch1Packages(branch1.getPackages());
    cout << "Receive " << name_branch2 << " packages..." << endl;
    cmp_packages.setBranch2Packages(branch2.getPackages());
    cout << "Received packages" << endl;
    cmp_packages.getAllDataConvertToJSON(argv[3]);

    return 0;
}
