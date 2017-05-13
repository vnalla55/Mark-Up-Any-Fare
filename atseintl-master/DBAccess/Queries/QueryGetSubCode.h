//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/SQLQuery.h"

namespace tse
{
class SubCodeInfo;

class QueryGetSubCode : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSubCode(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetSubCode(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetSubCode() {}

  virtual const char* getQueryName() const override;

  void findSubCodeInfo(std::vector<SubCodeInfo*>& subCodes,
                       const VendorCode& vendor,
                       const CarrierCode& carrier);

  static void initialize();

  const QueryGetSubCode& operator=(const QueryGetSubCode& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetSubCode& operator=(const std::string& Another)
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
};

class QueryGetSubCodeHistorical : public QueryGetSubCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSubCodeHistorical(DBAdapter* dbAdapt) : QueryGetSubCode(dbAdapt, _baseSQL) {}
  virtual ~QueryGetSubCodeHistorical() {}
  virtual const char* getQueryName() const override;

  void findSubCodeInfo(std::vector<SubCodeInfo*>& subCodes,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

