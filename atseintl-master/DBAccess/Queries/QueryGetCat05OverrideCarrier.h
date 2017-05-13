//----------------------------------------------------------------------------
//          File:           QueryGetCat05OverrideCarrier.h
//          Description:    QueryGetCat05OverrideCarrier
//          Created:        04/22/2013
//          Author:         Sashi Reddy
//     Copyright 2013, Sabre Inc. All rights reserved. This software/documentation is
//     confidential and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/Cat05OverrideCarrier.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetCat05OverrideCarrier : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCat05OverrideCarrier(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCat05OverrideCarrier(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCat05OverrideCarrier() {};
  virtual const char* getQueryName() const override;

  void
  findCat05OverrideCarriers(std::vector<Cat05OverrideCarrier*>& lst, const PseudoCityCode& pcc);

  static void initialize();

  const QueryGetCat05OverrideCarrier& operator=(const QueryGetCat05OverrideCarrier& rhs)
  {
    if (this != &rhs)
    {
      *((SQLQuery*)this) = (SQLQuery&)rhs;
    }
    return *this;
  };
  const QueryGetCat05OverrideCarrier& operator=(const std::string& rhs)
  {
    if (this != &rhs)
    {
      *((SQLQuery*)this) = rhs;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse
