//----------------------------------------------------------------------------
//          File:           QueryGetOpenJawRuleSQLStatement.h
//          Description:    QueryGetOpenJawRuleSQLStatement
//          Created:        10/26/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetOpenJawRule.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetOpenJawRuleSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetOpenJawRuleSQLStatement() {};
  virtual ~QueryGetOpenJawRuleSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    OJAPPLIND,
    SAMECARRIERIND,
    HALFTRANSPORTIND,
    OWRT,
    SOJVALIDITYIND,
    HIGHRTIND,
    OJBACKHAULIND,
    STOPOVERCNT,
    FARECOMPIND,
    UNAVAILTAG,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,TEXTTBLITEMNO,"
                  "       OVERRIDEDATETBLITEMNO,OJAPPLIND,SAMECARRIERIND,"
                  "       HALFTRANSPORTIND,OWRT,SOJVALIDITYIND,HIGHRTIND,"
                  "       OJBACKHAULIND,STOPOVERCNT,FARECOMPIND,UNAVAILTAG,INHIBIT");
    this->From("=OPENJAWS ");
    this->Where("VENDOR = %1q "
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::OpenJawRule* mapRowToOpenJawRule(Row* row)
  {
    tse::OpenJawRule* openJaw = new tse::OpenJawRule;

    openJaw->vendor() = row->getString(VENDOR);
    openJaw->itemNo() = row->getInt(ITEMNO);
    openJaw->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    openJaw->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    openJaw->createDate() = row->getDate(CREATEDATE);
    openJaw->expireDate() = row->getDate(EXPIREDATE);
    openJaw->ojApplInd() = row->getChar(OJAPPLIND);
    openJaw->sameCarrierInd() = row->getChar(SAMECARRIERIND);
    openJaw->halftransportInd() = row->getChar(HALFTRANSPORTIND);
    openJaw->owrt() = row->getChar(OWRT);
    openJaw->sojvalidityInd() = row->getChar(SOJVALIDITYIND);
    openJaw->highrtInd() = row->getChar(HIGHRTIND);
    openJaw->ojbackhaulInd() = row->getChar(OJBACKHAULIND);
    openJaw->stopoverCnt() = row->getString(STOPOVERCNT);
    openJaw->farecompInd() = row->getChar(FARECOMPIND);
    openJaw->unavailtag() = row->getChar(UNAVAILTAG);
    openJaw->inhibit() = row->getChar(INHIBIT);
    return openJaw;
  } // mapRowToOpenJawRule()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetOpenJawRuleSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetOpenJawRuleHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetOpenJawRuleHistoricalSQLStatement : public QueryGetOpenJawRuleSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q "
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetOpenJawRuleHistoricalSQLStatement
} // tse
