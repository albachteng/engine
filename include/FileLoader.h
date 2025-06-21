#include <memory>
#include <string>
#include <vector>

/**
 * FileLoader - Safe file loading utilities
 * 
 * Provides multiple ways to load files with different memory management strategies.
 * Prefer the safe methods (loadFileAsString, loadFileAsBinary) for most use cases.
 */
class FileLoader {
public:
  // ============================================================================
  // RECOMMENDED SAFE METHODS - Use these for new code
  // ============================================================================
  
  /**
   * Load file as std::string (RECOMMENDED)
   * - Automatic memory management
   * - Exception safe
   * - Works with C APIs via .c_str()
   * - Best for most use cases including shaders, config files
   */
  static std::string loadFileAsString(const std::string &path);

  /**
   * Load file as binary data in std::vector<char> (RECOMMENDED)
   * - Automatic memory management
   * - Exception safe
   * - Efficient for large files
   * - Best for binary assets, images, models
   */
  static std::vector<char> loadFileAsBinary(const std::string &path);

  /**
   * Check if file exists
   * - Safe utility function
   * - Use before loading to handle missing files gracefully
   */
  static bool fileExists(const std::string &path);

  // ============================================================================
  // ADVANCED METHODS - Manual memory management required
  // ============================================================================
  
  /**
   * Load file as std::unique_ptr<char[]> (ADVANCED)
   * - Automatic cleanup via RAII
   * - Use when you need char* for C APIs but want safety
   * - Access via .get(), automatically freed when unique_ptr is destroyed
   */
  static std::unique_ptr<char[]> loadFileAsUniqueCharPtr(const std::string &path);

  /**
   * Load file as raw char* (ADVANCED - USE WITH CAUTION)
   * 
   * ⚠️  MANUAL MEMORY MANAGEMENT REQUIRED ⚠️
   * - You MUST call freeCharPtr() to avoid memory leaks
   * - Not exception safe - use RAII wrapper or try/catch
   * - Only use for legacy C API compatibility when alternatives don't work
   * 
   * Example safe usage:
   *   char* data = nullptr;
   *   try {
   *     data = FileLoader::loadFileAsCharPtr("file.txt");
   *     // Use data...
   *     FileLoader::freeCharPtr(data);
   *   } catch (...) {
   *     if (data) FileLoader::freeCharPtr(data);
   *     throw;
   *   }
   * 
   * Better alternative for C APIs:
   *   std::string content = FileLoader::loadFileAsString("file.txt");
   *   const char* data = content.c_str(); // No manual cleanup needed
   */
  static char *loadFileAsCharPtr(const std::string &path);

  /**
   * Free memory allocated by loadFileAsCharPtr()
   * - Must be called for every loadFileAsCharPtr() call
   * - Safe to call with nullptr
   */
  static void freeCharPtr(char *ptr);
};

// TODO: ChunkedFileLoader, CachedFileLoader, shader hot reloading?
