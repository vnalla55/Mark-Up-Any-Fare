//----------------------------------------------------------------------------
//
//     File:           ORACLEDefineBufferCollection.cpp
//     Description:
//     Created:        10/12/2009
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright @2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//----------------------------------------------------------------------------

#include "DBAccess/ORACLEDefineBufferCollection.h"

#include "DBAccess/ORACLEAdapter.h"

#include <oci.h>

namespace tse
{
boost::mutex ORACLEDefineBufferCollection::_mutex;
ORACLEDefineBufferCollection::BufferStackMap ORACLEDefineBufferCollection::_bufferCache;

log4cxx::LoggerPtr
ORACLEDefineBufferCollection::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ORACLEDefineBufferCollection"));

log4cxx::LoggerPtr&
ORACLEDefineBufferCollection::getLogger()
{
  return _logger;
}

ORACLEDefineBufferCollection::~ORACLEDefineBufferCollection() { deleteDefineBuffers(); }

bool
ORACLEDefineBufferCollection::createDefineBuffers(ORACLEAdapter& adapter,
                                                  uint16_t arrayFetchSize,
                                                  uint16_t maxBufferStackSize,
                                                  int32_t& oracleStatusCode)
{
  deleteDefineBuffers();

  if (arrayFetchSize > 0)
    _arrayFetchSize = arrayFetchSize;
  else
    _arrayFetchSize = 1;

  if (maxBufferStackSize > 0)
    _maxBufferStackSize = maxBufferStackSize;

  uint32_t position = 1;
  int32_t status = OCI_SUCCESS;

  for (; status == OCI_SUCCESS; ++position)
  {
    OCIParam* ociColumnParam;
    status = OCIParamGet(adapter.getOCIStatement(),
                         OCI_HTYPE_STMT,
                         adapter.getOCIError(),
                         (void**)&ociColumnParam,
                         position);

    if (status != OCI_SUCCESS)
    {
      // An error is used to indicate that we've reached the end of the
      // columns in the SELECT list
      //
      if (status == OCI_ERROR)
      {
        int32_t errorCode = 0;
        text buf[20];
        OCIErrorGet(
            adapter.getOCIError(), 1, (text*)nullptr, &errorCode, buf, sizeof(buf), OCI_HTYPE_ERROR);

        if (errorCode == 24334) // We've reached the end of the SELECT list
        {
          // There's no real error, so reset the status code
          status = OCI_SUCCESS;
        }
      }
      OCIDescriptorFree(ociColumnParam, OCI_DTYPE_PARAM);
      break;
    }

    uint16_t dataType = 0;

    // Get the data type of the column
    status = OCIAttrGet((void*)ociColumnParam,
                        OCI_DTYPE_PARAM,
                        (void*)&dataType,
                        nullptr,
                        OCI_ATTR_DATA_TYPE,
                        adapter.getOCIError());

    if (status != OCI_SUCCESS)
    {
      OCIDescriptorFree(ociColumnParam, OCI_DTYPE_PARAM);
      break;
    }

    uint16_t size = 0;

    // Get the length of the column
    status = OCIAttrGet((void*)ociColumnParam,
                        OCI_DTYPE_PARAM,
                        (void*)&size,
                        nullptr,
                        OCI_ATTR_DATA_SIZE,
                        adapter.getOCIError());

    if (status != OCI_SUCCESS)
    {
      OCIDescriptorFree(ociColumnParam, OCI_DTYPE_PARAM);
      break;
    }

    uint16_t sizeInChars = 0;

    // Get the size of the column in characters
    status = OCIAttrGet((void*)ociColumnParam,
                        OCI_DTYPE_PARAM,
                        (void*)&sizeInChars,
                        nullptr,
                        OCI_ATTR_CHAR_SIZE,
                        adapter.getOCIError());

    if (status != OCI_SUCCESS)
    {
      OCIDescriptorFree(ociColumnParam, OCI_DTYPE_PARAM);
      break;
    }

    OCIDescriptorFree(ociColumnParam, OCI_DTYPE_PARAM);

    ORACLEDefineBuffer* defineBuffer = ORACLEDefineBuffer::createDefineBuffer(
        dataType, position, size, _arrayFetchSize, sizeInChars);

    if (!defineBuffer)
      return false;

    addDefineBuffer(defineBuffer);
  }

  oracleStatusCode = status;

  if (status == OCI_SUCCESS)
    return true;

  return false;
}

void
ORACLEDefineBufferCollection::deleteDefineBuffers()
{
  for (auto oracleBuffer : _defineBuffers)
    delete oracleBuffer;

  _defineBuffers.clear();
}

void
ORACLEDefineBufferCollection::clearDefineBuffers(ORACLEAdapter& adapter, int32_t& oracleStatusCode)
{
  for (const auto oracleBuffer : _defineBuffers)
    oracleBuffer->clearBuffers();

  oracleStatusCode = OCI_SUCCESS;
}

void
ORACLEDefineBufferCollection::freeDefineBuffers(ORACLEAdapter& adapter)
{
  for (const auto oracleBuffer : _defineBuffers)
    oracleBuffer->freeBuffers(adapter);
}

bool
ORACLEDefineBufferCollection::setOracleDefines(ORACLEAdapter& adapter, int32_t& oracleStatusCode)
{
  int32_t status = OCI_ERROR;

  for (const auto oracleBuffer : _defineBuffers)
  {
    status = oracleBuffer->setOracleDefine(adapter);

    if (status != OCI_SUCCESS)
      break;
  }
  oracleStatusCode = status;

  if (status == OCI_SUCCESS)
    return true;

  return false;
}

int32_t
ORACLEDefineBufferCollection::getBufferCount() const
{
  return _defineBuffers.size();
}

uint16_t
ORACLEDefineBufferCollection::getDataType(int index) const
{
  const ORACLEDefineBuffer* buffer = getDefineBuffer(index);
  if (buffer)
  {
    return buffer->getDataType();
  }
  LOG4CXX_FATAL(getLogger(), "Invalid buffer index requested: " << index);

  throw std::runtime_error("INTERNAL SYSTEM ERROR");
}

int32_t
ORACLEDefineBufferCollection::getSize(int index) const
{
  const ORACLEDefineBuffer* buffer = getDefineBuffer(index);
  if (buffer)
  {
    return buffer->getSize();
  }
  LOG4CXX_FATAL(getLogger(), "Invalid buffer index requested: " << index);

  throw std::runtime_error("INTERNAL SYSTEM ERROR");
}

uint16_t
ORACLEDefineBufferCollection::getSizeInChars(int index) const
{
  const ORACLEDefineBuffer* buffer = getDefineBuffer(index);
  if (buffer)
  {
    return buffer->getSizeInChars();
  }
  LOG4CXX_FATAL(getLogger(), "Invalid buffer index requested: " << index);

  throw std::runtime_error("INTERNAL SYSTEM ERROR");
}

int
ORACLEDefineBufferCollection::getInt(int columnIndex, uint16_t rowIndex)
{
  const ORACLEDefineBuffer* buffer = getDefineBuffer(columnIndex);
  if (buffer)
  {
    // First try Int buffer type
    //
    const ORACLEIntDefineBuffer* intBuffer = buffer->toIntDefineBuffer();
    if (intBuffer)
    {
      int value(0);
      if (intBuffer->getValue(rowIndex, value))
      {
        return value;
      }
    }
    // Try Long buffer type
    //
    const ORACLELongDefineBuffer* longBuffer = buffer->toLongDefineBuffer();
    if (longBuffer)
    {
      int64_t value(0);
      if (longBuffer->getValue(rowIndex, value))
      {
        if (value > std::numeric_limits<int>::max() || value < std::numeric_limits<int>::min())
        {
          LOG4CXX_FATAL(getLogger(), "Long value: " << value << " exceeds range of datatype int");

          throw std::runtime_error("INTERNAL SYSTEM ERROR");
        }
        return value;
      }
    }
    // Try Numeric buffer type
    //
    const ORACLENumericDefineBuffer* numericBuffer = buffer->toNumericDefineBuffer();
    if (numericBuffer)
    {
      int value(0);
      if (numericBuffer->getIntValue(rowIndex, value))
      {
        return value;
      }
      else
      {
        LOG4CXX_FATAL(getLogger(), "Data type range error");

        throw std::runtime_error("INTERNAL SYSTEM ERROR");
      }
    }
    // Try Char buffer type
    //
    const ORACLECharDefineBuffer* charBuffer = buffer->toCharDefineBuffer();
    if (charBuffer)
    {
      char* value;
      if (charBuffer->getValue(rowIndex, value))
      {
        return atoi(value);
      }
    }
    // Try String buffer type
    //
    const ORACLEStringDefineBuffer* stringBuffer = buffer->toStringDefineBuffer();
    if (stringBuffer)
    {
      char* value;
      if (stringBuffer->getValue(rowIndex, value))
      {
        return atoi(value);
      }
    }
    LOG4CXX_FATAL(getLogger(),
                  "Data type mismatch on column: " << columnIndex
                                                   << " Requested type: int, actual column type: "
                                                   << buffer->getDataType());

    throw std::runtime_error("INTERNAL SYSTEM ERROR");
  }
  LOG4CXX_FATAL(getLogger(), "Invalid buffer index requested: " << columnIndex);

  throw std::runtime_error("INTERNAL SYSTEM ERROR");
}

long int
ORACLEDefineBufferCollection::getLong(int columnIndex, uint16_t rowIndex)
{
  const ORACLEDefineBuffer* buffer = getDefineBuffer(columnIndex);
  if (buffer)
  {
    // First try Long buffer type
    //
    const ORACLELongDefineBuffer* longBuffer = buffer->toLongDefineBuffer();
    if (longBuffer)
    {
      int64_t value(0);
      if (longBuffer->getValue(rowIndex, value))
      {
        return value;
      }
    }
    // Try Int buffer type
    //
    const ORACLEIntDefineBuffer* intBuffer = buffer->toIntDefineBuffer();
    if (intBuffer)
    {
      int value(0);
      if (intBuffer->getValue(rowIndex, value))
      {
        return value;
      }
    }
    // Try Numeric buffer type
    //
    const ORACLENumericDefineBuffer* numericBuffer = buffer->toNumericDefineBuffer();
    if (numericBuffer)
    {
      long value(0);
      if (numericBuffer->getLongValue(rowIndex, value))
      {
        return value;
      }
      else
      {
        LOG4CXX_FATAL(getLogger(), "Data type range error");
        throw std::runtime_error("INTERNAL SYSTEM ERROR");
      }
    }
    // Try Char buffer type
    //
    const ORACLECharDefineBuffer* charBuffer = buffer->toCharDefineBuffer();
    if (charBuffer)
    {
      char* value;
      if (charBuffer->getValue(rowIndex, value))
      {
        return atol(value);
      }
    }
    // Try String buffer type
    //
    const ORACLEStringDefineBuffer* stringBuffer = buffer->toStringDefineBuffer();
    if (stringBuffer)
    {
      char* value;
      if (stringBuffer->getValue(rowIndex, value))
      {
        return atol(value);
      }
    }
    LOG4CXX_FATAL(getLogger(),
                  "Data type mismatch on column: " << columnIndex
                                                   << " Requested type: long, actual column type: "
                                                   << buffer->getDataType());

    throw std::runtime_error("INTERNAL SYSTEM ERROR");
  }
  LOG4CXX_FATAL(getLogger(), "Invalid buffer index requested: " << columnIndex);

  throw std::runtime_error("INTERNAL SYSTEM ERROR");
}

long long int
ORACLEDefineBufferCollection::getLongLong(int columnIndex, uint16_t rowIndex)
{
  const ORACLEDefineBuffer* buffer = getDefineBuffer(columnIndex);
  if (buffer)
  {
    // First try Long buffer type
    //
    const ORACLELongDefineBuffer* longBuffer = buffer->toLongDefineBuffer();
    if (longBuffer)
    {
      int64_t value(0);
      if (longBuffer->getValue(rowIndex, value))
      {
        return value;
      }
    }
    // Try Int buffer type
    //
    const ORACLEIntDefineBuffer* intBuffer = buffer->toIntDefineBuffer();
    if (intBuffer)
    {
      int value(0);
      if (intBuffer->getValue(rowIndex, value))
      {
        return value;
      }
    }
    // Try Numeric buffer type
    //
    const ORACLENumericDefineBuffer* numericBuffer = buffer->toNumericDefineBuffer();
    if (numericBuffer)
    {
      long long value(0);
      if (numericBuffer->getLongLongValue(rowIndex, value))
      {
        return value;
      }
      else
      {
        LOG4CXX_FATAL(getLogger(), "Data type range error");
        throw std::runtime_error("INTERNAL SYSTEM ERROR");
      }
    }
    // Try Char buffer type
    //
    const ORACLECharDefineBuffer* charBuffer = buffer->toCharDefineBuffer();
    if (charBuffer)
    {
      char* value;
      if (charBuffer->getValue(rowIndex, value))
      {
        return atol(value);
      }
    }
    // Try String buffer type
    //
    const ORACLEStringDefineBuffer* stringBuffer = buffer->toStringDefineBuffer();
    if (stringBuffer)
    {
      char* value;
      if (stringBuffer->getValue(rowIndex, value))
      {
        return atol(value);
      }
    }
    LOG4CXX_FATAL(getLogger(),
                  "Data type mismatch on column: "
                      << columnIndex << " Requested type: long long, actual column type: "
                      << buffer->getDataType());

    throw std::runtime_error("INTERNAL SYSTEM ERROR");
  }
  LOG4CXX_FATAL(getLogger(), "Invalid buffer index requested: " << columnIndex);

  throw std::runtime_error("INTERNAL SYSTEM ERROR");
}

const char*
ORACLEDefineBufferCollection::getString(int columnIndex, uint16_t rowIndex)
{
  const ORACLEDefineBuffer* buffer = getDefineBuffer(columnIndex);
  if (buffer)
  {
    // First try String buffer type
    //
    const ORACLEStringDefineBuffer* stringBuffer = buffer->toStringDefineBuffer();
    if (stringBuffer)
    {
      char* value;
      if (stringBuffer->getValue(rowIndex, value))
      {
        return value;
      }
    }
    // Try Char buffer type
    //
    const ORACLECharDefineBuffer* charBuffer = buffer->toCharDefineBuffer();
    if (charBuffer)
    {
      char* value;
      if (charBuffer->getValue(rowIndex, value))
      {
        return value;
      }
    }
    LOG4CXX_FATAL(getLogger(),
                  "Data type mismatch on column: " << columnIndex
                                                   << " Requested type: char*, actual column type: "
                                                   << buffer->getDataType());

    throw std::runtime_error("INTERNAL SYSTEM ERROR");
  }
  LOG4CXX_FATAL(getLogger(), "Invalid buffer index requested: " << columnIndex);

  throw std::runtime_error("INTERNAL SYSTEM ERROR");
}

char
ORACLEDefineBufferCollection::getChar(int columnIndex, uint16_t rowIndex)
{
  const ORACLEDefineBuffer* buffer = getDefineBuffer(columnIndex);
  if (buffer)
  {
    // Try Char buffer type first
    //
    const ORACLECharDefineBuffer* charBuffer = buffer->toCharDefineBuffer();
    if (charBuffer)
    {
      char* value;
      if (charBuffer->getValue(rowIndex, value))
      {
        return value[0];
      }
    }
    // Try String buffer type
    //
    const ORACLEStringDefineBuffer* stringBuffer = buffer->toStringDefineBuffer();
    if (stringBuffer)
    {
      char* value;
      if (stringBuffer->getValue(rowIndex, value))
      {
        return value[0];
      }
    }
    // Try Int buffer type
    //
    const ORACLEIntDefineBuffer* intBuffer = buffer->toIntDefineBuffer();
    if (intBuffer)
    {
      int value(0);
      if (intBuffer->getValue(rowIndex, value))
      {
        if (value >= 0 && value <= 9)
          return (char)value + 48; // '0' = 48, '9' = 57
      }
    }
    // Try Numeric buffer type
    //
    const ORACLENumericDefineBuffer* numericBuffer = buffer->toNumericDefineBuffer();
    if (numericBuffer)
    {
      int value(0);
      if (numericBuffer->getIntValue(rowIndex, value))
      {
        if (value >= 0 && value <= 9)
          return (char)value + 48; // '0' = 48, '9' = 57
      }
    }
    LOG4CXX_FATAL(getLogger(),
                  "Data type mismatch on column: " << columnIndex
                                                   << " Requested type: char, actual column type: "
                                                   << buffer->getDataType());

    throw std::runtime_error("INTERNAL SYSTEM ERROR");
  }
  LOG4CXX_FATAL(getLogger(), "Invalid buffer index requested: " << columnIndex);

  throw std::runtime_error("INTERNAL SYSTEM ERROR");
}

DateTime
ORACLEDefineBufferCollection::getDateTime(int columnIndex,
                                          uint16_t rowIndex,
                                          ORACLEAdapter& adapter)
{
  const ORACLEDefineBuffer* buffer = getDefineBuffer(columnIndex);
  if (buffer)
  {
    // First try Date buffer type
    //
    const ORACLEDateDefineBuffer* dateBuffer = buffer->toDateDefineBuffer();
    if (dateBuffer)
    {
      DateTime value;
      if (dateBuffer->getValue(rowIndex, value))
      {
        return value;
      }
    }
    // Try Timestamp buffer type
    //
    const ORACLETimestampDefineBuffer* timestampBuffer = buffer->toTimestampDefineBuffer();
    if (timestampBuffer)
    {
      DateTime value;
      if (timestampBuffer->getValue(rowIndex, value, adapter))
      {
        return value;
      }
    }
    LOG4CXX_FATAL(getLogger(),
                  "Data type mismatch on column: "
                      << columnIndex << " Requested type: DateTime, actual column type: "
                      << buffer->getDataType());

    throw std::runtime_error("INTERNAL SYSTEM ERROR");
  }
  LOG4CXX_FATAL(getLogger(), "Invalid buffer index requested: " << columnIndex);

  throw std::runtime_error("INTERNAL SYSTEM ERROR");
}

bool
ORACLEDefineBufferCollection::isNull(int columnIndex, uint16_t rowIndex)
{
  const ORACLEDefineBuffer* buffer = getDefineBuffer(columnIndex);
  if (buffer)
  {
    return buffer->isNull(rowIndex);
  }
  LOG4CXX_FATAL(getLogger(), "Invalid buffer index requested: " << columnIndex);

  throw std::runtime_error("INTERNAL SYSTEM ERROR");
}

const ORACLEDefineBuffer*
ORACLEDefineBufferCollection::getDefineBuffer(uint16_t index) const
{
  if (index >= _defineBuffers.size())
    return nullptr;
  return _defineBuffers.at(index);
}

void
ORACLEDefineBufferCollection::addDefineBuffer(ORACLEDefineBuffer* defineBuffer)
{
  _defineBuffers.push_back(defineBuffer);
}

ORACLEDefineBufferCollection*
ORACLEDefineBufferCollection::findDefineBufferCollection(const char* queryName)
{
  boost::lock_guard<boost::mutex> g(_mutex);

  ORACLEDefineBufferCollection* defineBufferCollection = nullptr;

  std::string sQueryName(queryName);
  BufferStackMap::iterator iter = _bufferCache.find(sQueryName);

  if (iter != _bufferCache.end())
  {
    BufferStack& bufferStack((*iter).second);
    if (!bufferStack.empty())
    {
      defineBufferCollection = bufferStack.top();
      bufferStack.pop();

      LOG4CXX_DEBUG(getLogger(), "ORACLE Define buffers reused for query: " << queryName);
    }
  }
  return defineBufferCollection;
}

ORACLEDefineBufferCollection*
ORACLEDefineBufferCollection::createDefineBufferCollection(const char* queryName,
                                                           ORACLEAdapter& adapter,
                                                           uint16_t arrayFetchSize,
                                                           uint16_t maxBufferStackSize,
                                                           int32_t& oracleStatusCode)
{
  ORACLEDefineBufferCollection* defineBufferCollection =
      new ORACLEDefineBufferCollection(queryName);

  if (!defineBufferCollection->createDefineBuffers(
          adapter, arrayFetchSize, maxBufferStackSize, oracleStatusCode))
  {
    delete defineBufferCollection;
    defineBufferCollection = nullptr;
  }
  LOG4CXX_DEBUG(getLogger(), "ORACLE Define buffers created for query: " << queryName);

  return defineBufferCollection;
}

void
ORACLEDefineBufferCollection::releaseDefineBufferCollection(ORACLEDefineBufferCollection*& target,
                                                            ORACLEAdapter& adapter)
{
  if (target)
  {
    target->freeDefineBuffers(adapter);

    { // Mutex lock scope
      boost::lock_guard<boost::mutex> g(_mutex);

      BufferStackMap::iterator iter = _bufferCache.find(target->getQueryName());

      if (iter != _bufferCache.end())
      {
        BufferStack& bufferStack((*iter).second);
        if (bufferStack.size() >= target->getMaxBufferStackSize())
        {
          LOG4CXX_DEBUG(getLogger(),
                        "ORACLE Define buffers deleted for query: "
                            << target->getQueryName() << " stack size: " << bufferStack.size());

          delete target;
          target = nullptr;
        }
        else
        {
          bufferStack.push(target);

          LOG4CXX_DEBUG(getLogger(),
                        "ORACLE Define buffers saved for query: "
                            << target->getQueryName() << " stack size: " << bufferStack.size());
        }
      }
      else
      {
        BufferStack& bufferStack(_bufferCache[target->getQueryName()]);
        bufferStack.push(target);

        LOG4CXX_DEBUG(getLogger(),
                      "ORACLE Define buffers saved for query: "
                          << target->getQueryName() << " stack size: " << bufferStack.size());
      }
    }
    target = nullptr;
  }
}
}
