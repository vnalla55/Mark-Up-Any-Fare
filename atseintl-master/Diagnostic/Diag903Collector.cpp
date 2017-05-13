//----------------------------------------------------------------------------
//  File:        Diag903Collector.C
//  Created:     2004-08-20
//
//  Description: Diagnostic 903 formatter
//
//  Updates:
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
#include "Diagnostic/Diag903Collector.h"

#include "Common/Assert.h"
#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PosPaxType.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/RuleItemInfo.h"
#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/DiagCollector.h"

#include <iomanip>

namespace tse
{
class Diag903Collector::Diag903Filter
{
public:
  void overrideDoLocalMkts(bool& doLocalMkts) const
  {
    if (_view == "THRU")
      doLocalMkts = false;
    else if (_view == "ALL")
      doLocalMkts = true;
  }
  bool isFmFilteredOut(const FareMarket* const fm) const
  {
    return isFilteredOut(fm, _cxr, _orig, _dest);
  }
  bool isGovFmFilteredOut(const FareMarket* const fm) const
  {
    return isFilteredOut(fm, _govCxr, _govOrig, _govDest);
  }

  Diag903Filter(const Diagnostic* root, const Diag903Filter*& ptrToSet)
    : _view(root->diagParamMapItem("VW")),
      _cxr(root->diagParamMapItem("CX")),
      _orig(getMkt(root->diagParamMapItem("FM"), 0, 3)),
      _dest(getMkt(root->diagParamMapItem("FM"), 3, 6)),
      _govCxr(root->diagParamMapItem("CI")),
      _govOrig(getMkt(root->diagParamMapItem("FI"), 0, 3)),
      _govDest(getMkt(root->diagParamMapItem("FI"), 3, 6)),
      _ptrToSet(ptrToSet)
  {
    _ptrToSet = this;
  }

  ~Diag903Filter() { _ptrToSet = nullptr; }

private:
  static std::string getMkt(const std::string& diagMkt, const size_t begin, const size_t end)
  {
    return diagMkt.size() == 6 ? diagMkt.substr(begin, end) : "";
  }
  static bool isFilteredOut(const FareMarket* const fm,
                            const std::string& filterCxr,
                            const std::string& filterFMOrigin,
                            const std::string& filterFMDest)
  {
    return (!filterCxr.empty() && filterCxr != fm->governingCarrier()) ||
           (!filterFMOrigin.empty() && filterFMOrigin != fm->boardMultiCity()) ||
           (!filterFMDest.empty() && filterFMDest != fm->offMultiCity());
  }
  const std::string _view;
  const std::string _cxr;
  const std::string _orig;
  const std::string _dest;
  const std::string _govCxr;
  const std::string _govOrig;
  const std::string _govDest;
  const Diag903Filter*& _ptrToSet;
};

const Itin*
Diag903Collector::getCurrentItin(const ItinIndex& itinIndex, const ItinIndex::Key& cxrKey) const
{
  if (_shoppingTrx->getTrxType() == PricingTrx::ESV_TRX)
  {
    // Retrieve the itinerary
    return ShoppingUtil::getDummyItineraryFromCarrierIndex(const_cast<ItinIndex&>(itinIndex),
                                                           const_cast<ItinIndex::Key&>(cxrKey));
  }
  else
  {
    // Get the leaf
    const ItinIndex::ItinCell* const curCell =
        ShoppingUtil::retrieveDirectItin(itinIndex, cxrKey, ItinIndex::CHECK_NOTHING);

    // Retrieve the itinerary
    if (curCell)
    {
      return curCell->second;
    }
  }
  return nullptr;
}

bool
Diag903Collector::marketAlreadyDisplayed(const FareMarket* const fM,
                                         const FareMarket* const govFM,
                                         const size_t fmIdx)
{
  const std::pair<DisplayedMkts::iterator, bool> insertStatus = _displayedMkts.insert(
      DisplayedMkts::value_type(fM, DisplayMktInfo(govFM->governingCarrier(), fmIdx)));

  if (!insertStatus.second)
  {
    DiagCollector& dc(*this);
    const DisplayMktInfo& displInfo(insertStatus.first->second);

    dc << "\n\nMARKET HAS ALREADY BEEN DISPLAYED AS MKT " << displInfo.second << " FOR "
       << displInfo.first << "\n\n";
    return true;
  }

  return false;
}

bool
Diag903Collector::isThruFm(const FareMarket* const fM, const FareMarket* const govFM) const
{
  if (fM == govFM)
    return true;

  if (fM->boardMultiCity() == govFM->boardMultiCity() &&
      fM->offMultiCity() == govFM->offMultiCity())
    return true;

  if (fM->origin()->loc() == govFM->origin()->loc() &&
      fM->destination()->loc() == govFM->destination()->loc())
    return true;

  return false;
}

Diag903Collector& Diag903Collector::operator<<(const ItinIndex& itinIndex)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    if (!_filter)
      throw std::logic_error("Diag903Collector::operator<<(const ShoppingTrx&) wasn't called");

    if (itinIndex.root().empty())
    {
      return (*this);
    }

    // Adjust dc output to left justified
    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << std::endl;

    ItinIndex::ItinMatrixConstIterator iGIter = itinIndex.root().begin();
    const ItinIndex::ItinMatrixConstIterator iGEndIter = itinIndex.root().end();

    int count = 1;
    for (; iGIter != iGEndIter; ++iGIter)
    {
      const Itin* const curItin = getCurrentItin(itinIndex, iGIter->first);

      // If the retrieval failed, go to the next itinerary
      if (!curItin)
      {
        continue;
      }

      const std::vector<FareMarket*>& fMVector = curItin->fareMarket();

      const FareMarket* govFM(nullptr); // assumption: first fareMarket is the thru one.
      bool headerPrinted(false);
      size_t fmIdx(1);
      const size_t fmCnt(fMVector.size());

      // Retrieve fare markets
      for (std::vector<FareMarket*>::const_iterator fMIter = fMVector.begin();
           fMIter != fMVector.end();
           ++fMIter, ++fmIdx)
      {
        // Find the governing carrier from the fare market
        const FareMarket* const fM = *fMIter;
        if (fM == nullptr)
        {
          continue;
        }

        if (!govFM)
        {
          govFM = fM;

          // see if parameters to the diagnostic specify to exclude this gov fare market
          if (_filter->isGovFmFilteredOut(fM))
            break;
        }

        // see if parameters to the diagnostic specify to exclude this fare market
        if (_filter->isFmFilteredOut(fM))
          continue;

        if (!headerPrinted)
        {
          dc << "--------------------------------------------------------" << std::endl;
          dc << "LEG " << _legIndex << " GOVCXR " << count << " OF " << itinIndex.root().size()
             << " : " << govFM->governingCarrier() << std::endl;
          headerPrinted = true;
        }
        dc << "--------------------------------------------------------" << std::endl;

        if (_doLocalMarkets)
        {
          dc << "MKT " << fmIdx << " OF " << fmCnt << " FOR " << govFM->governingCarrier() << ": "
             << (isThruFm(fM, govFM) ? "THRU" : "LOCAL") << " ";
        }

        dc << fM->boardMultiCity() << "-" << fM->offMultiCity();

        if (_doLocalMarkets)
        {
          dc << "-" << fM->governingCarrier();
        }

        if (!marketAlreadyDisplayed(fM, govFM, fmIdx))
        {
          dc << *fM;
        }

        if (!_doLocalMarkets)
        {
          // For non-ESV and non-solo request there should be only one fare market
          break;
        }

      } // fMIter
      count++;
    }
    dc << "--------------------------------------------------------" << std::endl;
  }

  return (*this);
}

ShoppingTrx&
Diag903Collector::getShoppingTrx()
{
  ShoppingTrx* shoppingTrx = dynamic_cast<ShoppingTrx*>(trx());
  TSE_ASSERT(shoppingTrx);
  return *shoppingTrx;
}

const ShoppingTrx&
Diag903Collector::getShoppingTrx() const
{
  const ShoppingTrx* shoppingTrx = dynamic_cast<const ShoppingTrx*>(trx());
  TSE_ASSERT(shoppingTrx);
  return *shoppingTrx;
}

Diag903Collector& Diag903Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    // set _filter
    const Diag903Collector::Diag903Filter filter(dc.rootDiag(), _filter);
    // Set shopping trx
    _shoppingTrx = &shoppingTrx;
    _isSpanishDiscountTrx =
        _shoppingTrx->getOptions()->getSpanishLargeFamilyDiscountLevel() !=
            SLFUtil::DiscountLevel::NO_DISCOUNT;
    _displayedMkts.clear();

    _doLocalMarkets = _shoppingTrx->getTrxType() == PricingTrx::ESV_TRX ||
                      _shoppingTrx->isSumOfLocalsProcessingEnabled() ||
                      _shoppingTrx->isIataFareSelectionApplicable();
    _filter->overrideDoLocalMkts(_doLocalMarkets);

    const std::vector<ShoppingTrx::Leg>& sLV = shoppingTrx.legs();
    if (shoppingTrx.legs().empty())
    {
      return (*this);
    }

    std::vector<ShoppingTrx::Leg>::const_iterator sGLIter = sLV.begin();
    std::vector<ShoppingTrx::Leg>::const_iterator sGLEndIter = sLV.end();

    dc << "***************************************************" << std::endl;
    dc << "903 : FARE MARKETS" << std::endl;
    dc << "***************************************************" << std::endl;
    dc << std::endl;

    if (_trx->getRequest()->fareGroupRequested())
    {
      dc << "FGNUM STATUS" << std::endl;
      dc << "FAIL01 : NO MATCH FARE GROUP PAX TYPE OR PCC OR CORP ID" << std::endl;
      dc << "FAIL02 : USED ONLY FOR MINIMUM FARE AND DISCOUNTED" << std::endl;
      dc << "         FARE CREATION" << std::endl;
    }
    int count = 1;
    for (; sGLIter != sGLEndIter; ++sGLIter)
    {
      const ShoppingTrx::Leg& curLeg = *sGLIter;
      if (!_doLocalMarkets)
      {
        dc << "THRU-FARE MARKET" << std::endl;
      }
      dc << "LEG " << count << " OF " << sLV.size() << std::endl;
      if (curLeg.stopOverLegFlag())
      {
        dc << "#-ACROSS STOPOVER-#" << std::endl;
      }
      _legIndex = count;
      dc << curLeg.carrierIndex();
      count++;
    }
  }

  return (*this);
}

Diag903Collector& Diag903Collector::operator<<(const FareMarket& fareMarket)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    bool showVCTR =
        (_shoppingTrx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "VCTRINFO")
            ? true
            : false;

    // If we dont have travel segments, we count output this line
    if (fareMarket.travelSeg().size() == 0)
      return *this;

    std::vector<TravelSeg*>::const_iterator tvlSegItr;
    tvlSegItr = fareMarket.travelSeg().begin();

    dc << " #  " << fareMarket.getDirectionAsString() << "\n";

    const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

    // Display Fare market details
    dc << std::endl;
    dc << "FARE MARKET DETAILS: " << std::endl;
    dc << "  Origin: " << fareMarket.origin()->loc() << std::endl;
    dc << "  Destination: " << fareMarket.destination()->loc() << std::endl;
    dc << "  Board multi city: " << fareMarket.boardMultiCity() << std::endl;
    dc << "  Off multi city: " << fareMarket.offMultiCity() << std::endl;
    dc << "  Travel date: " << fareMarket.travelDate().dateToString(DDMMMYY, "") << std::endl;
    dc << "  Custom SOP FareMarket: "
       << (_shoppingTrx->isCustomSolutionFM(&fareMarket) ? "Yes" : "No") << std::endl;
    if (_isSpanishDiscountTrx)
    {
      dc << "  Spanish Family Discount: "
         << (_shoppingTrx->isSpanishDiscountFM(&fareMarket) ? "Yes" : "No") << std::endl;
    }

    // Print marketing carrier for ESV
    if (_doLocalMarkets)
    {
      dc << "  Marketing carrier: " << fareMarket.governingCarrier() << " ("
         << (fareMarket.isFlowJourneyFare() ? "F" : fareMarket.isLocalJourneyFare() ? "L" : "-")
         << ")" << std::endl;
    }

    if (fareMarket.isThroughFarePrecedenceNGS())
      dc << "  Through Fare Precedence enabled: Yes" << std::endl;

    _paxNum = 0;
    if (!paxTypeCortegeVec.empty())
    {
      std::vector<PaxTypeBucket>::const_iterator ptcIt = paxTypeCortegeVec.begin();
      std::vector<PaxTypeBucket>::const_iterator ptcEnd = paxTypeCortegeVec.end();

      while (ptcIt != ptcEnd)
      {
        const PaxTypeBucket& cortege = *ptcIt;
        const std::vector<PaxTypeFare*>& paxFareVec = cortege.paxTypeFare();
        if (!paxFareVec.empty())
        {
          dc << '\n';
          dc << "REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';

          dc << " INBOUND CURRENCY : " << cortege.inboundCurrency() << '\n';

          dc << "OUTBOUND CURRENCY : " << cortege.outboundCurrency() << '\n';
          dc << '\n';

          if (_trx->getRequest()->fareGroupRequested())
          {
            dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR FAR PAX  FG   \n"
               << "                            NUM R I              TYP TYP  NUM  \n"
               << "- -- - ---- --------------- --- - - -------- --- --- --- ------\n";
          }
          else
          {
            if (_shoppingTrx->getTrxType() == PricingTrx::FF_TRX)
            {
              dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR   ORIG FAR PAX\n"
                 << "                            NUM R I                 AMT TYP TYP\n"
                 << "- -- - ---- --------------- --- - - -------- --- ------ --- ---\n";
            }
            else
            {
              if (_shoppingTrx->awardRequest())
              {
                dc << "  GI V RULE   FARE BASIS    TRF O O     AMT CUR      MIL FAR PAX\n"
                   << "                            NUM R I                      TYP TYP\n"
                   << "- -- - ---- --------------- --- - - ------- --- -------- --- ---\n";
              }
              else
              {
                if (showVCTR)
                {
                  dc << "RULE VENDOR TRF  CC RULE SEQ      CAT LOC1 LOC1  LOC2 LOC2  FARE    FARE "
                        "ITEM   PASS  FAIL \n"
                     << "TYPE CODE                NUMBER       TYPE CODE  TYPE CODE  CLASS   TYPE "
                        "NUMBER COUNT COUNT\n"
                     << "---- ------ ---- -- ---- -------- --- ---- ----- ---- ----- ------- ---- "
                        "------ ----- -----\n";
                }
                else
                {
                  dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR FAR PAX\n"
                     << "                            NUM R I              TYP TYP\n"
                     << "- -- - ---- --------------- --- - - -------- --- --- ---\n";
                }
              }
            }
          }

          std::vector<PaxTypeFare*>::const_iterator ptfIt = paxFareVec.begin();
          std::vector<PaxTypeFare*>::const_iterator ptfEnd = paxFareVec.end();

          if (showVCTR)
          {
            typedef std::pair<uint16_t, uint16_t> PassFail;
            VecMap<const CategoryRuleInfo*, std::pair<PaxTypeFareRuleData*, PassFail> >
            vctrRuleInfoMap;
            VecMap<const CategoryRuleInfo*, std::pair<PaxTypeFareRuleData*, PassFail> >
            vctrGfrInfoMap;

            while (ptfIt != ptfEnd)
            {
              PaxTypeFare& paxFare = **ptfIt;

              // Collect Fare Rule Info
              PaxTypeFareRuleData* ptfRuleData1 = paxFare.paxTypeFareRuleData(1);
              if (ptfRuleData1 != nullptr && ptfRuleData1->categoryRuleInfo() != nullptr)
              {
                std::pair<PaxTypeFareRuleData*, PassFail>* pPair =
                    &vctrRuleInfoMap[ptfRuleData1->categoryRuleInfo()];
                pPair->first = ptfRuleData1;
                (paxFare.isCategoryValid(1)) ? (pPair->second.first)++ : (pPair->second.second)++;
              }
              PaxTypeFareRuleData* ptfRuleData15 = paxFare.paxTypeFareRuleData(15);
              if (ptfRuleData15 != nullptr && ptfRuleData15->categoryRuleInfo() != nullptr)
              {
                std::pair<PaxTypeFareRuleData*, PassFail>* pPair =
                    &vctrRuleInfoMap[ptfRuleData15->categoryRuleInfo()];
                pPair->first = ptfRuleData15;
                (paxFare.isCategoryValid(15)) ? (pPair->second.first)++ : (pPair->second.second)++;
              }

              // Collect General Rule Info
              PaxTypeFareRuleData* ptfGfrData1 = paxFare.paxTypeFareGfrData(1);
              if (ptfGfrData1 != nullptr && ptfGfrData1->categoryRuleInfo() != nullptr)
              {
                std::pair<PaxTypeFareRuleData*, PassFail>* pPair =
                    &vctrGfrInfoMap[ptfGfrData1->categoryRuleInfo()];
                pPair->first = ptfGfrData1;
                (paxFare.isCategoryValid(1)) ? (pPair->second.first)++ : (pPair->second.second)++;
              }
              PaxTypeFareRuleData* ptfGfrData15 = paxFare.paxTypeFareGfrData(15);
              if (ptfGfrData15 != nullptr && ptfGfrData15->categoryRuleInfo() != nullptr)
              {
                std::pair<PaxTypeFareRuleData*, PassFail>* pPair =
                    &vctrGfrInfoMap[ptfGfrData15->categoryRuleInfo()];
                pPair->first = ptfGfrData15;
                (paxFare.isCategoryValid(15)) ? (pPair->second.first)++ : (pPair->second.second)++;
              }

              ++ptfIt;
            }

            // Diplay aggregated VCTR info for the FM
            for (VecMap<const CategoryRuleInfo*,
                        std::pair<PaxTypeFareRuleData*, PassFail> >::const_iterator i =
                     vctrRuleInfoMap.begin();
                 i != vctrRuleInfoMap.end();
                 ++i)
            {
              // Print content of PaxTypeFareRuleData
              dc << std::setw(5) << "FR";
              dc << *(i->second.first);
              // Print number of fares matching this rule data
              dc << std::setw(6) << i->second.second.first << std::setw(6)
                 << i->second.second.second << '\n';
            }

            for (VecMap<const CategoryRuleInfo*,
                        std::pair<PaxTypeFareRuleData*, PassFail> >::const_iterator i =
                     vctrGfrInfoMap.begin();
                 i != vctrGfrInfoMap.end();
                 ++i)
            {
              // Print content of PaxTypeFareRuleData
              dc << std::setw(5) << "GR";
              dc << *(i->second.first);
              // Print number of fares matching this rule data
              dc << std::setw(6) << i->second.second.first << std::setw(6)
                 << i->second.second.second << '\n';
            }
          }
          else
          {
            while (ptfIt != ptfEnd)
            {
              PaxTypeFare& paxFare = **ptfIt;
              dc << paxFare;
              ++ptfIt;
            }
          }
        }
        else
        {
          dc << '\n' << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
             << fareMarket.destination()->loc()
             << ". REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';
        }
        ++ptcIt;
        ++_paxNum;
      }
    }
    else
    {
      dc << '\n' << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
         << fareMarket.destination()->description() << '\n';
    }

    dc << '\n';
  }

  return *this;
}

Diag903Collector& Diag903Collector::operator<<(const PaxTypeFare& paxFare)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(2) << cnvFlags(paxFare);

    std::string gd;
    globalDirectionToStr(gd, paxFare.fare()->globalDirection());

    dc << std::setw(3) << gd << std::setw(2) << Vendor::displayChar(paxFare.vendor())
       << std::setw(5) << paxFare.ruleNumber();

    std::string fareBasis = paxFare.createFareBasis(*_trx, false);
    if (fareBasis.size() > 15)
      fareBasis = fareBasis.substr(0, 15) + "*"; // Cross-of-lorraine?
    dc << std::setw(16) << fareBasis << std::setw(4) << paxFare.fareTariff();

    dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxFare);

    if (paxFare.directionality() == FROM)
      dc << std::setw(2) << "O";

    else if (paxFare.directionality() == TO)
      dc << std::setw(2) << "I";

    else
      dc << " ";

    dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";

    if (_shoppingTrx->awardRequest())
    {
      dc.setf(std::ios::right);
      dc << std::setw(8) << paxFare.mileage() << " ";
    }

    if (_shoppingTrx->getTrxType() == PricingTrx::FF_TRX &&
        !_trx->getRequest()->fareGroupRequested())
    {
      std::ios::fmtflags fmt =
          dc.setf(std::ios::right | std::ios::fixed | std::ios::adjustfield | std::ios::floatfield);

      int prec = dc.precision(1);
      dc << std::setw(6) << std::fixed << paxFare.originalFareAmount();
      dc.precision(prec);
      dc.setf(fmt);
    }

    if (!paxFare.isFareClassAppMissing())
    {
      dc << std::setw(4) << paxFare.fcaFareType();
    }
    else
    {
      dc << "UNK ";
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
      dc << "UNK ";
    }
    if (_trx->getRequest()->fareGroupRequested() &&
        !_trx->posPaxType()[_paxNum].empty() && // no fare group
        !paxFare.actualPaxTypeItem().empty() &&
        _paxNum < paxFare.actualPaxTypeItem().size())
    {
      uint16_t actualPaxTypeItem = paxFare.actualPaxTypeItem()[_paxNum];
      if (actualPaxTypeItem == PaxTypeFare::PAXTYPE_FAIL || !actualPaxTypeItem)
      {
        dc << "FAIL01";
      }
      // show fare that failed fare group but need to keep for minimum fare check ( keep same fare
      // as pricing )
      else if (actualPaxTypeItem == PaxTypeFare::PAXTYPE_NO_MATCHED)
      {
        dc << "FAIL02";
      }
      else
      {
        dc << actualPaxTypeItem;
      }
    }

    if (paxFare.isKeepForRoutingValidation())
    {
      dc << " SR";
    }

    dc << '\n';
    if (Vendor::displayChar(paxFare.vendor()) == '*')
    {
      dc << "    " << paxFare.vendor();
    }

    dc << '\n';
  }

  return *this;
}

Diag903Collector& Diag903Collector::operator<<(const PaxTypeFareRuleData& ptfRuleData)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    if (ptfRuleData.categoryRuleInfo() != nullptr)
    {
      const GeneralFareRuleInfo* generalFareRuleInfo =
          dynamic_cast<const GeneralFareRuleInfo*>(ptfRuleData.categoryRuleInfo());

      dc.setf(std::ios::left, std::ios::adjustfield);

      dc << std::setw(7) << generalFareRuleInfo->vendorCode() << std::setw(5)
         << generalFareRuleInfo->tariffNumber() << std::setw(3)
         << generalFareRuleInfo->carrierCode() << std::setw(5) << generalFareRuleInfo->ruleNumber()
         << std::setw(9) << generalFareRuleInfo->sequenceNumber() << std::setw(4)
         << generalFareRuleInfo->categoryNumber() << std::setw(5)
         << generalFareRuleInfo->loc1().locType() << std::setw(6)
         << generalFareRuleInfo->loc1().loc() << std::setw(5)
         << generalFareRuleInfo->loc2().locType() << std::setw(6)
         << generalFareRuleInfo->loc2().loc() << std::setw(8) << generalFareRuleInfo->fareClass()
         << std::setw(5) << generalFareRuleInfo->fareType();
    }
    else
    {
      return *this;
    }

    if (ptfRuleData.ruleItemInfo() != nullptr)
    {
      const RuleItemInfo& ruleItemInfo = *(ptfRuleData.ruleItemInfo());
      dc << std::setw(7) << ruleItemInfo.itemNo();
    }
    else
    {
      dc << std::setw(7) << '0';
    }
  }

  return *this;
}

} // namespace tse
