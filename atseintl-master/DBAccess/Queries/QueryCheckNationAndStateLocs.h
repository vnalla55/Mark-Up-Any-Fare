//----------------------------------------------------------------------------
//          File:           QueryCheckNationAndStateLocs.h
//          Description:    QueryCheckNationAndStateLocs
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
#include "DBAccess/NationStateHistIsCurrChk.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryCheckNationInSubArea : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckNationInSubArea(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckNationInSubArea() {};
  const char* getQueryName() const override;

  bool isNationInSubArea(const NationCode& nation, const LocCode& subArea);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCheckNationInSubArea

class QueryCheckNationInSubAreaHistorical : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckNationInSubAreaHistorical(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckNationInSubAreaHistorical() {};
  const char* getQueryName() const override;

  void findNationInSubArea(std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
                           const NationCode& nation,
                           const LocCode& subArea,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCheckNationInSubAreaHistorical

class QueryCheckNationInArea : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckNationInArea(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckNationInArea() {};
  const char* getQueryName() const override;

  bool isNationInArea(const NationCode& nation, const LocCode& area);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCheckNationInArea

class QueryCheckNationInAreaHistorical : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckNationInAreaHistorical(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckNationInAreaHistorical() {};
  const char* getQueryName() const override;

  void findNationInArea(std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
                        const NationCode& nation,
                        const LocCode& area,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCheckNationInAreaHistorical

class QueryCheckNationInZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckNationInZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckNationInZone() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  bool isNationInZone(const VendorCode& vendor, int zone, char zoneType, const NationCode& nation);

  static void initialize();

  const QueryCheckNationInZone& operator=(const QueryCheckNationInZone& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryCheckNationInZone& operator=(const std::string& Another)
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
}; // class QueryCheckNationInZone

class QueryCheckNationInZoneHistorical : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckNationInZoneHistorical(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckNationInZoneHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void findNationInZone(std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
                        const VendorCode& vendor,
                        int zone,
                        char zoneType,
                        const NationCode& nation,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

  const QueryCheckNationInZoneHistorical& operator=(const QueryCheckNationInZoneHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryCheckNationInZoneHistorical& operator=(const std::string& Another)
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
}; // class QueryCheckNationInZoneHistorical

class QueryCheckNationSubAreaInZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckNationSubAreaInZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckNationSubAreaInZone() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  std::string isNationSubAreaInZone(const VendorCode& vendor,
                                    char* zoneNo,
                                    char* zoneType,
                                    const NationCode& nation);

  static void initialize();

  const QueryCheckNationSubAreaInZone& operator=(const QueryCheckNationSubAreaInZone& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryCheckNationSubAreaInZone& operator=(const std::string& Another)
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
}; // class QueryCheckNationSubAreaInZone

class QueryCheckNationSubAreaInZoneHistorical : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckNationSubAreaInZoneHistorical(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckNationSubAreaInZoneHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void findNationSubAreaInZone(std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
                               const VendorCode& vendor,
                               char* zoneNo,
                               char* zoneType,
                               const NationCode& nation,
                               const DateTime& startDate,
                               const DateTime& endDate);

  static void initialize();

  const QueryCheckNationSubAreaInZoneHistorical&
  operator=(const QueryCheckNationSubAreaInZoneHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryCheckNationSubAreaInZoneHistorical& operator=(const std::string& Another)
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
}; // class QueryCheckNationSubAreaInZoneHistorical

class QueryCheckNationAreaInZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckNationAreaInZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryCheckNationAreaInZone(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryCheckNationAreaInZone() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  std::string isNationAreaInZone(const VendorCode& vendor,
                                 char* zoneNo,
                                 char* zoneType,
                                 const NationCode& nation);

  static void initialize();

  const QueryCheckNationAreaInZone& operator=(const QueryCheckNationAreaInZone& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryCheckNationAreaInZone& operator=(const std::string& Another)
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
}; // class QueryCheckNationAreaInZone

class QueryCheckNationAreaInZoneHistorical : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckNationAreaInZoneHistorical(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckNationAreaInZoneHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void findNationAreaInZone(std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
                            const VendorCode& vendor,
                            char* zoneNo,
                            char* zoneType,
                            const NationCode& nation,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static void initialize();

  const QueryCheckNationAreaInZoneHistorical&
  operator=(const QueryCheckNationAreaInZoneHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryCheckNationAreaInZoneHistorical& operator=(const std::string& Another)
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
}; // class QueryCheckNationAreaInZoneHistorical

class QueryCheckStateInSubArea : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckStateInSubArea(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckStateInSubArea() {};
  const char* getQueryName() const override;

  bool isStateInSubArea(const NationCode& nation, const StateCode& state, const LocCode& subArea);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCheckStateInSubArea

class QueryCheckStateInSubAreaHistorical : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckStateInSubAreaHistorical(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckStateInSubAreaHistorical() {};
  const char* getQueryName() const override;

  void findStateInSubArea(std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
                          const NationCode& nation,
                          const StateCode& state,
                          const LocCode& subArea,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCheckStateInSubAreaHistorical

class QueryCheckStateInArea : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckStateInArea(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckStateInArea() {};
  const char* getQueryName() const override;

  bool isStateInArea(const NationCode& nation, const StateCode& state, const LocCode& area);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCheckStateInArea

class QueryCheckStateInAreaHistorical : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckStateInAreaHistorical(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckStateInAreaHistorical() {};
  const char* getQueryName() const override;

  void findStateInArea(std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
                       const NationCode& nation,
                       const StateCode& state,
                       const LocCode& area,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCheckStateInAreaHistorical

class QueryCheckStateInZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckStateInZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckStateInZone() {};
  const char* getQueryName() const override;

  bool isStateInZone(const VendorCode& vendor,
                     int zone,
                     char zoneType,
                     const NationCode& nation,
                     const StateCode& state);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCheckStateInZone

class QueryCheckStateInZoneHistorical : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCheckStateInZoneHistorical(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryCheckStateInZoneHistorical() {};
  const char* getQueryName() const override;

  void findStateInZone(std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
                       const VendorCode& vendor,
                       int zone,
                       char zoneType,
                       const NationCode& nation,
                       const StateCode& state,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCheckStateInZoneHistorical
} // namespace tse
