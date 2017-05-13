//-------------------------------------------------------------------------------
// Copyright 2011, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#ifndef QUERY_GET_OPTIONAL_SERVICES_H
#define QUERY_GET_OPTIONAL_SERVICES_H

#include "DBAccess/SQLQuery.h"

namespace tse
{
class OptionalServicesInfo;

class QueryGetOptionalServicesMkt : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOptionalServicesMkt(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetOptionalServicesMkt(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetOptionalServicesMkt() {}

  virtual const char* getQueryName() const override;

  void findOptionalServicesMktInfo(std::vector<OptionalServicesInfo*>& optionalServices,
                                   const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const LocCode& loc1,
                                   const LocCode& loc2);

  static void initialize();

  const QueryGetOptionalServicesMkt& operator=(const QueryGetOptionalServicesMkt& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetOptionalServicesMkt& operator=(const std::string& Another)
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
};

class QueryGetOptionalServicesMktHistorical : public QueryGetOptionalServicesMkt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOptionalServicesMktHistorical(DBAdapter* dbAdapt)
    : QueryGetOptionalServicesMkt(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetOptionalServicesMktHistorical() {}
  virtual const char* getQueryName() const override;

  void findOptionalServicesMktInfo(std::vector<OptionalServicesInfo*>& optionalServices,
                                   const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const LocCode& loc1,
                                   const LocCode& loc2,
                                   const DateTime& startDate,
                                   const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

#endif
