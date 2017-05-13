//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/CustomerActivationControl.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetCustomerActivationControl : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCustomerActivationControl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetCustomerActivationControl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetCustomerActivationControl() {}

  virtual const char* getQueryName() const override;

  void
  findCustomerActivationControl(std::vector<CustomerActivationControl*>& customerActivationControls,
                                const std::string& projectCode);
  void getDetailActivationData(std::vector<CustomerActivationControl*>& customerActivationControls);

  static void initialize();

  const QueryGetCustomerActivationControl&
  operator=(const QueryGetCustomerActivationControl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };

  const QueryGetCustomerActivationControl& operator=(const std::string& Another)
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
}; // class QueryGetCustomerActivationControl

class QueryGetCustomerActivationControlHistorical : public QueryGetCustomerActivationControl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCustomerActivationControlHistorical(DBAdapter* dbAdapt)
    : QueryGetCustomerActivationControl(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCustomerActivationControlHistorical() {}
  virtual const char* getQueryName() const override;

  void
  findCustomerActivationControl(std::vector<CustomerActivationControl*>& customerActivationControls,
                                const std::string& projectCode,
                                const DateTime& startDate,
                                const DateTime& endDate);
  void getDetailActivationDataHistorical(
      std::vector<CustomerActivationControl*>& customerActivationControls);

  static void initialize();

  const QueryGetCustomerActivationControlHistorical&
  operator=(const QueryGetCustomerActivationControlHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };

  const QueryGetCustomerActivationControlHistorical& operator=(const std::string& Another)
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
}; // class QueryGetCustomerActivationControlHistorical

class QueryGetMultiHostActivationAppl : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMultiHostActivationAppl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetMultiHostActivationAppl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetMultiHostActivationAppl() {}

  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void findMultiHostActivationAppl(CustomerActivationControl* cacRec);

  static void initialize();

  const QueryGetMultiHostActivationAppl& operator=(const QueryGetMultiHostActivationAppl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };

  const QueryGetMultiHostActivationAppl& operator=(const std::string& Another)
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
}; // class QueryGetMultiHostActivationAppl

class QueryGetMultiHostActivationApplHistorical : public QueryGetMultiHostActivationAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMultiHostActivationApplHistorical(DBAdapter* dbAdapt)
    : QueryGetMultiHostActivationAppl(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetMultiHostActivationApplHistorical() {}
  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void findMultiHostActivationAppl(CustomerActivationControl* cacRec);

  static void initialize();

  const QueryGetMultiHostActivationApplHistorical&
  operator=(const QueryGetMultiHostActivationApplHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };

  const QueryGetMultiHostActivationApplHistorical& operator=(const std::string& Another)
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
}; // class QueryGetMultiHostActivationApplHistorical

class QueryGetCarrierActivationAppl : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierActivationAppl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetCarrierActivationAppl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetCarrierActivationAppl() {}

  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void findCarrierActivationAppl(CustomerActivationControl* cacRec);

  static void initialize();

  const QueryGetCarrierActivationAppl& operator=(const QueryGetCarrierActivationAppl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };

  const QueryGetCarrierActivationAppl& operator=(const std::string& Another)
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
}; // class QueryGetCarrierActivationAppl

class QueryGetCarrierActivationApplHistorical : public QueryGetCarrierActivationAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierActivationApplHistorical(DBAdapter* dbAdapt)
    : QueryGetCarrierActivationAppl(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCarrierActivationApplHistorical() {}
  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void findCarrierActivationAppl(CustomerActivationControl* cacRec);

  static void initialize();

  const QueryGetCarrierActivationApplHistorical&
  operator=(const QueryGetCarrierActivationApplHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };

  const QueryGetCarrierActivationApplHistorical& operator=(const std::string& Another)
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
}; // class QueryGetCarrierActivationApplHistorical

class QueryGetGeoLocationAppl : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGeoLocationAppl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetGeoLocationAppl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetGeoLocationAppl() {}

  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void findGeoLocationAppl(CustomerActivationControl* cacRec);

  static void initialize();

  const QueryGetGeoLocationAppl& operator=(const QueryGetGeoLocationAppl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };

  const QueryGetGeoLocationAppl& operator=(const std::string& Another)
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
}; // class QueryGetGeoLocationAppl

class QueryGetGeoLocationApplHistorical : public QueryGetGeoLocationAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGeoLocationApplHistorical(DBAdapter* dbAdapt) : QueryGetGeoLocationAppl(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetGeoLocationApplHistorical() {}
  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void findGeoLocationAppl(CustomerActivationControl* cacRec);

  static void initialize();

  const QueryGetGeoLocationApplHistorical&
  operator=(const QueryGetGeoLocationApplHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };

  const QueryGetGeoLocationApplHistorical& operator=(const std::string& Another)
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
}; // class QueryGetGeoLocationApplHistorical

} // tse

