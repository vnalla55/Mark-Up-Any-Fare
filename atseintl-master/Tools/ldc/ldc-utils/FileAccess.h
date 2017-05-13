#ifndef FileAccess_H
#define FileAccess_H

#include <stdio.h>
#include <string>
#include <vector>

class FileAccess
{
public:
  FileAccess(FILE* fp_): fp(fp_), lineNumber(0), lineLength(0)
  {
  }

  bool getLine(std::string& data);

  inline int getLineNumber() const { return lineNumber; }
  inline int getLineLength() const { return lineLength; }

  static size_t
  getFiles(const std::string& path, const std::string& pattern, std::vector<std::string>& fileList);

private:
  FILE* fp;
  size_t lineNumber;
  size_t lineLength;
};

#endif
