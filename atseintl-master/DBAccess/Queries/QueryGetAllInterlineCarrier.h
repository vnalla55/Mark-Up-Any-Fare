//----------------------------------------------------------------------------
//     (c)2015, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#ifndef QUERYGETALLINTERLINECARRIER_H
#define QUERYGETALLINTERLINECARRIER_H

#include "Common/Logger.h"
#include "DBAccess/InterlineCarrierInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetAllInterlineCarrier : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllInterlineCarrier(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAllInterlineCarrier(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAllInterlineCarrier() {};

  virtual const char* getQueryName() const override;

  void
  findAllInterlineCarrier(std::vector<tse::InterlineCarrierInfo*>& interlineCarriers);

  void
  execute(std::vector<tse::InterlineCarrierInfo*>& interlineCarriers)
  {
    findAllInterlineCarrier(interlineCarriers);
  }

  static void initialize();

  const QueryGetAllInterlineCarrier& operator=(const QueryGetAllInterlineCarrier& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAllInterlineCarrier& operator=(const std::string& Another)
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
}; // class QueryGetAllInterlineCarrier

class QueryGetAllInterlineCarrierHistorical : public tse::QueryGetAllInterlineCarrier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllInterlineCarrierHistorical(DBAdapter* dbAdapt)
    : QueryGetAllInterlineCarrier(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllInterlineCarrierHistorical() {};

  virtual const char* getQueryName() const override;

  void
  findAllInterlineCarrier(std::vector<tse::InterlineCarrierInfo*>& interlineCarriers);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllInterlineCarrierHistorical

} // namespace tse

#endif
