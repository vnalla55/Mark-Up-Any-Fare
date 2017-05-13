//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

#include <vector>
#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>

namespace tse
{

namespace RemoteCache
{

const boost::uint8_t _headerFirstFieldSz(4);
const boost::uint8_t _headerSecondFieldSz(4);
const boost::uint8_t _headerThirdFieldSz(11);
const boost::uint8_t _headerFourthFieldSz(11);

const uint8_t _headerSz(_headerFirstFieldSz
                        + _headerSecondFieldSz
                        + _headerThirdFieldSz
                        + _headerFourthFieldSz);

enum StatusType
{
  STATUS_NONE
  , COMPRESSED_VALUE
  , NOT_CLIENT_OR_DISABLED
  , CONNECTION_REFUSED
  , READ_ERROR
  , WRITE_ERROR
  , BAD_REQUEST
  , NOT_FOUND
  , UNCOMPRESSED_VALUE
  , NOT_COMPRESSED_CACHE
  , VERSION_MISMATCH
  , INTERNAL_SERVER_ERROR
  , NOT_IMPLEMENTED
  , STATUS_TIMEOUT
  , STATUS_BUSY
  , NUMBER_DEFINITIONS
};

const std::string statusStrings[] = { "None"
                                      , "Compressed value"
                                      , "Not client or disabled"
                                      , "Connection refused"
                                      , "Read error"
                                      , "Write error"
                                      , "Bad request"
                                      , "Not found"
                                      , "Uncompressed value"
                                      , "Not compressed cache"
                                      , "Version mismatch"
                                      , "Internal server error"
                                      , "Not implemented"
                                      , "Timeout"
                                      , "Busy"
                                    };

struct RemoteCacheHeader
{
  RemoteCacheHeader()
    : _status(STATUS_NONE)
    , _version(1)
    , _payloadSize(0)
    , _inflatedSize(0)
  {
  }
  StatusType _status;
  uint32_t _version;
  uint64_t _payloadSize;
  uint64_t _inflatedSize;
};

inline void writeHeaderStr(const RemoteCacheHeader& header,
                           std::string& headerStr)
{
  headerStr = boost::lexical_cast<std::string>(header._status);
  headerStr.append(_headerFirstFieldSz - headerStr.length(), '\0');
  headerStr.append(boost::lexical_cast<std::string>(header._version));
  headerStr.append(_headerFirstFieldSz + _headerSecondFieldSz - headerStr.length(), '\0');
  headerStr.append(boost::lexical_cast<std::string>(header._payloadSize));
  headerStr.append(_headerFirstFieldSz + _headerSecondFieldSz + _headerThirdFieldSz - headerStr.length(), '\0');
  headerStr.append(boost::lexical_cast<std::string>(header._inflatedSize));
  headerStr.append(_headerSz - headerStr.length(), '\0');
}

inline void readHeaderStr(RemoteCacheHeader& header,
                          const std::vector<char>& headerVect,
                          StatusType& status)
{
  header._status = static_cast<StatusType>(atoi(&headerVect[0]));
  if (COMPRESSED_VALUE == status)
  {
    status = header._status;
  }
  header._version = 
      static_cast<uint32_t>(atoi(&headerVect[_headerFirstFieldSz]));
  header._payloadSize = 
      static_cast<uint64_t>(atol(&headerVect[_headerFirstFieldSz + _headerSecondFieldSz]));
  header._inflatedSize = 
      static_cast<uint64_t>(atol(&headerVect[_headerFirstFieldSz +
                                             _headerSecondFieldSz +
                                             _headerThirdFieldSz]));
}

inline std::string statusToString(StatusType status)
{
  switch (status)
  {
  case STATUS_NONE:
  case COMPRESSED_VALUE:
  case NOT_CLIENT_OR_DISABLED:
  case CONNECTION_REFUSED:
  case READ_ERROR:
  case WRITE_ERROR:
  case BAD_REQUEST:
  case NOT_FOUND:
  case UNCOMPRESSED_VALUE:
  case NOT_COMPRESSED_CACHE:
  case VERSION_MISMATCH:
  case INTERNAL_SERVER_ERROR:
  case NOT_IMPLEMENTED:
  case STATUS_TIMEOUT:
  case STATUS_BUSY:
    return statusStrings[status];
  default:
    return statusStrings[STATUS_NONE];
  }
}

}// RemoteCache

}// tse
