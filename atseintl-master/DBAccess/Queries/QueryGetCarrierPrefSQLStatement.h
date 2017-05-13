//----------------------------------------------------------------------------
//          File:           QueryGetCarrierPrefSQLStatement.h
//          Description:    QueryGetCarrierPrefSQLStatement
//          Created:        10/29/2007
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

#include "Common/FallbackUtil.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCarrierPref.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCarrierPrefSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCarrierPrefSQLStatement() {};
  virtual ~QueryGetCarrierPrefSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CREATEDATE,
    EXPIREDATE,
    LOCKDATE,
    EFFDATE,
    DISCDATE,
    MEMONO,
    FREEBAGGAGEEXEMPT,
    AVAILABILITYIGRUL2ST,
    AVAILABILITYIGRUL3ST,
    BYPASSOSC,
    BYPASSRSC,
    APPLYSAMENUCTORT,
    APPLYRTEVALTOTERMINALPT,
    NOAPPLYDRVEXCEPTUS,
    APPLYLEASTRESTRSTOPTOPU,
    APPLYLEASTRESTRTRNSFTOPU,
    NOAPPLYCOMBTAG1AND3,
    APPLYSINGLEADDONCONSTR,
    APPLYSPECOVERADDON,
    NOAPPLYNIGERIACURADJ,
    NOSURFACEATFAREBREAK,
    CARRIERBASENATION,
    DESCRIPTION,
    FIRSTTVLDATE,
    LASTTVLDATE,
    FLOWMKTJOURNEYTYPE,
    LOCALMKTJOURNEYTYPE,
    ACTIVATESOLOPRICING,
    ACTIVATESOLOSHOPPING,
    ACTIVATEJOURNEYPRICING,
    APPLYUS2TAXONFREETKT,
    ACTIVATEJOURNEYSHOPPING,
    PUBLICPRIVATE1,
    VENDOR1,
    PUBLICPRIVATE2,
    VENDOR2,
    PRIVATEFAREIND,
    APPLYPREMBUSCABINDIFFCALC,
    APPLYPREMECONCABINDIFFCALC,
    NOAPPLYNONPREMIUMCABINFARE,
    FREEBAGGAGECARRIEREXEMPT,
    NOAPPLYBAGEXCEPTONCOMBAREA1_3,
    OVRIDEFREEBAGFARECOMPLOGIC,
    APPLYNORMALFAREOJINDIFFCNTRYS,
    APPLYSGLTOJBETWAREASSHORTERFC,
    APPLYSGLTOJBETWAREASLONGERFC,
    APPLYSPCLDOJEUROPE,
    APPLYHIGHERRTOJCPRIVATEFARES,
    APPLYHIGHERRTR102FOR35FARES,
    APPLYFAREBASISINFARECALCTN,
    APPLYBRANDEDFARESPERFC,
    NONREFUNDABLEYQCODE,
    NONREFUNDABLEYRCODE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select p.CARRIER,p.CREATEDATE,EXPIREDATE,LOCKDATE,EFFDATE,DISCDATE,MEMONO,"
        "       FREEBAGGAGEEXEMPT,AVAILABILITYIGRUL2ST,AVAILABILITYIGRUL3ST,BYPASSOSC,"
        "       BYPASSRSC,APPLYSAMENUCTORT,APPLYRTEVALTOTERMINALPT,"
        "       NOAPPLYDRVEXCEPTUS,APPLYLEASTRESTRSTOPTOPU,APPLYLEASTRESTRTRNSFTOPU,"
        "       NOAPPLYCOMBTAG1AND3,APPLYSINGLEADDONCONSTR,APPLYSPECOVERADDON,"
        "       NOAPPLYNIGERIACURADJ,NOSURFACEATFAREBREAK,CARRIERBASENATION,DESCRIPTION,"
        "       FIRSTTVLDATE,LASTTVLDATE,FLOWMKTJOURNEYTYPE,LOCALMKTJOURNEYTYPE,"
        "       ACTIVATESOLOPRICING,ACTIVATESOLOSHOPPING,ACTIVATEJOURNEYPRICING,"
        "       APPLYUS2TAXONFREETKT,ACTIVATEJOURNEYSHOPPING,PUBLICPRIVATE1,VENDOR1,"
        "       PUBLICPRIVATE2,VENDOR2,PRIVATEFAREIND,APPLYPREMBUSCABINDIFFCALC,"
        "       APPLYPREMECONCABINDIFFCALC,NOAPPLYNONPREMIUMCABINFARE,FREEBAGGAGECARRIEREXEMPT,"
        "       NOAPPLYBAGEXCEPTONCOMBAREA1_3,OVRIDEFREEBAGFARECOMPLOGIC,"
        "       APPLYNORMALFAREOJINDIFFCNTRYS,APPLYSGLTOJBETWAREASSHORTERFC,"
        "       APPLYSGLTOJBETWAREASLONGERFC,APPLYSPCLDOJEUROPE,APPLYHIGHERRTOJCPRIVATEFARES,"
        "       APPLYHIGHERRTR102FOR35FARES,"
        "       APPLYFAREBASISINFARECALCTN,APPLYBRANDEDFARESPERFC,NONREFUNDABLEYQCODE,"
        "       NONREFUNDABLEYRCODE");

    //		        this->From("=CARRIERPREFERENCE p left outer join =CARRIERPREFCOMBPREF c"
    //		                   "                             using (CARRIER,CREATEDATE) ");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(2);
    joinFields.push_back("CARRIER");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=CARRIERPREFERENCE",
                             "p",
                             "left outer join",
                             "=CARRIERPREFCOMBPREF",
                             "c",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("p.CARRIER = %1q "
                "    and %cd <= p.EXPIREDATE");
    // this->OrderBy("1,2");
    this->OrderBy("CARRIER, CREATEDATE, PUBLICPRIVATE1, VENDOR1, PUBLICPRIVATE2, VENDOR2");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CarrierPreference* mapRowToCarrierPref(Row* row)
  {
    tse::CarrierPreference* cxrPref = new tse::CarrierPreference;

    cxrPref->carrier() = row->getString(CARRIER);
    cxrPref->createDate() = row->getDate(CREATEDATE);
    cxrPref->expireDate() = row->getDate(EXPIREDATE);
    cxrPref->effDate() = row->getDate(EFFDATE);
    cxrPref->discDate() = row->getDate(DISCDATE);
    cxrPref->firstTvlDate() = row->getDate(FIRSTTVLDATE);
    cxrPref->lastTvlDate() = row->getDate(LASTTVLDATE);
    cxrPref->memoNo() = row->getInt(MEMONO);
    cxrPref->freebaggageexempt() = row->getChar(FREEBAGGAGEEXEMPT);
    cxrPref->availabilityApplyrul2st() = row->getChar(AVAILABILITYIGRUL2ST);
    cxrPref->availabilityApplyrul3st() = row->getChar(AVAILABILITYIGRUL3ST);
    cxrPref->bypassosc() = row->getChar(BYPASSOSC);
    cxrPref->bypassrsc() = row->getChar(BYPASSRSC);
    cxrPref->applysamenuctort() = row->getChar(APPLYSAMENUCTORT);
    cxrPref->applyrtevaltoterminalpt() = row->getChar(APPLYRTEVALTOTERMINALPT);
    cxrPref->noApplydrvexceptus() = row->getChar(NOAPPLYDRVEXCEPTUS);
    cxrPref->applyleastRestrStOptopu() = row->getChar(APPLYLEASTRESTRSTOPTOPU);
    cxrPref->applyleastRestrtrnsftopu() = row->getChar(APPLYLEASTRESTRTRNSFTOPU);
    cxrPref->noApplycombtag1and3() = row->getChar(NOAPPLYCOMBTAG1AND3);
    cxrPref->applysingleaddonconstr() = row->getChar(APPLYSINGLEADDONCONSTR);
    cxrPref->applyspecoveraddon() = row->getChar(APPLYSPECOVERADDON);
    cxrPref->noApplynigeriaCuradj() = row->getChar(NOAPPLYNIGERIACURADJ);
    cxrPref->noSurfaceAtFareBreak() = row->getChar(NOSURFACEATFAREBREAK);
    cxrPref->carrierbasenation() = row->getString(CARRIERBASENATION);
    cxrPref->description() = row->getString(DESCRIPTION);
    cxrPref->flowMktJourneyType() = row->getChar(FLOWMKTJOURNEYTYPE);
    cxrPref->localMktJourneyType() = row->getChar(LOCALMKTJOURNEYTYPE);
    cxrPref->activateSoloPricing() = row->getChar(ACTIVATESOLOPRICING);
    cxrPref->activateSoloShopping() = row->getChar(ACTIVATESOLOSHOPPING);
    cxrPref->activateJourneyPricing() = row->getChar(ACTIVATEJOURNEYPRICING);
    cxrPref->applyUS2TaxOnFreeTkt() = row->getChar(APPLYUS2TAXONFREETKT);
    cxrPref->activateJourneyShopping() = row->getChar(ACTIVATEJOURNEYSHOPPING);
    cxrPref->privateFareInd() = row->getChar(PRIVATEFAREIND);
    cxrPref->applyPremBusCabinDiffCalc() = row->getChar(APPLYPREMBUSCABINDIFFCALC);
    cxrPref->applyPremEconCabinDiffCalc() = row->getChar(APPLYPREMECONCABINDIFFCALC);
    cxrPref->noApplySlideToNonPremium() = row->getChar(NOAPPLYNONPREMIUMCABINFARE);
    cxrPref->noApplyBagExceptOnCombArea1_3() = row->getChar(NOAPPLYBAGEXCEPTONCOMBAREA1_3);
    cxrPref->freeBaggageCarrierExempt() = row->getChar(FREEBAGGAGECARRIEREXEMPT);
    cxrPref->ovrideFreeBagFareCompLogic() = row->getChar(OVRIDEFREEBAGFARECOMPLOGIC);

    if (!row->isNull(APPLYNORMALFAREOJINDIFFCNTRYS))
      cxrPref->applyNormalFareOJInDiffCntrys() = row->getChar(APPLYNORMALFAREOJINDIFFCNTRYS);
    if (!row->isNull(APPLYSGLTOJBETWAREASSHORTERFC))
      cxrPref->applySingleTOJBetwAreasShorterFC() = row->getChar(APPLYSGLTOJBETWAREASSHORTERFC);
    if (!row->isNull(APPLYSGLTOJBETWAREASLONGERFC))
      cxrPref->applySingleTOJBetwAreasLongerFC() = row->getChar(APPLYSGLTOJBETWAREASLONGERFC);
    if (!row->isNull(APPLYSPCLDOJEUROPE))
      cxrPref->applySpclDOJEurope() = row->getChar(APPLYSPCLDOJEUROPE);
    if (!row->isNull(APPLYHIGHERRTOJCPRIVATEFARES))
      cxrPref->applyHigherRTOJ() = row->getChar(APPLYHIGHERRTOJCPRIVATEFARES);
    if (!row->isNull(APPLYHIGHERRTR102FOR35FARES))
      cxrPref->applyHigherRT() = row->getChar(APPLYHIGHERRTR102FOR35FARES);

    cxrPref->applyFBCinFC() = row->getChar(APPLYFAREBASISINFARECALCTN);

    if (!row->isNull(APPLYBRANDEDFARESPERFC))
      cxrPref->setApplyBrandedFaresPerFc(row->getChar(APPLYBRANDEDFARESPERFC));

    if (!row->isNull(NONREFUNDABLEYQCODE))
      cxrPref->setNonRefundableYQCode(row->getString(NONREFUNDABLEYQCODE));

    if (!row->isNull(NONREFUNDABLEYRCODE))
      cxrPref->setNonRefundableYRCode(row->getString(NONREFUNDABLEYRCODE));

    return cxrPref;
  } // mapRowToCarrierPref()

  static const char* getCarrier(Row* row) { return row->getString(CARRIER); }

  static DateTime getCreateDate(Row* row) { return row->getDate(CREATEDATE); }

  static const char* getVendor1(Row* row) { return row->getString(VENDOR1); }

  static const char* getVendor2(Row* row) { return row->getString(VENDOR2); }

  static char getPublicPrivate1(Row* row) { return row->getChar(PUBLICPRIVATE1); }

  static char getPublicPrivate2(Row* row) { return row->getChar(PUBLICPRIVATE2); }

  static bool isNullPublicPrivate1(Row* row) { return row->isNull(PUBLICPRIVATE1); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL where clause for Historical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetCarrierPrefHistoricalSQLStatement : public QueryGetCarrierPrefSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where("p.CARRIER = %1q "); }
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCarrierPref
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCarrierPrefSQLStatement : public QueryGetCarrierPrefSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where("%cd <= p.EXPIREDATE"); }
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL where clause for Historical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCarrierPrefHistoricalSQLStatement
    : public QueryGetCarrierPrefSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where("1 = 1"); }
};

////////////////////////////////////////////////////////////////////////
//
// QueryGetCarrierPrefFBRPref
//
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetCarrierPrefFBRPrefSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCarrierPrefFBRPrefSQLStatement() {};
  virtual ~QueryGetCarrierPrefFBRPrefSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CREATEDATE,
    VENDOR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER,CREATEDATE,VENDOR");
    this->From("=CARRIERPREFFBRPREF");
    this->Where("CARRIER = %1q");
    this->OrderBy("1,2,3");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapRowToFbrPref(Row* row, fbrPref& newFbrPref)
  {
    newFbrPref.cxr = row->getString(CARRIER);
    newFbrPref.createDate = row->getDate(CREATEDATE);
    newFbrPref.vendor = row->getString(VENDOR);
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCarrierPrefFBRPref
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCarrierPrefFBRPrefSQLStatement
    : public QueryGetCarrierPrefFBRPrefSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where(""); }
};

} // tse
