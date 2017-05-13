//----------------------------------------------------------------------------
//          File:           QueryCheckLocsAndZones.h
//          Description:    QueryCheckLocsAndZones
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
#include "DBAccess/AddonZoneInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
enum IncludeExcludeLoc
{
  INCLUDE_LOC,
  EXCLUDE_LOC,
};


class QueryGetZoneLocTypes : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetZoneLocTypes(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetZoneLocTypes(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetZoneLocTypes() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  std::vector<std::string>
  getLocTypes(char* zone, char* zoneType, std::string inclExcl, const VendorCode& vendor);

  static void initialize();

  const QueryGetZoneLocTypes& operator=(const QueryGetZoneLocTypes& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetZoneLocTypes& operator=(const std::string& Another)
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
}; // class QueryGetZoneLocTypes

class QueryGetZoneLocTypesHistorical : public QueryGetZoneLocTypes
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetZoneLocTypesHistorical(DBAdapter* dbAdapt) : QueryGetZoneLocTypes(dbAdapt, _baseSQL) {};
  virtual ~QueryGetZoneLocTypesHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  std::vector<std::string> getLocTypes(char* zone,
                                       char* zoneType,
                                       std::string inclExcl,
                                       const VendorCode& vendor,
                                       const DateTime& startDate,
                                       const DateTime& endDate);

  static void initialize();

  const QueryGetZoneLocTypesHistorical& operator=(const QueryGetZoneLocTypesHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetZoneLocTypesHistorical& operator=(const std::string& Another)
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
}; // class QueryGetZoneLocTypesHistorical

class QueryGetMktCtyZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktCtyZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMktCtyZone(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMktCtyZone() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  int getScore(char* zoneType,
               std::string inclExcl,
               const tse::LocCode& loc,
               char* zone,
               const VendorCode& vendor);

  static void initialize();

  const QueryGetMktCtyZone& operator=(const QueryGetMktCtyZone& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktCtyZone& operator=(const std::string& Another)
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
}; // class QueryGetMktCtyZone

class QueryGetMktCtyZoneHistorical : public QueryGetMktCtyZone
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktCtyZoneHistorical(DBAdapter* dbAdapt) : QueryGetMktCtyZone(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMktCtyZoneHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  int getScore(char* zoneType,
               std::string inclExcl,
               const tse::LocCode& loc,
               char* zone,
               const VendorCode& vendor,
               const DateTime& startDate,
               const DateTime& endDate);

  static void initialize();

  const QueryGetMktCtyZoneHistorical& operator=(const QueryGetMktCtyZoneHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktCtyZoneHistorical& operator=(const std::string& Another)
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
}; // class QueryGetMktCtyZoneHistorical

class QueryGetMktStateZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktStateZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMktStateZone(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMktStateZone() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  int getScore(char* zoneType,
               std::string inclExcl,
               const tse::LocCode& loc,
               char* zone,
               const VendorCode& vendor);

  static void initialize();

  const QueryGetMktStateZone& operator=(const QueryGetMktStateZone& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktStateZone& operator=(const std::string& Another)
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
}; // class QueryGetMktStateZone

class QueryGetMktStateZoneHistorical : public QueryGetMktStateZone
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktStateZoneHistorical(DBAdapter* dbAdapt) : QueryGetMktStateZone(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMktStateZoneHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  int getScore(char* zoneType,
               std::string inclExcl,
               const tse::LocCode& loc,
               char* zone,
               const VendorCode& vendor,
               const DateTime& startDate,
               const DateTime& endDate);

  static void initialize();

  const QueryGetMktStateZoneHistorical& operator=(const QueryGetMktStateZoneHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktStateZoneHistorical& operator=(const std::string& Another)
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
}; // class QueryGetMktStateZoneHistorical

class QueryGetMktNationZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktNationZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMktNationZone(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMktNationZone() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  int getScore(char* zoneType,
               std::string inclExcl,
               const tse::LocCode& loc,
               char* zone,
               const VendorCode& vendor);

  static void initialize();

  const QueryGetMktNationZone& operator=(const QueryGetMktNationZone& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktNationZone& operator=(const std::string& Another)
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
}; // class QueryGetMktNationZone

class QueryGetMktNationZoneHistorical : public QueryGetMktNationZone
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktNationZoneHistorical(DBAdapter* dbAdapt) : QueryGetMktNationZone(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMktNationZoneHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  int getScore(char* zoneType,
               std::string inclExcl,
               const tse::LocCode& loc,
               char* zone,
               const VendorCode& vendor,
               const DateTime& startDate,
               const DateTime& endDate);

  static void initialize();

  const QueryGetMktNationZoneHistorical& operator=(const QueryGetMktNationZoneHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktNationZoneHistorical& operator=(const std::string& Another)
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
}; // class QueryGetMktNationZoneHistorical

class QueryGetMktSubAreaZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktSubAreaZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMktSubAreaZone(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMktSubAreaZone() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  int getScore(char* zoneType,
               std::string inclExcl,
               const tse::LocCode& loc,
               char* zone,
               const VendorCode& vendor);

  static void initialize();

  const QueryGetMktSubAreaZone& operator=(const QueryGetMktSubAreaZone& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktSubAreaZone& operator=(const std::string& Another)
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
}; // class QueryGetMktSubAreaZone

class QueryGetMktSubAreaZoneHistorical : public QueryGetMktSubAreaZone
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktSubAreaZoneHistorical(DBAdapter* dbAdapt)
    : QueryGetMktSubAreaZone(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMktSubAreaZoneHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  int getScore(char* zoneType,
               std::string inclExcl,
               const tse::LocCode& loc,
               char* zone,
               const VendorCode& vendor,
               const DateTime& startDate,
               const DateTime& endDate);

  static void initialize();

  const QueryGetMktSubAreaZoneHistorical& operator=(const QueryGetMktSubAreaZoneHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktSubAreaZoneHistorical& operator=(const std::string& Another)
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
}; // class QueryGetMktSubAreaZoneHistorical

class QueryGetMktAreaZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktAreaZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMktAreaZone(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMktAreaZone() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  int getScore(char* zoneType,
               std::string inclExcl,
               const tse::LocCode& loc,
               char* zone,
               const VendorCode& vendor);

  static void initialize();

  const QueryGetMktAreaZone& operator=(const QueryGetMktAreaZone& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktAreaZone& operator=(const std::string& Another)
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
}; // class QueryGetMktAreaZone

class QueryGetMktAreaZoneHistorical : public QueryGetMktAreaZone
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktAreaZoneHistorical(DBAdapter* dbAdapt) : QueryGetMktAreaZone(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMktAreaZoneHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  int getScore(char* zoneType,
               std::string inclExcl,
               const tse::LocCode& loc,
               char* zone,
               const VendorCode& vendor,
               const DateTime& startDate,
               const DateTime& endDate);

  static void initialize();

  const QueryGetMktAreaZoneHistorical& operator=(const QueryGetMktAreaZoneHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktAreaZoneHistorical& operator=(const std::string& Another)
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
}; // class QueryGetMktAreaZoneHistorical

class QueryGetMktZoneZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktZoneZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMktZoneZone(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMktZoneZone() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  std::vector<int>
  getZones(char* zoneType, std::string inclExcl, char* zone, const VendorCode& vendor);

  static void initialize();

  const QueryGetMktZoneZone& operator=(const QueryGetMktZoneZone& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktZoneZone& operator=(const std::string& Another)
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
}; // class QueryGetMktZoneZone

class QueryGetMktZoneZoneHistorical : public QueryGetMktZoneZone
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktZoneZoneHistorical(DBAdapter* dbAdapt) : QueryGetMktZoneZone(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMktZoneZoneHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  std::vector<int> getZones(char* zoneType,
                            std::string inclExcl,
                            char* zone,
                            const VendorCode& vendor,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static void initialize();

  const QueryGetMktZoneZoneHistorical& operator=(const QueryGetMktZoneZoneHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMktZoneZoneHistorical& operator=(const std::string& Another)
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
}; // class QueryGetMktZoneZoneHistorical

class QueryGetSITAAddonScore : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSITAAddonScore(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSITAAddonScore(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSITAAddonScore() {};

  const char* getQueryName() const override;

  void evaluateAddonZoneSITA(LocCode& loc,
                             VendorCode& vendor,
                             CarrierCode& carrier,
                             std::vector<AddonZoneInfo*>& validZones);

  bool isLocInZone(const tse::LocCode& loc, int zone, char zoneType, const VendorCode& vendor);

  int evaluateZone(const tse::LocCode& loc,
                   int zone,
                   char zoneType,
                   const VendorCode& vendor,
                   IncludeExcludeLoc inclExclSpec);

  static void initialize();

  const QueryGetSITAAddonScore& operator=(const QueryGetSITAAddonScore& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSITAAddonScore& operator=(const std::string& Another)
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

  int stringToInteger(const char* stringVal, int lineNumber);
}; // class QueryGetSITAAddonScore

class QueryGetSITAAddonScoreHistorical : public QueryGetSITAAddonScore
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSITAAddonScoreHistorical(DBAdapter* dbAdapt)
    : QueryGetSITAAddonScore(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSITAAddonScoreHistorical() {};
  const char* getQueryName() const override;

  void evaluateAddonZoneSITA(LocCode& loc,
                             VendorCode& vendor,
                             CarrierCode& carrier,
                             std::vector<AddonZoneInfo*>& validZones,
                             const DateTime& startDate,
                             const DateTime& endDate);

  bool isLocInZone(const tse::LocCode& loc,
                   int zone,
                   char zoneType,
                   const VendorCode& vendor,
                   const DateTime& startDate,
                   const DateTime& endDate);

  int evaluateZone(const tse::LocCode& loc,
                   int zone,
                   char zoneType,
                   const VendorCode& vendor,
                   IncludeExcludeLoc inclExclSpec,
                   const DateTime& startDate,
                   const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
  int stringToInteger(const char* stringVal, int lineNumber);
}; // class QueryGetSITAAddonScoreHistorical
} // namespace tse
