/* --*- vim: set ts=2 sw=2 sts=2 et: -*-- */
//----------------------------------------------------------------------------
//  File:     COPMinimumFare.cpp
//  Created:
//  Authors:
//
//  Description    : Country of Payment Minimum Fare Check.
//
//  Copyright Sabre 2003
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------

#include "MinFares/COPMinimumFare.h"

#include "Common/DiagMonitor.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/GoverningCarrier.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CopMinimum.h"
#include "DBAccess/CopParticipatingNation.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "Diagnostic/DCFactory.h"
#include "MinFares/MatchRuleLevelExclTable.h"
#include "MinFares/MinFareLogic.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "MinFares/MinFareSpecialFareSelection.h"

namespace tse
{
static Logger
logger("atseintl.MinFares.COPMinimumFare");

COPMinimumFare::COPMinimumFare(PricingTrx& trx, FarePath& farePath)
  : MinimumFare(farePath.itin()->travelDate()),
    _trx(trx),
    _farePath(farePath),
    _dataHandle(trx.ticketingDate())
{
  _diagType = trx.diagnostic().diagnosticType();
  if (UNLIKELY(_diagType == Diagnostic702 || _diagType == Diagnostic765))
  {
    _diagEnabled = true;
  }

  const Loc* saleLoc = TrxUtil::saleLoc(_trx);
  if (LIKELY(saleLoc != nullptr))
    _saleCountry = saleLoc->nation();

  _tktgCxr = _farePath.itin()->validatingCarrier().empty() ? _farePath.itin()->ticketingCarrier()
                                                           : _farePath.itin()->validatingCarrier();

  if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
    adjustPortExcDates(trx);
}

/**
 *   @method ~COPMinimumFare
 *
 *   Description: Reclaims diagnostic output.
 **/

COPMinimumFare::~COPMinimumFare()
{
  if (UNLIKELY(_diagEnabled))
  {
    DiagManager diag(_trx, _diagType);
    diag << _diag.str();
  }
}

/**
 *   @method process
 *
 *   Description: Apply COP Processing Logic:
 *                Check if PU qualifies for COP -- qualifyPu()
 *                Perform matching logic        -- matchPu()
 *                Perform Fare Calculation      -- calculateFare()
 *                Select the appropriate Fare   -- selectFare()
 *                Perform Fare Comparison       -- compareFare()
 *                Finally update the COP plus-up amount and return.
 *
 *   @return true if COP is processed without error, false otherwise.
 */
MoneyAmount
COPMinimumFare::process()
{
  if (UNLIKELY(_diagEnabled))
  {
    _diag << " \n"
             "********************** COUNTRY OF PAYMENT *********************\n";
  }

  // Proceed only if the transaction is sold outside
  // country of commencement of journey, i.e., SOTI or SOTO.
  Itin* itin = _farePath.itin(); // lint !e530
  if (UNLIKELY(itin == nullptr))
    return 0.0;

  if (UNLIKELY(diagEnabled(Diagnostic702)))
    _diag << "COP/" << _saleCountry << " TKTG CXR/" << _tktgCxr << "\n";

  if ((_farePath.intlSaleIndicator() != Itin::SOTI) &&
      (_farePath.intlSaleIndicator() != Itin::SOTO))
  {
    if (UNLIKELY(_diagEnabled))
    {
      _diag << "ISI IS NOT SOTI OR SOTO.\n"
            << "COP DOES NOT APPLY TO THE ITIN.\n";
    }

    return 0.0;
  }

  // Process for each PU in the fare path and accumulate COP plus up
  MoneyAmount copPlusUpAmount = 0.0;
  const std::vector<PricingUnit*>& puVec = _farePath.pricingUnit();
  std::vector<PricingUnit*>::const_iterator i = puVec.begin();
  for (; i != puVec.end(); ++i)
  {
    PricingUnit& pu = **i;
    if (!pu.isSideTripPU() && processPU(pu))
    {
      copPlusUpAmount += pu.minFarePlusUp().getSum(COP);
    }

    // Check side trip PUs
    std::vector<PricingUnit*>::const_iterator stPuI = pu.sideTripPUs().begin();
    for (; stPuI != pu.sideTripPUs().end(); ++stPuI)
    {
      PricingUnit& stPu = **stPuI;
      if (processPU(stPu))
      {
        copPlusUpAmount += stPu.minFarePlusUp().getSum(COP);
      }
    }
  }

  return copPlusUpAmount;
}

bool
COPMinimumFare::processPU(PricingUnit& pu)
{
  displayPu(pu);

  if (pu.exemptMinFare())
  {
    LOG4CXX_INFO(logger, "PU is exempted.");
    if (_diagEnabled)
    {
      _diag << "PU IS EXEMPTED.\n";
    }
    return false;
  }

  if (qualifyPu(pu) && // Does PU qualifies for COP?
      matchPu(pu)) // match PU against COP table
  {
    if (diagEnabled(Diagnostic702))
    {
      _diag << "COP APPLIES TO PU.\n";
    }

    return calculateFare(pu);
  }
  else
  {
    if (_diagEnabled)
    {
      _diag << "COP DOES NOT APPLY TO PU.\n";
    }
    return false;
  }
}

/**
 *   @method qualifyPu
 *
 *   Description: Check if the Pricing Unit qualifies for COP.
 *                Proceed only if the PU is RT/CT or RT/CT including side-trip.
 *                Find the PU Sale/TicketIndicator .
 *                Find the PU type based on FareType i.e. Normal or Special
 *                If the PU is Special
 *                    Find the category of Special PU.
 *                    Find if the COP is in the Gulf Region.
 *
 *   @param  PricingUnit& - Pricing Unit
 *   @param  Itin&        - Itinerary
 *   @return bool         - true if PU is prequalified for COP
 */

bool
COPMinimumFare::qualifyPu(const PricingUnit& pu)
{
  // Proceed only if the PU geoTravelType is International.
  if (pu.geoTravelType() != GeoTravelType::International && pu.geoTravelType() != GeoTravelType::Transborder)
  {
    if (_diagEnabled)
      (_diag) << "DOMESTIC PU.\n";

    return false;
  }

  // Proceed only if the PU is RT/CT or RT/CT including side-trip.
  if ((pu.puType() != PricingUnit::Type::ROUNDTRIP) && (pu.puType() != PricingUnit::Type::CIRCLETRIP))
  {
    if (_diagEnabled)
      (_diag) << "PU IS NOT RT/CT.\n";

    return false;
  }

  // Determine if the PU contains travel to or via any point in the
  // same country or country group where the payment is made.
  if (!isViaCOPNation(pu.travelSeg(), VIA))
  {
    if (_diagEnabled)
      (_diag) << "PU IS NOT VIA COP COUNTRY - " << _saleCountry << ".\n";

    return false;
  }

  // Check if this is a Normal PU.
  switch (pu.puFareType())
  {
  case PricingUnit::NL:
    return true;

  case PricingUnit::SP: // Need to seperate from GRP/PRO type
    // determine if the country of payment
    // (involves travel to or via) is in the Gulf Region.
    if (!isGulfRegion(_saleCountry))
    {
      if (_diagEnabled)
        (_diag) << "PU IS SPCL BUT COP NATION IS NOT IN GULF REGION.\n";

      return false;
    }
    else
      return true;

  default:
    if (_diagEnabled)
      (_diag) << "UNKNOWN PU TYPE.\n";
    return false;
  }
}

void
COPMinimumFare::displayPu(const PricingUnit& pu)
{
  if (diagEnabled(Diagnostic702) || _trx.diagnostic().diagParamMapItem("DD") == "PU")
  {
    _diag << "PU: " << ((pu.puFareType() == PricingUnit::NL) ? "NORMAL" : "SPECIAL")
          << (pu.isSideTripPU() ? " SIDETRIP PU" : "") << "\n";

    const std::vector<TravelSeg*>& tvlSegs = pu.travelSeg();
    std::vector<TravelSeg*>::const_iterator i = tvlSegs.begin();
    for (; i != tvlSegs.end(); ++i)
    {
      AirSeg* curTvlSeg = dynamic_cast<AirSeg*>(*i);
      if (curTvlSeg == nullptr)
        continue;

      // Display the Travel Points
      _diag << "   " << curTvlSeg->boardMultiCity() << "-" << curTvlSeg->carrier() << "-"
            << curTvlSeg->offMultiCity() << "\n";
    }
    _diag << "\n";
  }
}

bool
COPMinimumFare::isViaCOPNation(const std::vector<TravelSeg*>& tvlSegs, TravelAppl tvlAppl) const
{
  std::vector<TravelSeg*>::const_iterator i = tvlSegs.begin();
  for (; i != tvlSegs.end(); ++i)
  {
    AirSeg* curTvlSeg = dynamic_cast<AirSeg*>(*i);
    if (curTvlSeg == nullptr)
      continue;

    const Loc* loc = curTvlSeg->origin();
    if ((loc != nullptr) && isCOPLoc(*loc))
      return true;

    if (LIKELY(tvlAppl == VIA))
    {
      std::vector<const Loc*>& hiddenStops = curTvlSeg->hiddenStops();
      std::vector<const Loc*>::iterator stopIter = hiddenStops.begin();
      for (; stopIter != hiddenStops.end(); ++stopIter)
      {
        if (isCOPLoc(**stopIter))
          return true;
      }
    }

    loc = curTvlSeg->destination();
    if ((loc != nullptr) && isCOPLoc(*loc))
      return true;
  }

  return false;
}

bool
COPMinimumFare::isFrenchNationGroup(const NationCode& nation) const
{
  const CopParticipatingNation* copPtNation =
      _trx.dataHandle().getCopParticipatingNation(nation, NATION_FRANCE, _travelDate);
  if (copPtNation)
  {
    return true;
  }

  return false;
}

bool
COPMinimumFare::isUSSRNationGroup(const NationCode& nation) const
{
  const CopParticipatingNation* copPtNation =
      _trx.dataHandle().getCopParticipatingNation(nation, NATION_USSR, _travelDate);
  if (copPtNation)
  {
    return true;
  }

  return false;
}

bool
COPMinimumFare::isGulfRegion(const NationCode& nation) const
{
  if (nation == NATION_BAHARAIN || nation == NATION_KUWAIT || nation == NATION_QUTAR ||
      nation == NATION_SAUDI_ARABIA || nation == NATION_UNITED_ARAB_EMIRATES)
    return true;
  else
    return false;
}

bool
COPMinimumFare::isCOPLoc(const Loc& loc) const
{
  return isCOPLoc(loc.nation(), _saleCountry);
}

bool
COPMinimumFare::isCOPLoc(const NationCode& country, const NationCode& saleCountry) const
{
  return ((country == saleCountry) ||
          (isFrenchNationGroup(country) && isFrenchNationGroup(saleCountry)) ||
          (isUSSRNationGroup(country) && isUSSRNationGroup(saleCountry)));
}

/**
 *   @method matchPu
 *
 *   Description: Match the PU's fields against the the corresponding fields
 *                in the COP Table rows and if a match is found, process
 *                the matching item.
 *
 *   @param  PricingTrx&  - Transaction object
 *   @param  PricingUnit& - Pricing Unit
 *   @param  Itin&        - Itinerary
 *   @return bool         - true if COP is applicable
 */

bool
COPMinimumFare::matchPu(const PricingUnit& pu)
{
  // Get COP items for the sale country
  const std::vector<CopMinimum*> copItems = getCopInfo(_saleCountry);

  // Whether we should loop match against the participating carrier
  bool matchPartCxr = false;

  // Collect the participating carrier from the pu
  std::set<CarrierCode> participatingCxrs;
  const std::vector<TravelSeg*>& tvlSegs = pu.travelSeg();
  for (const auto elem : tvlSegs)
  {
    AirSeg* airSeg = elem->toAirSeg();
    if (airSeg == nullptr)
      continue;

    const CarrierCode& carrier = airSeg->carrier();
    participatingCxrs.insert(carrier);
  }

  // First match against the ticketing carrier
  bool ret = matchTktCopItems(copItems, pu, _tktgCxr, participatingCxrs, matchPartCxr);

  // Match against the participating carrier if required.
  if (matchPartCxr)
  {
    for (std::set<CarrierCode>::iterator i = participatingCxrs.begin(),
                                         iend = participatingCxrs.end();
         !ret && i != iend;
         ++i)
    {
      ret = matchPrtCopItems(copItems, pu, *i);
    }
  }

  if (!ret && _diagEnabled)
    (_diag) << "NO MATCHING COP MINIMUM ITEM.\n";

  return ret;
}

const CopCarrier*
COPMinimumFare::checkCopCxr(const CopMinimum& cop, const CarrierCode& carrier, bool isTktCarrier)
{
  for (const auto elem : cop.cxrs())
  {
    // For tkt carrier, we will not match again blank cop carrier, however,
    // blank cop carrier will match against any participating carrier.
    if (isTktCarrier)
    {
      if (carrier == elem->copCarrier())
        return elem;
    }
    else
    {
      if (elem->copCarrier().empty() || elem->copCarrier() == " " || carrier == elem->copCarrier())
      {
        return elem;
      }
    }
  }

  return nullptr;
}

void
COPMinimumFare::displayCOPHeader()
{
  if (diagEnabled(Diagnostic702))
  {
    _diag << "\n"
          << "COP TVL TKTG COP PRPT PU FT  PU  PU     PU     CON    APL APPL\n"
          << "LOC APP CXR  CXR IND  TYP    TRP ORG    WI     PT         SAME\n"
          << "        EXPT                 TYP LOC    LOC               FT  \n";
  }
}

void
COPMinimumFare::displayCOPItem(CopMinimum& cop)
{
  if (cop.tktgCxrExcpts().size() > 0)
  {
    displayCOPItem(cop, 0);
  }
  else
  {
    for (int i = 0, n = cop.cxrs().size(); i < n; i++)
    {
      if (i == 0)
      {
        displayCOPItem(cop, i);
      }
      else
      {
        _diag << "             " << std::setw(4) << cop.cxrs()[i]->copCarrier() << std::setw(5)
              << cop.cxrs()[i]->participationInd() << '\n';
      }
    }
  }
}

void
COPMinimumFare::displayCOPItem(CopMinimum& cop, int copCxrIdx)
{
  if (_diagEnabled)
  {
    _diag.setf(std::ios::left, std::ios::adjustfield);
    _diag << std::setw(4) << cop.copNation() << std::setw(4) << cop.travelAppl();

    const std::vector<CarrierCode>& tkgtCarriers = cop.tktgCxrExcpts();
    if (tkgtCarriers.size() < 1)
    {
      _diag << "     ";
    }
    else
    {
      // If Ticketing Carrier Exception is specified, the rest of fields
      // should be empty.
      _diag << tkgtCarriers.front();
      std::vector<CarrierCode>::const_iterator i = tkgtCarriers.begin() + 1;
      for (; i != tkgtCarriers.end(); ++i)
        _diag << "," << *i;
      _diag << "\n";
      return;
    }

    _diag << std::setw(4) << cop.cxrs()[copCxrIdx]->copCarrier() << std::setw(5)
          << cop.cxrs()[copCxrIdx]->participationInd() << std::setw(3) << cop.puNormalSpecialType()
          << std::setw(4) << cop.fareType() << std::setw(4) << cop.puTripType() << std::setw(2)
          << cop.puOrigLoc().locType() << std::setw(5) << cop.puOrigLoc().loc() << std::setw(2)
          << cop.puWithinLoc().locType() << std::setw(5) << cop.puWithinLoc().loc() << std::setw(2)
          << cop.constPointLoc().locType() << std::setw(5) << cop.constPointLoc().loc()
          << std::setw(2) << cop.puAppl() << std::setw(2) << cop.fareTypeAppl() << "\n";
  }
}

//----------------------------------------------------------------------------
// calculateBaseFare()
//---------------------------------------------------------------------------
MoneyAmount
COPMinimumFare::calculateBaseFare(const PricingUnit& pu,
                                  MinFarePlusUpItem& minFarePlusUp,
                                  bool mixedCabin,
                                  const CabinType& lowestCabin)
{
  minFarePlusUp.plusUpAmount = 0.0;
  minFarePlusUp.baseAmount = 0.0;
  minFarePlusUp.boardPoint = "";
  minFarePlusUp.offPoint = "";
  minFarePlusUp.constructPoint = "";

  const std::vector<FareUsage*>& fus = pu.fareUsage();

  if (mixedCabin && diagEnabled(Diagnostic765))
  {
    _diag << "REPRICED FARE:\n";
  }

  for (const auto fu : fus)
  {
    if (!fu)
      continue;

    const PaxTypeFare* ptFare = fu->paxTypeFare();
    CabinType cabin(ptFare->cabin());

    if (mixedCabin && cabin < lowestCabin)
    {
      minFarePlusUp.baseAmount += repriceFare(pu, *fu, lowestCabin);
    }
    else
    {
      if (mixedCabin && diagEnabled(Diagnostic765))
      {
        printPaxTypeFare(*ptFare, _diag, COP, "* ");
      }
      minFarePlusUp.baseAmount += accumulateFareAmount(*fu);
    }
  }
  minFarePlusUp.baseAmount += pu.minFarePlusUp().getSum(CTM);

  return minFarePlusUp.baseAmount;
}

//----------------------------------------------------------------------------
// Check table application/exclusion
//---------------------------------------------------------------------------
bool
COPMinimumFare::checkExclusionByTableEntry(const PricingUnit& pu)
{
  for (const auto elem : pu.fareUsage())
  {

    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    {
      _travelDate = _farePath.itin()->travelDate();
      adjustRexPricingDates(_trx, elem->paxTypeFare()->retrievalDate());
    }

    MatchRuleLevelExclTable matchRuleLevelExcl(
        COP, _trx, *(_farePath.itin()), *elem->paxTypeFare(), _travelDate);

    if (matchRuleLevelExcl())
    {
      _matchedRuleItem = matchRuleLevelExcl.matchedRuleItem();
      if (_matchedRuleItem->copMinFareAppl() == YES)
      {
        if (_diagEnabled)
        {
          printException(*elem->paxTypeFare(), _diag);
          (_diag) << " EXEMPT BY RULE LEVEL EXCL. TABLE - " << _matchedRuleItem->seqNo() << "\n";
        }

        if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
          restorePricingDates(_trx, _trx.ticketingDate());

        return true;
      }
    }
  }
  return false;
}

//----------------------------------------------------------------------------
// calculateFare()
//----------------------------------------------------------------------------
bool
COPMinimumFare::calculateFare(PricingUnit& pu)
{
  if (checkExclusionByTableEntry(pu))
  {
    return false; // COP check does not apply
  }

  // Print out the fares before proceeding to repricing
  if (diagEnabled(Diagnostic765))
  {
    _diag << "\nCOUNTRY OF PAYMENT CHECK\n"
             "CITY      GOV    CLASS                       DIR FARE  GLOB EXCL\n"
             "PAIR      CXR    CUR          AMOUNT RTG TAG I/O TYPE  IND  IND \n";

    for (std::vector<FareUsage*>::const_iterator i = pu.fareUsage().begin(),
                                                 end = pu.fareUsage().end();
         i != end;
         ++i)
    {
      printPaxTypeFare(*(*i)->paxTypeFare(), _diag, COP, "* ");
    }
  }

  // Determine if mixed cabin
  CabinType lowestCabin;
  lowestCabin.setUnknownClass();
  bool mixedCabin = MinFareLogic::isMixedCabin(pu, lowestCabin);

  // Init the plus up info and accummulate the base fare
  MinFarePlusUpItem* copPlusUp = nullptr;
  _trx.dataHandle().get(copPlusUp);
  if (!copPlusUp)
  {
    LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (copPlusUp)");

    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      restorePricingDates(_trx, _trx.ticketingDate());

    return false;
  }

  if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    _travelDate = _farePath.itin()->travelDate();
    adjustRexPricingDates(_trx, pu.fareUsage().front()->paxTypeFare()->retrievalDate());
    _dataHandle.setTicketDate(pu.fareUsage().front()->paxTypeFare()->retrievalDate());
  }

  // lint --e{413}
  calculateBaseFare(pu, *copPlusUp, mixedCabin, lowestCabin);

  calculateFarePlusUp(pu, *copPlusUp, mixedCabin, lowestCabin);

  if (diagEnabled(Diagnostic765))
    _diag << "  FARE NUC " << copPlusUp->baseAmount << "        " << _copBoardPoint << "-"
          << _copOffPoint << " NUC " << _copFare << " P-" << copPlusUp->plusUpAmount << "\n";

  // Save the plus up to PU
  if (copPlusUp->plusUpAmount > 0.0)
  {
    LOG4CXX_DEBUG(logger, "Save PlusUp for PU: " << copPlusUp->plusUpAmount);
    pu.minFarePlusUp().addItem(COP, copPlusUp);
  }

  if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    restorePricingDates(_trx, _trx.ticketingDate());

  return true;
}

void
COPMinimumFare::calculateFarePlusUp(const PricingUnit& pu,
                                    MinFarePlusUpItem& minFarePlusUp,
                                    bool mixedCabin,
                                    const CabinType& lowestCabin)
{
  std::vector<std::vector<TravelSeg*>> copTvlSegs;
  if (!getTvlSegFromCopPoint(pu, copTvlSegs))
  {
    if (diagEnabled(Diagnostic765))
    {
      (_diag) << " \n"
              << "*** SELECT FARE FOR COP FAILED - REPRICE TRAVEL SEG\n";
    }
    return;
  }

  for (auto & tvlSeg : copTvlSegs)
  {
    selectFare(pu, tvlSeg, minFarePlusUp, mixedCabin, lowestCabin);
  }
}

bool
COPMinimumFare::selectFare(const PricingUnit& pu,
                           std::vector<TravelSeg*>& tvlSeg,
                           MinFarePlusUpItem& minFarePlusUp,
                           bool mixedCabin,
                           const CabinType& lowestCabin)
{
  for (std::vector<TravelSeg*>::const_iterator ai = tvlSeg.begin(), zi = ai + 1, end = tvlSeg.end();
       zi != end;
       ++zi)
  {
    // Determin the COP thru fare, needed for fare selection
    const FareUsage* copFareComp = getCopFareComponent(pu, (*ai)->origin());
    if (!copFareComp)
    {
      return false;
    }
    const PaxTypeFare* thruFare = copFareComp->paxTypeFare();

    // Outbound Direction
    std::vector<TravelSeg*> obTvlSeg(ai, zi);
    const PaxTypeFare* obFare = repriceAndSelectFare(
        pu, thruFare, obTvlSeg, MinFareFareSelection::OUTBOUND, mixedCabin, lowestCabin);

    // Inbound Direction
    std::vector<TravelSeg*> ibTvlSeg(zi, end);
    const PaxTypeFare* ibFare = repriceAndSelectFare(
        pu, thruFare, ibTvlSeg, MinFareFareSelection::INBOUND, mixedCabin, lowestCabin);

    calculateFarePlusUp(minFarePlusUp,
                        pu,
                        obFare,
                        ibFare,
                        (*ai)->boardMultiCity(),
                        (*zi)->boardMultiCity(),
                        mixedCabin,
                        lowestCabin);
  }

  return true;
}

RepricingTrx*
COPMinimumFare::getRepricingTrx(std::vector<TravelSeg*>& tvlSeg)
{
  GlobalDirection gDir = getGlobalDirection(_trx, tvlSeg, _travelDate);
  CarrierCode cxr = getGoverningCarrier(tvlSeg);

  RepricingTrx* rpTrx = nullptr;

  if (gDir != GlobalDirection::ZZ)
  {
    try
    {
      rpTrx = TrxUtil::reprice(
          _trx, tvlSeg, FMDirection::UNKNOWN, false, (cxr.empty() ? nullptr : &cxr), &gDir);
    }
    catch (const ErrorResponseException& ex)
    {
      LOG4CXX_ERROR(logger,
                    "Exception during repricing for ["
                        << tvlSeg.front()->boardMultiCity().data() << " - "
                        << tvlSeg.back()->offMultiCity().data() << " " << ex.code() << " - "
                        << ex.message());
    }
    catch (...)
    {
      LOG4CXX_ERROR(logger,
                    "Unknown exception during repricing for ["
                        << tvlSeg.front()->boardMultiCity().data() << " - "
                        << tvlSeg.back()->offMultiCity().data());
    }
  }

  return rpTrx;
}

const PaxTypeFare*
COPMinimumFare::repriceAndSelectFare(const PricingUnit& pu,
                                     const PaxTypeFare* thruFare,
                                     std::vector<TravelSeg*>& tvlSeg,
                                     MinFareFareSelection::FareDirectionChoice fareDirection,
                                     bool mixedCabin,
                                     const CabinType& lowestCabin)
{
  GlobalDirection gDir = getGlobalDirection(_trx, tvlSeg, _travelDate);

  const PaxTypeFare* selFare = nullptr;

  RepricingTrx* rpTrx = nullptr;
  if (gDir != GlobalDirection::ZZ)
  {
    rpTrx = getRepricingTrx(tvlSeg);
  }

  bool selectNormalFare = pu.puFareType() == PricingUnit::NL ? true : false;

  // For Inbound, if repriced failed, do not call select fare as there is no
  // fare to select.
  // if (fareDirection == MinFareFareSelection::INBOUND && !rpTrx)
  //  return selFare;

  selFare = MinFareLogic::selectQualifyFare(COP,
                                            _trx,
                                            *(_farePath.itin()),
                                            *thruFare,
                                            *_farePath.paxType(),
                                            lowestCabin,
                                            selectNormalFare,
                                            fareDirection,
                                            MinFareLogic::eligibleFare(pu),
                                            tvlSeg,
                                            _travelDate,
                                            nullptr,
                                            nullptr,
                                            rpTrx,
                                            &_farePath,
                                            &pu);

  if (selFare)
  {
    LOG4CXX_DEBUG(logger,
                  "Fare Selected [" << tvlSeg.front()->boardMultiCity().data() << " - "
                                    << tvlSeg.back()->offMultiCity().data()
                                    << "] NUC: " << selFare->nucFareAmount());

    if (diagEnabled(Diagnostic765))
    {
      if (rpTrx)
        printPaxTypeFare(*selFare, _diag, COP, "- ");
      else
        printPaxTypeFare(*selFare, _diag, COP, "  ");
    }
  }
  else
  {
    if (diagEnabled(Diagnostic765))
      _diag << "  NO FARE FOUND " << tvlSeg.front()->boardMultiCity() << "-"
            << tvlSeg.back()->offMultiCity() << "                      "
            << (fareDirection == MinFareFareSelection::OUTBOUND ? "O" : "I") << "\n";
  }

  return selFare;
}

GlobalDirection
COPMinimumFare::getGlobalDirection(PricingTrx& trx,
                                   const std::vector<TravelSeg*>& tvlSegs,
                                   const DateTime& travelDate) const
{
  GlobalDirection dir;
  if (GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, travelDate, tvlSegs, dir))
    return dir;

  return GlobalDirection::ZZ;
}

CarrierCode
COPMinimumFare::getGoverningCarrier(const std::vector<TravelSeg*>& tvlSegs) const
{
  GoverningCarrier govCxr(&_trx);
  std::set<CarrierCode> govCxrSet;
  govCxr.getGoverningCarrier(tvlSegs, govCxrSet);
  if (!govCxrSet.empty())
  {
    return *govCxrSet.begin();
  }
  return "";
}

/**
 * Retrieve the fare component that has the given loc as origin
 **/
const FareUsage*
COPMinimumFare::getCopFareComponent(const PricingUnit& pu, const Loc* origin) const
{
  const std::vector<FareUsage*>& fus = pu.fareUsage();
  for (const auto fu : fus)
  {
    const std::vector<TravelSeg*>& tvlSeg = fu->travelSeg();
    if (std::find_if(tvlSeg.begin(), tvlSeg.end(), TravelSegUtil::OrigEqual(origin)) !=
        tvlSeg.end())
    {
      return fu;
    }
  }

  return nullptr;
}

MoneyAmount
COPMinimumFare::accumulateFareAmount(const FareUsage& fu) const
{
  if (fu.paxTypeFare() != nullptr)
  {
    MoneyAmount accFareAmount = fu.paxTypeFare()->nucFareAmount() + fu.minFarePlusUp().getSum(HIP) +
                                fu.paxTypeFare()->mileageSurchargeAmt();

    return accFareAmount;
  }

  return 0.0;
}

void
COPMinimumFare::calculateFarePlusUp(MinFarePlusUpItem& minFarePlusUp,
                                    const PricingUnit& pu,
                                    const PaxTypeFare* obFare,
                                    const PaxTypeFare* ibFare,
                                    const LocCode& copPoint,
                                    const LocCode& endPoint,
                                    bool mixedCabin,
                                    const CabinType& lowestCabin)
{
  MoneyAmount obFareAmount = 0.0;
  MoneyAmount ibFareAmount = 0.0;

  if (obFare)
  {
    // Quan, need some change here
    // temp create a FareUsage here to be able to call accumulateFareAmount
    FareUsage tmpFu;
    tmpFu.paxTypeFare() = const_cast<PaxTypeFare*>(obFare);

    obFareAmount = accumulateFareAmount(tmpFu) + pu.minFarePlusUp().getSum(CTM);
  }

  if (ibFare)
  {
    // Quan, need some change here
    // temp create a FareUsage here to be able to call accumulateFareAmount
    FareUsage tmpFu;
    tmpFu.paxTypeFare() = const_cast<PaxTypeFare*>(ibFare);

    ibFareAmount = accumulateFareAmount(tmpFu) + pu.minFarePlusUp().getSum(CTM);
  }

  MoneyAmount fareAmount;
  if (obFareAmount > 0 && ibFareAmount > 0)
    fareAmount = std::min(obFareAmount, ibFareAmount) * 2;
  else
    fareAmount = std::max(obFareAmount, ibFareAmount) * 2;

  MoneyAmount plusUp = fareAmount - minFarePlusUp.baseAmount;
  if (plusUp > minFarePlusUp.plusUpAmount)
  {
    minFarePlusUp.plusUpAmount = plusUp;
    minFarePlusUp.boardPoint = copPoint;
    minFarePlusUp.offPoint = endPoint;
  }

  if (fareAmount > _copFare)
  {
    _copFare = fareAmount;
    _copBoardPoint = copPoint;
    _copOffPoint = endPoint;
  }

  LOG4CXX_DEBUG(logger,
                "COP::calculateFarePlusUp"
                    << "\nbaseFare = " << minFarePlusUp.baseAmount
                    << "\nobFareAmount = " << obFareAmount << ", ibFareAmount = " << ibFareAmount
                    << "\nfareAmount = " << fareAmount
                    << ", plusUp = " << minFarePlusUp.plusUpAmount);
}

bool
COPMinimumFare::getTvlSegFromCopPoint(const PricingUnit& pu,
                                      std::vector<std::vector<TravelSeg*>>& copTvlSegs) const
{
  copTvlSegs.clear();
  const std::vector<TravelSeg*>& tvlSeg = pu.travelSeg();

  // Circle trip provision: if the CT provision airport is at the end of the pu,
  // append an Arunk segment.
  ArunkSeg* aSeg = nullptr;
  if (tvlSeg.front()->origin() != tvlSeg.back()->destination())
  {
    _dataHandle.get(aSeg);
    if (aSeg)
    {
      aSeg->origin() = tvlSeg.back()->destination();
      aSeg->origAirport() = tvlSeg.back()->destAirport();
      aSeg->destination() = tvlSeg.front()->origin();
      aSeg->destAirport() = tvlSeg.front()->origAirport();
    }
  }

  for (std::vector<TravelSeg*>::const_iterator i = tvlSeg.begin(), end = tvlSeg.end(); i != end;
       ++i)
  {
    if (isCOPLoc(*(*i)->origin()))
    {
      std::vector<TravelSeg*> repriceTvlSeg;
      std::copy(i, tvlSeg.end(), std::back_inserter(repriceTvlSeg));
      if (aSeg)
      {
        repriceTvlSeg.push_back(aSeg);
      }
      std::copy(tvlSeg.begin(), i, std::back_inserter(repriceTvlSeg));
      copTvlSegs.push_back(repriceTvlSeg);
    }
  }

  return (copTvlSegs.size() > 0);
}

bool
COPMinimumFare::diagEnabled(DiagnosticTypes diagType) const
{
  return (_diagEnabled && _diagType == diagType);
}

const std::vector<CopMinimum*>&
COPMinimumFare::getCopInfo(const NationCode& saleLoc)
{
  LOG4CXX_DEBUG(logger, "getCopInfo - saleLoc: " << saleLoc);

  NationCode copLoc = saleLoc;
  if (isFrenchNationGroup(saleLoc))
  {
    copLoc = NATION_FRANCE;
  }
  else if (isUSSRNationGroup(saleLoc))
  {
    copLoc = NATION_USSR;
  }

  const std::vector<CopMinimum*>& copLst = _dataHandle.getCopMinimum(copLoc, _travelDate);

  if (diagEnabled(Diagnostic702))
  {
    if (copLst.size() > 0)
    {
      _diag << " \n";
      displayCOPHeader();

      for (const auto elem : copLst)
      {
        //_diag << "- ";
        displayCOPItem(*elem);
      }
    }
    else
    {
      _diag << "NO MATCHING COP MINIMUM ITEM: " << saleLoc << "\n";
    }
  }

  return copLst;
}

MoneyAmount
COPMinimumFare::repriceFare(const PricingUnit& pu,
                            const FareUsage& fu,
                            const CabinType& lowestCabin)
{
  const PaxTypeFare* paxTypeFare = nullptr;

  FareUsage* repriceFu = nullptr;

  paxTypeFare = MinFareLogic::getRepricedNormalFare(_trx,
                                                    _farePath,
                                                    pu,
                                                    fu,
                                                    repriceFu,
                                                    MinFareFareSelection::HALF_ROUND_TRIP,
                                                    lowestCabin,
                                                    _travelDate,
                                                    true);

  if (paxTypeFare)
  {
    paxTypeFare = MinFareLogic::selectQualifyFare(COP,
                                                  _trx,
                                                  *_farePath.itin(),
                                                  *paxTypeFare,
                                                  *_farePath.paxType(),
                                                  lowestCabin,
                                                  paxTypeFare->isNormal(),
                                                  MinFareLogic::fareDirection(*repriceFu),
                                                  MinFareLogic::eligibleFare(pu),
                                                  repriceFu->travelSeg(),
                                                  _travelDate,
                                                  nullptr,
                                                  nullptr,
                                                  nullptr,
                                                  &_farePath,
                                                  &pu);

    if (paxTypeFare && diagEnabled(Diagnostic765))
    {
      printPaxTypeFare(*paxTypeFare, _diag, COP, "* ");
    }
  }

  if (paxTypeFare && repriceFu)
  {
    return (paxTypeFare->nucFareAmount() + paxTypeFare->mileageSurchargeAmt() +
            repriceFu->minFarePlusUp().getSum(HIP));
  }
  else
  {
    return accumulateFareAmount(fu);
  }
}

/**
 * This information is provide by Darrin May 19, 2005
 *
 * Addendum to COP item matching:
 *
 * (Lek's) Blank COP Carrier means ANY carrier. The intention of this
 * item is no matter who is the ticketing or participating carrier, if it
 * is paid in SA, and travel is to/via SA, COP will apply.
 *
 * The problem is that when it is coded with Blank COP carrier, the
 * coding convention requires it to be coded as PARTICIPATION only, but
 * that happened after the requirement was written and it had never in my
 * mind that I need to go back to change the part of requirements.
 *
 * (Darrin) So, if I say, in the ticketing carrier loop, if no match
 * is found, is it also the participating carrier in the PU?
 *    if yes, can it match to any participating carrier item?
 *        If yes, then check whether Origin/TWW/FC is coded or not;
 *            if coded, APPLY or EXEMPT based on APPLY is coded (Y to apply
 *                COP, N to NOT apply COP),
 *            if Origin/TWW/FC is not coded, APPLY COP
 *        if no, go to step 10 to match any participating carrier that is
 *            differ than the ticketing carrier and try to match to COP
 *            carrier with E or P coded (Blank match to all carriers)
 *    If no, go to step 10 to match any participating carrier that is differ
 *        than the ticketing carrier and try to match to COP carrier with E
 *        or P coded (Blank match to all carriers)
 *
 **/

bool
COPMinimumFare::matchTktCopItems(const std::vector<CopMinimum*>& copItems,
                                 const PricingUnit& pu,
                                 const CarrierCode& carrier,
                                 const std::set<CarrierCode>& participatingCxrs,
                                 bool& matchPartCxr)
{
  const CopCarrier* copCxr = nullptr;

  if (diagEnabled(Diagnostic702))
  {
    _diag << "TICKETING CARRIER LOOP:\n";
  }

  for (const auto copItem : copItems)
  {
    CopMinimum& cop = *copItem;

    // Step 3:
    if (checkTktCxrException(carrier, cop))
    {
      if (diagEnabled(Diagnostic702))
      {
        //_diag << "* ";
        displayCOPItem(cop);
        _diag << "TICKETING CARRIER EXCEPTION: " << carrier << "\n";
      }
      matchPartCxr = false;
      return false;
    }
    else if (cop.tktgCxrExcpts().size() > 0)
    {
      continue;
    }

    // Step 4:
    copCxr = checkCopCxr(cop, carrier);
    if (!copCxr)
    {
      if (diagEnabled(Diagnostic702))
      {
        //_diag << "F ";
        displayCOPItem(cop);
        _diag << "TKT CARRIER DOES NOT MATCH COP CARRIER.\n";
      }
      continue;
    }

    if (copCxr && carrier == copCxr->copCarrier())
    {
      if (copCxr->participationInd() != ' ' && copCxr->participationInd() != TKTG_CARRIER &&
          copCxr->participationInd() != EITHER_CARRIER &&
          copCxr->participationInd() != BOTH_CARRIER)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PART. IND. MUST BE T, E, OR B.\n";
        }
        continue;
      }
    }

    switch (copCxr->participationInd())
    {
    case TKTG_CARRIER:
    case EITHER_CARRIER:
      break;
    case BOTH_CARRIER:
      if (participatingCxrs.find(carrier) == participatingCxrs.end())
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "TKT CARRIER MUST ALSO BE PARTICIPATING CARRIER.\n";
        }
        continue;
      }
      break;
    }

    // Step 5: check PU type
    switch (cop.puNormalSpecialType())
    {
    case NORMAL:
      if (pu.puFareType() != PricingUnit::NL)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PU MUST BE NORMAL PU.\n";
        }
        continue;
      }
      break;

    case SPECIAL:
      if (pu.puFareType() != PricingUnit::SP)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PU MUST BE SPECIAL PU.\n";
        }
        continue;
      }
      break;
    }

    // Check Fare Type/Fare Type Generics
    // No carrier is using the field yet. More discussion needed.
    // Implement later

    // Check PU trip type
    switch (cop.puTripType())
    {
    case RT: // Must be RT
      if (pu.puType() != PricingUnit::Type::ROUNDTRIP)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PU MUST BE RT.\n";
        }
        continue;
      }
      break;

    case CT: // Must be CT
      if (pu.puType() != PricingUnit::Type::CIRCLETRIP)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PU MUST BE CT.\n";
        }
        continue;
      }
      break;
    }

    bool puOriginData = false;
    bool puWhollyWithinData = false;

    // Step 6: Check PU origin
    if ((cop.puOrigLoc().locType() != ' ') && !cop.puOrigLoc().loc().empty())
    {
      const Loc* origin = originOfPricingUnit(pu);
      if ((origin == nullptr) || (!LocUtil::isInLoc(*origin,
                                                    cop.puOrigLoc().locType(),
                                                    cop.puOrigLoc().loc(),
                                                    Vendor::SABRE,
                                                    MANUAL,
                                                    LocUtil::OTHER,
                                                    GeoTravelType::International,
                                                    EMPTY_STRING(),
                                                    _trx.getRequest()->ticketingDT())))
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "FAILED PU ORIGIN CHECK.\n";
        }
        continue;
      }
      puOriginData = true;
    }

    // Step 7: Check PU Wholly Within, and Fare Construct Point
    if ((cop.puWithinLoc().locType() != ' ' && !cop.puWithinLoc().loc().empty()) &&
        (cop.constPointLoc().locType() != ' ' && !cop.constPointLoc().loc().empty()))
    {
      if (!isWithinLoc(_trx, pu.travelSeg(), cop.puWithinLoc().locType(), cop.puWithinLoc().loc()))
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "FAILED PU WHOLLY WITHIN CHECK.\n";
        }
        continue;
      }

      if (!isWithinLoc(_trx,
                       pu.travelSeg(),
                       cop.constPointLoc().locType(),
                       cop.constPointLoc().loc(),
                       false))
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "FAILED PU FARE CONSTRUCT POINT CHECK.\n";
        }
        continue;
      }
      puWhollyWithinData = true;
    }
    else
    {
      if (puOriginData)
      {
        // Step 9:
        if (cop.puAppl() == NOT_APPLY)
        {
          if (diagEnabled(Diagnostic702))
          {
            //_diag << "F ";
            displayCOPItem(cop);
            _diag << "PU APPL - COP DO NOT APPLY.\n";
          }
          return false;
        }
        else
        {
          // check fare type appl.
        }
      }
    }

    // Step 8:
    if (puOriginData && puWhollyWithinData)
    {
      if (cop.puAppl() == APPLY)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "* ";
          displayCOPItem(cop);
        }
        return true;
      }
      else if (cop.puAppl() == NOT_APPLY)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PU APPL - COP DO NOT APPLY.\n";
        }
        return false;
      }
    }

    // Step 9:
    if (puOriginData || puWhollyWithinData)
    {
      if (cop.puAppl() == NOT_APPLY)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PU APPL - COP DO NOT APPLY.\n";
        }
        return false;
      }
      else
      {
        // check fare type appl.
      }
    }

    // if we get thru to here - then we have a match cop item.
    if (diagEnabled(Diagnostic702))
    {
      //_diag << "* ";
      displayCOPItem(cop);
    }
    return true;
  }

  if (!copCxr)
  {
    matchPartCxr = true;
  }

  return false;
}

bool
COPMinimumFare::matchPrtCopItems(const std::vector<CopMinimum*>& copItems,
                                 const PricingUnit& pu,
                                 const CarrierCode& carrier)
{
  const CopCarrier* copCxr = nullptr;

  if (diagEnabled(Diagnostic702))
  {
    _diag << "PARTICIPATING CARRIER LOOP:\n";
  }

  for (const auto copItem : copItems)
  {
    CopMinimum& cop = *copItem;

    // Step 10:

    // Step 11: if item has tkt carrier exception encoded, skip to next item.
    if (cop.tktgCxrExcpts().size() > 0)
    {
      continue;
    }

    // Step 12:
    copCxr = checkCopCxr(cop, carrier, false);
    if (!copCxr)
    {
      if (diagEnabled(Diagnostic702))
      {
        //_diag << "F ";
        displayCOPItem(cop);
        _diag << "PART. CXR DOES NOT MATCH COP CXR.\n";
      }
      continue;
    }

    if (copCxr)
    {
      if (copCxr->participationInd() != ' ' && copCxr->participationInd() != PARTICIPATE_CARRIER &&
          copCxr->participationInd() != EITHER_CARRIER)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "ONLY MATCH PART. CXR WITH P, OR E COP ITEM.\n";
        }
        continue;
      }
    }

    // Step 13: check PU type
    switch (cop.puNormalSpecialType())
    {
    case NORMAL:
      if (pu.puFareType() != PricingUnit::NL)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PU MUST BE NORMAL.\n";
        }
        continue;
      }
      break;

    case SPECIAL:
      if (pu.puFareType() != PricingUnit::SP)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PU MUST BE SPECIAL.\n";
        }
        continue;
      }
      break;
    }

    // Check Fare Type/Fare Type Generics
    // No carrier is using the field yet. More discussion needed.
    // Implement later

    // Check PU trip type
    switch (cop.puTripType())
    {
    case RT: // Must be RT
      if (pu.puType() != PricingUnit::Type::ROUNDTRIP)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PU MUST BE RT.\n";
        }
        continue;
      }
      break;

    case CT: // Must be CT
      if (pu.puType() != PricingUnit::Type::CIRCLETRIP)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "PU MUST BE CT.\n";
        }
        continue;
      }
      break;
    }

    bool puOriginData = false;
    bool puWhollyWithinData = false;

    // Step 14: Check PU origin
    if ((cop.puOrigLoc().locType() != ' ') && !cop.puOrigLoc().loc().empty())
    {
      const Loc* origin = originOfPricingUnit(pu);
      if ((origin == nullptr) || (!LocUtil::isInLoc(*origin,
                                                    cop.puOrigLoc().locType(),
                                                    cop.puOrigLoc().loc(),
                                                    Vendor::SABRE,
                                                    MANUAL,
                                                    LocUtil::OTHER,
                                                    GeoTravelType::International,
                                                    EMPTY_STRING(),
                                                    _trx.getRequest()->ticketingDT())))
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "FAILED PU ORIGIN CHECK.\n";
        }
        continue;
      }
      puOriginData = true;
    }

    // Step 15: Check PU Wholly Within, and Fare Construct Point
    if ((cop.puWithinLoc().locType() != ' ' && !cop.puWithinLoc().loc().empty()) &&
        (cop.constPointLoc().locType() != ' ' && !cop.constPointLoc().loc().empty()))
    {
      if (!isWithinLoc(_trx, pu.travelSeg(), cop.puWithinLoc().locType(), cop.puWithinLoc().loc()))
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "FAILED PU WHOLLY WITHIN CHECK.\n";
        }
        continue;
      }

      if (!isWithinLoc(_trx,
                       pu.travelSeg(),
                       cop.constPointLoc().locType(),
                       cop.constPointLoc().loc(),
                       false))
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "F ";
          displayCOPItem(cop);
          _diag << "FAILED PU FARE CONSTRUCT POINT CHECK.\n";
        }
        continue;
      }
      puWhollyWithinData = true;
    }
    else
    {
      if (puOriginData)
      {
        if (cop.puAppl() == NOT_APPLY)
        {
          if (diagEnabled(Diagnostic702))
          {
            //_diag << "F ";
            displayCOPItem(cop);
            _diag << "PU ORIGIN/PU WW: APPL - DO NOT APPLY COP.\n";
          }
          continue;
        }
        else
        {
          // check fare type appl.
          if (diagEnabled(Diagnostic702))
          {
            //_diag << "* ";
            displayCOPItem(cop);
          }
          return true;
        }
      }
    }

    // Step 16:
    if (puOriginData && puWhollyWithinData)
    {
      if (cop.puAppl() == APPLY)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "* ";
          displayCOPItem(cop);
        }
        return true;
      }
      else if (cop.puAppl() == NOT_APPLY)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "* ";
          displayCOPItem(cop);
          _diag << "PU ORIGIN + PU WW - APPL - DO NOT APPLY COP.\n";
        }
        return false;
      }
    }

    // Step 17:
    if (puOriginData || puWhollyWithinData)
    {
      if (cop.puAppl() == NOT_APPLY)
      {
        if (diagEnabled(Diagnostic702))
        {
          //_diag << "* ";
          displayCOPItem(cop);
          _diag << "PU ORIGIN / PU WW - APPL - DO NOT APPLY COP.\n";
        }
        return false;
      }
      else
      {
        // check fare type appl.

        if (diagEnabled(Diagnostic702))
        {
          //_diag << "* ";
          displayCOPItem(cop);
        }
        return true;
      }
    }

    // if we get thru to here - then we have a match cop item.
    if (diagEnabled(Diagnostic702))
    {
      //_diag << "* ";
      displayCOPItem(cop);
    }
    return true;
  }

  return false;
}

bool
COPMinimumFare::checkTktCxrException(const CarrierCode& tktCxr, const CopMinimum& cop)
{
  const std::vector<CarrierCode>& cxrExceptions = cop.tktgCxrExcpts();

  if (cxrExceptions.size() == 0)
    return false;

  if (std::find(cxrExceptions.begin(), cxrExceptions.end(), tktCxr) != cxrExceptions.end())
  {
    return true;
  }

  return false;
}
}
