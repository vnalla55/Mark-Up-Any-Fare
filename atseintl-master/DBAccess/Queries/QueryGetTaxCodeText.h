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

class TaxCodeTextInfo;

class QueryGetTaxCodeText : public SQLQuery
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetTaxCodeText(DBAdapter* dbAdapt);

  QueryGetTaxCodeText(DBAdapter* dbAdapt, const std::string& sqlStatement);

  virtual ~QueryGetTaxCodeText();

  virtual const char* getQueryName() const override;

  void findTaxCodeTextInfo(std::vector<const TaxCodeTextInfo*>& data,
                           const VendorCode& vendor,
                           int itemNumber);

  static void initialize();

  const QueryGetTaxCodeText& operator=(const QueryGetTaxCodeText& another);

  const QueryGetTaxCodeText& operator=(const std::string& another);

private:
  static void deinitialize() { _isInitialized = false; }

  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

}; // class QueryGetTaxCodeText

class QueryGetTaxCodeTextHistorical : public QueryGetTaxCodeText
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetTaxCodeTextHistorical(DBAdapter* dbAdapt);
  virtual ~QueryGetTaxCodeTextHistorical();

  virtual const char* getQueryName() const override;

  void findTaxCodeTextInfo(std::vector<const TaxCodeTextInfo*>& data,
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
}; // class QueryGetTaxCodeTextHistorical

} // namespace tse

