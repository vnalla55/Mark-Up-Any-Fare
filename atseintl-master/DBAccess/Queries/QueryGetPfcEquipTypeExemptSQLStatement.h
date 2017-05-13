//----------------------------------------------------------------------------
//          File:           QueryGetPfcEquipTypeExemptSQLStatement.h
//          Description:    QueryGetPfcEquipTypeExemptSQLStatement
//          Created:        10/8/2007
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
#include "DBAccess/Queries/QueryGetPfcEquipTypeExempt.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetPfcEquipTypeExemptSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPfcEquipTypeExemptSQLStatement() {}
  virtual ~QueryGetPfcEquipTypeExemptSQLStatement() {}

  enum ColumnIndexes
  {
    EQUIP = 0,
    STATE,
    EFFDATE,
    DISCDATE,
    CREATEDATE,
    EXPIREDATE,
    EQUIPNAME,
    NOSEATS,
    VENDOR,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum
  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select EQUIP,STATE,EFFDATE,DISDATE DISCDATE,CREATEDATE,"
                  "       EXPIREDATE,EQUIPNAME,NOSEATS,VENDOR,INHIBIT ");
    this->From("=PFCEQUIPTYPEEXEMPT ");
    this->Where("VALIDITYIND = 'Y'"
                "    and EQUIP = %1q "
                "    and STATE = %2q "
                "    and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("EQUIP,STATE,EFFDATE,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::PfcEquipTypeExempt* mapRowToPfcEquipTypeExempt(Row* row)
  {
    tse::PfcEquipTypeExempt* ete = new tse::PfcEquipTypeExempt;

    ete->equip() = row->getString(EQUIP);
    ete->state() = row->getString(STATE);
    ete->effDate() = row->getDate(EFFDATE);
    ete->createDate() = row->getDate(CREATEDATE);
    ete->expireDate() = row->getDate(EXPIREDATE);
    ete->discDate() = row->getDate(DISCDATE);
    ete->equipName() = row->getString(EQUIPNAME);
    ete->noSeats() = row->getInt(NOSEATS);
    ete->vendor() = row->getString(VENDOR);
    ete->inhibit() = row->getChar(INHIBIT);

    return ete;
  } // mapRowToPfcEquipTypeExempt()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllPfcEquipTypeExemptSQLStatement
    : public QueryGetPfcEquipTypeExemptSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y' and %cd <= EXPIREDATE");
    this->OrderBy("1, 2");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetPfcCollectMethHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetPfcEquipTypeExemptHistoricalSQLStatement
    : public QueryGetPfcEquipTypeExemptSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'"
                "    and EQUIP = %1q "
                "    and STATE = %2q "
                "    and %3n <= EXPIREDATE"
                "    and (%4n >= CREATEDATE"
                "     or %5n >= EFFDATE)");
    this->OrderBy("CREATEDATE");
  }
};
}

