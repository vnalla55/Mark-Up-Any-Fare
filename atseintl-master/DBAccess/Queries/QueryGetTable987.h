//----------------------------------------------------------------------------
//          File:           QueryGetTable987.h
//          Description:    QueryGetTable987
//          Created:        4/24/2007
// Authors:         Grzegorz Cholewiak
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/Waiver.h"

namespace tse
{

class QueryGetTable987 : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTable987(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetTable987(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetTable987() {};
  virtual const char* getQueryName() const override;

  void findWaiver(std::vector<tse::Waiver*>& table987s, const VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetTable987& operator=(const QueryGetTable987& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetTable987& operator=(const std::string& Another)
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
}; // class QueryGetTable987

class QueryGetTable987Historical : public QueryGetTable987
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTable987Historical(DBAdapter* dbAdapt) : QueryGetTable987(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTable987Historical() {}
  virtual const char* getQueryName() const override;

  void findWaiver(std::vector<tse::Waiver*>& table987s, const VendorCode& vendor, int itemNo);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTable987Historical

class QueryGetAllTable987Historical : public QueryGetTable987
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTable987Historical(DBAdapter* dbAdapt) : QueryGetTable987(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAllTable987Historical() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::Waiver*>& table987s) { findAllWaivers(table987s); }

  void findAllWaivers(std::vector<tse::Waiver*>& table987s);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTable987Historical
} // namespace tse

