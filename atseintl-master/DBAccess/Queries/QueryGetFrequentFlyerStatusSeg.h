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
class FreqFlyerStatusSeg;
class Logger;

class QueryGetFrequentFlyerStatusSeg : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFrequentFlyerStatusSeg(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFrequentFlyerStatusSeg(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};

  const char* getQueryName() const override;
  void findTierStatus(const CarrierCode carrier, std::vector<FreqFlyerStatusSeg*>& statuses);

  static void initialize();
  const QueryGetFrequentFlyerStatusSeg& operator=(const QueryGetFrequentFlyerStatusSeg& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFrequentFlyerStatusSeg& operator=(const std::string& Another)
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

class QueryGetFrequentFlyerStatusSegHistorical : public QueryGetFrequentFlyerStatusSeg
{
public:
  QueryGetFrequentFlyerStatusSegHistorical(DBAdapter* dbAdapt)
    : QueryGetFrequentFlyerStatusSegHistorical(dbAdapt, _baseSQL) {};
  QueryGetFrequentFlyerStatusSegHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetFrequentFlyerStatusSeg(dbAdapt, sqlStatement) {};

  const char* getQueryName() const override;
  void findTierStatusSegs(const CarrierCode carrier,
                          const DateTime& startDate,
                          const DateTime& endDate,
                          std::vector<FreqFlyerStatusSeg*>& statuses);
  static void initialize();
  void resetSQL() { *this = _baseSQL; };
  const QueryGetFrequentFlyerStatusSegHistorical&
  operator=(const QueryGetFrequentFlyerStatusSegHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFrequentFlyerStatusSegHistorical& operator=(const std::string& Another)
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
