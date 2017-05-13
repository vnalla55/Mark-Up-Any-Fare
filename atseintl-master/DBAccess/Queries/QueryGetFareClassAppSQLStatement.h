//----------------------------------------------------------------------------
//          File:           QueryGetFareClassAppSQLStatement.h
//          Description:    QueryGetFareClassAppSQLStatement
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
#include "DBAccess/Queries/QueryGetFareClassApp.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareClassAppSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareClassAppSQLStatement() {};
  virtual ~QueryGetFareClassAppSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    RULETARIFF,
    RULE,
    FARECLASS,
    SEQNO,
    CREATEDATE,
    ORDERNO,
    FOOTNOTE1,
    FOOTNOTE2,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    MCN,
    TEXTTBLITEMNO,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    OWRT,
    ROUTINGAPPL,
    ROUTING,
    FARETYPE,
    SEASONTYPE,
    DOWTYPE,
    PRICINGCATTYPE,
    DISPLAYCATTYPE,
    UNAVAILTAG,
    SEGCNT,
    INHIBIT,
    DIRECTIONALITY,
    BOOKINGCODETBLITEMNO,
    OVERRIDEDATETBLNO,
    MINAGE,
    MAXAGE,
    TKTCODE,
    TKTCODEMODIFIER,
    TKTDESIGNATOR,
    TKTDESIGNATORMODIFIER,
    PSGTYPE,
    BOOKINGCODE1,
    BOOKINGCODE2,
    BOOKINGCODE3,
    BOOKINGCODE4,
    BOOKINGCODE5,
    BOOKINGCODE6,
    BOOKINGCODE7,
    BOOKINGCODE8,
    CARRIERAPPLTBLITEMNO,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select r1.VENDOR,r1.CARRIER,r1.RULETARIFF,r1.RULE,r1.FARECLASS,r1.SEQNO,"
                  " r1.CREATEDATE,r1s.ORDERNO,FOOTNOTE1,FOOTNOTE2,EFFDATE,DISCDATE,"
                  " EXPIREDATE,MCN,TEXTTBLITEMNO,LOC1TYPE,LOC1,LOC2TYPE,LOC2,OWRT,"
                  " ROUTINGAPPL,ROUTING,FARETYPE,SEASONTYPE,DOWTYPE,PRICINGCATTYPE,"
                  " DISPLAYCATTYPE,UNAVAILTAG,SEGCNT,INHIBIT,"
                  " r1s.DIRECTIONALITY,BOOKINGCODETBLITEMNO,OVERRIDEDATETBLNO,MINAGE,"
                  " MAXAGE,TKTCODE,TKTCODEMODIFIER,TKTDESIGNATOR,TKTDESIGNATORMODIFIER,"
                  " PSGTYPE,BOOKINGCODE1,BOOKINGCODE2,BOOKINGCODE3,BOOKINGCODE4,"
                  " BOOKINGCODE5,BOOKINGCODE6,BOOKINGCODE7,BOOKINGCODE8,CARRIERAPPLTBLITEMNO");
    this->From("=FARECLASSAPPL r1, =FARECLASSAPPSEG r1s");
    this->Where("r1.VENDOR = %1q"
                " and r1.CARRIER = %2q"
                " and r1.RULETARIFF = %3n"
                " and r1.RULE = %4q"
                " and r1.FARECLASS = %5q"
                " and r1.VALIDITYIND = 'Y'"
                " and r1.OVERRIDEIND = ''"
                " and %cd <= r1.DISCDATE"
                " and %cd <= r1.EXPIREDATE"
                " and r1.EFFDATE <= r1.DISCDATE"
                " and r1.VENDOR    = r1s.VENDOR"
                " and r1.CARRIER   = r1s.CARRIER"
                " and r1.RULETARIFF= r1s.RULETARIFF"
                " and r1.RULE      = r1s.RULE"
                " and r1.FARECLASS = r1s.FARECLASS"
                " and r1.VERSIONDATE = r1s.VERSIONDATE"
                " and r1.SEQNO     = r1s.SEQNO"
                " and r1.CREATEDATE= r1s.CREATEDATE");
    this->OrderBy("VENDOR,CARRIER,RULETARIFF,RULE,FARECLASS,SEQNO,CREATEDATE,ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareClassAppInfo* mapRowToFareClassAppInfo(Row* row, FareClassAppInfo* fcaPrev)
  {
    // Get the OrderBy fields to determine new parent
    VendorCode vnd = row->getString(VENDOR);
    CarrierCode cxr = row->getString(CARRIER);
    TariffNumber rlTrf = row->getInt(RULETARIFF);
    RuleNumber rlNo = row->getString(RULE);
    FareClassCode frCls = row->getString(FARECLASS);
    long seqNo = row->getLong(SEQNO);
    DateTime crtDt = row->getDate(CREATEDATE);

    tse::FareClassAppInfo* fca;

    if (fcaPrev != nullptr && vnd == fcaPrev->_vendor && cxr == fcaPrev->_carrier &&
        rlTrf == fcaPrev->_ruleTariff && rlNo == fcaPrev->_ruleNumber &&
        frCls == fcaPrev->_fareClass && seqNo == fcaPrev->_seqNo && crtDt == fcaPrev->_createDate)
    {
      fca = fcaPrev;
    }
    else
    { // Create new parent
      fca = new tse::FareClassAppInfo;

      fca->_vendor = vnd;
      fca->_carrier = cxr;
      fca->_ruleTariff = rlTrf;
      fca->_ruleNumber = rlNo;
      fca->_fareClass = frCls;
      fca->_seqNo = seqNo;
      fca->_createDate = crtDt;

      fca->_footnote1 = row->getString(FOOTNOTE1);
      fca->_footnote2 = row->getString(FOOTNOTE2);

      fca->_effectiveDate = row->getDate(EFFDATE);
      fca->_discDate = row->getDate(DISCDATE);
      fca->_expirationDate = row->getDate(EXPIREDATE);

      fca->_MCN = row->getInt(MCN);
      fca->_textTBLItemNo = row->getInt(TEXTTBLITEMNO);
      fca->_location1Type = row->getChar(LOC1TYPE);
      fca->_location1 = row->getString(LOC1);
      fca->_location2Type = row->getChar(LOC2TYPE);
      fca->_location2 = row->getString(LOC2);
      fca->_owrt = row->getString(OWRT)[0];
      fca->_routingAppl = row->getChar(ROUTINGAPPL);
      fca->_routingNumber = row->getString(ROUTING);
      fca->_fareType = row->getString(FARETYPE);
      fca->_seasonType = row->getChar(SEASONTYPE);
      fca->_dowType = row->getChar(DOWTYPE);
      fca->_pricingCatType = row->getChar(PRICINGCATTYPE);
      fca->_displayCatType = row->getChar(DISPLAYCATTYPE);
      fca->_unavailTag = row->getChar(UNAVAILTAG);
      fca->_segCount = row->getInt(SEGCNT);
      fca->_inhibit = row->getChar(INHIBIT);
    }

    if (LIKELY(!row->isNull(ORDERNO)))
    { // Create new seg and hook it in
      tse::FareClassAppSegInfo* seg = new tse::FareClassAppSegInfo;
      seg->_bookingCode[0] = row->getString(BOOKINGCODE1);
      seg->_bookingCode[1] = row->getString(BOOKINGCODE2);
      seg->_bookingCode[2] = row->getString(BOOKINGCODE3);
      seg->_bookingCode[3] = row->getString(BOOKINGCODE4);
      seg->_bookingCode[4] = row->getString(BOOKINGCODE5);
      seg->_bookingCode[5] = row->getString(BOOKINGCODE6);
      seg->_bookingCode[6] = row->getString(BOOKINGCODE7);
      seg->_bookingCode[7] = row->getString(BOOKINGCODE8);
      seg->_bookingCodeTblItemNo = row->getInt(BOOKINGCODETBLITEMNO);
      std::string direct = row->getString(DIRECTIONALITY);
      if (direct == "3")
        seg->_directionality = ORIGINATING_LOC1;
      else if (direct == "4")
        seg->_directionality = ORIGINATING_LOC2;
      else
        seg->_directionality = DIR_IND_NOT_DEFINED;
      seg->_maxAge = row->getInt(MAXAGE);
      seg->_minAge = row->getInt(MINAGE);
      seg->_overrideDateTblNo = row->getInt(OVERRIDEDATETBLNO);
      seg->_paxType = row->getString(PSGTYPE);
      seg->_seqNo = row->getLong(SEQNO);
      seg->_tktCode = row->getString(TKTCODE);
      seg->_tktCodeModifier = row->getString(TKTCODEMODIFIER);
      seg->_tktDesignator = row->getString(TKTDESIGNATOR);
      seg->_tktDesignatorModifier = row->getString(TKTDESIGNATORMODIFIER);
      seg->_carrierApplTblItemNo = row->getInt(CARRIERAPPLTBLITEMNO);

      fca->_segs.push_back(seg);
    }

    return fca;
  } // mapRowToFareClassAppInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetFareClassAppSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace From and Where clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFareClassAppHistoricalSQLStatement
    : public QueryGetFareClassAppSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "("
        "select r1h.VENDOR,r1h.CARRIER,r1h.RULETARIFF,r1h.RULE,r1h.FARECLASS,r1h.SEQNO,"
        " r1h.CREATEDATE,r1sh.ORDERNO,FOOTNOTE1,FOOTNOTE2,EFFDATE,DISCDATE,"
        " EXPIREDATE,MCN,TEXTTBLITEMNO,LOC1TYPE,LOC1,LOC2TYPE,LOC2,OWRT,"
        " ROUTINGAPPL,ROUTING,FARETYPE,SEASONTYPE,DOWTYPE,PRICINGCATTYPE,"
        " DISPLAYCATTYPE,UNAVAILTAG,SEGCNT,INHIBIT,"
        " r1sh.DIRECTIONALITY,BOOKINGCODETBLITEMNO,OVERRIDEDATETBLNO,MINAGE,"
        " MAXAGE,TKTCODE,TKTCODEMODIFIER,TKTDESIGNATOR,TKTDESIGNATORMODIFIER,"
        " PSGTYPE,BOOKINGCODE1,BOOKINGCODE2,BOOKINGCODE3,BOOKINGCODE4,"
        " BOOKINGCODE5,BOOKINGCODE6,BOOKINGCODE7,BOOKINGCODE8,CARRIERAPPLTBLITEMNO");
    partialStatement.From("=FARECLASSAPPLH r1h, =FARECLASSAPPSEGH r1sh");
    partialStatement.Where("r1h.VENDOR = %1q"
                           " and r1h.CARRIER = %2q"
                           " and r1h.RULETARIFF = %3n"
                           " and r1h.RULE = %4q"
                           " and r1h.FARECLASS = %5q"
                           " and r1h.VALIDITYIND = 'Y'"
                           " and r1h.OVERRIDEIND = ''"
                           " and %6n <= r1h.EXPIREDATE"
                           " and ( %7n >=  r1h.CREATEDATE"
                           "      or %8n >= r1h.EFFDATE)"
                           " and r1h.EFFDATE <= r1h.DISCDATE"
                           " and r1h.VENDOR    = r1sh.VENDOR"
                           " and r1h.CARRIER   = r1sh.CARRIER"
                           " and r1h.RULETARIFF= r1sh.RULETARIFF"
                           " and r1h.RULE      = r1sh.RULE"
                           " and r1h.FARECLASS = r1sh.FARECLASS"
                           " and r1h.SEQNO     = r1sh.SEQNO"
                           " and r1h.CREATEDATE= r1sh.CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        " ("
        "select r1.VENDOR,r1.CARRIER,r1.RULETARIFF,r1.RULE,r1.FARECLASS,r1.SEQNO,"
        " r1.CREATEDATE,r1s.ORDERNO,FOOTNOTE1,FOOTNOTE2,EFFDATE,DISCDATE,"
        " EXPIREDATE,MCN,TEXTTBLITEMNO,LOC1TYPE,LOC1,LOC2TYPE,LOC2,OWRT,"
        " ROUTINGAPPL,ROUTING,FARETYPE,SEASONTYPE,DOWTYPE,PRICINGCATTYPE,"
        " DISPLAYCATTYPE,UNAVAILTAG,SEGCNT,INHIBIT,"
        " r1s.DIRECTIONALITY,BOOKINGCODETBLITEMNO,OVERRIDEDATETBLNO,MINAGE,"
        " MAXAGE,TKTCODE,TKTCODEMODIFIER,TKTDESIGNATOR,TKTDESIGNATORMODIFIER,"
        " PSGTYPE,BOOKINGCODE1,BOOKINGCODE2,BOOKINGCODE3,BOOKINGCODE4,"
        " BOOKINGCODE5,BOOKINGCODE6,BOOKINGCODE7,BOOKINGCODE8,CARRIERAPPLTBLITEMNO");
    partialStatement.From("=FARECLASSAPPL r1, =FARECLASSAPPSEG r1s");
    partialStatement.Where("r1.VENDOR = %9q"
                           " and r1.CARRIER = %10q"
                           " and r1.RULETARIFF = %11n"
                           " and r1.RULE = %12q"
                           " and r1.FARECLASS = %13q"
                           " and r1.VALIDITYIND = 'Y'"
                           " and r1.OVERRIDEIND = ''"
                           " and %14n <= r1.EXPIREDATE"
                           " and (   %15n >=  r1.CREATEDATE"
                           "      or %16n >= r1.EFFDATE)"
                           " and r1.EFFDATE <= r1.DISCDATE"
                           " and r1.VENDOR    = r1s.VENDOR"
                           " and r1.CARRIER   = r1s.CARRIER"
                           " and r1.RULETARIFF= r1s.RULETARIFF"
                           " and r1.RULE      = r1s.RULE"
                           " and r1.FARECLASS = r1s.FARECLASS"
                           " and r1.VERSIONDATE = r1s.VERSIONDATE"
                           " and r1.SEQNO     = r1s.SEQNO"
                           " and r1.CREATEDATE= r1s.CREATEDATE"
                           ")");
    partialStatement.OrderBy(
        " VENDOR,CARRIER,RULETARIFF,RULE,FARECLASS,SEQNO,EFFDATE desc,CREATEDATE desc,ORDERNO");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetFareClassAppHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareClassApp
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllFareClassAppSQLStatement : public QueryGetFareClassAppSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("r1.VALIDITYIND = 'Y'"
                "    and r1.OVERRIDEIND = ''"
                "    and %cd <= r1.DISCDATE"
                "    and %cd <= r1.EXPIREDATE"
                "    and r1.EFFDATE <= r1.DISCDATE"
                "    and r1.VENDOR    = r1s.VENDOR"
                "    and r1.CARRIER   = r1s.CARRIER"
                "    and r1.RULETARIFF= r1s.RULETARIFF"
                "    and r1.RULE      = r1s.RULE"
                "    and r1.FARECLASS = r1s.FARECLASS"
                "    and r1.VERSIONDATE = r1s.VERSIONDATE"
                "    and r1.SEQNO     = r1s.SEQNO"
                "    and r1.CREATEDATE= r1s.CREATEDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,CARRIER,RULETARIFF,RULE,FARECLASS,SEQNO,CREATEDATE,ORDERNO");
    else
      this->OrderBy("");
  }
}; // QueryGetAllFareClassAppSQLStatement
} // tse
