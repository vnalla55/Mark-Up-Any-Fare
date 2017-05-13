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
class OptionalServicesConcur;

class QueryGetOptionalServicesConcur : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOptionalServicesConcur(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetOptionalServicesConcur(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetOptionalServicesConcur() {}

  virtual const char* getQueryName() const override;

  void findOptionalServicesConcur(std::vector<OptionalServicesConcur*>& concurs,
                                  const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const ServiceTypeCode& serviceTypeCode);

  static void initialize();

  const QueryGetOptionalServicesConcur& operator=(const QueryGetOptionalServicesConcur& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetOptionalServicesConcur& operator=(const std::string& Another)
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

class QueryGetOptionalServicesConcurHistorical : public QueryGetOptionalServicesConcur
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOptionalServicesConcurHistorical(DBAdapter* dbAdapt)
    : QueryGetOptionalServicesConcur(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetOptionalServicesConcurHistorical() {}
  virtual const char* getQueryName() const override;

  void findOptionalServicesConcur(std::vector<OptionalServicesConcur*>& concurs,
                                  const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const ServiceTypeCode& serviceTypeCode,
                                  const DateTime& startDate,
                                  const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

