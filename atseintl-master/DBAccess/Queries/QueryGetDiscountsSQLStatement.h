//----------------------------------------------------------------------------
//          File:           QueryGetDiscountsSQLStatement.h
//          Description:    QueryGetDiscountsSQLStatement
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
#include "DBAccess/Queries/QueryGetDiscounts.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetAgentDiscountSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAgentDiscountSQLStatement() {};
  virtual ~QueryGetAgentDiscountSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    DISCPERCENT,
    FAREAMT1,
    FAREAMT2,
    TEXTTBLITEMNO,
    GEOTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    SEGCNT,
    DISCPERCENTNODEC,
    NODEC1,
    NODEC2,
    MINAGE,
    MAXAGE,
    FIRSTOCCUR,
    LASTOCCUR,
    DISCAPPL,
    PSGTYPE,
    PSGID,
    FARECALCIND,
    OWRT,
    BASEFAREIND,
    BASEFARECLASS,
    BASEPSGTYPE,
    BASEFARETYPE,
    BOOKINGAPPL,
    R1BOOKINGCODE,
    BETWEENMARKET,
    ANDMARKET,
    CUR1,
    CUR2,
    RESULTOWRT,
    RESULTFARECLASS,
    RESULTBKGCD,
    TKTCODE,
    TKTCODEMODIFIER,
    TKTDESIGNATOR,
    TKTDESIGNATORMODIFIER,
    ACCIND,
    ACCTVLALLSECTORS,
    ACCTVLOUT,
    ACCTVLONESECTOR,
    ACCTVLSAMECPMT,
    ACCTVLSAMERULE,
    ACCPSGAPPL,
    FARECLASSBKGCDIND,
    ORDERNO,
    MINNOPSG,
    MAXNOPSG,
    MINAGE1,
    MAXAGE1,
    MINAGE2,
    MAXAGE2,
    MINAGE3,
    MAXAGE3,
    ACCPSGTYPE1,
    ACCPSGTYPE2,
    ACCPSGTYPE3,
    FARECLASS,
    R1SBOOKINGCODE,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select r1.VENDOR,r1.ITEMNO,r1.CREATEDATE,EXPIREDATE,UNAVAILTAG,"
                  "       DISCPERCENT,FAREAMT1,FAREAMT2,TEXTTBLITEMNO,GEOTBLITEMNO,"
                  "       OVERRIDEDATETBLITEMNO,SEGCNT,DISCPERCENTNODEC,NODEC1,NODEC2,"
                  "       MINAGE,MAXAGE,FIRSTOCCUR,LASTOCCUR,DISCAPPL,PSGTYPE,PSGID,"
                  "       FARECALCIND,OWRT,BASEFAREIND,BASEFARECLASS,BASEPSGTYPE,"
                  "       BASEFARETYPE,BOOKINGAPPL,r1.BOOKINGCODE R1BOOKINGCODE,"
                  "       BETWEENMARKET,ANDMARKET,CUR1,CUR2,RESULTOWRT,RESULTFARECLASS,"
                  "       RESULTBKGCD,TKTCODE,TKTCODEMODIFIER,TKTDESIGNATOR,"
                  "       TKTDESIGNATORMODIFIER,ACCIND,ACCTVLALLSECTORS,ACCTVLOUT,"
                  "       ACCTVLONESECTOR,ACCTVLSAMECPMT,ACCTVLSAMERULE,ACCPSGAPPL,"
                  "       FARECLASSBKGCDIND,ORDERNO,MINNOPSG,MAXNOPSG,MINAGE1,MAXAGE1,"
                  "       MINAGE2,MAXAGE2,MINAGE3,MAXAGE3,ACCPSGTYPE1,ACCPSGTYPE2,"
                  "       ACCPSGTYPE3,FARECLASS,r1s.BOOKINGCODE R1SBOOKINGCODE,INHIBIT");

    //		        this->From("=AGENTDISCOUNTS as r1 LEFT OUTER JOIN =AGENTACCPSG as r1s "
    //		                   "                              USING (VENDOR,ITEMNO,CREATEDATE) ");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=AGENTDISCOUNTS", "r1", "LEFT OUTER JOIN", "=AGENTACCPSG", "r1s", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("r1.VENDOR = %1q "
                " and r1.ITEMNO = %2n "
                " and r1.validityind = 'Y' "
                " and %cd <= r1.EXPIREDATE ");
    if (DataManager::forceSortOrder())
      this->OrderBy("ORDERNO,ITEMNO,CREATEDATE");
    else
      this->OrderBy("orderno");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static DiscountSegInfo* mapRowToDiscountSegInfo(Row* row)
  {
    tse::DiscountSegInfo* seg = new tse::DiscountSegInfo;

    seg->orderNo() = row->getInt(ORDERNO);
    seg->minNoPsg() = row->getInt(MINNOPSG);
    seg->maxNoPsg() = row->getInt(MAXNOPSG);
    seg->minAge1() = row->getInt(MINAGE1);
    seg->maxAge1() = row->getInt(MAXAGE1);
    seg->minAge2() = row->getInt(MINAGE2);
    seg->maxAge2() = row->getInt(MAXAGE2);
    seg->minAge3() = row->getInt(MINAGE3);
    seg->maxAge3() = row->getInt(MAXAGE3);
    seg->accPsgType1() = row->getString(ACCPSGTYPE1);
    seg->accPsgType2() = row->getString(ACCPSGTYPE2);
    seg->accPsgType3() = row->getString(ACCPSGTYPE3);
    seg->fareClass() = row->getString(FARECLASS);
    seg->bookingCode() = row->getString(R1SBOOKINGCODE);

    return seg;
  } // mapRowToDiscountSegInfo()

  static DiscountInfo* mapRowToDiscountInfo(Row* row, DiscountInfo* diPrev)
  {
    unsigned int itemNo = row->getInt(ITEMNO);
    DateTime createDate = row->getDate(CREATEDATE);

    // Parent not changed, just have a new child row
    if (diPrev != nullptr && itemNo == diPrev->itemNo() && createDate == diPrev->createDate())
    {
      DiscountSegInfo* seg = mapRowToDiscountSegInfo(row);
      diPrev->segs().push_back(seg);
      return diPrev;
    }
    else
    {
      tse::DiscountInfo* di = new tse::DiscountInfo;

      di->createDate() = row->getDate(CREATEDATE);
      di->expireDate() = row->getDate(EXPIREDATE);
      di->vendor() = row->getString(VENDOR);
      di->itemNo() = row->getInt(ITEMNO);
      di->unavailtag() = row->getChar(UNAVAILTAG);
      di->discPercentNoDec() = row->getInt(DISCPERCENTNODEC);
      di->discPercent() =
          QUERYCLASS::adjustDecimal(row->getInt(DISCPERCENT), di->discPercentNoDec());
      di->noDec1() = row->getInt(NODEC1);
      di->noDec2() = row->getInt(NODEC2);
      di->fareAmt1() = QUERYCLASS::adjustDecimal(row->getInt(FAREAMT1), di->noDec1());
      di->fareAmt2() = QUERYCLASS::adjustDecimal(row->getInt(FAREAMT2), di->noDec2());
      di->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
      di->geoTblItemNo() = row->getInt(GEOTBLITEMNO);
      di->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
      di->segCnt() = row->getInt(SEGCNT);
      di->minAge() = row->getInt(MINAGE);
      di->maxAge() = row->getInt(MAXAGE);
      di->firstOccur() = row->getInt(FIRSTOCCUR);
      di->lastOccur() = row->getInt(LASTOCCUR);
      di->discAppl() = row->getChar(DISCAPPL);
      di->paxType() = row->getString(PSGTYPE);
      di->psgid() = row->getChar(PSGID);
      di->farecalcInd() = row->getChar(FARECALCIND);
      di->owrt() = row->getChar(OWRT);
      di->baseFareInd() = row->getChar(BASEFAREIND);
      di->baseFareClass() = row->getString(BASEFARECLASS);
      di->basePsgType() = row->getString(BASEPSGTYPE);
      di->baseFareType() = row->getString(BASEFARETYPE);
      di->bookingAppl() = row->getChar(BOOKINGAPPL);
      di->bookingCode() = row->getString(R1BOOKINGCODE);
      di->betweenMarket() = row->getString(BETWEENMARKET);
      di->andMarket() = row->getString(ANDMARKET);
      di->cur1() = row->getString(CUR1);
      di->cur2() = row->getString(CUR2);
      di->resultOwrt() = row->getChar(RESULTOWRT);
      di->resultFareClass() = row->getString(RESULTFARECLASS);
      di->resultBookingCode() = row->getString(RESULTBKGCD);
      di->tktCode() = row->getString(TKTCODE);
      di->tktCodeModifier() = row->getString(TKTCODEMODIFIER);
      di->tktDesignator() = row->getString(TKTDESIGNATOR);
      di->tktDesignatorModifier() = row->getString(TKTDESIGNATORMODIFIER);
      di->accInd() = row->getChar(ACCIND);
      di->accTvlAllSectors() = row->getChar(ACCTVLALLSECTORS);
      di->accTvlOut() = row->getChar(ACCTVLOUT);
      di->accTvlOneSector() = row->getChar(ACCTVLONESECTOR);
      di->accTvlSameCpmt() = row->getChar(ACCTVLSAMECPMT);
      di->accTvlSameRule() = row->getChar(ACCTVLSAMERULE);
      di->accPsgAppl() = row->getChar(ACCPSGAPPL);
      di->fareClassBkgCodeInd() = row->getChar(FARECLASSBKGCDIND);
      di->inhibit() = row->getChar(INHIBIT);

      if (!row->isNull(ORDERNO))
      {
        di->segs().push_back(mapRowToDiscountSegInfo(row));
      }

      return di;
    }
  } // mapRowToDiscountInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAgentDiscountHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAgentDiscountHistoricalSQLStatement
    : public QueryGetAgentDiscountSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("r1.VENDOR = %1q "
                "  and r1.ITEMNO = %2n "
                "  and r1.validityind = 'Y' "
                "  and %3n <= r1.EXPIREDATE"
                "  and %4n >= r1.CREATEDATE");
  }
}; // class QueryGetAgentDiscountHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetOthersDiscount
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetOthersDiscountSQLStatement : public QueryGetAgentDiscountSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {

    //		        this->From("=OTHERDISCOUNTS as r1 LEFT OUTER JOIN =OTHERACCPSG as r1s "
    //		                   "        USING (VENDOR, ITEMNO, CREATEDATE) ");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=OTHERDISCOUNTS", "r1", "LEFT OUTER JOIN", "=OTHERACCPSG", "r1s", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------
  }
}; // class QueryGetOthersDiscountSQLStatemen

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetOthersDiscountHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetOthersDiscountHistoricalSQLStatement
    : public QueryGetAgentDiscountSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {

    //		        this->From("=OTHERDISCOUNTS as r1 LEFT OUTER JOIN =OTHERACCPSG as r1s "
    //		                   "        USING (VENDOR, ITEMNO, CREATEDATE) ");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=OTHERDISCOUNTS", "r1", "LEFT OUTER JOIN", "=OTHERACCPSG", "r1s", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("r1.VENDOR = %1q "
                "  and r1.ITEMNO = %2n "
                "  and r1.validityind = 'Y' "
                "  and %3n <= r1.EXPIREDATE"
                "  and %4n >= r1.CREATEDATE");
  }
}; // class QueryGetOthersDiscountHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTourDiscount
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTourDiscountSQLStatement : public QueryGetAgentDiscountSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {

    //		        this->From("=TOURCONDUCTORDISCNT as r1  LEFT OUTER JOIN"
    //		                   "      =TOURCONDUCTORACCPSG as r1s USING
    //(VENDOR,ITEMNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=TOURCONDUCTORDISCNT",
                             "r1",
                             "LEFT OUTER JOIN",
                             "=TOURCONDUCTORACCPSG",
                             "r1s",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------
  }
}; // class QueryGetTourDiscountSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTourDiscountHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTourDiscountHistoricalSQLStatement
    : public QueryGetAgentDiscountSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {

    //		        this->From("=TOURCONDUCTORDISCNT as r1  LEFT OUTER JOIN"
    //		                   "      =TOURCONDUCTORACCPSG as r1s USING
    //(VENDOR,ITEMNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=TOURCONDUCTORDISCNT",
                             "r1",
                             "LEFT OUTER JOIN",
                             "=TOURCONDUCTORACCPSG",
                             "r1s",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("r1.VENDOR = %1q "
                "  and r1.ITEMNO = %2n "
                "  and r1.validityind = 'Y' "
                "  and %3n <= r1.EXPIREDATE"
                "  and %4n >= r1.CREATEDATE");
  }
}; // class QueryGetTourDiscountHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetChdDiscount
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetChdDiscountSQLStatement : public QueryGetAgentDiscountSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {

    //		        this->From("=CHILDRENSDISCOUNTS as r1 LEFT OUTER JOIN =CHILDRENSACCPSG as r1s"
    //		                   "                                  USING
    //(VENDOR,ITEMNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=CHILDRENSDISCOUNTS",
                             "r1",
                             "LEFT OUTER JOIN",
                             "=CHILDRENSACCPSG",
                             "r1s",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------
  }
}; // class QueryGetChdDiscountSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetChdDiscountHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetChdDiscountHistoricalSQLStatement
    : public QueryGetAgentDiscountSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {

    //		        this->From("=CHILDRENSDISCOUNTS as r1 LEFT OUTER JOIN =CHILDRENSACCPSG as r1s"
    //		                   "                                  USING
    //(VENDOR,ITEMNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=CHILDRENSDISCOUNTS",
                             "r1",
                             "LEFT OUTER JOIN",
                             "=CHILDRENSACCPSG",
                             "r1s",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("r1.VENDOR = %1q "
                "  and r1.ITEMNO = %2n "
                "  and r1.validityind = 'Y' "
                "  and %3n <= r1.EXPIREDATE"
                "  and %4n >= r1.CREATEDATE");
  }
}; // class QueryGetChdDiscountHistoricalSQLStatement
} // tse
