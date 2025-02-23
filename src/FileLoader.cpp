#include "../include/FileLoader.h"
#include <fstream>
#include <ios>
#include <memory>
#include <sstream>
#include <stdexcept>

char *FileLoader::loadFileAsCharPtr(const std::string &path) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + path);
  }

  std::ostringstream contents;
  contents << file.rdbuf();
  std::string str = contents.str();
  char *buffer = new char[str.size() + 1];
  std::copy(str.begin(), str.end(), buffer);
  buffer[str.size()] = '\0';
  return buffer;
};

void FileLoader::freeCharPtr(char *ptr) { delete[] ptr; };

std::string FileLoader::loadFileAsString(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + path);
  }

  std::ostringstream contents;
  contents << file.rdbuf();
  return contents.str();
};

std::unique_ptr<char[]>
FileLoader::loadFileAsUniqueCharPtr(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + path);
  }

  std::ostringstream contents;
  contents << file.rdbuf();
  std::string str = contents.str();

  std::unique_ptr<char[]> buffer(new char[str.size() + 1]);
  std::copy(str.begin(), str.end(), buffer.get());
  buffer[str.size()] = '\0';
  return buffer;
};

std::vector<char> FileLoader::loadFileAsBinary(const std::string &path) {
  std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + path);
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);

  if (!file.read(buffer.data(), size)) {
    throw std::runtime_error("Failed to read binary file: " + path);
  }

  return buffer;
}

bool FileLoader::fileExists(const std::string &path) {
  std::ifstream file(path);
  return file.good();
};
