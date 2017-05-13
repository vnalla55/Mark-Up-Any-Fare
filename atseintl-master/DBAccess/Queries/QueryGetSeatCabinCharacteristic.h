//----------------------------------------------------------------------------
//      File:         QueryGetSeatCabinCharacteristic.h
//      Description:  QueryGetSeatCabinCharacteristic
//      Created:      10/10/2012
//      Authors:      Ram Papineni
//
//      Updates:
//
//   � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SeatCabinCharacteristicInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetSeatCabinCharacteristic : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSeatCabinCharacteristic(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSeatCabinCharacteristic(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSeatCabinCharacteristic() {};
  virtual const char* getQueryName() const override;

  void findSeatCabinCharacteristic(std::vector<tse::SeatCabinCharacteristicInfo*>& sccInfoList,
                                   const CarrierCode& carrier,
                                   const Indicator& codeType);

  static void initialize();

  const QueryGetSeatCabinCharacteristic& operator=(const QueryGetSeatCabinCharacteristic& rhs)
  {
    if (this != &rhs)
    {
      *((SQLQuery*)this) = (SQLQuery&)rhs;
    }
    return *this;
  };
  const QueryGetSeatCabinCharacteristic& operator=(const std::string& rhs)
  {
    if (this != &rhs)
    {
      *((SQLQuery*)this) = rhs;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSeatCabinCharacteristic

class QueryGetSeatCabinCharacteristicHistorical : public QueryGetSeatCabinCharacteristic
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSeatCabinCharacteristicHistorical(DBAdapter* dbAdapt)
    : QueryGetSeatCabinCharacteristic(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSeatCabinCharacteristicHistorical() {};
  virtual const char* getQueryName() const override;

  void findSeatCabinCharacteristic(std::vector<tse::SeatCabinCharacteristicInfo*>& sccInfoList,
                                   const CarrierCode& carrier,
                                   const Indicator& codeType,
                                   const DateTime& startDate,
                                   const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSeatCabinCharacteristicHistorical

class QueryGetAllSeatCabinCharacteristic : public QueryGetSeatCabinCharacteristic
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllSeatCabinCharacteristic(DBAdapter* dbAdapt)
    : QueryGetSeatCabinCharacteristic(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllSeatCabinCharacteristic() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::SeatCabinCharacteristicInfo*>& sccInfoList)
  {
    findAllSeatCabinCharacteristic(sccInfoList);
  }

  void findAllSeatCabinCharacteristic(std::vector<tse::SeatCabinCharacteristicInfo*>& sccInfoList);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllSeatCabinCharacteristic
} // namespace tse

