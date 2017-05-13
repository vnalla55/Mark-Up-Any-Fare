#pragma once

#include <string>

namespace tse
{
class GZStream
{
private:
  void* _gzFile;
  // not implemented
  GZStream(const GZStream&);
  GZStream& operator=(const GZStream&);

public:
  GZStream(const std::string& url);
  ~GZStream();

  std::streamsize read(void* pBuffer, std::streamsize size);
};
}
