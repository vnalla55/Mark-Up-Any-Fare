//----------------------------------------------------------------------------
//          File:           QueryGetBkgExceptionSQLStatement.h
//          Description:    QueryGetBkgExceptionSQLStatement
//          Created:        10/26/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBkgException.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetBkgExceptionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetBkgExceptionSQLStatement() {};
  virtual ~QueryGetBkgExceptionSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    SEGCNT,
    PRIMEIND,
    TBLTYPEIND,
    SEQTYPEIND,
    SEGAPPLIND,
    CONDSEGIND,
    SEGNO,
    NEGCARRIERIND,
    VIACARRIER,
    PRIMESECIND,
    FLTRANGEAPPL,
    FLT1,
    FLT2,
    EQUIPTYPE,
    TVLPORTION,
    TSI,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    POSTSI,
    POSLOCTYPE,
    POSLOC,
    SELLTKTIND,
    TVLEFFYEAR,
    TVLEFFMONTH,
    TVLEFFDAY,
    TVLDISCYEAR,
    TVLDISCMONTH,
    TVLDISCDAY,
    DOW,
    TVLSTARTTIME,
    TVLENDTIME,
    FARECLASSTYPEIND,
    FARECLASS,
    ARBROUTING,
    ARBZONENO,
    BKGCODERESTIND,
    BOOKINGCODE1,
    BOOKINGCODE2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select r999.VENDOR,r999.ITEMNO,r999.SEQNO,r999.CREATEDATE,EXPIREDATE, "
                  "       SEGCNT,PRIMEIND,TBLTYPEIND,"
                  "       SEQTYPEIND,SEGAPPLIND,CONDSEGIND,SEGNO,NEGCARRIERIND,VIACARRIER,"
                  "       PRIMESECIND,FLTRANGEAPPL,FLT1,FLT2,EQUIPTYPE,TVLPORTION,TSI,"
                  "       DIRECTIONALITY,LOC1TYPE,LOC1,LOC2TYPE,LOC2,POSTSI,POSLOCTYPE,"
                  "       POSLOC,SELLTKTIND,TVLEFFYEAR,TVLEFFMONTH,TVLEFFDAY,TVLDISCYEAR,"
                  "       TVLDISCMONTH,TVLDISCDAY,DOW,TVLSTARTTIME,TVLENDTIME,"
                  "       FARECLASSTYPEIND,FARECLASS,ARBROUTING,ARBZONENO,BKGCODERESTIND,"
                  "       BOOKINGCODE1,BOOKINGCODE2");
    this->From("=BKGCDEXCEPTIONS r999, =BKGCDEXCEPTIONSSEG r999s");
    this->Where("r999.vendor = %1q"
                "    and r999.itemno = %2n"
                "    and r999.vendor = r999s.vendor"
                "    and r999.itemno  = r999s.itemno"
                "    and r999.seqno = r999s.seqno"
                "    and r999.createdate = r999s.createdate"
                "    and r999.expiredate >= %cd");
    this->OrderBy("r999.seqno, r999.createdate, r999s.segno");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::BookingCodeExceptionSequence*
  mapRowToBookingExceptionSequence(Row* row, BookingCodeExceptionSequence* bkgExceptPrev)
  {
    unsigned int itemNo = row->getInt(ITEMNO);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    // Parent not changed, just have a new child row
    if (bkgExceptPrev != nullptr && itemNo == bkgExceptPrev->itemNo() &&
        seqNo == bkgExceptPrev->seqNo() && createDate == bkgExceptPrev->createDate())
    {
      BookingCodeExceptionSegment* seg = mapRowToBookingExceptionSegment(row);
      bkgExceptPrev->segmentVector().push_back(seg);
      return bkgExceptPrev;
    }
    else
    {
      tse::BookingCodeExceptionSequence* seq = new tse::BookingCodeExceptionSequence;
      seq->vendor() = row->getString(VENDOR);
      seq->itemNo() = row->getInt(ITEMNO);
      seq->seqNo() = row->getLong(SEQNO);
      seq->createDate() = row->getDate(CREATEDATE);
      seq->expireDate() = row->getDate(EXPIREDATE);
      seq->primeInd() = row->getChar(PRIMEIND);
      seq->tableType() = row->getChar(TBLTYPEIND);
      seq->ifTag() = row->getChar(CONDSEGIND);
      seq->constructSpecified() = row->getChar(SEQTYPEIND);
      seq->segCnt() = row->getInt(SEGCNT);

      BookingCodeExceptionSegment* seg = mapRowToBookingExceptionSegment(row);
      seq->segmentVector().push_back(seg);

      return seq;
    }
  } // mapRowToBookingExceptionSequence()

  static tse::BookingCodeExceptionSegment* mapRowToBookingExceptionSegment(Row* row)
  {
    tse::BookingCodeExceptionSegment* seg = new tse::BookingCodeExceptionSegment;

    seg->segNo() = row->getInt(SEGNO);
    seg->viaCarrier() = row->getString(VIACARRIER);
    seg->primarySecondary() = row->getChar(PRIMESECIND);
    seg->fltRangeAppl() = row->getChar(FLTRANGEAPPL);
    seg->flight1() = row->getInt(FLT1);
    seg->flight2() = row->getInt(FLT2);
    seg->equipType() = row->getString(EQUIPTYPE);
    seg->tvlPortion() = row->getString(TVLPORTION);
    seg->tsi() = row->getInt(TSI);
    seg->directionInd() = row->getChar(DIRECTIONALITY);
    seg->loc1Type() = LocType(row->getString(LOC1TYPE)[0]);
    seg->loc1() = row->getString(LOC1);
    seg->loc2Type() = LocType(row->getString(LOC2TYPE)[0]);
    seg->loc2() = row->getString(LOC2);
    seg->posTsi() = row->getInt(POSTSI);
    seg->posLocType() = LocType(row->getString(POSLOCTYPE)[0]);
    seg->posLoc() = row->getString(POSLOC);
    seg->soldInOutInd() = row->getChar(SELLTKTIND);
    seg->tvlEffYear() = QUERYCLASS::mapYear(row->getString(TVLEFFYEAR));
    seg->tvlEffMonth() = row->getInt(TVLEFFMONTH);
    seg->tvlEffDay() = row->getInt(TVLEFFDAY);
    seg->tvlDiscYear() = QUERYCLASS::mapYear(row->getString(TVLDISCYEAR));
    seg->tvlDiscMonth() = row->getInt(TVLDISCMONTH);
    seg->tvlDiscDay() = row->getInt(TVLDISCDAY);
    seg->daysOfWeek() = row->getString(DOW);
    seg->tvlStartTime() = row->getInt(TVLSTARTTIME);
    seg->tvlEndTime() = row->getInt(TVLENDTIME);
    seg->fareclassType() = row->getChar(FARECLASSTYPEIND);
    seg->fareclass() = row->getString(FARECLASS);
    seg->restrictionTag() = row->getChar(BKGCODERESTIND);
    seg->bookingCode1() = row->getString(BOOKINGCODE1);
    seg->bookingCode2() = row->getString(BOOKINGCODE2);
    seg->sellTktInd() = row->getChar(SELLTKTIND);
    seg->arbZoneNo() = row->getString(ARBZONENO);

    return seg;
  } // mapRowToBookingExceptionSegment()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetBkgExceptionHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetBkgExceptionHistoricalSQLStatement
    : public QueryGetBkgExceptionSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        " select hr999.VENDOR,hr999.ITEMNO,hr999.SEQNO,hr999.CREATEDATE,EXPIREDATE, "
        "       SEGCNT,PRIMEIND,TBLTYPEIND,"
        "       SEQTYPEIND,SEGAPPLIND,CONDSEGIND,SEGNO,NEGCARRIERIND,VIACARRIER,"
        "       PRIMESECIND,FLTRANGEAPPL,FLT1,FLT2,EQUIPTYPE,TVLPORTION,TSI,"
        "       DIRECTIONALITY,LOC1TYPE,LOC1,LOC2TYPE,LOC2,POSTSI,POSLOCTYPE,"
        "       POSLOC,SELLTKTIND,TVLEFFYEAR,TVLEFFMONTH,TVLEFFDAY,TVLDISCYEAR,"
        "       TVLDISCMONTH,TVLDISCDAY,DOW,TVLSTARTTIME,TVLENDTIME,"
        "       FARECLASSTYPEIND,FARECLASS,ARBROUTING,ARBZONENO,BKGCODERESTIND,"
        "       BOOKINGCODE1,BOOKINGCODE2");
    partialStatement.From("=BKGCDEXCEPTIONSH hr999, =BKGCDEXCEPTIONSSEGH hr999s");
    partialStatement.Where("hr999.vendor = %1q"
                           "  and hr999.itemno = %2n"
                           "  and hr999.vendor = hr999s.vendor"
                           "  and hr999.itemno  = hr999s.itemno"
                           "  and hr999.seqno = hr999s.seqno"
                           "  and hr999.createdate = hr999s.createdate"
                           "  and %3n <= hr999.EXPIREDATE"
                           "  and %4n >= hr999.CREATEDATE");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        "       select r999.VENDOR,r999.ITEMNO,r999.SEQNO,r999.CREATEDATE,EXPIREDATE, "
        "       SEGCNT,PRIMEIND,TBLTYPEIND,"
        "       SEQTYPEIND,SEGAPPLIND,CONDSEGIND,SEGNO,NEGCARRIERIND,VIACARRIER,"
        "       PRIMESECIND,FLTRANGEAPPL,FLT1,FLT2,EQUIPTYPE,TVLPORTION,TSI,"
        "       DIRECTIONALITY,LOC1TYPE,LOC1,LOC2TYPE,LOC2,POSTSI,POSLOCTYPE,"
        "       POSLOC,SELLTKTIND,TVLEFFYEAR,TVLEFFMONTH,TVLEFFDAY,TVLDISCYEAR,"
        "       TVLDISCMONTH,TVLDISCDAY,DOW,TVLSTARTTIME,TVLENDTIME,"
        "       FARECLASSTYPEIND,FARECLASS,ARBROUTING,ARBZONENO,BKGCODERESTIND,"
        "       BOOKINGCODE1,BOOKINGCODE2");
    partialStatement.From("=BKGCDEXCEPTIONS r999, =BKGCDEXCEPTIONSSEG r999s");
    partialStatement.Where("r999.vendor = %5q"
                           "    and r999.itemno = %6n"
                           "    and r999.vendor = r999s.vendor"
                           "    and r999.itemno  = r999s.itemno"
                           "    and r999.seqno = r999s.seqno"
                           "    and r999.createdate = r999s.createdate"
                           "    and %7n <= r999.EXPIREDATE"
                           "    and %8n >= r999.CREATEDATE");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("seqno, createdate, segno");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetBkgExceptionHistoricalSQLStatement
} // tse
