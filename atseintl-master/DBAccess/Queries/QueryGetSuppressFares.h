//----------------------------------------------------------------------------
//          File:           QueryGetSuppressFares.h
//          Description:    QueryGetSuppressFares
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
#include "DBAccess/FDSuppressFare.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetSuppressFarePcc : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSuppressFarePcc(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSuppressFarePcc() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void findSuppressFares(std::vector<const tse::FDSuppressFare*>& lstSF,
                         const PseudoCityCode& pseudoCityCode,
                         const Indicator pseudoCityType,
                         const TJRGroup& ssgGroupNo);

  static void initialize();

  const QueryGetSuppressFarePcc& operator=(const QueryGetSuppressFarePcc& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSuppressFarePcc& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSuppressFarePcc

class QueryGetSuppressFarePccCc : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSuppressFarePccCc(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSuppressFarePccCc(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSuppressFarePccCc() {};

  const char* getQueryName() const override;

  void findSuppressFares(std::vector<const tse::FDSuppressFare*>& lstSF,
                         const PseudoCityCode& pseudoCityCode,
                         const Indicator pseudoCityType,
                         const TJRGroup& ssgGroupNo,
                         const CarrierCode& carrier = EMPTY_STRING());

  static void initialize();

  const QueryGetSuppressFarePccCc& operator=(const QueryGetSuppressFarePccCc& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSuppressFarePccCc& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSuppressFarePccCc
} // namespace tse

