//----------------------------------------------------------------------------
//          File:           QueryGetAccompaniedTravelInfoSQLStatement.h
//          Description:    QueryGetAccompaniedTravelInfoSQLStatement
//          Created:        10/25/2007
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
#include "DBAccess/Queries/QueryGetAccompaniedTravelInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetAccompaniedTravelInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAccompaniedTravelInfoSQLStatement() {};
  virtual ~QueryGetAccompaniedTravelInfoSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    SEGCNT,
    GEOTBLITEMNOVIA1,
    GEOTBLITEMNOVIA2,
    MINAGE,
    MAXAGE,
    MINNOPSG,
    MAXNOPSG,
    ACCTVLALLSECTORS,
    ACCTVLOUT,
    ACCTVLONESECTOR,
    ACCTVLSAMECPMT,
    ACCTVLSAMERULE,
    ACCPSGAPPL,
    ACCPSGTYPE,
    ACCPSGID,
    FARECLASSBKGCDIND,
    INHIBIT,
    FARECLASSBKGCD,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select a.VENDOR,a.ITEMNO,a.CREATEDATE,EXPIREDATE,UNAVAILTAG,"
                  "       TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,SEGCNT,GEOTBLITEMNOVIA1,"
                  "       GEOTBLITEMNOVIA2,MINAGE,MAXAGE,MINNOPSG,MAXNOPSG,ACCTVLALLSECTORS,"
                  "       ACCTVLOUT,ACCTVLONESECTOR,ACCTVLSAMECPMT,ACCTVLSAMERULE,ACCPSGAPPL,"
                  "       ACCPSGTYPE,ACCPSGID,FARECLASSBKGCDIND,INHIBIT,s.FARECLASSBKGCD");

    //		        this->From("=ACCOMPANIEDTRAVEL a LEFT OUTER JOIN =FARECLASSBKGCODESEG s"
    //		                   "                             USING (VENDOR,ITEMNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=ACCOMPANIEDTRAVEL",
                             "a",
                             "LEFT OUTER JOIN",
                             "=FARECLASSBKGCODESEG",
                             "s",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("a.VENDOR = %1q"
                "    and a.ITEMNO = %2n"
                "    and VALIDITYIND = 'Y'"
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("a.VENDOR, a.ITEMNO, a.CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::AccompaniedTravelInfo*
  mapRowToAccompaniedTravelInfo(Row* row, AccompaniedTravelInfo* atiPrev)
  { // Load up Parent Determinant Fields
    VendorCode vendor = row->getString(VENDOR);
    uint itemNo = row->getInt(ITEMNO);
    DateTime createDate = row->getDate(CREATEDATE);

    AccompaniedTravelInfo* ati;

    // If Parent hasn't changed, add to Children
    if (atiPrev != nullptr && atiPrev->vendor() == vendor && atiPrev->itemNo() == itemNo &&
        atiPrev->createDate() == createDate)
    { // Just add to Prev
      ati = atiPrev;
    }
    else
    { // Time for a new Parent
      ati = new tse::AccompaniedTravelInfo;
      ati->vendor() = vendor;
      ati->itemNo() = itemNo;
      ati->createDate() = createDate;

      ati->expireDate() = row->getDate(EXPIREDATE);
      ati->unavailTag() = row->getChar(UNAVAILTAG);
      ati->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
      ati->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
      ati->segCnt() = row->getInt(SEGCNT);
      ati->geoTblItemNoVia1() = row->getInt(GEOTBLITEMNOVIA1);
      ati->geoTblItemNoVia2() = row->getInt(GEOTBLITEMNOVIA2);
      ati->minAge() = row->getInt(MINAGE);
      ati->maxAge() = row->getInt(MAXAGE);
      ati->minNoPsg() = row->getInt(MINNOPSG);
      ati->maxNoPsg() = row->getInt(MAXNOPSG);
      ati->accTvlAllSectors() = row->getChar(ACCTVLALLSECTORS);
      ati->accTvlOut() = row->getChar(ACCTVLOUT);
      ati->accTvlOneSector() = row->getChar(ACCTVLONESECTOR);
      ati->accTvlSameCpmt() = row->getChar(ACCTVLSAMECPMT);
      ati->accTvlSameRule() = row->getChar(ACCTVLSAMERULE);
      ati->accPsgAppl() = row->getChar(ACCPSGAPPL);
      ati->accPsgType() = row->getString(ACCPSGTYPE);
      ati->accPsgId() = row->getChar(ACCPSGID);
      ati->fareClassBkgCdInd() = row->getChar(FARECLASSBKGCDIND);
      ati->inhibit() = row->getChar(INHIBIT);
    } // New Parent

    if (!row->isNull(FARECLASSBKGCD))
    { // Add new Segment & return
      ati->fareClassBkgCds().push_back(row->getString(FARECLASSBKGCD));
    }
    return ati;
  } // mapRowToAccompaniedTravelInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetAccompaniedTravelInfoSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAccompaniedTravelInfoHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAccompaniedTravelInfoHistoricalSQLStatement
    : public QueryGetAccompaniedTravelInfoSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("a.VENDOR = %1q"
                " and a.ITEMNO = %2n"
                " and a.VALIDITYIND = 'Y'"
                " and %3n <= a.EXPIREDATE"
                " and %4n >= a.CREATEDATE");
  }
}; // class QueryGetAccompaniedTravelInfoHistoricalSQLStatement
} // tse
