//----------------------------------------------------------------------------
//          File:           QueryGetMarketCarriers.h
//          Description:    QueryGetMarketCarriers
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MarketCarrier.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetMarketCarriers : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMarketCarriers(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMarketCarriers(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMarketCarriers() {};
  virtual const char* getQueryName() const override;
  void resetSQL() { *this = _baseSQL; }

  void findMarketCarriers(std::vector<const tse::MarketCarrier*>& lstMC,
                          const LocCode& market1,
                          const LocCode& market2);

  static void initialize();

  const QueryGetMarketCarriers& operator=(const QueryGetMarketCarriers& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMarketCarriers& operator=(const std::string& Another)
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

protected:
}; // class QueryGetMarketCarriers

} // namespace tse
