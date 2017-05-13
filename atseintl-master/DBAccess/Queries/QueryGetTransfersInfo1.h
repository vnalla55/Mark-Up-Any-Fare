//----------------------------------------------------------------------------
//          File:           QueryGetTransfersInfo1.h
//          Description:    QueryGetTransfersInfo1
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TransfersInfo1.h"

namespace tse
{

class QueryGetTransfersInfo1 : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTransfersInfo1(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTransfersInfo1(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTransfersInfo1() {}
  virtual const char* getQueryName() const override;

  void findTransfersInfo1(std::vector<tse::TransfersInfo1*>& lstTI, VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetTransfersInfo1& operator=(const QueryGetTransfersInfo1& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTransfersInfo1& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTransfersInfo1

class QueryGetTransfersInfo1Historical : public QueryGetTransfersInfo1
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTransfersInfo1Historical(DBAdapter* dbAdapt) : QueryGetTransfersInfo1(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetTransfersInfo1Historical() {}
  virtual const char* getQueryName() const override;

  void findTransfersInfo1(std::vector<tse::TransfersInfo1*>& lstTI,
                          VendorCode& vendor,
                          int itemNo,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTransfersInfo1Historical
} // namespace tse

