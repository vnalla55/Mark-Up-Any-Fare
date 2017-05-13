//----------------------------------------------------------------------------
//          File:           QueryGetTPMExclusionSQLStatement.h
//          Description:    QueryGetTPMExclusionSQLStatement
//          Created:        08/10/2009
//          Authors:        Adam Szalajko
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
#include "DBAccess/SQLStatement.h"
#include "DBAccess/TPMExclusion.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTPMExclusionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTPMExclusionSQLStatement() {};
  virtual ~QueryGetTPMExclusionSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    USERAPPLTYPE,
    USERAPPL,
    NOTAPPLICABLETOYY,
    ONLINESVCONLY,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    GLOBALDIR,
    SECTOR1APPL,
    SECTOR1LOC1TYPE,
    SECTOR1LOC1,
    SECTOR1LOC2TYPE,
    SECTOR1LOC2,
    SECTOR2APPL,
    SECTOR2LOC1TYPE,
    SECTOR2LOC1,
    SECTOR2LOC2TYPE,
    SECTOR2LOC2,
    VIAPOINTRESTR,
    CONSECTMUSTBEONGOVCXR,
    SURFACEPERMITTED,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER, SEQNO, CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE,"
                  "USERAPPLTYPE, USERAPPL, NOTAPPLICABLETOYY, ONLINESVCONLY,"
                  "DIRECTIONALITY, LOC1TYPE, LOC1, LOC2TYPE, LOC2, GLOBALDIR, SECTOR1APPL,"
                  "SECTOR1LOC1TYPE, SECTOR1LOC1, SECTOR1LOC2TYPE, SECTOR1LOC2, SECTOR2APPL,"
                  "SECTOR2LOC1TYPE, SECTOR2LOC1, SECTOR2LOC2TYPE, SECTOR2LOC2, VIAPOINTRESTR,"
                  "CONSECTMUSTBEONGOVCXR, SURFACEPERMITTED");
    this->From("=TPMEXCLUSION ");
    this->Where("CARRIER = %1q "
                "   and %cd <= EXPIREDATE ");
    this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TPMExclusion* mapRowToTPMExcl(Row* row)
  {
    TPMExclusion* tpm = new TPMExclusion;

    tpm->carrier() = row->getString(CARRIER);
    tpm->seqNo() = row->getLong(SEQNO);
    tpm->createDate() = row->getDate(CREATEDATE);
    tpm->expireDate() = row->getDate(EXPIREDATE);
    tpm->effDate() = row->getDate(EFFDATE);
    tpm->discDate() = row->getDate(DISCDATE);
    tpm->userApplType() = row->getChar(USERAPPLTYPE);
    tpm->userAppl() = row->getString(USERAPPL);
    tpm->notApplToYY() = row->getChar(NOTAPPLICABLETOYY);
    tpm->onlineSrvOnly() = row->getChar(ONLINESVCONLY);
    std::string dir = row->getString(DIRECTIONALITY);
    if (dir == "F")
      tpm->directionality() = FROM;
    else if (dir == "W")
      tpm->directionality() = WITHIN;
    else if (dir == "O")
      tpm->directionality() = ORIGIN;
    else if (dir == "X")
      tpm->directionality() = TERMINATE;
    else
      tpm->directionality() = BETWEEN;
    tpm->loc1type() = LocType(row->getChar(LOC1TYPE));
    tpm->loc1() = row->getString(LOC1);
    tpm->loc2type() = LocType(row->getChar(LOC2TYPE));
    tpm->loc2() = row->getString(LOC2);
    strToGlobalDirection(tpm->globalDir(), row->getString(GLOBALDIR));
    tpm->sec1Appl() = row->getChar(SECTOR1APPL);
    tpm->sec1Loc1Type() = LocType(row->getChar(SECTOR1LOC1TYPE));
    tpm->sec1Loc1() = row->getString(SECTOR1LOC1);
    tpm->sec1Loc2Type() = LocType(row->getChar(SECTOR1LOC2TYPE));
    tpm->sec1Loc2() = row->getString(SECTOR1LOC2);
    tpm->sec2Appl() = row->getChar(SECTOR2APPL);
    tpm->sec2Loc1Type() = LocType(row->getChar(SECTOR2LOC1TYPE));
    tpm->sec2Loc1() = row->getString(SECTOR2LOC1);
    tpm->sec2Loc2Type() = LocType(row->getChar(SECTOR2LOC2TYPE));
    tpm->sec2Loc2() = row->getString(SECTOR2LOC2);
    tpm->viaPointRest() = row->getChar(VIAPOINTRESTR);
    tpm->consecMustBeOnGovCxr() = row->getChar(CONSECTMUSTBEONGOVCXR);
    tpm->surfacePermitted() = row->getChar(SURFACEPERMITTED);

    return tpm;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTPMExclusionHistoricalSQLStaement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetTPMExclusionHistoricalSQLStatement
    : public QueryGetTPMExclusionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER = %1q ");
    if (DataManager::forceSortOrder())
      this->OrderBy("SEQNO,CREATEDATE");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllTPMExclusionSQLStatement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllTPMExclusionSQLStatement : public QueryGetTPMExclusionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("CARRIER, SEQNO");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllTPMExclusionHistoricalSQLStaement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllTPMExclusionHistoricalSQLStatement
    : public QueryGetTPMExclusionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("1=1");
    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER, SEQNO, CREATEDATE");
    else
      this->OrderBy("CARRIER, SEQNO");
  };
};
} // tse
