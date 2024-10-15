#include <fstream>
#include <filesystem>
#include "asset_packer.h"

namespace fs = std::filesystem;

// simply creates a package.
void AssetPacker::CreatePackage(std::string& pakFileName, const std::string& basePath)
{
    std::ofstream pakFile(pakFileName + ".pak", std::ios::binary);
    int fileCount = 0;
    int fileIterator = 0;
    memcpy(m_Header.fileSignature, "ASPACK", 6);
    m_Header.fileVersion = 1.0f;

    for (const auto& entry : fs::directory_iterator(basePath)) {
        if (entry.is_regular_file()) {
            fileCount++;
        }
    }
    m_Header.numOfEntries = fileCount;

    pakFile.write(reinterpret_cast<char*>(&m_Header), sizeof(FileHeader));

    FileTableEntry* fileTableEntry;

    fileTableEntry = new FileTableEntry[fileCount];

    int offset = 0;

    offset = sizeof(FileHeader) + (fileCount * sizeof(FileTableEntry));

    std::cout << offset << std::endl;
    for (const auto& entry : fs::directory_iterator(basePath))
    {
        if (entry.is_regular_file())
        {
            std::string filePath = entry.path().string();
            std::string relativePath = filePath.substr(basePath.length());

            relativePath.erase(0, 1);

            size_t formatPos = filePath.find_last_of('.');
            std::string fileFormat = (formatPos != std::string::npos) ? filePath.substr(formatPos + 1) : "";


            std::ifstream inputFile(filePath, std::ios::binary);

            std::stringstream buffer;
            buffer << inputFile.rdbuf();
            std::string fileData = buffer.str();
            memcpy(fileTableEntry[fileIterator].fileName, relativePath.c_str(), 30);
            fileTableEntry[fileIterator].fileSize = fileData.size();

            if (fileIterator == 0)
            {
                offset += 1;
            }
            else if (fileIterator > 0) 
            {
                offset += fileTableEntry[fileIterator - 1].fileSize + 1;
            }
            fileTableEntry[fileIterator].offset = offset;
            fileIterator++;
        }
    }


    pakFile.write(reinterpret_cast<char*>(fileTableEntry), fileCount * sizeof(FileTableEntry));
    
    fileIterator = 0;

    for (const auto& entry : fs::directory_iterator(basePath))
    {
        if (entry.is_regular_file())
        {
            std::string filePath = entry.path().string();
            std::string relativePath = filePath.substr(basePath.length());

            size_t formatPos = filePath.find_last_of('.');
            std::string fileFormat = (formatPos != std::string::npos) ? filePath.substr(formatPos + 1) : "";


            std::ifstream inputFile(filePath, std::ios::binary);

            std::stringstream buffer;
            buffer << inputFile.rdbuf();
            std::string fileData = buffer.str();
            pakFile.seekp(fileTableEntry[fileIterator].offset);

            pakFile << fileData;
            fileIterator++;
        }
    }

    delete[] fileTableEntry;
}

// Extracts the contents from a package.
void AssetPacker::DumpPackageContents(const std::string& pakFilePath, const std::string& outputFolderPath)
{
    std::ifstream pakFile(pakFilePath, std::ios::binary);

    if (!pakFile.is_open()) {
        std::cerr << "Error opening package file for reading." << std::endl;
        return;
    }

    FileHeader tmpHeader;
    pakFile.read(reinterpret_cast<char*>(&tmpHeader), sizeof(FileHeader));

    FileTableEntry* fileTableEntry;

    fileTableEntry = new FileTableEntry[tmpHeader.numOfEntries];

    pakFile.seekg(sizeof(FileHeader));

    //std::cout << tmpHeader.numOfEntries << " entry count" << std::endl;

    pakFile.read(reinterpret_cast<char*>(fileTableEntry), tmpHeader.numOfEntries * sizeof(FileTableEntry));
    char* buffer = new char[fileTableEntry[0].fileSize];
    std::string relativePath;
    for (int i = 0; i < tmpHeader.numOfEntries; i++) {
        relativePath = fileTableEntry[i].fileName;
        std::string filePath = outputFolderPath + "/" + relativePath;
        std::ofstream outputFile;
        outputFile.open(filePath, std::ios::binary);
        if (!outputFile.is_open()) {
            std::cerr << "Error creating output file" << std::endl;
            continue;
        }
        std::cout << fileTableEntry[i].offset << std::endl;
        pakFile.seekg(fileTableEntry[i].offset);
        buffer = new char[fileTableEntry[i].fileSize];
        pakFile.read(buffer, fileTableEntry[i].fileSize);
        outputFile.write(buffer, fileTableEntry[i].fileSize);
        outputFile.close();
        relativePath.clear();
        delete[] buffer;
    }

    delete[] buffer;

    pakFile.close();

    delete[] fileTableEntry;
}


// Loads an entire package into memory. (not sure as to why you would want to do this)
void AssetPacker::LoadPackage(const std::string& pakFilePath)
{
    std::ifstream pakFile(pakFilePath, std::ios::binary);

    if (!pakFile.is_open()) {
        std::cerr << "Error loading package file for loading." << std::endl;
        return;
    }

    FileHeader tmpHeader;
    pakFile.read(reinterpret_cast<char*>(&tmpHeader), sizeof(FileHeader));

    FileTableEntry* fileTableEntry;

    fileTableEntry = new FileTableEntry[tmpHeader.numOfEntries];

    pakFile.seekg(sizeof(FileHeader));

    //loadedFiles new LoadedFile[tmpHeader.numOfEntries];

    //std::cout << tmpHeader.numOfEntries << " entry count" << std::endl;

    pakFile.read(reinterpret_cast<char*>(fileTableEntry), tmpHeader.numOfEntries * sizeof(FileTableEntry));
    std::string relativePath;
    for (int i = 0; i < tmpHeader.numOfEntries; i++) {
        LoadedFile tmpfile;
        relativePath = fileTableEntry[i].fileName;
        std::cout << fileTableEntry[i].offset << std::endl;
        pakFile.seekg(fileTableEntry[i].offset);
        tmpfile.data = new unsigned char[fileTableEntry[i].fileSize];
        pakFile.read(reinterpret_cast<char*>(tmpfile.data), fileTableEntry[i].fileSize);
        tmpfile.fileName = relativePath;
        tmpfile.fileSize = fileTableEntry[i].fileSize;
        loadedFiles.push_back(tmpfile);
        relativePath.clear();
    }

    pakFile.close();

    delete[] fileTableEntry;
}

// as the name says loads a file into memory from a package.
AssetPacker::LoadedFile AssetPacker::LoadFileFromPackage(const std::string& pakFilePath, const std::string fileName)
{
    std::ifstream pakFile(pakFilePath, std::ios::binary);

    if (!pakFile.is_open()) {
        std::cerr << "Error loading package file for loading." << std::endl;
        //return;
    }

    FileHeader tmpHeader;
    pakFile.read(reinterpret_cast<char*>(&tmpHeader), sizeof(FileHeader));

    FileTableEntry* fileTableEntry;

    fileTableEntry = new FileTableEntry[tmpHeader.numOfEntries];

    pakFile.seekg(sizeof(FileHeader));


    //std::cout << tmpHeader.numOfEntries << " entry count" << std::endl;

    LoadedFile loadedFile;

    pakFile.read(reinterpret_cast<char*>(fileTableEntry), tmpHeader.numOfEntries * sizeof(FileTableEntry));
    std::string relativePath;
    for (int i = 0; i < tmpHeader.numOfEntries; i++) {
        relativePath = fileTableEntry[i].fileName;
        if (relativePath == fileName) {
            std::cout << fileTableEntry[i].offset << std::endl;
            pakFile.seekg(fileTableEntry[i].offset);
            loadedFile.data = new unsigned char[fileTableEntry[i].fileSize];
            pakFile.read(reinterpret_cast<char*>(loadedFile.data), fileTableEntry[i].fileSize);
            loadedFile.fileName = relativePath;
            loadedFile.fileSize = fileTableEntry[i].fileSize;
            relativePath.clear();
        }
    }

    pakFile.close();

    delete[] fileTableEntry;

    return loadedFile;
}

