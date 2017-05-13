//----------------------------------------------------------------------------
//          File:           QueryGetMiscFareTagSQLStatement.h
//          Description:    QueryGetMiscFareTagSQLStatement
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
#include "DBAccess/Queries/QueryGetMiscFareTag.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMiscFareTagSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMiscFareTagSQLStatement() {};
  virtual ~QueryGetMiscFareTagSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    CONSTIND,
    PRORATEIND,
    DIFFCALCIND,
    REFUNDCALCIND,
    PROPORTIONALIND,
    CURRADJUSTIND,
    FARECLASSTYPE1,
    FARECLASSTYPE1IND,
    FARECLASSTYPE2IND,
    FARECLASSTYPE2,
    GEOTBLITEMNO,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,TEXTTBLITEMNO,"
                  "       OVERRIDEDATETBLITEMNO,CONSTIND,PRORATEIND,DIFFCALCIND,"
                  "       REFUNDCALCIND,PROPORTIONALIND,CURRADJUSTIND,"
                  "       FARECLASSTYPE1,FARECLASSTYPE1IND,FARECLASSTYPE2IND,"
                  "       FARECLASSTYPE2,GEOTBLITEMNO,INHIBIT");
    this->From("=MISCFARETAGS");
    this->Where("VENDOR = %1q"
                "    and VALIDITYIND = 'Y'"
                "    and ITEMNO = %2n"
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MiscFareTag* mapRowToMiscFareTag(Row* row)
  {
    tse::MiscFareTag* misc = new tse::MiscFareTag;

    misc->vendor() = row->getString(VENDOR);
    misc->itemNo() = row->getInt(ITEMNO);
    misc->createDate() = row->getDate(CREATEDATE);
    misc->expireDate() = row->getDate(EXPIREDATE);
    misc->unavailtag() = row->getChar(UNAVAILTAG);
    misc->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    misc->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    misc->constInd() = row->getChar(CONSTIND);
    misc->prorateInd() = row->getChar(PRORATEIND);
    misc->diffcalcInd() = row->getChar(DIFFCALCIND);
    misc->refundcalcInd() = row->getChar(REFUNDCALCIND);
    misc->proportionalInd() = row->getChar(PROPORTIONALIND);
    misc->curradjustInd() = row->getChar(CURRADJUSTIND);
    misc->fareClassType1Ind() = row->getChar(FARECLASSTYPE1IND);
    misc->fareClassType1() = row->getString(FARECLASSTYPE1);
    misc->fareClassType2Ind() = row->getChar(FARECLASSTYPE2IND);
    misc->fareClassType2() = row->getString(FARECLASSTYPE2);
    misc->geoTblItemNo() = row->getInt(GEOTBLITEMNO);
    misc->inhibit() = row->getChar(INHIBIT);

    return misc;
  } // mapRowToMiscFareTag()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMiscFareTagSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMiscFareTagHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMiscFareTagHistoricalSQLStatement : public QueryGetMiscFareTagSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetMiscFareTagHistoricalSQLStatement
} // tse
