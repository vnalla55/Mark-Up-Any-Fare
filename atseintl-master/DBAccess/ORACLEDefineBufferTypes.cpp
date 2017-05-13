//----------------------------------------------------------------------------
//
//     File:           ORACLEDefineBufferTypes.cpp
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

#include "DBAccess/ORACLEDefineBufferTypes.h"

#include "Common/Logger.h"
#include "Common/TSEException.h"
#include "DBAccess/ORACLEAdapter.h"

#include <limits>

#include <oci.h>

namespace tse
{
const int16_t ORACLEDefineBuffer::NULL_INDICATOR = -1;
const int16_t ORACLEDefineBuffer::NOT_NULL_INDICATOR = 0;

static inline log4cxx::LoggerPtr&
getLogger()
{
  static log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.DBAccess.ORACLEDefineBufferTypes"));
  return logger;
}

ORACLEDefineBuffer::ORACLEDefineBuffer(
    uint16_t dataType, uint32_t position, int32_t size, uint32_t count, uint16_t sizeInChars)
  : _dataType(dataType), _position(position), _size(size), _count(count), _sizeInChars(sizeInChars)
{
  _nullIndicator = new int16_t[count];
  _returnedDataLength = new uint16_t[count];
}

ORACLEDefineBuffer::ORACLEDefineBuffer(uint16_t dataType,
                                       uint32_t position,
                                       int32_t size,
                                       uint32_t count)
  : _dataType(dataType), _position(position), _size(size), _count(count), _sizeInChars(0)
{
  _nullIndicator = new int16_t[count];
  _returnedDataLength = new uint16_t[count];
}

ORACLEDefineBuffer::~ORACLEDefineBuffer()
{
  delete[] _nullIndicator;
  delete[] _returnedDataLength;
}

void
ORACLEDefineBuffer::clearBuffers()
{
  clearInternalBuffers();
  clearNullIndicatorBuffers();
  clearDataLengthBuffers();
}

ORACLEDefineBuffer*
ORACLEDefineBuffer::createDefineBuffer(
    uint16_t dataType, uint32_t position, int32_t size, uint32_t count, uint16_t sizeInChars)
{
  switch (dataType)
  {
  case SQLT_INT: // Int
    if (size < 10)
      return new ORACLEIntDefineBuffer(position, count);
    else
      return new ORACLELongDefineBuffer(position, count);
    break;
  case SQLT_NUM: // Numeric
    return new ORACLENumericDefineBuffer(position, count);
    break;
  case SQLT_STR: // Varchar2
  case SQLT_CHR:
    return new ORACLEStringDefineBuffer(position, size, count, sizeInChars);
    break;
  case SQLT_AFC: // Char
  case SQLT_AVC:
    return new ORACLECharDefineBuffer(position, size, count, sizeInChars);
    break;
  case SQLT_DAT: // Date
    return new ORACLEDateDefineBuffer(position, count);
    break;
  case SQLT_TIMESTAMP: // Timestamp
    return new ORACLETimestampDefineBuffer(position, count);
    break;
  default:
    break;
  }
  LOG4CXX_ERROR(getLogger(),
                "Unrecognized column data type: "
                    << dataType << " in ORACLEDefineBuffer::createDefineBuffer()");
  return nullptr;
}

uint16_t
ORACLEDefineBuffer::getDataType() const
{
  return _dataType;
}

uint32_t
ORACLEDefineBuffer::getPosition() const
{
  return _position;
}

int32_t
ORACLEDefineBuffer::getSize() const
{
  return _size;
}

uint32_t
ORACLEDefineBuffer::getCount() const
{
  return _count;
}

uint16_t
ORACLEDefineBuffer::getSizeInChars() const
{
  return _sizeInChars;
}

bool
ORACLEDefineBuffer::isNull(uint32_t index) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount()
                            << ") in ORACLEDefineBuffer::isNull()");

    throw std::runtime_error("Invalid (Buffer Index)");
  }
  if (_nullIndicator[index - 1] == NULL_INDICATOR)
    return true;

  return false;
}

bool
ORACLEDefineBuffer::freeBuffers(ORACLEAdapter& adapter)
{
  return true;
}

int16_t*
ORACLEDefineBuffer::getNullIndicatorBufferPointer()
{
  return _nullIndicator;
}

uint16_t*
ORACLEDefineBuffer::getReturnedDataLengthBufferPointer()
{
  return _returnedDataLength;
}

bool
ORACLEDefineBuffer::getReturnedDataLength(uint32_t index, uint16_t& out) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount() << ") "
                            << "in ORACLEDefineBuffer::getReturnedDataLength()");
    return false;
  }
  out = _returnedDataLength[index - 1];
  return true;
}

void
ORACLEDefineBuffer::clearNullIndicatorBuffers()
{
  memset(_nullIndicator, NOT_NULL_INDICATOR, getCount() * sizeof(NOT_NULL_INDICATOR));
}

void
ORACLEDefineBuffer::clearDataLengthBuffers()
{
  memset(_returnedDataLength, 0, sizeof(uint16_t) * getCount());
}

ORACLEStringDefineBuffer::ORACLEStringDefineBuffer(uint32_t position,
                                                   int32_t size,
                                                   uint32_t count,
                                                   uint16_t sizeInChars)
  : ORACLEDefineBuffer(SQLT_CHR, position, size + 1, count, sizeInChars)
{
  _data = new char[count * (size + 1)];
}

ORACLEStringDefineBuffer::~ORACLEStringDefineBuffer() { delete[] _data; }

bool
ORACLEStringDefineBuffer::isNull(uint32_t index) const
{
  if (ORACLEDefineBuffer::isNull(index))
    return true;

  // If the column size is greater than 1 then it's not
  //  an indicator column, so if it contains a single space
  //  it's a NULL value.
  //
  if (getSizeInChars() > 1)
  {
    char* value;
    getValue(index, value);

    // Need to check the length of the data value. Some description
    //  columns contain space padding on the front, so we need to
    //  make sure the data value has a length of 1 in order to
    //  correctly apply the "single space is null" rule.
    //
    uint16_t dataLength = 0;
    if (getReturnedDataLength(index, dataLength))
    {
      if (dataLength == 1 && *value == ' ')
      {
        return true;
      }
    }
  }
  return false;
}

int32_t
ORACLEStringDefineBuffer::setOracleDefine(ORACLEAdapter& adapter)
{
  OCIDefine* ociDefine;
  int32_t status = OCIDefineByPos(adapter.getOCIStatement(),
                                  &ociDefine,
                                  adapter.getOCIError(),
                                  getPosition(),
                                  _data,
                                  getSize(),
                                  getDataType(),
                                  getNullIndicatorBufferPointer(),
                                  getReturnedDataLengthBufferPointer(),
                                  nullptr,
                                  OCI_DEFAULT);

  return status;
}

void
ORACLEStringDefineBuffer::clearInternalBuffers()
{
  memset(_data, 0, getSize() * getCount());
}

bool
ORACLEStringDefineBuffer::getValue(uint32_t index, char*& out) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount()
                            << ") in ORACLEStringDefineBuffer::getValue()");
    return false;
  }

  uint16_t dataLength = 0;
  if (!getReturnedDataLength(index, dataLength))
    return false;

  char* chunk = _data + ((index - 1) * getSize());
  chunk[dataLength] = 0; // Add the null terminator
  out = chunk;

  return true;
}

ORACLECharDefineBuffer::ORACLECharDefineBuffer(uint32_t position,
                                               int32_t size,
                                               uint32_t count,
                                               uint16_t sizeInChars)
  : ORACLEDefineBuffer(SQLT_AFC, position, size + 1, count, sizeInChars)
{
  _data = new char[count * (size + 1)];
}

ORACLECharDefineBuffer::~ORACLECharDefineBuffer() { delete[] _data; }

bool
ORACLECharDefineBuffer::isNull(uint32_t index) const
{
  if (ORACLEDefineBuffer::isNull(index))
    return true;

  // If the column size is greater than 1 then it's not
  //  an indicator column, so if it contains a single space
  //  it's a NULL value.
  //
  if (getSizeInChars() > 1)
  {
    char* value;
    getValue(index, value);

    // Need to check the length of the data value. Some description
    //  columns contain space padding on the front, so we need to
    //  make sure the data value has a length of 1 in order to
    //  correctly apply the "single space is null" rule.
    //
    uint16_t dataLength = 0;
    if (getReturnedDataLength(index, dataLength))
    {
      if (dataLength == 1 && *value == ' ')
      {
        return true;
      }
    }
  }
  return false;
}

int32_t
ORACLECharDefineBuffer::setOracleDefine(ORACLEAdapter& adapter)
{
  OCIDefine* ociDefine;
  int32_t status = OCIDefineByPos(adapter.getOCIStatement(),
                                  &ociDefine,
                                  adapter.getOCIError(),
                                  getPosition(),
                                  _data,
                                  getSize(),
                                  getDataType(),
                                  getNullIndicatorBufferPointer(),
                                  getReturnedDataLengthBufferPointer(),
                                  nullptr,
                                  OCI_DEFAULT);
  return status;
}

void
ORACLECharDefineBuffer::clearInternalBuffers()
{
  memset(_data, 0, getSize() * getCount());
}

bool
ORACLECharDefineBuffer::getValue(uint32_t index, char*& out) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount()
                            << ") in ORACLECharDefineBuffer::getValue()");
    return false;
  }

  uint16_t dataLength = 0;
  if (!getReturnedDataLength(index, dataLength))
    return false;

  char* chunk = _data + ((index - 1) * getSize());
  chunk[dataLength] = 0; // Add the null terminator
  out = chunk;

  return true;
}

ORACLEIntDefineBuffer::ORACLEIntDefineBuffer(uint32_t position, uint32_t count)
  : ORACLEDefineBuffer(SQLT_INT, position, sizeof(int32_t), count)
{
  _data = new int32_t[count];
}

ORACLEIntDefineBuffer::~ORACLEIntDefineBuffer() { delete[] _data; }

int32_t
ORACLEIntDefineBuffer::setOracleDefine(ORACLEAdapter& adapter)
{
  OCIDefine* ociDefine;
  int32_t status = OCIDefineByPos(adapter.getOCIStatement(),
                                  &ociDefine,
                                  adapter.getOCIError(),
                                  getPosition(),
                                  _data,
                                  getSize(),
                                  getDataType(),
                                  getNullIndicatorBufferPointer(),
                                  nullptr,
                                  nullptr,
                                  OCI_DEFAULT);
  return status;
}

void
ORACLEIntDefineBuffer::clearInternalBuffers()
{
  memset(_data, 0, sizeof(int32_t) * getCount());
}

bool
ORACLEIntDefineBuffer::getValue(uint32_t index, int32_t& out) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount()
                            << ") in ORACLEIntDefineBuffer::getValue()");
    return false;
  }
  out = _data[index - 1];

  return true;
}

ORACLELongDefineBuffer::ORACLELongDefineBuffer(uint32_t position, uint32_t count)
  : ORACLEDefineBuffer(SQLT_INT, position, sizeof(int64_t), count)
{
  _data = new int64_t[count];
}

ORACLELongDefineBuffer::~ORACLELongDefineBuffer() { delete[] _data; }

int32_t
ORACLELongDefineBuffer::setOracleDefine(ORACLEAdapter& adapter)
{
  OCIDefine* ociDefine;
  int32_t status = OCIDefineByPos(adapter.getOCIStatement(),
                                  &ociDefine,
                                  adapter.getOCIError(),
                                  getPosition(),
                                  _data,
                                  getSize(),
                                  getDataType(),
                                  getNullIndicatorBufferPointer(),
                                  nullptr,
                                  nullptr,
                                  OCI_DEFAULT);
  return status;
}

void
ORACLELongDefineBuffer::clearInternalBuffers()
{
  memset(_data, 0, sizeof(int64_t) * getCount());
}

bool
ORACLELongDefineBuffer::getValue(uint32_t index, int64_t& out) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount()
                            << ") in ORACLEIntDefineBuffer::getValue()");
    return false;
  }
  out = _data[index - 1];

  return true;
}

ORACLENumericDefineBuffer::ORACLENumericDefineBuffer(uint32_t position, uint32_t count)
  : ORACLEDefineBuffer(SQLT_NUM, position, 21, count)
{
  _data = new unsigned char[getSize() * count];
}

ORACLENumericDefineBuffer::~ORACLENumericDefineBuffer() { delete[] _data; }

int32_t
ORACLENumericDefineBuffer::setOracleDefine(ORACLEAdapter& adapter)
{
  OCIDefine* ociDefine;
  int32_t status = OCIDefineByPos(adapter.getOCIStatement(),
                                  &ociDefine,
                                  adapter.getOCIError(),
                                  getPosition(),
                                  _data,
                                  getSize(),
                                  getDataType(),
                                  getNullIndicatorBufferPointer(),
                                  getReturnedDataLengthBufferPointer(),
                                  nullptr,
                                  OCI_DEFAULT);
  return status;
}

void
ORACLENumericDefineBuffer::clearInternalBuffers()
{
  memset(_data, 0, getSize() * getCount());
}

bool
ORACLENumericDefineBuffer::getIntValue(uint32_t index, int& out) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount()
                            << ") in ORACLENumericDefineBuffer::getIntValue()");
    return false;
  }

  long long int value = 0;
  if (!getLongLongValue(index, value))
    return false;

  if (value > std::numeric_limits<int>::max() || value < std::numeric_limits<int>::min())
  {
    LOG4CXX_ERROR(getLogger(), "NUMBER value: " << value << " exceeds range of datatype int");
    return false;
  }

  out = value;
  return true;
}

bool
ORACLENumericDefineBuffer::getLongValue(uint32_t index, long& out) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount()
                            << ") in ORACLENumericDefineBuffer::getLongValue()");
    return false;
  }

  long long int value = 0;
  if (!getLongLongValue(index, value))
    return false;

  if (value > std::numeric_limits<long>::max() || value < std::numeric_limits<long>::min())
  {
    LOG4CXX_ERROR(getLogger(), "NUMBER value: " << value << " exceeds range of datatype long");
    return false;
  }

  out = value;
  return true;
}

bool
ORACLENumericDefineBuffer::getLongLongValue(uint32_t index, long long& out) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount()
                            << ") in ORACLENumericDefineBuffer::getLongLongValue()");
    return false;
  }

  // This conversion algorithm converts integer values from
  //  the proprietary Oracle 21 byte format to C++ long long int.
  //  The range of values which can be accurately converted is:
  //  -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
  //  Values outside this range cannot be accurately represented by a
  //  64-bit C++ long long int
  //
  // Oracle NUMBER datatype format
  //  First byte is the exponent.
  //  High-order bit of the exponent is the sign bit.
  //
  //
  unsigned char* bufferChunk = _data + ((index - 1) * getSize());
  bool isPositive = 0x80 == (bufferChunk[0] & 0x80);

  int exponent = bufferChunk[0] & 0x7F; // Mask off the sign bit
  exponent = isPositive ? exponent - 65 : 65 - exponent;
  long long int value = 0;
  uint16_t dataLength = 0;

  if (!getReturnedDataLength(index, dataLength))
    return false;

  if (!isPositive)
    --dataLength;

  for (int i = 1; i < dataLength; i++)
  {
    if (i - 1 <= exponent)
    {
      value *= 100;
      int digit = (isPositive ? (bufferChunk[i] - 1) : (101 - bufferChunk[i]));
      if (isPositive && std::numeric_limits<long long>::max() - digit < value)
      {
        LOG4CXX_ERROR(getLogger(),
                      "NUMBER value: (" << value << " + " << digit
                                        << ") exceeds range of datatype long long");
        return false;
      }
      else if (UNLIKELY(!isPositive && std::numeric_limits<long long>::min() + digit > -value))
      {
        LOG4CXX_ERROR(getLogger(),
                      "NUMBER value: -(" << value << " + " << digit
                                         << ") exceeds range of datatype long long");
        return false;
      }
      value += digit;
    }
    else
    {
      break;
    }
  }
  if (isPositive)
  {
    for (int e = 0; e < (exponent + 1 - (dataLength - 1)); e++)
      value *= 100;
  }
  else
  {
    for (int e = 0; e < (exponent - 1 - dataLength); e++)
      value *= 100;
  }
  out = isPositive ? value : -value;

  return true;
}

ORACLEDateDefineBuffer::ORACLEDateDefineBuffer(uint32_t position, uint32_t count)
  : ORACLEDefineBuffer(SQLT_ODT, position, sizeof(OCIDate), count)
// ORACLEDefineBuffer(SQLT_DAT, position, sizeof(OCIDate), count)
{
  _data = new OCIDate[count];
}

ORACLEDateDefineBuffer::~ORACLEDateDefineBuffer() { delete[] _data; }

int32_t
ORACLEDateDefineBuffer::setOracleDefine(ORACLEAdapter& adapter)
{
  OCIDefine* ociDefine;
  int32_t status = OCIDefineByPos(adapter.getOCIStatement(),
                                  &ociDefine,
                                  adapter.getOCIError(),
                                  getPosition(),
                                  _data,
                                  getSize(),
                                  getDataType(),
                                  getNullIndicatorBufferPointer(),
                                  nullptr,
                                  nullptr,
                                  OCI_DEFAULT);

  return status;
}

void
ORACLEDateDefineBuffer::clearInternalBuffers()
{
}

bool
ORACLEDateDefineBuffer::getValue(uint32_t index, DateTime& out) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount()
                            << ") in ORACLEDateDefineBuffer::getValue()");
    return false;
  }

  int16_t year(0);
  uint8_t month(0), day(0), hour(0), min(0), sec(0);

  OCIDate* ociDate = (OCIDate*)(_data + (index - 1));

  OCIDateGetDate(ociDate, &year, &month, &day);
  OCIDateGetTime(ociDate, &hour, &min, &sec);

  if (year == 0)
  {
    out = boost::date_time::neg_infin;
  }
  else if (year >= 9999 && month == 12 && day == 31)
  {
    out = boost::date_time::pos_infin;
  }
  else
  {
    DateTime dt(year, month, day, hour, min, sec);
    out = dt;
  }
  return true;
}

ORACLETimestampDefineBuffer::ORACLETimestampDefineBuffer(uint32_t position, uint32_t count)
  : ORACLEDefineBuffer(SQLT_TIMESTAMP, position, sizeof(OCIDateTime*), count),
    _descriptorAllocated(false),
    _ociEnv(nullptr)
{
  _data = new OCIDateTime* [count];
}

ORACLETimestampDefineBuffer::~ORACLETimestampDefineBuffer() { delete[] _data; }

int32_t
ORACLETimestampDefineBuffer::setOracleDefine(ORACLEAdapter& adapter)
{
  OCIDefine* ociDefine;
  OCIArrayDescriptorAlloc(
      adapter.getOCIEnvironment(), (void**)_data, OCI_DTYPE_TIMESTAMP, getCount(), 0, nullptr);

  _ociEnv = adapter.getOCIEnvironment();
  _descriptorAllocated = true;

  int32_t status = OCIDefineByPos(adapter.getOCIStatement(),
                                  &ociDefine,
                                  adapter.getOCIError(),
                                  getPosition(),
                                  _data,
                                  getSize(),
                                  getDataType(),
                                  getNullIndicatorBufferPointer(),
                                  nullptr,
                                  nullptr,
                                  OCI_DEFAULT);

  return status;
}

bool
ORACLETimestampDefineBuffer::freeBuffers(ORACLEAdapter& adapter)
{
  if (_descriptorAllocated)
  {
    // Compare the current OCI Environment handle from the adapter with
    // the one that was used to allocate the descriptor array. If it is
    // different, then an error must have occurred and the original
    // environment has been destroyed along with any associated resources
    // including any OCI descriptors. In this situation we don't need to
    // call OCIArrayDescriptorFree and if we do it will cause a segfault.
    //
    if (_ociEnv && _ociEnv == adapter.getOCIEnvironment())
    {
      OCIArrayDescriptorFree((void**)_data, OCI_DTYPE_TIMESTAMP);
    }
    _descriptorAllocated = false;
  }
  return true;
}

void
ORACLETimestampDefineBuffer::clearInternalBuffers()
{
}

bool
ORACLETimestampDefineBuffer::getValue(uint32_t index, DateTime& out, ORACLEAdapter& adapter) const
{
  if (index == 0 || index > getCount())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Index: " << index << " out of range (1-" << getCount()
                            << ") in ORACLETimestampDefineBuffer::getValue()");
    return false;
  }

  int16_t year(0);
  uint8_t month(0), day(0), hour(0), min(0), sec(0);
  uint32_t fsec(0);

  OCIDateTime* ociDateTime = (OCIDateTime*)(_data[(index - 1)]);

  OCIDateTimeGetDate(
      adapter.getOCIEnvironment(), adapter.getOCIError(), ociDateTime, &year, &month, &day);
  OCIDateTimeGetTime(
      adapter.getOCIEnvironment(), adapter.getOCIError(), ociDateTime, &hour, &min, &sec, &fsec);

  fsec /= 1000;

  if (year == 0)
  {
    out = boost::date_time::neg_infin;
  }
  else if (year >= 9999 && month == 12 && day == 31)
  {
    out = boost::date_time::pos_infin;
  }
  else
  {
    DateTime dt(year, month, day, hour, min, sec, fsec);
    out = dt;
  }
  return true;
}
}
