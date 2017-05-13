//----------------------------------------------------------------------------
//          File:           QueryGetCarrierPref.h
//          Description:    QueryGetCarrierPref
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
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


namespace
{
struct fbrPref
{
  CarrierCode cxr;
  DateTime createDate;
  VendorCode vendor;
  bool operator==(const fbrPref& rhs) const
  {
    const fbrPref& lhs(*this);
    return lhs.cxr == rhs.cxr && lhs.createDate == rhs.createDate && lhs.vendor == rhs.vendor;
  }
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, cxr);
    FLATTENIZE(archive, createDate);
    FLATTENIZE(archive, vendor);
  }
};
}

class QueryGetCarrierPrefFBRPref : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierPrefFBRPref(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCarrierPrefFBRPref() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getFbrPrefs(std::vector<fbrPref>& fbrPrefs, const CarrierCode& carrier);

  static void initialize();

  const QueryGetCarrierPrefFBRPref& operator=(const QueryGetCarrierPrefFBRPref& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCarrierPrefFBRPref& operator=(const std::string& Another)
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
}; // class QueryGetCarrierPrefFBRPref

class QueryGetAllCarrierPrefFBRPref : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCarrierPrefFBRPref(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCarrierPrefFBRPref() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  virtual const char* getQueryName() const override;

  void getAllFbrPrefs(std::vector<fbrPref>& cxrFbrPrefs);

  static void initialize();

  const QueryGetAllCarrierPrefFBRPref& operator=(const QueryGetAllCarrierPrefFBRPref& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAllCarrierPrefFBRPref& operator=(const std::string& Another)
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
}; // class QueryGetAllCarrierPrefFBRPref

class QueryGetCarrierPref : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierPref(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCarrierPref(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCarrierPref() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  virtual const char* getQueryName() const override;

  void findCarrierPref(std::vector<tse::CarrierPreference*>& cxrprefs, const CarrierCode& carrier);

  static void initialize();

  const QueryGetCarrierPref& operator=(const QueryGetCarrierPref& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = (SQLQuery&)Another;
    return *this;
  };
  const QueryGetCarrierPref& operator=(const std::string& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = Another;
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCarrierPref

class QueryGetCarrierPrefHistorical : public tse::QueryGetCarrierPref
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierPrefHistorical(DBAdapter* dbAdapt) : QueryGetCarrierPref(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCarrierPrefHistorical() {};
  void resetSQL()
  {
    *this = _baseSQL;
  };
  virtual const char* getQueryName() const override;

  void findCarrierPref(std::vector<tse::CarrierPreference*>& cxrprefs, const CarrierCode& carrier);

  static void initialize();

  const QueryGetCarrierPrefHistorical& operator=(const QueryGetCarrierPrefHistorical& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = (SQLQuery&)Another;
    return *this;
  };
  const QueryGetCarrierPrefHistorical& operator=(const std::string& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = Another;
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCarrierPrefHistorical

class QueryGetAllCarrierPref : public QueryGetCarrierPref
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCarrierPref(DBAdapter* dbAdapt) : QueryGetCarrierPref(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCarrierPref() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::CarrierPreference*>& cxrprefs) { findAllCarrierPref(cxrprefs); }

  void findAllCarrierPref(std::vector<tse::CarrierPreference*>& cxrprefs);

  static void initialize();

  const QueryGetAllCarrierPref& operator=(const QueryGetAllCarrierPref& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = (SQLQuery&)Another;
    return *this;
  };
  const QueryGetAllCarrierPref& operator=(const std::string& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = Another;
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCarrierPref

class QueryGetAllCarrierPrefHistorical : public QueryGetCarrierPref
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCarrierPrefHistorical(DBAdapter* dbAdapt) : QueryGetCarrierPref(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCarrierPrefHistorical() {};
  void resetSQL()
  {
    *this = _baseSQL;
  };
  virtual const char* getQueryName() const override;

  void findAllCarrierPref(std::vector<tse::CarrierPreference*>& cxrprefs);

  static void initialize();

  const QueryGetAllCarrierPrefHistorical& operator=(const QueryGetAllCarrierPrefHistorical& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = (SQLQuery&)Another;
    return *this;
  };
  const QueryGetAllCarrierPrefHistorical& operator=(const std::string& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = Another;
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCarrierPrefHistorical
} // namespace tse

