//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareTypeTable.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareTypeTableSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    FARETYPE,
    EXPIREDATE,
    INHIBIT,
    FARETYPEAPPL
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, CREATEDATE, FARETYPE, EXPIREDATE, "
                  "       INHIBIT, FARETYPEAPPL");
    this->From("=FARETYPE974 ");
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,CREATEDATE,FARETYPE");

    adjustBaseSQL();

    this->ConstructSQL();

    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareTypeTable* mapRowToFareTypeTable(Row* row)
  {
    FareTypeTable* fareTypeTable = new FareTypeTable;

    fareTypeTable->vendor() = row->getString(VENDOR);
    fareTypeTable->itemNo() = row->getInt(ITEMNO);
    fareTypeTable->createDate() = row->getDate(CREATEDATE);
    fareTypeTable->fareType() = row->getString(FARETYPE);
    fareTypeTable->expireDate() = row->getDate(EXPIREDATE);
    fareTypeTable->inhibit() = row->getChar(INHIBIT);
    fareTypeTable->fareTypeAppl() = row->getChar(FARETYPEAPPL);

    return fareTypeTable;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetAllFareTypeTableSQLStatement : public QueryGetFareTypeTableSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE"
                " and VALIDITYIND = 'Y'");
  }
};

template <class QUERYCLASS>
class QueryGetFareTypeTableHistoricalSQLStatement
    : public QueryGetFareTypeTableSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'");
  }
};
}
