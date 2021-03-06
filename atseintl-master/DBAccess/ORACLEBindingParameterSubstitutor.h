//----------------------------------------------------------------------------
//
//     File:           ORACLEBindingParameterSubstitutor.h
//     Description:    Binding Parameter Substitutor
//                      for Oracle OCI implementation.
//
//     Created:        11/17/2009
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

class ORACLEBindingParameterSubstitutor : public ParameterSubstitutorImpl
{
public:
  ORACLEBindingParameterSubstitutor();
  ~ORACLEBindingParameterSubstitutor();

  virtual void substituteParameter(ParameterSubstitutor& substitutor,
                                   std::string& sql,
                                   const char* parm,
                                   int32_t index,
                                   bool forceLiteral) const override;

  virtual void substituteParameter(ParameterSubstitutor& substitutor,
                                   std::string& sql,
                                   const std::string& parm,
                                   int32_t index,
                                   bool forceLiteral) const override;

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
  ORACLEBindingParameterSubstitutor(const ORACLEBindingParameterSubstitutor& rhs);
  ORACLEBindingParameterSubstitutor& operator=(const ORACLEBindingParameterSubstitutor& rhs);

  log4cxx::LoggerPtr& logger() const { return _logger; }

  static log4cxx::LoggerPtr _logger;

}; // class ORACLEBindingParameterSubstitutor

} // namespace DBAccess

