//----------------------------------------------------------------------------
//          File:           QueryGetYQYRFeesNonConcurSQLStatement.h
//          Description:    QueryGetYQYRFeesNonConcurSQLStatement
//          Created:        10/5/2007
//          Authors:         Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetYQYRFeesNonConcur.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetYQYRFeesNonConcurSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetYQYRFeesNonConcurSQLStatement() {}
  virtual ~QueryGetYQYRFeesNonConcurSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    SELFAPPL,
    TAXCARRIERAPPLTBLITEMNO,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select fnc.VENDOR,fnc.CARRIER,fnc.CREATEDATE,fnc.EXPIREDATE,fnc.EFFDATE,"
                  " fnc.DISCDATE,fnc.SELFAPPL,fnc.TAXCARRIERAPPLTBLITEMNO");
    this->From("=YQYRFEESNONCONCUR fnc");
    this->Where("%cd <= fnc.EXPIREDATE"
                " and fnc.CARRIER = %1q"
                " and fnc.VALIDITYIND = 'Y'");

    if (DataManager::forceSortOrder())
      this->OrderBy("fnc.VENDOR,fnc.CARRIER,fnc.CREATEDATE, fnc.TAXCARRIERAPPLTBLITEMNO");
    else
      this->OrderBy("fnc.VENDOR,fnc.CARRIER,fnc.CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static YQYRFeesNonConcur* mapRowToYQYRFeesNonConcur(Row* row, YQYRFeesNonConcur* fncPrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    CarrierCode carrier = row->getString(CARRIER);
    DateTime createDate = row->getDate(CREATEDATE);

    YQYRFeesNonConcur* fnc;

    fnc = new tse::YQYRFeesNonConcur;
    fnc->vendor() = vendor;
    fnc->carrier() = carrier;
    fnc->createDate() = createDate;

    fnc->expireDate() = row->getDate(EXPIREDATE);
    fnc->effDate() = row->getDate(EFFDATE);
    fnc->discDate() = row->getDate(DISCDATE);

    // Times not included from ATP for discdate, so must bump 1 day!
    if (fnc->discDate() < fnc->expireDate())
    {
      DateTime discPlusOne = fnc->discDate().addDays(1);
      fnc->discDate() = discPlusOne;
    }

    fnc->selfAppl() = row->getChar(SELFAPPL);
    fnc->taxCarrierApplTblItemNo() = row->getInt(TAXCARRIERAPPLTBLITEMNO);

    return fnc;
  } // mapRowToYQYRFeesNonConcur()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllYQYRFeesNonConcurSQLStatement
    : public QueryGetYQYRFeesNonConcurSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("%cd <= fnc.EXPIREDATE and fnc.VALIDITYIND = 'Y'"); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetYQYRFeesNonConcurHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetYQYRFeesNonConcurHistoricalSQLStatement
    : public QueryGetYQYRFeesNonConcurSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->From("=YQYRFEESNONCONCUR fnc");

    this->Where("fnc.CARRIER = %1q"
                " and fnc.VALIDITYIND = 'Y'"
                " and %2n <= fnc.EXPIREDATE"
                " and %3n >= fnc.CREATEDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("fnc.VENDOR,fnc.CARRIER,fnc.CREATEDATE");
    else
      this->OrderBy("fnc.VENDOR,fnc.CREATEDATE");
  }
};
}

