//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/MarkupControl.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetMarkup : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMarkup(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMarkup(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMarkup() {};
  virtual const char* getQueryName() const override;

  void findMarkupControl(std::vector<tse::MarkupControl*>& lstMC,
                         VendorCode& vendor,
                         CarrierCode& carrier,
                         TariffNumber ruleTariff,
                         RuleNumber& rule,
                         int seqNo,
                         PseudoCityCode& pcc);

  static void initialize();

  const QueryGetMarkup& operator=(const QueryGetMarkup& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMarkup& operator=(const std::string& Another)
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
}; // class QueryGetMarkup

class QueryGetMarkupHistorical : public QueryGetMarkup
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMarkupHistorical(DBAdapter* dbAdapt) : QueryGetMarkup(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMarkupHistorical() {};
  virtual const char* getQueryName() const override;

  void findMarkupControl(std::vector<tse::MarkupControl*>& lstMC,
                         VendorCode& vendor,
                         CarrierCode& carrier,
                         TariffNumber ruleTariff,
                         RuleNumber& rule,
                         int seqNo,
                         PseudoCityCode& pcc,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

  void setAdapterAndBaseSQL(DBAdapter* dbAdapt);
}; // class QueryGetMarkupHistorical
} // namespace tse
