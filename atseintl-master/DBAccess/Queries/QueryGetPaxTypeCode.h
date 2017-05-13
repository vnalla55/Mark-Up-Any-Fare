//----------------------------------------------------------------------------
//  � 2013, Sabre Inc. All rights reserved. This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/SQLQuery.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

class PaxTypeCodeInfo;

class QueryGetPaxTypeCode : public SQLQuery
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetPaxTypeCode(DBAdapter* dbAdapt);

  QueryGetPaxTypeCode(DBAdapter* dbAdapt, const std::string& sqlStatement);

  virtual ~QueryGetPaxTypeCode();

  virtual const char* getQueryName() const override;

  void findPaxTypeCodeInfo(std::vector<const PaxTypeCodeInfo*>& data,
                           const VendorCode& vendor,
                           int itemNumber);

  static void initialize();

  const QueryGetPaxTypeCode& operator=(const QueryGetPaxTypeCode& another);

  const QueryGetPaxTypeCode& operator=(const std::string& another);

private:
  static void deinitialize() { _isInitialized = false; }

  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

}; // class QueryGetPaxTypeCode

class QueryGetPaxTypeCodeHistorical : public QueryGetPaxTypeCode
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetPaxTypeCodeHistorical(DBAdapter* dbAdapt);
  virtual ~QueryGetPaxTypeCodeHistorical();

  virtual const char* getQueryName() const override;

  void findPaxTypeCodeInfo(std::vector<const PaxTypeCodeInfo*>& data,
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
}; // class QueryGetPaxTypeCodeHistorical

} // namespace tse

