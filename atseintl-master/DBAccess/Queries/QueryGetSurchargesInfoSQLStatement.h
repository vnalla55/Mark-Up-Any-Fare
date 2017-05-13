//----------------------------------------------------------------------------
//          File:           QueryGetSurchargesInfoSQLStatement.h
//          Description:    QueryGetSurchargesInfoSQLStatement
//          Created:        11/02/2007
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
#include "DBAccess/Queries/QueryGetSurchargesInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetSurchargesInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSurchargesInfoSQLStatement() {};
  virtual ~QueryGetSurchargesInfoSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    GEOTBLITEMNO,
    GEOTBLITEMNOBTW,
    GEOTBLITEMNOAND,
    SURCHARGENODEC1,
    SURCHARGEAMT1,
    SURCHARGENODEC2,
    SURCHARGEAMT2,
    STARTTIME,
    STOPTIME,
    SURCHARGEPERCENTNODEC,
    SURCHARGEPERCENT,
    STARTYEAR,
    STARTMONTH,
    STARTDAY,
    STOPYEAR,
    STOPMONTH,
    STOPDAY,
    TODAPPL,
    DOW,
    SURCHARGETYPE,
    EQUIPTYPE,
    TVLPORTION,
    SURCHARGEGROUPIND,
    SURCHARGEAPPL,
    SURCHARGECUR1,
    SURCHARGECUR2,
    SURCHARGEPERCENTAPPL,
    INHIBIT,
    SURCHARGEDESC,
    BOOKINGCODE,
    CARRIERFLTTBLITEMNO,
    SECTORPORTION,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,s.CREATEDATE,EXPIREDATE,UNAVAILTAG,TEXTTBLITEMNO,"
                  "       OVERRIDEDATETBLITEMNO,GEOTBLITEMNO,GEOTBLITEMNOBTW,"
                  "       GEOTBLITEMNOAND,SURCHARGENODEC1,SURCHARGEAMT1,SURCHARGENODEC2,"
                  "       SURCHARGEAMT2,STARTTIME,STOPTIME,SURCHARGEPERCENTNODEC,"
                  "       SURCHARGEPERCENT,STARTYEAR,STARTMONTH,STARTDAY,STOPYEAR,STOPMONTH,"
                  "       STOPDAY,TODAPPL,DOW,s.SURCHARGETYPE,EQUIPTYPE,TVLPORTION,"
                  "       SURCHARGEGROUPIND,SURCHARGEAPPL,SURCHARGECUR1,SURCHARGECUR2,"
                  "       SURCHARGEPERCENTAPPL,INHIBIT,d.SURCHARGEDESC,BOOKINGCODE,"
                  "       CARRIERFLTTBLITEMNO,SECTORPORTION");

    //		        this->From("=SURCHARGES s LEFT OUTER JOIN =SURCHARGEDESC d"
    //		                   "                      USING (SURCHARGETYPE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(1);
    joinFields.push_back("SURCHARGETYPE");
    this->generateJoinString(
        "=SURCHARGES", "s", "LEFT OUTER JOIN", "=SURCHARGEDESC", "d", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("s.VENDOR = %1q"
                "    and s.ITEMNO = %2n"
                "    and s.VALIDITYIND = 'Y'"
                "    and %cd <= s.EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("s.VENDOR, s.ITEMNO, s.CREATEDATE, d.SURCHARGETYPE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::SurchargesInfo* mapRowToSurchargesInfo(Row* row)
  {
    tse::SurchargesInfo* si = new tse::SurchargesInfo;

    si->vendor() = row->getString(VENDOR);
    si->itemNo() = row->getInt(ITEMNO);
    si->createDate() = row->getDate(CREATEDATE);
    si->expireDate() = row->getDate(EXPIREDATE);
    si->unavailTag() = row->getChar(UNAVAILTAG);
    si->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    si->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    si->geoTblItemNo() = row->getInt(GEOTBLITEMNO);
    si->geoTblItemNoBtw() = row->getInt(GEOTBLITEMNOBTW);
    si->geoTblItemNoAnd() = row->getInt(GEOTBLITEMNOAND);

    si->surchargeNoDec1() = row->getInt(SURCHARGENODEC1);
    si->surchargeAmt1() =
        QUERYCLASS::adjustDecimal(row->getInt(SURCHARGEAMT1), si->surchargeNoDec1());

    si->surchargeNoDec2() = row->getInt(SURCHARGENODEC2);
    si->surchargeAmt2() =
        QUERYCLASS::adjustDecimal(row->getInt(SURCHARGEAMT2), si->surchargeNoDec2());

    si->startTime() = row->getInt(STARTTIME);
    si->stopTime() = row->getInt(STOPTIME);

    si->surchargePercentNoDec() = row->getInt(SURCHARGEPERCENTNODEC);
    si->surchargePercent() =
        QUERYCLASS::adjustDecimal(row->getInt(SURCHARGEPERCENT), si->surchargePercentNoDec());

    uint32_t year = row->getInt(STARTYEAR);

    if (year > 0)
    {
      si->startYear() = adjustYear(year);
    }

    si->startMonth() = row->getInt(STARTMONTH);
    si->startDay() = row->getInt(STARTDAY);

    year = row->getInt(STOPYEAR);

    if (year > 0)
    {
      si->stopYear() = adjustYear(year);
    }

    si->stopMonth() = row->getInt(STOPMONTH);
    si->stopDay() = row->getInt(STOPDAY);
    si->todAppl() = row->getChar(TODAPPL);
    si->dow() = row->getString(DOW);
    si->surchargeType() = row->getChar(SURCHARGETYPE);
    si->equipType() = row->getString(EQUIPTYPE);
    si->tvlPortion() = row->getChar(TVLPORTION);
    si->surchargeGroupInd() = row->getChar(SURCHARGEGROUPIND);
    si->surchargeAppl() = row->getChar(SURCHARGEAPPL);
    si->surchargeCur1() = row->getString(SURCHARGECUR1);
    si->surchargeCur2() = row->getString(SURCHARGECUR2);
    si->surchargePercentAppl() = row->getChar(SURCHARGEPERCENTAPPL);
    si->inhibit() = row->getChar(INHIBIT);
    si->bookingCode() = row->getString(BOOKINGCODE);

    if (!row->isNull(SURCHARGEDESC))
      si->surchargeDesc() = row->getString(SURCHARGEDESC);

    si->carrierFltTblItemNo() = row->getInt(CARRIERFLTTBLITEMNO);
    si->sectorPortion() = row->getChar(SECTORPORTION);

    return si;
  } // mapRowToSurchargesInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}

  static int adjustYear(int year) { return (year > 80) ? (year + 1900) : (year + 2000); }
}; // class QueryGetSurchargesInfoSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetSurchargesInfoHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSurchargesInfoHistoricalSQLStatement
    : public QueryGetSurchargesInfoSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("s.VENDOR = %1q "
                " and s.ITEMNO = %2n"
                " and s.VALIDITYIND = 'Y'"
                " and %3n <= s.EXPIREDATE"
                " and %4n >= s.CREATEDATE");
  }
}; // class QueryGetSurchargesInfoHistoricalSQLStatement
} // tse
