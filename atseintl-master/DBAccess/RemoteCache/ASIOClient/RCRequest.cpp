#include "DBAccess/RemoteCache/ASIOClient/RCRequest.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/Currency.h"
#include "DBAccess/CurrencyDAO.h"

#include <atomic>
#include <cmath>

namespace tse
{
FALLBACK_DECL(reworkTrxAborter);
namespace RemoteCache
{

namespace
{

Logger logger("atseintl.DBAccess.RemoteCache.RCRequest");

std::atomic<uint64_t> _globalId(0);

std::atomic<uint64_t> _numberRequests(0);

}// namespace

RCRequest::RCRequest(const std::string& dataType)
  : _dataType(dataType)
  , _requestId(_globalId++)
  , _responseHeader(_requestId)
  , _maxProcessingTime(calculateMaxProcessingTime())
  , _start(std::chrono::steady_clock::now())
  , _expirationPoint(_start + std::chrono::milliseconds(_maxProcessingTime))
{
  ++_numberRequests;
}

RCRequest::RCRequest(const std::string& dataType,
                     uint32_t timeout)
  : _dataType(dataType)
  , _requestId(_globalId++)
  , _responseHeader(_requestId)
  , _maxProcessingTime(timeout * 1000)
  , _start(std::chrono::steady_clock::now())
  , _expirationPoint(_start + std::chrono::milliseconds(_maxProcessingTime))
{
  ++_numberRequests;
}

RCRequest::~RCRequest()
{
  --_numberRequests;
  // LOG4CXX_FATAL(logger, __FUNCTION__ << " _numberRequests=" << _numberRequests);
}

void RCRequest::checkTimeout()
{
  Trx* trx(TseCallableTrxTask::currentTrx());
  if (trx)
  {
/*
    LOG4CXX_WARN(logger, __FUNCTION__ << ':' << "trxId=" << trx->getBaseIntId()
                 << ",trxStart=" << trx->transactionStartTime());
*/
    if (fallback::reworkTrxAborter(trx))
      checkTrxAborted(*trx);
    else
      trx->checkTrxAborted();
  }
}

void RCRequest::checkTrxTimeout() const
{
  checkTimeout();
}

void RCRequest::wait()
{
  std::cv_status status(std::cv_status::no_timeout);
  try
  {
    std::unique_lock<std::mutex> lock(_mutex);
    while (std::cv_status::no_timeout == status
           && RC_NONE == _responseHeader._status)
    {
      status = _condition.wait_until(lock, _expirationPoint);
    }
  }
  catch (...)
  {
    LOG4CXX_WARN(logger, __FUNCTION__ << ":exception");
  }
  if (std::cv_status::timeout == status)
  {
    _responseHeader._status = RC_CLIENT_PROCESSING_TIMEOUT;
  }
}

void RCRequest::cancel()
{
  std::unique_lock<std::mutex> lock(_mutex);
  _responseHeader._status = RC_REQUEST_CANCELED;
  _condition.notify_one();
}

void RCRequest::notify()
{
  std::unique_lock<std::mutex> lock(_mutex);
  _condition.notify_one();
}

boost::posix_time::ptime RCRequest::expiresAt() const
{
  return boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(_maxProcessingTime);
}

long RCRequest::calculateMaxProcessingTime() const
{
  double defaultMaxProcessingTime(ReadConfig::getClientProcessingTimeout());
  bool useSpecifiedTimeout(ReadConfig::isClientSpecifiedTimeout());
  double maxProcessingTime(defaultMaxProcessingTime);
  if (useSpecifiedTimeout)
  {
    Trx* trx(TseCallableTrxTask::currentTrx());
    TrxAborter* aborter(nullptr);
    if (trx && (aborter = trx->aborter()))
    {
      if (trx->isForceNoTimeout())
      {
        maxProcessingTime = INT_MAX;
      }
      else
      {
        time_t expiresAt(aborter->getTimeOutAt());
        if (expiresAt != 0)
        {
          time_t now(std::time(nullptr));
          if (now != -1)
          {
            maxProcessingTime = std::difftime(expiresAt, now);
          }
        }
      }
    }
  }
  long result(std::lround(maxProcessingTime * 1000) + 500);
  if (result < 0)
  {
    result = 0;
  }
  return result;
}

boost::tribool RCRequest::readResponse(const std::vector<char>& buffer,
                                       size_t bytesTransferred)
{
  if (_responseHeader._payloadSize < _bytesProcessed
      || _responseHeaderVect.size() > _headerSz
      || _responsePayloadVect.size() > _responseHeader._payloadSize)
  {
    assert(false);
    return false;
  }
  size_t remainingBytes(bytesTransferred);
  size_t shift(0);
  if (remainingBytes > 0 && _responseHeaderVect.size() < _headerSz)
  {
    if (0 == _responseHeaderVect.capacity())
    {
      _responseHeaderVect.reserve(_headerSz);
    }
    size_t bytesToCopy(std::min(remainingBytes, _headerSz - _responseHeaderVect.size()));
    std::copy(buffer.begin(),
              buffer.begin() + bytesToCopy,
              std::back_inserter(_responseHeaderVect));
    remainingBytes -= bytesToCopy;
    shift += bytesToCopy;
    if (_headerSz == _responseHeaderVect.size())
    {
      if (!readHeader(_responseHeader, _responseHeaderVect))
      {
        return false;
      }
      if (0 == _responseHeader._payloadSize)
      {
        return true;
      }
      else
      {
        _responsePayloadVect.resize(_responseHeader._payloadSize);
      }
    }
  }
  if (_headerSz == _responseHeaderVect.size())
  {
    if (_responseHeader._payloadSize > _bytesProcessed && remainingBytes > 0)
    {
      size_t bytesTocopy(std::min(remainingBytes,
                                  _responseHeader._payloadSize - _bytesProcessed));
      std::memcpy(&_responsePayloadVect[_bytesProcessed],
                  &buffer[shift],
                  bytesTocopy);
      _bytesProcessed += bytesTocopy;
    }
    if (_bytesProcessed == _responseHeader._payloadSize)
    {
      return true;
    }
  }
  return boost::indeterminate;
}

void RCRequest::onConnectTimeout()
{
  _responseHeader._status = RC_CLIENT_CONNECTION_TIMEOUT;
  LOG4CXX_INFO(logger, "connection timeout:" << _requestId);
}

void RCRequest::onProcessingTimeout()
{
  _responseHeader._status = RC_CLIENT_PROCESSING_TIMEOUT;
  LOG4CXX_INFO(logger, "processing timeout:" << _requestId);
}

void RCRequest::onReadError(const boost::system::error_code& err)
{
  _responseHeader._status = RC_READ_ERROR;
  LOG4CXX_INFO(logger, __FUNCTION__ << ' ' << _requestId << ' ' << err.message());
}

void RCRequest::onWriteError(const boost::system::error_code& err)
{
  _responseHeader._status = RC_WRITE_ERROR;
  LOG4CXX_INFO(logger, __FUNCTION__ << ' ' << _requestId << ' ' << err.message());
}

void RCRequest::onConnectError(const boost::system::error_code& err)
{
  switch (err.value())
  {
  case boost::system::errc::address_not_available:
    _responseHeader._status = RC_ADDRESS_NOT_AVAILABLE;
    break;
  default:
    _responseHeader._status = RC_CONNECTION_REFUSED;
    break;
  }
  LOG4CXX_INFO(logger, __FUNCTION__ << ' ' << _requestId << ' ' << err.message());
}

void RCRequest::onError(const std::string& message)
{
  if (RC_COMPRESSED_VALUE == _responseHeader._status
      || RC_UNCOMPRESSED_VALUE == _responseHeader._status
      || RC_NONE == _responseHeader._status)
  {
    _responseHeader._status = RC_CLIENT_ERROR;
  }
  LOG4CXX_WARN(logger, __FUNCTION__ << ' ' << _requestId << ' ' << message);
}

void RCRequest::onClientError(int status,
                              const boost::system::error_code& err,
                              const std::string& text)
{
  assert(status >= RC_NONE && status < RC_NUMBER_STATUS_DEFINITIONS);
  std::string message(text);
  if (err && RC_NONE == status)
  {
    _responseHeader._status = RC_CLIENT_ERROR;
    message = err.message();
  }
  else if (status != RC_NONE)
  {
    _responseHeader._status = static_cast<StatusType>(status);
  }
  std::unique_lock<std::mutex> lock(_mutex);
  _condition.notify_one();
}

void RCRequest::debugOutput(const std::string& text)
{
  LOG4CXX_INFO(logger, __FUNCTION__ << ' ' << _requestId << ',' << text);
}

size_t RCRequest::fillSendBuffers(std::vector<boost::asio::const_buffer>& buffers)
{
  buffers.push_back(boost::asio::buffer(_requestVector));
  return _requestVector.size();
}

size_t RCRequest::convertToBuffers(std::vector<boost::asio::const_buffer>& buffers)
{
  buffers.push_back(boost::asio::buffer(_requestVector, _requestVector.size()));
  return _requestVector.size();
}

sfc::CompressedDataPtr RCRequest::processResponse()
{
  sfc::CompressedDataPtr compressed;
  if (!_responsePayloadVect.empty())
  {
    sfc::CompressedData* ptr(0);
    if (_responsePayloadVect.size() == _responseHeader._payloadSize
        && _responsePayloadVect.capacity() == _responseHeader._payloadSize)
    {
      ptr = new sfc::CompressedData(_responsePayloadVect, _responseHeader._inflatedSize);
    }
    else
    {
      assert(false);
      ptr = new sfc::CompressedData(_responsePayloadVect,
                                    _responseHeader._inflatedSize,
                                    _responsePayloadVect.size());
    }
    compressed.reset(ptr);
  }
  return compressed;
}

RCRequestHealthcheck::RCRequestHealthcheck(const std::string& dataType,
                                           uint32_t timeout)
  : RCRequest(dataType, timeout)
{
}

RCRequestHealthcheck::~RCRequestHealthcheck()
{
}

void RCRequestHealthcheck::checkTrxTimeout() const
{
}

RCRequestHealthcheckPtr RCRequestHealthcheck::createRequest()
{
  CurrencyDAO& dao(CurrencyDAO::instance());
  uint32_t daoVersion(dao.tableVersion());
  std::string dataType(boost::to_upper_copy(dao.name()));
  uint32_t timeout(ReadConfig::getHealthcheckTimeout());
  RCRequestHealthcheckPtr request(new RCRequestHealthcheck(dataType, timeout));
  static const bool isHistorical(false);
  CurrencyKey key("USD");
  populateRequest(request, key, isHistorical, daoVersion);
  return request;
}

}// RemoteCache

}// tse
