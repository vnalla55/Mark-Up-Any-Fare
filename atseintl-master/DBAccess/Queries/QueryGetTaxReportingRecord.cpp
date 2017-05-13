//----------------------------------------------------------------------------
// � 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetTaxReportingRecord.h"

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxReportingRecordSQLStatement.h"
#include "DBAccess/TaxReportingRecordInfo.h"

namespace tse
{

Logger QueryGetTaxReportingRecord::_logger("atseintl.DBAccess.SQLQuery.GetTaxReportingRecord");

std::string QueryGetTaxReportingRecord::_baseSQL;
bool QueryGetTaxReportingRecord::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxReportingRecord> g_GetTaxReportingRecord;

QueryGetTaxReportingRecord::QueryGetTaxReportingRecord(DBAdapter* dbAdapt)
  : SQLQuery(dbAdapt, _baseSQL) {};

QueryGetTaxReportingRecord::QueryGetTaxReportingRecord(DBAdapter* dbAdapt,
                                                       const std::string& sqlStatement)
  : SQLQuery(dbAdapt, sqlStatement) {};

QueryGetTaxReportingRecord::~QueryGetTaxReportingRecord() {};

const QueryGetTaxReportingRecord&
QueryGetTaxReportingRecord::
operator=(const QueryGetTaxReportingRecord& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
}

const QueryGetTaxReportingRecord&
QueryGetTaxReportingRecord::
operator=(const std::string& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
};

void
QueryGetTaxReportingRecord::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxReportingRecordSQLStatement<QueryGetTaxReportingRecord> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXREPORTINGRECORD");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

const char*
QueryGetTaxReportingRecord::getQueryName() const
{
  return "GETTAXREPORTINGRECORD";
}

void
QueryGetTaxReportingRecord::findTaxReportingRecordInfo(
    std::vector<const TaxReportingRecordInfo*>& paxTypeCodeVector,
    const VendorCode& vendor,
    const NationCode& nation,
    const CarrierCode& taxCarrier,
    const TaxCode& taxCode,
    const TaxType& taxType)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(nation, 2);
  substParm(taxCarrier, 3);
  substParm(taxCode, 4);
  substParm(taxType, 5);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    paxTypeCodeVector.push_back(QueryGetTaxReportingRecordSQLStatement<
        QueryGetTaxReportingRecord>::mapRowToTaxReportingRecordInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

Logger QueryGetTaxReportingRecordHistorical::
  _logger("atseintl.DBAccess.SQLQuery.QueryGetTaxReportingRecordHistorical");

std::string QueryGetTaxReportingRecordHistorical::_baseSQL;
bool QueryGetTaxReportingRecordHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxReportingRecordHistorical> g_GetTaxReportingRecordHistorical;

QueryGetTaxReportingRecordHistorical::QueryGetTaxReportingRecordHistorical(DBAdapter* dbAdapt)
  : QueryGetTaxReportingRecord(dbAdapt, _baseSQL)
{
}

QueryGetTaxReportingRecordHistorical::~QueryGetTaxReportingRecordHistorical() {}

const char*
QueryGetTaxReportingRecordHistorical::getQueryName() const
{
  return "GETTAXREPORTINGRECORDHISTORICAL";
}

void
QueryGetTaxReportingRecordHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxReportingRecordHistoricalSQLStatement<QueryGetTaxReportingRecordHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXREPORTINGRECORDHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetTaxReportingRecordHistorical::findTaxReportingRecordInfo(
    std::vector<const TaxReportingRecordInfo*>& paxTypeCode,
    const VendorCode& vendor,
    const NationCode& nation,
    const CarrierCode& taxCarrier,
    const TaxCode& taxCode,
    const TaxType& taxType,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  substParm(1, vendor);
  substParm(2, nation);
  substParm(3, taxCarrier);
  substParm(4, taxCode);
  substParm(5, taxType);
  substParm(6, startDate);
  substParm(7, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    paxTypeCode.push_back(QueryGetTaxReportingRecordHistoricalSQLStatement<
        QueryGetTaxReportingRecordHistorical>::mapRowToTaxReportingRecordInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

// BY-CODE version
Logger QueryGetTaxReportingRecordByCode::_logger("atseintl.DBAccess.SQLQuery.GetTaxReportingRecordByCode");

std::string QueryGetTaxReportingRecordByCode::_baseSQL;
bool QueryGetTaxReportingRecordByCode::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxReportingRecordByCode> g_GetTaxReportingRecordByCode;

QueryGetTaxReportingRecordByCode::QueryGetTaxReportingRecordByCode(DBAdapter* dbAdapt)
  : SQLQuery(dbAdapt, _baseSQL) {};

QueryGetTaxReportingRecordByCode::QueryGetTaxReportingRecordByCode(DBAdapter* dbAdapt,
                                                       const std::string& sqlStatement)
  : SQLQuery(dbAdapt, sqlStatement) {};

QueryGetTaxReportingRecordByCode::~QueryGetTaxReportingRecordByCode() {};

const QueryGetTaxReportingRecordByCode&
QueryGetTaxReportingRecordByCode::
operator=(const QueryGetTaxReportingRecordByCode& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
}

const QueryGetTaxReportingRecordByCode&
QueryGetTaxReportingRecordByCode::
operator=(const std::string& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
};

void
QueryGetTaxReportingRecordByCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxReportingRecordByCodeSQLStatement<QueryGetTaxReportingRecordByCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXREPORTINGRECORDBYCODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

const char*
QueryGetTaxReportingRecordByCode::getQueryName() const
{
  return "GETTAXREPORTINGRECORDBYCODE";
}

void
QueryGetTaxReportingRecordByCode::findTaxReportingRecordInfo(
    std::vector<const TaxReportingRecordInfo*>& paxTypeCodeVector,
    const TaxCode& taxCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(taxCode, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    paxTypeCodeVector.push_back(QueryGetTaxReportingRecordByCodeSQLStatement<
        QueryGetTaxReportingRecordByCode>::mapRowToTaxReportingRecordInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

Logger QueryGetTaxReportingRecordByCodeHistorical::
  _logger("atseintl.DBAccess.SQLQuery.QueryGetTaxReportingRecordByCodeHistorical");

std::string QueryGetTaxReportingRecordByCodeHistorical::_baseSQL;
bool QueryGetTaxReportingRecordByCodeHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxReportingRecordByCodeHistorical> g_GetTaxReportingRecordByCodeHistorical;

QueryGetTaxReportingRecordByCodeHistorical::QueryGetTaxReportingRecordByCodeHistorical(DBAdapter* dbAdapt)
  : QueryGetTaxReportingRecordByCode(dbAdapt, _baseSQL)
{
}

QueryGetTaxReportingRecordByCodeHistorical::~QueryGetTaxReportingRecordByCodeHistorical() {}

const char*
QueryGetTaxReportingRecordByCodeHistorical::getQueryName() const
{
  return "GETTAXREPORTINGRECORDBYCODEHISTORICAL";
}

void
QueryGetTaxReportingRecordByCodeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxReportingRecordByCodeHistoricalSQLStatement<QueryGetTaxReportingRecordByCodeHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXREPORTINGRECORDBYCODEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetTaxReportingRecordByCodeHistorical::findTaxReportingRecordInfo(
    std::vector<const TaxReportingRecordInfo*>& paxTypeCode,
    const TaxCode& taxCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  substParm(1, taxCode);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    paxTypeCode.push_back(QueryGetTaxReportingRecordByCodeHistoricalSQLStatement<
        QueryGetTaxReportingRecordByCodeHistorical>::mapRowToTaxReportingRecordInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

}
