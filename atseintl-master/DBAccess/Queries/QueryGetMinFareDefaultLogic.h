//----------------------------------------------------------------------------
//          File:           QueryGetMinFareDefaultLogic.h
//          Description:    QueryGetMinFareDefaultLogic
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
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetMinFareDefaultLogic : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareDefaultLogic(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareDefaultLogic(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareDefaultLogic() {};
  virtual const char* getQueryName() const override;

  void findMinFareDefaultLogic(std::vector<tse::MinFareDefaultLogic*>& lstMFDL,
                               const CarrierCode& governingCarrier);

  static void initialize();

  const QueryGetMinFareDefaultLogic& operator=(const QueryGetMinFareDefaultLogic& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareDefaultLogic& operator=(const std::string& Another)
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
}; // class QueryGetMinFareDefaultLogic

class QueryGetMinFareDefaultLogicHistorical : public QueryGetMinFareDefaultLogic
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareDefaultLogicHistorical(DBAdapter* dbAdapt)
    : QueryGetMinFareDefaultLogic(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMinFareDefaultLogicHistorical() {};
  virtual const char* getQueryName() const override;

  void findMinFareDefaultLogic(std::vector<tse::MinFareDefaultLogic*>& lstMFDL,
                               const CarrierCode& governingCarrier,
                               const VendorCode& vendor,
                               const DateTime& startDate,
                               const DateTime& endDate);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMinFareDefaultLogicHist

class QueryGetAllMinFareDefaultLogic : public QueryGetMinFareDefaultLogic
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMinFareDefaultLogic(DBAdapter* dbAdapt)
    : QueryGetMinFareDefaultLogic(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllMinFareDefaultLogic() {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::MinFareDefaultLogic*>& lstMFDL)
  {
    findAllMinFareDefaultLogic(lstMFDL);
  }

  void findAllMinFareDefaultLogic(std::vector<tse::MinFareDefaultLogic*>& lstMFDL);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMinFareDefaultLogic

class QueryGetAllMinFareDefaultLogicHistorical : public QueryGetMinFareDefaultLogic
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMinFareDefaultLogicHistorical(DBAdapter* dbAdapt)
    : QueryGetMinFareDefaultLogic(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllMinFareDefaultLogicHistorical() {};

  virtual const char* getQueryName() const override;

  void findAllMinFareDefaultLogic(std::vector<tse::MinFareDefaultLogic*>& lstMFDL);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMinFareDefaultLogicHistorical

} // namespace tse
