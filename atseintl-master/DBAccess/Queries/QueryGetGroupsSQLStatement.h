//----------------------------------------------------------------------------
//          File:           QueryGetGroupsSQLStatement.h
//          Description:    QueryGetGroupsSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetGroups.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetGroupsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetGroupsSQLStatement() {};
  virtual ~QueryGetGroupsSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    FLTMAXPERCENTNODEC,
    FLTMAXPERCENT,
    CHILDPERCENTNODEC,
    CHILDPERCENT,
    ADDLPERCENTNODEC,
    ADDLPERCENT,
    SUBSTPERCENTNODEC,
    SUBSTPERCENT,
    INDVPERCENTNODEC,
    INDVPERCENT,
    NAMEDATE,
    INDVGEOTBLITEMNOBETW,
    INDVGEOTBLITEMNOAND,
    ASSEMBLYGEOTBLITEMNO,
    GRPGEOTBLITEMNOBETW,
    GRPGEOTBLITEMNOAND,
    INDVNODEC1,
    INDVAMT1,
    INDVNODEC2,
    INDVAMT2,
    GEOTBLITEMNOBETWEEN,
    TOURGEOTBLITEMNOBETW,
    TOURGEOTBLITEMNOAND,
    OVERRIDEDATETBLITEMNO,
    TEXTTBLITEMNO,
    GEOTBLITEMNOAND,
    CHILDNO,
    NAMETOD,
    INDVTOD,
    ADDLTOD,
    SUBSTMAXNO,
    SUBSTTOD,
    MINNOSECTORS,
    ADDLTKTTOD,
    SUBSTTKTTOD,
    TOURMINNO,
    TOURMAXNO,
    NOTOURCONDUCTORS,
    NOPAYINGPSGRS,
    MAXNOTOURCONDUCTORS,
    TOURCONDUCTORMINAGE,
    ASSEMBLYMINNO,
    VALIDITYIND,
    INHIBIT,
    UNAVAILTAG,
    GROUPTYPE,
    MINGROUPSIZE,
    MAXGROUPSIZE,
    FARECLASSGROUPIND,
    FLTMAXNOPSGR,
    FLTMAXNO,
    LOCVALIDITYIND,
    LOCALEAPPLIND,
    CHILDFARECLASS,
    CHILDTYPE,
    NAMEREQ,
    NAMEPERIOD,
    NAMEUNIT,
    NAMEAPPL,
    ITINERARY,
    PNR,
    FARECHARGEIND,
    ADDLMAXNO,
    ADDLPERIOD,
    ADDLUNIT,
    ADDLTKTPERIOD,
    ADDLTKTUNIT,
    ADDLTKTIND,
    SUBSTPERIOD,
    SUBSTUNIT,
    SUBSTTKTPERIOD,
    SUBSTTKTUNIT,
    SUBSTTKTIND,
    TVLCONDOUT,
    TVLCONDINB,
    TVLCONDEITHER,
    GRPTIME,
    GRPWAIVER,
    INDVPERMITTED,
    INDVSAME,
    INDVPERIOD,
    INDVUNIT,
    INDVINOUTIND,
    INDVCURR1,
    INDVCURR2,
    ASSEMBLYRETURNIND,
    TOURCONDUCTORTVLIND,
    TOURCONDUCTORDIFFIND,
    TOURCONDUCTORAPPIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,FLTMAXPERCENTNODEC,"
                  " FLTMAXPERCENT,CHILDPERCENTNODEC,CHILDPERCENT,ADDLPERCENTNODEC,"
                  " ADDLPERCENT,SUBSTPERCENTNODEC,SUBSTPERCENT,INDVPERCENTNODEC,"
                  " INDVPERCENT,NAMEDATE,INDVGEOTBLITEMNOBETW,INDVGEOTBLITEMNOAND,"
                  " ASSEMBLYGEOTBLITEMNO,GRPGEOTBLITEMNOBETW,GRPGEOTBLITEMNOAND,"
                  " INDVNODEC1,INDVAMT1,INDVNODEC2,INDVAMT2,GEOTBLITEMNOBETWEEN,"
                  " TOURGEOTBLITEMNOBETW,TOURGEOTBLITEMNOAND,OVERRIDEDATETBLITEMNO,"
                  " TEXTTBLITEMNO,GEOTBLITEMNOAND,CHILDNO,NAMETOD,INDVTOD,ADDLTOD,"
                  " SUBSTMAXNO,SUBSTTOD,MINNOSECTORS,ADDLTKTTOD,SUBSTTKTTOD,TOURMINNO,"
                  " TOURMAXNO,NOTOURCONDUCTORS,NOPAYINGPSGRS,MAXNOTOURCONDUCTORS,"
                  " TOURCONDUCTORMINAGE,ASSEMBLYMINNO,VALIDITYIND,INHIBIT,UNAVAILTAG,"
                  " GROUPTYPE,MINGROUPSIZE,MAXGROUPSIZE,FARECLASSGROUPIND,FLTMAXNOPSGR,"
                  " FLTMAXNO,LOCVALIDITYIND,LOCALEAPPLIND,CHILDFARECLASS,CHILDTYPE,"
                  " NAMEREQ,NAMEPERIOD,NAMEUNIT,NAMEAPPL,ITINERARY,PNR,FARECHARGEIND,"
                  " ADDLMAXNO,ADDLPERIOD,ADDLUNIT,ADDLTKTPERIOD,ADDLTKTUNIT,"
                  " ADDLTKTIND,SUBSTPERIOD,SUBSTUNIT,SUBSTTKTPERIOD,SUBSTTKTUNIT,"
                  " SUBSTTKTIND,TVLCONDOUT,TVLCONDINB,TVLCONDEITHER,GRPTIME,GRPWAIVER,"
                  " INDVPERMITTED,INDVSAME,INDVPERIOD,INDVUNIT,INDVINOUTIND,INDVCURR1,"
                  " INDVCURR2,ASSEMBLYRETURNIND,TOURCONDUCTORTVLIND,"
                  " TOURCONDUCTORDIFFIND,TOURCONDUCTORAPPIND ");
    this->From("=GROUPS");
    this->Where("VENDOR = %1q "
                " and ITEMNO = %2n "
                " and EXPIREDATE >= %cd "
                " and VALIDITYIND = 'Y'");
    this->OrderBy("VENDOR,ITEMNO,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::Groups* mapRowToGroups(Row* row)
  {
    Groups* rec = new Groups();

    rec->vendor() = row->getString(VENDOR);
    rec->itemNo() = row->getInt(ITEMNO);
    rec->createDate() = row->getDate(CREATEDATE);
    rec->expireDate() = row->getDate(EXPIREDATE);

    rec->fltMaxPercentNoDec() = row->getInt(FLTMAXPERCENTNODEC);
    rec->fltMaxPercent() =
        QUERYCLASS::adjustDecimal(row->getInt(FLTMAXPERCENT), rec->fltMaxPercentNoDec());

    rec->childPercentNoDec() = row->getInt(CHILDPERCENTNODEC);
    rec->childPercent() =
        QUERYCLASS::adjustDecimal(row->getInt(CHILDPERCENT), rec->childPercentNoDec());

    rec->addlPercentNoDec() = row->getInt(ADDLPERCENTNODEC);
    rec->addlPercent() =
        QUERYCLASS::adjustDecimal(row->getInt(ADDLPERCENT), rec->addlPercentNoDec());

    rec->substPercentNoDec() = row->getInt(SUBSTPERCENTNODEC);
    rec->substPercent() =
        QUERYCLASS::adjustDecimal(row->getInt(SUBSTPERCENT), rec->substPercentNoDec());

    rec->indvPercentNoDec() = row->getInt(INDVPERCENTNODEC);
    rec->indvPercent() =
        QUERYCLASS::adjustDecimal(row->getInt(INDVPERCENT), rec->indvPercentNoDec());

    rec->nameDate() = row->getDate(NAMEDATE);
    rec->indvGeoTblItemNoBetw() = row->getInt(INDVGEOTBLITEMNOBETW);
    rec->indvGeoTblItemNoAnd() = row->getInt(INDVGEOTBLITEMNOAND);
    rec->assemblyGeoTblItemNo() = row->getInt(ASSEMBLYGEOTBLITEMNO);
    rec->grpGeoTblItemNoBetw() = row->getInt(GRPGEOTBLITEMNOBETW);
    rec->grpGeoTblItemNoAnd() = row->getInt(GRPGEOTBLITEMNOAND);

    rec->indvNoDec1() = row->getInt(INDVNODEC1);
    rec->indvAmt1() = QUERYCLASS::adjustDecimal(row->getInt(INDVAMT1), rec->indvNoDec1());

    rec->indvNoDec2() = row->getInt(INDVNODEC2);
    rec->indvAmt2() = QUERYCLASS::adjustDecimal(row->getInt(INDVAMT2), rec->indvNoDec2());

    rec->geoTblItemNoBetween() = row->getInt(GEOTBLITEMNOBETWEEN);
    rec->tourGeoTblItemNoBetw() = row->getInt(TOURGEOTBLITEMNOBETW);
    rec->tourGeoTblItemNoAnd() = row->getInt(TOURGEOTBLITEMNOAND);
    rec->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    rec->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    rec->geoTblItemNoAnd() = row->getInt(GEOTBLITEMNOAND);
    rec->childNo() = row->getInt(CHILDNO);
    rec->nameTOD() = row->getInt(NAMETOD);
    rec->indvTOD() = row->getInt(INDVTOD);
    rec->addlTOD() = row->getInt(ADDLTOD);
    rec->substMaxNo() = row->getInt(SUBSTMAXNO);
    rec->substTOD() = row->getInt(SUBSTTOD);
    rec->minNoSectors() = row->getInt(MINNOSECTORS);
    rec->addlTktTOD() = row->getInt(ADDLTKTTOD);
    rec->substTktTOD() = row->getInt(SUBSTTKTTOD);
    rec->tourMinNo() = row->getInt(TOURMINNO);
    rec->tourMaxNo() = row->getInt(TOURMAXNO);
    rec->noTourConductors() = row->getInt(NOTOURCONDUCTORS);
    rec->noPayingPsgrs() = row->getInt(NOPAYINGPSGRS);
    rec->maxNoTourConductors() = row->getInt(MAXNOTOURCONDUCTORS);
    rec->tourConductorMinAge() = row->getInt(TOURCONDUCTORMINAGE);
    rec->assemblyMinNo() = row->getInt(ASSEMBLYMINNO);
    rec->validityInd() = row->getChar(VALIDITYIND);
    rec->inhibit() = row->getChar(INHIBIT);
    rec->unavailTag() = row->getChar(UNAVAILTAG);
    rec->groupType() = row->getString(GROUPTYPE);
    rec->minGroupSize() = row->getString(MINGROUPSIZE);
    rec->maxGroupSize() = row->getString(MAXGROUPSIZE);
    rec->fareClassGroupInd() = row->getChar(FARECLASSGROUPIND);
    rec->fltMaxNoPsgr() = row->getString(FLTMAXNOPSGR);
    rec->fltMaxNo() = row->getString(FLTMAXNO);
    rec->locValidityInd() = row->getChar(LOCVALIDITYIND);
    rec->localeApplInd() = row->getChar(LOCALEAPPLIND);
    rec->childFareClass() = row->getString(CHILDFARECLASS);
    rec->childType() = row->getChar(CHILDTYPE);
    rec->nameReq() = row->getChar(NAMEREQ);
    rec->namePeriod() = row->getString(NAMEPERIOD);
    rec->nameUnit() = row->getString(NAMEUNIT);
    rec->nameAppl() = row->getChar(NAMEAPPL);
    rec->itinerary() = row->getChar(ITINERARY);
    rec->pnr() = row->getChar(PNR);
    rec->fareChargeInd() = row->getChar(FARECHARGEIND);
    rec->addlMaxNo() = row->getString(ADDLMAXNO);
    rec->addlPeriod() = row->getString(ADDLPERIOD);
    rec->addlUnit() = row->getString(ADDLUNIT);
    rec->addlTktPeriod() = row->getString(ADDLTKTPERIOD);
    rec->addlTktunit() = row->getString(ADDLTKTUNIT);
    rec->addlTktInd() = row->getChar(ADDLTKTIND);
    rec->substPeriod() = row->getString(SUBSTPERIOD);
    rec->substUnit() = row->getString(SUBSTUNIT);
    rec->substTktPeriod() = row->getString(SUBSTTKTPERIOD);
    rec->substTktUnit() = row->getString(SUBSTTKTUNIT);
    rec->substTktInd() = row->getChar(SUBSTTKTIND);
    rec->tvlCondOut() = row->getChar(TVLCONDOUT);
    rec->tvlCondInb() = row->getChar(TVLCONDINB);
    rec->tvlCondEither() = row->getChar(TVLCONDEITHER);
    rec->grpTime() = row->getChar(GRPTIME);
    rec->grpWaiver() = row->getChar(GRPWAIVER);
    rec->indvPermitted() = row->getChar(INDVPERMITTED);
    rec->indvSame() = row->getChar(INDVSAME);
    rec->indvPeriod() = row->getString(INDVPERIOD);
    rec->indvUnit() = row->getString(INDVUNIT);
    rec->indvInOutInd() = row->getChar(INDVINOUTIND);
    rec->indvCurr1() = row->getString(INDVCURR1);
    rec->indvCurr2() = row->getString(INDVCURR2);
    rec->assemblyReturnInd() = row->getChar(ASSEMBLYRETURNIND);
    rec->tourConductorTvlInd() = row->getChar(TOURCONDUCTORTVLIND);
    rec->tourConductorDiffInd() = row->getChar(TOURCONDUCTORDIFFIND);
    rec->tourConductorAppInd() = row->getChar(TOURCONDUCTORAPPIND);

    return rec;
  } // mapRowToGroups()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetGroupsSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetGroupsHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetGroupsHistoricalSQLStatement : public QueryGetGroupsSQLStatement<QUERYCLASS>
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
}; // class QueryGetGroupsHistoricalSQLStatement
} // tse
