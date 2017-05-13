//----------------------------------------------------------------------------
//  � 2013, Sabre Inc. All rights reserved. This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class ServiceBaggageInfo;

class QueryGetServiceBaggage : public SQLQuery
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetServiceBaggage(DBAdapter* dbAdapt);

  QueryGetServiceBaggage(DBAdapter* dbAdapt, const std::string& sqlStatement);

  virtual ~QueryGetServiceBaggage();

  virtual const char* getQueryName() const override;

  void findServiceBaggageInfo(std::vector<const ServiceBaggageInfo*>& data,
                              const VendorCode& vendor,
                              int itemNumber);

  static void initialize();

  const QueryGetServiceBaggage& operator=(const QueryGetServiceBaggage& another);

  const QueryGetServiceBaggage& operator=(const std::string& another);

private:
  static void deinitialize() { _isInitialized = false; }

  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

}; // class QueryGetServiceBaggage

class QueryGetServiceBaggageHistorical : public QueryGetServiceBaggage
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetServiceBaggageHistorical(DBAdapter* dbAdapt);
  virtual ~QueryGetServiceBaggageHistorical();

  virtual const char* getQueryName() const override;

  void findServiceBaggageInfo(std::vector<const ServiceBaggageInfo*>& data,
                              const VendorCode& vendor,
                              int itemNumber,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static void deinitialize() { _isInitialized = false; }

  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetServiceBaggageHistorical

} // namespace tse

