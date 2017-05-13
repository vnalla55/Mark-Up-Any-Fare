//----------------------------------------------------------------------------
//  File:           QueryGetTaxCode.cpp
//  Description:    QueryGetTaxCode
//  Created:        8/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetTaxCode.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxCodeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxCode::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode"));
std::string QueryGetTaxCode::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxCodeHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCodeHistorical"));
std::string QueryGetTaxCodeHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxCodeGenText::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxCodeGenText"));
std::string QueryGetTaxCodeGenText::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxCodeGenTextHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxCodeGenTextHistorical"));
std::string QueryGetTaxCodeGenTextHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrValCxr::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrValCxr"));
std::string QueryGetTaxRestrValCxr::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrValCxrHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrValCxrHistorical"));
std::string QueryGetTaxRestrValCxrHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrPsgr::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrPsgr"));
std::string QueryGetTaxRestrPsgr::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrPsgrHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrPsgrHistorical"));
std::string QueryGetTaxRestrPsgrHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrFareType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrFareType"));
std::string QueryGetTaxRestrFareType::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrFareTypeHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrFareTypeHistorical"));
std::string QueryGetTaxRestrFareTypeHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrFareClass::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrFareClass"));
std::string QueryGetTaxRestrFareClass::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrFareClassHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrFareClassHistorical"));
std::string QueryGetTaxRestrFareClassHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxExempEquip::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxExempEquip"));
std::string QueryGetTaxExempEquip::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxExempEquipHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxExempEquipHistorical"));
std::string QueryGetTaxExempEquipHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxExempCxr::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxExempCxr"));
std::string QueryGetTaxExempCxr::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxExempCxrHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxExempCxrHistorical"));
std::string QueryGetTaxExempCxrHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxOnTax::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxOnTax"));
std::string QueryGetTaxOnTax::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxOnTaxHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxOnTaxHistorical"));
std::string QueryGetTaxOnTaxHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrTransit::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrTransit"));
std::string QueryGetTaxRestrTransit::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrTransitHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrTransitHistorical"));
std::string QueryGetTaxRestrTransitHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrTktDsg::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrTktDsg"));
std::string QueryGetTaxRestrTktDsg::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxRestrTktDsgHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxRestrTktDsgHistorical"));
std::string QueryGetTaxRestrTktDsgHistorical::_baseSQL;

log4cxx::LoggerPtr
QueryGetAllTaxCodeRegsHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetTaxCode.GetAllTaxCodeRegsHistorical"));
std::string QueryGetAllTaxCodeRegsHistorical::_baseSQL;

// QueryGetTaxCodeGenTextSeqSQLStatement
log4cxx::LoggerPtr
QueryGetTaxCodeGenTextSeq::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCode.GetTaxCodeGenTextSeq"));
std::string QueryGetTaxCodeGenTextSeq::_baseSQL;

// QueryGetTaxCodeGenTextSeqSQLStatement historical
log4cxx::LoggerPtr
QueryGetTaxCodeGenTextSeqHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetTaxCode.QueryGetTaxCodeGenTextSeqHistorical"));
std::string QueryGetTaxCodeGenTextSeqHistorical::_baseSQL;

bool QueryGetTaxCode::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCode> g_GetTaxCode;

const char*
QueryGetTaxCode::getQueryName() const
{
  return "GETTAXCODE";
};

void
QueryGetTaxCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCodeSQLStatement<QueryGetTaxCode> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXCODE");
    substTableDef(&_baseSQL);

    QueryGetTaxCodeGenText::initialize();
    QueryGetTaxCodeGenTextSeq::initialize();
    QueryGetTaxRestrValCxr::initialize();
    QueryGetTaxRestrPsgr::initialize();
    QueryGetTaxRestrFareType::initialize();
    QueryGetTaxRestrFareClass::initialize();
    QueryGetTaxExempEquip::initialize();
    QueryGetTaxExempCxr::initialize();
    QueryGetTaxOnTax::initialize();
    QueryGetTaxRestrTransit::initialize();
    QueryGetTaxRestrTktDsg::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxCode::findTaxCodeReg(std::vector<tse::TaxCodeReg*>& taxC, const TaxCode& code)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  std::vector<TaxCodeGenText*> texts;
  QueryGetTaxCodeGenText SQLTaxCodeGenText(_dbAdapt);
  SQLTaxCodeGenText.findTaxCodeGenText(texts, code);

  substParm(code, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxCodeReg* tcr = nullptr;
  tse::TaxCodeReg* tcrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    tcr = QueryGetTaxCodeSQLStatement<QueryGetTaxCode>::mapRowToTaxCodeReg(row, tcrPrev);
    if (tcr != tcrPrev)
    {
      std::vector<TaxCodeGenText*>::iterator t = texts.begin();
      for (; t != texts.end(); t++)
      {
        tcr->taxCodeGenTexts().push_back(new TaxCodeGenText(**t));
      }
      taxC.push_back(tcr);
    }

    tcrPrev = tcr;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXCODE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                        << stopCPU() << ")");
  res.freeResult();

  // Clean up original Text Children
  std::vector<TaxCodeGenText*>::iterator txtIt = texts.begin();
  for (; txtIt != texts.end(); txtIt++)
  {
    delete *txtIt;
  }
  texts.clear();

  if (taxC.size() == 0)
    return; // no use checking for children if no parent

  buildTaxCodeRegChildren(taxC);
} // QueryGetTaxCode::findTaxCodeReg()

int
QueryGetTaxCode::checkFlightWildCard(const char* fltStr)
{
  if (fltStr[0] == '*')
    return -1;
  else
    return stringToInteger(fltStr, __LINE__); // lint !e668
} // QueryGetTaxCode::checkFlightWildCard()

int
QueryGetTaxCode::stringToInteger(const char* stringVal, int lineNumber)
{
  if (stringVal == nullptr)
  {
    LOG4CXX_FATAL(_logger,
                  "stringToInteger - Null pointer to int data. LineNumber: " << lineNumber);
    throw std::runtime_error("Null pointer to int data");
  }
  else if (*stringVal == '-' || *stringVal == '+')
  {
    if (stringVal[1] < '0' || stringVal[1] > '9')
      return 0;
  }
  else if (*stringVal < '0' || *stringVal > '9')
  {
    return 0;
  }
  return atoi(stringVal);
} // QueryGetTaxCode::stringToInteger()

void
QueryGetTaxCode::buildTaxCodeRegChildren(std::vector<tse::TaxCodeReg*>& vecTaxCodeReg)
{
  TaxCode taxCodePrev = "";
  std::vector<tse::TaxCodeReg*>::iterator taxCodeRegIt = vecTaxCodeReg.begin();
  for (; taxCodeRegIt != vecTaxCodeReg.end(); taxCodeRegIt++)
  { // Main Iteration thru TaxCodeRegs
    if (taxCodePrev == (*taxCodeRegIt)->taxCode())
      continue;

    taxCodePrev = (*taxCodeRegIt)->taxCode();

    QueryGetTaxRestrValCxr SQLTaxRestrValCxr(_dbAdapt);
    SQLTaxRestrValCxr.getRestrValCxrs(taxCodePrev, taxCodeRegIt, vecTaxCodeReg);

    QueryGetTaxRestrPsgr SQLTaxRestrPsgr(_dbAdapt);
    SQLTaxRestrPsgr.getRestrPsgrs(taxCodePrev, taxCodeRegIt, vecTaxCodeReg);

    QueryGetTaxRestrFareType SQLTaxRestrFareType(_dbAdapt);
    SQLTaxRestrFareType.getRestrFareTypes(taxCodePrev, taxCodeRegIt, vecTaxCodeReg);

    QueryGetTaxRestrFareClass SQLTaxRestrFareClass(_dbAdapt);
    SQLTaxRestrFareClass.getRestrFareClasses(taxCodePrev, taxCodeRegIt, vecTaxCodeReg);

    QueryGetTaxExempEquip SQLTaxExempEquip(_dbAdapt);
    SQLTaxExempEquip.getExempEquips(taxCodePrev, taxCodeRegIt, vecTaxCodeReg);

    QueryGetTaxExempCxr SQLTaxExempCxr(_dbAdapt);
    SQLTaxExempCxr.getExempCxrs(taxCodePrev, taxCodeRegIt, vecTaxCodeReg);

    QueryGetTaxOnTax SQLTaxOnTax(_dbAdapt);
    SQLTaxOnTax.getTaxOnTaxes(taxCodePrev, taxCodeRegIt, vecTaxCodeReg);

    QueryGetTaxRestrTransit SQLTaxRestrTransit(_dbAdapt);
    SQLTaxRestrTransit.getRestrTransits(taxCodePrev, taxCodeRegIt, vecTaxCodeReg);

    QueryGetTaxRestrTktDsg SQLTaxRestrTktDsg(_dbAdapt);
    SQLTaxRestrTktDsg.getRestrTktDsgs(taxCodePrev, taxCodeRegIt, vecTaxCodeReg);

    QueryGetTaxCodeGenTextSeq SQLTaxCodeGenTextSeq(_dbAdapt);
    SQLTaxCodeGenTextSeq.getCodeTextSeq(taxCodePrev, taxCodeRegIt, vecTaxCodeReg);

  } // for (Main Iteration thru TaxCodeRegs                                 )
} // QueryGetTaxCode::buildTaxCodeRegChildren()

///////////////////////////////////////////////////////////
//
//  QueryGetAllTaxCodeRegs
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTaxCodeRegs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTaxCodeRegs"));
std::string QueryGetAllTaxCodeRegs::_baseSQL;
bool QueryGetAllTaxCodeRegs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTaxCodeRegs> g_GetAllTaxCodeRegs;

const char*
QueryGetAllTaxCodeRegs::getQueryName() const
{
  return "GETALLTAXCODEREGS";
};

void
QueryGetAllTaxCodeRegs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTaxCodeRegsSQLStatement<QueryGetAllTaxCodeRegs> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTAXCODEREGS");
    substTableDef(&_baseSQL);
    QueryGetTaxCodeGenText::initialize();

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTaxCodeRegs::findAllTaxCodeReg(std::vector<tse::TaxCodeReg*>& vecTaxCodeReg)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxCodeReg* taxCodeReg = nullptr;
  tse::TaxCodeReg* taxCodeRegPrev = nullptr;
  while ((row = res.nextRow()))
  {
    taxCodeRegPrev = taxCodeReg;
    taxCodeReg = QueryGetAllTaxCodeRegsSQLStatement<QueryGetAllTaxCodeRegs>::mapRowToTaxCodeReg(
        row, taxCodeRegPrev);
    if (taxCodeReg == taxCodeRegPrev)
      continue;

    vecTaxCodeReg.push_back(taxCodeReg);
  }
  LOG4CXX_INFO(_logger,
               "GETALLTAXCODEREGS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ") mSecs");
  res.freeResult();

  if (vecTaxCodeReg.empty())
    return;

  QueryGetTaxCodeGenText SQLTaxCodeGenText(_dbAdapt);
  std::vector<TaxCodeGenText*> texts;
  std::vector<TaxCodeGenText*>::iterator txtIt;
  TaxCode taxCode = "";
  std::vector<tse::TaxCodeReg*>::iterator taxCodeRegIt = vecTaxCodeReg.begin();
  for (; taxCodeRegIt != vecTaxCodeReg.end(); taxCodeRegIt++)
  {
    if (taxCode != (*taxCodeRegIt)->taxCode())
    {
      taxCode = (*taxCodeRegIt)->taxCode();
      for (txtIt = texts.begin(); txtIt != texts.end(); txtIt++)
      {
        delete *txtIt;
      }
      texts.clear();
      SQLTaxCodeGenText.findTaxCodeGenText(texts, taxCode);
    }

    for (txtIt = texts.begin(); txtIt != texts.end(); txtIt++)
    {
      (*taxCodeRegIt)->taxCodeGenTexts().push_back(new TaxCodeGenText(**txtIt));
    }
  }
  for (txtIt = texts.begin(); txtIt != texts.end(); txtIt++)
  {
    delete *txtIt;
  }

  buildTaxCodeRegChildren(vecTaxCodeReg);
} // findAllTaxCodeReg()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxCodeGenTextSeq
//
///////////////////////////////////////////////////////////
bool QueryGetTaxCodeGenTextSeq::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCodeGenTextSeq> g_GetTaxCodeGenTextSeq;

const char*
QueryGetTaxCodeGenTextSeq::getQueryName() const
{
  return "GETTAX_CODE_GEN_TEXTSEQ";
}

void
QueryGetTaxCodeGenTextSeq::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCodeGenTextSeqSQLStatement<QueryGetTaxCodeGenTextSeq> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAX_CODE_GEN_TEXTSEQ");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxCodeGenTextSeq::getCodeTextSeq(TaxCode txCd,
                                          std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                          std::vector<tse::TaxCodeReg*>& vecTCR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;

  tse::TaxCodeGenText* TaxText;
  tse::TaxCodeGenText* prevTaxText = nullptr;

  while ((row = res.nextRow()))
  {

    TaxCode taxCode =
        QueryGetTaxCodeGenTextSeqSQLStatement<QueryGetTaxCodeGenTextSeq>::mapRowToTaxCode(row);
    uint64_t versionDate =
        QueryGetTaxCodeGenTextSeqSQLStatement<QueryGetTaxCodeGenTextSeq>::mapRowToVersionDate(row);
    int seqNo =
        QueryGetTaxCodeGenTextSeqSQLStatement<QueryGetTaxCodeGenTextSeq>::mapRowToSeqNo(row);
    DateTime createDate =
        QueryGetTaxCodeGenTextSeqSQLStatement<QueryGetTaxCodeGenTextSeq>::mapRowToCreateDate(row);

    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;

      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;
      // Got a match record
      TaxText =
          QueryGetTaxCodeGenTextSeqSQLStatement<QueryGetTaxCodeGenTextSeq>::mapRowToTaxCodeGenText(
              row, prevTaxText);
      if (TaxText != prevTaxText)
      {
        tcr->taxCodeGenTexts().push_back(TaxText);
      }
      prevTaxText = TaxText;
      break;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETTAX_CODE_GEN_TEXTSEQ: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");

  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetTaxCodeGenText
//
///////////////////////////////////////////////////////////
bool QueryGetTaxCodeGenText::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCodeGenText> g_GetTaxCodeGenText;

const char*
QueryGetTaxCodeGenText::getQueryName() const
{
  return "GETTAX_CODE_GEN_TEXT";
}

void
QueryGetTaxCodeGenText::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCodeGenTextSQLStatement<QueryGetTaxCodeGenText> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAX_CODE_GEN_TEXT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxCodeGenText::findTaxCodeGenText(std::vector<tse::TaxCodeGenText*>& taxTexts,
                                           const TaxCode& taxCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(taxCode, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxCodeGenText* TaxText = nullptr;
  tse::TaxCodeGenText* prevTaxText = nullptr;
  while ((row = res.nextRow()))
  {
    TaxText = QueryGetTaxCodeGenTextSQLStatement<QueryGetTaxCodeGenText>::mapRowToTaxCodeGenText(
        row, prevTaxText);
    if (TaxText != prevTaxText)
      taxTexts.push_back(TaxText);

    prevTaxText = TaxText;
  }
  LOG4CXX_INFO(_logger,
               "GETTAX_CODE_GEN_TEXT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetTaxCodeGenText::findTaxCodeGenText

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrValCxr
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrValCxr::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrValCxr> g_GetTaxRestrValCxr;

const char*
QueryGetTaxRestrValCxr::getQueryName() const
{
  return "GETTAXRESTRVALCXR";
}

void
QueryGetTaxRestrValCxr::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrValCxrSQLStatement<QueryGetTaxRestrValCxr> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRVALCXR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrValCxr::getRestrValCxrs(TaxCode txCd,
                                        std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                        std::vector<tse::TaxCodeReg*>& vecTCR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrValCxrs
    TaxCode taxCode =
        QueryGetTaxRestrValCxrSQLStatement<QueryGetTaxRestrValCxr>::mapRowToTaxCode(row);
    uint64_t versionDate =
        QueryGetTaxRestrValCxrSQLStatement<QueryGetTaxRestrValCxr>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxRestrValCxrSQLStatement<QueryGetTaxRestrValCxr>::mapRowToSeqNo(row);
    DateTime createDate =
        QueryGetTaxRestrValCxrSQLStatement<QueryGetTaxRestrValCxr>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add carrier to this TCR and go on to next carrier
      CarrierCode carrier =
          QueryGetTaxRestrValCxrSQLStatement<QueryGetTaxRestrValCxr>::mapRowToCarrier(row);
      tcr->restrictionValidationCxr().push_back(carrier);
      break;
    } // while (true)
  } // Fetchin RestrValCxrs

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRVALCXR"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrValCxr::getRestrValCxrs()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrPsgr
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrPsgr::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrPsgr> g_GetTaxRestrPsgr;

const char*
QueryGetTaxRestrPsgr::getQueryName() const
{
  return "GETTAXRESTRPSGR";
}

void
QueryGetTaxRestrPsgr::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrPsgrSQLStatement<QueryGetTaxRestrPsgr> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRPSGR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrPsgr::getRestrPsgrs(TaxCode txCd,
                                    std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                    std::vector<tse::TaxCodeReg*>& vecTCR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrPsgrs
    TaxCode taxCode = QueryGetTaxRestrPsgrSQLStatement<QueryGetTaxRestrPsgr>::mapRowToTaxCode(row);
    uint64_t versionDate =
        QueryGetTaxRestrPsgrSQLStatement<QueryGetTaxRestrPsgr>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxRestrPsgrSQLStatement<QueryGetTaxRestrPsgr>::mapRowToSeqNo(row);
    DateTime createDate =
        QueryGetTaxRestrPsgrSQLStatement<QueryGetTaxRestrPsgr>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add psgr to this TCR and go on to next psgr
      TaxRestrictionPsg restrictionPsg;
      QueryGetTaxRestrPsgrSQLStatement<QueryGetTaxRestrPsgr>::mapRowToTaxRestrictionPsg(
          row, restrictionPsg);
      tcr->restrictionPsg().push_back(restrictionPsg);
      break;
    } // while (true)
  } // Fetchin RestrPsgrs

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRPSGR"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrPsgr::getRestrPsgrs()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrFareType
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrFareType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrFareType> g_GetTaxRestrFareType;

const char*
QueryGetTaxRestrFareType::getQueryName() const
{
  return "GETTAXRESTRFARETYPE";
}

void
QueryGetTaxRestrFareType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrFareTypeSQLStatement<QueryGetTaxRestrFareType> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRFARETYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrFareType::getRestrFareTypes(TaxCode txCd,
                                            std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                            std::vector<tse::TaxCodeReg*>& vecTCR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrFareTypes
    TaxCode taxCode =
        QueryGetTaxRestrFareTypeSQLStatement<QueryGetTaxRestrFareType>::mapRowToTaxCode(row);
    uint64_t versionDate =
        QueryGetTaxRestrFareTypeSQLStatement<QueryGetTaxRestrFareType>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxRestrFareTypeSQLStatement<QueryGetTaxRestrFareType>::mapRowToSeqNo(row);
    DateTime createDate =
        QueryGetTaxRestrFareTypeSQLStatement<QueryGetTaxRestrFareType>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add carrier to this TCR and go on to next carrier
      FareType fareType =
          QueryGetTaxRestrFareTypeSQLStatement<QueryGetTaxRestrFareType>::mapRowToFareType(row);
      tcr->restrictionFareType().push_back(fareType);
      break;
    } // while (true)
  } // Fetchin RestrFareTypes

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRFARETYPE"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrFareType::getRestrFareTypes()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrFareClass
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrFareClass::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrFareClass> g_GetTaxRestrFareClass;
const char*
QueryGetTaxRestrFareClass::getQueryName() const
{
  return "GETTAXRESTRFARECLASS";
}

void
QueryGetTaxRestrFareClass::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrFareClassSQLStatement<QueryGetTaxRestrFareClass> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRFARECLASS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrFareClass::getRestrFareClasses(TaxCode txCd,
                                               std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                               std::vector<tse::TaxCodeReg*>& vecTCR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrFareClasses
    TaxCode taxCode =
        QueryGetTaxRestrFareClassSQLStatement<QueryGetTaxRestrFareClass>::mapRowToTaxCode(row);
    uint64_t versionDate =
        QueryGetTaxRestrFareClassSQLStatement<QueryGetTaxRestrFareClass>::mapRowToVersionDate(row);
    int seqNo =
        QueryGetTaxRestrFareClassSQLStatement<QueryGetTaxRestrFareClass>::mapRowToSeqNo(row);
    DateTime createDate =
        QueryGetTaxRestrFareClassSQLStatement<QueryGetTaxRestrFareClass>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add carrier to this TCR and go on to next carrier
      FareClassCode fareClass =
          QueryGetTaxRestrFareClassSQLStatement<QueryGetTaxRestrFareClass>::mapRowToFareClass(row);
      tcr->restrictionFareClass().push_back(fareClass);
      break;
    } // while (true)
  } // Fetchin RestrFareClasses

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRFARECLASS"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrFareClass::getRestrFareClasses()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxExempEquip
//
///////////////////////////////////////////////////////////
bool QueryGetTaxExempEquip::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxExempEquip> g_GetTaxExempEquip;
const char*
QueryGetTaxExempEquip::getQueryName() const
{
  return "GETTAXEXEMPEQUIP";
}

void
QueryGetTaxExempEquip::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxExempEquipSQLStatement<QueryGetTaxExempEquip> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXEXEMPEQUIP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxExempEquip::getExempEquips(TaxCode txCd,
                                      std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                      std::vector<tse::TaxCodeReg*>& vecTCR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin equipmentCodes
    TaxCode taxCode =
        QueryGetTaxExempEquipSQLStatement<QueryGetTaxExempEquip>::mapRowToTaxCode(row);
    uint64_t versionDate =
        QueryGetTaxExempEquipSQLStatement<QueryGetTaxExempEquip>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxExempEquipSQLStatement<QueryGetTaxExempEquip>::mapRowToSeqNo(row);
    DateTime createDate =
        QueryGetTaxExempEquipSQLStatement<QueryGetTaxExempEquip>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add carrier to this TCR and go on to next carrier
      std::string equipmentCode =
          QueryGetTaxExempEquipSQLStatement<QueryGetTaxExempEquip>::mapRowToEquipCode(row);
      tcr->equipmentCode().push_back(equipmentCode);
      break;
    } // while (true)
  } // Fetchin equipmentCodes

  LOG4CXX_INFO(_logger,
               "GETTAXEXEMPEQUIP"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxExempEquip::getExempEquips()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxExempCxr
//
///////////////////////////////////////////////////////////
bool QueryGetTaxExempCxr::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxExempCxr> g_GetTaxExempCxr;
const char*
QueryGetTaxExempCxr::getQueryName() const
{
  return "GETTAXEXEMPCXR";
}

void
QueryGetTaxExempCxr::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxExempCxrSQLStatement<QueryGetTaxExempCxr> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXEXEMPCXR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxExempCxr::getExempCxrs(TaxCode txCd,
                                  std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                  std::vector<tse::TaxCodeReg*>& vecTCR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin ExempCxrs
    TaxCode taxCode = QueryGetTaxExempCxrSQLStatement<QueryGetTaxExempCxr>::mapRowToTaxCode(row);
    uint64_t versionDate =
        QueryGetTaxExempCxrSQLStatement<QueryGetTaxExempCxr>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxExempCxrSQLStatement<QueryGetTaxExempCxr>::mapRowToSeqNo(row);
    DateTime createDate =
        QueryGetTaxExempCxrSQLStatement<QueryGetTaxExempCxr>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add psgr to this TCR and go on to next psgr
      TaxExemptionCarrier exemptionCarrier;
      QueryGetTaxExempCxrSQLStatement<QueryGetTaxExempCxr>::mapRowToTaxExemptionCarrier(
          row, exemptionCarrier);
      tcr->exemptionCxr().push_back(exemptionCarrier);
      break;
    } // while (true)
  } // Fetchin ExempCxrs

  LOG4CXX_INFO(_logger,
               "GETTAXEXEMPCXR"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxExempCxr::getExempCxrs()

int
QueryGetTaxExempCxr::checkFlightWildCard(const char* fltStr)
{
  if (fltStr[0] == '*')
    return -1;
  else
    return stringToInteger(fltStr, __LINE__); // lint !e668
} // QueryGetTaxExempCxr::checkFlightWildCard()

int
QueryGetTaxExempCxr::stringToInteger(const char* stringVal, int lineNumber)
{
  if (stringVal == nullptr)
  {
    LOG4CXX_FATAL(_logger,
                  "stringToInteger - Null pointer to int data. LineNumber: " << lineNumber);
    throw std::runtime_error("Null pointer to int data");
  }
  else if (*stringVal == '-' || *stringVal == '+')
  {
    if (stringVal[1] < '0' || stringVal[1] > '9')
      return 0;
  }
  else if (*stringVal < '0' || *stringVal > '9')
  {
    return 0;
  }
  return atoi(stringVal);
} // QueryGetTaxExempCxr::stringToInteger()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxOnTax
//
///////////////////////////////////////////////////////////
bool QueryGetTaxOnTax::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxOnTax> g_GetTaxOnTax;
const char*
QueryGetTaxOnTax::getQueryName() const
{
  return "GETTAXONTAX";
}

void
QueryGetTaxOnTax::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxOnTaxSQLStatement<QueryGetTaxOnTax> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXONTAX");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxOnTax::getTaxOnTaxes(TaxCode txCd,
                                std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                std::vector<tse::TaxCodeReg*>& vecTCR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin equipmentCodes
    TaxCode taxCode = QueryGetTaxOnTaxSQLStatement<QueryGetTaxOnTax>::mapRowToTaxCode(row);
    uint64_t versionDate = QueryGetTaxOnTaxSQLStatement<QueryGetTaxOnTax>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxOnTaxSQLStatement<QueryGetTaxOnTax>::mapRowToSeqNo(row);
    DateTime createDate = QueryGetTaxOnTaxSQLStatement<QueryGetTaxOnTax>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add carrier to this TCR and go on to next carrier
      std::string taxOnTaxCode =
          QueryGetTaxOnTaxSQLStatement<QueryGetTaxOnTax>::mapRowToTaxOnTaxCode(row);
      tcr->taxOnTaxCode().push_back(taxOnTaxCode);
      break;
    } // while (true)
  } // Fetchin equipmentCodes

  LOG4CXX_INFO(_logger,
               "GETTAXONTAX"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxOnTax::getTaxOnTaxes()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrTransit
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrTransit::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrTransit> g_GetTaxRestrTransit;
const char*
QueryGetTaxRestrTransit::getQueryName() const
{
  return "GETTAXRESTRTRANSIT";
}

void
QueryGetTaxRestrTransit::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrTransitSQLStatement<QueryGetTaxRestrTransit> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRTRANSIT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrTransit::getRestrTransits(TaxCode txCd,
                                          std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                          std::vector<tse::TaxCodeReg*>& vecTCR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrTransits
    TaxCode taxCode =
        QueryGetTaxRestrTransitSQLStatement<QueryGetTaxRestrTransit>::mapRowToTaxCode(row);
    uint64_t versionDate =
        QueryGetTaxRestrTransitSQLStatement<QueryGetTaxRestrTransit>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxRestrTransitSQLStatement<QueryGetTaxRestrTransit>::mapRowToSeqNo(row);
    DateTime createDate =
        QueryGetTaxRestrTransitSQLStatement<QueryGetTaxRestrTransit>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add psgr to this TCR and go on to next psgr
      TaxRestrictionTransit rt;
      QueryGetTaxRestrTransitSQLStatement<QueryGetTaxRestrTransit>::mapRowToTaxRestrictionTransit(
          row, rt);
      tcr->restrictionTransit().push_back(rt);
      break;
    } // while (true)
  } // Fetchin RestrTransits

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRTRANSIT"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrTransit::getRestrTransits()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrTktDsg
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrTktDsg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrTktDsg> g_GetTaxRestrTktDsg;
const char*
QueryGetTaxRestrTktDsg::getQueryName() const
{
  return "GETTAXRESTRTKTDSG";
}

void
QueryGetTaxRestrTktDsg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrTktDsgSQLStatement<QueryGetTaxRestrTktDsg> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRTKTDSG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrTktDsg::getRestrTktDsgs(TaxCode txCd,
                                        std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                        std::vector<tse::TaxCodeReg*>& vecTCR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrTktDsgs
    TaxCode taxCode =
        QueryGetTaxRestrTktDsgSQLStatement<QueryGetTaxRestrTktDsg>::mapRowToTaxCode(row);
    uint64_t versionDate =
        QueryGetTaxRestrTktDsgSQLStatement<QueryGetTaxRestrTktDsg>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxRestrTktDsgSQLStatement<QueryGetTaxRestrTktDsg>::mapRowToSeqNo(row);
    DateTime createDate =
        QueryGetTaxRestrTktDsgSQLStatement<QueryGetTaxRestrTktDsg>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (UNLIKELY(tcr->taxCode() < taxCode))
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (UNLIKELY(tcr->versionDate() < versionDate))
      {
        ++chldMatchIt;
        continue;
      }
      if (UNLIKELY(tcr->versionDate() > versionDate))
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (UNLIKELY(tcr->createDate() < createDate))
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add psgr to this TCR and go on to next psgr
      TaxRestrictionTktDesignator* td = new TaxRestrictionTktDesignator;
      QueryGetTaxRestrTktDsgSQLStatement<
          QueryGetTaxRestrTktDsg>::mapRowToTaxRestrictionTktDesignator(row, td);
      tcr->taxRestrTktDsgs().push_back(td);
      break;
    } // while (true)
  } // Fetchin RestrTktDsgs

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRTKTDSG"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrTktDsg::getRestrTktDsgs()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxCodeHistorical
//
///////////////////////////////////////////////////////////

bool QueryGetTaxCodeHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCodeHistorical> g_GetTaxCodeHistorical;

const char*
QueryGetTaxCodeHistorical::getQueryName() const
{
  return "GETTAXCODEHISTORICAL";
};

void
QueryGetTaxCodeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCodeHistoricalSQLStatement<QueryGetTaxCodeHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXCODEHISTORICAL");
    substTableDef(&_baseSQL);
    QueryGetTaxCodeGenTextHistorical::initialize();
    QueryGetTaxCodeGenTextSeqHistorical::initialize();
    QueryGetTaxRestrValCxrHistorical::initialize();
    QueryGetTaxRestrPsgrHistorical::initialize();
    QueryGetTaxRestrFareTypeHistorical::initialize();
    QueryGetTaxRestrFareClassHistorical::initialize();
    QueryGetTaxExempEquipHistorical::initialize();
    QueryGetTaxExempCxrHistorical::initialize();
    QueryGetTaxOnTaxHistorical::initialize();
    QueryGetTaxRestrTransitHistorical::initialize();
    QueryGetTaxRestrTktDsgHistorical::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxCodeHistorical::findTaxCodeRegHistorical(std::vector<tse::TaxCodeReg*>& taxC,
                                                    const TaxCode& code,
                                                    const DateTime& startDate,
                                                    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  std::vector<TaxCodeGenText*> texts;
  QueryGetTaxCodeGenTextHistorical SQLTaxCodeGenText(_dbAdapt);
  SQLTaxCodeGenText.findTaxCodeGenText(texts, code, startDate, endDate);

  substParm(code, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxCodeReg* tcr = nullptr;
  tse::TaxCodeReg* tcrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    tcr = QueryGetTaxCodeHistoricalSQLStatement<QueryGetTaxCodeHistorical>::mapRowToTaxCodeReg(
        row, tcrPrev);
    if (tcr != tcrPrev)
    {
      std::vector<TaxCodeGenText*>::iterator t = texts.begin();
      for (; t != texts.end(); t++)
      {
        tcr->taxCodeGenTexts().push_back(new TaxCodeGenText(**t));
      }
      taxC.push_back(tcr);
    }

    tcrPrev = tcr;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXCODEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();

  // Clean up original Text Children
  std::vector<TaxCodeGenText*>::iterator txtIt = texts.begin();
  for (; txtIt != texts.end(); txtIt++)
  {
    delete *txtIt;
  }
  texts.clear();

  if (taxC.size() == 0)
    return; // no use checking for children if no parent

  buildTaxCodeRegChildren(taxC, startDate, endDate);
} // QueryGetTaxCode::findTaxCodeReg()

void
QueryGetTaxCodeHistorical::buildTaxCodeRegChildren(std::vector<tse::TaxCodeReg*>& vecTaxCodeReg,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)
{
  TaxCode taxCodePrev = "";
  std::vector<tse::TaxCodeReg*>::iterator taxCodeRegIt = vecTaxCodeReg.begin();
  for (; taxCodeRegIt != vecTaxCodeReg.end(); taxCodeRegIt++)
  { // Main Iteration thru TaxCodeRegs
    if (taxCodePrev == (*taxCodeRegIt)->taxCode())
      continue;

    taxCodePrev = (*taxCodeRegIt)->taxCode();

    QueryGetTaxRestrValCxrHistorical SQLTaxRestrValCxr(_dbAdapt);
    SQLTaxRestrValCxr.getRestrValCxrs(taxCodePrev, taxCodeRegIt, vecTaxCodeReg, startDate, endDate);

    QueryGetTaxRestrPsgrHistorical SQLTaxRestrPsgr(_dbAdapt);
    SQLTaxRestrPsgr.getRestrPsgrs(taxCodePrev, taxCodeRegIt, vecTaxCodeReg, startDate, endDate);

    QueryGetTaxRestrFareTypeHistorical SQLTaxRestrFareType(_dbAdapt);
    SQLTaxRestrFareType.getRestrFareTypes(
        taxCodePrev, taxCodeRegIt, vecTaxCodeReg, startDate, endDate);

    QueryGetTaxRestrFareClassHistorical SQLTaxRestrFareClass(_dbAdapt);
    SQLTaxRestrFareClass.getRestrFareClasses(
        taxCodePrev, taxCodeRegIt, vecTaxCodeReg, startDate, endDate);

    QueryGetTaxExempEquipHistorical SQLTaxExempEquip(_dbAdapt);
    SQLTaxExempEquip.getExempEquips(taxCodePrev, taxCodeRegIt, vecTaxCodeReg, startDate, endDate);

    QueryGetTaxExempCxrHistorical SQLTaxExempCxr(_dbAdapt);
    SQLTaxExempCxr.getExempCxrs(taxCodePrev, taxCodeRegIt, vecTaxCodeReg, startDate, endDate);

    QueryGetTaxOnTaxHistorical SQLTaxOnTax(_dbAdapt);
    SQLTaxOnTax.getTaxOnTaxes(taxCodePrev, taxCodeRegIt, vecTaxCodeReg, startDate, endDate);

    QueryGetTaxRestrTransitHistorical SQLTaxRestrTransit(_dbAdapt);
    SQLTaxRestrTransit.getRestrTransits(
        taxCodePrev, taxCodeRegIt, vecTaxCodeReg, startDate, endDate);

    QueryGetTaxRestrTktDsgHistorical SQLTaxRestrTktDsg(_dbAdapt);
    SQLTaxRestrTktDsg.getRestrTktDsgs(taxCodePrev, taxCodeRegIt, vecTaxCodeReg, startDate, endDate);

    QueryGetTaxCodeGenTextSeqHistorical SQLTaxCodeGenTextSeq(_dbAdapt);
    SQLTaxCodeGenTextSeq.getCodeTextSeq(
        taxCodePrev, taxCodeRegIt, vecTaxCodeReg, startDate, endDate);

  } // for (Main Iteration thru TaxCodeRegs                                 )
} // QueryGetTaxCodeHistorical::buildTaxCodeRegChildren()

///////////////////////////////////////////////////////////
//
//  QueryGetAllTaxCodeRegsHistorical
//
///////////////////////////////////////////////////////////

bool QueryGetAllTaxCodeRegsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTaxCodeRegsHistorical> g_GetAllTaxCodeRegsHistorical;

const char*
QueryGetAllTaxCodeRegsHistorical::getQueryName() const
{
  return "GETALLTAXCODEREGSHISTORICAL";
};

void
QueryGetAllTaxCodeRegsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTaxCodeRegsHistoricalSQLStatement<QueryGetAllTaxCodeRegsHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTAXCODEREGSHISTORICAL");
    substTableDef(&_baseSQL);
    QueryGetTaxCodeGenTextHistorical::initialize();

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTaxCodeRegsHistorical::findAllTaxCodeReg(std::vector<tse::TaxCodeReg*>& vecTaxCodeReg,
                                                    const DateTime& startDate,
                                                    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, startDate);
  substParm(2, endDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxCodeReg* taxCodeReg = nullptr;
  tse::TaxCodeReg* taxCodeRegPrev = nullptr;
  while ((row = res.nextRow()))
  {
    taxCodeRegPrev = taxCodeReg;
    taxCodeReg = QueryGetAllTaxCodeRegsHistoricalSQLStatement<
        QueryGetAllTaxCodeRegsHistorical>::mapRowToTaxCodeReg(row, taxCodeRegPrev);
    if (taxCodeReg == taxCodeRegPrev)
      continue;

    vecTaxCodeReg.push_back(taxCodeReg);
  }
  LOG4CXX_INFO(_logger,
               "GETALLTAXCODEREGSHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();

  if (vecTaxCodeReg.empty())
    return;

  QueryGetTaxCodeGenTextHistorical SQLTaxCodeGenText(_dbAdapt);
  std::vector<TaxCodeGenText*> texts;
  std::vector<TaxCodeGenText*>::iterator txtIt;
  TaxCode taxCode = "";
  std::vector<tse::TaxCodeReg*>::iterator taxCodeRegIt = vecTaxCodeReg.begin();
  for (; taxCodeRegIt != vecTaxCodeReg.end(); taxCodeRegIt++)
  {
    if (taxCode != (*taxCodeRegIt)->taxCode())
    {
      taxCode = (*taxCodeRegIt)->taxCode();
      for (txtIt = texts.begin(); txtIt != texts.end(); txtIt++)
      {
        delete *txtIt;
      }
      texts.clear();
      SQLTaxCodeGenText.findTaxCodeGenText(texts, taxCode, startDate, endDate);
    }

    for (txtIt = texts.begin(); txtIt != texts.end(); txtIt++)
    {
      (*taxCodeRegIt)->taxCodeGenTexts().push_back(new TaxCodeGenText(**txtIt));
    }
  }
  for (txtIt = texts.begin(); txtIt != texts.end(); txtIt++)
  {
    delete *txtIt;
  }

  buildTaxCodeRegChildren(vecTaxCodeReg, startDate, endDate);
} // findAllTaxCodeReg()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxCodeGenTextSeqHistorical
//
///////////////////////////////////////////////////////////
bool QueryGetTaxCodeGenTextSeqHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCodeGenTextSeqHistorical> g_GetTaxCodeGenTextSeqHistorical;

const char*
QueryGetTaxCodeGenTextSeqHistorical::getQueryName() const
{
  return "GETTAX_CODE_GEN_TEXTSEQHISTORICAL";
}

void
QueryGetTaxCodeGenTextSeqHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCodeGenTextSeqHistoricalSQLStatement<QueryGetTaxCodeGenTextSeqHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAX_CODE_GEN_TEXTSEQHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxCodeGenTextSeqHistorical::getCodeTextSeq(TaxCode txCd,
                                                    std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                                    std::vector<tse::TaxCodeReg*>& vecTCR,
                                                    const DateTime& startDate,
                                                    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;

  tse::TaxCodeGenText* TaxText;
  tse::TaxCodeGenText* prevTaxText = nullptr;

  while ((row = res.nextRow()))
  {

    TaxCode taxCode = QueryGetTaxCodeGenTextSeqHistoricalSQLStatement<
        QueryGetTaxCodeGenTextSeqHistorical>::mapRowToTaxCode(row);
    uint64_t versionDate = QueryGetTaxCodeGenTextSeqHistoricalSQLStatement<
        QueryGetTaxCodeGenTextSeqHistorical>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxCodeGenTextSeqHistoricalSQLStatement<
        QueryGetTaxCodeGenTextSeqHistorical>::mapRowToSeqNo(row);
    DateTime createDate = QueryGetTaxCodeGenTextSeqHistoricalSQLStatement<
        QueryGetTaxCodeGenTextSeqHistorical>::mapRowToCreateDate(row);

    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;

      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;
      // Got a match record
      TaxText = QueryGetTaxCodeGenTextSeqHistoricalSQLStatement<
          QueryGetTaxCodeGenTextSeqHistorical>::mapRowToTaxCodeGenText(row, prevTaxText);
      if (TaxText != prevTaxText)
      {
        tcr->taxCodeGenTexts().push_back(TaxText);
        prevTaxText = TaxText;
      }
      break;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETTAX_CODE_GEN_TEXTSEQHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");

  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetTaxCodeGenTextHistorical
//
///////////////////////////////////////////////////////////

bool QueryGetTaxCodeGenTextHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCodeGenTextHistorical> g_GetTaxCodeGenTextHistorical;

const char*
QueryGetTaxCodeGenTextHistorical::getQueryName() const
{
  return "GETTAX_CODE_GEN_TEXTHISTORICAL";
}

void
QueryGetTaxCodeGenTextHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCodeGenTextHistoricalSQLStatement<QueryGetTaxCodeGenTextHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAX_CODE_GEN_TEXTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxCodeGenTextHistorical::findTaxCodeGenText(std::vector<tse::TaxCodeGenText*>& taxTexts,
                                                     const TaxCode& taxCode,
                                                     const DateTime& startDate,
                                                     const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(taxCode, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxCodeGenText* TaxText = nullptr;
  tse::TaxCodeGenText* prevTaxText = nullptr;
  while ((row = res.nextRow()))
  {
    TaxText = QueryGetTaxCodeGenTextHistoricalSQLStatement<
        QueryGetTaxCodeGenTextHistorical>::mapRowToTaxCodeGenText(row, prevTaxText);
    if (TaxText != prevTaxText)
      taxTexts.push_back(TaxText);

    prevTaxText = TaxText;
  }
  LOG4CXX_INFO(_logger,
               "GETTAX_CODE_GEN_TEXTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetTaxCodeGenTextHistorical::findTaxCodeGenText

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrValCxrHistorical
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrValCxrHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrValCxrHistorical> g_GetTaxRestrValCxrHistorical;

const char*
QueryGetTaxRestrValCxrHistorical::getQueryName() const
{
  return "GETTAXRESTRVALCXRHISTORICAL";
}

void
QueryGetTaxRestrValCxrHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrValCxrHistoricalSQLStatement<QueryGetTaxRestrValCxrHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRVALCXRHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrValCxrHistorical::getRestrValCxrs(TaxCode txCd,
                                                  std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                                  std::vector<tse::TaxCodeReg*>& vecTCR,
                                                  const DateTime& startDate,
                                                  const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substParm(2, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrValCxrs
    TaxCode taxCode = QueryGetTaxRestrValCxrHistoricalSQLStatement<
        QueryGetTaxRestrValCxrHistorical>::mapRowToTaxCode(row);
    uint64_t versionDate = QueryGetTaxRestrValCxrHistoricalSQLStatement<
        QueryGetTaxRestrValCxrHistorical>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxRestrValCxrHistoricalSQLStatement<
        QueryGetTaxRestrValCxrHistorical>::mapRowToSeqNo(row);
    DateTime createDate = QueryGetTaxRestrValCxrHistoricalSQLStatement<
        QueryGetTaxRestrValCxrHistorical>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add carrier to this TCR and go on to next carrier
      CarrierCode carrier = QueryGetTaxRestrValCxrHistoricalSQLStatement<
          QueryGetTaxRestrValCxrHistorical>::mapRowToCarrier(row);
      tcr->restrictionValidationCxr().push_back(carrier);
      break;
    } // while (true)
  } // Fetchin RestrValCxrs

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRVALCXRHISTORICAL"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrValCxrHistorical::getRestrValCxrs()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrPsgrHistorical
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrPsgrHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrPsgrHistorical> g_GetTaxRestrPsgrHistorical;

const char*
QueryGetTaxRestrPsgrHistorical::getQueryName() const
{
  return "GETTAXRESTRPSGRHISTORICAL";
}

void
QueryGetTaxRestrPsgrHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrPsgrHistoricalSQLStatement<QueryGetTaxRestrPsgrHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRPSGRHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrPsgrHistorical::getRestrPsgrs(TaxCode txCd,
                                              std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                              std::vector<tse::TaxCodeReg*>& vecTCR,
                                              const DateTime& startDate,
                                              const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substParm(2, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrPsgrs
    TaxCode taxCode =
        QueryGetTaxRestrPsgrHistoricalSQLStatement<QueryGetTaxRestrPsgrHistorical>::mapRowToTaxCode(
            row);
    uint64_t versionDate = QueryGetTaxRestrPsgrHistoricalSQLStatement<
        QueryGetTaxRestrPsgrHistorical>::mapRowToVersionDate(row);
    int seqNo =
        QueryGetTaxRestrPsgrHistoricalSQLStatement<QueryGetTaxRestrPsgrHistorical>::mapRowToSeqNo(
            row);
    DateTime createDate = QueryGetTaxRestrPsgrHistoricalSQLStatement<
        QueryGetTaxRestrPsgrHistorical>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add psgr to this TCR and go on to next psgr
      TaxRestrictionPsg restrictionPsg;
      QueryGetTaxRestrPsgrHistoricalSQLStatement<
          QueryGetTaxRestrPsgrHistorical>::mapRowToTaxRestrictionPsg(row, restrictionPsg);
      tcr->restrictionPsg().push_back(restrictionPsg);
      break;
    } // while (true)
  } // Fetchin RestrPsgrs

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRPSGRHISTORICAL"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrPsgrHistorical::getRestrPsgrs()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrFareTypeHistorical
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrFareTypeHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrFareTypeHistorical> g_GetTaxRestrFareTypeHistorical;

const char*
QueryGetTaxRestrFareTypeHistorical::getQueryName() const
{
  return "GETTAXRESTRFARETYPEHISTORICAL";
}

void
QueryGetTaxRestrFareTypeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrFareTypeHistoricalSQLStatement<QueryGetTaxRestrFareTypeHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRFARETYPEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrFareTypeHistorical::getRestrFareTypes(
    TaxCode txCd,
    std::vector<tse::TaxCodeReg*>::iterator& itTCR,
    std::vector<tse::TaxCodeReg*>& vecTCR,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substParm(2, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrFareTypes
    TaxCode taxCode = QueryGetTaxRestrFareTypeHistoricalSQLStatement<
        QueryGetTaxRestrFareTypeHistorical>::mapRowToTaxCode(row);
    uint64_t versionDate = QueryGetTaxRestrFareTypeHistoricalSQLStatement<
        QueryGetTaxRestrFareTypeHistorical>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxRestrFareTypeHistoricalSQLStatement<
        QueryGetTaxRestrFareTypeHistorical>::mapRowToSeqNo(row);
    DateTime createDate = QueryGetTaxRestrFareTypeHistoricalSQLStatement<
        QueryGetTaxRestrFareTypeHistorical>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add carrier to this TCR and go on to next carrier
      FareType fareType = QueryGetTaxRestrFareTypeHistoricalSQLStatement<
          QueryGetTaxRestrFareTypeHistorical>::mapRowToFareType(row);
      tcr->restrictionFareType().push_back(fareType);
      break;
    } // while (true)
  } // Fetchin RestrFareTypes

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRFARETYPEHISTORICAL"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrFareTypeHistorical::getRestrFareTypes()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrFareClassHistorical
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrFareClassHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrFareClassHistorical> g_GetTaxRestrFareClassHistorical;
const char*
QueryGetTaxRestrFareClassHistorical::getQueryName() const
{
  return "GETTAXRESTRFARECLASSHISTORICAL";
}

void
QueryGetTaxRestrFareClassHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrFareClassHistoricalSQLStatement<QueryGetTaxRestrFareClass> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRFARECLASSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrFareClassHistorical::getRestrFareClasses(
    TaxCode txCd,
    std::vector<tse::TaxCodeReg*>::iterator& itTCR,
    std::vector<tse::TaxCodeReg*>& vecTCR,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substParm(2, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrFareClasses
    TaxCode taxCode = QueryGetTaxRestrFareClassHistoricalSQLStatement<
        QueryGetTaxRestrFareClassHistorical>::mapRowToTaxCode(row);
    uint64_t versionDate = QueryGetTaxRestrFareClassHistoricalSQLStatement<
        QueryGetTaxRestrFareClassHistorical>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxRestrFareClassHistoricalSQLStatement<
        QueryGetTaxRestrFareClassHistorical>::mapRowToSeqNo(row);
    DateTime createDate = QueryGetTaxRestrFareClassHistoricalSQLStatement<
        QueryGetTaxRestrFareClassHistorical>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add carrier to this TCR and go on to next carrier
      FareClassCode fareClass = QueryGetTaxRestrFareClassHistoricalSQLStatement<
          QueryGetTaxRestrFareClassHistorical>::mapRowToFareClass(row);
      tcr->restrictionFareClass().push_back(fareClass);
      break;
    } // while (true)
  } // Fetchin RestrFareClasses

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRFARECLASSHISTORICAL"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrFareClassHistorical::getRestrFareClasses()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxExempEquipHistorical
//
///////////////////////////////////////////////////////////
bool QueryGetTaxExempEquipHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxExempEquipHistorical> g_GetTaxExempEquipHistorical;
const char*
QueryGetTaxExempEquipHistorical::getQueryName() const
{
  return "GETTAXEXEMPEQUIPHISTORICAL";
}

void
QueryGetTaxExempEquipHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxExempEquipHistoricalSQLStatement<QueryGetTaxExempEquipHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXEXEMPEQUIPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxExempEquipHistorical::getExempEquips(TaxCode txCd,
                                                std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                                std::vector<tse::TaxCodeReg*>& vecTCR,
                                                const DateTime& startDate,
                                                const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substParm(2, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin equipmentCodes
    TaxCode taxCode = QueryGetTaxExempEquipHistoricalSQLStatement<
        QueryGetTaxExempEquipHistorical>::mapRowToTaxCode(row);
    uint64_t versionDate = QueryGetTaxExempEquipHistoricalSQLStatement<
        QueryGetTaxExempEquipHistorical>::mapRowToVersionDate(row);
    int seqNo =
        QueryGetTaxExempEquipHistoricalSQLStatement<QueryGetTaxExempEquipHistorical>::mapRowToSeqNo(
            row);
    DateTime createDate = QueryGetTaxExempEquipHistoricalSQLStatement<
        QueryGetTaxExempEquipHistorical>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add carrier to this TCR and go on to next carrier
      std::string equipmentCode = QueryGetTaxExempEquipHistoricalSQLStatement<
          QueryGetTaxExempEquipHistorical>::mapRowToEquipCode(row);
      tcr->equipmentCode().push_back(equipmentCode);
      break;
    } // while (true)
  } // Fetchin equipmentCodes

  LOG4CXX_INFO(_logger,
               "GETTAXEXEMPEQUIPHISTORICAL"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxExempEquipHistorical::getExempEquips()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxExempCxrHistorical
//
///////////////////////////////////////////////////////////
bool QueryGetTaxExempCxrHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxExempCxrHistorical> g_GetTaxExempCxrHistorical;
const char*
QueryGetTaxExempCxrHistorical::getQueryName() const
{
  return "GETTAXEXEMPCXRHISTORICAL";
}

void
QueryGetTaxExempCxrHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxExempCxrHistoricalSQLStatement<QueryGetTaxExempCxrHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXEXEMPCXRHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxExempCxrHistorical::getExempCxrs(TaxCode txCd,
                                            std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                            std::vector<tse::TaxCodeReg*>& vecTCR,
                                            const DateTime& startDate,
                                            const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substParm(2, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin ExempCxrs
    TaxCode taxCode =
        QueryGetTaxExempCxrHistoricalSQLStatement<QueryGetTaxExempCxrHistorical>::mapRowToTaxCode(
            row);
    uint64_t versionDate = QueryGetTaxExempCxrHistoricalSQLStatement<
        QueryGetTaxExempCxrHistorical>::mapRowToVersionDate(row);
    int seqNo =
        QueryGetTaxExempCxrHistoricalSQLStatement<QueryGetTaxExempCxrHistorical>::mapRowToSeqNo(
            row);
    DateTime createDate = QueryGetTaxExempCxrHistoricalSQLStatement<
        QueryGetTaxExempCxrHistorical>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add psgr to this TCR and go on to next psgr
      TaxExemptionCarrier exemptionCarrier;
      QueryGetTaxExempCxrHistoricalSQLStatement<
          QueryGetTaxExempCxrHistorical>::mapRowToTaxExemptionCarrier(row, exemptionCarrier);
      tcr->exemptionCxr().push_back(exemptionCarrier);
      break;
    } // while (true)
  } // Fetchin ExempCxrs

  LOG4CXX_INFO(_logger,
               "GETTAXEXEMPCXRHISTORICAL"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxExempCxrHistorical::getExempCxrs()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxOnTaxHistorical
//
///////////////////////////////////////////////////////////
bool QueryGetTaxOnTaxHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxOnTaxHistorical> g_GetTaxOnTaxHistorical;
const char*
QueryGetTaxOnTaxHistorical::getQueryName() const
{
  return "GETTAXONTAXHISTORICAL";
}

void
QueryGetTaxOnTaxHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxOnTaxHistoricalSQLStatement<QueryGetTaxOnTaxHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXONTAXHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxOnTaxHistorical::getTaxOnTaxes(TaxCode txCd,
                                          std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                          std::vector<tse::TaxCodeReg*>& vecTCR,
                                          const DateTime& startDate,
                                          const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substParm(2, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin equipmentCodes
    TaxCode taxCode =
        QueryGetTaxOnTaxHistoricalSQLStatement<QueryGetTaxOnTaxHistorical>::mapRowToTaxCode(row);
    uint64_t versionDate =
        QueryGetTaxOnTaxHistoricalSQLStatement<QueryGetTaxOnTaxHistorical>::mapRowToVersionDate(
            row);
    int seqNo =
        QueryGetTaxOnTaxHistoricalSQLStatement<QueryGetTaxOnTaxHistorical>::mapRowToSeqNo(row);
    DateTime createDate =
        QueryGetTaxOnTaxHistoricalSQLStatement<QueryGetTaxOnTaxHistorical>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add carrier to this TCR and go on to next carrier
      std::string taxOnTaxCode =
          QueryGetTaxOnTaxHistoricalSQLStatement<QueryGetTaxOnTaxHistorical>::mapRowToTaxOnTaxCode(
              row);
      tcr->taxOnTaxCode().push_back(taxOnTaxCode);
      break;
    } // while (true)
  } // Fetchin equipmentCodes

  LOG4CXX_INFO(_logger,
               "GETTAXONTAXHISTORICAL"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxOnTaxHistorical::getTaxOnTaxes()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrTransitHistorical
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrTransitHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrTransitHistorical> g_GetTaxRestrTransitHistorical;
const char*
QueryGetTaxRestrTransitHistorical::getQueryName() const
{
  return "GETTAXRESTRTRANSITHISTORICAL";
}

void
QueryGetTaxRestrTransitHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrTransitHistoricalSQLStatement<QueryGetTaxRestrTransitHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRTRANSITHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrTransitHistorical::getRestrTransits(TaxCode txCd,
                                                    std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                                    std::vector<tse::TaxCodeReg*>& vecTCR,
                                                    const DateTime& startDate,
                                                    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substParm(2, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrTransits
    TaxCode taxCode = QueryGetTaxRestrTransitHistoricalSQLStatement<
        QueryGetTaxRestrTransitHistorical>::mapRowToTaxCode(row);
    uint64_t versionDate = QueryGetTaxRestrTransitHistoricalSQLStatement<
        QueryGetTaxRestrTransitHistorical>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxRestrTransitHistoricalSQLStatement<
        QueryGetTaxRestrTransitHistorical>::mapRowToSeqNo(row);
    DateTime createDate = QueryGetTaxRestrTransitHistoricalSQLStatement<
        QueryGetTaxRestrTransitHistorical>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add psgr to this TCR and go on to next psgr
      TaxRestrictionTransit rt;
      QueryGetTaxRestrTransitHistoricalSQLStatement<
          QueryGetTaxRestrTransitHistorical>::mapRowToTaxRestrictionTransit(row, rt);
      tcr->restrictionTransit().push_back(rt);
      break;
    } // while (true)
  } // Fetchin RestrTransits

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRTRANSITHISTORICAL"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrTransitHistorical::getRestrTransits()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrTktDsgHistorical
//
///////////////////////////////////////////////////////////
bool QueryGetTaxRestrTktDsgHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrTktDsgHistorical> g_GetTaxRestrTktDsgHistorical;
const char*
QueryGetTaxRestrTktDsgHistorical::getQueryName() const
{
  return "GETTAXRESTRTKTDSGHISTORICAL";
}

void
QueryGetTaxRestrTktDsgHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrTktDsgHistoricalSQLStatement<QueryGetTaxRestrTktDsgHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRTKTDSGHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrTktDsgHistorical::getRestrTktDsgs(TaxCode txCd,
                                                  std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                                                  std::vector<tse::TaxCodeReg*>& vecTCR,
                                                  const DateTime& startDate,
                                                  const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(txCd, 1);
  substParm(2, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  std::vector<tse::TaxCodeReg*>::iterator chldMatchIt = itTCR;
  while ((row = res.nextRow()))
  { // Fetchin RestrTktDsgs
    TaxCode taxCode = QueryGetTaxRestrTktDsgHistoricalSQLStatement<
        QueryGetTaxRestrTktDsgHistorical>::mapRowToTaxCode(row);
    uint64_t versionDate = QueryGetTaxRestrTktDsgHistoricalSQLStatement<
        QueryGetTaxRestrTktDsgHistorical>::mapRowToVersionDate(row);
    int seqNo = QueryGetTaxRestrTktDsgHistoricalSQLStatement<
        QueryGetTaxRestrTktDsgHistorical>::mapRowToSeqNo(row);
    DateTime createDate = QueryGetTaxRestrTktDsgHistoricalSQLStatement<
        QueryGetTaxRestrTktDsgHistorical>::mapRowToCreateDate(row);
    while (true)
    { // loop through TaxCodeReg vector for match to current child record
      if (chldMatchIt == vecTCR.end())
        break;

      tse::TaxCodeReg* tcr = *chldMatchIt;
      if (tcr->taxCode() < taxCode)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->taxCode() > taxCode)
        break;
      if (tcr->versionDate() < versionDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->versionDate() > versionDate)
        break;
      if (tcr->seqNo() < seqNo)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->seqNo() > seqNo)
        break;
      if (tcr->createDate() < createDate)
      {
        ++chldMatchIt;
        continue;
      }
      if (tcr->createDate() > createDate)
        break;

      // Got a match so add psgr to this TCR and go on to next psgr
      TaxRestrictionTktDesignator* td = new TaxRestrictionTktDesignator;
      QueryGetTaxRestrTktDsgHistoricalSQLStatement<
          QueryGetTaxRestrTktDsgHistorical>::mapRowToTaxRestrictionTktDesignator(row, td);
      tcr->taxRestrTktDsgs().push_back(td);
      break;
    } // while (true)
  } // Fetchin RestrTktDsgs

  LOG4CXX_INFO(_logger,
               "GETTAXRESTRTKTDSGHISTORICAL"
                   << ": NumRows = " << res.numRows() << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetTaxRestrTktDsgHistorical::getRestrTktDsgs()
}
