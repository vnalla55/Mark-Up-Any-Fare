//----------------------------------------------------------------------------
//      File:         QueryGetSeatCabinCharacteristicSQLStatement.h
//      Description:  QueryGetSeatCabinCharacteristicSQLStatement
//      Created:      10/10/2012
//      Authors:      Ram Papineni
//
//      Updates:
//
//   (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//   the confidential and proprietary product of Sabre Inc. Any unauthorized
//   use, reproduction, or transfer of this software/documentation, in any
//   medium, or incorporation of this software/documentation into any system
//   or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSeatCabinCharacteristic.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetSeatCabinCharacteristicSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSeatCabinCharacteristicSQLStatement() {};
  virtual ~QueryGetSeatCabinCharacteristicSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CODETYPE,
    SEATCABINCODE,
    CODEDESCRIPTION,
    DISPLAYDESCRIPTION,
    ABBREVIATEDDESCRIPTION,
    CREATEDATE,
    EXPIREDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER, CODETYPE, SEATCABINCODE, CODEDESCRIPTION, DISPLAYDESCRIPTION, "
                  "ABBREVIATEDDESCRIPTION, CREATEDATE, EXPIREDATE");
    this->From("=SEATCABINCHARACTERISTIC");
    this->Where("CARRIER = %1q"
                " and CODETYPE = %2q"
                " and %cd <= EXPIREDATE");
    this->OrderBy("CREATEDATE, CARRIER");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();
    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static SeatCabinCharacteristicInfo* mapRowToSeatCabinCharacteristicInfo(Row* row)
  {
    SeatCabinCharacteristicInfo* info = new SeatCabinCharacteristicInfo;

    info->carrier() = row->getString(CARRIER);
    info->codeType() = row->getChar(CODETYPE);
    info->seatCabinCode() = row->getString(SEATCABINCODE);
    info->codeDescription() = row->getString(CODEDESCRIPTION);
    info->displayDescription() = row->getString(DISPLAYDESCRIPTION);
    info->abbreviatedDescription() = row->getString(ABBREVIATEDDESCRIPTION);
    info->createDate() = row->getDate(CREATEDATE);
    info->expireDate() = row->getDate(EXPIREDATE);

    return info;
  } // mapRowToSeatCabinCharacteristicInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetSeatCabinCharacteristicSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace From and Where clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSeatCabinCharacteristicHistoricalSQLStatement
    : public QueryGetSeatCabinCharacteristicSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER = %1q"
                " and CODETYPE = %2q"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
    this->OrderBy("CREATEDATE, CARRIER");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetSeatCabinCharacteristicHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllSeatCabinCharacteristic
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllSeatCabinCharacteristicSQLStatement
    : public QueryGetSeatCabinCharacteristicSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("CREATEDATE, CARRIER");
  }
}; // QueryGetAllSeatCabinCharacteristicSQLStatement
} // tse
