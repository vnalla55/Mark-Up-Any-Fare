//----------------------------------------------------------------------------
//
//     File:           SQLQueryHelper.h
//     Description:    SQLQueryHelper
//     Created:        06/25/2009
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

namespace DBAccess
{

// Forward declarations
//
class ParameterSubstitutorImpl;

class SQLQueryHelper
{
public:
  SQLQueryHelper() {}
  virtual ~SQLQueryHelper() = 0;

  virtual const ParameterSubstitutorImpl& getParameterSubstitutorImpl() const = 0;

  static const SQLQueryHelper& getSQLQueryHelper();

private:
  SQLQueryHelper(const SQLQueryHelper& rhs);
  SQLQueryHelper& operator=(const SQLQueryHelper& rhs);

}; // class SQLQueryHelper

} // namespace DBAccess

