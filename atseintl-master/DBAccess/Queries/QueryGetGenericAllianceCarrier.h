// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class Logger;

class QueryGetGenericAllianceCarrier : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGenericAllianceCarrier(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetGenericAllianceCarrier(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetGenericAllianceCarrier() {};
  virtual const char* getQueryName() const override;

  void findGenericAllianceCarrierInfo(
      std::vector<tse::AirlineAllianceCarrierInfo*>& AirlineAllianceCarrierInfovec,
      const GenericAllianceCode& genericAllianceCode);

  static void initialize();

  const QueryGetGenericAllianceCarrier& operator=(const QueryGetGenericAllianceCarrier& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetGenericAllianceCarrier& operator=(const std::string& Another)
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

class QueryGetGenericAllianceCarrierHistorical : public QueryGetGenericAllianceCarrier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGenericAllianceCarrierHistorical(DBAdapter* dbAdapt)
    : QueryGetGenericAllianceCarrier(dbAdapt, _baseSQL) {};
  virtual ~QueryGetGenericAllianceCarrierHistorical() {}
  virtual const char* getQueryName() const override;

  void findGenericAllianceCarrierInfo(
      std::vector<tse::AirlineAllianceCarrierInfo*>& AirlineAllianceCarrierInfovec,
      const GenericAllianceCode& genericAllianceCode,
      const DateTime& startDate,
      const DateTime& endDate);

  static void initialize();

private:
  static Logger _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
}

