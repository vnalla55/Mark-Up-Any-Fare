//----------------------------------------------------------------------------
//
//     File:           ORACLEQueryHelper.cpp
//     Description:    ORACLEQueryHelper
//     Created:        07/01/2009
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

#include "DBAccess/ORACLEQueryHelper.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "DBAccess/BindingParameterSubstitutor.h"
#include "DBAccess/NonBindingParameterSubstitutor.h"
#include "DBAccess/ORACLEBindingParameterSubstitutor.h"

namespace DBAccess
{
ORACLEQueryHelper::ORACLEQueryHelper()
{
  tse::ConfigMan& config = tse::Global::config();
  std::string disableBindVars;
  config.getValue("DISABLE_BIND_VARS", disableBindVars, "DATABASE_TUNING");

  if (disableBindVars == "Y")
    _parmSubstitutor = new NonBindingParameterSubstitutor;
  else
    _parmSubstitutor = new ORACLEBindingParameterSubstitutor;
  //_parmSubstitutor = new BindingParameterSubstitutor;
}

ORACLEQueryHelper::~ORACLEQueryHelper() { delete _parmSubstitutor; }

const ParameterSubstitutorImpl&
ORACLEQueryHelper::getParameterSubstitutorImpl() const
{
  return *_parmSubstitutor;
}
}
