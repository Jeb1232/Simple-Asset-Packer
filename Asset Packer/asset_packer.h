#pragma once
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<filesystem>
#include<vector>

class AssetPacker {
public:
	struct FileHeader {
		char fileSignature[6];
		float fileVersion;
		int numOfEntries;
	};
	struct FileTableEntry {
		char fileName[30];
		int fileSize = 0;
		int offset = 0;
	};
	struct LoadedFile {
		std::string fileName;
		int fileSize;
		unsigned char* data;
	};
	void CreatePackage(std::string& pakFileName, const std::string& basePath);
	void DumpPackageContents(const std::string& pakFilePath, const std::string& outputFolderPath);
	void LoadPackage(const std::string& pakFilePath);
	LoadedFile LoadFileFromPackage(const std::string& pakFilePath, const std::string fileName);
	FileHeader m_Header;
	std::vector<LoadedFile> loadedFiles;
private:
};