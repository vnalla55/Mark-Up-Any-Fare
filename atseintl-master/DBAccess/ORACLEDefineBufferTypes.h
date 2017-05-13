//----------------------------------------------------------------------------
//
//     File:           ORACLEDefineBufferTypes.h
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

#pragma once

#include "Common/DateTime.h"

#include <oci.h>
#include <stdint.h>

namespace tse
{
class ORACLEAdapter;

class ORACLEIntDefineBuffer;
class ORACLELongDefineBuffer;
class ORACLENumericDefineBuffer;
class ORACLECharDefineBuffer;
class ORACLEStringDefineBuffer;
class ORACLETimestampDefineBuffer;
class ORACLEDateDefineBuffer;

class ORACLEDefineBuffer
{
public:
  virtual ~ORACLEDefineBuffer() = 0;

  uint16_t getDataType() const;
  uint32_t getPosition() const;
  int32_t getSize() const;
  uint32_t getCount() const;
  uint16_t getSizeInChars() const;

  virtual bool isNull(uint32_t index) const;

  virtual int32_t setOracleDefine(ORACLEAdapter& adapter) = 0;
  virtual bool freeBuffers(ORACLEAdapter& adapter);

  virtual const ORACLEIntDefineBuffer* toIntDefineBuffer() const { return nullptr; }
  virtual const ORACLELongDefineBuffer* toLongDefineBuffer() const { return nullptr; }
  virtual const ORACLENumericDefineBuffer* toNumericDefineBuffer() const { return nullptr; }
  virtual const ORACLECharDefineBuffer* toCharDefineBuffer() const { return nullptr; }
  virtual const ORACLEStringDefineBuffer* toStringDefineBuffer() const { return nullptr; }
  virtual const ORACLETimestampDefineBuffer* toTimestampDefineBuffer() const { return nullptr; }
  virtual const ORACLEDateDefineBuffer* toDateDefineBuffer() const { return nullptr; }

  void clearBuffers();

  static ORACLEDefineBuffer* createDefineBuffer(
      uint16_t dataType, uint32_t position, int32_t size, uint32_t count, uint16_t sizeInChars);

protected:
  ORACLEDefineBuffer(
      uint16_t dataType, uint32_t position, int32_t size, uint32_t count, uint16_t sizeInChars);
  ORACLEDefineBuffer(uint16_t dataType, uint32_t position, int32_t size, uint32_t count);

  virtual void clearInternalBuffers() = 0;

  int16_t* getNullIndicatorBufferPointer();

  uint16_t* getReturnedDataLengthBufferPointer();
  bool getReturnedDataLength(uint32_t index, uint16_t& out) const;

private:
  ORACLEDefineBuffer();

  void clearNullIndicatorBuffers();
  void clearDataLengthBuffers();

  uint16_t _dataType;
  uint32_t _position;
  int32_t _size;
  uint32_t _count;
  uint16_t _sizeInChars; // Column width expressed as number of characters

  int16_t* _nullIndicator;
  uint16_t* _returnedDataLength;

  static const int16_t NULL_INDICATOR;
  static const int16_t NOT_NULL_INDICATOR;

}; // class ORACLEDefineBuffer

class ORACLEStringDefineBuffer : public virtual ORACLEDefineBuffer
{
public:
  ORACLEStringDefineBuffer(uint32_t position, int32_t size, uint32_t count, uint16_t sizeInChars);
  virtual ~ORACLEStringDefineBuffer();

  virtual bool isNull(uint32_t index) const override;

  virtual int32_t setOracleDefine(ORACLEAdapter& adapter) override;

  virtual const ORACLEStringDefineBuffer* toStringDefineBuffer() const override { return this; }

  bool getValue(uint32_t index, char*& out) const;

protected:
  virtual void clearInternalBuffers() override;

private:
  char* _data;

}; // class ORACLEStringDefineBuffer

class ORACLECharDefineBuffer : public virtual ORACLEDefineBuffer
{
public:
  ORACLECharDefineBuffer(uint32_t position, int32_t size, uint32_t count, uint16_t sizeInChars);
  virtual ~ORACLECharDefineBuffer();

  virtual bool isNull(uint32_t index) const override;

  virtual int32_t setOracleDefine(ORACLEAdapter& adapter) override;
  virtual const ORACLECharDefineBuffer* toCharDefineBuffer() const override { return this; }

  bool getValue(uint32_t index, char*& out) const;

protected:
  virtual void clearInternalBuffers() override;

private:
  char* _data;

}; // class ORACLECharDefineBuffer

class ORACLEIntDefineBuffer : public virtual ORACLEDefineBuffer
{
public:
  ORACLEIntDefineBuffer(uint32_t position, uint32_t count);
  virtual ~ORACLEIntDefineBuffer();

  virtual int32_t setOracleDefine(ORACLEAdapter& adapter) override;
  virtual const ORACLEIntDefineBuffer* toIntDefineBuffer() const override { return this; }

  bool getValue(uint32_t index, int32_t& out) const;

protected:
  virtual void clearInternalBuffers() override;

private:
  int32_t* _data;

}; // class ORACLEIntDefineBuffer

class ORACLELongDefineBuffer : public virtual ORACLEDefineBuffer
{
public:
  ORACLELongDefineBuffer(uint32_t position, uint32_t count);
  virtual ~ORACLELongDefineBuffer();

  virtual int32_t setOracleDefine(ORACLEAdapter& adapter) override;
  virtual const ORACLELongDefineBuffer* toLongDefineBuffer() const override { return this; }

  bool getValue(uint32_t index, int64_t& out) const;

protected:
  virtual void clearInternalBuffers() override;

private:
  int64_t* _data;

}; // class ORACLELongDefineBuffer

class ORACLENumericDefineBuffer : public virtual ORACLEDefineBuffer
{
public:
  ORACLENumericDefineBuffer(uint32_t position, uint32_t count);
  virtual ~ORACLENumericDefineBuffer();

  virtual int32_t setOracleDefine(ORACLEAdapter& adapter) override;
  virtual const ORACLENumericDefineBuffer* toNumericDefineBuffer() const override { return this; }

  bool getIntValue(uint32_t index, int& out) const;
  bool getLongValue(uint32_t index, long& out) const;
  bool getLongLongValue(uint32_t index, long long& out) const;

protected:
  virtual void clearInternalBuffers() override;

private:
  unsigned char* _data;

}; // class ORACLENumericDefineBuffer

class ORACLEDateDefineBuffer : public virtual ORACLEDefineBuffer
{
public:
  ORACLEDateDefineBuffer(uint32_t position, uint32_t count);
  virtual ~ORACLEDateDefineBuffer();

  virtual int32_t setOracleDefine(ORACLEAdapter& adapter) override;
  virtual const ORACLEDateDefineBuffer* toDateDefineBuffer() const override { return this; }

  bool getValue(uint32_t index, DateTime& out) const;

protected:
  virtual void clearInternalBuffers() override;

private:
  OCIDate* _data;

}; // class ORACLEDateDefineBuffer

class ORACLETimestampDefineBuffer : public virtual ORACLEDefineBuffer
{
public:
  ORACLETimestampDefineBuffer(uint32_t position, uint32_t count);
  virtual ~ORACLETimestampDefineBuffer();

  virtual int32_t setOracleDefine(ORACLEAdapter& adapter) override;
  virtual bool freeBuffers(ORACLEAdapter& adapter) override;
  virtual const ORACLETimestampDefineBuffer* toTimestampDefineBuffer() const override
  {
    return this;
  }

  bool getValue(uint32_t index, DateTime& out, ORACLEAdapter& adapter) const;

protected:
  virtual void clearInternalBuffers() override;

private:
  OCIDateTime** _data;
  bool _descriptorAllocated;
  OCIEnv* _ociEnv;

}; // class ORACLETimestampDefineBuffer

} // namespace tse

