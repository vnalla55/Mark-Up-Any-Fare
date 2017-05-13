//-------------------------------------------------------------------
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"


#include <string>

#define ROUTING_LEN 4

enum FLATFILEOPERATION
{
  LOAD,
  UPDATE,
  EXPIRED
};

namespace tse
{
class GZStream;

class TextFileLoader
{

public:
  TextFileLoader(const std::string& fileName, const std::string& desc);
  virtual ~TextFileLoader();

  bool parse();

protected:
  virtual bool parseStarting();
  virtual bool parseLine(const char* pLine, size_t length) = 0;
  virtual bool parseFinished();

  inline long atoi(const char* buffer, size_t sz)
  {
    long result(0);
    for (size_t i = 0; i < sz; ++i)
    {
      long res((result << 3) + (result << 1));
      result = res + (buffer[i] - static_cast<int>('0'));
    }
    /*
      cerr << "result=" << result << ",buffer:";
      cerr.write(buffer, sz);
      cerr << std::endl;
    */
    return result;
  }

  inline const char* find(const char* begin, const char* end, const char* sep, int len)
  {
    const char* start = begin;
    int i;
    while (begin < end)
    {
      for (i = 0; i < len; i++)
        if (*begin == sep[i])
          break;
      if ((i < len) && (begin > start))
        return begin;
      ++begin;
    }
    return end;
  }

  // ---------- Static Data Members ---------
private:

  // --------- Data Members --------------
private:
  std::string _fileName;
  std::string _desc;
  class GZStream* const _gzStream;
  const int _badEntriesThreshold;

}; // class TextFileLoader

} // end namespace

