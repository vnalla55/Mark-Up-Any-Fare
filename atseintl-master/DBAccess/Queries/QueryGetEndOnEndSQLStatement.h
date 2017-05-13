//----------------------------------------------------------------------------
//          File:           QueryGetEndOnEndSQLStatement.h
//          Description:    QueryGetEndOnEndSQLStatement
//          Created:        11/01/2007
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
#include "DBAccess/Queries/QueryGetEndOnEnd.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetEndOnEndSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetEndOnEndSQLStatement() {};
  virtual ~QueryGetEndOnEndSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    SEGCNT,
    EOERESTIND,
    EOENORMALIND,
    EOESPECIALIND,
    EOESPECIALAPPLIND,
    USCATRANSBORDERIND,
    DOMIND,
    INTLIND,
    SAMECARRIERIND,
    FARETYPELOCAPPL,
    FARETYPELOC1TYPE,
    FARETYPELOC1,
    FARETYPELOC2TYPE,
    FARETYPELOC2,
    CONSTLOCAPPL,
    CONSTLOC1TYPE,
    CONSTLOC1,
    CONSTLOC2TYPE,
    CONSTLOC2,
    TKTIND,
    ABACOMBIND,
    ALLSEGSIND,
    ORDERNO,
    TVLTSI,
    TVLAPPL,
    TVLOVERWATERIND,
    TVLNONSTOPIND,
    TVLFARETYPE,
    TVLFAREIND,
    TVLDIR,
    TVLLOC1TYPE,
    TVLLOC1,
    TVLLOC2TYPE,
    TVLLOC2,
    UNAVAILTAG,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select r104.VENDOR,r104.ITEMNO,r104.CREATEDATE,EXPIREDATE,TEXTTBLITEMNO,"
                  "       OVERRIDEDATETBLITEMNO,SEGCNT,EOERESTIND,EOENORMALIND,EOESPECIALIND,"
                  "       EOESPECIALAPPLIND,USCATRANSBORDERIND,DOMIND,INTLIND,SAMECARRIERIND,"
                  "       FARETYPELOCAPPL,FARETYPELOC1TYPE,FARETYPELOC1,FARETYPELOC2TYPE,"
                  "       FARETYPELOC2,CONSTLOCAPPL,CONSTLOC1TYPE,CONSTLOC1,CONSTLOC2TYPE,"
                  "       CONSTLOC2,TKTIND,ABACOMBIND,ALLSEGSIND,ORDERNO,TVLTSI,TVLAPPL,"
                  "       TVLOVERWATERIND,TVLNONSTOPIND,TVLFARETYPE,TVLFAREIND,TVLDIR,"
                  "       TVLLOC1TYPE,TVLLOC1,TVLLOC2TYPE,TVLLOC2,UNAVAILTAG,INHIBIT");
    this->From("=EOE r104 left join =EOETVLSEG r104s"
               "                  on r104.VENDOR = r104s.VENDOR"
               "                 and r104.ITEMNO = r104s.ITEMNO"
               "                 and r104.CREATEDATE = r104s.CREATEDATE");
    this->Where("r104.VENDOR = %1q"
                "    and r104.ITEMNO = %2n"
                "    and %cd <= EXPIREDATE"
                "    and VALIDITYIND = 'Y'");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,ORDERNO,CREATEDATE");
    else
      this->OrderBy("ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::EndOnEnd* mapRowToEndOnEnd(Row* row, EndOnEnd* endOnEndPrev)
  {
    int itemNo = row->getInt(ITEMNO);
    DateTime createDate = row->getDate(CREATEDATE);

    // Parent not changed, just have a new child row
    if (endOnEndPrev != nullptr && (unsigned int)itemNo == endOnEndPrev->itemNo() &&
        createDate == endOnEndPrev->createDate())
    {
      EndOnEndSegment* seg = mapRowToEndOnEndSegment(row);

      endOnEndPrev->segs().push_back(seg);
      return endOnEndPrev;
    }
    else
    {
      tse::EndOnEnd* endOnEnd = new tse::EndOnEnd;
      endOnEnd->vendor() = row->getString(VENDOR);
      endOnEnd->itemNo() = row->getInt(ITEMNO);
      endOnEnd->createDate() = row->getDate(CREATEDATE);
      endOnEnd->expireDate() = row->getDate(EXPIREDATE);
      endOnEnd->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
      endOnEnd->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
      endOnEnd->segCnt() = row->getInt(SEGCNT);
      endOnEnd->eoeRestInd() = row->getChar(EOERESTIND);
      endOnEnd->eoeNormalInd() = row->getChar(EOENORMALIND);
      endOnEnd->eoespecialInd() = row->getChar(EOESPECIALIND);
      endOnEnd->eoespecialApplInd() = row->getChar(EOESPECIALAPPLIND);
      endOnEnd->uscatransborderInd() = row->getChar(USCATRANSBORDERIND);
      endOnEnd->domInd() = row->getChar(DOMIND);
      endOnEnd->intlInd() = row->getChar(INTLIND);
      endOnEnd->sameCarrierInd() = row->getChar(SAMECARRIERIND);
      endOnEnd->fareTypeLocAppl() = row->getChar(FARETYPELOCAPPL);
      endOnEnd->fareTypeLoc1Type() = LocType(row->getString(FARETYPELOC1TYPE)[0]);
      endOnEnd->fareTypeLoc1() = row->getString(FARETYPELOC1);
      endOnEnd->fareTypeLoc2Type() = LocType(row->getString(FARETYPELOC2TYPE)[0]);
      endOnEnd->fareTypeLoc2() = row->getString(FARETYPELOC2);
      endOnEnd->constLocAppl() = row->getChar(CONSTLOCAPPL);
      endOnEnd->constLoc1Type() = LocType(row->getString(CONSTLOC1TYPE)[0]);
      endOnEnd->constLoc1() = row->getString(CONSTLOC1);
      endOnEnd->constLoc2Type() = LocType(row->getString(CONSTLOC2TYPE)[0]);
      endOnEnd->constLoc2() = row->getString(CONSTLOC2);
      endOnEnd->tktInd() = row->getChar(TKTIND);
      endOnEnd->abacombInd() = row->getChar(ABACOMBIND);
      endOnEnd->allsegsInd() = row->getChar(ALLSEGSIND);
      endOnEnd->unavailTag() = row->getChar(UNAVAILTAG);
      endOnEnd->inhibit() = row->getChar(INHIBIT);

      if (!row->isNull(ORDERNO))
      {
        EndOnEndSegment* seg = mapRowToEndOnEndSegment(row);
        endOnEnd->segs().push_back(seg);
      }

      return endOnEnd;
    }
  } // mapRowToEndOnEnd()

  static tse::EndOnEndSegment* mapRowToEndOnEndSegment(Row* row)
  {
    tse::EndOnEndSegment* seg = new tse::EndOnEndSegment;

    seg->orderNo() = row->getInt(ORDERNO);
    seg->tvltsi() = row->getInt(TVLTSI);
    seg->tvlAppl() = row->getChar(TVLAPPL);
    seg->tvloverwaterInd() = row->getChar(TVLOVERWATERIND);
    seg->tvlNonstopInd() = row->getChar(TVLNONSTOPIND);
    seg->tvlFareType() = row->getString(TVLFARETYPE);
    seg->tvlFareInd() = row->getChar(TVLFAREIND);
    seg->tvldir() = row->getChar(TVLDIR);
    seg->tvlLoc1Type() = LocType(row->getString(TVLLOC1TYPE)[0]);
    seg->tvlLoc1() = row->getString(TVLLOC1);
    seg->tvlLoc2Type() = LocType(row->getString(TVLLOC2TYPE)[0]);
    seg->tvlLoc2() = row->getString(TVLLOC2);

    return seg;
  } // mapRowToEndOnEndSegment()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetEndOnEndSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetEndOnEndHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetEndOnEndHistoricalSQLStatement : public QueryGetEndOnEndSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("r104.VENDOR = %1q"
                "  and r104.ITEMNO = %2n"
                "  and r104.VALIDITYIND = 'Y'"
                "  and %3n <= r104.EXPIREDATE"
                "  and %4n >= r104.CREATEDATE");
  }
}; // class QueryGetEndOnEndHistoricalSQLStatement
} // tse
