//----------------------------------------------------------------------------
//      File:         QueryGetTaxRulesRecord.h
//      Description:  QueryGetTaxRulesRecord
//      Created:      03/06/2013
//      Authors:      Ram Papineni
//
//      Updates:
//
//   © 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#ifndef GET_TAX_RULES_RECORD_H
#define GET_TAX_RULES_RECORD_H

#include "DBAccess/SQLQuery.h"

namespace tse
{

class TaxRulesRecord;

class QueryGetTaxRulesRecord : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRulesRecord(DBAdapter* dbAdapt);
  QueryGetTaxRulesRecord(DBAdapter* dbAdapt, const std::string& sqlStatement);
  virtual ~QueryGetTaxRulesRecord();
  virtual const char* getQueryName() const override;

  void getTaxRulesRecord(std::vector<const TaxRulesRecord*>& ruleRecs,
                         const NationCode& nation,
                         const Indicator& taxPointTag);

  static void initialize();

  const QueryGetTaxRulesRecord& operator=(const QueryGetTaxRulesRecord& rhs);
  const QueryGetTaxRulesRecord& operator=(const std::string& rhs);

private:
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxRulesRecord

class QueryGetTaxRulesRecordHistorical : public QueryGetTaxRulesRecord
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRulesRecordHistorical(DBAdapter* dbAdapt);
  virtual ~QueryGetTaxRulesRecordHistorical();
  virtual const char* getQueryName() const override;

  void getTaxRulesRecord(std::vector<const TaxRulesRecord*>& ruleRecs,
                         const NationCode& nation,
                         const Indicator& taxPointTag,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxRulesRecordHistorical

/////////////////////

class QueryGetTaxRulesRecordByCode : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRulesRecordByCode(DBAdapter* dbAdapt);
  QueryGetTaxRulesRecordByCode(DBAdapter* dbAdapt, const std::string& sqlStatement);
  virtual ~QueryGetTaxRulesRecordByCode();
  virtual const char* getQueryName() const override;

  void getTaxRulesRecord(std::vector<const TaxRulesRecord*>& ruleRecs,
                         const TaxCode& taxCode);

  static void initialize();

  const QueryGetTaxRulesRecordByCode& operator=(const QueryGetTaxRulesRecordByCode& rhs);
  const QueryGetTaxRulesRecordByCode& operator=(const std::string& rhs);

private:
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxRulesRecordByCode

class QueryGetTaxRulesRecordByCodeHistorical : public QueryGetTaxRulesRecordByCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRulesRecordByCodeHistorical(DBAdapter* dbAdapt);
  virtual ~QueryGetTaxRulesRecordByCodeHistorical();
  virtual const char* getQueryName() const override;

  void getTaxRulesRecord(std::vector<const TaxRulesRecord*>& ruleRecs,
                         const TaxCode& taxCode,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxRulesRecordByCodeHistorical

/////////////////////

// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
class QueryGetTaxRulesRecordByCodeAndType : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRulesRecordByCodeAndType(DBAdapter* dbAdapt);
  QueryGetTaxRulesRecordByCodeAndType(DBAdapter* dbAdapt, const std::string& sqlStatement);
  virtual ~QueryGetTaxRulesRecordByCodeAndType();
  virtual const char* getQueryName() const override;

  void getTaxRulesRecord(std::vector<const TaxRulesRecord*>& ruleRecs,
                         const TaxCode& taxCode,
                         const TaxType& taxType);

  static void initialize();

  const QueryGetTaxRulesRecordByCodeAndType& operator=(const QueryGetTaxRulesRecordByCodeAndType& rhs);
  const QueryGetTaxRulesRecordByCodeAndType& operator=(const std::string& rhs);

private:
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxRulesRecordByCodeAndType

// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
class QueryGetTaxRulesRecordByCodeAndTypeHistorical : public QueryGetTaxRulesRecordByCodeAndType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRulesRecordByCodeAndTypeHistorical(DBAdapter* dbAdapt);
  virtual ~QueryGetTaxRulesRecordByCodeAndTypeHistorical();
  virtual const char* getQueryName() const override;

  void getTaxRulesRecord(std::vector<const TaxRulesRecord*>& ruleRecs,
                         const TaxCode& taxCode,
                         const TaxType& taxType,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxRulesRecordByCodeAndTypeHistorical

//////////////////////////////

class QueryGetAllTaxRulesRecords : public QueryGetTaxRulesRecord
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTaxRulesRecords(DBAdapter* dbAdapt);
  virtual ~QueryGetAllTaxRulesRecords();
  virtual const char* getQueryName() const override;

  void execute(std::vector<TaxRulesRecord*>& ruleRecs);

  void getAllTaxRulesRecord(std::vector<TaxRulesRecord*>& ruleRecs);

  static void initialize();

private:
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTaxRulesRecords

} // namespace tse

#endif // GETTAXRULESRECORD_H

