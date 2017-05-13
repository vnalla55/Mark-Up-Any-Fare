//----------------------------------------------------------------------------
//
//     File:           BoundParameterTypes.h
//     Description:
//     Created:        07/02/2009
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

namespace DBAccess
{

class BoundInteger : public BoundParameter
{
public:
  explicit BoundInteger(int32_t index, size_t position, int32_t value)
    : BoundParameter(index, position), _value(value)
  {
  }

  virtual ~BoundInteger();

  virtual void bind(const ParameterBinder& binder) override;

private:
  BoundInteger();
  BoundInteger(const BoundInteger& rhs);
  BoundInteger& operator=(const BoundInteger& rhs);

  int32_t _value;
};

class BoundLong : public BoundParameter
{
public:
  explicit BoundLong(int32_t index, size_t position, int64_t value)
    : BoundParameter(index, position), _value(value)
  {
  }

  virtual ~BoundLong();

  virtual void bind(const ParameterBinder& binder) override;

private:
  BoundLong();
  BoundLong(const BoundLong& rhs);
  BoundLong& operator=(const BoundLong& rhs);

  int64_t _value;
};

class BoundFloat : public BoundParameter
{
public:
  explicit BoundFloat(int32_t index, size_t position, float value)
    : BoundParameter(index, position), _value(value)
  {
  }

  virtual ~BoundFloat();

  virtual void bind(const ParameterBinder& binder) override;

private:
  BoundFloat();
  BoundFloat(const BoundFloat& rhs);
  BoundFloat& operator=(const BoundFloat& rhs);

  float _value;
};

class BoundString : public BoundParameter
{
public:
  explicit BoundString(int32_t index, size_t position, const std::string& value)
    : BoundParameter(index, position), _value(value)
  {
    if (_value.empty())
      _value = " ";
  }

  explicit BoundString(int32_t index, size_t position, const char* value)
    : BoundParameter(index, position), _value(value)
  {
    if (_value.empty())
      _value = " ";
  }

  virtual ~BoundString();

  virtual void bind(const ParameterBinder& binder) override;

private:
  BoundString();
  BoundString(const BoundString& rhs);
  BoundString& operator=(const BoundString& rhs);

  std::string _value;
};

class BoundDate : public BoundParameter
{
public:
  explicit BoundDate(int32_t index, size_t position, const tse::DateTime& value)
    : BoundParameter(index, position), _value(value)
  {
  }

  virtual ~BoundDate();

  virtual void bind(const ParameterBinder& binder) override;

private:
  BoundDate();
  BoundDate(const BoundDate& rhs);
  BoundDate& operator=(const BoundDate& rhs);

  tse::DateTime _value;
};

class BoundDateTime : public BoundParameter
{
public:
  explicit BoundDateTime(int32_t index, size_t position, const tse::DateTime& value)
    : BoundParameter(index, position), _value(value)
  {
  }

  virtual ~BoundDateTime();

  virtual void bind(const ParameterBinder& binder) override;

private:
  BoundDateTime();
  BoundDateTime(const BoundDateTime& rhs);
  BoundDateTime& operator=(const BoundDateTime& rhs);

  tse::DateTime _value;
};

} // namespace DBAccess

