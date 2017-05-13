//----------------------------------------------------------------------------
//          File:           QueryGetSurfaceTransfersInfo.h
//          Description:    QueryGetSurfaceTransfersInfo
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
#include "DBAccess/SQLQuery.h"
#include "DBAccess/SurfaceTransfersInfo.h"

namespace tse
{

class QueryGetSurfaceTransfersInfo : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSurfaceTransfersInfo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSurfaceTransfersInfo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSurfaceTransfersInfo() {};
  virtual const char* getQueryName() const override;

  void findSurfaceTransfersInfo(std::vector<tse::SurfaceTransfersInfo*>& lstTI,
                                VendorCode& vendor,
                                int itemNo);

  static void initialize();

  const QueryGetSurfaceTransfersInfo& operator=(const QueryGetSurfaceTransfersInfo& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSurfaceTransfersInfo& operator=(const std::string& Another)
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
}; // class QueryGetSurfaceTransfersInfo

class QueryGetSurfaceTransfersInfoHistorical : public QueryGetSurfaceTransfersInfo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSurfaceTransfersInfoHistorical(DBAdapter* dbAdapt)
    : QueryGetSurfaceTransfersInfo(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetSurfaceTransfersInfoHistorical() {}
  virtual const char* getQueryName() const override;

  void findSurfaceTransfersInfo(std::vector<tse::SurfaceTransfersInfo*>& lstTI,
                                VendorCode& vendor,
                                int itemNo,
                                const DateTime& startDate,
                                const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSurfaceTransfersInfoHistorical
} // namespace tse

