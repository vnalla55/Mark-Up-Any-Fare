//----------------------------------------------------------------------------
//          File:           QueryGetBasicBookingRequest.h
//          Description:    QueryGetBasicBookingRequest
//          Created:        3/2/2006
//          Authors:        Bruce Melberg
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
#include "DBAccess/BasicBookingRequest.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetBasicBookingRequest : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBasicBookingRequest(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetBasicBookingRequest(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};

  virtual const char* getQueryName() const override;

  void
  findBasicBookingRequest(std::vector<tse::BasicBookingRequest*>& bbr, const CarrierCode& carrier);

  static void initialize();

  const QueryGetBasicBookingRequest& operator=(const QueryGetBasicBookingRequest& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }

    return *this;
  };

  const QueryGetBasicBookingRequest& operator=(const std::string& Another)
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

}; // class QueryGetBasicBookingRequest

class QueryGetBasicBookingRequests : public QueryGetBasicBookingRequest
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBasicBookingRequests(DBAdapter* dbAdapt)
    : QueryGetBasicBookingRequest(dbAdapt, _baseSQL) {};

  void execute(std::vector<tse::BasicBookingRequest*>& bbr) { findAllBBR(bbr); }

  void findAllBBR(std::vector<tse::BasicBookingRequest*>& bbr);

  const char* getQueryName() const override;

  static void initialize();

  const QueryGetBasicBookingRequests& operator=(const QueryGetBasicBookingRequests& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }

    return *this;
  };

  const QueryGetBasicBookingRequests& operator=(const std::string& Another)
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

}; // class QueryGetBasicBookingRequests
} // namespace tse

