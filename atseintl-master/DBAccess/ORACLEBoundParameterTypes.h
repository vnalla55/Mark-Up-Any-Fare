//----------------------------------------------------------------------------
//
//     File:           ORACLEBoundParameterTypes.h
//     Description:    Bound parameter types for Oracle OCI implementation.
//
//     Created:        11/17/2009
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
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "DBAccess/BoundParameter.h"

#include <string>

#include <oci.h>

namespace DBAccess
{

class ORACLEBoundInteger : public BoundParameter
{
public:
  explicit ORACLEBoundInteger(int32_t index, size_t position, int32_t value)
    : BoundParameter(index, position), _value(value)
  {
  }

  ORACLEBoundInteger(const ORACLEBoundInteger&) = delete;
  ORACLEBoundInteger& operator=(const ORACLEBoundInteger&) = delete;

  virtual void bind(const ParameterBinder& binder) override;

  int32_t* getBindBuffer() { return &_value; }
  int32_t getBindBufferSize() { return sizeof(_value); }

private:
  int32_t _value = 0;
};

class ORACLEBoundLong : public BoundParameter
{
public:
  explicit ORACLEBoundLong(int32_t index, size_t position, int64_t value)
    : BoundParameter(index, position), _value(value)
  {
  }

  ORACLEBoundLong(const ORACLEBoundLong&) = delete;
  ORACLEBoundLong& operator=(const ORACLEBoundLong&) = delete;

  virtual void bind(const ParameterBinder& binder) override;

  int64_t* getBindBuffer() { return &_value; }
  int32_t getBindBufferSize() { return sizeof(_value); }

private:
  int64_t _value = 0;
};

class ORACLEBoundString : public BoundParameter
{
public:
  explicit ORACLEBoundString(int32_t index, size_t position, const std::string& value);

  explicit ORACLEBoundString(int32_t index, size_t position, const char* value);

  ORACLEBoundString(const ORACLEBoundString&) = delete;
  ORACLEBoundString& operator=(const ORACLEBoundString&) = delete;

  virtual ~ORACLEBoundString() { delete[] _value; }

  virtual void bind(const ParameterBinder& binder) override;

  char* getBindBuffer() { return _value; }
  int32_t getBindBufferSize() { return _size; }

private:
  void trim(const char* value);

  char* _value = nullptr;
  uint16_t _size = 0;
};

class ORACLEBoundDate : public BoundParameter
{
public:
  explicit ORACLEBoundDate(int32_t index, size_t position, const tse::DateTime& value);
  ORACLEBoundDate(const ORACLEBoundDate&) = delete;
  ORACLEBoundDate& operator=(const ORACLEBoundDate&) = delete;

  virtual void bind(const ParameterBinder& binder) override;

  char* getBindBuffer() { return _value; }
  int32_t getBindBufferSize() { return sizeof(_value); }

private:
  char _value[7];
};

class ORACLEBoundDateTime : public BoundParameter
{
public:
  explicit ORACLEBoundDateTime(int32_t index, size_t position, const tse::DateTime& value)
    : BoundParameter(index, position), _value(value)
  {
  }

  ORACLEBoundDateTime(const ORACLEBoundDateTime&) = delete;
  ORACLEBoundDateTime& operator=(const ORACLEBoundDateTime&) = delete;

  virtual void bind(const ParameterBinder& binder) override;

  tse::DateTime& getValue() { return _value; }
  OCIDateTime*& getBindBuffer() { return _bindBuffer; }
  int32_t getBindBufferSize() { return sizeof(_bindBuffer); }

private:
  tse::DateTime _value;
  OCIDateTime* _bindBuffer = nullptr;
};
} // namespace DBAccess
