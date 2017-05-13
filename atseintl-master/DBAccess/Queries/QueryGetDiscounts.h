//----------------------------------------------------------------------------
//          File:           QueryGetDiscounts.h
//          Description:    QueryGetDiscounts
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
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/DiscountSegInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTourDiscount : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTourDiscount(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetTourDiscount(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetTourDiscount() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getTourDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                        const VendorCode& vendor,
                        int itemNumber);

  static void initialize();

  const QueryGetTourDiscount& operator=(const QueryGetTourDiscount& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetTourDiscount& operator=(const std::string& Another)
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
}; // class QueryGetTourDiscount

class QueryGetTourDiscountHistorical : public QueryGetTourDiscount
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTourDiscountHistorical(DBAdapter* dbAdapt) : QueryGetTourDiscount(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTourDiscountHistorical() {}
  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getTourDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                        const VendorCode& vendor,
                        int itemNumber,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

  const QueryGetTourDiscountHistorical& operator=(const QueryGetTourDiscountHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTourDiscountHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTourDiscountHistorical

class QueryGetAgentDiscount : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAgentDiscount(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAgentDiscount(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAgentDiscount() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getAgentDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                         const VendorCode& vendor,
                         int itemNumber);

  static void initialize();

  const QueryGetAgentDiscount& operator=(const QueryGetAgentDiscount& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAgentDiscount& operator=(const std::string& Another)
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
}; // class QueryGetAgentDiscount

class QueryGetAgentDiscountHistorical : public QueryGetAgentDiscount
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAgentDiscountHistorical(DBAdapter* dbAdapt) : QueryGetAgentDiscount(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAgentDiscountHistorical() {}
  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getAgentDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                         const VendorCode& vendor,
                         int itemNumber,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

  const QueryGetAgentDiscountHistorical& operator=(const QueryGetAgentDiscountHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetAgentDiscountHistorical& operator=(const std::string& Another)
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
}; // class QueryGetAgentDiscountHistorical

class QueryGetOthersDiscount : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOthersDiscount(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetOthersDiscount(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetOthersDiscount() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getOthersDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                          const VendorCode& vendor,
                          int itemNumber);

  static void initialize();

  const QueryGetOthersDiscount& operator=(const QueryGetOthersDiscount& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetOthersDiscount& operator=(const std::string& Another)
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
}; // class QueryGetOthersDiscount

class QueryGetOthersDiscountHistorical : public QueryGetOthersDiscount
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOthersDiscountHistorical(DBAdapter* dbAdapt) : QueryGetOthersDiscount(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetOthersDiscountHistorical() {}
  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getOthersDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                          const VendorCode& vendor,
                          int itemNumber,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

  const QueryGetOthersDiscountHistorical& operator=(const QueryGetOthersDiscountHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetOthersDiscountHistorical& operator=(const std::string& Another)
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
}; // class QueryGetOthersDiscountHistorical

class QueryGetChdDiscount : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetChdDiscount(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetChdDiscount(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetChdDiscount() {};
  virtual const char* getQueryName() const override;

  void findDiscountInfo(std::vector<tse::DiscountInfo*>& discount,
                        const VendorCode& vendor,
                        int itemNumber,
                        int category);

  static void initialize();

  const QueryGetChdDiscount& operator=(const QueryGetChdDiscount& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = (SQLQuery&)Another;
    return *this;
  };
  const QueryGetChdDiscount& operator=(const std::string& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = Another;
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetChdDiscount

class QueryGetChdDiscountHistorical : public QueryGetChdDiscount
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetChdDiscountHistorical(DBAdapter* dbAdapt) : QueryGetChdDiscount(dbAdapt, _baseSQL) {}
  virtual ~QueryGetChdDiscountHistorical() {}
  virtual const char* getQueryName() const override;

  void findDiscountInfo(std::vector<tse::DiscountInfo*>& discount,
                        const VendorCode& vendor,
                        int itemNumber,
                        int category,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetChdDiscountHistorical
} // namespace tse

