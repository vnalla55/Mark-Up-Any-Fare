//----------------------------------------------------------------------------
//          File:           QueryGetCommissionCapSQLStatement.h
//          Description:    QueryGetCommissionCapSQLStatement
//          Created:        10/30/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionCap.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCommissionCapSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCommissionCapSQLStatement() {};
  virtual ~QueryGetCommissionCapSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CUR,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    AMTNODEC,
    AMT,
    MINAMTNODEC,
    MINAMT,
    MAXAMTNODEC,
    MAXAMT,
    AGENCYTYPE,
    AMTTYPE,
    MINAMTTYPE,
    MAXAMTTYPE,
    CANADA,
    DOMESTIC,
    INTERNATIONAL,
    TRANSBORDER,
    OWRT,
    VALCARRIERTVL,
    TKTISSUENATION,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER,CUR,SEQNO,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
                  "       AMTNODEC,AMT,MINAMTNODEC,MINAMT,MAXAMTNODEC,MAXAMT,AGENCYTYPE,"
                  "       AMTTYPE,MINAMTTYPE,MAXAMTTYPE,CANADA,DOMESTIC,INTERNATIONAL,"
                  "       TRANSBORDER,OWRT,VALCARRIERTVL,TKTISSUENATION,INHIBIT");
    this->From("=COMMISSIONCAP");
    this->Where("CARRIER = %1q "
                "    and CUR = %2q "
                "    and %cd <= EXPIREDATE"
                "    and VALIDITYIND = 'Y'");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::CommissionCap* mapRowToCommissionCap(Row* row)
  {
    tse::CommissionCap* cc = new tse::CommissionCap;

    cc->carrier() = row->getString(CARRIER);
    cc->cur() = row->getString(CUR);
    cc->seqNo() = row->getLong(SEQNO);
    cc->createDate() = row->getDate(CREATEDATE);
    cc->expireDate() = row->getDate(EXPIREDATE);
    cc->effDate() = row->getDate(EFFDATE);
    cc->discDate() = row->getDate(DISCDATE);

    cc->amtNoDec() = row->getInt(AMTNODEC);
    cc->amt() = QUERYCLASS::adjustDecimal(row->getInt(AMT), cc->amtNoDec());

    cc->minAmtNoDec() = row->getInt(MINAMTNODEC);
    cc->minAmt() = QUERYCLASS::adjustDecimal(row->getInt(MINAMT), cc->minAmtNoDec());

    cc->maxAmtNoDec() = row->getInt(MAXAMTNODEC);
    cc->maxAmt() = QUERYCLASS::adjustDecimal(row->getInt(MAXAMT), cc->maxAmtNoDec());

    cc->agencyType() = row->getChar(AGENCYTYPE);
    cc->amtType() = row->getChar(AMTTYPE);
    cc->minAmtType() = row->getChar(MINAMTTYPE);
    cc->maxAmtType() = row->getChar(MAXAMTTYPE);
    cc->canada() = row->getChar(CANADA);
    cc->domestic() = row->getChar(DOMESTIC);
    cc->international() = row->getChar(INTERNATIONAL);
    cc->transBorder() = row->getChar(TRANSBORDER);
    cc->owRt() = row->getChar(OWRT);
    cc->valCarrierTvl() = row->getChar(VALCARRIERTVL);
    cc->tktIssueNation() = row->getString(TKTISSUENATION);
    cc->inhibit() = row->getChar(INHIBIT);

    return cc;
  } // mapRowToCommissionCap()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for where clause
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetCommissionCapHistoricalSQLStatement
    : public QueryGetCommissionCapSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER = %1q "
                "    and CUR = %2q "
                "    and VALIDITYIND = 'Y'");
  }
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCommissionCap
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCommissionCapSQLStatement : public QueryGetCommissionCapSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE"
                "    and VALIDITYIND = 'Y'");
    this->OrderBy("1,2");
  };
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for where clause
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCommissionCapHistoricalSQLStatement
    : public QueryGetCommissionCapSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'");
    this->OrderBy("1,2");
  }
};
} // tse
