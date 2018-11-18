/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "FileSystem.h"
#include "../Wiring/WString.h"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;


file_t fileOpen(const String& name, FileOpenFlags flags) {
	int res;

	if((flags & eFO_CreateNewAlways) == eFO_CreateNewAlways) {
		if (fileExist(name)) {
			fileDelete(name);
		}
		
		flags = (FileOpenFlags)((int)flags & ~eFO_Truncate);
	}
	
	// TODO: implement full handling of file open flags.
	res = std::fopen(name.c_str(), "r+b"); // ignore flags, always open RW (binary).
	return res;
}

void fileClose(file_t file) {
	std::fclose(file);
}

size_t fileWrite(file_t file, const void* data, size_t size) {
	int res = std::fwrite((void*) data, size, size, file);	
	return res;
}

size_t fileRead(file_t file, void* data, size_t size) {
	int res = std::fread(data, size, size, file);
	return res;
}

int fileSeek(file_t file, int offset, SeekOriginFlags origin) {
	return std::fseek(file, offset, origin);
}

bool fileIsEOF(file_t file)
{
	return true;
	//return SPIFFS_eof(&_filesystemStorageHandle, file);
}

int32_t fileTell(file_t file)
{
	return 0; //SPIFFS_tell(&_filesystemStorageHandle, file);
}

int fileFlush(file_t file)
{
	return 0; //SPIFFS_fflush(&_filesystemStorageHandle, file);
}

int fileStats(const String& name, spiffs_stat* stat)
{
	return 0; //SPIFFS_stat(&_filesystemStorageHandle, name.c_str(), stat);
}

int fileStats(file_t file, spiffs_stat* stat) {
	return 0; //SPIFFS_fstat(&_filesystemStorageHandle, file, stat);
}

void fileDelete(const String& name) {
	fs::remove(name.c_str());
}

void fileDelete(file_t file) {
	//SPIFFS_fremove(&_filesystemStorageHandle, file); 
}

bool fileExist(const String& name) {
	std::error_code ec;
	bool ret = fs::is_regular_file(name.c_str(), ec);
	return ret;
}

int fileLastError(file_t fd) {
	return 0;
	//return SPIFFS_errno(&_filesystemStorageHandle);
}

void fileClearLastError(file_t fd) {
	return 0;
	//SPIFFS_clearerr(&_filesystemStorageHandle);
}

void fileSetContent(const String& fileName, const String& content) {
	fileSetContent(fileName, content.c_str());
}

void fileSetContent(const String& fileName, const char* content) {
	file_t file = fileOpen(fileName.c_str(), eFO_CreateNewAlways | eFO_WriteOnly);
	fileWrite(file, content, strlen(content));
	fileClose(file);
}

uint32_t fileGetSize(const String& fileName) {
	int size = 0;
	try {
        size = fs::file_size(filename.c_str());
    } 
	catch (fs::filesystem_error& e) {
        std::cout << e.what() << std::endl;
    }
	
	return size;
}

void fileRename(const String& oldName, const String& newName) {
	try {
		fs::rename(oldName.c_str(), newName.c_str());
	}
	catch (fs::filesystem_error& e) {
		std::cout << e.what() << std::endl;
	}
}

Vector<String> fileList() {
	Vector<String> result;
	/* spiffs_DIR d;
	spiffs_dirent info;

	SPIFFS_opendir(&_filesystemStorageHandle, "/", &d);
	while(true) {
		if(!SPIFFS_readdir(&d, &info))
			break;
		result.add(String((char*)info.name));
	}
	SPIFFS_closedir(&d); */
	return result;
}

String fileGetContent(const String& fileName) {
	std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	
    std::ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);

    return String(bytes.data(), fileSize);
}

int fileGetContent(const String& fileName, char* buffer, int bufSize) {
	if (buffer == NULL || bufSize == 0) { return 0; }
	*buffer = 0;
	
	std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	
    std::ifstream::pos_type fileSize = ifs.tellg();
	if (size <= 0 || bufSize <= size) {
		return 0;
	}
	
    buffer[size] = 0;
    ifs.seekg(0, std::ios::beg);
	fileRead(file, buffer, size);
	ifs.close();

    return (int) fileSize;
}
