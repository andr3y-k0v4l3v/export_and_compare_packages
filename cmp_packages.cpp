#include "export_packages.h"
#include <iostream>

using namespace std;

int main()
{
    ExportPackages::Client client("p10");
    cout << client.getResponse().at("packages").at(0).at("name") << endl;
    return 0;
}
