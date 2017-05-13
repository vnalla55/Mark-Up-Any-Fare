//----------------------------------------------------------------------------
//
//     File:           ORACLEQueryHelper.h
//     Description:    ORACLEQueryHelper
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

#include "DBAccess/SQLQueryHelper.h"

// Forward declarations
//

namespace tse
{
class DataManager;
}

namespace DBAccess
{

// Forward declarations
//
class ParameterSubstitutorImpl;

class ORACLEQueryHelper : public SQLQueryHelper
{
public:
  virtual ~ORACLEQueryHelper();

  virtual const ParameterSubstitutorImpl& getParameterSubstitutorImpl() const override;

  friend class tse::DataManager;

protected:
  ORACLEQueryHelper();

private:
  ORACLEQueryHelper(const ORACLEQueryHelper& rhs);
  ORACLEQueryHelper& operator=(const ORACLEQueryHelper& rhs);

  const ParameterSubstitutorImpl* _parmSubstitutor;

}; // class ORACLEQueryHelper

} // namespace DBAccess

