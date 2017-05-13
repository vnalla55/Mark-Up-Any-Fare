//----------------------------------------------------------------------------
//          File:           QueryGetFDSFareBasisComb.h
//          Description:    QueryGetFDSFareBasisComb
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
#include "DBAccess/FDSFareBasisComb.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFDSFareBasisComb : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFDSFareBasisComb(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFDSFareBasisComb(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFDSFareBasisComb() {};

  virtual const char* getQueryName() const override;

  void findFDSFareBasisComb(std::vector<tse::FDSFareBasisComb*>& infos,
                            const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const TJRGroup& tjrGroup);

  static void initialize();

  const QueryGetFDSFareBasisComb& operator=(const QueryGetFDSFareBasisComb& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFDSFareBasisComb& operator=(const std::string& Another)
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
}; // class QueryGetFDSFareBasisComb

class QueryGetAllFDSFareBasisComb : public QueryGetFDSFareBasisComb
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFDSFareBasisComb(DBAdapter* dbAdapt) : QueryGetFDSFareBasisComb(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FDSFareBasisComb*>& infos) { findAllFDSFareBasisComb(infos); }

  void findAllFDSFareBasisComb(std::vector<tse::FDSFareBasisComb*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFDSFareBasisComb
} // namespace tse

