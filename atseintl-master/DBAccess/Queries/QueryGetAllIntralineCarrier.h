//----------------------------------------------------------------------------
//     (c)2015, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#ifndef QUERYGETALLINTERLINECARRIER_H
#define QUERYGETALLINTERLINECARRIER_H

#include "Common/Logger.h"
#include "DBAccess/IntralineCarrierInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetAllIntralineCarrier : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllIntralineCarrier(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAllIntralineCarrier(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAllIntralineCarrier() {};

  virtual const char* getQueryName() const override;

  void
  findAllIntralineCarrier(std::vector<tse::IntralineCarrierInfo*>& intralineCarriers);

  void
  execute(std::vector<tse::IntralineCarrierInfo*>& intralineCarriers)
  {
    findAllIntralineCarrier(intralineCarriers);
  }

  static void initialize();

  const QueryGetAllIntralineCarrier& operator=(const QueryGetAllIntralineCarrier& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAllIntralineCarrier& operator=(const std::string& Another)
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
}; // class QueryGetAllIntralineCarrier

class QueryGetAllIntralineCarrierHistorical : public tse::QueryGetAllIntralineCarrier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllIntralineCarrierHistorical(DBAdapter* dbAdapt)
    : QueryGetAllIntralineCarrier(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllIntralineCarrierHistorical() {};

  virtual const char* getQueryName() const override;

  void
  findAllIntralineCarrier(std::vector<tse::IntralineCarrierInfo*>& intralineCarriers);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllIntralineCarrierHistorical

} // namespace tse

#endif
