//----------------------------------------------------------------------------
//              File:           QueryGetAddonMarketCarriers.h
//              Description:    QueryGetAddonMarketCarriers
//              Created:        3/2/2006
//     Authors:         Mike Lillis
//
//              Updates:
//
//         � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//         and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//         or transfer of this software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/MarketCarrier.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetAddonMarketCarriers : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAddonMarketCarriers(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAddonMarketCarriers(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAddonMarketCarriers() {};
  virtual const char* getQueryName() const override;

  void findAddOnMarketCarriers(std::vector<const tse::MarketCarrier*>& lstMC,
                               const LocCode& market1,
                               const LocCode& market2);

  static void initialize();

  const QueryGetAddonMarketCarriers& operator=(const QueryGetAddonMarketCarriers& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAddonMarketCarriers& operator=(const std::string& Another)
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
}; // class QueryGetAddonMarketCarriers

class QueryGetAddonMarketCarriersHistorical : public QueryGetAddonMarketCarriers
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAddonMarketCarriersHistorical(DBAdapter* dbAdapt)
    : QueryGetAddonMarketCarriers(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAddonMarketCarriersHistorical() {};
  virtual const char* getQueryName() const override;

  void findAddOnMarketCarriers(std::vector<const tse::MarketCarrier*>& lstMC,
                               const LocCode& market1,
                               const LocCode& market2,
                               const DateTime& startDate,
                               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAddonMarketCarriersHistorical
} // namespace tse
