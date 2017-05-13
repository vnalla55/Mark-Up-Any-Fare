//----------------------------------------------------------------------------
//
//     File:           ParameterBinder.h
//     Description:    ParameterBinder
//     Created:        06/26/2009
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

#include <string>

#include <cstdint>

namespace tse
{
class DateTime;
}

namespace DBAccess
{
class BoundParameter;

class ParameterBinder
{
public:
  ParameterBinder() = default;

  ParameterBinder(const ParameterBinder&) = delete;
  ParameterBinder& operator=(const ParameterBinder&) = delete;

  virtual inline ~ParameterBinder() = 0;

  virtual void bindAllParameters() const = 0;

  virtual void bindParameter(const char* parm, int32_t index) const = 0;

  virtual void bindParameter(const std::string& parm, int32_t index) const = 0;

  virtual void bindParameter(int32_t parm, int32_t index) const = 0;

  virtual void bindParameter(int64_t parm, int32_t index) const = 0;

  virtual void bindParameter(float parm, int32_t index) const = 0;

  virtual void
  bindParameter(const tse::DateTime& parm, int32_t index, bool dateOnly = false) const = 0;

  virtual void bindParameter(BoundParameter* parm) const = 0;
}; // class ParameterBinder

ParameterBinder::~ParameterBinder() = default;
} // namespace DBAccess
