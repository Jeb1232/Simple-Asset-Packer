#include <iostream>
#include "asset_packer.h"

int main() {
	std::string cin1, cin2;
	std::cout << "Path to package contents: " << std::endl;
	std::getline(std::cin, cin1);
	std::cout << "Package name:  " << std::endl;
	std::getline(std::cin, cin2);
	AssetPacker packer;
	packer.CreatePackage(cin2, cin1);
	return 0;
}