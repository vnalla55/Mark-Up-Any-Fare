//----------------------------------------------------------------------------
//          File:           QueryGetFreeBaggageSQLStatement.h
//          Description:    QueryGetFreeBaggageSQLStatement
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

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFreeBaggage.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetFreeBaggageSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFreeBaggageSQLStatement() {}

  virtual ~QueryGetFreeBaggageSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    LOCKDATE,
    EFFDATE,
    DISCDATE,
    MEMONO,
    NEWSEQNO,
    UNITS,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    GLOBALAPPL,
    GLOBALDIR,
    EQUIPMENTCODE,
    MEASUREMENTTYPE,
    PSGTYPE,
    PSGTYPEINFANT,
    PSGTYPECHILD,
    BOOKINGCODE,
    FARETYPE,
    CABIN,
    CREATORBUSINESSUNIT,
    CREATORID,
    VERSIONINHERITEDIND,
    VERSIONDISPLAYIND,
    VENDOR,
    RULETARIFF,
    RULE,
    FARECLASS,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select CARRIER, VERSIONDATE, SEQNO, CREATEDATE, EXPIREDATE,"
        "       LOCKDATE, EFFDATE, DISCDATE, MEMONO, NEWSEQNO,"
        "       UNITS, DIRECTIONALITY, LOC1TYPE, LOC1, LOC2TYPE,"
        "       LOC2, GLOBALAPPL, GLOBALDIR, EQUIPMENTCODE, MEASUREMENTTYPE,"
        "       PSGTYPE, PSGTYPEINFANT, PSGTYPECHILD, BOOKINGCODE, FARETYPE,"
        "       CABIN, CREATORBUSINESSUNIT, CREATORID, VERSIONINHERITEDIND, VERSIONDISPLAYIND,"
        "       VENDOR, RULETARIFF, RULE, FARECLASS");

    this->From("=FREEBAGGAGE");
    this->Where("CARRIER = %1q and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER,VERSIONDATE,SEQNO,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::FreeBaggageInfo* mapRowToFreeBaggage(Row* row)
  {
    Indicator cabin = row->getChar(CABIN);

    tse::FreeBaggageInfo* bag = new tse::FreeBaggageInfo;

    bag->carrier() = row->getString(CARRIER);
    bag->createDate() = row->getDate(CREATEDATE);
    bag->effDate() = row->getDate(EFFDATE);
    bag->expireDate() = row->getDate(EXPIREDATE);
    bag->seqNo() = row->getLong(SEQNO);
    bag->units() = row->getInt(UNITS);
    bag->directionality() = row->getChar(DIRECTIONALITY);
    bag->loc1().locType() = row->getChar(LOC1TYPE);
    bag->loc1().loc() = row->getString(LOC1);
    bag->loc2().locType() = row->getChar(LOC2TYPE);
    bag->loc2().loc() = row->getString(LOC2);
    bag->globalAppl() = row->getChar(GLOBALAPPL);
    strToGlobalDirection(bag->globalDir(), row->getString(GLOBALDIR));
    bag->equipmentCode() = row->getString(EQUIPMENTCODE);
    bag->measurementType() = row->getChar(MEASUREMENTTYPE);
    bag->psgType() = row->getString(PSGTYPE);
    bag->psgTypeInfant() = row->getChar(PSGTYPEINFANT);
    bag->psgTypeChild() = row->getChar(PSGTYPECHILD);
    bag->bookingCode() = row->getString(BOOKINGCODE);
    bag->fareType() = row->getString(FARETYPE);
    bag->cabin().setClass(cabin);
    bag->vendorCode() = row->getString(VENDOR);
    bag->tariffNumber() = row->getInt(RULETARIFF);
    bag->ruleNumber() = row->getString(RULE);
    bag->fareClassCode() = row->getString(FARECLASS);

    return bag;
  }; // mapRowToFreeBaggage()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetFreeBaggageSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFreeBaggageHistoricalSQLStatement : public QueryGetFreeBaggageSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" CARRIER = %1q"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  };
}; // class QueryGetFreeBaggageHistoricalSQLStatement
}
