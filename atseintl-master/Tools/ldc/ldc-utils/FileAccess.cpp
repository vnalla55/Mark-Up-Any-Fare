#include "Tools/ldc/ldc-utils/FileAccess.h"

#include <cstring>
#include <dirent.h>
#include <linux/unistd.h>
#include <regex.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>


bool
FileAccess::getLine(std::string& data)
{
  bool retval = false;
  char buffer[500] = { 0 };
  fgets(buffer, 500, fp);
  lineLength = strlen(buffer);
  if (lineLength)
  {
    retval = true;
    buffer[--lineLength] = 0;
    data = buffer;
    ++lineNumber;
  }
  return retval;
}

size_t
FileAccess::getFiles(const std::string& path,
                     const std::string& pattern,
                     std::vector<std::string>& fileList)
{
  size_t retval = 0;
  regex_t preg;

  fileList.clear();
  int coptions = REG_NOSUB;

  int result = regcomp(&preg, pattern.c_str(), coptions);
  if (result == 0)
  {
    DIR* dir = opendir(path.c_str());
    static struct dirent* dirp;

    if (dir)
    {
      while ((dirp = readdir(dir)) != nullptr)
      {
        std::string nextFile(dirp->d_name);
        std::string filename(path + "/" + nextFile);

        struct stat statBuf;
        if (stat(filename.c_str(), &statBuf) == 0)
        {
          size_t checkMask = S_IFREG;

          if ((statBuf.st_mode & checkMask) != checkMask)
          {
            continue;
          }
        }

        result = regexec(&preg, nextFile.c_str(), 0, nullptr, 0);

        if (result == 0)
        {
          fileList.push_back(nextFile);
        }
      }
    }
  }
#if 0
  else
  {
    fprintf( stderr, " regcomp() fails %d\n", result );
    switch (result)
    {
      case REG_BADBR: { printf("REG_BADBR\n"); break; }
      case REG_BADPAT: { printf("REG_BADPAT\n"); break; }
      case REG_BADRPT: { printf("REG_BADRPT\n"); break; }
      case REG_ECOLLATE: { printf("REG_ECOLLATE\n"); break; }
      case REG_ECTYPE: { printf("REG_ECTYPE\n"); break; }
      case REG_EESCAPE: { printf("REG_EESCAPE\n"); break; }
      case REG_EBRACK: { printf("REG_EBRACK\n"); break; }
      case REG_EPAREN: { printf("REG_EPAREN\n"); break; }
      case REG_EBRACE: { printf("REG_EBRACE\n"); break; }
      case REG_ERANGE: { printf("REG_ERANGE\n"); break; }
      default: { printf("unknown error\n"); break; }
    }
  }
#endif
  return fileList.size();
}
