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
class NvbNvaInfo;

class QueryGetNvbNva : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNvbNva(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetNvbNva(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetNvbNva() {}

  virtual const char* getQueryName() const override;

  void findNvbNvaInfo(std::vector<NvbNvaInfo*>& vNvbNvaInfo,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& ruleTariff,
                      const RuleNumber& rule);

  static void initialize();

  const QueryGetNvbNva& operator=(const QueryGetNvbNva& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetNvbNva& operator=(const std::string& Another)
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
}; // class QueryGetNvbNva

class QueryGetNvbNvaHistorical : public QueryGetNvbNva
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNvbNvaHistorical(DBAdapter* dbAdapt) : QueryGetNvbNva(dbAdapt, _baseSQL) {}
  virtual ~QueryGetNvbNvaHistorical() {}
  virtual const char* getQueryName() const override;

  void findNvbNvaInfo(std::vector<NvbNvaInfo*>& vNvbNvaInfo,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& ruleTariff,
                      const RuleNumber& rule,
                      const DateTime& startDate,
                      const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

}; // class QueryGetNvbNvaHistorical

class QueryGetAllNvbNva : public QueryGetNvbNva
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllNvbNva(DBAdapter* dbAdapt) : QueryGetNvbNva(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllNvbNva() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<NvbNvaInfo*>& vNvbNvaInfo) { findAllNvbNvaInfo(vNvbNvaInfo); }

  void findAllNvbNvaInfo(std::vector<NvbNvaInfo*>& vNvbNvaInfo);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllNvbNva

} // namespace tse

