//----------------------------------------------------------------------------
//          File:           QueryGetFareByRuleAppSQLStatement.h
//          Description:    QueryGetFareByRuleAppSQLStatement
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

#include "Common/TseConsts.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareByRuleApp.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareByRuleAppSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareByRuleAppSQLStatement() {};
  virtual ~QueryGetFareByRuleAppSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    PRIMEPSGTYPE,
    VENDOR,
    RECID,
    RULENO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    RULETARIFF,
    PSGMINAGE,
    PSGMAXAGE,
    PSGOCCFIRST,
    PSGOCCLAST,
    SEGCNT,
    JOINTCARRIERTBLITEMNO,
    MCN,
    VALIDITYIND,
    INHIBIT,
    NEGPSGSTATUSIND,
    PASSENGERIND,
    PSGID,
    PSGLOC,
    PSGLOCTYPE,
    FARELOC1,
    FARELOC1TYPE,
    FARELOC2,
    FARELOC2TYPE,
    LOC1ZONETBLITEMNO,
    LOC2ZONETBLITEMNO,
    WHOLLYWITHINLOC,
    WHOLLYWITHINLOCTYPE,
    TVLLOC1,
    TVLLOC1TYPE,
    DIRECTIONALITY,
    GLOBALDIR,
    TSI,
    ACCOUNTCODE,
    TKTDESIGNATOR,
    SAMECARRIER,
    CARRIERFLTTBLITEMNO,
    SECONDPSGTYPE,
    BATCHCI,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select r1.CARRIER,r1.PRIMEPSGTYPE,r1.VENDOR,r1.RECID,r1.RULENO,r1.CREATEDATE,"
                  " EXPIREDATE,EFFDATE,DISCDATE,r1.RULETARIFF,PSGMINAGE,PSGMAXAGE,"
                  " PSGOCCFIRST,PSGOCCLAST,SEGCNT,JOINTCARRIERTBLITEMNO,MCN,VALIDITYIND,INHIBIT,"
                  " NEGPSGSTATUSIND,PASSENGERIND,PSGID,PSGLOC,PSGLOCTYPE,FARELOC1,FARELOC1TYPE,"
                  " FARELOC2,FARELOC2TYPE,LOC1ZONETBLITEMNO,LOC2ZONETBLITEMNO,WHOLLYWITHINLOC,"
                  " WHOLLYWITHINLOCTYPE,TVLLOC1,TVLLOC1TYPE,DIRECTIONALITY,GLOBALDIR,TSI,"
                  " ACCOUNTCODE,TKTDESIGNATOR,SAMECARRIER,CARRIERFLTTBLITEMNO,SECONDPSGTYPE,"
                  " BATCHCI ");

    //		        this->From("=FAREBYRULEAPP as r1 LEFT OUTER JOIN =FAREBYRULEAPPSEG as r2 "
    //		                   "   USING
    //(CARRIER,VENDOR,PRIMEPSGTYPE,RECID,RULENO,CREATEDATE,RULETARIFF)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(7);
    joinFields.push_back("CARRIER");
    joinFields.push_back("VENDOR");
    joinFields.push_back("PRIMEPSGTYPE");
    joinFields.push_back("RECID");
    joinFields.push_back("RULENO");
    joinFields.push_back("CREATEDATE");
    joinFields.push_back("RULETARIFF");
    this->generateJoinString(
        "=FAREBYRULEAPP", "r1", "LEFT OUTER JOIN", "=FAREBYRULEAPPSEG", "r2", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("r1.CARRIER = %1q "
                " and r1.ACCOUNTCODE = %2q "
                " and %cd <= r1.EXPIREDATE "
                " and r1.VALIDITYIND = 'Y' ");

    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER,ACCOUNTCODE,VALIDITYIND,EXPIREDATE,PRIMEPSGTYPE,VENDOR,RECID,"
                    "RULETARIFF,RULENO,CREATEDATE,EFFDATE,ORDERNO,SECONDPSGTYPE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareByRuleApp* mapRowToFareByRuleApp(Row* row, FareByRuleApp* fbrPrev)
  {
    CarrierCode carrier = row->getString(CARRIER);
    PaxTypeCode paxType = row->getString(PRIMEPSGTYPE);
    VendorCode vendor = row->getString(VENDOR);
    int ruleTariff = row->getInt(RULETARIFF);
    RuleNumber ruleNo = row->getString(RULENO);
    int recId = row->getInt(RECID);
    DateTime createDate = row->getDate(CREATEDATE);

    // Parent not changed, just have a new child row
    if (fbrPrev != nullptr && fbrPrev->carrier() == carrier && fbrPrev->primePaxType() == paxType &&
        fbrPrev->vendor() == vendor && fbrPrev->ruleNo() == ruleNo && fbrPrev->recId() == recId &&
        fbrPrev->ruleTariff() == ruleTariff && fbrPrev->createDate() == createDate)
    {
      if (LIKELY(!row->isNull(SECONDPSGTYPE)))
      {
        fbrPrev->secondaryPaxTypes().push_back(row->getString(SECONDPSGTYPE));
      }

      return fbrPrev;
    }
    else
    {
      tse::FareByRuleApp* fareBR = new tse::FareByRuleApp;

      fareBR->carrier() = carrier;
      fareBR->primePaxType() = paxType;
      fareBR->vendor() = vendor;
      fareBR->ruleTariff() = ruleTariff;
      fareBR->ruleNo() = ruleNo;
      fareBR->recId() = recId;

      fareBR->createDate() = createDate;
      fareBR->expireDate() = row->getDate(EXPIREDATE);
      fareBR->effDate() = row->getDate(EFFDATE);
      fareBR->discDate() = row->getDate(DISCDATE);

      fareBR->paxMinAge() = row->getInt(PSGMINAGE);
      fareBR->paxMaxAge() = row->getInt(PSGMAXAGE);
      fareBR->paxOccFirst() = row->getInt(PSGOCCFIRST);
      fareBR->paxOccLast() = row->getInt(PSGOCCLAST);
      fareBR->segCnt() = row->getInt(SEGCNT);
      fareBR->jointCarrierItemNo() = row->getInt(JOINTCARRIERTBLITEMNO);
      fareBR->negPaxStatusInd() = row->getChar(NEGPSGSTATUSIND);
      fareBR->paxInd() = row->getChar(PASSENGERIND);
      fareBR->paxLoc().locType() = row->getChar(PSGLOCTYPE);
      fareBR->paxLoc().loc() = row->getString(PSGLOC);
      fareBR->fareLoc1().locType() = row->getChar(FARELOC1TYPE);
      fareBR->fareLoc1().loc() = row->getString(FARELOC1);
      fareBR->loc1zoneItemNo() = row->getString(LOC1ZONETBLITEMNO);
      fareBR->loc2zoneItemNo() = row->getString(LOC2ZONETBLITEMNO);
      fareBR->fareLoc2().locType() = row->getChar(FARELOC2TYPE);

      fareBR->fareLoc2().loc() = row->getString(FARELOC2);

      fareBR->whollyWithinLoc().loc() = row->getString(WHOLLYWITHINLOC);
      fareBR->whollyWithinLoc().locType() = row->getChar(WHOLLYWITHINLOCTYPE);

      fareBR->tvlLoc1().loc() = row->getString(TVLLOC1);
      fareBR->tvlLoc1().locType() = row->getChar(TVLLOC1TYPE);
      fareBR->directionality() = row->getChar(DIRECTIONALITY);
      fareBR->paxId() = row->getChar(PSGID);

      std::string gd = row->getString(GLOBALDIR);
      strToGlobalDirection(fareBR->globalDir(), gd);

      fareBR->tsi() = row->getInt(TSI);
      fareBR->accountCode() = row->getString(ACCOUNTCODE);
      fareBR->tktDesignator() = row->getString(TKTDESIGNATOR);
      fareBR->sameCarrier() = row->getChar(SAMECARRIER);
      fareBR->carrierFltTblItemNo() = row->getInt(CARRIERFLTTBLITEMNO);
      fareBR->inhibit() = row->getChar(INHIBIT);

      if (!row->isNull(SECONDPSGTYPE))
        fareBR->secondaryPaxTypes().push_back(row->getString(SECONDPSGTYPE));

      fareBR->vendorFWS() = (BATCHCI_FROM_VENDR_FWS == row->getString(BATCHCI));

      return fareBR;
    }
  } // mapRowToFareByRuleApp()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetFareByRuleAppSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetFareByRuleAppHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFareByRuleAppHistoricalSQLStatement
    : public QueryGetFareByRuleAppSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;
    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(7);
    joinFields.push_back("CARRIER");
    joinFields.push_back("VENDOR");
    joinFields.push_back("PRIMEPSGTYPE");
    joinFields.push_back("RECID");
    joinFields.push_back("RULENO");
    joinFields.push_back("CREATEDATE");
    joinFields.push_back("RULETARIFF");

    partialStatement.Command(
        "("
        "select r1h.CARRIER,r1h.PRIMEPSGTYPE,r1h.VENDOR,r1h.RECID,r1h.RULENO,r1h.CREATEDATE,"
        " EXPIREDATE,EFFDATE,DISCDATE,r1h.RULETARIFF,PSGMINAGE,PSGMAXAGE,"
        " PSGOCCFIRST,PSGOCCLAST,SEGCNT,JOINTCARRIERTBLITEMNO,MCN,VALIDITYIND,INHIBIT,"
        " NEGPSGSTATUSIND,PASSENGERIND,PSGID,PSGLOC,PSGLOCTYPE,FARELOC1,FARELOC1TYPE,"
        " FARELOC2,FARELOC2TYPE,LOC1ZONETBLITEMNO,LOC2ZONETBLITEMNO,WHOLLYWITHINLOC,"
        " WHOLLYWITHINLOCTYPE,TVLLOC1,TVLLOC1TYPE,DIRECTIONALITY,GLOBALDIR,TSI,"
        " ACCOUNTCODE,TKTDESIGNATOR,SAMECARRIER,CARRIERFLTTBLITEMNO,SECONDPSGTYPE,BATCHCI,ORDERNO");
    // partialStatement.From("=FAREBYRULEAPPH as r1h LEFT OUTER JOIN =FAREBYRULEAPPSEGH as r2h "
    //                      "   USING
    // (CARRIER,VENDOR,PRIMEPSGTYPE,RECID,RULENO,CREATEDATE,RULETARIFF)");
    this->generateJoinString(partialStatement,
                             "=FAREBYRULEAPPH",
                             "r1h",
                             "LEFT OUTER JOIN",
                             "=FAREBYRULEAPPSEGH",
                             "r2h",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where(" (r1h.CARRIER = %1q or r1h.CARRIER = 'YY')"
                           " and r1h.ACCOUNTCODE = %2q "
                           " and %3n <= r1h.EXPIREDATE"
                           " and %4n >= r1h.CREATEDATE"
                           " and r1h.VALIDITYIND = 'Y'"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    from.clear();
    partialStatement.Command(
        " union all"
        " ("
        "select r1.CARRIER,r1.PRIMEPSGTYPE,r1.VENDOR,r1.RECID,r1.RULENO,r1.CREATEDATE,"
        " EXPIREDATE,EFFDATE,DISCDATE,r1.RULETARIFF,PSGMINAGE,PSGMAXAGE,"
        " PSGOCCFIRST,PSGOCCLAST,SEGCNT,JOINTCARRIERTBLITEMNO,MCN,VALIDITYIND,INHIBIT,"
        " NEGPSGSTATUSIND,PASSENGERIND,PSGID,PSGLOC,PSGLOCTYPE,FARELOC1,FARELOC1TYPE,"
        " FARELOC2,FARELOC2TYPE,LOC1ZONETBLITEMNO,LOC2ZONETBLITEMNO,WHOLLYWITHINLOC,"
        " WHOLLYWITHINLOCTYPE,TVLLOC1,TVLLOC1TYPE,DIRECTIONALITY,GLOBALDIR,TSI,"
        " ACCOUNTCODE,TKTDESIGNATOR,SAMECARRIER,CARRIERFLTTBLITEMNO,SECONDPSGTYPE,BATCHCI,ORDERNO");
    // partialStatement.From("=FAREBYRULEAPP as r1 LEFT OUTER JOIN =FAREBYRULEAPPSEG as r2 "
    //                      "   USING
    // (CARRIER,VENDOR,PRIMEPSGTYPE,RECID,RULENO,CREATEDATE,RULETARIFF)");
    this->generateJoinString(partialStatement,
                             "=FAREBYRULEAPP",
                             "r1",
                             "LEFT OUTER JOIN",
                             "=FAREBYRULEAPPSEG",
                             "r2",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where(" (r1.CARRIER = %5q or r1.CARRIER = 'YY')"
                           " and r1.ACCOUNTCODE = %6q "
                           " and %7n <= r1.EXPIREDATE"
                           " and %8n >= r1.CREATEDATE"
                           " and r1.VALIDITYIND = 'Y'"
                           ")");
    if (DataManager::forceSortOrder())
      partialStatement.OrderBy(
          " CARRIER,PRIMEPSGTYPE,VENDOR,RULENO,RULETARIFF,EFFDATE desc,CREATEDATE desc,ORDERNO ");
    else
      partialStatement.OrderBy(
          " CARRIER,PRIMEPSGTYPE,VENDOR,RULENO,RULETARIFF,EFFDATE desc,CREATEDATE desc ");
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
}; // class QueryGetFareByRuleAppHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetFareByRuleAppRuleTariff
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFareByRuleAppRuleTariffSQLStatement
    : public QueryGetFareByRuleAppSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("r1.CARRIER = %1q "
                "    and r1.RULETARIFF = %2n "
                "    and %cd <= r1.EXPIREDATE "
                "    and r1.VALIDITYIND = 'Y' ");
    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER,PRIMEPSGTYPE,VENDOR,RECID,RULETARIFF,RULENO,CREATEDATE,EFFDATE,"
                    "ORDERNO,SECONDPSGTYPE");
  }
}; // class QueryGetFareByRuleAppRuleTariffSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetFareByRuleAppRuleTariffHist
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFareByRuleAppRuleTariffHistoricalSQLStatement
    : public QueryGetFareByRuleAppSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(7);
    joinFields.push_back("CARRIER");
    joinFields.push_back("VENDOR");
    joinFields.push_back("PRIMEPSGTYPE");
    joinFields.push_back("RECID");
    joinFields.push_back("RULENO");
    joinFields.push_back("CREATEDATE");
    joinFields.push_back("RULETARIFF");

    partialStatement.Command(
        "("
        "select r1h.CARRIER,r1h.PRIMEPSGTYPE,r1h.VENDOR,r1h.RECID,r1h.RULENO,r1h.CREATEDATE,"
        " EXPIREDATE,EFFDATE,DISCDATE,r1h.RULETARIFF,PSGMINAGE,PSGMAXAGE,"
        " PSGOCCFIRST,PSGOCCLAST,SEGCNT,JOINTCARRIERTBLITEMNO,MCN,VALIDITYIND,INHIBIT,"
        " NEGPSGSTATUSIND,PASSENGERIND,PSGID,PSGLOC,PSGLOCTYPE,FARELOC1,FARELOC1TYPE,"
        " FARELOC2,FARELOC2TYPE,LOC1ZONETBLITEMNO,LOC2ZONETBLITEMNO,WHOLLYWITHINLOC,"
        " WHOLLYWITHINLOCTYPE,TVLLOC1,TVLLOC1TYPE,DIRECTIONALITY,GLOBALDIR,TSI,"
        " ACCOUNTCODE,TKTDESIGNATOR,SAMECARRIER,CARRIERFLTTBLITEMNO,SECONDPSGTYPE,BATCHCI");
    // partialStatement.From("=FAREBYRULEAPPH as r1h LEFT OUTER JOIN =FAREBYRULEAPPSEGH as r2h "
    //                      "   USING
    // (CARRIER,VENDOR,PRIMEPSGTYPE,RECID,RULENO,CREATEDATE,RULETARIFF)");
    this->generateJoinString(partialStatement,
                             "=FAREBYRULEAPPH",
                             "r1h",
                             "LEFT OUTER JOIN",
                             "=FAREBYRULEAPPSEGH",
                             "r2h",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where(" (r1h.CARRIER = %1q or r1h.CARRIER = 'YY')"
                           " and r1h.RULETARIFF = %2n "
                           " and %3n <= r1h.EXPIREDATE"
                           " and %4n >= r1h.CREATEDATE"
                           " and r1h.VALIDITYIND = 'Y'"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    from.clear();
    partialStatement.Command(
        " union all"
        " ("
        "select r1.CARRIER,r1.PRIMEPSGTYPE,r1.VENDOR,r1.RECID,r1.RULENO,r1.CREATEDATE,"
        " EXPIREDATE,EFFDATE,DISCDATE,r1.RULETARIFF,PSGMINAGE,PSGMAXAGE,"
        " PSGOCCFIRST,PSGOCCLAST,SEGCNT,JOINTCARRIERTBLITEMNO,MCN,VALIDITYIND,INHIBIT,"
        " NEGPSGSTATUSIND,PASSENGERIND,PSGID,PSGLOC,PSGLOCTYPE,FARELOC1,FARELOC1TYPE,"
        " FARELOC2,FARELOC2TYPE,LOC1ZONETBLITEMNO,LOC2ZONETBLITEMNO,WHOLLYWITHINLOC,"
        " WHOLLYWITHINLOCTYPE,TVLLOC1,TVLLOC1TYPE,DIRECTIONALITY,GLOBALDIR,TSI,"
        " ACCOUNTCODE,TKTDESIGNATOR,SAMECARRIER,CARRIERFLTTBLITEMNO,SECONDPSGTYPE,BATCHCI");
    // partialStatement.From("=FAREBYRULEAPP as r1 LEFT OUTER JOIN =FAREBYRULEAPPSEG as r2 "
    //                      "   USING
    // (CARRIER,VENDOR,PRIMEPSGTYPE,RECID,RULENO,CREATEDATE,RULETARIFF)");
    this->generateJoinString(partialStatement,
                             "=FAREBYRULEAPP",
                             "r1",
                             "LEFT OUTER JOIN",
                             "=FAREBYRULEAPPSEG",
                             "r2",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where(" (r1.CARRIER = %5q or r1.CARRIER = 'YY') "
                           " and r1.RULETARIFF = %6n "
                           " and %7n <= r1.EXPIREDATE"
                           " and %8n >= r1.CREATEDATE"
                           " and r1.VALIDITYIND = 'Y'"
                           ")");
    partialStatement.OrderBy(
        " CARRIER,PRIMEPSGTYPE,VENDOR,RULENO,RULETARIFF,EFFDATE desc,CREATEDATE desc ");
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
}; // class QueryGetFareByRuleAppRuleTariffHistSQLStatement
} // tse
