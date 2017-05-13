//----------------------------------------------------------------------------
//  © 2013, Sabre Inc. All rights reserved. This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#ifndef QUERY_GET_TAX_REPORTING_RECORD_H
#define QUERY_GET_TAX_REPORTING_RECORD_H

#include "DBAccess/SQLQuery.h"

namespace tse
{

class TaxReportingRecordInfo;
class Logger;

class QueryGetTaxReportingRecord : public SQLQuery
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetTaxReportingRecord(DBAdapter* dbAdapt);

  QueryGetTaxReportingRecord(DBAdapter* dbAdapt, const std::string& sqlStatement);

  virtual ~QueryGetTaxReportingRecord();

  virtual const char* getQueryName() const override;

  void findTaxReportingRecordInfo(std::vector<const TaxReportingRecordInfo*>& taxReportingRecords,
                                  const VendorCode& vendor,
                                  const NationCode& nation,
                                  const CarrierCode& taxCarrier,
                                  const TaxCode& taxCode,
                                  const TaxType& taxType);

  static void initialize();

  const QueryGetTaxReportingRecord& operator=(const QueryGetTaxReportingRecord& another);

  const QueryGetTaxReportingRecord& operator=(const std::string& another);

private:
  static void deinitialize() { _isInitialized = false; }

  static Logger _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

}; // class QueryGetTaxReportingRecord

class QueryGetTaxReportingRecordHistorical : public QueryGetTaxReportingRecord
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetTaxReportingRecordHistorical(DBAdapter* dbAdapt);
  virtual ~QueryGetTaxReportingRecordHistorical();

  virtual const char* getQueryName() const override;

  void findTaxReportingRecordInfo(std::vector<const TaxReportingRecordInfo*>& taxReportingRecords,
                                  const VendorCode& vendor,
                                  const NationCode& nation,
                                  const CarrierCode& taxCarrier,
                                  const TaxCode& taxCode,
                                  const TaxType& taxType,
                                  const DateTime& startDate,
                                  const DateTime& endDate);

  static void initialize();

private:
  static void deinitialize() { _isInitialized = false; }

  static Logger _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxReportingRecordHistorical

// BY-CODE variant

class QueryGetTaxReportingRecordByCode : public SQLQuery
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetTaxReportingRecordByCode(DBAdapter* dbAdapt);

  QueryGetTaxReportingRecordByCode(DBAdapter* dbAdapt, const std::string& sqlStatement);

  virtual ~QueryGetTaxReportingRecordByCode();

  virtual const char* getQueryName() const override;

  void findTaxReportingRecordInfo(std::vector<const TaxReportingRecordInfo*>& taxReportingRecords,
                                  const TaxCode& taxCode);

  static void initialize();

  const QueryGetTaxReportingRecordByCode& operator=(const QueryGetTaxReportingRecordByCode& another);

  const QueryGetTaxReportingRecordByCode& operator=(const std::string& another);

private:
  static void deinitialize() { _isInitialized = false; }

  static Logger _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

}; // class QueryGetTaxReportingRecordByCode

class QueryGetTaxReportingRecordByCodeHistorical : public QueryGetTaxReportingRecordByCode
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetTaxReportingRecordByCodeHistorical(DBAdapter* dbAdapt);
  virtual ~QueryGetTaxReportingRecordByCodeHistorical();

  virtual const char* getQueryName() const override;

  void findTaxReportingRecordInfo(std::vector<const TaxReportingRecordInfo*>& taxReportingRecords,
                                  const TaxCode& taxCode);

  static void initialize();

private:
  static void deinitialize() { _isInitialized = false; }

  static Logger _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxReportingRecordByCodeHistorical


} // namespace tse

#endif // QUERY_GET_TAX_REPORTING_RECORD_H
