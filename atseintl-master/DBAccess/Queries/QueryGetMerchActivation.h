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
class MerchActivationInfo;

class QueryGetMerchActivation : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMerchActivation(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetMerchActivation(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetMerchActivation() {}

  virtual const char* getQueryName() const override;

  void findMerchActivationInfo(std::vector<MerchActivationInfo*>& merchActivations,
                               int productId,
                               const CarrierCode& carrier,
                               const PseudoCityCode& pseudoCity);

  static void initialize();

  const QueryGetMerchActivation& operator=(const QueryGetMerchActivation& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetMerchActivation& operator=(const std::string& Another)
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

class QueryGetMerchActivationHistorical : public QueryGetMerchActivation
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMerchActivationHistorical(DBAdapter* dbAdapt) : QueryGetMerchActivation(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetMerchActivationHistorical() {}
  virtual const char* getQueryName() const override;

  void findMerchActivationInfo(std::vector<MerchActivationInfo*>& merchActivations,
                               int productId,
                               const CarrierCode& carrier,
                               const PseudoCityCode& pseudoCity,
                               const DateTime& startDate,
                               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

