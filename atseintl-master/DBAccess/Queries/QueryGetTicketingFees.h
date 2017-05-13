//----------------------------------------------------------------------------
//          File:           QueryGetTicketingFees.h
//          Description:    QueryGetTicketingFees.h
//          Created:        2/26/2009
// Authors:
//
//          Updates:
//
//      2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TicketingFeesInfo.h"

namespace tse
{

class QueryGetTicketingFees : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTicketingFees(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetTicketingFees(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetTicketingFees() {};
  virtual const char* getQueryName() const override;

  void findTicketingFeesInfo(std::vector<tse::TicketingFeesInfo*>& tktFees,
                             const VendorCode& vendor,
                             const CarrierCode& carrier);

  static void initialize();

  const QueryGetTicketingFees& operator=(const QueryGetTicketingFees& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetTicketingFees& operator=(const std::string& Another)
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
}; // class QueryGetTicketingFees

class QueryGetTicketingFeesHistorical : public QueryGetTicketingFees
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTicketingFeesHistorical(DBAdapter* dbAdapt) : QueryGetTicketingFees(dbAdapt, _baseSQL) {};
  virtual ~QueryGetTicketingFeesHistorical() {}
  virtual const char* getQueryName() const override;

  void findTicketingFeesInfo(std::vector<tse::TicketingFeesInfo*>& tktFees,
                             const VendorCode& vendor,
                             const CarrierCode& carrier,
                             const DateTime& startDate,
                             const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTicketingFeesHistorical
} // namespace tse

