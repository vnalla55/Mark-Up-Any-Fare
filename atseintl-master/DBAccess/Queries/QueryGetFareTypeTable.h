//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareTypeTable.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetFareTypeTable : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareTypeTable(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareTypeTable(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareTypeTable() {};
  virtual const char* getQueryName() const override;

  void findFareTypeTable(std::vector<FareTypeTable*>& FareTypeTableVec,
                         const VendorCode& vendor,
                         int itemNo);

  static void initialize();

  const QueryGetFareTypeTable& operator=(const QueryGetFareTypeTable& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareTypeTable& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareTypeTable : public QueryGetFareTypeTable
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareTypeTable(DBAdapter* dbAdapt) : QueryGetFareTypeTable(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAllFareTypeTable() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareTypeTable*>& fareTypeTableVec)
  {
    findAllFareTypeTable(fareTypeTableVec);
  }

  void findAllFareTypeTable(std::vector<FareTypeTable*>& fareTypeTableVec);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetFareTypeTableHistorical : public QueryGetFareTypeTable
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareTypeTableHistorical(DBAdapter* dbAdapt) : QueryGetFareTypeTable(dbAdapt, _baseSQL) {}
  virtual ~QueryGetFareTypeTableHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareTypeTable(std::vector<FareTypeTable*>& FareTypeTableVec,
                         const VendorCode& vendor,
                         int itemNo);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
}
