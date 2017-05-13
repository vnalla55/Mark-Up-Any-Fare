#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace tse
{

namespace RemoteCache
{

const std::string _RCVV("RC05");

const uint8_t _rcVVSz(4);

const uint8_t _headerSz(_rcVVSz + 2 * sizeof(uint32_t) + 3 * sizeof(uint64_t));

enum StatusType
{
  RC_NONE
  , RC_WRONG_MAGIC_STRING
  , RC_BAD_REQUEST_STATUS
  , RC_COMPRESSED_VALUE
  , RC_NOT_CLIENT_OR_DISABLED
  , RC_CONNECTION_REFUSED
  , RC_ADDRESS_NOT_AVAILABLE
  , RC_READ_ERROR
  , RC_WRITE_ERROR
  , RC_BAD_REQUEST
  , RC_MASTER_CACHE_UPDATE_STOPPED
  , RC_NOT_FOUND
  , RC_UNCOMPRESSED_VALUE
  , RC_NOT_COMPRESSED_CACHE
  , RC_DAO_VERSION_MISMATCH
  , RC_DATABASE_MISMATCH
  , RC_NOT_SERVER_FOR_DATATYPE
  , RC_SERVER_ERROR
  , RC_CLIENT_ERROR
  , RC_NOT_IMPLEMENTED
  , RC_SERVER_TIMEOUT
  , RC_CLIENT_CONNECTION_TIMEOUT
  , RC_CLIENT_PROCESSING_TIMEOUT
  , RC_SERVER_BUSY
  , RC_SERVER_NOT_READY
  , RC_MAX_NUMBER_CLIENTS_EXCEEDED
  , RC_UNKNOWN_ERROR
  , RC_HEADER_VERSION_MISMATCH
  , RC_REQUEST_CANCELED
  , RC_HEARTBEAT
  , RC_REQUEST_ID_MISMATCH
  , RC_BASELINE_MISMATCH
  , RC_REQUEST_FROM_SAME_HOST
  , RC_QUEUE_LIMIT_EXCEEDED
  , RC_HEALTHCHECK_TIMEOUT
  , RC_INCOMPATIBLE_MODE
  , RC_MASTER_NONHISTORICAL
  , RC_NUMBER_STATUS_DEFINITIONS
};

typedef std::atomic<StatusType> RCStatus;

inline const std::string& statusToString(StatusType status)
{
  static const std::string statusStrings[] = {"RC None",
                                              "RC Wrong magic number",
                                              "RC Bad request status",
                                              "RC Compressed value",
                                              "RC Not client or disabled",
                                              "RC Connection refused",
                                              "RC Address not available",
                                              "RC Read error",
                                              "RC Write error",
                                              "RC Bad request",
                                              "RC master cache update stopped",
                                              "RC Not found",
                                              "RC Uncompressed value",
                                              "RC Not compressed cache",
                                              "RC DAO version mismatch",
                                              "RC Database mismatch",
                                              "RC Not server for datatype",
                                              "RC server error",
                                              "RC client error",
                                              "RC Not implemented",
                                              "RC server timeout",
                                              "RC client connection timeout",
                                              "RC client processing timeout",
                                              "RC server busy",
                                              "RC server not ready",
                                              "RC maximum number clients exceeded",
                                              "RC Unknown Error",
                                              "RC Header version mismatch",
                                              "RC Request canceled",
                                              "RC Heartbeat",
                                              "RC Request id mismatch",
                                              "RC Baseline mismatch",
                                              "RC Request from same host",
                                              "RC Queue limit exceeded",
                                              "RC Healthcheck timeout",
                                              "RC Incompatible mode",
                                              "RC Master nonhistorical",
                                              "RC This should not appear"};

  return statusStrings[status];
}

struct RemoteCacheHeader
{
  explicit RemoteCacheHeader(uint64_t requestId)
    : _requestId(requestId)
  {
    std::memset(_rcId, 0, _rcVVSz);
  }
  char _rcId[_rcVVSz];
  RCStatus _status{RC_NONE};
  uint32_t _daoVersion{};
  uint64_t _requestId;
  uint64_t _payloadSize{};
  uint64_t _inflatedSize{};
};

inline void writeHeader(const RemoteCacheHeader& header,
                        std::vector<char>& headerVector)
{
  std::memcpy(&headerVector[0], _RCVV.c_str(), _rcVVSz);
  StatusType status(header._status);
  std::memcpy(&headerVector[_rcVVSz], &status, sizeof(uint32_t));
  std::memcpy(&headerVector[_rcVVSz + sizeof(uint32_t)], &header._daoVersion, sizeof(uint32_t));
  std::memcpy(&headerVector[_rcVVSz + 2 * sizeof(uint32_t)],
    &header._requestId, sizeof(uint64_t));
  std::memcpy(&headerVector[_rcVVSz + 2 * sizeof(uint32_t) + sizeof(uint64_t)],
    &header._payloadSize, sizeof(uint64_t));
  std::memcpy(&headerVector[_rcVVSz + 2 * sizeof(uint32_t) + 2 * sizeof(uint64_t)],
    &header._inflatedSize, sizeof(uint64_t));
}

inline bool readHeader(RemoteCacheHeader& header,
                       const std::vector<char>& headerVect)
{
  if (_RCVV[0] != headerVect[0]
      || _RCVV[1] != headerVect[1]
      || _RCVV[2] != headerVect[2]
      || _RCVV[3] != headerVect[3])
  {
    header._status = RC_WRONG_MAGIC_STRING;
    return false;
  }
  std::memcpy(header._rcId, &headerVect[0], _rcVVSz);
  StatusType status(RC_NONE);
  std::memcpy(&status, &headerVect[_rcVVSz], sizeof(uint32_t));
  header._status = status;
  if (header._status >= RC_NUMBER_STATUS_DEFINITIONS)
  {
    header._status = RC_BAD_REQUEST_STATUS;
    return false;
  }
  std::memcpy(&header._daoVersion, &headerVect[_rcVVSz + sizeof(uint32_t)], sizeof(uint32_t));
  std::memcpy(&header._requestId,
    &headerVect[_rcVVSz + 2 * sizeof(uint32_t)], sizeof(uint64_t));
  std::memcpy(&header._payloadSize,
    &headerVect[_rcVVSz + 2 * sizeof(uint32_t)] + sizeof(uint64_t), sizeof(uint64_t));
  std::memcpy(&header._inflatedSize,
    &headerVect[_rcVVSz + 2 * sizeof(uint32_t) + 2 * sizeof(uint64_t)], sizeof(uint64_t));
  return true;
}

} // RemoteCache

} // tse
