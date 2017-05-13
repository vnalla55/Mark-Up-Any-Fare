//----------------------------------------------------------------------------
//      2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/ServiceFeesCxrActivation.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetServiceFeesCxrActivation : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetServiceFeesCxrActivation(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetServiceFeesCxrActivation(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetServiceFeesCxrActivation() {};
  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void findServiceFeesCxrActivation(std::vector<tse::ServiceFeesCxrActivation*>& svcCxr,
                                    const CarrierCode& carrier);

  static void initialize();

  const QueryGetServiceFeesCxrActivation& operator=(const QueryGetServiceFeesCxrActivation& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };

  const QueryGetServiceFeesCxrActivation& operator=(const std::string& Another)
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
};

class QueryGetServiceFeesCxrActivationHistorical : public QueryGetServiceFeesCxrActivation
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetServiceFeesCxrActivationHistorical(DBAdapter* dbAdapt)
    : QueryGetServiceFeesCxrActivation(dbAdapt, _baseSQL) {};
  virtual ~QueryGetServiceFeesCxrActivationHistorical() {}
  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };
  void findServiceFeesCxrActivation(std::vector<tse::ServiceFeesCxrActivation*>& svcCxr,
                                    const CarrierCode& carrier,
                                    const DateTime& startDate,
                                    const DateTime& endDate);
  static void initialize();

  const QueryGetServiceFeesCxrActivationHistorical&
  operator=(const QueryGetServiceFeesCxrActivationHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };

  const QueryGetServiceFeesCxrActivationHistorical& operator=(const std::string& Another)
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
};
}
