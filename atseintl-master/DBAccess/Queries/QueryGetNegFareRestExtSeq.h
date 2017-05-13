//----------------------------------------------------------------------------
//          File:           QueryGetNegFareRestExtSeq.h
//          Description:    QueryGetNegFareRestExtSeq
//          Created:        9/9/2010
// Authors:         Artur Krezel
//
//          Updates:
//
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetNegFareRestExtSeq : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNegFareRestExtSeq(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetNegFareRestExtSeq(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetNegFareRestExtSeq() {};
  virtual const char* getQueryName() const override;

  void findNegFareRestExtSeq(std::vector<tse::NegFareRestExtSeq*>& negFareRestExtSeqs,
                             const VendorCode& vendor,
                             int itemNo);

  static void initialize();

  const QueryGetNegFareRestExtSeq& operator=(const QueryGetNegFareRestExtSeq& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetNegFareRestExtSeq& operator=(const std::string& Another)
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
}; // class QueryGetNegFareRestExtSeq

class QueryGetNegFareRestExtSeqHistorical : public QueryGetNegFareRestExtSeq
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNegFareRestExtSeqHistorical(DBAdapter* dbAdapt)
    : QueryGetNegFareRestExtSeq(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetNegFareRestExtSeqHistorical() {}
  virtual const char* getQueryName() const override;

  void findNegFareRestExtSeq(std::vector<tse::NegFareRestExtSeq*>& negFareRestExtSeqs,
                             const VendorCode& vendor,
                             int itemNo,
                             const DateTime& startDate,
                             const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetNegFareRestExtSeqHistorical
} // namespace tse

