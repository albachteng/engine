#include <memory>
#include <string>
#include <vector>

class FileLoader {
public:
  static char *loadFileAsCharPtr(const std::string &path);
  static void freeCharPtr(char *ptr);

  static std::string loadFileAsString(const std::string &path);

  static std::vector<char> loadFileAsBinary(const std::string &path);

  static bool fileExists(const std::string &path);
  static std::unique_ptr<char[]>
  loadFileAsUniqueCharPtr(const std::string &path);
};

// TODO: ChunkedFileLoader, CachedFileLoader, shader hot reloading?
