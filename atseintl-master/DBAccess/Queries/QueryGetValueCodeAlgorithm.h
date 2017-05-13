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
class ValueCodeAlgorithm;

class QueryGetValueCodeAlgorithm : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetValueCodeAlgorithm(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetValueCodeAlgorithm(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetValueCodeAlgorithm() {}

  virtual const char* getQueryName() const override;

  void findValueCodeAlgorithm(std::vector<ValueCodeAlgorithm*>& valueCodeAlgorithms,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const std::string& name);

  static void initialize();

  const QueryGetValueCodeAlgorithm& operator=(const QueryGetValueCodeAlgorithm& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetValueCodeAlgorithm& operator=(const std::string& Another)
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

class QueryGetValueCodeAlgorithmHistorical : public QueryGetValueCodeAlgorithm
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetValueCodeAlgorithmHistorical(DBAdapter* dbAdapt)
    : QueryGetValueCodeAlgorithm(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetValueCodeAlgorithmHistorical() {}
  virtual const char* getQueryName() const override;

  void findValueCodeAlgorithm(std::vector<ValueCodeAlgorithm*>& valueCodeAlgorithms,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const std::string& name,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

