//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
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

class QueryGetOptionalServices : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOptionalServices(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetOptionalServices(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetOptionalServices() {}

  virtual const char* getQueryName() const override;

  void findOptionalServicesInfo(std::vector<OptionalServicesInfo*>& optionalServices,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const Indicator& fltTktMerchInd);

  static void initialize();

  const QueryGetOptionalServices& operator=(const QueryGetOptionalServices& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetOptionalServices& operator=(const std::string& Another)
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

class QueryGetOptionalServicesHistorical : public QueryGetOptionalServices
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOptionalServicesHistorical(DBAdapter* dbAdapt)
    : QueryGetOptionalServices(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetOptionalServicesHistorical() {}
  virtual const char* getQueryName() const override;

  void findOptionalServicesInfo(std::vector<OptionalServicesInfo*>& optionalServices,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const Indicator& fltTktMerchInd,
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
