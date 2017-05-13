//----------------------------------------------------------------------------
//          File:           QueryGetRuleApplSQLStatement.h
//          Description:    QueryGetRuleApplSQLStatement
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
#include "DBAccess/Queries/QueryGetRuleAppl.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetRuleApplSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetRuleApplSQLStatement() {};
  virtual ~QueryGetRuleApplSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    INHIBIT,
    UNAVAILTAG,
    APPLTITLE,
    SERVICEFIRST,
    SERVICEBUS,
    SERVICEECON,
    SERVICECOACH,
    SERVICEOFFPEAK,
    SERVICESUPER,
    ONEWAYIND,
    ROUNDTRIPIND,
    JOURNEYONEWAYIND,
    JOURNEYROUNDTRIPIND,
    JOURNEYCIRCLETRIPIND,
    JOURNEYOPENJAWIND,
    JOURNEYSINGLEOPENJAWIND,
    JOURNEYSOJATORIGININD,
    JOURNEYSOJTURNAROUNDIND,
    JOURNEYDOJAWIND,
    JOURNEYRTWIND,
    JOINTTRANSIND,
    INCLUSIVETOURSIND,
    GROUPSIND,
    TEXTTBLITEMNOADDIONALFARE,
    PURCHASEIND,
    INTERMEDIATEPOINTSIND,
    CAPACITYIND,
    CAPACITYTEXTTBLITEMNO,
    RULESNOTAPPLTEXTTBLITEMNO,
    OTHERTEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    TEXTTBLITEMNO,
    SEGCNT,
    ORDERNO,
    APPLIND,
    LOC,
    LOCTYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select r50.VENDOR,r50.ITEMNO,r50.CREATEDATE,EXPIREDATE,VALIDITYIND,INHIBIT,"
                  "       UNAVAILTAG,APPLTITLE,SERVICEFIRST,SERVICEBUS,SERVICEECON,"
                  "       SERVICECOACH,SERVICEOFFPEAK,SERVICESUPER,ONEWAYIND,ROUNDTRIPIND,"
                  "       JOURNEYONEWAYIND,JOURNEYROUNDTRIPIND,JOURNEYCIRCLETRIPIND,"
                  "       JOURNEYOPENJAWIND,JOURNEYSINGLEOPENJAWIND,JOURNEYSOJATORIGININD,"
                  "       JOURNEYSOJTURNAROUNDIND,JOURNEYDOJAWIND,JOURNEYRTWIND,JOINTTRANSIND,"
                  "       INCLUSIVETOURSIND,GROUPSIND,TEXTTBLITEMNOADDIONALFARE,PURCHASEIND,"
                  "       INTERMEDIATEPOINTSIND,CAPACITYIND,CAPACITYTEXTTBLITEMNO,"
                  "       RULESNOTAPPLTEXTTBLITEMNO,OTHERTEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,"
                  "       TEXTTBLITEMNO,SEGCNT,ORDERNO,APPLIND,LOC,LOCTYPE");

    //		        this->From("=RULEAPPLCOND r50 left outer join =RULEAPPLCONDSEG"
    //		                   "                          using (VENDOR,ITEMNO,CREATEDATE)");
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
        "=RULEAPPLCOND", "r50", "left outer join", "=RULEAPPLCONDSEG", "", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("r50.VENDOR = %1q"
                "    and r50.ITEMNO = %2n"
                "    and EXPIREDATE >= %cd"
                "    and VALIDITYIND = 'Y'");
    this->OrderBy("r50.VENDOR,r50.ITEMNO,r50.CREATEDATE,ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::RuleApplication* mapRowToRuleApplication(Row* row, RuleApplication* prev)
  {
    VendorCode vendor = row->getString(VENDOR);
    int itemNo = row->getInt(ITEMNO);
    DateTime createDate = row->getDate(CREATEDATE);

    RuleApplication* rec;
    if (prev != nullptr && prev->vendor() == vendor && prev->itemNo() == itemNo &&
        prev->createDate() == createDate)
    {
      rec = prev;
    }
    else
    {
      rec = new RuleApplication();
      rec->vendor() = vendor;
      rec->itemNo() = itemNo;
      rec->createDate() = createDate;
      rec->expireDate() = row->getDate(EXPIREDATE);
      rec->validityInd() = row->getChar(VALIDITYIND);
      rec->inhibit() = row->getChar(INHIBIT);
      rec->unavailTag() = row->getChar(UNAVAILTAG);
      rec->applTitle() = row->getString(APPLTITLE);
      rec->serviceFirst() = row->getChar(SERVICEFIRST);
      rec->serviceBus() = row->getChar(SERVICEBUS);
      rec->serviceEcon() = row->getChar(SERVICEECON);
      rec->serviceCoach() = row->getChar(SERVICECOACH);
      rec->serviceOffPeak() = row->getChar(SERVICEOFFPEAK);
      rec->serviceSuper() = row->getChar(SERVICESUPER);
      rec->oneWayInd() = row->getChar(ONEWAYIND);
      rec->roundTripInd() = row->getChar(ROUNDTRIPIND);
      rec->journeyOneWayInd() = row->getChar(JOURNEYONEWAYIND);
      rec->journeyRoundTripInd() = row->getChar(JOURNEYROUNDTRIPIND);
      rec->journeyCircleTripInd() = row->getChar(JOURNEYCIRCLETRIPIND);
      rec->journeyOpenJawInd() = row->getChar(JOURNEYOPENJAWIND);
      rec->journeySingleOpenJawInd() = row->getChar(JOURNEYSINGLEOPENJAWIND);
      rec->journeySOJAtOriginInd() = row->getChar(JOURNEYSOJATORIGININD);
      rec->journeySOJTurnaroundInd() = row->getChar(JOURNEYSOJTURNAROUNDIND);
      rec->journeyDOJawInd() = row->getChar(JOURNEYDOJAWIND);
      rec->journeyRTWInd() = row->getChar(JOURNEYRTWIND);
      rec->jointTransInd() = row->getChar(JOINTTRANSIND);
      rec->inclusiveToursInd() = row->getChar(INCLUSIVETOURSIND);
      rec->groupsInd() = row->getChar(GROUPSIND);
      rec->textTblItemNoAdditionalFare() = row->getInt(TEXTTBLITEMNOADDIONALFARE);
      rec->purchaseInd() = row->getChar(PURCHASEIND);
      rec->intermediatePointsInd() = row->getChar(INTERMEDIATEPOINTSIND);
      rec->capacityInd() = row->getChar(CAPACITYIND);
      rec->capacityTextTblItemNo() = row->getInt(CAPACITYTEXTTBLITEMNO);
      rec->rulesNotApplTextTblItemNo() = row->getInt(RULESNOTAPPLTEXTTBLITEMNO);
      rec->otherTextTblItemNo() = row->getInt(OTHERTEXTTBLITEMNO);
      rec->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
      rec->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
      rec->segCnt() = row->getInt(SEGCNT);
    }

    if (!row->isNull(ORDERNO))
    {
      RuleApplication::RuleApplSeg* seg = new RuleApplication::RuleApplSeg();

      seg->orderNo() = row->getInt(ORDERNO);
      seg->applInd() = row->getChar(APPLIND);
      seg->loc().loc() = row->getString(LOC);
      seg->loc().locType() = row->getChar(LOCTYPE);

      rec->segs().push_back(seg);
    }

    return rec;
  } // mapRowToRuleApplication()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetRuleApplSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetRuleApplHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetRuleApplHistoricalSQLStatement : public QueryGetRuleApplSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("r50.VENDOR = %1q "
                " and r50.ITEMNO = %2n"
                " and r50.VALIDITYIND = 'Y'"
                " and %3n <= r50.EXPIREDATE"
                " and %4n >= r50.CREATEDATE");
  }
}; // class QueryGetRuleApplHistoricalSQLStatement
} // tse
