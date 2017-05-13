#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/RemoteCache/CurrentDatabase.h"
#include "DBAccess/RemoteCache/ReadConfig.h"
#include "DBAccess/RemoteCache/RemoteCacheHeader.h"
#include "DBAccess/RemoteCache/ASIOClient/ASIORequest.h"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

namespace tse
{

namespace RemoteCache
{

typedef std::shared_ptr<struct RCRequest> RCRequestPtr;

struct RCRequest : public ASIORequest
{
  explicit RCRequest(const std::string& dataType);

  RCRequest(const std::string& dataType,
            uint32_t timeout);

  virtual ~RCRequest();

  virtual size_t fillSendBuffers(std::vector<boost::asio::const_buffer>& buffers) override;
  virtual size_t convertToBuffers(std::vector<boost::asio::const_buffer>& buffers) override;

  virtual boost::tribool
  readResponse(const std::vector<char>& buffer, size_t bytesTransferred) override;

  virtual void onConnectTimeout() override;

  virtual void onProcessingTimeout() override;

  virtual long calculateMaxProcessingTime() const override;

  virtual void onReadError(const boost::system::error_code& err) override;

  virtual void onWriteError(const boost::system::error_code& err) override;

  virtual void onConnectError(const boost::system::error_code& err) override;

  virtual void onError(const std::string& message = "") override;

  virtual void debugOutput(const std::string& text) override;

  virtual void onClientError(int status,
                             const boost::system::error_code& err,
                             const std::string& message = "") override;
  virtual void checkTrxTimeout() const;

  sfc::CompressedDataPtr processResponse();
  boost::posix_time::ptime expiresAt() const;
  static void checkTimeout();

  void wait();
  void cancel();
  void notify();

  const std::string& _dataType;
  const uint64_t _requestId;
  std::vector<char> _requestVector;
  std::vector<char> _responseHeaderVect;
  std::vector<char> _responsePayloadVect;
  RemoteCacheHeader _responseHeader;
  size_t _bytesProcessed{};
  const unsigned long _maxProcessingTime;
  const boost::posix_time::ptime _expiresAt;
  std::chrono::steady_clock::time_point _start;
  std::chrono::steady_clock::time_point _expirationPoint;
  std::mutex _mutex;
  std::condition_variable _condition;
};

typedef std::shared_ptr<struct RCRequestHealthcheck> RCRequestHealthcheckPtr;

struct RCRequestHealthcheck : public RCRequest
{
  RCRequestHealthcheck(const std::string& dataType,
                       uint32_t timeout);
  virtual ~RCRequestHealthcheck();

  virtual void checkTrxTimeout() const override;

  static RCRequestHealthcheckPtr createRequest();
};

template <typename Key> bool populateRequest(RCRequestPtr request,
                                             const Key& key,
                                             bool isHistorical,
                                             uint32_t daoVersion)
{
  bool inTransition(false);
  std::string database(getCurrentDatabase(isHistorical, inTransition));
  bool ignoreDBMismatch(ReadConfig::getIgnoreDatabaseMismatch());
  if (!ignoreDBMismatch && inTransition)
  {
    request->_responseHeader._status = RC_DATABASE_MISMATCH;
    return false;
  }
  else
  {
    WBuffer os;
    bool persistent(ReadConfig::usePersistentConnections());
    os.write(persistent ? 'P' : 'N');
    os.write(BUILD_LABEL_STRING);
    os.write(request->_dataType);
    os.write(isHistorical ? 'H' : 'N');
    os.write(database);
    os.write(key);
    RemoteCacheHeader requestHeader(request->_requestId);
    requestHeader._daoVersion = daoVersion;
    requestHeader._payloadSize = os.size();
    request->_requestVector.resize(_headerSz + os.size());
    writeHeader(requestHeader, request->_requestVector);
    std::memcpy(&request->_requestVector[_headerSz], os.buffer(), os.size());
    return true;
  }
}

}// RemoteCache

}// tse
