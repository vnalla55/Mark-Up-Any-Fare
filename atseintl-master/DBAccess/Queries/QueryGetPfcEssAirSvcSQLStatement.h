//----------------------------------------------------------------------------
//          File:           QueryGetPfcEssAirSvcSQLStatement.h
//          Description:    QueryGetPfcEssAirSvcSQLStatement
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
#include "DBAccess/Queries/QueryGetPfcEssAirSvc.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetPfcEssAirSvcSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPfcEssAirSvcSQLStatement() {}
  virtual ~QueryGetPfcEssAirSvcSQLStatement() {}

  enum ColumnIndexes
  {
    EASHUBARPT = 0,
    EASARPT,
    EFFDATE,
    CREATEDATE,
    EXPIREDATE,
    DISCDATE,
    SEGCNT,
    VENDOR,
    ORDERNO,
    EASCARRIER,
    FLT1,
    FLT2,
    VENDP,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select a.EASHUBARPT,a.EASARPT,a.EFFDATE,a.CREATEDATE,EXPIREDATE,"
                  "       DISDATE DISCDATE,SEGCNT,a.VENDOR,p.ORDERNO,EASCARRIER,"
                  "       FLT1,FLT2,p.VENDOR VENDP,INHIBIT");

    //		        this->From(" =ESSAIRSVC a LEFT OUTER JOIN =ESSAIRSVCPROVIDER p"
    //		                  " USING (EASHUBARPT,EASARPT,EFFDATE,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("EASHUBARPT");
    joinFields.push_back("EASARPT");
    joinFields.push_back("EFFDATE");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=ESSAIRSVC", "a", "LEFT OUTER JOIN", "=ESSAIRSVCPROVIDER", "p", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where(" VALIDITYIND = 'Y'"
                "    and INHIBIT = 'N' "
                "    and a.EASHUBARPT = %1q "
                "    and a.EASARPT = %2q "
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("EASHUBARPT, EASARPT, EFFDATE, CREATEDATE, ORDERNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::PfcEssAirSvc* mapRowToPfcEssAirSvc(Row* row, PfcEssAirSvc* easPrev)
  {
    LocCode easHubArpt = row->getString(EASHUBARPT);
    LocCode easArpt = row->getString(EASARPT);
    DateTime effDate;
    effDate = row->getDate(EFFDATE);
    DateTime createDate = row->getDate(CREATEDATE);

    PfcEssAirSvc* eas;

    // If Parent hasn't changed, add to Child (segs)
    if (easPrev != nullptr && easPrev->easHubArpt() == easHubArpt && easPrev->easArpt() == easArpt &&
        easPrev->effDate() == effDate && easPrev->createDate() == createDate)
    { // Add to Prev
      eas = easPrev;
    } // Previous Parent
    else
    { // Time for a new Parent
      eas = new tse::PfcEssAirSvc;
      eas->easHubArpt() = easHubArpt;
      eas->easArpt() = easArpt;
      eas->effDate() = effDate;
      eas->createDate() = createDate;

      eas->expireDate() = row->getDate(EXPIREDATE);
      eas->discDate() = row->getDate(DISCDATE);
      eas->segCnt() = row->getInt(SEGCNT);
      eas->vendor() = row->getString(VENDOR);
      eas->inhibit() = row->getChar(INHIBIT);
    }
    if (!row->isNull(ORDERNO))
    {
      PfcEssAirSvcProv* newASP = new PfcEssAirSvcProv;

      newASP->orderNo() = row->getInt(ORDERNO);
      newASP->easCarrier() = row->getString(EASCARRIER);
      newASP->flt1() = QUERYCLASS::checkFlightWildCard(row->getString(FLT1));
      newASP->flt2() = QUERYCLASS::checkFlightWildCard(row->getString(FLT2));
      newASP->vendor() = row->getString(VENDP);

      eas->asProvs().push_back(newASP);
    }
    return eas;
  } // mapRowToPfcEssAirSvc()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllPfcEssAirSvcSQLStatement : public QueryGetPfcEssAirSvcSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'"
                "    and INHIBIT = 'N' ");
    this->OrderBy("EASHUBARPT, EASARPT, EFFDATE, CREATEDATE, ORDERNO");
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template to adjust the SQL for QueryGetAllPfcEssAirSvcHistorical
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetPfcEssAirSvcHistoricalSQLStatement
    : public QueryGetPfcEssAirSvcSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where(" VALIDITYIND = 'Y'"
                "    and a.EASHUBARPT = %1q "
                "    and a.EASARPT = %2q "
                "    and %3n <= a.EXPIREDATE"
                "    and (%4n >= a.CREATEDATE"
                "     or %5n >= a.EFFDATE)");
  }
};
}

