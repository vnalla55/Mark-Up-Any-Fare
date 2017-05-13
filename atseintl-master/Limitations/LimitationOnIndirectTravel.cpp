//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------
#include "Limitations/LimitationOnIndirectTravel.h"

#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/Global.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/LimitationCmn.h"
#include "DBAccess/LimitationFare.h"
#include "DBAccess/LimitationJrny.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MileageSubstitution.h"
#include "DBAccess/MultiAirportCity.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/Nation.h"
#include "DBAccess/SurfaceSectorExemptionInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "Util/BranchPrediction.h"

#include <boost/thread/locks.hpp>

#include <map>
#include <sstream>

namespace tse
{

namespace
{
Logger logger("atseintl.Limitations.LimitationOnIndirectTravel");
}

FALLBACK_DECL(overusedMutexInLimitationOnIndirectTravel)

const std::string LimitationOnIndirectTravel::REPLACE_TOKEN = "XX";
const std::string LimitationOnIndirectTravel::CONFIRMED_STATUS = "OK";

const char LimitationOnIndirectTravel::BLANK = ' ';

namespace
{
struct isCrsEqual : std::unary_function<const std::string, bool>
{
  const std::string& crs;

  isCrsEqual(const std::string& ssei) : crs(ssei) {}

  bool operator()(const SurfaceSectorExemptionInfo* ssei)
  {
    return ssei->crs() == crs || ssei->crs().empty();
  }
};
}

void
LimitationOnIndirectTravel::validateJourney()
{
  const std::vector<TravelSeg*>& travelSegVec = _itin.travelSeg();
  bool internationalItin = true;
  DiagCollector* diag = get370();
  if (diag)
  {
    (*diag) << " \n**************** LIMITATIONS ON INDIRECT TRAVEL ***************\n"
            << "J\n";

    if (_trx.getOptions()->isRtw())
    {
      (*diag) << "RTW REQUEST IS NOT SUBJECT TO JOURNEY LIMITATIONS VALIDATION\n";
      end370(*diag);
    }
    else
      displayTravelSegs(travelSegVec, *diag, true);
  }

  if (_trx.getOptions()->isRtw())
  {
    LOG4CXX_INFO(logger, "RTW Limitation bypass validateJourney");
    return;
  }

  const GeoTravelType& geoTrvlType = _itin.geoTravelType();
  if (geoTrvlType == GeoTravelType::Domestic || geoTrvlType == GeoTravelType::ForeignDomestic)
    internationalItin = false;

  if (travelSegVec.empty() || !internationalItin)
  {
    if (diag != nullptr)
    {
      if (!internationalItin)
        (*diag) << "\n  ITIN NOT INTERNATIONAL NOT SUBJECT TO LIMITATIONS VALIDATION\n";
      diag->flushMsg();
    }

    return;
  }

  const std::vector<LimitationJrny*>& limitations =
      //_trx.dataHandle().getJLimitation(SABRE_USER, _travelCommenceDT); // Alex needs to match
      //empty user
      _trx.dataHandle().getJLimitation(_travelCommenceDT);

  std::vector<LimitationJrny*>::const_iterator iter = limitations.begin();
  for (; iter != limitations.end(); iter++)
  {
    try { validateJourneyItem(**iter, diag); }
    catch (ErrorResponseException& ex)
    {
      LOG4CXX_INFO(logger, "validateJourney throws ErrorResponseException.");

      if (diag != nullptr)
      {
        (*diag) << " \n  FAILED LIMITATIONS VALIDATION\n"
                << "  FAILED ITEM NO " << (*iter)->seqNo() << ":\n";

        displayLimitation(*iter, *diag);

        diag->flushMsg();
      }
      throw ex;
    }
  }

  if (diag != nullptr)
  {
    (*diag) << " \n  PASSED LIMITATIONS VALIDATION\n";
    diag->flushMsg();
  }
}

bool
LimitationOnIndirectTravel::validateJourneyItem(const LimitationJrny& lim, DiagCollector* diag)
{
  // Prequalify for the item
  if (isExemptedByTicketingCarrier(lim) || !isMustWhollyWithinOrNot(lim) ||
      !isJourneyOriginAppl(lim) || !isSalesLoc(lim) || !isTicketLoc(lim))
    return true;

  // Check General Retransit Location
  if (!checkGeneralRetransit(JOURNEY, _itin.travelSeg(), false, lim, diag) ||
      !checkSpecificRetransit(_itin.travelSeg(), lim, diag) ||
      !checkGeneralStopOver(JOURNEY, _itin.travelSeg(), false, lim, diag, true) ||
      !checkSpecificStopOver(_itin.travelSeg(), lim, diag, true) ||
      !checkMaxStopOver(_itin.travelSeg(), lim, diag, true))
  {
    // Journey is not valid when issued on one ticket
    return false;
  }

  return true;
}

bool
LimitationOnIndirectTravel::validatePricingUnit(const PricingUnit& pu, DiagCollector& diag682)
{
  DiagCollector* diag = nullptr; // For Diagnostic370
  bool internationalItin = true;

  if (_diagEnabled)
  {
    if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic370))
    {
      DCFactory* factory = DCFactory::instance();
      diag = factory->create(_trx);
      if (diag != nullptr)
      {
        diag->enable(Diagnostic370);
        (*diag) << " \n---------------------------------------------------------------\n"
                << "PU \n";
      }
    }

    diag682 << " ----------\n";
    const std::vector<FareUsage*>& fuVec = pu.fareUsage();
    std::vector<FareUsage*>::const_iterator fuIter = fuVec.begin();
    for (; fuIter != fuVec.end(); fuIter++)
    {
      FareMarket& fareMarket = *((**fuIter).paxTypeFare()->fareMarket()); // lint !e530

      if (UNLIKELY(diag != nullptr))
      {
        (*diag) << " \nFC " << fareMarket.governingCarrier();

        (*diag) << ((*fuIter)->isInbound() ? " INBOUND" : " OUTBOUND") << "\n";

        displayTravelSegs(fareMarket.travelSeg(), *diag, fareMarket.sideTripTravelSeg().empty());
      }

      displayTravelSegs(fareMarket.travelSeg(), diag682, fareMarket.sideTripTravelSeg().empty());
    }
  }

  const GeoTravelType& geoTrvlType = _itin.geoTravelType();
  if (geoTrvlType == GeoTravelType::Domestic || geoTrvlType == GeoTravelType::ForeignDomestic)
    internationalItin = false;

  if (internationalItin)
  {
    retrievePULimitation();
    for (const auto* limitationFare : _puApplyItems)
    {
      if (!validatePricingUnitItem(pu, *limitationFare, diag))
      {
        if (_diagEnabled)
        {
          diag682 << "     PU FAILED LIMITATIONS VALIDATION\n"
                  << "     PU FAILED ITEM NO " << limitationFare->seqNo() << ":\n";
          displayLimitation(limitationFare, diag682);

          if (diag != nullptr)
          {
            (*diag) << " \n  PU FAILED LIMITATIONS VALIDATION\n"
                    << "  PU FAILED ITEM NO " << limitationFare->seqNo() << ":\n";

            displayLimitation(limitationFare, *diag);

            (*diag) << "---------------------------------------------------------------\n";

            diag->flushMsg();
          }
        }

        return false;
      }

      if (_diagEnabled)
        diag682 << "     PU PASSED ITEM NO " << limitationFare->seqNo() << "\n";
    }
  }

  if (_diagEnabled && !internationalItin)
    diag682 << "  ITIN NOT INTERNATIONAL NOT SUBJECT TO LIMITATIONS VALIDATION\n";

  if (UNLIKELY(diag != nullptr))
  {
    if (!internationalItin)
      (*diag) << "\n  ITIN NOT INTERNATIONAL NOT SUBJECT TO LIMITATIONS VALIDATION\n";
    else
      (*diag) << " \n  PU PASSED LIMITATIONS VALIDATION\n";

    (*diag) << "---------------------------------------------------------------\n";

    diag->flushMsg();
  }

  return true;
}

bool
LimitationOnIndirectTravel::validateFarePath(FarePath& farePath, DiagCollector& diag)
{
  if (UNLIKELY(_trx.getOptions() && _trx.getOptions()->isRtw()))
  {
    diag.enable(Diagnostic682);
    diag << farePath;
    diag << "RTW REQUEST IS NOT SUBJECT TO FARE PATH LIMITATIONS VALIDATION\n\n";
    LOG4CXX_INFO(logger, "RTW Limitation bypass validateFarePath");
    return true;
  }

  bool isValid(true);

  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();
  for (; puIt != puItEnd; ++puIt)
  {
    diag.enable(Diagnostic682);
    diag << *(*puIt);
    if (!validatePricingUnit(*(*puIt), diag))
    {
      diag << "FAILED: PRICING UNIT LIMITATIONS" << std::endl;
      isValid = false;
    }
  }
  return isValid;
}

bool
LimitationOnIndirectTravel::validateFareComponent(FareMarket& fareMarket)
{
  DiagCollector* diag = get370();
  if (UNLIKELY(diag))
  {
    (*diag) << " \nFC " << fareMarket.governingCarrier();

    if (_trx.getOptions()->isRtw())
    {
      (*diag) << "\nRTW REQUEST IS NOT SUBJECT TO COMPONENT LIMITATIONS VALIDATION\n";
      end370(*diag);
    }
    else
    {
      switch (fareMarket.direction())
      {
      case FMDirection::OUTBOUND:
        (*diag) << " OUTBOUND\n";
        break;
      case FMDirection::INBOUND:
        (*diag) << " INBOUND\n";
        break;
      default:
        (*diag) << " UNKNOWN\n";
        break;
      }

      displayTravelSegs(fareMarket.travelSeg(), *diag, fareMarket.sideTripTravelSeg().empty());
    }
  }

  if (_trx.getOptions()->isRtw())
  {
    LOG4CXX_INFO(logger, "RTW Limitation bypass validateFareComponent");
    return true;
  }

  if (!isFareComponentInternational(fareMarket))
  {
    if (UNLIKELY(diag != nullptr))
    {
      (*diag) << "  DOMESTIC FARE COMPONENT NOT SUBJECT TO LIMITATIONS VALIDATION\n";
      diag->flushMsg();
    }

    return true;
  }

  // Fare Component subject to Limitations table validation
  retrieveFCLimitation();
  for (const auto* limitationFare : _fcApplyItems)
  {
    if (UNLIKELY(!validateFareComponentItem(fareMarket, *limitationFare, diag)))
    {
      if (diag != nullptr)
      {
        (*diag) << " \n  FAILED LIMITATIONS VALIDATION\n"
                << "  FAILED ITEM NO " << limitationFare->seqNo() << ":\n";
        displayLimitation(limitationFare, *diag);
        diag->flushMsg();
      }

      return false;
    }
  }

  if (UNLIKELY(diag != nullptr))
  {
    (*diag) << " \n  PASSED LIMITATIONS VALIDATION\n";
    diag->flushMsg();
  }

  return true;
}

bool
LimitationOnIndirectTravel::validateFareComponentAfterHip(const FareUsage& fareUsage)
{
  const PaxTypeFare& thruFare = *fareUsage.paxTypeFare();
  const FareMarket& fareMarket = *(thruFare.fareMarket());

  DiagCollector* diag = nullptr;
  if (_diagEnabled && (_trx.diagnostic().diagnosticType() == Diagnostic370))
  {
    DCFactory* factory = DCFactory::instance();
    diag = factory->create(_trx);
    if (diag != nullptr)
    {
      diag->enable(Diagnostic370);
      (*diag) << " \nFC AFTER HIP";
      (*diag) << " \nFC " << fareMarket.governingCarrier() << " ";
      displayFare(fareUsage, *diag);
      displayTravelSegs(fareMarket.travelSeg(), *diag, fareMarket.sideTripTravelSeg().empty());
    }
  }

  // Fare Component subject to Limitations table validation
  if (isFareComponentInternational(fareMarket))
  {
    retrieveFCLimitation();
    for (const auto* limitationFare : _fcApplyItems)
    {
      if (UNLIKELY(!validateNotViaHipItem(fareUsage, *limitationFare, diag)))
      {
        if (diag != nullptr)
        {
          (*diag) << " \n  FAILED LIMITATIONS VALIDATION\n"
                  << "  FAILED ITEM NO " << limitationFare->seqNo() << ":\n";
          displayLimitation(limitationFare, *diag);
          diag->flushMsg();
        }

        return false;
      }
    }
  }
  if (diag != nullptr)
  {
    (*diag) << " \n  PASSED LIMITATIONS VALIDATION\n";
    diag->flushMsg();
  }

  return true;
}

void
LimitationOnIndirectTravel::retrieveFCLimitation()
{
  boost::unique_lock<boost::mutex> g(_mutex, boost::defer_lock);
  if (fallback::overusedMutexInLimitationOnIndirectTravel(&_trx))
  {
    g.lock();

    if (UNLIKELY(!_fcApplyItems.empty())) // Has been populated
      return;
  }

  // Get LimitationFare records
  const std::vector<LimitationFare*>& limitations =
      //_trx.dataHandle().getFCLimitation(SABRE_USER, _travelCommenceDT);
      _trx.dataHandle().getFCLimitation(_travelCommenceDT);

  for (const auto* limitationFare : limitations)
  {
    if (limitationFare->originTvlAppl() == PU || isExemptedByTicketingCarrier(*limitationFare) ||
        !isSalesLoc(*limitationFare) || !isTicketLoc(*limitationFare) ||
        !isMustWhollyWithinOrNot(*limitationFare) || !isJourneyOriginAppl(*limitationFare))
      continue;

    _fcApplyItems.push_back(limitationFare);
  }
}

void
LimitationOnIndirectTravel::retrievePULimitation()
{
  boost::unique_lock<boost::mutex> g(_mutex, boost::defer_lock);
  if (fallback::overusedMutexInLimitationOnIndirectTravel(&_trx))
  {
    g.lock();

    if (!_puApplyItems.empty()) // Has been populated
      return;
  }

  // Get LimitationFare records with PricingUnitAppl as 'P' or
  // FareComponentAppl is not blank.
  const std::vector<LimitationFare*>& limitations =
      //_trx.dataHandle().getFCLimitation(SABRE_USER, _travelCommenceDT);
      _trx.dataHandle().getFCLimitation(_travelCommenceDT);

  for (const auto* limitationFare : limitations)
  {
    if (((limitationFare->originTvlAppl() != PU) &&
         (limitationFare->fareComponentAppl() == BLANK) && limitationFare->routings().empty()) ||
        isExemptedByTicketingCarrier(*limitationFare) || !isSalesLoc(*limitationFare) ||
        !isTicketLoc(*limitationFare) || !isMustWhollyWithinOrNot(*limitationFare) ||
        !isJourneyOriginAppl(*limitationFare))
      continue;

    _puApplyItems.push_back(limitationFare);
  }
}

bool
LimitationOnIndirectTravel::validateFareComponentItem(FareMarket& fareMarket,
                                                      const LimitationFare& lim,
                                                      DiagCollector* diag,
                                                      const FareUsage* fareUsage /*=0*/)
{
  if ((fareUsage == nullptr) &&
      ((lim.mustNotViaHip() == YES) || !lim.fareType().empty() ||
       !lim.routings().empty())) // The item has to be checked after fare is available.
    return true;

  const std::vector<TravelSeg*>& travelSegs = fareMarket.travelSeg();

  // Prequlify conditions
  if (!matchGovCxrAppl(fareMarket, lim) || // This item does not apply to the FC.
      !matchFareComponentDirection(fareMarket, lim, fareUsage) ||
      !matchGlobalDirection(fareMarket, lim) || !matchAllViaCxr(fareMarket, lim, diag) ||
      !matchRetransitViaGovCxr(fareMarket, lim, diag))
    return true;

  if (fareUsage != nullptr)
  {
    const PaxTypeFare& paxTypeFare = *(fareUsage->paxTypeFare());
    if (!isFareType(paxTypeFare, lim) || !notViaRouting(paxTypeFare, lim))
      return true;
  }

  bool reverseDirection =
      ((fareUsage != nullptr) ? fareUsage->isInbound() : (fareMarket.direction() == FMDirection::INBOUND));

  // Fail conditions
  if (!checkConfirmedStatus(travelSegs, lim, diag) || !notViaIntermediateLoc(travelSegs, lim) ||
      !checkMaxNumDomSeg(travelSegs, lim, diag, (fareUsage == nullptr ? &fareMarket : nullptr)) ||
      !checkGeneralRetransit(FC, travelSegs, reverseDirection, lim, diag, &fareMarket) ||
      !checkSpecificRetransit(travelSegs, lim, diag) ||
      !checkGeneralStopOver(FC,
                            travelSegs,
                            reverseDirection,
                            lim,
                            diag,
                            fareMarket.sideTripTravelSeg().empty(),
                            &fareMarket) ||
      !checkSpecificStopOver(travelSegs, lim, diag, fareMarket.sideTripTravelSeg().empty()) ||
      !checkMaxStopOver(travelSegs, lim, diag, fareMarket.sideTripTravelSeg().empty()))
    return false;

  return true;
}

bool
LimitationOnIndirectTravel::validateNotViaHipItem(const FareUsage& fareUsage,
                                                  const LimitationFare& lim,
                                                  DiagCollector* diag)
{
  if (LIKELY(lim.mustNotViaHip() != YES)) // Only check items with "MustNotViaHip" as YES
    return true;

  const PaxTypeFare& paxTypeFare = *fareUsage.paxTypeFare(); // lint !e530

  const FareMarket& fareMarket = *(paxTypeFare.fareMarket());
  bool reverseDirection = fareUsage.isInbound();

  const std::vector<TravelSeg*>& travelSegs = fareMarket.travelSeg();
  if (!matchGovCxrAppl(fareMarket, lim) || // This item does not apply to the FC.
      !matchFareComponentDirection(fareMarket, lim, &fareUsage) ||
      !matchGlobalDirection(fareMarket, lim) || !isFareType(paxTypeFare, lim) ||
      !notViaRouting(paxTypeFare, lim) || !matchAllViaCxr(fareMarket, lim, diag) ||
      !matchRetransitViaGovCxr(fareMarket, lim, diag))
    return true;

  // Fail conditions
  if (!checkConfirmedStatus(travelSegs, lim, diag) || !notViaIntermediateLoc(travelSegs, lim) ||
      !checkNotViaHip(fareUsage, lim, diag) || !checkMaxNumDomSeg(travelSegs, lim, diag) ||
      !checkGeneralRetransit(FC, travelSegs, reverseDirection, lim, diag, &fareMarket) ||
      !checkSpecificRetransit(travelSegs, lim, diag) ||
      !checkGeneralStopOver(FC,
                            travelSegs,
                            reverseDirection,
                            lim,
                            diag,
                            fareMarket.sideTripTravelSeg().empty(),
                            &fareMarket) ||
      !checkSpecificStopOver(travelSegs, lim, diag, fareMarket.sideTripTravelSeg().empty()) ||
      !checkMaxStopOver(travelSegs, lim, diag, fareMarket.sideTripTravelSeg().empty()))
    return false;

  return true;
}

bool
LimitationOnIndirectTravel::validatePricingUnitItem(const PricingUnit& pu,
                                                    const LimitationFare& lim,
                                                    DiagCollector* diag)
{
  if (!isPricingUnitOriginAppl(pu, lim))
    return true;

  const std::vector<FareUsage*>& fareUsages = pu.fareUsage();
  std::vector<FareUsage*>::const_iterator i = fareUsages.begin();
  for (; i != fareUsages.end(); i++)
  {
    FareMarket& fareMarket = *((*i)->paxTypeFare()->fareMarket()); // lint !e530

    if (!isFareComponentInternational(fareMarket)) // Skip domestic FM
    {
      continue;
    }

    if (lim.routings().empty() && lim.fareType().empty() && (lim.fareComponentAppl() != BLANK) &&
        (fareMarket.direction() != FMDirection::UNKNOWN) &&
        (lim.originTvlAppl() != PU)) // FC item checked already for FC with direction identified up
                                     // front
    {
      continue;
    }

    if (!validateFareComponentItem(fareMarket, lim, diag, *i))
    {
      if (diag != nullptr)
      {
        (*diag) << " \n  FAILED FC:";

        (*diag) << ((*i)->isInbound() ? " INBOUND" : " OUTBOUND") << "\n  ";

        displayFare(**i, *diag);
        displayTravelSegs(fareMarket.travelSeg(), *diag, fareMarket.sideTripTravelSeg().empty());
      }

      return false;
    }
  }
  return true;
}

bool
LimitationOnIndirectTravel::isExemptedByTicketingCarrier(const LimitationCmn& lim)
{
  if (lim.exceptTktgCxrInd() == BLANK)
    return false;

  const CarrierCode& tktCxr = _itin.ticketingCarrier();

  const std::vector<CarrierCode>& tktCxrs = lim.tktgCarriers();
  bool foundCxr = (std::find(tktCxrs.begin(), tktCxrs.end(), tktCxr) != tktCxrs.end());

  if (LIKELY(lim.exceptTktgCxrInd() == YES)) // Only apply to Carriers not in the std::vector
    return foundCxr;
  else // Only apply to Carriers in the std::vector
    return !foundCxr;
}

bool
LimitationOnIndirectTravel::isMustWhollyWithinOrNot(const LimitationCmn& lim)
{
  if (lim.whollyWithinAppl() == BLANK)
    return true;

  bool isPositiveCheck = (lim.whollyWithinAppl() == MUST_BE_WHOLLY_WITHIN);

  return isWithinLoc(_itin.travelSeg(), lim.whollyWithinLoc()) == isPositiveCheck;
}

bool
LimitationOnIndirectTravel::isWithinLoc(const std::vector<TravelSeg*>& travelSegs,
                                        const LocKey& loc)
{
  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  for (; i != travelSegs.end(); i++)
  {
    TravelSeg* curTvlSeg = *i;
    if (UNLIKELY(curTvlSeg == nullptr))
      continue;

    const Loc* curLoc = curTvlSeg->origin();
    if ((curLoc != nullptr) &&
        !LocUtil::isInLoc(*curLoc,
                          loc.locType(),
                          loc.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          _trx.getRequest()->ticketingDT()))
      return false;

    curLoc = curTvlSeg->destination();
    if ((curLoc != nullptr) &&
        !LocUtil::isInLoc(*curLoc,
                          loc.locType(),
                          loc.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          _trx.getRequest()->ticketingDT()))
      return false;
  }

  return true;
}

bool
LimitationOnIndirectTravel::isViaLoc(const std::vector<TravelSeg*>& travelSegs, const LocKey& loc)
{
  std::vector<TravelSeg*>::const_iterator origPos;
  originPos(travelSegs, origPos);
  if (origPos == travelSegs.end())
    return false;

  std::vector<TravelSeg*>::const_iterator destPos;
  destinationPos(travelSegs, destPos);
  if (destPos == travelSegs.end())
    return false;

  std::vector<TravelSeg*>::const_iterator i = origPos;
  for (; i != travelSegs.end(); i++)
  {
    TravelSeg* curTvlSeg = *i;
    if (curTvlSeg == nullptr)
      continue;

    const Loc* curLoc = nullptr;
    if (i != origPos) // Not first TravelSeg, check origin
    {
      curLoc = curTvlSeg->origin();
      if ((curLoc != nullptr) && LocUtil::isInLoc(*curLoc,
                                                  loc.locType(),
                                                  loc.loc(),
                                                  Vendor::SABRE,
                                                  MANUAL,
                                                  LocUtil::OTHER,
                                                  GeoTravelType::International,
                                                  EMPTY_STRING(),
                                                  _trx.getRequest()->ticketingDT()))
        return true;
    }

    if (i != destPos) // Not last TravelSeg, check destin
    {
      curLoc = curTvlSeg->destination();
      if ((curLoc != nullptr) && LocUtil::isInLoc(*curLoc,
                                                  loc.locType(),
                                                  loc.loc(),
                                                  Vendor::SABRE,
                                                  MANUAL,
                                                  LocUtil::OTHER,
                                                  GeoTravelType::International,
                                                  EMPTY_STRING(),
                                                  _trx.getRequest()->ticketingDT()))
        return true;
    }
  }

  return false;
}

bool
LimitationOnIndirectTravel::isToFromLoc(const Loc& loc, const LimitationCmn& lim)
{
  const LimitationFare* fcLim = dynamic_cast<const LimitationFare*>(&lim);
  if (UNLIKELY(fcLim == nullptr))
    return true;

  // Check "Must Be to/from Loc" fields
  const LocKey& locKey = fcLim->notViaToFromLoc();

  if (LIKELY(locKey.locType() == BLANK))
    return true;

  return LocUtil::isInLoc(loc,
                          locKey.locType(),
                          locKey.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          _trx.getRequest()->ticketingDT());
}

bool
LimitationOnIndirectTravel::isToFromLoc(const TravelSeg& travelSeg,
                                        const TravelSeg& nextTravelSeg,
                                        const LimitationCmn& lim)
{
  const LimitationFare* fcLim = dynamic_cast<const LimitationFare*>(&lim);
  if (fcLim == nullptr)
    return true;

  // Check "Must Be to/from Loc" fields
  const LocKey& locKey = fcLim->notViaToFromLoc();

  if (locKey.locType() == BLANK)
    return true;

  // if the travel segment is Foreign Domestic or Domestic, do not count it as
  // to/from loc
  if ((travelSeg.geoTravelType() != GeoTravelType::ForeignDomestic) &&
      (travelSeg.geoTravelType() != GeoTravelType::Domestic) &&
      LocUtil::isInLoc(*(travelSeg.origin()),
                       locKey.locType(),
                       locKey.loc(),
                       Vendor::SABRE,
                       MANUAL,
                       LocUtil::OTHER,
                       GeoTravelType::International,
                       EMPTY_STRING(),
                       _trx.getRequest()->ticketingDT()))
  {
    return true;
  }

  if ((nextTravelSeg.geoTravelType() != GeoTravelType::ForeignDomestic) &&
      (nextTravelSeg.geoTravelType() != GeoTravelType::Domestic) &&
      LocUtil::isInLoc(*(nextTravelSeg.destination()),
                       locKey.locType(),
                       locKey.loc(),
                       Vendor::SABRE,
                       MANUAL,
                       LocUtil::OTHER,
                       GeoTravelType::International,
                       EMPTY_STRING(),
                       _trx.getRequest()->ticketingDT()))
  {
    return true;
  }

  return false;
}

bool
LimitationOnIndirectTravel::isBetweenLocs(const std::vector<TravelSeg*>& travelSegs,
                                          const LocKey& loc1,
                                          const LocKey& loc2)
{
  const Loc* orig = origin(travelSegs);
  const Loc* dest = destination(travelSegs);

  if (UNLIKELY(orig == nullptr || dest == nullptr))
    return false;

  return (isFromLoc1ToLoc2(*orig, *dest, loc1, loc2, _trx.getRequest()->ticketingDT()) ||
          isFromLoc1ToLoc2(*dest, *orig, loc1, loc2, _trx.getRequest()->ticketingDT()));
}

bool
LimitationOnIndirectTravel::isFromLoc1ToLoc2(const std::vector<TravelSeg*>& travelSegs,
                                             const LocKey& loc1,
                                             const LocKey& loc2)
{
  const Loc* orig = origin(travelSegs);
  const Loc* dest = destination(travelSegs);

  if (UNLIKELY(orig == nullptr || dest == nullptr))
    return false;

  return isFromLoc1ToLoc2(*orig, *dest, loc1, loc2, _trx.getRequest()->ticketingDT());
}

bool
LimitationOnIndirectTravel::isFromLoc1ToLoc2(const Loc& orig,
                                             const Loc& dest,
                                             const LocKey& loc1,
                                             const LocKey& loc2,
                                             const DateTime& ticketingDT)
{
  return LocUtil::isInLoc(orig,
                          loc1.locType(),
                          loc1.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          ticketingDT) &&
         LocUtil::isInLoc(dest,
                          loc2.locType(),
                          loc2.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          ticketingDT);
}

bool
LimitationOnIndirectTravel::isJourneyOriginAppl(const LimitationCmn& lim)
{
  if (lim.originTvlAppl() == BLANK)
    return true;

  if (lim.originTvlAppl() == JOURNEY)
  {
    const Loc* orig = origin(_itin.travelSeg());
    if (UNLIKELY(orig == nullptr))
      return false;

    return LocUtil::isInLoc(*orig,
                            lim.originLoc().locType(),
                            lim.originLoc().loc(),
                            Vendor::SABRE,
                            MANUAL,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            _trx.getRequest()->ticketingDT());
  }

  return true;
}

bool
LimitationOnIndirectTravel::isPricingUnitOriginAppl(const PricingUnit& pu, const LimitationCmn& lim)
{
  if (lim.originTvlAppl() == BLANK)
    return true;

  if (lim.originTvlAppl() == PU)
  {
    const Loc* orig = origin(pu.travelSeg());
    if (UNLIKELY(orig == nullptr))
      return false;

    return LocUtil::isInLoc(*orig,
                            lim.originLoc().locType(),
                            lim.originLoc().loc(),
                            Vendor::SABRE,
                            MANUAL,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            _trx.getRequest()->ticketingDT());
  }

  return true;
}

bool
LimitationOnIndirectTravel::isFareType(const PaxTypeFare& paxTypeFare, const LimitationCmn& lim)
{
  if (LIKELY(lim.fareType().empty()))
    return true;

  bool isGeneric = (lim.fareType()[0] == '*');

  if (isGeneric)
  {
    if (!isSameFareTypeDesig(paxTypeFare.fcaFareType(), lim.fareType()))
      return false;
  }
  else
  {
    if (paxTypeFare.fcaFareType() != lim.fareType())
      return false;
  }

  return true;
}

bool
LimitationOnIndirectTravel::isSameFareTypeDesig(const FareType& fareType1,
                                                const FareType& fareType2)
{
  const FareTypeMatrix* fareTypeMatrix1 =
      _trx.dataHandle().getFareTypeMatrix(fareType1, _travelCommenceDT);
  if (fareTypeMatrix1 == nullptr)
    return false;

  const FareTypeMatrix* fareTypeMatrix2 =
      _trx.dataHandle().getFareTypeMatrix(fareType2, _travelCommenceDT);
  if (fareTypeMatrix2 == nullptr)
    return false;

  return (fareTypeMatrix1->fareTypeDesig() == fareTypeMatrix2->fareTypeDesig());
}

bool
LimitationOnIndirectTravel::isSalesLoc(const LimitationCmn& lim)
{
  if (LIKELY(lim.posLoc().locType() == BLANK))
    return true;

  return LocUtil::isInLoc(*(TrxUtil::saleLoc(_trx)),
                          lim.posLoc().locType(),
                          lim.posLoc().loc(),
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          _trx.getRequest()->ticketingDT());
}

bool
LimitationOnIndirectTravel::isTicketLoc(const LimitationCmn& lim)
{
  if (LIKELY(lim.potLoc().locType() == BLANK))
    return true;

  return LocUtil::isInLoc(*(TrxUtil::ticketingLoc(_trx)),
                          lim.potLoc().locType(),
                          lim.potLoc().loc(),
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          _trx.getRequest()->ticketingDT());
}

bool
LimitationOnIndirectTravel::checkGeneralRetransit(LimitationAppl limAppl,
                                                  const std::vector<TravelSeg*>& travelSegs,
                                                  bool reverseDirection,
                                                  const LimitationCmn& lim,
                                                  DiagCollector* diag,
                                                  const FareMarket* fareMarket)
{
  switch (lim.retransitPointAppl())
  {
  case COUNTRY_OF_J_ORIGIN:
  {
    const Loc* orig = origin(_itin.travelSeg());
    if (UNLIKELY(orig == nullptr))
      return true;

    return checkMaxIntlDepartArrivalAtLoc(travelSegs, LOCTYPE_NATION, orig->nation(), lim, diag) &&
           checkMaxRetransitAtLoc(travelSegs, LOCTYPE_NATION, orig->nation(), lim, diag);
  }

  case COUNTRY_OF_FC_ORIGIN:
    if (limAppl == FC)
    {
      const Loc* orig = (reverseDirection ? destination(travelSegs) : origin(travelSegs));
      if (orig == nullptr)
        return true;

      return checkMaxIntlDepartArrivalAtLoc(
                 travelSegs, LOCTYPE_NATION, orig->nation(), lim, diag) &&
             checkMaxRetransitAtLoc(travelSegs, LOCTYPE_NATION, orig->nation(), lim, diag);
    }
    break;

  case COUNTRY_OF_FC_DESTIN:
    if (limAppl == FC)
    {
      const Loc* dest = (reverseDirection ? origin(travelSegs) : destination(travelSegs));
      if (dest == nullptr)
        return true;

      return checkMaxIntlDepartArrivalAtLoc(
                 travelSegs, LOCTYPE_NATION, dest->nation(), lim, diag) &&
             checkMaxRetransitAtLoc(travelSegs, LOCTYPE_NATION, dest->nation(), lim, diag);
    }
    break;

  case COUNTRY_OF_PAYMENT:
  {
    const Loc* saleLoc = TrxUtil::saleLoc(_trx);
    if (saleLoc == nullptr)
      return true;

    return checkMaxIntlDepartArrivalAtLoc(
               travelSegs, LOCTYPE_NATION, saleLoc->nation(), lim, diag) &&
           checkMaxRetransitAtLoc(travelSegs, LOCTYPE_NATION, saleLoc->nation(), lim, diag);
  }

  case INTERMEDIATE_POINT_WITHIN_FC:
    if (limAppl == FC)
    {
      return checkMaxRetransitAtIntermLoc(
          travelSegs, (fareMarket != nullptr ? fareMarket->governingCarrier() : ""), lim, diag);
    }
    break;

  default:
    break;
  }

  return true;
}

// Check Specific Retransit for J/P/F.
bool
LimitationOnIndirectTravel::checkSpecificRetransit(const std::vector<TravelSeg*>& travelSegs,
                                                   const LimitationCmn& lim,
                                                   DiagCollector* diag)
{
  if (lim.retransitLoc().locType() == BLANK || lim.retransitLoc().loc().empty())
    return true;

  return checkMaxIntlDepartArrivalAtLoc(
             travelSegs, lim.retransitLoc().locType(), lim.retransitLoc().loc(), lim, diag) &&
         checkMaxRetransitAtLoc(
             travelSegs, lim.retransitLoc().locType(), lim.retransitLoc().loc(), lim, diag);
}

bool
LimitationOnIndirectTravel::checkMaxIntlDepartArrivalAtLoc(
    const std::vector<TravelSeg*>& travelSegs,
    const LocTypeCode& locType,
    const LocCode& locCode,
    const LimitationCmn& lim,
    DiagCollector* diag)
{
  int maxDepart = lim.intlDepartMaxNo();
  int maxArrival = lim.intlArrivalMaxNo();

  bool checkDepart = true;
  bool checkArrival = true;

  if (UNLIKELY((maxDepart < 0) || (maxDepart > MAX_DEPART)))
    checkDepart = false;

  if (UNLIKELY((maxArrival < 0) || (maxArrival > MAX_ARRIVAL)))
    checkArrival = false;

  if (UNLIKELY(!checkDepart && !checkArrival))
    return true;

  // Create a map to save number of retransits in any of countries within
  // the locType and locCode.
  std::map<NationCode, int> departCountry;
  std::map<NationCode, int>::iterator departCountryPos;
  std::map<NationCode, int> arrivalCountry;
  std::map<NationCode, int>::iterator arrivalCountryPos;

  bool isRetransit = true;
  // const Loc* lastDest = 0;

  std::vector<TravelSeg*>::const_iterator travelSegIter = travelSegs.begin();
  for (; travelSegIter != travelSegs.end(); travelSegIter++)
  {
    TravelSeg* travelSeg = *travelSegIter;

    // Origin
    const Loc* departLoc = travelSeg->origin();
    bool isDepartAtRetransit = isRetransit;
    bool isInternational = false;

    // Destination
    isRetransit = LocUtil::isInLoc(*(travelSeg->destination()),
                                   locType,
                                   locCode,
                                   Vendor::SABRE,
                                   MANUAL,
                                   LocUtil::OTHER,
                                   GeoTravelType::International,
                                   EMPTY_STRING(),
                                   _trx.getRequest()->ticketingDT());

    isInternational = !isSameNation(*departLoc, *((*travelSegIter)->destination()));

    if (checkDepart && isDepartAtRetransit && isInternational)
    {
      if ((departCountryPos = departCountry.find(departLoc->nation())) == departCountry.end())
      {
        // First encounter the country, add it to the departCountry map
        departCountry.insert(std::map<NationCode, int>::value_type(departLoc->nation(), 1));
      }
      else
      {
        // Increment the retransit number for the country
        departCountryPos->second++;
      }
    }

    if (checkArrival && isRetransit && isInternational)
    {
      if ((arrivalCountryPos = arrivalCountry.find(travelSeg->destination()->nation())) ==
          arrivalCountry.end())
      {
        // First encounter the country, add it to the arrivalCountry map
        arrivalCountry.insert(
            std::map<NationCode, int>::value_type(travelSeg->destination()->nation(), 1));
      }
      else
      {
        // Increment the retransit number for the country
        arrivalCountryPos->second++;
      }
    }

    // lastDest = travelSeg->destination();
  }

  // Check if any country within the locType and locCode exceed maximum retransit limits.
  departCountryPos = departCountry.begin();
  for (; departCountryPos != departCountry.end(); departCountryPos++)
  {
    if (departCountryPos->second > maxDepart)
    {
      displayRetransitErrMsg(departCountryPos->first, lim, diag);

      return false;
    }
  }
  arrivalCountryPos = arrivalCountry.begin();
  for (; arrivalCountryPos != arrivalCountry.end(); arrivalCountryPos++)
  {
    if (arrivalCountryPos->second > maxArrival)
    {
      // Print out text message in format of lim.textMessage()
      // with the country code.
      displayRetransitErrMsg(arrivalCountryPos->first, lim, diag);

      return false;
    }
  }

  return true;
}

void
LimitationOnIndirectTravel::displayRetransitErrMsg(const NationCode& nation,
                                                   const LimitationCmn& lim,
                                                   DiagCollector* diag)
{
  // Find text message for Journey defined in table
  const LimitationJrny* jLim = dynamic_cast<const LimitationJrny*>(&lim);
  if (jLim != nullptr)
  {
    std::string textMsg;

    const std::vector<std::string>& textMsgVec = jLim->textMsg();
    std::vector<std::string>::const_iterator i = textMsgVec.begin();
    for (; i != textMsgVec.end(); i++)
      textMsg += *i;

    size_t pos = textMsg.find(REPLACE_TOKEN);
    if (pos != std::string::npos) // lint !e530
    {
      DataHandle dataHandle(_trx.ticketingDate());
      const Nation* nationDetail = dataHandle.getNation(nation, _travelCommenceDT);
      if (nationDetail != nullptr)
        textMsg.replace(pos, REPLACE_TOKEN.size(), nationDetail->description());
    }

    if (diag != nullptr)
      (*diag) << " \n  " << textMsg << "\n";

    // For Journey, throw exception to stop further process
    throw ErrorResponseException(ErrorResponseException::LMT_ISSUE_SEP_TKTS_EXCEED_NUM_DEPT_ARR,
                                 textMsg.c_str());
  }
  else // For FC or PU, just display hardcoded message in diagnostic
  {
    if (diag != nullptr)
    {
      (*diag) << "  MORE THAN " << lim.intlDepartMaxNo() << " ARR/DEP IN ";

      DataHandle dataHandle(_trx.ticketingDate());
      const Nation* nationDetail = dataHandle.getNation(nation, _travelCommenceDT);
      if (nationDetail != nullptr)
        (*diag) << nationDetail->description();

      (*diag) << " \n";
    }
  }
}

bool
LimitationOnIndirectTravel::isSameNation(const Loc& loc1, const Loc& loc2)
{
  return (loc1.nation() == loc2.nation()) ? true : false;
}

bool
LimitationOnIndirectTravel::retransitFareComponentBoardOffPoint(const FareMarket& fareMarket)
{
  const std::vector<TravelSeg*>& travelSegs = fareMarket.travelSeg();

  std::vector<TravelSeg*>::const_iterator origPos;
  fareMarketOriginPos(fareMarket, origPos);
  if (UNLIKELY(origPos == travelSegs.end()))
    return false;

  std::vector<TravelSeg*>::const_iterator destPos;
  fareMarketDestinationPos(fareMarket, destPos);
  if (UNLIKELY(destPos == travelSegs.end()))
    return false;

  LocCode fcBoardCity = FareMarketUtil::getBoardMultiCity(fareMarket, **origPos);
  LocCode fcOffCity = FareMarketUtil::getOffMultiCity(fareMarket, **destPos);

  std::vector<TravelSeg*>::const_iterator i = origPos;
  std::vector<TravelSeg*>::const_iterator ei = travelSegs.end();

  // Check if there is retransit of the board point and off point including
  // stopover
  for (; i != ei; i++)
  {
    TravelSeg* travelSeg = *i;
    if (UNLIKELY(travelSeg == nullptr))
      continue;

    LocCode origCity = FareMarketUtil::getBoardMultiCity(fareMarket, *travelSeg);
    LocCode destCity = FareMarketUtil::getOffMultiCity(fareMarket, *travelSeg);

    if (origCity == destCity)
      continue;

    // Check board point of travel seg.
    if (i != origPos) // Not first TravelSeg, check origin point
    {
      if (fcBoardCity == origCity)
      {
        displayRetransitFareComponentBoardOffPointFailed(fareMarket, true);
        return true;
      }

      if (UNLIKELY(fcOffCity == origCity))
      {
        displayRetransitFareComponentBoardOffPointFailed(fareMarket, false);
        return true;
      }
    }

    // Check off point of travel seg

    if (i != destPos) // Not last travelSeg, check dest point
    {
      if (fcBoardCity == destCity)
      {
        displayRetransitFareComponentBoardOffPointFailed(fareMarket, true);
        return true;
      }

      if (fcOffCity == destCity)
      {
        displayRetransitFareComponentBoardOffPointFailed(fareMarket, false);
        return true;
      }
    }
  }

  return false;
}

void
LimitationOnIndirectTravel::displayRetransitFareComponentBoardOffPointFailed(
    const FareMarket& fareMarket, bool retransitBoard)
{
  if (_diagEnabled && (_trx.diagnostic().diagnosticType() == Diagnostic370))
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(_trx);
    if (diag != nullptr)
    {
      diag->enable(Diagnostic370);
      (*diag) << " \nFC " << fareMarket.governingCarrier();

      switch (fareMarket.direction())
      {
      case FMDirection::OUTBOUND:
        (*diag) << " OUTBOUND\n";
        break;
      case FMDirection::INBOUND:
        (*diag) << " INBOUND\n";
        break;
      default:
        (*diag) << " UNKNOWN\n";
        break;
      }

      displayTravelSegs(fareMarket.travelSeg(), *diag, fareMarket.sideTripTravelSeg().empty());

      (*diag) << (retransitBoard ? "\n  RETRANSIT FARE COMPONENT BOARD POINT\n"
                                 : "\n  RETRANSIT FARE COMPONENT OFF POINT\n")
              << " \n  FAILED LIMITATIONS VALIDATION\n";

      diag->flushMsg();
    }
  }
}

const Loc*
LimitationOnIndirectTravel::origin(const std::vector<TravelSeg*>& travelSegs)
{
  if (UNLIKELY(travelSegs.empty()))
    return nullptr;

  return travelSegs.front()->origin(); // Even include ARNK.
}

const Loc*
LimitationOnIndirectTravel::destination(const std::vector<TravelSeg*>& travelSegs)
{
  if (UNLIKELY(travelSegs.empty()))
    return nullptr;

  return travelSegs.back()->destination(); // Even include ARNK.
}

void
LimitationOnIndirectTravel::originPos(const std::vector<TravelSeg*>& travelSegs,
                                      std::vector<TravelSeg*>::const_iterator& origPos)
{
  origPos = travelSegs.begin();
}

void
LimitationOnIndirectTravel::fareMarketOriginPos(const FareMarket& fareMarket,
                                                std::vector<TravelSeg*>::const_iterator& origPos)
{
  const std::vector<TravelSeg*>& travelSegs = fareMarket.travelSeg();
  origPos = travelSegs.begin();
  for (; origPos != travelSegs.end(); ++origPos)
  {
    TravelSeg& travelSeg = **origPos;
    if (FareMarketUtil::getBoardMultiCity(fareMarket, travelSeg) !=
        FareMarketUtil::getOffMultiCity(fareMarket, travelSeg))
      break;
  }
}

void
LimitationOnIndirectTravel::destinationPos(const std::vector<TravelSeg*>& travelSegs,
                                           std::vector<TravelSeg*>::const_iterator& destPos)
{
  if (UNLIKELY(travelSegs.empty()))
    destPos = travelSegs.end();
  else
    destPos = travelSegs.end() - 1;
}

void
LimitationOnIndirectTravel::fareMarketDestinationPos(
    const FareMarket& fareMarket, std::vector<TravelSeg*>::const_iterator& destPos)
{
  const std::vector<TravelSeg*>& travelSegs = fareMarket.travelSeg();
  std::vector<TravelSeg*>::const_reverse_iterator i = travelSegs.rbegin();
  for (; i != travelSegs.rend(); ++i)
  {
    TravelSeg& travelSeg = **i;
    if (FareMarketUtil::getBoardMultiCity(fareMarket, travelSeg) !=
        FareMarketUtil::getOffMultiCity(fareMarket, travelSeg))
      break;
  }

  if (UNLIKELY(i == travelSegs.rend()))
    destPos = travelSegs.end();
  else
    destPos = i.base() - 1;
}

void
LimitationOnIndirectTravel::displayLimitation(const LimitationCmn* lim, DiagCollector& diag)
{
  diag << " \n  APPLICATION - " << lim->limitationAppl() << "\n"
       << "  EXCEPT TKTG CXR - " << lim->exceptTktgCxrInd() << "\n"
       << "  TKTG CXR - ";

  const std::vector<CarrierCode>& tktgCxrs = lim->tktgCarriers();
  std::vector<CarrierCode>::const_iterator tktgCxrIter = tktgCxrs.begin();
  for (; tktgCxrIter != tktgCxrs.end(); tktgCxrIter++)
    diag << *tktgCxrIter << " ";
  diag << " \n";

  diag << "  JOURNEY MUSTBE/MUSTNOTBE WHOLLY WITHIN APPL - " << lim->whollyWithinAppl() << "\n"
       << "  JOURNEY MUSTBE/MUSTNOTBE WHOLLY WITHIN LOC - " << lim->whollyWithinLoc().locType()
       << " " << lim->whollyWithinLoc().loc() << "\n"
       << "  JOURNEY/PU ORIGIN APPL - " << lim->originTvlAppl() << "\n"
       << "  JOURNEY/PU ORIGIN LOC - " << lim->originLoc().locType() << " "
       << lim->originLoc().loc() << "\n"
       << "  MAX NBR INTL DEPARTS - ";
  if (lim->intlDepartMaxNo() >= 0)
    diag << lim->intlDepartMaxNo();
  diag << " \n"
       << "  MAX NBR INTL ARRIVALS - ";
  if (lim->intlArrivalMaxNo() >= 0)
    diag << lim->intlArrivalMaxNo();
  diag << " \n"
       << "  MAX NBR RETRANSITS ALLOWED - ";
  if (lim->maxRetransitAllowed() >= 0)
    diag << lim->maxRetransitAllowed();
  diag << " \n"
       << "  GENERAL RETRANSIT LOC - " << lim->retransitPointAppl() << "\n"
       << "  SPECIFIC RETRANSIT LOC - " << lim->retransitLoc().locType() << " "
       << lim->retransitLoc().loc() << "\n"
       << "  MAX NBR STOPOVERS AT RETRANSIT LOCS - ";
  if (lim->maxStopsAtRetransit() >= 0)
    diag << lim->maxStopsAtRetransit();
  diag << " \n"
       << "  GENERAL STOPOVER LOC - " << lim->viaPointStopoverAppl() << "\n"
       << "  SPECIFIC STOPOVER LOC - " << lim->stopoverLoc().locType() << " "
       << lim->stopoverLoc().loc() << "\n"
       << "  SALES LOC - " << lim->posLoc().locType() << " " << lim->posLoc().loc() << "\n"
       << "  TICKET LOC - " << lim->potLoc().locType() << " " << lim->potLoc().loc() << "\n";

  // Fields only apply to Journey
  const LimitationJrny* jLim = dynamic_cast<const LimitationJrny*>(lim);
  if (jLim != nullptr)
  {
    diag << "  ISSUE SEPARATE TICKET - " << jLim->separateTktInd() << "\n";
    const std::vector<std::string>& textMsg = jLim->textMsg();
    std::vector<std::string>::const_iterator textMsgIter = textMsg.begin();
    for (; textMsgIter != textMsg.end(); textMsgIter++)
      diag << "  " << (*textMsgIter) << "\n";
  }

  // Fields only apply to PU

  // Fields only apply to FareComponent
  const LimitationFare* fcLim = dynamic_cast<const LimitationFare*>(lim);
  if (fcLim != nullptr)
  {
    diag << "  GOV CXR APPL - " << fcLim->govCarrierAppl() << "\n"
         << "  GOV CXR - ";

    const std::vector<CarrierCode>& govCxrs = fcLim->govCarriers();
    std::vector<CarrierCode>::const_iterator govCxrIter = govCxrs.begin();
    for (; govCxrIter != govCxrs.end(); govCxrIter++)
      diag << *govCxrIter << " ";
    diag << "\n";

    diag << "  FC DIRECTIONAL IND - " << fcLim->directionality() << "\n"
         << "  FC OUTBOUND/INBOUND IND - " << fcLim->fareComponentAppl() << "\n"
         << "  FC LOC 1 - " << fcLim->loc1().locType() << " " << fcLim->loc1().loc() << "\n"
         << "  FC LOC 2 - " << fcLim->loc2().locType() << " " << fcLim->loc2().loc() << "\n"
         << "  GLOBAL IND - ";

    std::string gd;
    globalDirectionToStr(gd, fcLim->globalDir());
    diag << gd << "\n"
         << "  MUST NOT VIA HIP - " << fcLim->mustNotViaHip() << "\n"
         << "  FARE TYPE - " << fcLim->fareType() << "\n"
         << "  CONFIRMED FLIGHT SEGMENT STATUS REQUIRED - " << fcLim->confirmedAppl() << "\n"
         << "  NOT VIA ROUTING - ";

    const std::vector<RoutingNumber>& routingVec = fcLim->routings();
    std::vector<RoutingNumber>::const_iterator routingIter = routingVec.begin();
    for (; routingIter != routingVec.end(); routingIter++)
      diag << *routingIter << " ";
    diag << "\n";

    diag << "  FC MUST NOT VIA INTERMEDIATE LOC - " << fcLim->notViaLoc().locType() << " "
         << fcLim->notViaLoc().loc() << "\n"
         << "  MAX NBR DOMESTIC SEGS ALLOWED IN FC - " << fcLim->maxDomSegments() << "\n"
         << "  EXCEPT VIA CXR/LOC IND - " << fcLim->exceptViaCxrLocInd() << "\n";

    const std::vector<LimitFareCxrLoc*>& cxrLocVec = fcLim->exceptViaCxrs();
    std::vector<LimitFareCxrLoc*>::const_iterator cxrLocIter = cxrLocVec.begin();
    for (; cxrLocIter != cxrLocVec.end(); cxrLocIter++)
    {
      diag << "  ONLY VALID VIA CXR - " << (*cxrLocIter)->carrier() << "\n"
           << "  VIA CXR LOC 1 - " << (*cxrLocIter)->loc1().locType() << " "
           << (*cxrLocIter)->loc1().loc() << "\n"
           << "  VIA CXR LOC 2 - " << (*cxrLocIter)->loc2().locType() << " "
           << (*cxrLocIter)->loc2().loc() << "\n";
    }

    diag << "  MUST BE TO/FROM LOC - " // Field name has changed to opposite meaning!
         << fcLim->notViaToFromLoc().locType() << " " << fcLim->notViaToFromLoc().loc() << "\n"
         << "  ALL TRAVEL FROM/TO INTERM RETRANSIT POINT MUST VIA GOV CXR - "
         << fcLim->retransitGovCxrAppl() << "\n"
         << "  ALL TRAVEL MUST BE VIA GOX CXR - " << fcLim->viaGovCarrierInd() << "\n";
  }

  diag << " \n";
  diag.flushMsg();
}

bool
LimitationOnIndirectTravel::isFareComponentInternational(const FareMarket& fareMarket)
{
  // For verifing if FC is Domestic, USA and Canada considered the same country.
  const GeoTravelType geoTrvlType = fareMarket.geoTravelType();
  if (geoTrvlType == GeoTravelType::Domestic || geoTrvlType == GeoTravelType::ForeignDomestic)
    return false;
  else
    return true;
}

void
LimitationOnIndirectTravel::displayTravelSegs(const std::vector<TravelSeg*>& travelSegVec,
                                              DiagCollector& diag,
                                              bool noSideTrip)
{
  std::vector<TravelSeg*>::const_iterator destPos;
  destinationPos(travelSegVec, destPos);

  std::vector<TravelSeg*>::const_iterator i = travelSegVec.begin();
  for (; i != travelSegVec.end(); i++)
  {
    TravelSeg* curTvlSeg = *i;

    // Display the Travel Points
    diag << "  " << curTvlSeg->origin()->loc() << "-";

    AirSeg* curAirSeg = dynamic_cast<AirSeg*>(curTvlSeg);
    if (curAirSeg == nullptr)
    {
      diag << "--";
    }
    else
      diag << curAirSeg->carrier();

    diag << "-" << curTvlSeg->destination()->loc() << "  ";

    if (i != destPos)
      diag << (isStopOver(curTvlSeg, noSideTrip ? nullptr : *(i + 1)) ? "O  " : "   ");
    else
      diag << "   ";

    if (curAirSeg != nullptr)
      diag << curAirSeg->resStatus();

    diag << " \n";
  }
}

bool
LimitationOnIndirectTravel::matchGovCxrAppl(const FareMarket& fareMarket, const LimitationFare& lim)
{
  if (lim.govCarrierAppl() == BLANK)
    return true;

  bool foundCxr = false;
  const CarrierCode& govCxr = fareMarket.governingCarrier();

  const std::vector<CarrierCode>& govCxrs = lim.govCarriers();
  std::vector<CarrierCode>::const_iterator govCxrIter = govCxrs.begin();
  for (; govCxrIter != govCxrs.end(); govCxrIter++)
  {
    if (govCxr == *govCxrIter)
    {
      foundCxr = true;
      break;
    }
  }

  if (foundCxr && lim.govCarrierAppl() == NO) // Must not via Carriers in the std::vector
    return false;

  if (!foundCxr && lim.govCarrierAppl() == YES) // Must via Carriers in the std::vector
    return false;

  return true;
}

bool
LimitationOnIndirectTravel::matchFareComponentDirection(const FareMarket& fareMarket,
                                                        const LimitationFare& lim,
                                                        const FareUsage* fareUsage /*=0*/)
{
  // Check Outbound/Inbound Ind (Note: FareMarket direction maybe UNKNOWN.)
  Indicator fcAppl = lim.fareComponentAppl();
  if (((fcAppl == OUTBOUND) &&
       ((fareUsage != nullptr ? fareUsage->isInbound()
                        : (fareMarket.direction() != FMDirection::OUTBOUND)))) ||
      ((fcAppl == INBOUND) &&
       ((fareUsage != nullptr ? (!fareUsage->isInbound())
                        : (fareMarket.direction() != FMDirection::INBOUND)))) ||
      ((fcAppl == BOTH) && (fareUsage == nullptr) && (fareMarket.direction() == FMDirection::UNKNOWN)))
  {
    // This item does not apply to the FC
    return false;
  }

  Indicator directInd = lim.directionality();
  const std::vector<TravelSeg*>& travelSegs = fareMarket.travelSeg();

  switch (directInd)
  {
  case BLANK:
    return true;

  case BETWEEN:
    return isBetweenLocs(travelSegs, lim.loc1(), lim.loc2());

  case WITHIN:
    return isWithinLoc(travelSegs, lim.loc1());

  case FROM:
    if (UNLIKELY(((fareUsage != nullptr) && fareUsage->isInbound()) || // Special code for inbound FC from match
        (fareMarket.direction() == FMDirection::INBOUND)))
    {
      return LocUtil::isInLoc(*destination(travelSegs),
                              lim.loc1().locType(),
                              lim.loc1().loc(),
                              Vendor::SABRE,
                              MANUAL,
                              LocUtil::OTHER,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              _trx.getRequest()->ticketingDT()) &&
             LocUtil::isInLoc(*origin(travelSegs),
                              lim.loc2().locType(),
                              lim.loc2().loc(),
                              Vendor::SABRE,
                              MANUAL,
                              LocUtil::OTHER,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              _trx.getRequest()->ticketingDT());
    }
    else
    {
      return isFromLoc1ToLoc2(travelSegs, lim.loc1(), lim.loc2());
    }

  default:
    return true;
  }
}

bool
LimitationOnIndirectTravel::matchGlobalDirection(const FareMarket& fareMarket,
                                                 const LimitationFare& lim)
{
  if (lim.globalDir() == GlobalDirection::ZZ)
    return true;

  return fareMarket.getGlobalDirection() == lim.globalDir();
}

bool
LimitationOnIndirectTravel::notViaIntermediateLoc(const std::vector<TravelSeg*>& travelSegs,
                                                  const LimitationFare& lim)
{
  const LocKey& notViaLoc = lim.notViaLoc();

  if (LIKELY(notViaLoc.locType() == BLANK || notViaLoc.loc().empty()))
    return true;

  return !isViaLoc(travelSegs, notViaLoc);
}

bool
LimitationOnIndirectTravel::checkConfirmedStatus(const std::vector<TravelSeg*>& travelSegs,
                                                 const LimitationFare& lim,
                                                 DiagCollector* diag)
{
  if (LIKELY(lim.confirmedAppl() != YES))
    return true;

  // Check if all travel segs are confirmed
  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  for (; i != travelSegs.end(); i++)
  {
    AirSeg* curTvlSeg = dynamic_cast<AirSeg*>(*i);
    if (curTvlSeg == nullptr)
      continue;

    if (curTvlSeg->resStatus() != CONFIRMED_STATUS)
    {
      if (diag != nullptr)
        (*diag) << "  FLIGHT SEGMENT STATUS MUST BE CONFIRMED\n";

      return false;
    }
  }

  return true;
}

bool
LimitationOnIndirectTravel::checkNotViaHip(const FareUsage& fareUsage,
                                           const LimitationFare& lim,
                                           DiagCollector* diag)
{
  if (lim.mustNotViaHip() != YES)
    return true;

  // Check if there is HIP plus up
  if (fareUsage.minFarePlusUp().getSum(HIP) > 0)
  {
    if (diag != nullptr)
      (*diag) << "  MUST NOT BE VIA HIP\n";

    return false;
  }

  return true;
}

bool
LimitationOnIndirectTravel::notViaRouting(const PaxTypeFare& paxTypeFare, const LimitationFare& lim)
{
  const std::vector<RoutingNumber>& exemptedRoutings = lim.routings();
  if (exemptedRoutings.empty())
    return true;

  if (paxTypeFare.isRouting())
  {
    const RoutingNumber& routingNum = paxTypeFare.routingNumber();

    if (std::find(exemptedRoutings.begin(), exemptedRoutings.end(), routingNum) !=
        exemptedRoutings.end())
      return false;
  }

  return true;
}

bool
LimitationOnIndirectTravel::checkMaxNumDomSeg(const std::vector<TravelSeg*>& travelSegs,
                                              const LimitationFare& lim,
                                              DiagCollector* diag,
                                              FareMarket* fm)
{
  int maxDom = -1;

  if (lim.maxDomSegments() != BLANK)
  {
    std::ostringstream maxStr;
    maxStr << lim.maxDomSegments();
    maxDom = atoi(maxStr.str().c_str());
  }

  const std::vector<LimitFareCxrLoc*>& exceptViaCxrLocs = lim.exceptViaCxrs();

  if ((maxDom < 0) || (maxDom > MAX_DOMESTIC)) // Unlimited Domestic Seg
  {
    if (UNLIKELY(lim.exceptViaCxrLocInd() == YES &&
        !exceptViaCxrLocs.empty())) // No travel seg allowed for the Cxrs and Locs
    {
      std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
      for (; i != travelSegs.end(); i++)
      {
        AirSeg* curTvlSeg = dynamic_cast<AirSeg*>(*i);
        if (curTvlSeg == nullptr)
          continue;

        std::vector<LimitFareCxrLoc*>::const_iterator cxrLocIter = exceptViaCxrLocs.begin();
        for (; cxrLocIter != exceptViaCxrLocs.end(); cxrLocIter++)
        {
          if (!(*cxrLocIter)->carrier().empty()) // Via Cxr specified
          {
            if (curTvlSeg->carrier() == (*cxrLocIter)->carrier())
            {
              if (diag != nullptr)
                (*diag) << "  NO TRAVEL ALLOWED VIA CARRIER " << (*cxrLocIter)->carrier() << " \n";
              return false; // This seg is via the excepted cxr.
            }
          }

          if ((*cxrLocIter)->loc1().locType() != BLANK) // Except Locs specified
          {
            if (LocUtil::isInLoc(*(curTvlSeg->origin()),
                                 (*cxrLocIter)->loc1().locType(),
                                 (*cxrLocIter)->loc1().loc(),
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::OTHER,
                                 GeoTravelType::International,
                                 EMPTY_STRING(),
                                 _trx.getRequest()->ticketingDT()))
            {
              if (diag != nullptr)
                (*diag) << "  NO TRAVEL ALLOWED VIA LOC " << (*cxrLocIter)->loc1().locType() << " "
                        << (*cxrLocIter)->loc1().loc() << " \n";

              return false;
            }
          }
        }
      }
    }
  }
  else // Max Domestic Segs Specified
  {
    if (LIKELY(lim.exceptViaCxrLocInd() == NO &&
        !exceptViaCxrLocs.empty())) // Max Domestic segs only apply for the Cxrs and Locs
    {
      int numDom = 0;

      std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
      for (; i != travelSegs.end(); i++)
      {
        TravelSeg* curTvlSeg = *i;
        AirSeg* curAirSeg = dynamic_cast<AirSeg*>(curTvlSeg);
        if (curAirSeg == nullptr) // Do not count surface travel for domestic segment check.
          continue;

        if (isSameNation(*(curTvlSeg->origin()), *(curTvlSeg->destination()))) // Domestic Seg
        {
          std::vector<LimitFareCxrLoc*>::const_iterator cxrLocIter =
              exceptViaCxrLocs.begin(); // It should only contain one element.
          for (; cxrLocIter != exceptViaCxrLocs.end(); cxrLocIter++)
          {
            if ((*cxrLocIter)->carrier().empty() || // Via Cxr not specified
                (curAirSeg->carrier() == (*cxrLocIter)->carrier())) // Via Cxr specified
            {
              if (((*cxrLocIter)->loc1().locType() == BLANK) || // Locs not specified
                  LocUtil::isInLoc(*(curTvlSeg->origin()), // Locs specified
                                   (*cxrLocIter)->loc1().locType(),
                                   (*cxrLocIter)->loc1().loc(),
                                   Vendor::SABRE,
                                   MANUAL,
                                   LocUtil::OTHER,
                                   GeoTravelType::International,
                                   EMPTY_STRING(),
                                   _trx.getRequest()->ticketingDT())) // Domestic in the country
              {
                numDom++; // count this domestic seg
                break;
              }
            }
          }
        }
      }

      if (numDom > maxDom)
      {
        if (diag != nullptr)
          (*diag) << "  MORE THAN " << maxDom << " DOMESTIC "
                  << (maxDom > 1 ? "SEGMENTS" : "SEGMENT") << " AT LOC "
                  << exceptViaCxrLocs[0]->loc1().locType() << " "
                  << exceptViaCxrLocs[0]->loc1().loc() << " \n";

        // Pass "No domestic allowed in Nation VN/MM/MG in intl FC" for MinFare
        // But mark the through FC not for Pricing
        if ((maxDom == 0) && (fm != nullptr) && (exceptViaCxrLocs[0]->loc1().locType() == 'N') &&
            ((exceptViaCxrLocs[0]->loc1().loc() == "VN") ||
             (exceptViaCxrLocs[0]->loc1().loc() == "MM") ||
             (exceptViaCxrLocs[0]->loc1().loc() == "MG")))
        {
          fm->setBreakIndicator(true);

          if (diag != nullptr)
            (*diag) << "  N/A FOR PRICING \n";

          return true;
        }

        return false;
      }
    }
  }

  return true;
}

bool
LimitationOnIndirectTravel::matchAllViaCxr(const FareMarket& fareMarket,
                                           const LimitationFare& lim,
                                           DiagCollector* diag)
{
  if (lim.viaGovCarrierInd() == YES)
    return allViaGovCxr(fareMarket.travelSeg(), fareMarket.governingCarrier());
  else if (lim.viaGovCarrierInd() == NO)
    return !allViaGovCxr(fareMarket.travelSeg(), fareMarket.governingCarrier());
  else
    return true;
}

bool
LimitationOnIndirectTravel::allViaGovCxr(const std::vector<TravelSeg*>& travelSegs,
                                         const CarrierCode& govCxr)
{
  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  for (; i != travelSegs.end(); i++)
  {
    AirSeg* curTvlSeg = dynamic_cast<AirSeg*>(*i);
    if (curTvlSeg == nullptr)
      continue;

    if (curTvlSeg->carrier() != govCxr)
      return false;
  }

  return true;
}

bool
LimitationOnIndirectTravel::matchRetransitViaGovCxr(const FareMarket& fareMarket,
                                                    const LimitationFare& lim,
                                                    DiagCollector* diag)
{
  if (lim.retransitGovCxrAppl() == BLANK)
    return true;

  const std::vector<TravelSeg*>& travelSegs = fareMarket.travelSeg();
  const CarrierCode& govCxr = fareMarket.governingCarrier();

  std::map<LocCode, int> retransits;
  getRetransits(travelSegs, fareMarket.governingCarrier(), retransits);

  std::map<LocCode, int>::iterator retrIter = retransits.begin();
  for (; retrIter != retransits.end(); retrIter++)
  {
    int numRetransit = (*retrIter).second;
    if (numRetransit <= 0)
      continue;

    bool isAllViaCxr = retransitViaGovCxr(travelSegs, govCxr, (*retrIter).first, numRetransit);
    if (((lim.retransitGovCxrAppl() == YES) && (!isAllViaCxr)) ||
        ((lim.retransitGovCxrAppl() == NO) && isAllViaCxr))
      return false;
  }

  return true;
}

void
LimitationOnIndirectTravel::getRetransits(const std::vector<TravelSeg*>& travelSegs,
                                          const CarrierCode& govCxr,
                                          std::map<LocCode, int>& retransits)
{
  std::vector<TravelSeg*>::const_iterator origPos;
  originPos(travelSegs, origPos);
  if (origPos == travelSegs.end())
    return;

  std::vector<TravelSeg*>::const_iterator destPos;
  destinationPos(travelSegs, destPos);
  if (destPos == travelSegs.end())
    return;

  LocCode origCity = boardMultiCity(_trx.dataHandle(), **origPos, _itin.geoTravelType(), govCxr);
  LocCode destCity = offMultiCity(_trx.dataHandle(), **destPos, _itin.geoTravelType(), govCxr);

  const TravelSeg* lastTravelSeg = nullptr;

  std::map<LocCode, int>::iterator pos;
  std::vector<TravelSeg*>::const_iterator travelSegIter = origPos;
  for (; travelSegIter != travelSegs.end(); travelSegIter++)
  {
    TravelSeg* travelSeg = *travelSegIter;

    // Origin
    LocCode departCity =
        boardMultiCity(_trx.dataHandle(), *travelSeg, _itin.geoTravelType(), govCxr);

    if (departCity != origCity && departCity != destCity)
    {
      if ((lastTravelSeg != nullptr) &&
          (departCity !=
           offMultiCity(_trx.dataHandle(), *lastTravelSeg, _itin.geoTravelType(), govCxr)))
      {
        // Increase retansit number for this loc
        if ((pos = retransits.find(departCity)) == retransits.end())
        {
          // First encounter the loc, add it to the map
          retransits.insert(std::map<LocCode, int>::value_type(departCity, 0));
        }
        else
        {
          // Increment the retransit number for the loc
          pos->second++;
        }
      }
    }

    // Destination
    lastTravelSeg = travelSeg;
    LocCode arrCity =
        offMultiCity(_trx.dataHandle(), *lastTravelSeg, _itin.geoTravelType(), govCxr);
    ;
    if (arrCity != origCity && arrCity != destCity && arrCity != departCity)
    {
      if ((pos = retransits.find(arrCity)) == retransits.end())
      {
        // First encounter the loc, add it to the map
        retransits.insert(std::map<LocCode, int>::value_type(arrCity, 0));
      }
      else
      {
        // Increment the retransit number for the country
        pos->second++;
      }
    }
  }
}

bool
LimitationOnIndirectTravel::retransitViaGovCxr(const std::vector<TravelSeg*>& travelSegs,
                                               const CarrierCode& govCxr,
                                               const LocCode& retransitCity,
                                               int numRetransit)
{
  bool checkCxr = false;
  int numChecked = 0;

  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  for (; i != travelSegs.end(); i++)
  {
    AirSeg* curTvlSeg = dynamic_cast<AirSeg*>(*i);
    if (curTvlSeg == nullptr)
      continue;

    LocCode destCity = offMultiCity(_trx.dataHandle(), *curTvlSeg, _itin.geoTravelType(), govCxr);

    if (destCity == retransitCity) // TravelSeg to the retransit point
    {
      if (!checkCxr) // Start new set of travel segs between retransit points
      {
        checkCxr = true;
        continue;
      }
      else
      {
        if (curTvlSeg->carrier() != govCxr)
          return false;

        numChecked++; // End of one set of retransit travel segments

        if (numChecked >= numRetransit) // Reach the retransit times
          return true;
      }
    }
    else
    {
      if (checkCxr) // TravelSeg is between retransit points
      {
        if (curTvlSeg->carrier() != govCxr)
          return false;
      }
    }
  }

  return true;
}

bool
LimitationOnIndirectTravel::checkMaxRetransitAtIntermLoc(const std::vector<TravelSeg*>& travelSegs,
                                                         const CarrierCode& govCxr,
                                                         const LimitationCmn& lim,
                                                         DiagCollector* diag)
{
  int maxRetransit = lim.maxRetransitAllowed();
  if ((maxRetransit < 0) || (maxRetransit > MAX_RETRANSIT))
    return true;

  std::vector<TravelSeg*>::const_iterator origPos;
  originPos(travelSegs, origPos);
  if (origPos == travelSegs.end())
    return true;

  std::vector<TravelSeg*>::const_iterator destPos;
  destinationPos(travelSegs, destPos);
  if (destPos == travelSegs.end())
    return true;

  LocCode origCity = boardMultiCity(_trx.dataHandle(), **origPos, _itin.geoTravelType(), govCxr);
  LocCode destCity = offMultiCity(_trx.dataHandle(), **destPos, _itin.geoTravelType(), govCxr);

  const TravelSeg* lastTravelSeg = nullptr;

  std::map<LocCode, int> retransits;
  std::map<LocCode, int>::iterator pos;
  std::vector<TravelSeg*>::const_iterator travelSegIter = origPos;
  for (; travelSegIter != travelSegs.end(); travelSegIter++)
  {
    TravelSeg* travelSeg = *travelSegIter;

    // Origin
    LocCode departCity =
        boardMultiCity(_trx.dataHandle(), *travelSeg, _itin.geoTravelType(), govCxr);

    if (departCity != origCity && departCity != destCity)
    {
      if ((lastTravelSeg != nullptr) &&
          (departCity !=
           offMultiCity(_trx.dataHandle(), *lastTravelSeg, _itin.geoTravelType(), govCxr)) &&
          isToFromLoc(*(travelSeg->destination()), lim))
      {
        // Increase retansit number for this loc
        if ((pos = retransits.find(departCity)) == retransits.end())
        {
          // First encounter the loc, add it to the map
          retransits.insert(std::map<LocCode, int>::value_type(departCity, 0));
        }
        else
        {
          // Increment the retransit number for the loc
          pos->second++;
          if (pos->second > maxRetransit)
          {
            if (diag != nullptr)
            {
              (*diag) << "  MORE THAN " << maxRetransit << " "
                      << (maxRetransit > 1 ? "RETRANSITS" : "RETRANSIT") << " AT " << departCity
                      << "\n";
            }
            return false;
          }
        }
      }
    }

    // Destination
    lastTravelSeg = travelSeg;
    LocCode arrCity =
        offMultiCity(_trx.dataHandle(), *lastTravelSeg, _itin.geoTravelType(), govCxr);
    if (arrCity != origCity && arrCity != destCity && arrCity != departCity &&
        isToFromLoc(*(travelSeg->origin()), lim))
    {
      if ((pos = retransits.find(arrCity)) == retransits.end())
      {
        // First encounter the loc, add it to the map
        retransits.insert(std::map<LocCode, int>::value_type(arrCity, 0));
      }
      else
      {
        // Increment the retransit number for the country
        pos->second++;
        if (pos->second > maxRetransit)
        {
          if (diag != nullptr)
          {
            (*diag) << "  MORE THAN " << maxRetransit << " "
                    << (maxRetransit > 1 ? "RETRANSITS" : "RETRANSIT") << " AT " << arrCity << "\n";
          }
          return false;
        }
      }
    }
  }

  return true;
}

bool
LimitationOnIndirectTravel::checkMaxRetransitAtLoc(const std::vector<TravelSeg*>& travelSegs,
                                                   const LocTypeCode& locType,
                                                   const LocCode& locCode,
                                                   const LimitationCmn& lim,
                                                   DiagCollector* diag)
{
  int maxRetransit = lim.maxRetransitAllowed();
  if (LIKELY((maxRetransit < 0) || (maxRetransit > MAX_RETRANSIT)))
    return true;

  LocCode origCity =
      boardMultiCity(_trx.dataHandle(), *(travelSegs.front()), _itin.geoTravelType(), "");
  LocCode destCity =
      offMultiCity(_trx.dataHandle(), *(travelSegs.back()), _itin.geoTravelType(), "");

  const TravelSeg* lastTravelSeg = nullptr;
  int numRetransit = -1;
  bool isLastDestRetransit = false;

  std::vector<TravelSeg*>::const_iterator travelSegIter = travelSegs.begin();
  for (; travelSegIter != travelSegs.end(); travelSegIter++)
  {
    TravelSeg* travelSeg = *travelSegIter;

    // Origin
    LocCode departCity = boardMultiCity(_trx.dataHandle(), *travelSeg, _itin.geoTravelType(), "");
    if (departCity != origCity && departCity != destCity)
    {
      if ((lastTravelSeg != nullptr) &&
          (departCity !=
           offMultiCity(_trx.dataHandle(), *lastTravelSeg, _itin.geoTravelType(), "")))
      {
        if (LocUtil::isInLoc(*(travelSeg->origin()),
                             locType,
                             locCode,
                             Vendor::SABRE,
                             MANUAL,
                             LocUtil::OTHER,
                             GeoTravelType::International,
                             EMPTY_STRING(),
                             _trx.getRequest()->ticketingDT()))
        {
          if (!isLastDestRetransit && isToFromLoc(*(travelSeg->destination()), lim))
          {
            // Increase retansit number for this loc
            numRetransit++;
            isLastDestRetransit = true;
            if (numRetransit > maxRetransit)
            {
              if (diag != nullptr)
              {
                (*diag) << "  MORE THAN " << maxRetransit << " "
                        << (maxRetransit > 1 ? "RETRANSITS" : "RETRANSIT") << " AT LOC " << locType
                        << " " << locCode << "\n";
              }
              return false;
            }
          }
        }
        else
          isLastDestRetransit = false;
      }
    }

    // Destination
    LocCode arrCity = offMultiCity(_trx.dataHandle(), *travelSeg, _itin.geoTravelType(), "");
    if (arrCity != origCity && arrCity != destCity)
    {
      if (LocUtil::isInLoc(*(travelSeg->destination()),
                           locType,
                           locCode,
                           Vendor::SABRE,
                           MANUAL,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           _trx.getRequest()->ticketingDT()))
      {
        if (!isLastDestRetransit && isToFromLoc(*(travelSeg->origin()), lim))
        {
          // Increase retansit number for this loc
          numRetransit++;
          isLastDestRetransit = true;
          if (numRetransit > maxRetransit)
          {
            if (diag != nullptr)
            {
              (*diag) << "  MORE THAN " << maxRetransit << " "
                      << (maxRetransit > 1 ? "RETRANSITS" : "RETRANSIT") << " AT LOC " << locType
                      << " " << locCode << "\n";
            }
            return false;
          }
        }
      }
      else
        isLastDestRetransit = false;
    }
  }

  return true;
}

bool
LimitationOnIndirectTravel::checkGeneralStopOver(LimitationAppl limAppl,
                                                 const std::vector<TravelSeg*>& travelSegs,
                                                 bool reverseDirection,
                                                 const LimitationCmn& lim,
                                                 DiagCollector* diag,
                                                 bool noSideTrip,
                                                 const FareMarket* fareMarket)
{
  switch (lim.viaPointStopoverAppl())
  {
  case COUNTRY_OF_J_ORIGIN:
  {
    const Loc* orig = origin(_itin.travelSeg());
    if (orig == nullptr)
      return true;

    if (limAppl == FC) // Special check for Fare component
    {
      const LimitationFare* fcLim = dynamic_cast<const LimitationFare*>(&lim);
      if (fcLim == nullptr)
        return true;

      // Check check if "Must Be to/from Loc" field defined
      // If defined and fare component destination nation is journey origin
      // nation, do not verify this item.
      const LocKey& locKey = fcLim->notViaToFromLoc();
      if ((locKey.locType() != BLANK) &&
          (travelSegs.back()->destination()->nation() == orig->nation()))
        return true;
    }

    return checkMaxStopOverAtLoc(travelSegs, LOCTYPE_NATION, orig->nation(), lim, diag, noSideTrip);
  }

  case COUNTRY_OF_FC_ORIGIN:
    if (limAppl == FC)
    {
      const Loc* orig = (reverseDirection ? destination(travelSegs) : origin(travelSegs));
      if (orig == nullptr)
        return true;

      return checkMaxStopOverAtLoc(
          travelSegs, LOCTYPE_NATION, orig->nation(), lim, diag, noSideTrip);
    }
    break;

  case COUNTRY_OF_FC_DESTIN:
    if (limAppl == FC)
    {
      const Loc* dest = (reverseDirection ? origin(travelSegs) : destination(travelSegs));
      if (dest == nullptr)
        return true;

      return checkMaxStopOverAtLoc(
          travelSegs, LOCTYPE_NATION, dest->nation(), lim, diag, noSideTrip);
    }
    break;

  case COUNTRY_OF_PAYMENT:
  {
    const Loc* saleLoc = TrxUtil::saleLoc(_trx);
    if (saleLoc == nullptr)
      return true;

    return checkMaxStopOverAtLoc(
        travelSegs, LOCTYPE_NATION, saleLoc->nation(), lim, diag, noSideTrip);
  }

  case INTERMEDIATE_POINT_WITHIN_FC:
    if (LIKELY(limAppl == FC))
    {
      return checkMaxStopOverAtIntermLoc(travelSegs,
                                         (fareMarket != nullptr ? fareMarket->governingCarrier() : ""),
                                         lim,
                                         diag,
                                         noSideTrip);
    }
    break;

  default:
    break;
  }

  return true;
}

bool
LimitationOnIndirectTravel::checkSpecificStopOver(const std::vector<TravelSeg*>& travelSegs,
                                                  const LimitationCmn& lim,
                                                  DiagCollector* diag,
                                                  bool noSideTrip)
{
  if (LIKELY(lim.stopoverLoc().locType() == BLANK || lim.stopoverLoc().loc().empty()))
    return true;

  return checkMaxStopOverAtLoc(
      travelSegs, lim.stopoverLoc().locType(), lim.stopoverLoc().loc(), lim, diag, noSideTrip);
}

bool
LimitationOnIndirectTravel::checkMaxStopOverAtLoc(const std::vector<TravelSeg*>& travelSegs,
                                                  const LocTypeCode& locType,
                                                  const LocCode& locCode,
                                                  const LimitationCmn& lim,
                                                  DiagCollector* diag,
                                                  bool noSideTrip)
{
  int maxStopOver = lim.maxStopsAtRetransit();
  if ((maxStopOver < 0) || (maxStopOver > MAX_STOPOVER))
    return true;

  std::vector<TravelSeg*>::const_iterator origPos;
  originPos(travelSegs, origPos);
  if (origPos == travelSegs.end())
    return true;

  std::vector<TravelSeg*>::const_iterator destPos;
  destinationPos(travelSegs, destPos);
  if (destPos == travelSegs.end())
    return true;

  int numStopOver = 0;
  bool isLastSegStopOver = true;
  bool isEndSegSurface = false;
  AirSeg* endSeg = dynamic_cast<AirSeg*>(*destPos);
  if (endSeg == nullptr)
    isEndSegSurface = true;

  std::vector<TravelSeg*>::const_iterator travelSegIter = origPos;
  for (; travelSegIter != destPos; travelSegIter++)
  {
    TravelSeg* travelSeg = *travelSegIter;
    AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSeg);
    if (((airSeg == nullptr) &&
         isLastSegStopOver) || // Surface sector with last travel segment is stopover.
        ((airSeg != nullptr) &&
         // Air segment as stopover
         (isStopOver(airSeg, (noSideTrip ? nullptr : *(travelSegIter + 1))) ||
          // Air segment as connection with next segment as the end surface segment
          (isEndSegSurface && ((travelSegIter + 1) == destPos)))))
    {
      if (LocUtil::isInLoc(*(travelSeg->destination()),
                           locType,
                           locCode,
                           Vendor::SABRE,
                           MANUAL,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           _trx.getRequest()->ticketingDT()) &&
          isToFromLoc(*travelSeg, **(travelSegIter + 1), lim))
      {
        numStopOver++;
        if (numStopOver > maxStopOver)
        {
          if (diag != nullptr)
          {
            (*diag) << "  MORE THAN " << maxStopOver << " "
                    << (maxStopOver > 1 ? "STOPOVERS" : "STOPOVER") << " AT LOC " << locType << " "
                    << locCode << "\n";
          }
          return false;
        }
      }

      isLastSegStopOver = true;
    }
    else
      isLastSegStopOver = false;
  }

  return true;
}

bool
LimitationOnIndirectTravel::checkMaxStopOver(const std::vector<TravelSeg*>& travelSegs,
                                             const LimitationCmn& lim,
                                             DiagCollector* diag,
                                             bool noSideTrip)
{
  int maxStopOver = lim.maxStopsAtRetransit();
  if ((maxStopOver < 0) || (maxStopOver > MAX_STOPOVER))
    return true;

  if (LIKELY((lim.stopoverLoc().locType() != BLANK) || // General stopover specified
      ((lim.viaPointStopoverAppl() >= COUNTRY_OF_J_ORIGIN) && // Specific stopover specified
       (lim.viaPointStopoverAppl() <= INTERMEDIATE_POINT_WITHIN_FC))))
    return true;

  std::vector<TravelSeg*>::const_iterator origPos;
  originPos(travelSegs, origPos);
  if (origPos == travelSegs.end())
    return true;

  std::vector<TravelSeg*>::const_iterator destPos;
  destinationPos(travelSegs, destPos);
  if (destPos == travelSegs.end())
    return true;

  int numStopOver = 0;
  bool isLastSegStopOver = true;
  bool isEndSegSurface = false;
  AirSeg* endSeg = dynamic_cast<AirSeg*>(*destPos);
  if (endSeg == nullptr)
    isEndSegSurface = true;

  std::vector<TravelSeg*>::const_iterator travelSegIter = origPos;
  for (; travelSegIter != destPos; travelSegIter++)
  {
    TravelSeg* travelSeg = *travelSegIter;
    AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSeg);
    if (((airSeg == nullptr) &&
         isLastSegStopOver) || // Surface sector with last travel segment is stopover.
        ((airSeg != nullptr) &&
         // Air segment as stopover
         (isStopOver(airSeg, (noSideTrip ? nullptr : *(travelSegIter + 1))) ||
          // Air segment as connection with next segment as the end surface segment
          (isEndSegSurface && ((travelSegIter + 1) == destPos)))))
    {
      numStopOver++;
      isLastSegStopOver = true;
    }
    else
      isLastSegStopOver = false;
  }

  if (numStopOver > maxStopOver)
  {
    if (diag != nullptr)
      (*diag) << "  MORE THAN " << maxStopOver << " "
              << (maxStopOver > 1 ? "STOPOVERS" : "STOPOVER") << "\n";
    return false;
  }

  return true;
}

bool
LimitationOnIndirectTravel::checkMaxStopOverAtIntermLoc(const std::vector<TravelSeg*>& travelSegs,
                                                        const CarrierCode& govCxr,
                                                        const LimitationCmn& lim,
                                                        DiagCollector* diag,
                                                        bool noSideTrip)
{
  int maxStopOver = lim.maxStopsAtRetransit();
  if (UNLIKELY((maxStopOver < 0) || (maxStopOver > MAX_STOPOVER)))
    return true;

  std::vector<TravelSeg*>::const_iterator origPos;
  originPos(travelSegs, origPos);
  if (UNLIKELY(origPos == travelSegs.end()))
    return true;

  std::vector<TravelSeg*>::const_iterator destPos;
  destinationPos(travelSegs, destPos);
  if (UNLIKELY(destPos == travelSegs.end()))
    return true;

  std::map<LocCode, int> stopOvers;
  std::map<LocCode, int>::iterator pos;
  bool isLastSegStopOver = true;
  bool isEndSegSurface = false;
  AirSeg* endSeg = dynamic_cast<AirSeg*>(*destPos);
  if (endSeg == nullptr)
    isEndSegSurface = true;

  std::vector<TravelSeg*>::const_iterator travelSegIter = origPos;
  for (; travelSegIter != destPos; travelSegIter++)
  {
    TravelSeg* travelSeg = *travelSegIter;

    AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSeg);
    if (((airSeg == nullptr) &&
         isLastSegStopOver) || // Surface sector with last travel segment is stopover.
        ((airSeg != nullptr) &&
         // Air segment as stopover
         (isStopOver(airSeg, (noSideTrip ? nullptr : *(travelSegIter + 1))) ||
          // Air segment as connection with next segment as the end surface segment
          (isEndSegSurface && ((travelSegIter + 1) == destPos)))))
    {
      // Destination
      LocCode arrCity = offMultiCity(_trx.dataHandle(), *travelSeg, _itin.geoTravelType(), govCxr);

      if (LIKELY(isToFromLoc(*(travelSeg->origin()), lim) ||
          isToFromLoc(*((*(travelSegIter + 1))->destination()), lim)))
      {
        if (LIKELY((pos = stopOvers.find(arrCity)) == stopOvers.end()))
        {
          if (UNLIKELY(maxStopOver == 0))
          {
            if (diag != nullptr)
              (*diag) << "  MORE THAN " << maxStopOver << " STOPOVER AT " << arrCity << "\n";
            return false;
          }

          // First encounter the loc, add it to the map
          stopOvers.insert(std::map<LocCode, int>::value_type(arrCity, 1));
        }
        else
        {
          // Increment the retransit number for the country
          pos->second++;
          if (pos->second > maxStopOver)
          {
            if (diag != nullptr)
              (*diag) << "  MORE THAN " << maxStopOver << " "
                      << (maxStopOver > 1 ? "STOPOVERS" : "STOPOVER") << " AT " << arrCity << "\n";
            return false;
          }
        }
      }

      isLastSegStopOver = true;
    }
    else
    {
      isLastSegStopOver = false;
    }
  }
  return true;
}

void
LimitationOnIndirectTravel::displayFare(const FareUsage& fareUsage, DiagCollector& diag)
{
  const PaxTypeFare& paxTypeFare = *fareUsage.paxTypeFare(); // lint !e530
  diag << paxTypeFare.origin() << "-" << paxTypeFare.destination() << " " << paxTypeFare.carrier()
       << " ";

  std::string gd;
  globalDirectionToStr(gd, paxTypeFare.globalDirection());

  diag << gd << " " << paxTypeFare.fcaFareType() << " " << (paxTypeFare.isRouting() ? "R" : "M")
       << " ";
  diag << std::setw(7); // std::setw in one line only!
  diag << paxTypeFare.routingNumber() << " NUC";
  diag.setf(std::ios::fixed, std::ios::floatfield);
  diag.precision(2);
  diag << std::setw(9);
  diag << paxTypeFare.nucFareAmount();

  MoneyAmount hipPlusUp = fareUsage.minFarePlusUp().getSum(HIP);
  if (hipPlusUp > 0)
  {
    diag << " HIP ";
    diag.setf(std::ios::fixed, std::ios::floatfield);
    diag.precision(2);
    diag << std::setw(9);
    diag << hipPlusUp;
  }

  diag << " \n";
}

const std::vector<SurfaceSectorExemptionInfo*>&
LimitationOnIndirectTravel::getSurfaceSectorExemptionInfo()
{
  DataHandle& dataHandle = _trx.dataHandle();
  return dataHandle.getSurfaceSectorExemptionInfo(_itin.validatingCarrier(),
                                                  dataHandle.ticketDate());
}

const std::vector<SurfaceSectorExemptionInfo*>&
LimitationOnIndirectTravel::getSurfaceSectorExemptionInfo(const CarrierCode& cxr)
{
  DataHandle& dataHandle = _trx.dataHandle();
  return dataHandle.getSurfaceSectorExemptionInfo(cxr, dataHandle.ticketDate());
}

bool
LimitationOnIndirectTravel::validateCRSOnIndirectTravel(const SurfaceSectorExemptionInfo* info,
                                                        const std::string& crs)
{
  if (info->userAppl().empty())
    return true;
  else
    return (info->crs() == crs);
}

bool
LimitationOnIndirectTravel::validatePOSOnIndirectTravel(const SurfaceSectorExemptionInfo* info,
                                                        const LocCode& loc)
{
  bool isThere = LocUtil::isInLoc(loc,
                                  info->posLocType(),
                                  info->posLoc(),
                                  Vendor::ATPCO,
                                  MANUAL,
                                  GeoTravelType::International,
                                  LocUtil::OTHER,
                                  _trx.getRequest()->ticketingDT());

  return info->posLocException() == 'Y' ? !isThere : isThere;
}

bool
LimitationOnIndirectTravel::validateLocOnIndirectTravel(Indicator locException,
                                                        LocTypeCode locTypeCode,
                                                        const LocCode& locCode,
                                                        const LocCode& city)
{
  bool isThere = LocUtil::isInLoc(city,
                                  locTypeCode,
                                  locCode,
                                  Vendor::ATPCO,
                                  MANUAL,
                                  GeoTravelType::International,
                                  LocUtil::OTHER,
                                  _trx.getRequest()->ticketingDT());

  return locException == 'Y' ? !isThere : isThere;
}

bool
LimitationOnIndirectTravel::validateOperCxrOnIndirectTravel(const std::set<CarrierCode>& carriers,
                                                            const Indicator exception)
{
  if (_trx.noPNRPricing())
    return true;

  AirSeg* airSeg;
  std::vector<TravelSeg*>::const_iterator iter = _itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = _itin.travelSeg().end();
  std::set<CarrierCode>::const_iterator iterCxrRes;
  std::set<CarrierCode>::const_iterator iterCxrEnd = carriers.end();

  while (iter != iterEnd)
  {
    TravelSeg* tvlSeg = (*iter);

    if (tvlSeg->segmentType() == Open)
    {
      ++iter;
      continue;
    }

    if ((airSeg = dynamic_cast<AirSeg*>(tvlSeg)) != nullptr)
    {
      iterCxrRes = carriers.find(airSeg->operatingCarrierCode());

      if (exception == 'Y')
      {
        if (iterCxrRes != iterCxrEnd)
        {
          return false;
        }
      }
      else
      {
        if (carriers.empty())
        {
          return true;
        }

        if (iterCxrRes == iterCxrEnd)
        {
          return false;
        }
      }
    }

    ++iter;
  }

  return true;
}

bool
LimitationOnIndirectTravel::validateMktgCxrOnIndirectTravel(const std::set<CarrierCode>& carriers,
                                                            const Indicator exception)
{
  AirSeg* airSeg;
  std::vector<TravelSeg*>::const_iterator iter = _itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = _itin.travelSeg().end();
  std::set<CarrierCode>::const_iterator iterCxrRes;
  std::set<CarrierCode>::const_iterator iterCxrEnd = carriers.end();

  while (iter != iterEnd)
  {
    if ((airSeg = dynamic_cast<AirSeg*>(*iter)) != nullptr)
    {
      iterCxrRes = carriers.find(airSeg->marketingCarrierCode());

      if (exception == 'Y')
      {
        if (iterCxrRes != iterCxrEnd)
        {
          return false;
        }
      }
      else
      {
        if (carriers.empty())
        {
          return true;
        }

        if (iterCxrRes == iterCxrEnd)
        {
          return false;
        }
      }
    }

    ++iter;
  }

  return true;
}

bool
LimitationOnIndirectTravel::validatePTCOnIndirectTravel(const SurfaceSectorExemptionInfo* info)
{
  std::vector<PaxType*>::iterator iter = _trx.paxType().begin();
  std::vector<PaxType*>::iterator iterEnd = _trx.paxType().end();
  std::set<PaxTypeCode>::const_iterator iterPTCRes;
  std::set<PaxTypeCode>::const_iterator iterPTCEnd = info->paxTypes().end();

  bool mapAdtToNeg = false;
  if (_trx.getRequest()->ticketingAgent()->abacusUser() ||
      _trx.getRequest()->ticketingAgent()->infiniUser())
  {

    if (_trx.noPNRPricing())
    {
      const NoPNRPricingTrx* noPNRPricingTrx = static_cast<const NoPNRPricingTrx*>(&_trx);
      const NoPNROptions* wqcc = noPNRPricingTrx->noPNROptions();

      if (wqcc->negPassengerTypeMapping() == '1')
      {
        mapAdtToNeg = true;
      }
    }
    else if (_trx.getTrxType() == PricingTrx::PRICING_TRX)
    {
      const FareCalcConfig* fcc = _trx.fareCalcConfig();

      if (fcc->negPermitted() == '2')
      {
        mapAdtToNeg = true;
      }
    }
  }

  while (iter != iterEnd)
  {
    PaxType* paxType = *iter;
    iterPTCRes = info->paxTypes().find(paxType->paxType());

    if (mapAdtToNeg && paxType->paxType() == ADULT)
    {
      if (iterPTCRes == iterPTCEnd)
      {
        iterPTCRes = info->paxTypes().find(NEG);
      }
    }

    if (info->exceptPassengersTypes() == 'Y')
    {
      if (iterPTCRes != iterPTCEnd)
      {
        return false;
      }
    }
    else
    {
      if (info->paxTypes().empty())
      {
        return true;
      }

      if (iterPTCRes == iterPTCEnd)
      {
        return false;
      }
    }

    ++iter;
  }

  return true;
}

bool
LimitationOnIndirectTravel::validateIntlSurfaceTravel(FarePath& farePath)
{
  // Validate Journey International Surface Travel restriction
  // after Minimum Fare check is done.
  bool internationalItin = true;
  const GeoTravelType& geoTrvlType = _itin.geoTravelType();
  if (geoTrvlType == GeoTravelType::Domestic || geoTrvlType == GeoTravelType::ForeignDomestic)
    internationalItin = false;

  DiagCollector* diag = nullptr;
  if (UNLIKELY(_diagEnabled && (_trx.diagnostic().diagnosticType() == Diagnostic370)))
  {
    DCFactory* factory = DCFactory::instance();
    diag = factory->create(_trx);
    if (diag != nullptr)
    {
      diag->enable(Diagnostic370);

      (*diag) << " \n---------------------------------------------------------------\n"
              << "INTL SURFACE TRAVEL VALIDATION \n";

      if (_trx.getOptions() && _trx.getOptions()->isRtw())
      {
        (*diag) << "RTW REQUEST IS NOT SUBJECT TO INTL SURFACE TRAVEL\n";
        (*diag) << "          LIMITATIONS VALIDATION\n";
        diag->flushMsg();
      }
    }
  }

  if (UNLIKELY(_trx.getOptions() && _trx.getOptions()->isRtw()))
  {
    LOG4CXX_INFO(logger, "RTW Limitation bypass validateIntlSurfaceTravel");
    return false;
  }

  if (!internationalItin)
  {
    if (UNLIKELY(diag != nullptr))
    {
      (*diag) << " \n  ITIN NOT INTERNATIONAL NOT SUBJECT TO LIMITATIONS VALIDATION\n";
      diag->flushMsg();
    }
    return false;
  }

  // Only check all Normal Fare itin.
  const std::vector<PricingUnit*>& pus = farePath.pricingUnit();
  std::vector<PricingUnit*>::const_iterator puIter = pus.begin();
  for (; puIter != pus.end(); puIter++)
  {
    if ((**puIter).puFareType() != PricingUnit::NL)
    {
      if (UNLIKELY(diag != nullptr))
      {
        (*diag) << " \n  FARE PATH CONTAINS SPECIAL FARE\n"
                << " \n  INTL SURFACE TRAVEL RESTRICTION NOT APPLY\n";

        diag->flushMsg();
      }

      return false;
    }
  }

  // Find 1st intl surface break between fare components
  LocCode surfBoardPoint;
  LocCode surfOffPoint;
  int endSegOrder = -1;
  const Itin& itin = *farePath.itin(); // lint !e530
  if (!find1stIntlSurface(itin, pus, surfBoardPoint, surfOffPoint, endSegOrder, diag))
  {
    if (diag != nullptr)
    {
      (*diag) << " \n  INTL SURFACE TRAVEL RESTRICTION NOT APPLY\n";

      diag->flushMsg();
    }

    return false;
  }

  // Check TPM Surface Sector Exempt
  if (hasTpmSurfExempt(surfBoardPoint, surfOffPoint))
  {
    if (diag != nullptr)
    {
      (*diag) << " \n  EXEMPTED BY TPM SURFACE SECTOR EXEMPT TABLE\n"
              << " \n  INTL SURFACE TRAVEL RESTRICTION NOT APPLY\n";

      diag->flushMsg();
    }

    return false;
  }

  // Calculate TPM for surface sector
  if (diag != nullptr)
    (*diag) << " \n  SURFACE TPM: \n";

  TravelSeg* surfaceSector = _itin.travelSeg()[endSegOrder - 1];
  const uint32_t surfTpm = getMileage(surfaceSector, diag);

  // Calculate TPM from itin orig to the surf board point
  if (diag != nullptr)
    (*diag) << " \n  SECTOR TPM FROM ITIN ORIG TO SURFACE TRAVEL BOARD POINT:\n";

  uint32_t sectorsTpm = 0;
  const std::vector<TravelSeg*>& travelSegs = _itin.travelSeg();
  std::vector<TravelSeg*>::const_iterator iter = travelSegs.begin();
  for (; iter != travelSegs.end(); iter++)
  {
    TravelSeg* curTvlSeg = *iter;

    if (itin.segmentOrder(curTvlSeg) >= endSegOrder)
      break;

    sectorsTpm += getMileage(curTvlSeg, diag);
  }

  if (diag != nullptr)
  {
    (*diag) << " \n  SURFACE TPM      - " << surfTpm << "\n"
            << "  TOTAL SECTOR TPM - " << sectorsTpm << "\n";
  }

  if (surfTpm > sectorsTpm)
  {
    int seq = -1;

    farePath.ignoreSurfaceTPM() = true;

    seq = ignoreIntlSurfaceTravelRestrictionVC(farePath, surfBoardPoint, surfOffPoint);

    if (seq != -1)
    {
      if (diag != nullptr)
      {
        (*diag) << " \n  SURFACE TPM GREATER THAN TOTAL SECTOR TPM\n"
                << " \n  INTL SURFACE TRAVEL RESTRICTION MAY APPLY - SKIPPED\n";
        (*diag) << "TPM SURFACE RESTRICTION EXEMPTION APPLIES - CHECK DIAGNOSTIC 372 - "
                << "FOR MATCH SEQUENCE";

        diag->flushMsg();
      }
      return false;
    }
    else
    {
      if (diag != nullptr)
      {
        (*diag) << " \n  SURFACE TPM GREATER THAN TOTAL SECTOR TPM\n"
                << " \n  INTL SURFACE TRAVEL RESTRICTION MAY APPLY\n";

        diag->flushMsg();
      }

      return true; // Intl Surface Travel Restricted
    }
  }

  if (diag != nullptr)
  {
    (*diag) << " \n  INTL SURFACE TRAVEL RESTRICTION NOT APPLY\n";

    diag->flushMsg();
  }

  return false;
}

bool
LimitationOnIndirectTravel::find1stIntlSurface(const Itin& itin,
                                               const std::vector<PricingUnit*>& pus,
                                               LocCode& surfBoardPoint,
                                               LocCode& surfOffPoint,
                                               int& endSegOrder,
                                               DiagCollector* diag)
{
  const Loc* lastDest = nullptr;
  LocCode lastDestCity;
  int lastSegOrder = -1;
  bool lastSegArnk = false;
  const std::vector<TravelSeg*>& travelSegs = _itin.travelSeg();
  std::vector<TravelSeg*>::const_iterator iter = travelSegs.begin();
  for (; iter != travelSegs.end(); iter++)
  {
    AirSeg* curTrvlSeg = dynamic_cast<AirSeg*>(*iter);
    if (curTrvlSeg == nullptr)
    {
      lastSegOrder = itin.segmentOrder(*iter);
      lastSegArnk = true;
      if (diag != nullptr)
        (*diag) << "  SURFACE\n";

      continue;
    }

    const Loc* orig = curTrvlSeg->origin();
    const Loc* dest = curTrvlSeg->destination();
    if (diag != nullptr)
    {
      (*diag) << "  " << orig->loc() << "-" << curTrvlSeg->carrier() << "-" << dest->loc() << "\n";
    }

    LocCode origCity = boardMultiCity(_trx.dataHandle(), *curTrvlSeg, _itin.geoTravelType(), "");
    LocCode destCity = offMultiCity(_trx.dataHandle(), *curTrvlSeg, _itin.geoTravelType(), "");
    if (lastDest == nullptr || origCity == lastDestCity)
    {
      // Consecutive travel
      lastDestCity = destCity;
      lastDest = dest;
      lastSegOrder = itin.segmentOrder(curTrvlSeg);
      lastSegArnk = false;
      continue;
    }
    else
    {
      // Surface Travel
      if (lastDest->nation() == orig->nation())
      {
        lastDestCity = destCity;
        lastDest = dest;
        lastSegOrder = itin.segmentOrder(curTrvlSeg);
        lastSegArnk = false;
        continue; // Domestic surface travel
      }

      if (LocUtil::isDomesticUSCA(*lastDest) && LocUtil::isDomesticUSCA(*orig))
      {
        // 1st Surface Travel is between US and CA
        if (diag != nullptr)
        {
          (*diag) << " \n  SURFACE TRAVEL " << lastDest->loc() << "-" << orig->loc()
                  << " BETWEEN US AND CA\n";
        }

        return false;
      }

      if (diag != nullptr)
      {
        (*diag) << " \n  FARE COMPONENTS: \n";
      }

      // Found the 1st Intl Surface Travel
      // Check if it is between Fare Components
      std::vector<PricingUnit*>::const_iterator puIter = pus.begin();
      for (; puIter != pus.end(); puIter++)
      {
        PricingUnit& curPu = **puIter;
        const std::vector<FareUsage*>& fus = curPu.fareUsage();
        std::vector<FareUsage*>::const_iterator fuIter = fus.begin();
        for (; fuIter != fus.end(); fuIter++)
        {
          // Check if the surface travel is within a FC
          FareUsage& curFu = **fuIter;
          PaxTypeFare& curPaxTypeFare = *(curFu.paxTypeFare());
          FareMarket& curFm = *(curPaxTypeFare.fareMarket());

          if (diag != nullptr)
          {
            (*diag) << "    " << curFm.origin()->loc() << "-" << curFm.destination()->loc() << "\n";
          }

          int curFmStartSegOrder = itin.segmentOrder(curFm.travelSeg().front());
          int curFmEndSegOrder = itin.segmentOrder(curFm.travelSeg().back());

          if ((lastSegOrder >= curFmStartSegOrder && lastSegOrder <= curFmEndSegOrder &&
               itin.segmentOrder(curTrvlSeg) >= curFmStartSegOrder &&
               itin.segmentOrder(curTrvlSeg) <= curFmEndSegOrder) ||
              (lastSegArnk && lastSegOrder >= curFmStartSegOrder &&
               lastSegOrder <= curFmEndSegOrder))

          {
            if (diag != nullptr)
            {
              (*diag) << " \n  INTL SURFACE TRAVEL " << lastDest->loc() << "-" << orig->loc()
                      << " WITHIN FARE COMPONENT " << curFm.origin()->loc() << "-"
                      << curFm.destination()->loc() << "\n";
            }

            return false;
          }
        }
      }

      surfBoardPoint = lastDestCity;
      surfOffPoint = origCity;
      endSegOrder = lastSegOrder;

      if (diag != nullptr)
      {
        (*diag) << " \n  INTL SURFACE TRAVEL " << surfBoardPoint << "-" << surfOffPoint
                << " BETWEEN FARE COMPONENTS\n";
      }

      return true;
    }
  }

  // Intl Surface Travel not found
  if (diag != nullptr)
  {
    (*diag) << " \n  NO INTL SURFACE TRAVEL\n";
  }

  return false;
}

bool
LimitationOnIndirectTravel::hasTpmSurfExempt(const LocCode& city1, const LocCode& city2)
{
  if (_trx.dataHandle().getSurfaceSectorExempt(city1, city2, _travelCommenceDT) != nullptr)
    return true;

  return false;
}

uint32_t
LimitationOnIndirectTravel::getMileage(TravelSeg* surfaceSeg, DiagCollector* diag)
{
  if (surfaceSeg == nullptr)
    return 0;

  // Find GlobalDirection
  std::vector<TravelSeg*> travelSegs;
  travelSegs.push_back(surfaceSeg);

  GlobalDirection gd;
  GlobalDirectionFinderV2Adapter::getGlobalDirection(&_trx, _itin.travelDate(), travelSegs, gd);

  LocCode loc1City = boardMultiCity(_trx.dataHandle(), *surfaceSeg, _itin.geoTravelType(), "");
  LocCode loc2City = offMultiCity(_trx.dataHandle(), *surfaceSeg, _itin.geoTravelType(), "");

  // Get TPM
  const Mileage* mileage =
      _trx.dataHandle().getMileage(loc1City, loc2City, TPM, gd, _travelCommenceDT);
  if (mileage != nullptr)
  {
    if (diag != nullptr)
    {
      (*diag) << "    " << loc1City << "-" << loc2City << " TPM " << mileage->mileage() << "\n";
    }
    return mileage->mileage();
  }

  // Find Mileage Substitue City
  const MileageSubstitution* subLoc1 =
      _trx.dataHandle().getMileageSubstitution(loc1City, _travelCommenceDT);
  const MileageSubstitution* subLoc2 =
      _trx.dataHandle().getMileageSubstitution(loc2City, _travelCommenceDT);

  if (subLoc1 != nullptr && subLoc2 != nullptr)
  {
    mileage = _trx.dataHandle().getMileage(
        subLoc1->publishedLoc(), subLoc2->publishedLoc(), TPM, gd, _travelCommenceDT);
    if (mileage != nullptr)
    {
      if (diag != nullptr)
      {
        (*diag) << "    " << subLoc1->publishedLoc() << "-" << subLoc2->publishedLoc() << " TPM "
                << mileage->mileage() << "\n";
      }
      return mileage->mileage();
    }
  }

  // Find Multi-Transport Substitution
  const LocCode trans1City = _trx.dataHandle().getMultiTransportCity(loc1City);
  const LocCode trans2City = _trx.dataHandle().getMultiTransportCity(loc2City);

  if (!trans1City.empty() && !trans2City.empty())
  {
    mileage = _trx.dataHandle().getMileage(trans1City, trans2City, TPM, gd, _travelCommenceDT);
    if (mileage != nullptr)
    {
      if (diag != nullptr)
      {
        (*diag) << "    " << trans1City << "-" << trans2City << " TPM " << mileage->mileage()
                << "\n";
      }
      return mileage->mileage();
    }
  }

  // Get MPM
  mileage = _trx.dataHandle().getMileage(loc1City, loc2City, MPM, gd, _travelCommenceDT);
  if (mileage != nullptr)
  {
    if (diag != nullptr)
    {
      (*diag) << "    " << loc1City << "-" << loc2City << " MPM " << mileage->mileage()
              << " CONSTRUCTED TPM " << mileage->mileage() << "/" << TseUtil::TPM_TO_MPM_RATIO
              << "\n";
    }
    return TseUtil::getTPMFromMPM(mileage->mileage());
  }

  if (!trans1City.empty() && !trans2City.empty())
  {
    mileage = _trx.dataHandle().getMileage(trans1City, trans2City, MPM, gd, _travelCommenceDT);
    if (mileage != nullptr)
    {
      if (diag != nullptr)
      {
        (*diag) << "    " << trans1City << "-" << trans2City << " MPM " << mileage->mileage()
                << " CONSTRUCTED TPM " << mileage->mileage() << "/" << TseUtil::TPM_TO_MPM_RATIO
                << "\n";
      }
      return TseUtil::getTPMFromMPM(mileage->mileage());
    }
  }

  // Get Greater Circle Mileage
  const Loc* cityLoc1 = _trx.dataHandle().getLoc(loc1City, _travelCommenceDT);
  const Loc* cityLoc2 = _trx.dataHandle().getLoc(loc2City, _travelCommenceDT);

  const uint32_t gcm = TseUtil::greatCircleMiles(*cityLoc1, *cityLoc2);

  if (diag != nullptr)
  {
    (*diag) << "    " << loc1City << "-" << loc2City << " GCM " << gcm << "\n";
  }

  return gcm;
}

bool
LimitationOnIndirectTravel::isStopOver(const TravelSeg* travelSeg, const TravelSeg* nextTravelSeg)
{
  if (UNLIKELY(!travelSeg))
    return false;

  if (UNLIKELY(travelSeg->isForcedStopOver()))
    return true;

  if (UNLIKELY(travelSeg->isForcedConx()))
    return false;

  if (nextTravelSeg)
    return nextTravelSeg->isStopOver(travelSeg, _itin.geoTravelType());
  else
    return travelSeg->stopOver();
}

const LocCode
LimitationOnIndirectTravel::boardMultiCity(DataHandle& dataHandle,
                                           const TravelSeg& travelSeg,
                                           const GeoTravelType& itinTravelType,
                                           const CarrierCode& govCxr)
{
  if (UNLIKELY(travelSeg.boardMultiCity().empty())) // PricingModelMap sets city by flight carrier not
                                          // governing carrier.
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(&travelSeg);
    const std::vector<tse::MultiTransport*>& mtcList = dataHandle.getMultiTransportCity(
        travelSeg.origAirport(),
        ((airSeg != nullptr && govCxr.empty()) ? airSeg->carrier() : govCxr),
        itinTravelType,
        travelSeg.departureDT());

    if (!mtcList.empty())
      return mtcList.front()->multitranscity();
  }

  return travelSeg.boardMultiCity();
}

const LocCode
LimitationOnIndirectTravel::offMultiCity(DataHandle& dataHandle,
                                         const TravelSeg& travelSeg,
                                         const GeoTravelType& itinTravelType,
                                         const CarrierCode& govCxr)
{
  if (UNLIKELY(travelSeg.offMultiCity().empty())) // PricingModelMap sets city by flight carrier not governing
                                        // carrier.
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(&travelSeg);

    const std::vector<tse::MultiTransport*>& mtcList = dataHandle.getMultiTransportCity(
        travelSeg.destAirport(),
        ((airSeg != nullptr && govCxr.empty()) ? airSeg->carrier() : govCxr),
        itinTravelType,
        travelSeg.departureDT());

    if (!mtcList.empty())
      return mtcList.front()->multitranscity();
  }

  return travelSeg.offMultiCity();
}

DiagCollector*
LimitationOnIndirectTravel::get370() const
{
  DiagCollector* diag = nullptr;

  if (UNLIKELY(_diagEnabled && (_trx.diagnostic().diagnosticType() == Diagnostic370)))
  {
    DCFactory* factory = DCFactory::instance();
    diag = factory->create(_trx);
    if (diag != nullptr)
      diag->enable(Diagnostic370);
  }

  return diag;
}
void
LimitationOnIndirectTravel::end370(DiagCollector& diag) const
{
  diag.flushMsg();
}

// Validating Carrier project

int
LimitationOnIndirectTravel::ignoreIntlSurfaceTravelRestrictionVC(FarePath& farePath,
                                                                 const LocCode locCode1,
                                                                 const LocCode locCode2)
{
   if(farePath.validatingCarriers().empty() || !_trx.isValidatingCxrGsaApplicable())
   {
      ignoreIntlSurfaceTravelRestriction(farePath, locCode1, locCode2, _itin.validatingCarrier());
   }
   else
   {
      std::vector<CarrierCode>::const_iterator valCxrI=farePath.validatingCarriers().begin();
      std::vector<CarrierCode>::const_iterator valCxrE=farePath.validatingCarriers().end();

      std::vector<CarrierCode> cxrToRemove;
      bool diagCxrFound = false;

      for(; valCxrI != valCxrE; ++valCxrI )
      {
         CarrierCode cxr = *valCxrI;
         if(!defineDiagCxrRequested(cxr, diagCxrFound))
            continue;
         ignoreIntlSurfaceTravelRestriction(farePath, locCode1, locCode2, cxr);

         if(_diagSQFail)
           continue;
         if(_valCxrResult == -1)
           cxrToRemove.push_back(cxr);
      }
      if(!checkDiagReqCxr(diagCxrFound))
        return -1;

      if(!anyCxrPass(farePath, cxrToRemove))
         return -1;
      else
         _valCxrResult = 1;
   }
   return _valCxrResult;
}

int
LimitationOnIndirectTravel::ignoreIntlSurfaceTravelRestriction(FarePath& farePath,
                                                               const LocCode locCode1,
                                                               const LocCode locCode2,
                                                               const CarrierCode& cxr)
{
  std::string crs = getCrs();
  setDiag372InRequest();

  bool validationResult = false;
  std::string failedOn = "";
  initializeAttrib();

  const std::vector<SurfaceSectorExemptionInfo*>& exemptions = getSurfaceSectorExemptionInfo(cxr);

  int exemptionsSize =
      static_cast<int>(count_if(exemptions.begin(), exemptions.end(), isCrsEqual(crs)));

  Diag372Collector::DiagStream diagStr;

  if(!defineDiagSQinRequest(cxr, diagStr, exemptionsSize, !exemptions.empty()))
    return _valCxrResult;

  int count = 1;
  std::vector<SurfaceSectorExemptionInfo*>::const_iterator iter = exemptions.begin();
  std::vector<SurfaceSectorExemptionInfo*>::const_iterator iterEnd = exemptions.end();
  while (iter != iterEnd)
  {
    validationResult = true;
    SurfaceSectorExemptionInfo* info = *iter;
    bool diagSeq = _diag && (_diagSeqNumber == -1 || _diagSeqNumber == count);

    if (crs == info->crs() || info->userAppl().empty())
    {
      if (diagSeq)
      {
        diagStr.addSectorExemptionInfo(count, *info);
      }
    }
    else if (_diag)
    {
      ++iter;
      continue;
    }

    // Validation of SurfaceSectorExemptionInfo - Begin
    validateSurfaceSectorExemption(info, locCode1, locCode2, crs, validationResult, failedOn);
    // Validation of SurfaceSectorExemptionInfo - End

    if (diagSeq)
      displayValitionResult(diagStr, validationResult, failedOn);

    if (validationResult)
    {
      _valCxrResult = count;
      if (!_diag)
        break;
    }
    ++count;
    ++iter;
  }
  prepareDiag372OutPut(farePath, crs, diagStr);

  return _valCxrResult;
}

std::string
LimitationOnIndirectTravel::getCrs()
{
  Agent* agent = _trx.getRequest()->ticketingAgent();

  std::string crs = agent->vendorCrsCode();
  if (crs.empty() && agent->hostCarrier().empty())
  {
     crs = agent->cxrCode();
  }
  return crs;
}

bool
LimitationOnIndirectTravel::defineDiagCxrRequested(CarrierCode& cxr, bool &diagCxrFound)
{
   if (_diag)
   {
      std::string diagCxr = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
      if (diagCxr.empty())
        return true;
      if (!diagCxr.empty() && diagCxr != cxr)
        return false;
      else
        diagCxrFound = true;
   }
   return true;
}

void
LimitationOnIndirectTravel::setDiag372InRequest()
{
   _diag = _diagEnabled && (_trx.diagnostic().diagnosticType() == Diagnostic372);
}

bool
LimitationOnIndirectTravel::defineDiagSQinRequest(const CarrierCode& cxr,
                                                 Diag372Collector::DiagStream& diagStr,
                                                 const int& exemptionsSize, bool seqFound)
{
   if (_diag)
   {
      const std::string& diagSeqStr = _trx.diagnostic().diagParamMapItem("SQ");
      if (!diagSeqStr.empty())
        _diagSeqNumber = atoi(diagSeqStr.c_str());

      if (_diagSeqNumber > exemptionsSize)
      {
        DiagManager diagManager(_trx, Diagnostic372);
        diagManager << "INCORRECT PARAMETER FOR CARRIER " << cxr << "\n";

        _trx.diagnostic().deActivate();
        _diagSQFail = true;
        return false;
      }
      diagStr << "***************** VALIDATING CARRIER " << cxr << " **********************" << std::endl;
      if(!seqFound)
        diagStr << "*************** NO SEQUENCE DETAIL FOUND **********************" << std::endl;
      else
        diagStr << "******************** SEQUENCE DETAIL **************************" << std::endl;
   }
   return true;
}

void
LimitationOnIndirectTravel::validateSurfaceSectorExemption(
                                        const SurfaceSectorExemptionInfo* info,
                                        const LocCode locCode1,
                                        const LocCode locCode2,
                                        const std::string& crs,
                                        bool& validationResult,
                                        std::string& failedOn)
{
   const Loc* posLoc = TrxUtil::saleLoc(_trx);
   if (!validateCRSOnIndirectTravel(info, crs))
   {
      validationResult = false;

      if (_diag)
      {
        failedOn = "CRS";
      }
   }
   if (validationResult && !validatePOSOnIndirectTravel(info, posLoc->loc()))
   {
     validationResult = false;

     if (_diag)
     {
       failedOn = "POS";
     }
   }
   if (validationResult)
   {
     bool loc1in = false;
     bool loc1out = false;
     bool loc2in = false;
     bool loc2out = false;

     if (validateLocOnIndirectTravel(
             info->locException(), info->loc1Type(), info->loc1(), locCode1))
     {
       loc1out = true;
     }
     if (validateLocOnIndirectTravel(
             info->locException(), info->loc1Type(), info->loc1(), locCode2))
     {
       loc1in = true;
     }
     if (validateLocOnIndirectTravel(
              info->locException(), info->loc2Type(), info->loc2(), locCode1))
     {
       loc2out = true;
     }
     if (validateLocOnIndirectTravel(
              info->locException(), info->loc2Type(), info->loc2(), locCode2))
     {
       loc2in = true;
     }

     if ((!loc1out || !loc2in) && (!loc2out || !loc1in))
     {
        validationResult = false;

        if (_diag)
        {
          failedOn = "LOC";
        }
     }
   }
   if (validationResult && !validateMktgCxrOnIndirectTravel(info->marketingCarriers(),
                                                            info->exceptMarketingCarriers()))
   {
     validationResult = false;

     if (_diag)
     {
       failedOn = "MARKETING CARRIER";
     }
   }
   if (validationResult && !validateOperCxrOnIndirectTravel(info->operatingCarriers(),
                                                            info->exceptOperatingCarriers()))
   {
     validationResult = false;

     if (_diag)
     {
       failedOn = "OPERATING CARRIER";
     }
   }
   if (validationResult && !validatePTCOnIndirectTravel(info))
   {
     validationResult = false;

     if (_diag)
     {
       failedOn = "PTC";
     }
   }
}

void
LimitationOnIndirectTravel::displayValitionResult(Diag372Collector::DiagStream& diagStr,
                                                  const bool& validationResult,
                                                  const std::string& failedOn)
{
   diagStr << "VALIDATION RESULT: ";
   if (validationResult)
      diagStr << "MATCH " << std::endl;
   else
      diagStr << "FAIL - " << failedOn << std::endl;
   diagStr << "***************************************************************" << std::endl;
}

bool
LimitationOnIndirectTravel::checkDiagReqCxr(bool diagCxrFound)
{
   Diag372Collector::DiagStream diagStr;
   bool diag = _diagEnabled && (_trx.diagnostic().diagnosticType() == Diagnostic372);
   std::string diagCxr = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
   if (diag && !diagCxrFound && !diagCxr.empty())
   {
      DiagManager diagManager(_trx, Diagnostic372);
      diagManager << "REQUESTED CARRIER CODE NOT A VALIDATING CXR\n";
      _trx.diagnostic().deActivate();
        return false;
   }
   return true;
}

bool
LimitationOnIndirectTravel::anyCxrPass(FarePath& fp, const std::vector<CarrierCode>& cxrToRemove)
{
  if(cxrToRemove.size() == fp.validatingCarriers().size())
  {
     fp.validatingCarriers().erase(fp.validatingCarriers().begin(),
                                   fp.validatingCarriers().end());
     return false;
  }

  Diag372Collector::DiagStream diagStr;
  bool diag = _diagEnabled && (_trx.diagnostic().diagnosticType() == Diagnostic372);

  if (diag)
  {
     std::string diagCxr = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
     if (!diagCxr.empty())
     {
       std::vector<CarrierCode>::const_iterator itci = cxrToRemove.begin();
       std::vector<CarrierCode>::const_iterator itcie = cxrToRemove.end();
       for(;itci != itcie; ++ itci)
       {
          if(find(cxrToRemove.begin(), cxrToRemove.end(), diagCxr) != cxrToRemove.end())
            return false;
       }
     }
  }

  std::vector<CarrierCode>::iterator itI = fp.validatingCarriers().begin();
  std::vector<CarrierCode>::iterator itEnd = fp.validatingCarriers().end();
  for(;itI != itEnd;)
  {
     CarrierCode carrier = *itI;
     std::vector<CarrierCode>::const_iterator itci = cxrToRemove.begin();
     std::vector<CarrierCode>::const_iterator itcie = cxrToRemove.end();
     for(;itci != itcie; )
     {
        if(find(cxrToRemove.begin(), cxrToRemove.end(), carrier) != cxrToRemove.end())
        {
           fp.validatingCarriers().erase(itI);
           break;
        }
        else
           ++itci;
     }
     ++itI;
  }
  return !fp.validatingCarriers().empty();
}


void
LimitationOnIndirectTravel::prepareDiag372OutPut(FarePath& fp,
                                                 std::string& crs,
                                                 Diag372Collector::DiagStream& diagStr)
{
  if (_diag)
  {
     DCFactory* factory = DCFactory::instance();
     Diag372Collector* diagCollector = dynamic_cast<Diag372Collector*>(factory->create(_trx));

     if (diagCollector != nullptr)
     {
       diagCollector->enable(Diagnostic372);

       // crs
       std::string userApplStr;

       if (!crs.empty())
       {
          if (crs == INFINI_MULTIHOST_ID)
            userApplStr = INFINI_USER;
          else if (crs == AXESS_MULTIHOST_ID)
            userApplStr = AXESS_USER;
          else if (crs == ABACUS_MULTIHOST_ID)
            userApplStr = ABACUS_USER;
          else if (crs == SABRE_MULTIHOST_ID)
            userApplStr = SABRE_USER;
       }
       if(!displayFarePath())
       {
          displayFarePath(true);
          (*diagCollector) << "***************************************************************" << std::endl;
          (*diagCollector) << fp << std::endl;
          (*diagCollector) << "**********  TPM SURFACE RESTRICTION EXEMPTION TABLE ***********" << std::endl
                         << "CRS USER APPLICATION:" << std::setw(42) << userApplStr << std::endl
                         << "MULTI-HOST USER APPLICATION:" << std::setw(35) << _trx.getRequest()->ticketingAgent()->hostCarrier() << std::endl;
       }
       (*diagCollector) << "***************************************************************" << std::endl;
       (*diagCollector) << "*********** TPM SURFACE RESTRICTION EXEMPTION CHECK ***********" << std::endl
                         << "MATCH" << std::setw(58) << ((_valCxrResult == -1) ? "NO" : "YES") << std::endl;

       (*diagCollector) << "SEQUENCE NO";
       diagCollector->setf(std::ios::right, std::ios::adjustfield);

       if (_valCxrResult != -1)
       {
         (*diagCollector) << std::setw(52) << _valCxrResult;
       }

       (*diagCollector) << std::endl;
       (*diagCollector) << diagStr << std::endl;
       diagCollector->flushMsg();
     }
  }
}

} // namespace tse

