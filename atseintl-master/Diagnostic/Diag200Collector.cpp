//----------------------------------------------------------------------------
//  File:        Diag200Collector.C
//  Authors:     Mohammad Hossan, Vadim Nikushin
//  Created:     Feb 2004
//
//  Description: Diagnostic 02 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Diagnostic/Diag200Collector.h"

#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/FareTypeMatrixUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Money.h"
#include "Common/RtwUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Rules/RuleUtil.h"

#include <boost/utility.hpp>

#include <iomanip>

namespace tse
{
class Cat31PseudoDecorator : boost::noncopyable
{
public:
  explicit Cat31PseudoDecorator(const PricingTrx& trx) : _trx(trx) {}

  void initialize(const FareMarket& fm, Diag200Collector& dc)
  {
    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    {
      const RexPricingTrx& rexTrx = static_cast<const RexPricingTrx&>(_trx);

      _dc = &dc;
      _keepFareInThisFM = (rexTrx.getKeepPtf(fm) != nullptr);

      if (!rexTrx.needRetrieveHistoricalFare() &&
          (fm.retrievalFlag() & FareMarket::RetrievHistorical))
        _keepFareInParentFM = rexTrx.isFareMarketNeededForMinFares(fm);

      if (_keepFareInThisFM || _keepFareInParentFM)
        _dc->setCat31PseudoDecorator(this);
    }
  }

  ~Cat31PseudoDecorator()
  {
    if (_dc)
      _dc->setCat31PseudoDecorator(nullptr);
  }

  bool attachAdditionalInfo(const PaxTypeFare& ptf) const
  {
    if (_keepFareInParentFM)
    {
      if (ptf.isFareByRule())
      {
        *_dc << "FBR FARE NOT VALID FOR MIN FARE - REMOVED\n";
        return true;
      }

      if (ptf.isNegotiated())
      {
        *_dc << "NEGOTIATED FARE NOT VALID FOR MIN FARE - REMOVED\n";
        return true;
      }

      return false;
    }

    if (_keepFareInThisFM && ptf.retrievalFlag() == FareMarket::RetrievNone)
    {
      *_dc << "FARE HAS NO APPLICATION - REMOVED\n";
      return true;
    }

    return false;
  }

private:
  const PricingTrx& _trx;
  Diag200Collector* _dc = nullptr;
  bool _keepFareInThisFM = false;
  bool _keepFareInParentFM = false;
};

const std::string SEPARATOR = "- -- - ---- ------------ --- - - -------- --- --- --- --------";

const std::string SEPARATORAWARD =
    "- -- - ---- ------------ --- - - -------- --- -------- --- --- --------";

void
Diag200Collector::writeSeparator(SeparatorType st)
{
  if (_active)
  {
    std::string strLine;
    if (_application)
      strLine = "-";

    if (st == FARE_HEADER)
    {
      std::string strLine1, strLine2;
      if (_application)
      {
        strLine1 = "F";
        strLine2 = "A";
      }

      if (_trx->awardRequest())
      {
        ((DiagCollector&)*this)
            << "  GI V RULE  FARE BASIS  TRF O O      AMT CUR      MIL FAR PAX   CNV   " << strLine1
            << "\n"
            << "                         NUM R I                       TPE TPE   AMT   " << strLine2
            << "\n" << SEPARATORAWARD << strLine << "\n";
      }
      else
      {
        ((DiagCollector&)*this) << "  GI V RULE  FARE BASIS  TRF O O      AMT CUR FAR PAX   CNV   "
                                << strLine1 << "\n"
                                << "                         NUM R I              TPE TPE   AMT   "
                                << strLine2 << "\n" << SEPARATOR << strLine << "\n";
      }
    }
    else if (st == DIVIDER)
    {
      if (_trx->awardRequest())
      {
        ((DiagCollector&)*this)
            << "-------------------------------------------------------------------" << strLine
            << "\n";
      }
      else
      {
        ((DiagCollector&)*this) << "--------------------------------------------------------------"
                                << strLine << "\n";
      }
    }
  }
}

Diag200Collector&
Diag200Collector::operator << ( const PaxTypeFare& paxFare )
{
  // trap for debuggin. please don't remove ======================>
  //
  // if( paxFare.fareClass() == "Y2" )
  //  (*this) << "---------------- FareClass Y2 ------------------ \n";
  //
  // =================================================> end of trap

  if (_active)
  {
    if (!checkPaxTypeFare(paxFare))
      return *this;

    Diag200Collector& dc = *this;

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << std::setw(2) << cnvFlags(paxFare);

    std::string gd;
    globalDirectionToStr(gd, paxFare.fare()->globalDirection());

    dc << std::setw(3) << gd << std::setw(2) << Vendor::displayChar(paxFare.vendor())
       << std::setw(5) << paxFare.ruleNumber();
    //<< std::setw( 13) << paxFare.fareClass()
    std::string fareBasis = paxFare.createFareBasis(*_trx, false);
    if (fareBasis.size() > 12)
      fareBasis = fareBasis.substr(0, 12) + "*"; // Cross-of-lorraine?
    dc << std::setw(13) << fareBasis << std::setw(4) << paxFare.tcrRuleTariff();

    dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxFare);

    if (paxFare.directionality() == FROM)
      dc << std::setw(2) << "O";

    else if (paxFare.directionality() == TO)
      dc << std::setw(2) << "I";

    else
      dc << "  ";

    if (paxFare.invalidFareCurrency())
    {
      dc << " MISSING NUC RATE FOR " << paxFare.currency() << '\n';
      return *this; // return so that invalid parts of the paxFare are not attempted.
    }

    else // FareCurrency is valid
    {

      dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";

      if (_trx)
        if (_trx->awardRequest())
        {
          dc.setf(std::ios::right);
          dc << std::setw(8) << paxFare.mileage();
        }

      if (!paxFare.isFareClassAppMissing())
      {
        dc << std::setw(4) << paxFare.fcaFareType();
      }
      else
      {
        dc << "UNK";
      }

      if (!paxFare.isFareClassAppSegMissing())
      {
        if (paxFare.fcasPaxType().empty())
          dc << "*** ";
        else
          dc << std::setw(4) << paxFare.fcasPaxType();
      }
      else
      {
        dc << "UNK";
      }

      dc.setf(std::ios::right, std::ios::adjustfield);
      dc.setf(std::ios::fixed, std::ios::floatfield);
      dc.precision(2);
      dc << std::setw(8) << paxFare.nucFareAmount();
    } // end of else -  FareCurrency is valid

    if (_application)
    {
      FareMarket::FareRetrievalFlags flag = paxFare.retrievalFlag();

      dc << FareMarket::fareRetrievalFlagToStr(flag);
    }
    dc << '\n';

    if (_application)
    {
      RexPricingTrx* rexTrx = dynamic_cast<RexPricingTrx*>(_trx);
      if (rexTrx && !rexTrx->newItinSecondROEConversionDate().isEmptyDate() &&
          paxFare.rexSecondNucFareAmount() > EPSILON)
      {
        dc << "                                     SECOND NUC AMT : ";
        dc << std::setw(8) << paxFare.rexSecondNucFareAmount() << "\n";
      }
    }
    if (Vendor::displayChar(paxFare.vendor()) == '*')
    {
      dc << "     " << paxFare.vendor() << '\n';
    }

    if (_info)
    {
      if (_cat31PseudoDecorator && _cat31PseudoDecorator->attachAdditionalInfo(paxFare))
        return *this;

      dc << "  MKT1:" << paxFare.market1() << " MKT2:" << paxFare.market2() << "  DIR:";

      if (paxFare.fare()->directionality() == FROM)
        dc << std::setw(2) << "F";
      else if (paxFare.fare()->directionality() == TO)
        dc << std::setw(2) << "T";
      else
        dc << "  ";

      dc << "  ORIGIN:" << paxFare.origin() << " DEST:" << paxFare.destination();

      dc << (paxFare.fare()->isReversed() ? "  REV:Y " : "  REV:N ") << '\n';

      dc << "  FARETARIFF: " << paxFare.fareTariff() << "  FTN1: " << paxFare.footNote1()
         << "  FTN2: " << paxFare.footNote2() << "  RTNG: " << paxFare.routingNumber();
      if (paxFare.fare()->domesticFootNote() != BLANK)
      {
        dc << "  DOM FTN: " << paxFare.fare()->domesticFootNote();
      }
      dc << " \n";

      displayFareDates(paxFare);

      dc << (paxFare.isCategoryValid(1) ? "  CAT1-P " : "  CAT1-F ")
         << (paxFare.fare()->isCategoryValid(15) ? " CAT15-P " : " CAT15-F ")
         << (paxFare.isCategoryValid(35) ? " CAT35-P " : " CAT35-F ");

      PricingTrx* pTrx = dynamic_cast<PricingTrx*>(_trx);
      if (pTrx && RtwUtil::isRtw(*pTrx))
      {
        ValidateContinentBased vcb(*pTrx, _rtwContinentCount);
        if (vcb.continentBasedFare(fareBasis))
          dc << " MAX CONTINENTS " << _rtwContinentCount << " -" << (vcb(&paxFare) ? "F " : "P ");
      }
      dc << '\n';

      if (!paxFare.isFareClassAppMissing()) // and if /DDR1 is reques  ted
      {
        const FareClassAppInfo* fca = paxFare.fareClassAppInfo();

        dc << "  R1-"
           << " SEQ:" << fca->_seqNo << " FC:" << fca->_fareClass << " OWRT:" << fca->_owrt
           << " CNT:" << fca->_segCount << " LOC1:" << fca->_location1Type << "-" << fca->_location1
           << " LOC2:" << fca->_location2Type << "-" << fca->_location2 << '\n';

        dc << "  N/S:" << (paxFare.isNormal() ? "N" : "S")
           << " CONST:" << (paxFare.isConstructed() ? "Y" : "N")
           << " YY:" << (paxFare.fare()->isIndustry() ? "Y" : "N")
           << " FBR:" << (paxFare.isFareByRule() ? "Y" : "N")
           << " DISC:" << (paxFare.isDiscounted() ? "Y" : "N") << " DOW:" << paxFare.fcaDowType()
           << " SN:" << paxFare.fcaSeasonType() << " NEG:" << (paxFare.isNegotiated() ? "Y" : "N")
           << " WEB:" << (paxFare.isWebFare() ? "Y" : "N") << '\n';

        FareClassAppSegInfoList::const_iterator fcas = fca->_segs.begin();
        for (; fcas != fca->_segs.end(); fcas++)
        {
          dc << "  R1B- DIR: " << (*fcas)->_directionality;

          if ((*fcas)->_paxType.empty())
            dc << " PAX: ***";
          else
            dc << " PAX: " << (*fcas)->_paxType;

          if ((*fcas) == paxFare.fareClassAppSegInfo())
            dc << " - PASS";
          dc << '\n';

          dc << "  ACTUAL PAXTYPE:" << paxFare.actualPaxType()->paxType()
             << "  AGE: " << paxFare.actualPaxType()->age()
             << "   NBR: " << paxFare.actualPaxType()->number() << '\n';
        }
      }

      // constructed fare details

      if (paxFare.isConstructed())
      {
        dc << "  GW1: " << paxFare.gateway1() << " GW2: " << paxFare.gateway2()
           << " SPEC FCLASS: " << paxFare.fareClass()
           << " SPEC AMT: " << Money(paxFare.specifiedFareAmount(), paxFare.currency()) << '\n';

        if (paxFare.constructionType() != ConstructedFareInfo::SINGLE_DESTINATION)
        {
          dc << "  ORIG ADN FCLASS: " << paxFare.origAddonFareClass()
             << " TAR: " << paxFare.origAddonTariff() << " FTN1: " << paxFare.origAddonFootNote1()
             << " FTN2: " << paxFare.origAddonFootNote2() << '\n'
             << "           RTNG: " << paxFare.origAddonRouting()
             << " AMT: " << Money(paxFare.origAddonAmount(), paxFare.origAddonCurrency()) << '\n';
        }

        if (paxFare.constructionType() != ConstructedFareInfo::SINGLE_ORIGIN)
        {
          dc << "  DEST ADN FCLASS: " << paxFare.destAddonFareClass()
             << " TAR: " << paxFare.destAddonTariff() << " FTN1: " << paxFare.destAddonFootNote1()
             << " FTN2: " << paxFare.destAddonFootNote2() << '\n'
             << "           RTNG: " << paxFare.destAddonRouting()
             << " AMT: " << Money(paxFare.destAddonAmount(), paxFare.destAddonCurrency()) << '\n';
        }
      }
    }
  }
  return *this;
}

bool
Diag200Collector::checkPaxTypeFare(const PaxTypeFare& paxFare)
{
  if (_active)
  {
    // Inhibit display for Abacus User for fares failed by Security
    if (_restrictSecurity && paxFare.cat15SecurityFail())
    {
      return false;
    }

    /*
        if (!paxFare.invalidFareCurrency())
        {
    */

    if (!_ddAllPax && !_allFares && !_info)
    {
      if (!paxFare.isValid())
      {
        return false;
      }
    }

    if (!_vendor.empty())
    {
      if (paxFare.vendor() != _vendor)
        return false;
    }

    if (!_fareClass.empty())
    {
      if (!RuleUtil::matchFareClass(_fareClass.c_str(), paxFare.fareClass().c_str()))
        return false;
    }

    if (_checkSpecial)
    {
      if (_isSpecial != paxFare.isSpecial())
        return false;
    }

    if (_globalDirection != GlobalDirection::ZZ)
    {
      if (paxFare.globalDirection() != GlobalDirection::ZZ &&
          _globalDirection != paxFare.globalDirection())
        return false;
    }

    if (!_fareTypeDesignator.isFTDUnknown())
    {
      if (_fareTypeDesignator != paxFare.fareTypeDesignator())
        return false;
    }

    if (!_fareType.empty())
    {
      if (!RuleUtil::matchFareType(_fareType, paxFare.fcaFareType()))
        return false;
    }

    if (!_fareBasis.empty())
    {
      if (_fareBasis != paxFare.createFareBasis(*_trx, false))
        return false;
    }

    if ((_tariffNumber && _tariffNumber != paxFare.tcrRuleTariff()) ||
        (!_ruleNumber.empty() && _ruleNumber != paxFare.ruleNumber()))
      return false;
  }

  return true;
}

void
Diag200Collector::print(const Itin& itn, const FareMarket& fareMarket)
{
  if (_active)
  {
    Cat31PseudoDecorator c31pd(*_trx);
    c31pd.initialize(fareMarket, *this);

    if(!_trx->isValidatingCxrGsaApplicable())
    {
      *this << itn << fareMarket;
    }
    else
    {
      Diag200Collector& dc = *this;
      if (dynamic_cast<const ExcItin*>(&itn))
         dc << "EXCHANGE ITIN " << '\n';

         dc << "CONVERTED CURRENCY : " << itn.calculationCurrency() << '\n';

      if(!fareMarket.validatingCarriers().empty())
      {
         dc << "TICKETING CARRIER : ";
         for (const CarrierCode& cxr : fareMarket.validatingCarriers())
         {
           dc << cxr << "  ";
         }
         dc  << '\n';
      }
      else
        dc << "TICKETING CARRIER : " << itn.ticketingCarrier() << '\n';

      *this << fareMarket;
    }
  }
}

Diag200Collector&
Diag200Collector::operator << ( const FareMarket& fareMarket )
{
  if (_active)
  {
    if (_ddFareCount)
    {
      return displayFareCount(fareMarket);
    }

    if (_ddAllPax)
    {
      return displayAllPax(fareMarket);
    }

    Diag200Collector& dc = *this;

    // If we dont have travel segments, we count output this line
    if (fareMarket.travelSeg().size() == 0)
      return *this;

    ExchangePricingTrx* exchangePricingTrx = dynamic_cast<ExchangePricingTrx*>(_trx);
    if (exchangePricingTrx != nullptr)
    {
      dc << "TICKETING DATE : "
         << exchangePricingTrx->dataHandle().ticketDate().dateToString(YYYYMMDD, "-") << "\n"
         << "ROE RETRIEVAL DATE: "
         << exchangePricingTrx->dataHandle().ticketDate().toIsoExtendedString() << "\n";
    }

    std::vector<TravelSeg*>::const_iterator tvlSegItr;
    tvlSegItr = fareMarket.travelSeg().begin();

    dc << " \n" << FareMarketUtil::getFullDisplayString(fareMarket) << "\n";

    if (_trx->getRequest()->multiTicketActive() && !_trx->multiTicketMap().empty())
    {
      dc.displayFareMarketItinNumsMultiTkt("\nAPPLIES TO ITINS :", *_trx, fareMarket);
    }
    else
    {
      dc.displayFareMarketItinNums(" \nAPPLIES TO ITINS :", *_trx, fareMarket);
    }
    dc << " \n";

    dc << "GEOTRAVELTYPE : ";
    dc << DiagnosticUtil::geoTravelTypeToString(fareMarket.geoTravelType());
    dc << "\n \n";

    if (SLFUtil::isSpanishFamilyDiscountApplicable(*_trx))
    {
      if (SLFUtil::DiscountLevel::LEVEL_1 == _trx->getOptions()->getSpanishLargeFamilyDiscountLevel())
        dc << "SPANISH LARGE FAMILY DISCOUNT: LEVEL 1 - 5% APPLIED" << std::endl;
      else if (SLFUtil::DiscountLevel::LEVEL_2 == _trx->getOptions()->getSpanishLargeFamilyDiscountLevel())
        dc << "SPANISH LARGE FAMILY DISCOUNT: LEVEL 2 - 10% APPLIED" << std::endl;
    }

    RexPricingTrx* rexTrx = dynamic_cast<RexPricingTrx*>(_trx);

    const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();
    if (!paxTypeCortegeVec.empty())
    {
      std::vector<PaxTypeBucket>::const_iterator ptcIt = paxTypeCortegeVec.begin();
      std::vector<PaxTypeBucket>::const_iterator ptcEnd = paxTypeCortegeVec.end();

      for (; ptcIt != ptcEnd; ++ptcIt)
      {
        const PaxTypeBucket& cortege = *ptcIt;
        const std::vector<PaxTypeFare*>& paxFareVec = cortege.paxTypeFare();
        if (!paxFareVec.empty())
        {
          dc << " \n";
          dc << "REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';

          dc << " INBOUND CURRENCY : " << cortege.inboundCurrency() << '\n';

          dc << "OUTBOUND CURRENCY : " << cortege.outboundCurrency() << '\n';

          if (fareMarket.breakIndicator())
            dc << "N/A FOR PRICING\n";
          else
          {
            if (!fareMarket.fareBasisCode().empty())
              dc << "FARE CLASS " << fareMarket.fareBasisCode() << " FOR PRICING\n";
          }
          if (fareMarket.removeOutboundFares())
            dc << "OUTBOUND FARE REMOVED\n";

          dc << " \n";

          if (rexTrx)
          {
            if (rexTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
            {
              _application = true;

              RexExchangeTrx* rexExcTrx = dynamic_cast<RexExchangeTrx*>(_trx);
              if (rexExcTrx)
              {
                dc << "ITIN " << _itinIndex << " - ";
              }

              if (!rexTrx->needRetrieveHistoricalFare() &&
                  (fareMarket.retrievalFlag() & FareMarket::RetrievHistorical))
              {
                dc << "COLLECT HISTORICAL FARE FOR HIP PROCESS ON KEEP FARE\n";
              }

              if (!rexTrx->newItinROEConversionDate(_itinIndex).isEmptyDate())
              {
                if ((rexTrx->newItin()[_itinIndex])->exchangeReissue() == EXCHANGE)
                  dc << "EXCHANGE - ";
                else
                  dc << "REISSUE - ";
                dc << "ROE RETRIEVAL DATE : " << rexTrx->newItinROEConversionDate(_itinIndex)
                                                     .toIsoExtendedString() << "\n";
                if (!rexTrx->newItinSecondROEConversionDate().isEmptyDate())
                  dc << "SECOND ROE RETRIEVAL DATE : "
                     << rexTrx->newItinSecondROEConversionDate().toIsoExtendedString() << "\n";
              }
            }
            if (rexTrx->isAnalyzingExcItin())
            {
              if (rexTrx->isPlusUpCalculationNeeded() &&
                  rexTrx->exchangeItin().front()->isFareMarketJustForRexPlusUps(&fareMarket))
              {
                dc << "COLLECT HISTORICAL FARE FOR HIP PROCESS ON KEEP FARE\n";
              }

              if (rexTrx->applyReissueExchange())
              {
                dc << "RETRIEVAL DATE: " << rexTrx->ticketingDate().toIsoExtendedString() << "\n";
              }
            }
          }

          if (fareMarket.retrievalDate() != DateTime::emptyDate())
            dc << "RETRIEVAL DATE: " << fareMarket.retrievalDate().toIsoExtendedString() << "\n";
          dc << " \n";

          writeSeparator(FARE_HEADER);

          if(fareMarket.failCode() != ErrorResponseException::NO_ERROR)
            dc << "ERROR: BAD FARE MARKET WITH ERROR CODE: " << fareMarket.failCode() << "\n";
          else
          {
            std::vector<PaxTypeFare*>::const_iterator ptfIt = paxFareVec.begin();
            std::vector<PaxTypeFare*>::const_iterator ptfEnd = paxFareVec.end();
            bool isValidPaxTypeFareFound = false;

            for (; ptfIt != ptfEnd; ++ptfIt)
            {
              PaxTypeFare& paxFare = **ptfIt;
              dc << paxFare;
              isValidPaxTypeFareFound = isValidPaxTypeFareFound || checkPaxTypeFare(paxFare);
            }
            if( !isValidPaxTypeFareFound )
              dc << "***NO VALID PAXTYPE FARE FOUND IN THE FARE MARKET***" << "\n";
          }
        }
        else
        {
          dc << " \n";
          dc << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
             << fareMarket.destination()->loc()
             << ". REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';
        }
      }
    }
    else
    {
      dc << " \n";
      dc << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
         << fareMarket.destination()->description() << '\n';
    }

    dc << SEPARATOR;
    if (_application)
      dc << "-";
    dc << "\n \n";
  }

  return *this;
}

Diag200Collector&
Diag200Collector::operator << ( const Itin& itn )
{
  if (_active)
  {
    Diag200Collector& dc = *this;

    if (dynamic_cast<const ExcItin*>(&itn))
      dc << "EXCHANGE ITIN " << '\n';

    dc << "CONVERTED CURRENCY : " << itn.calculationCurrency() << '\n';

    dc << "TICKETING CARRIER : " << itn.ticketingCarrier() << '\n';
  }

  return *this;
}

Diag200Collector&
Diag200Collector::operator << ( const PricingTrx& trx )
{
  if (_active)
  {
    Diag200Collector& dc = *this;

    dc << "CONVERTED CURRENCY : " << trx.itin()[0]->calculationCurrency() << '\n';
  }

  return *this;
}

bool
Diag200Collector::parseQualifiers(PricingTrx& trx, const FareMarket& fareMarket, const Itin* itin)
{
  if (!parseFareMarket(trx, fareMarket))
    return false;

  if (RtwUtil::isRtw(trx))
  {
    const std::vector<AirlineAllianceCarrierInfo*>& aaci =
        trx.dataHandle().getAirlineAllianceCarrier(fareMarket.governingCarrier());

    if (!aaci.empty())
      _rtwContinentCount = ItinUtil::countContinents(trx, fareMarket, *aaci.front());
  }

  // yes, we do care. parse diagnostic qualifiers.
  parseQualifiers(trx);

  if (itin)
  {
    RexExchangeTrx* rexExcTrx = dynamic_cast<RexExchangeTrx*>(&trx);
    if (rexExcTrx)
    {
      _itinIndex = rexExcTrx->getItinPos(itin);
    }
  }

  return true;
}

void
Diag200Collector::parseQualifiers(PricingTrx& trx)
{
  _trx = &trx;

  std::map<std::string, std::string>::const_iterator e = trx.diagnostic().diagParamMap().end();

  std::map<std::string, std::string>::const_iterator i;

  i = trx.diagnostic().diagParamMap().find(Diagnostic::DIAG_VENDOR);
  if (i != e)
    if (!i->second.empty())
      _vendor = i->second;

  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
  if (i != e)
    if (!i->second.empty())
      _fareClass = i->second;

  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_ASSIGNMENT);
  if (i != e)
  {
    _isSpecial = (i->second == "S");
    _checkSpecial = true;
  }

  i = trx.diagnostic().diagParamMap().find(Diagnostic::GLOBAL_DIRECTION);
  if (i != e)
    if (!i->second.empty())
      strToGlobalDirection(_globalDirection, i->second.c_str());

  i = trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);
  if (i != e)
  {
    _info = (i->second == "INFO");
    _allFares = (i->second == "ALLFARES");
    _displayR1 = (i->second == "ALLFARES");
    _ddAllPax = (i->second == "ALLPAX");
    _ddFareCount = (i->second == "FARECOUNT");
  }

  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_TYPE_DESIGNATOR);
  if (i != e)
    _fareTypeDesignator = FareTypeMatrixUtil::convert(i->second);

  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_TYPE);
  if (i != e)
    _fareType = i->second;

  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_BASIS_CODE);
  if (i != e)
    _fareBasis = i->second;

  i = trx.diagnostic().diagParamMap().find(Diagnostic::TARIFF_NUMBER);
  if (i != e)
    _tariffNumber = std::atoi(i->second.c_str());

  i = trx.diagnostic().diagParamMap().find(Diagnostic::RULE_NUMBER);
  if (i != e)
    _ruleNumber = i->second;

  _restrictSecurity = trx.getRequest()->ticketingAgent()->abacusUser();
}

Diag200Collector&
Diag200Collector::displayFareCount(const FareMarket& fareMarket)
{
  if (fareMarket.travelSeg().size() == 0)
    return *this;

  Diag200Collector& dc = *this;

  std::vector<TravelSeg*>::const_iterator tvlSegItr;
  tvlSegItr = fareMarket.travelSeg().begin();

  dc << " \n" << FareMarketUtil::getFullDisplayString(fareMarket) << "\n \n";

  std::map<std::string, int> fmFareCounter;
  std::string fareType;

  const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();
  if (!paxTypeCortegeVec.empty())
  {
    std::vector<PaxTypeBucket>::const_iterator ptcIt = paxTypeCortegeVec.begin();
    std::vector<PaxTypeBucket>::const_iterator ptcEnd = paxTypeCortegeVec.end();

    for (; ptcIt != ptcEnd; ++ptcIt)
    {
      const PaxTypeBucket& cortege = *ptcIt;
      const std::vector<PaxTypeFare*>& paxFareVec = cortege.paxTypeFare();
      if (!paxFareVec.empty())
      {
        std::vector<PaxTypeFare*>::const_iterator ptfIt = paxFareVec.begin();
        std::vector<PaxTypeFare*>::const_iterator ptfEnd = paxFareVec.end();

        for (; ptfIt != ptfEnd; ++ptfIt)
        {
          fareType = cnvFlags(**ptfIt);
          fmFareCounter[fareType] += 1;
        }
      }
    }

    int total = 0;
    dc << "\nFARE COUNT - ";
    for (const auto& elem : fmFareCounter)
    {
      dc << std::setw(5) << elem.first << ":" << std::setw(8) << elem.second << '\n'
         << "             ";
      total += elem.second;
    }
    dc << "TOTAL:" << std::setw(8) << total << '\n';
    writeSeparator(DIVIDER);
  }

  return *this;
}

Diag200Collector&
Diag200Collector::displayAllPax(const FareMarket& fareMarket)
{
  // If we dont have travel segments, we count output this line
  if (fareMarket.travelSeg().size() == 0)
    return *this;

  Diag200Collector& dc = *this;

  std::vector<TravelSeg*>::const_iterator tvlSegItr;
  tvlSegItr = fareMarket.travelSeg().begin();

  dc << " \n" << FareMarketUtil::getFullDisplayString(fareMarket) << "\n \n";

  dc << "GEOTRAVELTYPE : ";
  dc << DiagnosticUtil::geoTravelTypeToString(fareMarket.geoTravelType());

  dc << "\n \n";
  if (fareMarket.breakIndicator())
    dc << "N/A FOR PRICING\n";
  else
  {
    if (!fareMarket.fareBasisCode().empty())
      dc << "FARE CLASS " << fareMarket.fareBasisCode() << " FOR PRICING\n";
  }
  if (fareMarket.removeOutboundFares())
    dc << "OUTBOUND FARE REMOVED\n";

  if (!fareMarket.allPaxTypeFare().empty())
  {
    writeSeparator(FARE_HEADER);

    std::vector<PaxTypeFare*>::const_iterator ptfIt = fareMarket.allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::const_iterator ptfEnd = fareMarket.allPaxTypeFare().end();

    for (; ptfIt != ptfEnd; ++ptfIt)
    {
      dc << **ptfIt;
    }

    writeSeparator(DIVIDER);
  }
  else
  {
    dc << " \nNO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
       << fareMarket.destination()->loc() << '\n';
    writeSeparator(DIVIDER);
  }

  return *this;
}
}
