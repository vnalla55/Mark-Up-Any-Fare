//----------------------------------------------------------------------------
//  File:         QueryGetTaxRulesRecord.cpp
//  Description:  QueryGetTaxRulesRecord
//  Created:      03/06/2013
//  Authors:      Ram Papineni
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetTaxRulesRecord.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxRulesRecordSQLStatement.h"
#include "DBAccess/TaxRulesRecord.h"

namespace tse
{

std::string QueryGetTaxRulesRecord::_baseSQL;
bool QueryGetTaxRulesRecord::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRulesRecord> g_GetTaxRulesRecord;

QueryGetTaxRulesRecord::QueryGetTaxRulesRecord(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};

QueryGetTaxRulesRecord::QueryGetTaxRulesRecord(DBAdapter* dbAdapt, const std::string& sqlStatement)
  : SQLQuery(dbAdapt, sqlStatement) {};

QueryGetTaxRulesRecord::~QueryGetTaxRulesRecord() {};

const QueryGetTaxRulesRecord&
QueryGetTaxRulesRecord::
operator=(const QueryGetTaxRulesRecord& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
}

const QueryGetTaxRulesRecord&
QueryGetTaxRulesRecord::
operator=(const std::string& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
};

const char*
QueryGetTaxRulesRecord::getQueryName() const
{
  return "GETTAXRULESRECORD";
}

void
QueryGetTaxRulesRecord::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRulesRecordSQLStatement<QueryGetTaxRulesRecord> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRULESRECORD");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRulesRecord::getTaxRulesRecord(std::vector<const TaxRulesRecord*>& ruleRecs,
                                          const NationCode& nation,
                                          const Indicator& taxPointTag)
{
  static tse::Logger logger("atseintl.DBAccess.SQLQuery.GetTaxRulesRecord");

  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substParm(2, taxPointTag);
  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    ruleRecs.push_back(
        QueryGetTaxRulesRecordSQLStatement<QueryGetTaxRulesRecord>::mapRowToRulesRecord(row));
  }

  LOG4CXX_INFO(logger,
               "GETTAXRULESRECORD: NumRows: " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
} // getTaxRulesRecord()

///////////////////////////////////////////////////////////
//  QueryGetTaxRulesRecordHistorical
///////////////////////////////////////////////////////////
std::string QueryGetTaxRulesRecordHistorical::_baseSQL;
bool QueryGetTaxRulesRecordHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRulesRecordHistorical> g_GetTaxRulesRecordHistorical;

QueryGetTaxRulesRecordHistorical::QueryGetTaxRulesRecordHistorical(DBAdapter* dbAdapt)
  : QueryGetTaxRulesRecord(dbAdapt, _baseSQL) {};

QueryGetTaxRulesRecordHistorical::~QueryGetTaxRulesRecordHistorical() {};

const char*
QueryGetTaxRulesRecordHistorical::getQueryName() const
{
  return "GETTAXRULESRECORDHISTORICAL";
}

void
QueryGetTaxRulesRecordHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRulesRecordHistoricalSQLStatement<QueryGetTaxRulesRecordHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRULESRECORDHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRulesRecordHistorical::getTaxRulesRecord(
    std::vector<const TaxRulesRecord*>& ruleRecs,
    const NationCode& nation,
    const Indicator& taxPointTag,
    const DateTime& startDate,
    const DateTime& endDate)
{
  static tse::Logger logger("atseintl.DBAccess.SQLQuery.GetTaxRulesRecordHistorical");
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substParm(2, taxPointTag);
  substParm(3, startDate);
  substParm(4, endDate);

  LOG4CXX_EXECUTECODE_INFO(logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    ruleRecs.push_back(
        QueryGetTaxRulesRecordSQLStatement<QueryGetTaxRulesRecordHistorical>::mapRowToRulesRecord(
            row));
  }
  LOG4CXX_INFO(logger,
               "GETTAXRULESRECORDHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // getTaxRulesRecord()

///////////////////////
//////////////////////////
// BY-CODE versions
/////////////////////
///////////////////
std::string QueryGetTaxRulesRecordByCode::_baseSQL;
bool QueryGetTaxRulesRecordByCode::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRulesRecordByCode> g_GetTaxRulesRecordByCode;

QueryGetTaxRulesRecordByCode::QueryGetTaxRulesRecordByCode(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};

QueryGetTaxRulesRecordByCode::QueryGetTaxRulesRecordByCode(DBAdapter* dbAdapt, const std::string& sqlStatement)
  : SQLQuery(dbAdapt, sqlStatement) {};

QueryGetTaxRulesRecordByCode::~QueryGetTaxRulesRecordByCode() {};

const QueryGetTaxRulesRecordByCode&
QueryGetTaxRulesRecordByCode::
operator=(const QueryGetTaxRulesRecordByCode& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
}

const QueryGetTaxRulesRecordByCode&
QueryGetTaxRulesRecordByCode::
operator=(const std::string& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
};

const char*
QueryGetTaxRulesRecordByCode::getQueryName() const
{
  return "GETTAXRULESRECORDBYCODE";
}

void
QueryGetTaxRulesRecordByCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRulesRecordByCodeSQLStatement<QueryGetTaxRulesRecordByCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRULESRECORDBYCODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRulesRecordByCode::getTaxRulesRecord(std::vector<const TaxRulesRecord*>& ruleRecs,
                                                const TaxCode& taxCode)
{
  static tse::Logger logger("atseintl.DBAccess.SQLQuery.GetTaxRulesRecordByCode");

  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, taxCode);
  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    ruleRecs.push_back(
        QueryGetTaxRulesRecordByCodeSQLStatement<QueryGetTaxRulesRecordByCode>::mapRowToRulesRecord(row));
  }

  LOG4CXX_INFO(logger,
               "GETTAXRULESRECORDBYCODE: NumRows: " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
} // getTaxRulesRecord()

///////////////////////////////////////////////////////////
//  QueryGetTaxRulesRecordHistorical
///////////////////////////////////////////////////////////

std::string QueryGetTaxRulesRecordByCodeHistorical::_baseSQL;
bool QueryGetTaxRulesRecordByCodeHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRulesRecordByCodeHistorical> g_GetTaxRulesRecordByCodeHistorical;

QueryGetTaxRulesRecordByCodeHistorical::QueryGetTaxRulesRecordByCodeHistorical(DBAdapter* dbAdapt)
  : QueryGetTaxRulesRecordByCode(dbAdapt, _baseSQL) {};

QueryGetTaxRulesRecordByCodeHistorical::~QueryGetTaxRulesRecordByCodeHistorical() {};

const char*
QueryGetTaxRulesRecordByCodeHistorical::getQueryName() const
{
  return "GETTAXRULESRECORDBYCODEHISTORICAL";
}

void
QueryGetTaxRulesRecordByCodeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRulesRecordByCodeHistoricalSQLStatement<QueryGetTaxRulesRecordByCodeHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRULESRECORDBYCODEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRulesRecordByCodeHistorical::getTaxRulesRecord(
    std::vector<const TaxRulesRecord*>& ruleRecs,
    const TaxCode& taxCode,
    const DateTime& startDate,
    const DateTime& endDate)
{
  static tse::Logger logger("atseintl.DBAccess.SQLQuery.GetTaxRulesRecordByCodeHistorical");
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, taxCode);
  substParm(2, startDate);
  substParm(3, endDate);

  LOG4CXX_EXECUTECODE_INFO(logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    ruleRecs.push_back(
        QueryGetTaxRulesRecordByCodeSQLStatement<QueryGetTaxRulesRecordByCodeHistorical>::mapRowToRulesRecord(
            row));
  }
  LOG4CXX_INFO(logger,
               "GETTAXRULESRECORDBYCODEHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // getTaxRulesRecord()

///////////////////////////////////////////////////////////
// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
//  QueryGetTaxRulesRecordByCodeAndType
///////////////////////////////////////////////////////////

std::string QueryGetTaxRulesRecordByCodeAndType::_baseSQL;
bool QueryGetTaxRulesRecordByCodeAndType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRulesRecordByCodeAndType> g_GetTaxRulesRecordByCodeAndType;

QueryGetTaxRulesRecordByCodeAndType::QueryGetTaxRulesRecordByCodeAndType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};

QueryGetTaxRulesRecordByCodeAndType::QueryGetTaxRulesRecordByCodeAndType(DBAdapter* dbAdapt, const std::string& sqlStatement)
  : SQLQuery(dbAdapt, sqlStatement) {};

QueryGetTaxRulesRecordByCodeAndType::~QueryGetTaxRulesRecordByCodeAndType() {};

const QueryGetTaxRulesRecordByCodeAndType&
QueryGetTaxRulesRecordByCodeAndType::
operator=(const QueryGetTaxRulesRecordByCodeAndType& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
}

const QueryGetTaxRulesRecordByCodeAndType&
QueryGetTaxRulesRecordByCodeAndType::
operator=(const std::string& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
};

const char*
QueryGetTaxRulesRecordByCodeAndType::getQueryName() const
{
  return "GETTAXRULESRECORDBYCODEANDTYPE";
}

void
QueryGetTaxRulesRecordByCodeAndType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRulesRecordByCodeAndTypeSQLStatement<QueryGetTaxRulesRecordByCodeAndType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRULESRECORDBYCODEANDTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRulesRecordByCodeAndType::getTaxRulesRecord(std::vector<const TaxRulesRecord*>& ruleRecs,
                                                       const TaxCode& taxCode,
                                                       const TaxType& taxType)
{
  static tse::Logger logger("atseintl.DBAccess.SQLQuery.GetTaxRulesRecordByCode");

  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, taxCode);
  substParm(2, taxType);
  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    ruleRecs.push_back(
        QueryGetTaxRulesRecordByCodeAndTypeSQLStatement<QueryGetTaxRulesRecordByCodeAndType>::mapRowToRulesRecord(row));
  }

  LOG4CXX_INFO(logger,
               "GETTAXRULESRECORDBYCODE: NumRows: " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
} // getTaxRulesRecord()

///////////////////////////////////////////////////////////
// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
//  QueryGetTaxRulesRecordByCodeAndTypeHistorical
///////////////////////////////////////////////////////////

std::string QueryGetTaxRulesRecordByCodeAndTypeHistorical::_baseSQL;
bool QueryGetTaxRulesRecordByCodeAndTypeHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRulesRecordByCodeAndTypeHistorical> g_GetTaxRulesRecordByCodeAndTypeHistorical;

QueryGetTaxRulesRecordByCodeAndTypeHistorical::QueryGetTaxRulesRecordByCodeAndTypeHistorical(DBAdapter* dbAdapt)
  : QueryGetTaxRulesRecordByCodeAndType(dbAdapt, _baseSQL) {};

QueryGetTaxRulesRecordByCodeAndTypeHistorical::~QueryGetTaxRulesRecordByCodeAndTypeHistorical() {};

const char*
QueryGetTaxRulesRecordByCodeAndTypeHistorical::getQueryName() const
{
  return "GETTAXRULESRECORDBYCODEANDTYPEHISTORICAL";
}

void
QueryGetTaxRulesRecordByCodeAndTypeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRulesRecordByCodeAndTypeHistoricalSQLStatement<QueryGetTaxRulesRecordByCodeAndTypeHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRULESRECORDBYCODEANDTYPEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRulesRecordByCodeAndTypeHistorical::getTaxRulesRecord(
    std::vector<const TaxRulesRecord*>& ruleRecs,
    const TaxCode& taxCode,
    const TaxType& taxType,
    const DateTime& startDate,
    const DateTime& endDate)
{
  static tse::Logger logger("atseintl.DBAccess.SQLQuery.GetTaxRulesRecordByCodeHistorical");
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, taxCode);
  substParm(2, taxType);
  substParm(3, startDate);
  substParm(4, endDate);

  LOG4CXX_EXECUTECODE_INFO(logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    ruleRecs.push_back(
        QueryGetTaxRulesRecordByCodeAndTypeSQLStatement<QueryGetTaxRulesRecordByCodeAndTypeHistorical>::mapRowToRulesRecord(
            row));
  }
  LOG4CXX_INFO(logger,
               "GETTAXRULESRECORDBYCODEANDTYPEHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // getTaxRulesRecord()


///////////////////////////////////////////////////////////
//  QueryGetAllTaxRulesRecords
///////////////////////////////////////////////////////////
std::string QueryGetAllTaxRulesRecords::_baseSQL;
bool QueryGetAllTaxRulesRecords::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTaxRulesRecords> g_GetAllTaxRulesRecord;

const char*
QueryGetAllTaxRulesRecords::getQueryName() const
{
  return "GETALLTAXRULESRECORD";
}

void
QueryGetAllTaxRulesRecords::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTaxRulesRecordSQLStatement<QueryGetAllTaxRulesRecords> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTAXRULESRECORD");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
tse::QueryGetAllTaxRulesRecords::getAllTaxRulesRecord(std::vector<TaxRulesRecord*>& ruleRecs)
{
  static tse::Logger logger("atseintl.DBAccess.SQLQuery.GetAllTaxRulesRecord");
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    ruleRecs.push_back(
        QueryGetTaxRulesRecordSQLStatement<QueryGetAllTaxRulesRecords>::mapRowToRulesRecord(row));
  }

  LOG4CXX_INFO(logger,
               "GETALLTAXRULESRECORD: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

QueryGetAllTaxRulesRecords::QueryGetAllTaxRulesRecords(DBAdapter* dbAdapt)
  : QueryGetTaxRulesRecord(dbAdapt, _baseSQL) {};

QueryGetAllTaxRulesRecords::~QueryGetAllTaxRulesRecords() {};

void
QueryGetAllTaxRulesRecords::execute(std::vector<TaxRulesRecord*>& ruleRecs)
{
  getAllTaxRulesRecord(ruleRecs);
}

} // tse
