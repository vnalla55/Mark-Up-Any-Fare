//----------------------------------------------------------------------------
//              File:           QueryGetDomIntFares.h
//              Description:    QueryGetDomIntFares
//              Created:        3/2/2006
//     Authors:         Mike Lillis
//
//              Updates:
//
//          2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//         and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//         or transfer of this software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/SITAFareInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetDomMarkets_Fare : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDomMarkets_Fare(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetDomMarkets_Fare(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetDomMarkets_Fare() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getDomMarkets(std::vector<LocCode>& domMkts);

  static void initialize();

  const QueryGetDomMarkets_Fare& operator=(const QueryGetDomMarkets_Fare& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetDomMarkets_Fare& operator=(const std::string& Another)
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
}; // class QueryGetDomMarkets_Fare

struct MarketAndDate
{
public:
  LocCode market;
  DateTime createDate;
  DateTime expireDate;
  MarketAndDate() {}
  MarketAndDate(LocCode mkt, const DateTime& create, const DateTime& expire)
    : market(mkt), createDate(create), expireDate(expire)
  {
  }
  ~MarketAndDate() {}
};

class QueryGetDomMarkets_FareHistorical : public tse::QueryGetDomMarkets_Fare
{
public:
  QueryGetDomMarkets_FareHistorical(DBAdapter* dbAdapt)
    : QueryGetDomMarkets_Fare(dbAdapt, _baseSQL) {};
  virtual ~QueryGetDomMarkets_FareHistorical() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getDomMarkets(std::vector<MarketAndDate>& domMkts);

  static void initialize();

  const QueryGetDomMarkets_FareHistorical&
  operator=(const QueryGetDomMarkets_FareHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetDomMarkets_FareHistorical& operator=(const std::string& Another)
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
}; // class QueryGetDomMarkets_FareHistorical

class QueryGetIntFares : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetIntFares(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetIntFares(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetIntFares() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  virtual const char* getQueryName() const override;

  void getIntFares(std::vector<const tse::FareInfo*>& lstFI,
                   const LocCode& market1,
                   const LocCode& market2,
                   const CarrierCode& carrier);

  void getIntFares(std::vector<const tse::FareInfo*>& lstFI,
                   const LocCode& market1,
                   const LocCode& market2,
                   const std::vector<CarrierCode>& carriers);

  static void initialize();

  const QueryGetIntFares& operator=(const QueryGetIntFares& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetIntFares& operator=(const std::string& Another)
  {
    if (LIKELY(this != &Another))
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetIntFares

class QueryGetIntFaresHistorical : public tse::QueryGetIntFares
{
public:
  QueryGetIntFaresHistorical(DBAdapter* dbAdapt) : QueryGetIntFares(dbAdapt, _baseSQL) {};
  virtual ~QueryGetIntFaresHistorical() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  virtual const char* getQueryName() const override;

  void getIntFares(std::vector<const tse::FareInfo*>& lstFI,
                   const LocCode& market1,
                   const LocCode& market2,
                   const CarrierCode& carrier,
                   const DateTime& startDate,
                   const DateTime& endDate);

  void getIntFares(std::vector<const tse::FareInfo*>& lstFI,
                   const LocCode& market1,
                   const LocCode& market2,
                   const std::vector<CarrierCode>& carriers,
                   const DateTime& startDate,
                   const DateTime& endDate);

  static void initialize();

  const QueryGetIntFaresHistorical& operator=(const QueryGetIntFaresHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetIntFaresHistorical& operator=(const std::string& Another)
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
}; // class QueryGetIntFaresHistorical

class QueryGetAllFaresHistorical : public tse::QueryGetIntFares
{
public:
  QueryGetAllFaresHistorical(DBAdapter* dbAdapt) : QueryGetIntFares(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllFaresHistorical() {};
  void resetSQL()
  {
    *this = _baseSQL;
  };
  virtual const char* getQueryName() const override;

  void getAllFares(std::vector<const tse::FareInfo*>& lstFI,
                   const LocCode& market1,
                   const LocCode& market2,
                   const CarrierCode& carrier,
                   const DateTime& startDate,
                   const DateTime& endDate);

  void getAllFares(std::vector<const tse::FareInfo*>& lstFI,
                   const LocCode& market1,
                   const LocCode& market2,
                   const std::vector<CarrierCode>& carriers,
                   const DateTime& startDate,
                   const DateTime& endDate);

  static void initialize();

  const QueryGetAllFaresHistorical& operator=(const QueryGetAllFaresHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAllFaresHistorical& operator=(const std::string& Another)
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
}; // class QueryGetAllFaresHistorical

class QueryGetDomFares : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDomFares(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetDomFares(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetDomFares() {};

  void resetSQL()
  {
    *this = _baseSQL;
  }; // If have to load domMkts (1st time)

  virtual const char* getQueryName() const override;

  void findFares(std::vector<const tse::FareInfo*>& lstFI,
                 const LocCode& market1,
                 const LocCode& market2,
                 const CarrierCode& carrier);

  void findFares(std::vector<const tse::FareInfo*>& lstFI,
                 const LocCode& market1,
                 const LocCode& market2,
                 const std::vector<CarrierCode>& carriers);

  void getDomFares(std::vector<const tse::FareInfo*>& lstFI,
                   const LocCode& market1,
                   const LocCode& market2,
                   const CarrierCode& carrier);

  void getDomFares(std::vector<const tse::FareInfo*>& lstFI,
                   const LocCode& market1,
                   const LocCode& market2,
                   const std::vector<CarrierCode>& carriers);

  static void initialize();

  const QueryGetDomFares& operator=(const QueryGetDomFares& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetDomFares& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

  static void clearDomMkts();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

  bool isDomestic(const LocCode& market1, const LocCode& market2);
  static boost::mutex _mutex; // Thread Safety
  static std::vector<LocCode> _domMkts;
}; // class QueryGetDomFares

class QueryGetDomFaresHistorical : public QueryGetDomFares
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDomFaresHistorical(DBAdapter* dbAdapt) : QueryGetDomFares(dbAdapt, _baseSQL) {};
  QueryGetDomFaresHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetDomFares(dbAdapt, sqlStatement) {};
  virtual ~QueryGetDomFaresHistorical() {};

  void resetSQL()
  {
    *this = _baseSQL;
  }; // If have to load domMkts (1st time)

  virtual const char* getQueryName() const override;

  void findFares(std::vector<const tse::FareInfo*>& lstFI,
                 const LocCode& market1,
                 const LocCode& market2,
                 const CarrierCode& carrier,
                 const DateTime& startDate,
                 const DateTime& endDate);

  void findFares(std::vector<const tse::FareInfo*>& lstFI,
                 const LocCode& market1,
                 const LocCode& market2,
                 const std::vector<CarrierCode>& carriers,
                 const DateTime& startDate,
                 const DateTime& endDate);

  void getDomFares(std::vector<const tse::FareInfo*>& lstFI,
                   const LocCode& market1,
                   const LocCode& market2,
                   const CarrierCode& carrier,
                   const DateTime& startDate,
                   const DateTime& endDate);

  void getDomFares(std::vector<const tse::FareInfo*>& lstFI,
                   const LocCode& market1,
                   const LocCode& market2,
                   const std::vector<CarrierCode>& carriers,
                   const DateTime& startDate,
                   const DateTime& endDate);

  static void initialize();

  const QueryGetDomFaresHistorical& operator=(const QueryGetDomFaresHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetDomFaresHistorical& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

  static void clearDomMkts();

private:
  enum FareRegionType
  {
    INTERNATIONAL_REGION,
    DOMESTIC_REGION,
    MIXED_REGION
  };
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

  FareRegionType isDomestic(const LocCode& market1,
                            const LocCode& market2,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static boost::mutex _mutex; // Thread Safety
  static std::vector<MarketAndDate> _domMkts;
}; // class QueryGetDomFaresHistorical

} // namespace tse

