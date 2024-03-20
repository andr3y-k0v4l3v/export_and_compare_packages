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
    cout << "Receive packages..." << endl;
    ExpAndCmpPackages::CmpPackages cmp_packages(argv[1], argv[2]);
    cout << "Received packages" << endl;
    cmp_packages.getAllDataConvertToJSON(argv[3]);

    return 0;
}
