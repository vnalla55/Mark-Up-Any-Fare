//----------------------------------------------------------------------------
//          File:           QueryGetMultiTransportSQLStatement.h
//          Description:    QueryGetMultiTransportSQLStatement
//          Created:        10/26/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     [C] 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMultiTransport.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMultiTransportSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMultiTransportSQLStatement() {};
  virtual ~QueryGetMultiTransportSQLStatement() {};

  enum ColumnIndexes
  {
    MULTITRANSCITY = 0,
    CARRIER,
    MULTITRANSLOC,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    DOMAPPL,
    INTLAPPL,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select MULTITRANSCITY,CARRIER,MULTITRANSLOC,CREATEDATE,EXPIREDATE,"
                  "       EFFDATE,DISCDATE,DOMAPPL,INTLAPPL");
    this->From("=MULTITRANSPORT ");
    this->Where("MULTITRANSCITY = %1q "
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("MULTITRANSCITY,CARRIER,MULTITRANSLOC,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MultiTransport* mapRowToMultiTransport(Row* row)
  {
    tse::MultiTransport* mT = new tse::MultiTransport;

    mT->multitranscity() = row->getString(MULTITRANSCITY);
    mT->carrier() = row->getString(CARRIER);
    mT->multitransLoc() = row->getString(MULTITRANSLOC);
    mT->createDate() = row->getDate(CREATEDATE);
    mT->expireDate() = row->getDate(EXPIREDATE);
    mT->effDate() = row->getDate(EFFDATE);
    mT->discDate() = row->getDate(DISCDATE);
    mT->domAppl() = row->getChar(DOMAPPL);
    mT->intlAppl() = row->getChar(INTLAPPL);

    return mT;
  } // mapRowToMultiTransport()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMultiTransportSQLStatement : public QueryGetMultiTransportSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("MULTITRANSCITY,CARRIER,MULTITRANSLOC,CREATEDATE");
    else
      this->OrderBy("MULTITRANSCITY");
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMultiTransportCitySQLStatement : public QueryGetMultiTransportSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("MULTITRANSLOC = %1q "
                "    and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("MULTITRANSLOC,MULTITRANSCITY,CARRIER,CREATEDATE");
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMultiTransportCitySQLStatement
    : public QueryGetMultiTransportSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("MULTITRANSLOC,MULTITRANSCITY,CARRIER,CREATEDATE");
    else
      this->OrderBy("MULTITRANSLOC");
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMultiTransportLocsSQLStatement : public QueryGetMultiTransportSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMultiTransportLocsSQLStatement
    : public QueryGetMultiTransportSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("MULTITRANSCITY,CARRIER,MULTITRANSLOC,CREATEDATE");
    else
      this->OrderBy("MULTITRANSCITY");
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template to adjust the SQL for QueryGetMultiTransportLocsHistoricalSQLStatement
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMultiTransportLocsHistoricalSQLStatement
    : public QueryGetMultiTransportSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("MULTITRANSCITY = %1q "
                "    and %2n <= EXPIREDATE"
                "    and (%3n >= CREATEDATE"
                "     or %4n >= EFFDATE)");
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template to adjust the SQL for QueryGetMultiTransportHistoricalSQLStatement
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMultiTransportHistoricalSQLStatement
    : public QueryGetMultiTransportSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("MULTITRANSCITY = %1q "
                "    and %2n <= EXPIREDATE"
                "    and (%3n >= CREATEDATE"
                "     or %4n >= EFFDATE)");
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template to adjust the SQL for QueryGetMultiTransportCityHistorical
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMultiTransportCityHistoricalSQLStatement
    : public QueryGetMultiTransportSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("MULTITRANSLOC = %1q "
                "    and %2n <= EXPIREDATE"
                "    and (%3n >= CREATEDATE"
                "     or %4n >= EFFDATE)");
    this->OrderBy("CREATEDATE");
  }
};

} // tse
