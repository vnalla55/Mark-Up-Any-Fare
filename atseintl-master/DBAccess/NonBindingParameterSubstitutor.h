//----------------------------------------------------------------------------
//
//     File:           NonBindingParameterSubstitutor.h
//     Description:    NonBindingParameterSubstitutor
//     Created:        07/02/2009
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright 2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/ParameterSubstitutorImpl.h"

#include <string>
#include <vector>

// Forward declarations
//
namespace tse
{
class DateTime;
}

namespace DBAccess
{

class NonBindingParameterSubstitutor : public ParameterSubstitutorImpl
{
public:
  NonBindingParameterSubstitutor();
  ~NonBindingParameterSubstitutor();

  virtual void substituteParameter(ParameterSubstitutor& substitutor,
                                   std::string& sql,
                                   const char* parm,
                                   int32_t index,
                                   bool forceLiteral = false) const override;

  virtual void substituteParameter(ParameterSubstitutor& substitutor,
                                   std::string& sql,
                                   const std::string& parm,
                                   int32_t index,
                                   bool forceLiteral = false) const override;

  virtual void substituteParameter(ParameterSubstitutor& substitutor,
                                   std::string& sql,
                                   int parm,
                                   int32_t index) const override;

  virtual void substituteParameter(ParameterSubstitutor& substitutor,
                                   std::string& sql,
                                   int64_t parm,
                                   int32_t index) const override;

  virtual void substituteParameter(ParameterSubstitutor& substitutor,
                                   std::string& sql,
                                   const tse::DateTime& parm,
                                   int32_t index,
                                   bool dateOnly = false) const override;

  virtual void substituteParameter(ParameterSubstitutor& substitutor,
                                   std::string& sql,
                                   const std::vector<tse::CarrierCode>& parm,
                                   int32_t index) const override;
  virtual void substituteParameter(ParameterSubstitutor& substitutor,
                                   std::string& sql,
                                   const std::vector<int64_t>& parm,
                                   int32_t index) const override;

  virtual void substituteParameter(ParameterSubstitutor& substitutor,
                                   std::string& sql,
                                   const tse::DateTime& parm,
                                   const char* placeHolder,
                                   bool dateOnly = false) const override;

private:
  NonBindingParameterSubstitutor(const NonBindingParameterSubstitutor& rhs);
  NonBindingParameterSubstitutor& operator=(const NonBindingParameterSubstitutor& rhs);

  log4cxx::LoggerPtr& logger() const { return _logger; }

  static log4cxx::LoggerPtr _logger;

}; // class NonBindingParameterSubstitutor

} // namespace DBAccess

