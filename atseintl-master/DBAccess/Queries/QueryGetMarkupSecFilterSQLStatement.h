//----------------------------------------------------------------------------
//          File:           QueryGetMarkupSecFilterSQLStatement.h
//          Description:    QueryGetMarkupSecFilterSQLStatement
//          Created:        10/26/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//      2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMarkupSecFilter.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMarkupSecFilterSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMarkupSecFilterSQLStatement() {};
  virtual ~QueryGetMarkupSecFilterSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    CREATEDATE,
    EXPIREDATE,
    RULETARIFF,
    RULE,
    PSEUDOCITYIND,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    IATATVLAGENCYNO,
    CARRIERCRS,
    LOCTYPE,
    LOC,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,CARRIER,CREATEDATE, EXPIREDATE,"
                  "       RULETARIFF,RULE,PSEUDOCITYIND,PSEUDOCITYTYPE,"
                  "       PSEUDOCITY,IATATVLAGENCYNO,CARRIERCRS,LOCTYPE,LOC");
    this->From("=MARKUPSECFILTER");
    this->Where("VENDOR = %1q"
                "    and CARRIER = %2q"
                "    and RULETARIFF = %3n"
                "    and RULE = %4q"
                "    and (   (CARRIERCRS not like '1%')"
                "         or (CARRIERCRS = '1S')"
                "         or (CARRIERCRS = '1J')"
                "         or (CARRIERCRS = '1B')"
                "         or (CARRIERCRS = '1F'))"
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR , CARRIER, RULETARIFF, RULE, PSEUDOCITYIND, PSEUDOCITYTYPE, "
                    "PSEUDOCITY, IATATVLAGENCYNO, CARRIERCRS, LOCTYPE, LOC, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MarkupSecFilter* mapRowToMarkupSecFilter(Row* row)
  {
    tse::MarkupSecFilter* msf = new tse::MarkupSecFilter;

    msf->vendor() = row->getString(VENDOR);
    msf->carrier() = row->getString(CARRIER);
    msf->createDate() = row->getDate(CREATEDATE);
    msf->expireDate() = row->getDate(EXPIREDATE);
    msf->ruleTariff() = row->getInt(RULETARIFF);
    msf->rule() = row->getString(RULE);
    msf->pseudoCityInd() = row->getChar(PSEUDOCITYIND);
    msf->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    msf->pseudoCity() = row->getString(PSEUDOCITY);
    msf->iataTvlAgencyNo() = row->getString(IATATVLAGENCYNO);
    msf->carrierCrs() = row->getString(CARRIERCRS);

    LocKey* loc = &msf->loc();
    loc->locType() = row->getChar(LOCTYPE);
    loc->loc() = row->getString(LOC);

    return msf;
  } // mapRowToMarkupSecFilter()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMarkupSecFilterSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMarkupSecFilterHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMarkupSecFilterHistoricalSQLStatement
    : public QueryGetMarkupSecFilterSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select VENDOR,CARRIER,CREATEDATE,EXPIREDATE,"
                             "       RULETARIFF,RULE,PSEUDOCITYIND,PSEUDOCITYTYPE,"
                             "       PSEUDOCITY,IATATVLAGENCYNO,CARRIERCRS,LOCTYPE,LOC");
    partialStatement.From("=MARKUPSECFILTERH ");
    partialStatement.Where("VENDOR = %1q"
                           "    and CARRIER = %2q"
                           "    and RULETARIFF = %3n"
                           "    and RULE = %4q"
                           "    and (   (CARRIERCRS not like '1%')"
                           "         or (CARRIERCRS = '1S')"
                           "         or (CARRIERCRS = '1J')"
                           "         or (CARRIERCRS = '1B'))"
                           "    and %5n >= CREATEDATE"
                           "    and %6n <= EXPIREDATE"
                           ")");

    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select VENDOR,CARRIER,CREATEDATE,EXPIREDATE,"
                             "       RULETARIFF,RULE,PSEUDOCITYIND,PSEUDOCITYTYPE,"
                             "       PSEUDOCITY,IATATVLAGENCYNO,CARRIERCRS,LOCTYPE,LOC");
    partialStatement.From("=MARKUPSECFILTER ");
    partialStatement.Where("VENDOR = %7q"
                           "    and CARRIER = %8q"
                           "    and RULETARIFF = %9n"
                           "    and RULE = %10q"
                           "    and (   (CARRIERCRS not like '1%')"
                           "         or (CARRIERCRS = '1S')"
                           "         or (CARRIERCRS = '1J')"
                           "         or (CARRIERCRS = '1B'))"
                           "    and %11n >= CREATEDATE"
                           "    and %12n <= EXPIREDATE"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}

}; // class QueryGetMarkupSecFilterHistoricalSQLStatement
} // tse
