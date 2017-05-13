#pragma once

#include "Common/DateTime.h"
#include "DBAccess/BoundFareAdditionalInfo.h"
#include "DBAccess/BoundFareDAO.h"

#include <string>
#include <vector>

namespace tse
{
class GZStream;

const int _ESTIMATEDNUMBEROFBINDINGS(10);

typedef HashKey<LocCode, LocCode, CarrierCode> FareKey;
typedef std::vector<class Record2Reference> Record2ReferenceVector;
typedef std::vector<const class FareInfo*> FareInfoVec;

class FileLoaderBase
{
public:
  virtual ~FileLoaderBase();

  virtual void parse() = 0;

protected:
  FileLoaderBase(const std::string& url, BoundFareCache* cache);

  static void parseDateTime(const char* pBuffer, size_t length, DateTime& result);
  static double adjustDecimal(long amount, int noDec)
  {
    static double pow[] = { 1.0,    1.0E1,  1.0E2,  1.0E3,  1.0E4,  1.0E5,  1.0E6,
                            1.0E7,  1.0E8,  1.0E9,  1.0E10, 1.0E11, 1.0E12, 1.0E13,
                            1.0E14, 1.0E15, 1.0E16, 1.0E17, 1.0E18, 1.0E19, 1.0E20 };
    if (noDec >= (int)(sizeof(pow) / sizeof(double)))
      return 0;
    return static_cast<double>(amount) / pow[noDec];
  }
  GZStream* const _gzStream;
  BoundFareCache* const _cache;
  FareInfoVec* _pVector;
  const int _badEntriesThreshold;

private:
  // not implemented
  FileLoaderBase(const FileLoaderBase&);
  FileLoaderBase& operator=(const FileLoaderBase&);
};
}
