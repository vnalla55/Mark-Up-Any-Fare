//----------------------------------------------------------------------------
//          File:           QueryGetFltTrkSQLStatement.h
//          Description:    QueryGetFltTrkSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
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
#include "DBAccess/Queries/QueryGetFltTrk.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFltTrkSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFltTrkSQLStatement() {};
  virtual ~QueryGetFltTrkSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    FLTTRKAPPLIND,
    NATION,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select f1.CARRIER,f1.CREATEDATE,EXPIREDATE,EFFDATE,"
                  "       DISCDATE,FLTTRKAPPLIND,NATION");
    this->From("=FLTTRKGCNTRYGRP f1, =FLTTRKGCNTRYGRPSEG f2");
    this->Where(" f1.CARRIER = f2.CARRIER"
                "    and f1.CREATEDATE = f2.CREATEDATE"
                "    and %cd <= EXPIREDATE"
                "    and f1.CARRIER = %1q ");
    if (DataManager::forceSortOrder())
      this->OrderBy("CREATEDATE,NATION");
    else
      this->OrderBy("CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::FltTrkCntryGrp* mapRowToFltTrkCntryGrp(Row* row, FltTrkCntryGrp* trkPrev)
  {
    DateTime tCreate = row->getDate(CREATEDATE);
    // If the TaxNation has not changed then let we just have a new TAXCodeOrder
    if (trkPrev != nullptr && trkPrev->createDate() == tCreate)
    {
      trkPrev->nations().push_back(row->getString(NATION));
      return trkPrev;
    }
    else
    {
      tse::FltTrkCntryGrp* trk = new tse::FltTrkCntryGrp;

      trk->carrier() = row->getString(CARRIER);
      trk->createDate() = row->getDate(CREATEDATE);
      trk->expireDate() = row->getDate(EXPIREDATE);
      trk->effDate() = row->getDate(EFFDATE);
      trk->discDate() = row->getDate(DISCDATE);
      trk->flttrkApplInd() = row->getString(FLTTRKAPPLIND)[0];

      trk->nations().push_back(row->getString(NATION));
      return trk;
    }
  }; // mapRowToFltTrkCntryGrp()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetFltTrkSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllFltTrkSQLStatement : public QueryGetFltTrkSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("f1.CARRIER = f2.CARRIER"
                "    and f1.CREATEDATE = f2.CREATEDATE"
                "    and %cd <= EXPIREDATE");

    this->OrderBy("CARRIER, CREATEDATE");
  }; // adjustBaseSQL()
}; // class QueryGetAllFltTrkSQLStatement

////////////////////////////////////////////////////////////////////////
//   adjust base query to QueryGetFltTrkHistoricalSQLStatement
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFltTrkHistoricalSQLStatement : public QueryGetFltTrkSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("f1.CARRIER = f2.CARRIER"
                "    and f1.CREATEDATE = f2.CREATEDATE"
                "    and f1.CARRIER = %1q ");
  }; // adjustBaseSQL()
}; // class QueryGetFltTrkHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
//   adjust base query to QueryGetAllFltTrkHistoricalSQLStatement
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllFltTrkHistoricalSQLStatement : public QueryGetFltTrkSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("f1.CARRIER = f2.CARRIER"
                "    and f1.CREATEDATE = f2.CREATEDATE");

    this->OrderBy("CARRIER, CREATEDATE");
  }; // adjustBaseSQL()
}; // class QueryGetAllFltTrkHistoricalSQLStatement
}

