//----------------------------------------------------------------------------
//          File:           QueryGetBrandedFareSQLStatement.h
//          Description:    QueryGetBrandedFareSQLStatement
//          Created:        03/22/2013
//
//     (C) 2013, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBrandedFare.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetBrandedFareSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    VENDOR,
    CARRIER,
    SOURCE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    PUBLICPRIVATEIND,
    EFFDATE,
    DISCDATE,
    TVLFIRSTYEAR,
    TVLFIRSTMONTH,
    TVLFIRSTDAY,
    TVLLASTYEAR,
    TVLLASTMONTH,
    TVLLASTDAY,
    PSGTYPE,
    SVCFEESACCOUNTCODETBLITEMNO,
    SVCFEESSECURITYTBLITEMNO,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC1ZONETBLITEMNO,
    LOC2TYPE,
    LOC2,
    LOC2ZONETBLITEMNO,
    GLOBALIND,
    ONEMATRIX,
    PROGRAMCODE,
    PROGRAMTEXT,
    SVCFEESFEATURETBLITEMNO,
    TAXTEXTTBLITEMNO,
    SEGCOUNT,
    // end of BrandedFare
    TIER,
    BRANDNAME,
    SVCFEESFAREIDTBLITEMNO,
    TAXTEXTTBLITEMNOSEG,
    SEGNO,
    // end of BrandedFaresSeg
    NUMBEROFCOLUMNS
  };
  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select t.VENDOR, t.CARRIER, t.SOURCE, t.SEQNO, t.CREATEDATE,"
            "EXPIREDATE, VALIDITYIND, PUBLICPRIVATEIND, EFFDATE, DISCDATE,"
            "TVLFIRSTYEAR, TVLFIRSTMONTH, TVLFIRSTDAY, TVLLASTYEAR, TVLLASTMONTH,"
            "TVLLASTDAY, PSGTYPE, SVCFEESACCOUNTCODETBLITEMNO, SVCFEESSECURITYTBLITEMNO, "
            "DIRECTIONALITY,"
            "LOC1TYPE, LOC1, LOC1ZONETBLITEMNO, LOC2TYPE, LOC2,"
            "LOC2ZONETBLITEMNO, GLOBALIND, ONEMATRIX, PROGRAMCODE, PROGRAMTEXT,"
            "SVCFEESFEATURETBLITEMNO, t.TAXTEXTTBLITEMNO, SEGCOUNT, TIER, BRANDNAME,"
            "SVCFEESFAREIDTBLITEMNO, ts.TAXTEXTTBLITEMNO, SEGNO");
    From("=BRANDEDFARES t, =BRANDEDFARESSEG ts");
    Where("t.VENDOR = %1q"
          " and t.CARRIER = %2q"
          " and %cd <= DISCDATE"
          " and %cd <= EXPIREDATE"
          " and EFFDATE <= DISCDATE"
          " and VALIDITYIND = 'Y'"
          " and t.VENDOR = ts.VENDOR"
          " and t.CARRIER = ts.CARRIER"
          " and t.SOURCE = ts.SOURCE"
          " and t.SEQNO = ts.SEQNO"
          " and t.CREATEDATE = ts.CREATEDATE");
    OrderBy("VENDOR, CARRIER, SOURCE, SEQNO, CREATEDATE");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static BrandedFare* mapRowToBrandedFare(Row* row, BrandedFare* bfPrev)
  {
    // get the OrderBy fields to determine new parent
    VendorCode vendor(row->getString(VENDOR));
    CarrierCode carrier(row->getString(CARRIER));
    Indicator source(row->getChar(SOURCE));
    SequenceNumberLong seqNo(row->getLong(SEQNO));
    DateTime createDate(row->getDate(CREATEDATE));
    BrandedFare* bf(nullptr);
    if (bfPrev != nullptr && vendor == bfPrev->vendor() && carrier == bfPrev->carrier() &&
        source == bfPrev->source() && seqNo == bfPrev->seqNo() &&
        createDate == bfPrev->createDate())
    {
      bf = bfPrev;
    }
    else
    {
      // create new parent
      bf = new BrandedFare;

      bf->vendor() = vendor;
      bf->carrier() = carrier;
      bf->source() = source;
      bf->seqNo() = seqNo;
      bf->createDate() = createDate;
      bf->expireDate() = row->getDate(EXPIREDATE);
      if (!row->isNull(VALIDITYIND))
      {
        bf->validityInd() = row->getChar(VALIDITYIND);
      }
      if (!row->isNull(PUBLICPRIVATEIND))
      {
        bf->publicPrivateInd() = row->getChar(PUBLICPRIVATEIND);
      }
      bf->effDate() = row->getDate(EFFDATE);
      bf->discDate() = row->getDate(DISCDATE);

      if (!row->isNull(TVLFIRSTYEAR))
      {
        bf->tvlFirstYear() = row->getInt(TVLFIRSTYEAR);
      }
      if (!row->isNull(TVLFIRSTMONTH))
      {
        bf->tvlFirstMonth() = row->getInt(TVLFIRSTMONTH);
      }
      if (!row->isNull(TVLFIRSTDAY))
      {
        bf->tvlFirstDay() = row->getInt(TVLFIRSTDAY);
      }
      if (!row->isNull(TVLLASTYEAR))
      {
        bf->tvlLastYear() = row->getInt(TVLLASTYEAR);
      }
      if (!row->isNull(TVLLASTMONTH))
      {
        bf->tvlLastMonth() = row->getInt(TVLLASTMONTH);
      }
      if (!row->isNull(TVLLASTDAY))
      {
        bf->tvlLastDay() = row->getInt(TVLLASTDAY);
      }
      bf->psgType() = row->getString(PSGTYPE);
      if (!row->isNull(SVCFEESACCOUNTCODETBLITEMNO))
      {
        bf->svcFeesAccountCodeTblItemNo() = row->getLong(SVCFEESACCOUNTCODETBLITEMNO);
      }
      if (!row->isNull(SVCFEESSECURITYTBLITEMNO))
      {
        bf->svcFeesSecurityTblItemNo() = row->getLong(SVCFEESSECURITYTBLITEMNO);
      }
      if (!row->isNull(DIRECTIONALITY))
      {
        bf->directionality() = row->getChar(DIRECTIONALITY);
      }
      std::string loc1Type(row->getString(LOC1TYPE));
      if (!loc1Type.empty())
      {
        bf->locKey1().locType() = loc1Type[0];
      }
      bf->locKey1().loc() = row->getString(LOC1);
      std::string loc1ZoneTblItemNo(row->getString(LOC1ZONETBLITEMNO));
      if (!loc1ZoneTblItemNo.empty())
      {
        bf->loc1ZoneTblItemNo() = loc1ZoneTblItemNo;
      }
      std::string loc2Type(row->getString(LOC2TYPE));
      if (!loc2Type.empty())
      {
        bf->locKey2().locType() = loc2Type[0];
      }
      bf->locKey2().loc() = row->getString(LOC2);
      std::string loc2ZoneTblItemNo(row->getString(LOC2ZONETBLITEMNO));
      if (!loc2ZoneTblItemNo.empty())
      {
        bf->loc2ZoneTblItemNo() = loc2ZoneTblItemNo;
      }
      if (!row->isNull(GLOBALIND))
      {
        strToGlobalDirection(bf->globalInd(), row->getString(GLOBALIND));
      }
      if (!row->isNull(ONEMATRIX))
      {
        bf->oneMatrix() = row->getChar(ONEMATRIX);
      }
      bf->programCode() = row->getString(PROGRAMCODE);
      bf->programText() = row->getString(PROGRAMTEXT);

      if (!row->isNull(SVCFEESFEATURETBLITEMNO))
      {
        bf->svcFeesFeatureTblItemNo() = row->getLong(SVCFEESFEATURETBLITEMNO);
      }
      if (!row->isNull(TAXTEXTTBLITEMNO))
      {
        bf->taxTextTblItemNo() = row->getLong(TAXTEXTTBLITEMNO);
      }
      if (!row->isNull(SEGCOUNT))
      {
        bf->segCount() = row->getInt(SEGCOUNT);
      }
    }
    // create new segment and push it to vector
    BrandedFareSeg* seg(new BrandedFareSeg);
    bf->segments().push_back(seg);
    if (!row->isNull(TIER))
    {
      seg->tier() = row->getInt(TIER);
    }
    seg->brandName() = row->getString(BRANDNAME);
    if (!row->isNull(SVCFEESFAREIDTBLITEMNO))
    {
      seg->svcFeesFareIdTblItemNo() = row->getLong(SVCFEESFAREIDTBLITEMNO);
    }
    if (!row->isNull(TAXTEXTTBLITEMNOSEG))
    {
      seg->taxTextTblItemNo() = row->getLong(TAXTEXTTBLITEMNOSEG);
    }
    if (!row->isNull(SEGNO))
    {
      seg->segNo() = row->getInt(SEGNO);
    }
    return bf;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetBrandedFareHistoricalSQLStatement : public QueryGetBrandedFareSQLStatement<QUERYCLASS>
{
  void adjustBaseSQL() override
  {
    this->Where("t.VENDOR = %1q"
                " and t.CARRIER = %2q"
                " and EFFDATE <= DISCDATE"
                " and VALIDITYIND = 'Y'"
                " and t.VENDOR = ts.VENDOR"
                " and t.CARRIER = ts.CARRIER"
                " and t.SOURCE = ts.SOURCE"
                " and t.SEQNO = ts.SEQNO"
                " and t.CREATEDATE = ts.CREATEDATE"
                " and %3n <= t.EXPIREDATE"
                " and %4n >= t.CREATEDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, CARRIER, SOURCE, SEQNO, CREATEDATE");
    else
      this->OrderBy("");
  }
};

template <class QUERYCLASS>
class QueryGetAllBrandedFareSQLStatement : public QueryGetBrandedFareSQLStatement<QUERYCLASS>
{
  void adjustBaseSQL() override
  {
    this->Where("%cd <= DISCDATE"
                " and %cd <= EXPIREDATE"
                " and EFFDATE <= DISCDATE"
                " and VALIDITYIND = 'Y'"
                " and t.VENDOR = ts.VENDOR"
                " and t.CARRIER = ts.CARRIER"
                " and t.SOURCE = ts.SOURCE"
                " and t.SEQNO = ts.SEQNO"
                " and t.CREATEDATE = ts.CREATEDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, CARRIER, SOURCE, SEQNO, CREATEDATE");
    else
      this->OrderBy("");
  }
};
} // tse
