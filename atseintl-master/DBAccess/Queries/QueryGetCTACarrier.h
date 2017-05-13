//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/CTACarrier.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCTACarrier : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCTACarrier(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCTACarrier(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCTACarrier() {};

  virtual const char* getQueryName() const override;

  void findCarrier(std::vector<tse::CTACarrier*>& lstCF, const CarrierCode& vendor);

  static void initialize();

  const QueryGetCTACarrier& operator=(const QueryGetCTACarrier& another)
  {
    if (this != &another)
      *((SQLQuery*)this) = (SQLQuery&)another;

    return *this;
  };

  const QueryGetCTACarrier& operator=(const std::string& another)
  {
    if (this != &another)
      *((SQLQuery*)this) = another;

    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCTACarrier

class QueryGetCTACarrierHistorical : public QueryGetCTACarrier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCTACarrierHistorical(DBAdapter* dbAdapt) : QueryGetCTACarrier(dbAdapt, _baseSQL) {}
  virtual ~QueryGetCTACarrierHistorical() {}

  virtual const char* getQueryName() const override;

  void findCarrier(std::vector<tse::CTACarrier*>& lstCF, const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCTACarrierHistorical

} // namespace tse

