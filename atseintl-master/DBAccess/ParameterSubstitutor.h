//----------------------------------------------------------------------------
//
//     File:           ParameterSubstitutor.h
//     Description:    ParameterSubstitutor
//     Created:        06/26/2009
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

#include "Common/Thread/TSEFastMutex.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/BoundParameter.h"

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

// Forward declarations
//
class ParameterBinder;
class ParameterSubstitutorImpl;

class ParameterSubstitutor
{
public:
  ParameterSubstitutor() = default;
  ~ParameterSubstitutor();

  void
  substituteParameter(std::string& sql, const char* parm, int32_t index, bool forceLiteral = false);

  void substituteParameter(std::string& sql,
                           const std::string& parm,
                           int32_t index,
                           bool forceLiteral = false);

  void substituteParameter(std::string& sql, int parm, int32_t index);

  void substituteParameter(std::string& sql, int64_t parm, int32_t index);

  void substituteParameter(std::string& sql,
                           const tse::DateTime& parm,
                           int32_t index,
                           bool dateOnly = false);

  void
  substituteParameter(std::string& sql, const std::vector<tse::CarrierCode>& parm, int32_t index);

  void substituteParameter(std::string& sql, const std::vector<int64_t>& parm, int32_t index);

  /*
  void substituteParameter(std::string& sql,
                           const std::vector<PaxTypeCode>& parm,
                           int32_t index);
  */

  void substituteParameter(std::string& sql,
                           const tse::DateTime& parm,
                           const char* placeHolder, // Use "%cd" for current date
                           bool dateOnly = false);

  void bindAllParameters(const ParameterBinder& binder) const;

  const BoundParameterCollection& getBoundParameters() const;
  void clearBoundParameters();

  void fillParameterString(std::string& displayString) const;
  void getSQLString(const std::string& orig, std::string& resultString) const;

  template <typename T, typename ...Args>
  void addBoundParameter(Args&& ...args)
  {
    _boundParameters.insert(new T(std::forward<Args>(args)...));
  }

private:
  ParameterSubstitutor(const ParameterSubstitutor&) = delete;
  ParameterSubstitutor& operator=(const ParameterSubstitutor&) = delete;

  const ParameterSubstitutorImpl& impl();

  const ParameterSubstitutorImpl* _impl = nullptr;

  BoundParameterCollection _boundParameters;

  static TSEFastMutex _mutex;

}; // class ParameterSubstitutor

} // namespace DBAccess

