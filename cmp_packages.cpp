#include "export_packages.h"
#include <iostream>

using namespace std;

int main()
{
    ExportPackages::Client client("p10");
    cout << client.getPackages()["armh"][0]["name"] << endl;
    return 0;
}
