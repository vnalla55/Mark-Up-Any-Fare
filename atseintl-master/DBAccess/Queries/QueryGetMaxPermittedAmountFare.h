//-------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DBAccess/HashKey.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class MaxPermittedAmountFareInfo;

typedef HashKey<LocCode, LocCode, NationCode, LocCode, LocCode, NationCode>
MaxPermittedAmountFareKey;

class QueryGetMaxPermittedAmountFare : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMaxPermittedAmountFare(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetMaxPermittedAmountFare(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }

  virtual const char* getQueryName() const override;

  void findMaxPermittedAmountFare(std::vector<MaxPermittedAmountFareInfo*>& lst,
                                  const MaxPermittedAmountFareKey& key);

  static void initialize();

  const QueryGetMaxPermittedAmountFare&
  operator=(const QueryGetMaxPermittedAmountFare& Another) = delete;

  const QueryGetMaxPermittedAmountFare& operator=(const std::string& Another)
  {
    if (this != &Another)
      SQLQuery::operator=(Another);
    return *this;
  }

private:
  static std::string _baseSQL;
  static bool _isInitialized;
};

typedef HashKey<LocCode, LocCode, NationCode, LocCode, LocCode, NationCode, DateTime, DateTime>
MaxPermittedAmountFareHistoricalKey;

class QueryGetMaxPermittedAmountFareHistorical : public QueryGetMaxPermittedAmountFare
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMaxPermittedAmountFareHistorical(DBAdapter* dbAdapt)
    : QueryGetMaxPermittedAmountFare(dbAdapt, _baseSQL)
  {
  }

  virtual const char* getQueryName() const override;

  void findMaxPermittedAmountFare(std::vector<MaxPermittedAmountFareInfo*>& lst,
                                  const MaxPermittedAmountFareHistoricalKey& key);

  static void initialize();

private:
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse
