//----------------------------------------------------------------------------
//          File:           QueryGetSurfaceSectorExemptionInfo.h
//          Description:    QueryGetSurfaceSectorExemptionInfo
//          Created:        1/13/2009
//          Author:        Marcin Augustyniak
//
//          Updates:
//
//     � 2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/SurfaceSectorExemptionInfo.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetSurfaceSectorExemptionInfo : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSurfaceSectorExemptionInfo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSurfaceSectorExemptionInfo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSurfaceSectorExemptionInfo() {};

  virtual const char* getQueryName() const override;

  void findSurfaceSectorExemptionInfo(std::vector<SurfaceSectorExemptionInfo*>* sseInfoVec,
                                      const CarrierCode& carrierCode);

  static void initialize();

  const QueryGetSurfaceSectorExemptionInfo&
  operator=(const QueryGetSurfaceSectorExemptionInfo& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetSurfaceSectorExemptionInfo& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetSurfaceSectorExemptionInfoHistorical : public QueryGetSurfaceSectorExemptionInfo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSurfaceSectorExemptionInfoHistorical(DBAdapter* dbAdapt)
    : QueryGetSurfaceSectorExemptionInfo(dbAdapt, _baseSQL) {};
  QueryGetSurfaceSectorExemptionInfoHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetSurfaceSectorExemptionInfo(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSurfaceSectorExemptionInfoHistorical() {};

  virtual const char* getQueryName() const override;

  static void initialize();

  const QueryGetSurfaceSectorExemptionInfoHistorical&
  operator=(const QueryGetSurfaceSectorExemptionInfoHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetSurfaceSectorExemptionInfoHistorical& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
}

