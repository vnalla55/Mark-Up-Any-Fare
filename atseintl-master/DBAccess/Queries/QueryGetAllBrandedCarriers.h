//----------------------------------------------------------------------------
//          File:           QueryGetAllBrandedCarriers.h
//          Description:    QueryGetAllBrandedCarriers
//          Created:        2/28/2008
//          Authors:        Mauricio Dantas
//
//          Updates:
//
//     � 2008, Sabre Inc.  All rights reserved.  This software/documentation is the
//     confidential and proprietary product of Sabre Inc. Any unauthorized use,
//     reproduction, or transfer of this software/documentation, in any medium,
//     or incorporation of this software/documentation into any system or publication,
//     is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/BrandedCarrier.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{

class QueryGetAllBrandedCarriers : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllBrandedCarriers(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAllBrandedCarriers(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAllBrandedCarriers() {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::BrandedCarrier*>& infos) { findAllBrandedCarriers(infos); }

  void findAllBrandedCarriers(std::vector<tse::BrandedCarrier*>& infos);

  static void initialize();

  const QueryGetAllBrandedCarriers& operator=(const QueryGetAllBrandedCarriers& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAllBrandedCarriers& operator=(const std::string& Another)
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

}; // class QueryGetAllBrandedCarriers
} // namespace tse

