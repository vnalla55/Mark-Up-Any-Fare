//----------------------------------------------------------------------------
//     2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TaxCarrierAppl.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTaxCarrierAppl : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxCarrierAppl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetTaxCarrierAppl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetTaxCarrierAppl() {};

  virtual const char* getQueryName() const override;

  void findTaxCarrierAppl(std::vector<tse::TaxCarrierAppl*>& lstCF, VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetTaxCarrierAppl& operator=(const QueryGetTaxCarrierAppl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetTaxCarrierAppl& operator=(const std::string& Another)
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
}; // class QueryGetTaxCarrierAppl

class QueryGetTaxCarrierApplHistorical : public QueryGetTaxCarrierAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxCarrierApplHistorical(DBAdapter* dbAdapt) : QueryGetTaxCarrierAppl(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetTaxCarrierApplHistorical() {}

  virtual const char* getQueryName() const override;

  void findTaxCarrierAppl(std::vector<tse::TaxCarrierAppl*>& lstCF,
                          VendorCode& vendor,
                          int itemNo,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxCarrierApplHistorical
} // namespace tse

