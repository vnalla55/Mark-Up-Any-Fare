//----------------------------------------------------------------------------
//          File:           QueryGetFareCalcConfigsSQLStatement.h
//          Description:    QueryGetFareCalcConfigsSQLStatement
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
#include "DBAccess/Queries/QueryGetFareCalcConfigs.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareCalcConfigsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareCalcConfigsSQLStatement() {};
  virtual ~QueryGetFareCalcConfigsSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITY,
    LOC1TYPE,
    LOC1,
    ITINDISPLAYIND,
    WPPSGTYPDISPLAY,
    ITINHEADERTEXTIND,
    WPCONNECTIONIND,
    WPCHILDINFANTFAREBASIS,
    FAREBASISTKTDESLNG,
    TRUEPSGRTYPEIND,
    FARETAXTOTALIND,
    NOOFTAXBOXES,
    BASETAXEQUIVTOTALLENGTH,
    TAXPLACEMENTIND,
    TAXCURCODEDISPLAYIND,
    ZPAMOUNTDISPLAYIND,
    TAXEXEMPTIONIND,
    TAXEXEMPTBREAKDOWNIND,
    FCPSGTYPDISPLAY,
    FCCHILDINFANTFAREBASIS,
    FCCONNECTIONIND,
    DOMESTICNUC,
    TVLCOMMENCEMENTDATE,
    WRAPAROUND,
    MULTISURCHARGESPACING,
    DOMESTICISI,
    INTERNATIONALISI,
    DISPLAYBSR,
    ENDORSEMENTS,
    WARNINGMESSAGES,
    LASTDAYTICKETDISPLAY,
    LASTDAYTICKETOUTPUT,
    RESERVATIONOVERRIDE,
    FAREBASISDISPLAYOPTION,
    GLOBALSIDETRIPIND,
    WPAPERMITTED,
    WPAPSGDTLFORMAT,
    WPAFARELINEPSGTYPE,
    WPAFARELINEHDR,
    WPAPRIMEPSGREFNO,
    WPA2NDPSGREFNO,
    WPAFAREOPTIONMAXNO,
    WPASORT,
    WPASHOWDUPAMTS,
    WPAPSGLINEBREAK,
    WPAPSGMULTILINEBREAK,
    WPANOMATCHHIGHERCABINFARE,
    WPASTOREWITHOUTREBOOK,
    WPAACCTVLOPTION,
    WPAACCTVLCAT13,
    WPNOMATCHPERMITTED,
    WPPSGDTLFORMAT,
    WPFARELINEPSGTYPE,
    WPFARELINEHDR,
    WPPRIMEPSGREFNO,
    WP2NDPSGREFNO,
    WPFAREOPTIONMAXNO,
    WPSORT,
    WPSHOWDUPAMTS,
    WPPSGLINEBREAK,
    WPPSGMULTILINEBREAK,
    WPNOMATCHHIGHERCABINFARE,
    WPSTOREWITHOUTREBOOK,
    WPACCTVLOPTION,
    WPACCTVLCAT13,
    VALCXRDISPLAYOPT,
    NEGPERMITTED,
    INTERLINETKTPERMITTED,
    NOMATCHAVAIL,
    WPWPATRAILERMSG,
    PARTICIPATINGAGREEMENT,
    WPROIND,
    WPAROIND,
    APPLYDOMESTICMULTICUR,
    APPLYINTLMULTICUR,
    IETPRICEINTERLINEACTIVE,
    VALUECODEBASE,
    MARKETLOC,
    DISPLAYLOC,
    CREATEDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select c.USERAPPLTYPE,c.USERAPPL,c.PSEUDOCITY,c.LOC1TYPE,c.LOC1,"
        "       c.ITINDISPLAYIND,c.WPPSGTYPDISPLAY,c.ITINHEADERTEXTIND,"
        "       c.WPCONNECTIONIND,c.WPCHILDINFANTFAREBASIS,c.FAREBASISTKTDESLNG,"
        "       c.TRUEPSGRTYPEIND,c.FARETAXTOTALIND,c.NOOFTAXBOXES,"
        "       c.BASETAXEQUIVTOTALLENGTH,c.TAXPLACEMENTIND,c.TAXCURCODEDISPLAYIND,"
        "       c.ZPAMOUNTDISPLAYIND,c.TAXEXEMPTIONIND,c.TAXEXEMPTBREAKDOWNIND,"
        "       c.FCPSGTYPDISPLAY,c.FCCHILDINFANTFAREBASIS,c.FCCONNECTIONIND,"
        "       c.DOMESTICNUC,c.TVLCOMMENCEMENTDATE,c.WRAPAROUND,c.MULTISURCHARGESPACING,"
        "       c.DOMESTICISI,c.INTERNATIONALISI,c.DISPLAYBSR,c.ENDORSEMENTS,"
        "       c.WARNINGMESSAGES,c.LASTDAYTICKETDISPLAY,c.LASTDAYTICKETOUTPUT,"
        "       c.RESERVATIONOVERRIDE,c.FAREBASISDISPLAYOPTION,c.GLOBALSIDETRIPIND,"
        "       c.WPAPERMITTED,c.WPAPSGDTLFORMAT,c.WPAFARELINEPSGTYPE,c.WPAFARELINEHDR,"
        "       c.WPAPRIMEPSGREFNO,c.WPA2NDPSGREFNO,c.WPAFAREOPTIONMAXNO,c.WPASORT,"
        "       c.WPASHOWDUPAMTS,c.WPAPSGLINEBREAK,c.WPAPSGMULTILINEBREAK,"
        "       c.WPANOMATCHHIGHERCABINFARE,c.WPASTOREWITHOUTREBOOK,c.WPAACCTVLOPTION,"
        "       c.WPAACCTVLCAT13,c.WPNOMATCHPERMITTED,c.WPPSGDTLFORMAT,c.WPFARELINEPSGTYPE,"
        "       c.WPFARELINEHDR,c.WPPRIMEPSGREFNO,c.WP2NDPSGREFNO,c.WPFAREOPTIONMAXNO,"
        "       c.WPSORT,c.WPSHOWDUPAMTS,c.WPPSGLINEBREAK,c.WPPSGMULTILINEBREAK,"
        "       c.WPNOMATCHHIGHERCABINFARE,c.WPSTOREWITHOUTREBOOK,c.WPACCTVLOPTION,"
        "       c.WPACCTVLCAT13,c.VALCXRDISPLAYOPT,c.NEGPERMITTED,c.INTERLINETKTPERMITTED,"
        "       c.NOMATCHAVAIL,c.WPWPATRAILERMSG,c.PARTICIPATINGAGREEMENT,"
        "       c.WPROIND,c.WPAROIND,"
        "       c.APPLYDOMESTICMULTICUR,c.APPLYINTLMULTICUR,c.IETPRICEINTERLINEACTIVE,"
        "       c.VALUECODEBASE,"
        "       cs.MARKETLOC,cs.DISPLAYLOC,c.CREATEDATE");

    //		        this->From("=FARECALCCONFIG c LEFT OUTER JOIN =FARECALCCONFIGSEG cs "
    //		                   "                        USING
    //(USERAPPLTYPE,USERAPPL,PSEUDOCITY,LOC1TYPE,LOC1)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(5);
    joinFields.push_back("USERAPPLTYPE");
    joinFields.push_back("USERAPPL");
    joinFields.push_back("PSEUDOCITY");
    joinFields.push_back("LOC1TYPE");
    joinFields.push_back("LOC1");
    this->generateJoinString(
        "=FARECALCCONFIG", "c", "LEFT OUTER JOIN", "=FARECALCCONFIGSEG", "cs", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("c.USERAPPLTYPE = %1q "
                "   and c.USERAPPL = %2q "
                "   and c.PSEUDOCITY = %3q ");
    this->OrderBy("c.USERAPPLTYPE desc,c.USERAPPL desc,c.PSEUDOCITY desc,"
                  "          c.LOC1TYPE desc,c.LOC1 desc");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareCalcConfig* mapRowToFareCalcConfig(Row* row, FareCalcConfig* fcCfgPrev)
  {
    Indicator userApplType = row->getChar(USERAPPLTYPE);
    UserApplCode userAppl = row->getString(USERAPPL);
    PseudoCityCode pseudoCity = row->getString(PSEUDOCITY);
    LocTypeCode locType1 = row->getChar(LOC1TYPE);
    LocCode loc1 = row->getString(LOC1);

    FareCalcConfig* fcc;

    // If Parent hasn't changed, add to Child (segs)
    if (fcCfgPrev != nullptr && fcCfgPrev->userApplType() == userApplType &&
        fcCfgPrev->userAppl() == userAppl && fcCfgPrev->pseudoCity() == pseudoCity &&
        fcCfgPrev->loc1().locType() == locType1 && fcCfgPrev->loc1().loc() == loc1)
    {
      fcc = fcCfgPrev;
    }
    else
    { // Time for a new Parent
      fcc = new tse::FareCalcConfig;
      fcc->userApplType() = userApplType;
      fcc->userAppl() = userAppl;
      fcc->pseudoCity() = pseudoCity;
      fcc->loc1().locType() = locType1;
      fcc->loc1().loc() = loc1;
      fcc->itinDisplayInd() = row->getChar(ITINDISPLAYIND);
      fcc->wpPsgTypDisplay() = row->getChar(WPPSGTYPDISPLAY);
      fcc->itinHeaderTextInd() = row->getChar(ITINHEADERTEXTIND);
      fcc->wpConnectionInd() = row->getChar(WPCONNECTIONIND);
      fcc->wpChildInfantFareBasis() = row->getChar(WPCHILDINFANTFAREBASIS);
      fcc->fareBasisTktDesLng() = row->getChar(FAREBASISTKTDESLNG);
      fcc->truePsgrTypeInd() = row->getChar(TRUEPSGRTYPEIND);
      fcc->fareTaxTotalInd() = row->getChar(FARETAXTOTALIND);
      fcc->noofTaxBoxes() = row->getChar(NOOFTAXBOXES);
      fcc->baseTaxEquivTotalLength() = row->getInt(BASETAXEQUIVTOTALLENGTH);
      fcc->taxPlacementInd() = row->getChar(TAXPLACEMENTIND);
      fcc->taxCurCodeDisplayInd() = row->getChar(TAXCURCODEDISPLAYIND);
      fcc->zpAmountDisplayInd() = row->getChar(ZPAMOUNTDISPLAYIND);
      fcc->taxExemptionInd() = row->getChar(TAXEXEMPTIONIND);
      fcc->taxExemptBreakdownInd() = row->getChar(TAXEXEMPTBREAKDOWNIND);
      fcc->fcPsgTypDisplay() = row->getChar(FCPSGTYPDISPLAY);
      fcc->fcChildInfantFareBasis() = row->getChar(FCCHILDINFANTFAREBASIS);
      fcc->fcConnectionInd() = row->getChar(FCCONNECTIONIND);
      fcc->domesticNUC() = row->getChar(DOMESTICNUC);
      fcc->tvlCommencementDate() = row->getChar(TVLCOMMENCEMENTDATE);
      fcc->wrapAround() = row->getChar(WRAPAROUND);
      fcc->multiSurchargeSpacing() = row->getChar(MULTISURCHARGESPACING);
      fcc->domesticISI() = row->getChar(DOMESTICISI);
      fcc->internationalISI() = row->getChar(INTERNATIONALISI);
      fcc->displayBSR() = row->getChar(DISPLAYBSR);
      fcc->endorsements() = row->getChar(ENDORSEMENTS);
      fcc->warningMessages() = row->getChar(WARNINGMESSAGES);
      fcc->lastDayTicketDisplay() = row->getChar(LASTDAYTICKETDISPLAY);
      fcc->lastDayTicketOutput() = row->getChar(LASTDAYTICKETOUTPUT);
      fcc->reservationOverride() = row->getChar(RESERVATIONOVERRIDE);
      fcc->fareBasisDisplayOption() = row->getChar(FAREBASISDISPLAYOPTION);
      fcc->globalSidetripInd() = row->getChar(GLOBALSIDETRIPIND);
      fcc->wpaPermitted() = row->getChar(WPAPERMITTED);
      fcc->wpaPsgDtlFormat() = row->getChar(WPAPSGDTLFORMAT);
      fcc->wpaFareLinePsgType() = row->getChar(WPAFARELINEPSGTYPE);
      fcc->wpaFareLineHdr() = row->getChar(WPAFARELINEHDR);
      fcc->wpaPrimePsgRefNo() = row->getChar(WPAPRIMEPSGREFNO);
      fcc->wpa2ndPsgRefNo() = row->getChar(WPA2NDPSGREFNO);
      fcc->wpaFareOptionMaxNo() = row->getInt(WPAFAREOPTIONMAXNO);
      fcc->wpaSort() = row->getChar(WPASORT);
      fcc->wpaShowDupAmounts() = row->getChar(WPASHOWDUPAMTS);
      fcc->wpaPsgLineBreak() = row->getChar(WPAPSGLINEBREAK);
      fcc->wpaPsgMultiLineBreak() = row->getChar(WPAPSGMULTILINEBREAK);
      fcc->wpaNoMatchHigherCabinFare() = row->getChar(WPANOMATCHHIGHERCABINFARE);
      fcc->wpaStoreWithoutRebook() = row->getChar(WPASTOREWITHOUTREBOOK);
      fcc->wpaAccTvlOption() = row->getChar(WPAACCTVLOPTION);
      fcc->wpaAccTvlCat13() = row->getChar(WPAACCTVLCAT13);
      fcc->wpNoMatchPermitted() = row->getChar(WPNOMATCHPERMITTED);
      fcc->wpPsgDtlFormat() = row->getChar(WPPSGDTLFORMAT);
      fcc->wpFareLinePsgType() = row->getChar(WPFARELINEPSGTYPE);
      fcc->wpFareLineHdr() = row->getChar(WPFARELINEHDR);
      fcc->wpPrimePsgRefNo() = row->getChar(WPPRIMEPSGREFNO);
      fcc->wp2ndPsgRefNo() = row->getChar(WP2NDPSGREFNO);
      fcc->wpFareOptionMaxNo() = row->getInt(WPFAREOPTIONMAXNO);
      fcc->wpSort() = row->getChar(WPSORT);
      fcc->wpShowDupAmounts() = row->getChar(WPSHOWDUPAMTS);
      fcc->wpPsgLineBreak() = row->getChar(WPPSGLINEBREAK);
      fcc->wpPsgMultiLineBreak() = row->getChar(WPPSGMULTILINEBREAK);
      fcc->wpNoMatchHigherCabinFare() = row->getChar(WPNOMATCHHIGHERCABINFARE);
      fcc->wpStoreWithoutRebook() = row->getChar(WPSTOREWITHOUTREBOOK);
      fcc->wpAccTvlOption() = row->getChar(WPACCTVLOPTION);
      fcc->wpAccTvlCat13() = row->getChar(WPACCTVLCAT13);
      fcc->valCxrDisplayOpt() = row->getChar(VALCXRDISPLAYOPT);
      fcc->negPermitted() = row->getChar(NEGPERMITTED);
      fcc->interlineTktPermitted() = row->getChar(INTERLINETKTPERMITTED);
      fcc->noMatchAvail() = row->getChar(NOMATCHAVAIL);
      fcc->wpWpaTrailerMsg() = row->getChar(WPWPATRAILERMSG);
      fcc->participatingAgreement() = row->getChar(PARTICIPATINGAGREEMENT);
      fcc->wpRoInd() = row->getChar(WPROIND);
      fcc->wpaRoInd() = row->getChar(WPAROIND);
      fcc->applyDomesticMultiCurrency() = row->getChar(APPLYDOMESTICMULTICUR);
      fcc->applyIntlMultiCurrency() = row->getChar(APPLYINTLMULTICUR);
      fcc->ietPriceInterlineActive() = row->getChar(IETPRICEINTERLINEACTIVE);
      fcc->valueCodeBase() = row->getChar(VALUECODEBASE);
      fcc->createDate() = row->getDate(CREATEDATE);
    }

    // Load up new Seg
    if (!row->isNull(MARKETLOC))
    {
      FareCalcConfigSeg* newSeg = new FareCalcConfigSeg;

      newSeg->marketLoc() = row->getString(MARKETLOC);
      newSeg->displayLoc() = row->getString(DISPLAYLOC);

      fcc->segs().push_back(newSeg);
    }

    return fcc;
  } // mapRowToFareCalcConfig()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareCalcConfigs
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllFareCalcConfigsSQLStatement
    : public QueryGetFareCalcConfigsSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("1 = 1"); }
};
} // tse
