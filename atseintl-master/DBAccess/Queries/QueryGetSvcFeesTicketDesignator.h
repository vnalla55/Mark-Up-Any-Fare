//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesTicketDesignator.h
//          Description:    QueryGetSvcFeesTicketDesignator.h
//          Created:        3/17/2009
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
#include "DBAccess/SvcFeesTktDesignatorInfo.h"

namespace tse
{

class QueryGetSvcFeesTicketDesignator : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesTicketDesignator(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSvcFeesTicketDesignator(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSvcFeesTicketDesignator() {};
  virtual const char* getQueryName() const override;

  void findSvcFeesTktDesignatorInfo(std::vector<tse::SvcFeesTktDesignatorInfo*>& tktD,
                                    const VendorCode& vendor,
                                    int itemNo);

  static void initialize();

  const QueryGetSvcFeesTicketDesignator& operator=(const QueryGetSvcFeesTicketDesignator& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSvcFeesTicketDesignator& operator=(const std::string& Another)
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
}; // class QueryGetSvcFeesTicketDesignator

class QueryGetSvcFeesTicketDesignatorHistorical : public QueryGetSvcFeesTicketDesignator
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesTicketDesignatorHistorical(DBAdapter* dbAdapt)
    : QueryGetSvcFeesTicketDesignator(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSvcFeesTicketDesignatorHistorical() {}
  virtual const char* getQueryName() const override;

  void findSvcFeesTktDesignatorInfo(std::vector<tse::SvcFeesTktDesignatorInfo*>& tktD,
                                    const VendorCode& vendor,
                                    int itemNo,
                                    const DateTime& startDate,
                                    const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSvcFeesTicketDesignatorHistorical
} // namespace tse

