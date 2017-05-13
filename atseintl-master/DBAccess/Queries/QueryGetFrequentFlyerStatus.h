//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class FreqFlyerStatus;
class Logger;

class QueryGetFrequentFlyerStatus : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFrequentFlyerStatus(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFrequentFlyerStatus(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};

  const char* getQueryName() const override;
  std::vector<FreqFlyerStatus*> findStatus(const CarrierCode carrier);

  static void initialize();
  const QueryGetFrequentFlyerStatus& operator=(const QueryGetFrequentFlyerStatus& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFrequentFlyerStatus& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static Logger _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetFrequentFlyerStatusHistorical : public QueryGetFrequentFlyerStatus
{
public:
  QueryGetFrequentFlyerStatusHistorical(DBAdapter* dbAdapt)
    : QueryGetFrequentFlyerStatusHistorical(dbAdapt, _baseSQL) {};
  QueryGetFrequentFlyerStatusHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetFrequentFlyerStatus(dbAdapt, sqlStatement) {};

  static void deinitialize() { _isInitialized = false; }
  const char* getQueryName() const override;
  std::vector<FreqFlyerStatus*>
  findStatus(const CarrierCode carrier, const DateTime& startDate, const DateTime& endDate);

  static void initialize();
  const QueryGetFrequentFlyerStatusHistorical&
  operator=(const QueryGetFrequentFlyerStatusHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFrequentFlyerStatusHistorical& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static Logger _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse namespace
