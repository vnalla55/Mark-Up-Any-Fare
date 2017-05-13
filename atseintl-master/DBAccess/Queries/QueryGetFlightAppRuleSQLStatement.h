//----------------------------------------------------------------------------
//          File:           QueryGetFlightAppRuleSQLStatement.h
//          Description:    QueryGetFlightAppRuleSQLStatement
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
#include "DBAccess/Queries/QueryGetFlightAppRule.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFlightAppRuleSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFlightAppRuleSQLStatement() {};
  virtual ~QueryGetFlightAppRuleSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    CARRIERFLTTBLITEMNO1,
    CARRIERFLTTBLITEMNO2,
    GEOTBLITEMNOBETWVIA,
    GEOTBLITEMNOANDVIA,
    GEOTBLITEMNOVIA,
    FLTAPPL,
    FLT1,
    CARRIER1,
    FLTRELATIONAL1,
    FLT2,
    CARRIER2,
    FLTRELATIONAL2,
    FLT3,
    CARRIER3,
    DOW,
    INOUTIND,
    GEOAPPL,
    LOCAPPL,
    VIAIND,
    HIDDEN,
    FLTNONSTOP,
    FLTDIRECT,
    FLTMULTISTOP,
    FLTONESTOP,
    FLTONLINE,
    FLTINTERLINE,
    FLTSAME,
    EQUIPAPPL,
    EQUIPTYPE,
    UNAVAILTAG,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,TEXTTBLITEMNO,"
                  " OVERRIDEDATETBLITEMNO,CARRIERFLTTBLITEMNO1,CARRIERFLTTBLITEMNO2,"
                  " GEOTBLITEMNOBETWVIA,GEOTBLITEMNOANDVIA,GEOTBLITEMNOVIA,FLTAPPL,FLT1,"
                  " CARRIER1,FLTRELATIONAL1,FLT2,CARRIER2,FLTRELATIONAL2,FLT3,CARRIER3,"
                  " DOW,INOUTIND,GEOAPPL,LOCAPPL,VIAIND,HIDDEN,FLTNONSTOP,FLTDIRECT,"
                  " FLTMULTISTOP,FLTONESTOP,FLTONLINE,FLTINTERLINE,FLTSAME,EQUIPAPPL,"
                  " EQUIPTYPE,UNAVAILTAG");
    this->From("=FLIGHTAPP");
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::FlightAppRule* mapRowToFlightAppRule(Row* row)
  {
    tse::FlightAppRule* flightApp = new tse::FlightAppRule;

    flightApp->vendor() = row->getString(VENDOR);
    flightApp->itemNo() = row->getInt(ITEMNO);
    flightApp->createDate() = row->getDate(CREATEDATE);
    flightApp->expireDate() = row->getDate(EXPIREDATE);
    flightApp->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    flightApp->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    flightApp->carrierFltTblItemNo1() = row->getInt(CARRIERFLTTBLITEMNO1);
    flightApp->carrierFltTblItemNo2() = row->getInt(CARRIERFLTTBLITEMNO2);
    flightApp->geoTblItemNoBetwVia() = row->getInt(GEOTBLITEMNOBETWVIA);
    flightApp->geoTblItemNoAndVia() = row->getInt(GEOTBLITEMNOANDVIA);
    flightApp->geoTblItemNoVia() = row->getInt(GEOTBLITEMNOVIA);
    flightApp->fltAppl() = row->getChar(FLTAPPL);
    flightApp->flt1() = checkFlightWildCard(row->getString(FLT1));
    flightApp->carrier1() = row->getString(CARRIER1);
    flightApp->fltRelational1() = row->getChar(FLTRELATIONAL1);
    flightApp->flt2() = checkFlightWildCard(row->getString(FLT2));
    flightApp->carrier2() = row->getString(CARRIER2);
    flightApp->fltRelational2() = row->getChar(FLTRELATIONAL2);
    flightApp->flt3() = checkFlightWildCard(row->getString(FLT3));
    flightApp->carrier3() = row->getString(CARRIER3);
    flightApp->dow() = row->getString(DOW);
    flightApp->inOutInd() = row->getChar(INOUTIND);
    flightApp->geoAppl() = row->getChar(GEOAPPL);
    flightApp->locAppl() = row->getChar(LOCAPPL);
    flightApp->viaInd() = row->getChar(VIAIND);
    flightApp->hidden() = row->getChar(HIDDEN);
    flightApp->fltNonStop() = row->getChar(FLTNONSTOP);
    flightApp->fltDirect() = row->getChar(FLTDIRECT);
    flightApp->fltMultiStop() = row->getChar(FLTMULTISTOP);
    flightApp->fltOneStop() = row->getChar(FLTONESTOP);
    flightApp->fltOnline() = row->getChar(FLTONLINE);
    flightApp->fltInterline() = row->getChar(FLTINTERLINE);
    flightApp->fltSame() = row->getChar(FLTSAME);
    flightApp->equipAppl() = row->getChar(EQUIPAPPL);
    flightApp->equipType() = row->getString(EQUIPTYPE);
    flightApp->unavailtag() = row->getChar(UNAVAILTAG);

    return flightApp;
  }; // mapRowToFlightAppRule()

  static int checkFlightWildCard(const char* fltStr)
  {
    if (fltStr[0] == '*')
      return -1;
    else
      return stringToInteger(fltStr, __LINE__); // lint !e668
  }; // checkFlightWildCard()

  static int stringToInteger(const char* stringVal, int lineNumber)
  {
    if (stringVal == nullptr)
    {
      throw std::runtime_error("Null pointer to int data");
    }
    else if (*stringVal == '-' || *stringVal == '+')
    {
      if (stringVal[1] < '0' || stringVal[1] > '9')
        return 0;
    }
    else if (*stringVal < '0' || *stringVal > '9')
    {
      return 0;
    }
    return atoi(stringVal);
  }; // stringToInteger()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetFlightAppRuleSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetFlightAppRuleHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFlightAppRuleHistoricalSQLStatement
    : public QueryGetFlightAppRuleSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                "  and ITEMNO = %2n"
                "  and VALIDITYIND = 'Y'"
                "  and %3n <= EXPIREDATE"
                "  and %4n >= CREATEDATE");
  }
}; // class QueryGetFlightAppRuleHistoricalSQLStatement
} // tse
