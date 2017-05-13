//----------------------------------------------------------------------------
//
//     File:           SQLQueryHelper.cpp
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

#include "DBAccess/SQLQueryHelper.h"

#include "DBAccess/DataManager.h"

namespace DBAccess
{
SQLQueryHelper::~SQLQueryHelper() {}

const SQLQueryHelper&
SQLQueryHelper::getSQLQueryHelper()
{
  return tse::DataManager::getSQLQueryHelper();
}
}
