//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/SQLQuery.h"

namespace tse
{
class PrintOption;

class QueryGetPrintOption : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPrintOption(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPrintOption(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPrintOption() {}

  virtual const char* getQueryName() const override;

  void findPrintOption(std::vector<PrintOption*>& printOptions,
                       const VendorCode& vendor,
                       const std::string& fareSource);

  static void initialize();

  const QueryGetPrintOption& operator=(const QueryGetPrintOption& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetPrintOption& operator=(const std::string& Another)
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

class QueryGetPrintOptionHistorical : public QueryGetPrintOption
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPrintOptionHistorical(DBAdapter* dbAdapt) : QueryGetPrintOption(dbAdapt, _baseSQL) {}
  virtual ~QueryGetPrintOptionHistorical() {}
  virtual const char* getQueryName() const override;

  void findPrintOption(std::vector<PrintOption*>& printOptions,
                       const VendorCode& vendor,
                       const std::string& fareSource,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

