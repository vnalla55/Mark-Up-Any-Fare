// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "Pricing/JourneyValidator.h"

#include "Common/BookingCodeUtil.h"
#include "Common/ClassOfService.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Pricing/FarePathUtils.h"
#include "Util/Algorithm/Container.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(fallbackPriceByCabinActivation)

bool
JourneyValidator::validateJourney(FarePath& fpath, std::string& results)
{
  bool valid = false;
  const bool journeyDiagOn = farepathutils::journeyDiag(_trx, fpath, _diag);
  if (UNLIKELY(journeyDiagOn))
    farepathutils::journeyDiagShowStatus(fpath, _diag, true);

  valid = journey(fpath, *(fpath.itin()), journeyDiagOn);
  results += (valid ? "JOURNEY:P " : "JOURNEY:F ");
  if (UNLIKELY(journeyDiagOn))
    farepathutils::journeyDiagShowStatus(fpath, _diag, false);

  return valid;
}
bool
JourneyValidator::journey(FarePath& fpath, Itin& itinerary, bool journeyDiagOn)
{
  std::vector<TravelSeg*> segmentsProcessed;

  std::vector<TravelSeg*>::const_iterator tvlIBegin;
  AirSeg* airSegBegin = nullptr;
  std::vector<TravelSeg*>::const_iterator segPI;
  std::vector<TravelSeg*>::const_iterator segPIE;
  bool fareMarketAlreadyProcessed = false;
  std::vector<FareMarket*> journeysForShoppingItin;
  if (LIKELY(_trx.getTrxType() == PricingTrx::MIP_TRX))
  {
    ItinUtil::journeys(itinerary, _trx, journeysForShoppingItin);
  }
  for (FareMarket* fareMarketPtr : itinerary.flowFareMarket())
  {
    FareMarket& fm = *fareMarketPtr;

    if (LIKELY(_trx.getTrxType() == PricingTrx::MIP_TRX))
    {
      if (UNLIKELY(!validShoppingJourney(journeysForShoppingItin, fareMarketPtr)))
        continue;
    }

    tvlIBegin = fm.travelSeg().begin();
    airSegBegin = dynamic_cast<AirSeg*>(*tvlIBegin);

    // did we already process this group of flights
    segPI = segmentsProcessed.begin();
    segPIE = segmentsProcessed.end();
    fareMarketAlreadyProcessed = false;
    for (; segPI != segPIE; segPI++)
    {
      if ((*tvlIBegin) == (*segPI))
      {
        fareMarketAlreadyProcessed = true;
        break;
      }
    }
    if (fareMarketAlreadyProcessed)
      continue;
    if (airSegBegin->flowJourneyCarrier() ||
        TrxUtil::isIntralineAvailabilityCxr(_trx, airSegBegin->carrier()))
    {
      if (!flowJourney(fpath, fm))
      {
        farepathutils::jDiagInfo(_diag, fm, FAIL_JOURNEY_SHOPPING, journeyDiagOn);
        return false;
      }
      continue;
    }

    if (UNLIKELY(!airSegBegin->localJourneyCarrier()))
      continue;

    if (sameBookingCode(fpath, fm))
    {
      for (TravelSeg* travelSeg : fm.travelSeg())
        segmentsProcessed.push_back(travelSeg);

      farepathutils::jDiagInfo(_diag, fm, INFO_SAME_BC, journeyDiagOn);
      if (!useFlowAvail(fpath, fm))
      {
        farepathutils::jDiag(_diag, segmentsProcessed, FAIL_USE_FLOW_AVAIL, journeyDiagOn);
        return false;
      }
      if (!needSetRebook(fm, fpath))
      {
        farepathutils::jDiag(_diag, segmentsProcessed, FAIL_NEED_SET_REBOOK, journeyDiagOn);
        return false;
      }
      farepathutils::jDiagInfo(_diag, fm, INFO_FLOW_AVAIL_PASS, journeyDiagOn);
      JourneyUtil::addOAndDMarket(_trx, itinerary, &fpath, &fm, true);
    }
    else
    {
      farepathutils::jDiagInfo(_diag, fm, INFO_TRY_LOCAL_AVAIL, journeyDiagOn);
      bool failJourney = false;
      bool flowPass = true;
      bool processStarted = false;
      FareUsage* fu = nullptr;
      uint16_t tvlSegIndex = 0;
      AirSeg* airSeg = nullptr;
      size_t flowLen = 0;
      std::vector<TravelSeg*>::const_iterator tvlL = itinerary.travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tvlLE = itinerary.travelSeg().end();

      for (; tvlL != tvlLE; tvlL++)
      {
        airSeg = dynamic_cast<AirSeg*>(*tvlL);
        if (airSeg == nullptr)
          continue;
        // start this process only from the start point of this FareMarket
        if (!processStarted)
        {
          if ((*tvlIBegin) == (*tvlL))
            processStarted = true;
          else
            continue;
        }

        if (airSegBegin->carrier() != airSeg->carrier())
          break;

        if (UNLIKELY(!airSeg->localJourneyCarrier()))
          break;

        fu = farepathutils::getFareUsage(fpath, *tvlL, tvlSegIndex);
        if (fu == nullptr || !checkLocalAvailStatus(fu, *tvlL, tvlSegIndex))
          return false;

        if (UNLIKELY(airSeg->resStatus() != CONFIRM_RES_STATUS))
        {
          segmentsProcessed.push_back(*tvlL);
          continue;
        }

        failJourney = false;
        flowLen = startFlow(fpath, (*tvlL), failJourney);
        if (failJourney)
        {
          farepathutils::jDiag(_diag, segmentsProcessed, FAIL_START_FLOW, journeyDiagOn);
          return false;
        }
        if (flowLen < 2)
        {
          segmentsProcessed.push_back(*tvlL);
          continue;
        }

        flowPass = foundFlow(fpath, (*tvlL), flowLen);
        if (flowLen < 2)
        {
          segmentsProcessed.push_back(*tvlL);
          continue;
        }

        for (size_t i = 1; (i < flowLen) && (tvlL != tvlLE);)
        {
          segmentsProcessed.push_back(*tvlL);
          ++tvlL;
          ++i;
        }
        if (!flowPass)
        {
          farepathutils::jDiag(_diag, segmentsProcessed, FAIL_FOUND_FLOW, journeyDiagOn);
          return false;
        }

        segmentsProcessed.push_back(*tvlL);
      }
    }
  }

  farepathutils::jDiag(_diag, segmentsProcessed, PASS_JOURNEY, journeyDiagOn);

  if (!processOtherSegs(fpath, segmentsProcessed))
  {
    farepathutils::jDiag(_diag, segmentsProcessed, FAIL_OTHER_SEGS, journeyDiagOn);
    return false;
  }

  farepathutils::jDiag(_diag, segmentsProcessed, PASS_OTHER_SEGS, journeyDiagOn);

  return true;
}

bool
JourneyValidator::validShoppingJourney(std::vector<FareMarket*>& journeysForShopping,
                                       const FareMarket* fm)
{
  return alg::contains(journeysForShopping, fm);
}

bool
JourneyValidator::flowJourney(FarePath& fpath, const FareMarket& fm)
{
  FareUsage* fu = nullptr;
  uint16_t tvlSegIndex = 0;
  const TravelSeg* tvlSegQF = nullptr;
  bool lastSegArunk = false;
  uint16_t iTvlFm = 0;
  auto tvlI = fm.travelSeg().begin();
  auto tvlE = fm.travelSeg().end();
  auto tvlLast = tvlE - 1;
  for (; tvlI != tvlE; ++tvlI, ++iTvlFm)
  {
    if (!(*tvlI)->isAir())
    {
      if (tvlI == tvlLast)
        lastSegArunk = true;
      continue;
    }
    fu = farepathutils::getFareUsage(fpath, *tvlI, tvlSegIndex);
    if (UNLIKELY(fu == nullptr))
      continue;

    PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[tvlSegIndex];

    // only for shopping check the real avail
    if (LIKELY(_trx.getTrxType() == PricingTrx::MIP_TRX) || (!fallback::fallbackPriceByCabinActivation(&_trx) &&
  	      !_trx.getOptions()->cabin().isUndefinedClass() && fm.travelSeg()[iTvlFm]->rbdReplaced()))
    {
      tvlSegQF = iTvlFm < fm.travelSeg().size() ? fm.travelSeg()[iTvlFm] : nullptr;
      bool checkBookedAvail = checkBookedClassAvail(tvlSegQF);
      if (!realAvail(fu, tvlSegIndex, fm, iTvlFm, checkBookedAvail, false))
        return false;
    }

    DifferentialData* diff = farepathutils::differentialData(fu, *tvlI);

    if (tvlI == tvlLast)
    {
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);
      if (diff != nullptr)
      {
        const PaxTypeFare::SegmentStatus& diffSegStat = farepathutils::diffSegStatus(diff, *tvlI);

        const_cast<PaxTypeFare::SegmentStatus&>(diffSegStat)
            ._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);
      }
    }
    else
    {
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, false);
      if (UNLIKELY(diff != nullptr))
      {
        const PaxTypeFare::SegmentStatus& diffSegStat = farepathutils::diffSegStatus(diff, *tvlI);

        const_cast<PaxTypeFare::SegmentStatus&>(diffSegStat)
            ._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, false);
      }
    }
  }

  if (UNLIKELY(lastSegArunk))
  {
    if (fm.travelSeg().size() < 2)
      return true;
    tvlLast = tvlE - 2;
    fu = farepathutils::getFareUsage(fpath, *tvlLast, tvlSegIndex);
    if (fu == nullptr)
      return true;
    PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[tvlSegIndex];
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);

    DifferentialData* diff = farepathutils::differentialData(fu, *tvlLast);
    if (diff != nullptr)
    {
      const PaxTypeFare::SegmentStatus& diffSegStat = farepathutils::diffSegStatus(diff, *tvlLast);

      const_cast<PaxTypeFare::SegmentStatus&>(diffSegStat)
          ._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);
    }
  }
  return true;
}

bool
JourneyValidator::foundFlow(FarePath& fpath, TravelSeg* startTvlseg, size_t& flowLen)
{
  AirSeg* airSeg = nullptr;
  const TravelSeg* tvlSegQF = nullptr;
  FareMarket* fm = nullptr;
  bool mileageCheckOk = false;
  std::vector<TravelSeg*>::const_iterator fmTvlI;
  std::vector<TravelSeg*>::const_iterator fmTvlE;
  FareUsage* fu = nullptr;
  uint16_t tvlSegIndex = 0;
  Itin& itin = *fpath.itin();
  std::vector<TravelSeg*>::const_iterator tvlI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();
  std::vector<TravelSeg*> jnyTvlSegs;
  std::vector<TravelSeg*> jnyTvlSegs2;

  for (; tvlI != tvlE; tvlI++)
  {
    if ((*tvlI) != startTvlseg)
      continue;

    // point to the last segment of the flow FareMarket
    for (size_t i = 1; (i < flowLen) && (tvlI != tvlE);)
    {
      jnyTvlSegs.push_back(*tvlI);
      ++tvlI;
      ++i;
    }

    if (LIKELY(tvlI != tvlE))
    {
      airSeg = dynamic_cast<AirSeg*>(*tvlI);
      // if last segment of the flow market is not ARUNK
      // then only put it in journey
      if (airSeg != nullptr)
        jnyTvlSegs.push_back(*tvlI);

      mileageCheckOk = ItinUtil::checkMileage(_trx, jnyTvlSegs, itin.travelDate());
      if (!mileageCheckOk)
      {
        // check if we can make a journey of 1st 2 flights
        if (flowLen == 3 && jnyTvlSegs.size() == 3)
        {
          // make sure the 2nd segment is not ARNK before stepping back
          airSeg = dynamic_cast<AirSeg*>(jnyTvlSegs[1]);

          if (airSeg == nullptr)
          {
            flowLen = 0;
            return false;
          }

          jnyTvlSegs2.resize(0);
          jnyTvlSegs2.push_back(jnyTvlSegs[0]);
          jnyTvlSegs2.push_back(jnyTvlSegs[1]);
          mileageCheckOk = ItinUtil::checkMileage(_trx, jnyTvlSegs2, itin.travelDate());
          if (mileageCheckOk)
          {
            --flowLen;
            jnyTvlSegs.resize(0);
            jnyTvlSegs.push_back(jnyTvlSegs2[0]);
            jnyTvlSegs.push_back(jnyTvlSegs2[1]);
          }
        }
      }

      if (!mileageCheckOk)
      {
        flowLen = 0;
        return false;
      }

      fm = ItinUtil::findMarket(itin, jnyTvlSegs);
      if (fm == nullptr)
      {
        // if we can not find this flow Market then try again with 2 flights
        if (flowLen == 3 && jnyTvlSegs.size() == 3)
        {
          // make sure the 2nd segment is not ARNK before stepping back
          airSeg = dynamic_cast<AirSeg*>(jnyTvlSegs[1]);

          if (airSeg == nullptr)
          {
            flowLen = 0;
            return false;
          }

          jnyTvlSegs2.resize(0);
          --flowLen;
          jnyTvlSegs2.push_back(jnyTvlSegs[0]);
          jnyTvlSegs2.push_back(jnyTvlSegs[1]);
          fm = ItinUtil::findMarket(itin, jnyTvlSegs2);
        }
      }

      if (fm == nullptr)
      {
        flowLen = 0;
        return false;
      }

      fmTvlI = fm->travelSeg().begin();
      fmTvlE = fm->travelSeg().end();
      for (uint16_t j = 0; fmTvlI != fmTvlE; fmTvlI++, j++)
      {
        airSeg = dynamic_cast<AirSeg*>(*fmTvlI);
        if (airSeg == nullptr)
          continue;
        fu = farepathutils::getFareUsage(fpath, (*fmTvlI), tvlSegIndex);
        if (LIKELY(fu != nullptr))
        {
          if(airSeg->localJourneyCarrier() &&
             !isLocalAvailForDualRbd(fu, tvlSegIndex, *fm, j))
            return false;

          PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[tvlSegIndex];

          tvlSegQF = j < fm->travelSeg().size() ? fm->travelSeg()[j] : nullptr;
          bool checkBookedAvail = checkBookedClassAvail(tvlSegQF);
          if (!realAvail(fu, tvlSegIndex, *fm, j, checkBookedAvail, false))
            return false;

          if (!needSetRebook(*fm, fpath))
            return false;

          // only the last segment of this fareMarket should have avail break
          DifferentialData* diff = farepathutils::differentialData(fu, *fmTvlI);

          if (j == fm->travelSeg().size() - 1)
          {
            segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);
            if (UNLIKELY(diff != nullptr))
            {
              const PaxTypeFare::SegmentStatus& diffSegStat =
                  farepathutils::diffSegStatus(diff, *fmTvlI);

              const_cast<PaxTypeFare::SegmentStatus&>(diffSegStat)
                  ._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);
            }
          }
          else
          {
            segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, false);
            if (UNLIKELY(diff != nullptr))
            {
              const PaxTypeFare::SegmentStatus& diffSegStat =
                  farepathutils::diffSegStatus(diff, *fmTvlI);

              const_cast<PaxTypeFare::SegmentStatus&>(diffSegStat)
                  ._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, false);
            }
          }
        }
      }
    }

    break;
  }

  JourneyUtil::addOAndDMarket(_trx, itin, &fpath, fm, true);
  return true;
}

bool
JourneyValidator::checkBookedClassAvail(const TravelSeg* tvlSeg)
{
  if (UNLIKELY(!_trx.getRequest()->isLowFareRequested()))
    return false;

  if (_trx.getTrxType() == PricingTrx::MIP_TRX && _trx.billing()->actionCode() != "WPNI.C" &&
      _trx.billing()->actionCode() != "WFR.C")
    return true;

  if (tvlSeg != nullptr)
  {
    if (tvlSeg->realResStatus() == QF_RES_STATUS)
      return true;
  }

  return false;
}

size_t
JourneyValidator::startFlow(FarePath& fpath, TravelSeg* startTvlseg, bool& failJourney)
{
  FareUsage* prevFu = nullptr;
  FareUsage* currFu = nullptr;
  FareUsage* firstFu = nullptr;
  uint16_t prevTvlSegIndex = 0;
  uint16_t currTvlSegIndex = 0;
  uint16_t firstTvlSegIndex = 0;
  BookingCode prevBc;
  BookingCode currBc;
  BookingCode firstBc = "";

  size_t flowLen = 0;
  size_t nonArunks = 0;
  bool startFound = false;
  AirSeg* startAirSeg = nullptr;
  AirSeg* airSeg = nullptr;
  AirSeg* prevAirSeg = nullptr;
  TravelSeg* prevTvlSeg = nullptr;
  Itin& itin = *fpath.itin();
  std::vector<TravelSeg*>::const_iterator tvlI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();
  for (; tvlI != tvlE; tvlI++)
  {
    if (!startFound)
    {
      if ((*tvlI) != startTvlseg)
        continue;
      startFound = true;
    }
    if ((*tvlI) == startTvlseg)
    {
      startAirSeg = dynamic_cast<AirSeg*>(*tvlI);
      prevAirSeg = startAirSeg;
      prevTvlSeg = (*tvlI);
      ++nonArunks;
      ++flowLen;

      currFu = farepathutils::getFareUsage(fpath, *tvlI, currTvlSegIndex);
      if (UNLIKELY(currFu == nullptr || !checkLocalAvailStatus(currFu, *tvlI, currTvlSegIndex)))
      {
        failJourney = true;
        return flowLen;
      }

      continue;
    }

    airSeg = dynamic_cast<AirSeg*>(*tvlI);
    if (airSeg == nullptr)
    {
      if ((*tvlI) != nullptr)
      {
        // only multiAirport ARUNKS are considered connections
        if (!(*tvlI)->arunkMultiAirportForAvailability())
          return flowLen;
      }
      ++flowLen;
    }
    else
    {
      if (!TrxUtil::intralineAvailabilityApply(_trx, startAirSeg->carrier(), airSeg->carrier()))
      {
        if (airSeg->carrier() != startAirSeg->carrier())
          return flowLen;
        if (UNLIKELY(!airSeg->localJourneyCarrier()))
          return flowLen;
      }

      if (UNLIKELY(airSeg->resStatus() != CONFIRM_RES_STATUS))
        return flowLen;

      if (!ItinUtil::journeyConnection(itin, airSeg, prevAirSeg, nonArunks + 1))
        return flowLen;

      currFu = farepathutils::getFareUsage(fpath, *tvlI, currTvlSegIndex);
      if (currFu == nullptr || !checkLocalAvailStatus(currFu, *tvlI, currTvlSegIndex))
      {
        failJourney = true;
        return flowLen;
      }

      currBc = bookingCode(currFu, *tvlI, currFu->segmentStatus()[currTvlSegIndex]);

      prevFu = farepathutils::getFareUsage(fpath, prevTvlSeg, prevTvlSegIndex);
      if (UNLIKELY(prevFu == nullptr))
        return flowLen;

      prevBc = bookingCode(prevFu, prevTvlSeg, prevFu->segmentStatus()[prevTvlSegIndex]);

      if (currBc != prevBc)
      {
        if (firstBc.empty())
        {
          firstFu = farepathutils::getFareUsage(fpath, startTvlseg, firstTvlSegIndex);
          if (UNLIKELY(firstFu == nullptr))
            return flowLen;

          firstBc = bookingCode(firstFu, startTvlseg, firstFu->segmentStatus()[firstTvlSegIndex]);
        }
        if (!BookingCodeUtil::premiumMarriage(_trx, *prevAirSeg, prevBc, currBc))
        {
          // In previous journeyConnection() call we might have allowed the 24 hour connection
          // on domestic part because of 3rd flight was international but now when we know the
          // third flight has different booking code then the first 2 flights must have 4 hour
          // connection only.
          if (recheckJourneyConnection(nonArunks, startAirSeg, prevAirSeg, airSeg))
          {
            if (!ItinUtil::journeyConnection(itin, prevAirSeg, startAirSeg, 0))
              failJourney = true;
          }
          return flowLen;
        }
      }

      ++flowLen;
      ++nonArunks;
    }
    if (airSeg != nullptr)
    {
      prevAirSeg = airSeg;
      prevTvlSeg = (*tvlI);
    }
    // a flow market can have maximum 3 flights
    if (nonArunks == 3)
      break;
  }
  return flowLen;
}

bool
JourneyValidator::checkLocalAvailStatus(FareUsage* fu, TravelSeg* tvlSeg, uint16_t tvlSegIndex)
{
  if (UNLIKELY(_trx.getRequest()->originBasedRTPricing()))
  {
    if ((tvlSeg != nullptr) && (tvlSeg->isAir()))
    {
      const AirSeg* airSeg = static_cast<const AirSeg*>((tvlSeg));
      if (airSeg->isFake())
      {
        return true;
      }
    }
  }

  if (UNLIKELY(fu == nullptr))
    return false;

  PaxTypeFare::BookingCodeStatus& bcStatus = fu->bookingCodeStatus();

  if (UNLIKELY(processingKeepFare(*fu)))
  {
    if (bcStatus.isSet(PaxTypeFare::BKS_FAIL) ||
        !(bcStatus.isSet(PaxTypeFare::BKS_PASS_LOCAL_AVAIL)))
    {
      bcStatus.set(PaxTypeFare::BKS_FAIL_CAT31_KEEP_FARE, true);
      return true;
    }
  }

  if (fu->mixClassStatus() == PaxTypeFare::MX_DIFF)
  {
    if (!bcStatus.isSet(PaxTypeFare::BKS_PASS) || bcStatus.isSet(PaxTypeFare::BKS_FAIL))
      return false;
  }
  else
  {
    if (!bcStatus.isSet(PaxTypeFare::BKS_PASS_LOCAL_AVAIL))
      bcStatus.set(PaxTypeFare::BKS_PASS, false);

    if (UNLIKELY(bcStatus.isSet(PaxTypeFare::BKS_FAIL)))
      return false;

    if (!(bcStatus.isSet(PaxTypeFare::BKS_PASS_LOCAL_AVAIL) ||
          bcStatus.isSet(PaxTypeFare::BKS_MIXED)))
      return false; // means local availability failed for this fare
  }

  // check CAT 5 status also
  if (LIKELY(tvlSegIndex < fu->segmentStatus().size()))
  {
    PaxTypeFare::SegmentStatus& fuSegStat = fu->segmentStatus()[tvlSegIndex];

    if (hasCat5Rebook(fu, tvlSeg, fuSegStat))
    {
      if (!realAvail(fu, tvlSegIndex, *(fu->paxTypeFare()->fareMarket()), tvlSegIndex, true, true))
        return false;
    }
  }

  return true;
}

bool
JourneyValidator::isLocalAvailForDualRbd(FareUsage* fu,
                                         uint16_t iTravelFu,
                                         FareMarket& fm,
                                         uint16_t iTravelFm)
{
  if (UNLIKELY(iTravelFm >= fm.travelSeg().size() || iTravelFu >= fu->segmentStatus().size()))
  {
    // safety check
    return false;
  }
  const PaxTypeFare::SegmentStatus& fuSegStat = fu->segmentStatus()[iTravelFu];
  if (!fuSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_DUAL_RBD_PASS) ||
       fuSegStat._dualRbd1.empty())
    return true;

  const BookingCode checkBc = fuSegStat._dualRbd1;
  std::vector<ClassOfService*>* cos = nullptr;
  uint16_t cosVecSize = fm.classOfServiceVec().size();
  if (cosVecSize == 0)
    return false;
  if (UNLIKELY(iTravelFm > cosVecSize - 1))
    return false;
  if (UNLIKELY(fm.classOfServiceVec()[iTravelFm] == nullptr))
    return false;
  cos = fm.classOfServiceVec()[iTravelFm];

  const uint16_t numSeatsRequired = PaxTypeUtil::numSeatsForFare(_trx, *(fu->paxTypeFare()));

  bool passFlow = false;
  uint16_t cosSize = cos->size();
  for (uint16_t k = 0; k < cosSize; k++)
  {
    ClassOfService& cs = *((*cos)[k]);
    if (cs.bookingCode() == checkBc && cs.numSeats() >= numSeatsRequired)
    {
      passFlow = true;
      break;
    }
  }
  return passFlow;
}

bool
JourneyValidator::realAvail(FareUsage* fu,
                            uint16_t iTravelFu,
                            const FareMarket& fm,
                            uint16_t iTravelFm,
                            bool checkBookedCosAlso,
                            bool useLocal)
{
  if (UNLIKELY(iTravelFm >= fm.travelSeg().size() || iTravelFu >= fu->segmentStatus().size()))
  {
    // safety check
    return false;
  }

  const BookingCode checkBc =
      bookingCode(fu, fm.travelSeg()[iTravelFm], fu->segmentStatus()[iTravelFu]);

  if (checkBc == fm.travelSeg()[iTravelFm]->getBookingCode())
  {
    if (!checkBookedCosAlso)
      return true;
  }

  const std::vector<ClassOfService*>* cos = nullptr;

  if (useLocal)
  {
    const TravelSeg* ts = fm.travelSeg()[iTravelFm];
    cos = &ts->classOfService();
  }
  else
  {
    uint16_t cosVecSize = fm.classOfServiceVec().size();
    if (cosVecSize == 0)
      return false;
    if (UNLIKELY(iTravelFm > cosVecSize - 1))
      return false;
    if (UNLIKELY(fm.classOfServiceVec()[iTravelFm] == nullptr))
      return false;
    cos = fm.classOfServiceVec()[iTravelFm];
  }

  const uint16_t numSeatsRequired = PaxTypeUtil::numSeatsForFare(_trx, *(fu->paxTypeFare()));

  bool passFlow = false;
  uint16_t cosSize = cos->size();
  for (uint16_t k = 0; k < cosSize; k++)
  {
    const ClassOfService& cs = *((*cos)[k]);
    if (cs.bookingCode() == checkBc && cs.numSeats() >= numSeatsRequired)
    {
      passFlow = true;
      break;
    }
  }
  return passFlow;
}

bool
JourneyValidator::hasCat5Rebook(const FareUsage* fu,
                                const TravelSeg* tvlSeg,
                                const PaxTypeFare::SegmentStatus& segStat)
{
  const DifferentialData* diff = farepathutils::differentialData(fu, tvlSeg);
  const auto& status = diff ? farepathutils::diffSegStatus(diff, tvlSeg) : segStat;

  return (status._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
          !(status._bkgCodeReBook.empty()) && tvlSeg->getBookingCode() == status._bkgCodeReBook);
}

bool
JourneyValidator::needSetRebook(FareMarket& fm, FarePath& fp)
{
  bool allTvlSegsNeedRebook = true;
  bool atleastOneTvlSegNeedRebook = false;
  uint16_t tvlSegIndex = 0;
  bool cat5Rebook = false;

  for (TravelSeg* tvlSeg : fm.travelSeg())
  {
    if (!tvlSeg->isAir())
      continue;
    FareUsage* fu = farepathutils::getFareUsage(fp, tvlSeg, tvlSegIndex);
    if (UNLIKELY(!fu))
      return false;

    if (reBookRequired(fu, tvlSeg, fu->segmentStatus()[tvlSegIndex]))
      atleastOneTvlSegNeedRebook = true;
    else
      allTvlSegsNeedRebook = false;

    if (hasCat5Rebook(fu, tvlSeg, fu->segmentStatus()[tvlSegIndex]))
      cat5Rebook = true;
  }

  if (!cat5Rebook)
  {
    if (allTvlSegsNeedRebook || !atleastOneTvlSegNeedRebook)
      return true;
  }

  uint16_t iTvlFm = 0;

  for (TravelSeg* tvlSeg : fm.travelSeg())
  {
    if (!tvlSeg->isAir())
    {
      ++iTvlFm;
      continue;
    }
    FareUsage* fu = farepathutils::getFareUsage(fp, tvlSeg, tvlSegIndex);
    if (UNLIKELY(!fu))
      return false;

    PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[tvlSegIndex];

    bool doRebook = false;
    if (reBookRequired(fu, tvlSeg, segStat))
    {
      if (!hasCat5Rebook(fu, tvlSeg, segStat))
      {
        ++iTvlFm;
        continue;
      }
      doRebook = true;
    }

    if (!realAvail(fu, tvlSegIndex, fm, iTvlFm, true, false))
      return false;

    if (!doRebook && _trx.getTrxType() == PricingTrx::MIP_TRX)
      {
        ++iTvlFm;
        continue;
      }

    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
    segStat._bkgCodeReBook = tvlSeg->getBookingCode();

    // also set the reBook indicator in differential fare
    DifferentialData* diff = farepathutils::differentialData(fu, tvlSeg);
    if (LIKELY(!diff))
      {
        ++iTvlFm;
        continue;
      }

    const PaxTypeFare::SegmentStatus& diffSegStat = farepathutils::diffSegStatus(diff, tvlSeg);
    const_cast<PaxTypeFare::SegmentStatus&>(diffSegStat)
        ._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
    const_cast<PaxTypeFare::SegmentStatus&>(diffSegStat)._bkgCodeReBook = tvlSeg->getBookingCode();
  }
  return true;
}

bool
JourneyValidator::realAvailLocal(FareUsage* fu,
                                 uint16_t iTravelFu,
                                 TravelSeg* tvlSeg,
                                 bool checkBookedCosAlso)
{
  const BookingCode checkBc = bookingCode(fu, tvlSeg, fu->segmentStatus()[iTravelFu]);

  fu->segmentStatus()[iTravelFu]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);
  if (checkBc == tvlSeg->getBookingCode() && !checkBookedCosAlso)
    return true;

  if (tvlSeg->classOfService().empty())
    return false;

  const uint16_t numSeatsRequired = PaxTypeUtil::numSeatsForFare(_trx, *(fu->paxTypeFare()));

  return std::any_of(
      tvlSeg->classOfService().begin(),
      tvlSeg->classOfService().end(),
      [=](ClassOfService* cs)
      { return cs->bookingCode() == checkBc && cs->numSeats() >= numSeatsRequired; });
}

bool
JourneyValidator::reBookRequired(const FareUsage* fu,
                                 const TravelSeg* tvlSeg,
                                 const PaxTypeFare::SegmentStatus& fuSegStat)
{
  const DifferentialData* diff = farepathutils::differentialData(fu, tvlSeg);
  const auto& status = diff ? farepathutils::diffSegStatus(diff, tvlSeg) : fuSegStat;
  return (status._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
          !(status._bkgCodeReBook.empty()));
}

bool
JourneyValidator::processJourneyAfterDifferential(FarePath& fpath, std::string& _results)
{
  bool valid = true;

  if (differentialFound(fpath))
  {
    valid = journeyAfterDifferential(fpath);
    _results += (valid ? "JOURNEY-DIFF:P " : "JOURNEY-DIFF:F ");
  }

  return valid;
}

bool
JourneyValidator::differentialFound(FarePath& fpath)
{
  Itin& itinerary = *fpath.itin();
  std::vector<TravelSeg*>::const_iterator tvlL = itinerary.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlLE = itinerary.travelSeg().end();

  FareUsage* fu = nullptr;
  uint16_t tvlSegIndex = 0;
  for (; tvlL != tvlLE; tvlL++)
  {
    if (UNLIKELY(*tvlL == nullptr))
      continue;

    if (dynamic_cast<AirSeg*>(*tvlL) == nullptr)
      continue;

    fu = farepathutils::getFareUsage(fpath, *tvlL, tvlSegIndex);
    if (UNLIKELY(fu == nullptr))
      return false;

    if (farepathutils::differentialData(fu, *tvlL) != nullptr)
      return true;
  }
  return false;
}

bool
JourneyValidator::journeyAfterDifferential(FarePath& fpath)
{
  // process the FLOW journey carriers
  Itin& itin = *fpath.itin();

  std::vector<FareMarket*> journeysForShoppingItin;
  if (_trx.getTrxType() == PricingTrx::MIP_TRX)
    ItinUtil::journeys(itin, _trx, journeysForShoppingItin);

  for (const auto* fm : itin.flowFareMarket())
  {
    if (_trx.getTrxType() == PricingTrx::MIP_TRX)
    {
      if (!validShoppingJourney(journeysForShoppingItin, fm))
        continue;
    }

    const AirSeg* airSegBegin = fm->travelSeg().front()->toAirSeg();

    if (airSegBegin->flowJourneyCarrier() ||
        TrxUtil::isIntralineAvailabilityCxr(_trx, airSegBegin->carrier()))
    {
      if (!flowJourney(fpath, *fm))
        return false;
    }
  }
  return processLocalJourneyCarriers(fpath, itin);
}
bool
JourneyValidator::processLocalJourneyCarriers(FarePath& fpath, Itin& itin)
{
  // process the LOCAL journey carriers
  bool failJourney = false;
  bool flowPass = true;
  FareUsage* fu = nullptr;
  uint16_t tvlSegIndex = 0;
  AirSeg* airSeg = nullptr;
  size_t flowLen = 0;
  std::vector<TravelSeg*>::const_iterator tvlL = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlLE = itin.travelSeg().end();

  for (; tvlL != tvlLE; tvlL++)
  {
    airSeg = dynamic_cast<AirSeg*>(*tvlL);
    if (airSeg == nullptr)
      continue;

    if (!airSeg->localJourneyCarrier())
      continue;

    fu = farepathutils::getFareUsage(fpath, *tvlL, tvlSegIndex);
    if (fu == nullptr || !checkLocalAvailStatus(fu, *tvlL, tvlSegIndex))
      return false;

    if (airSeg->resStatus() != CONFIRM_RES_STATUS)
      continue;

    failJourney = false;

    flowLen = startFlow(fpath, (*tvlL), failJourney);
    if (failJourney)
      return false;
    if (flowLen < 2)
      continue;

    flowPass = foundFlow(fpath, (*tvlL), flowLen);
    if (flowLen < 2)
      continue;

    for (size_t i = 1; (i < flowLen) && (tvlL != tvlLE);)
    {
      ++tvlL;
      ++i;
    }

    if (!flowPass)
      return false;
  }
  return true;
}
BookingCode
JourneyValidator::bookingCode(const FareUsage* fu,
                              const TravelSeg* tvlSeg,
                              const PaxTypeFare::SegmentStatus& fuSegStat)
{
  const DifferentialData* diff = farepathutils::differentialData(fu, tvlSeg);

  if (diff != nullptr)
  {
    const PaxTypeFare::SegmentStatus& diffSegStat = farepathutils::diffSegStatus(diff, tvlSeg);
    if (diffSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
        !(diffSegStat._bkgCodeReBook.empty()))
    {
      return diffSegStat._bkgCodeReBook;
    }
    else
    {
      return tvlSeg->getBookingCode();
    }
  }

  if (fuSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
      !(fuSegStat._bkgCodeReBook.empty()))
  {
    return fuSegStat._bkgCodeReBook;
  }

  return tvlSeg->getBookingCode();
}

bool
JourneyValidator::useFlowAvail(FarePath& fpath, FareMarket& fm)
{
  std::vector<TravelSeg*>::const_iterator tvlI = fm.travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator tvlE = fm.travelSeg().end();
  const std::vector<TravelSeg*>::const_iterator tvlLast = tvlE - 1;

  const AirSeg* airSeg = nullptr;
  const TravelSeg* tvlSegQF = nullptr;
  FareUsage* fu = nullptr;
  uint16_t tvlSegIndex = 0;
  uint16_t iTvlFm = 0;

  for (; tvlI != tvlE; tvlI++, iTvlFm++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      continue;
    fu = farepathutils::getFareUsage(fpath, *tvlI, tvlSegIndex);
    if (UNLIKELY(fu == nullptr))
      return false;

    PaxTypeFare::BookingCodeStatus& bcStatus = fu->bookingCodeStatus();
    if (UNLIKELY(flowAvailFailed(*fu)))
      return false;

    // do not reset BKS_MIXED if fareusage has more flights than journey fare market
    if (fu->paxTypeFare()->fareMarket()->travelSeg().size() <= fm.travelSeg().size())
    {
      if (bcStatus.isSet(PaxTypeFare::BKS_PASS_FLOW_AVAIL))
      {
        bcStatus.set(PaxTypeFare::BKS_PASS, true);
        bcStatus.set(PaxTypeFare::BKS_MIXED, false);
      }
    }

    if (bcStatus.isSet(PaxTypeFare::BKS_MIXED_FLOW_AVAIL))
    {
      bcStatus.set(PaxTypeFare::BKS_MIXED, true);
      bcStatus.set(PaxTypeFare::BKS_PASS, false);
    }

    if (LIKELY(!processingKeepFare(*fu) || !bcStatus.isSet(PaxTypeFare::BKS_FAIL_CAT31_KEEP_FARE)))
      bcStatus.set(PaxTypeFare::BKS_FAIL, false);

    if (UNLIKELY(tvlSegIndex >= fu->segmentStatus().size() ||
                  tvlSegIndex >= fu->segmentStatusRule2().size()))
      return false;

    PaxTypeFare::SegmentStatus& fuSegStat2 = fu->segmentStatusRule2()[tvlSegIndex];
    if(!fuSegStat2._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
      return false;

    fu->segmentStatus()[tvlSegIndex] = fu->segmentStatusRule2()[tvlSegIndex];

    PaxTypeFare::SegmentStatus& fuSegStat = fu->segmentStatus()[tvlSegIndex];

    // only for shopping check the real avail
    if (LIKELY(_trx.getTrxType() == PricingTrx::MIP_TRX) || (!fallback::fallbackPriceByCabinActivation(&_trx) &&
    	      !_trx.getOptions()->cabin().isUndefinedClass() && fm.travelSeg()[iTvlFm]->rbdReplaced()))
    {
      tvlSegQF = iTvlFm < fm.travelSeg().size() ? fm.travelSeg()[iTvlFm] : nullptr;
      bool checkBookedAvail = checkBookedClassAvail(tvlSegQF);
      if (!realAvail(fu, tvlSegIndex, fm, iTvlFm, checkBookedAvail, false))
        return false;
    }

    // break availability on the last travelSeg only
    if (tvlI == tvlLast)
      fuSegStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);
    else
      fuSegStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, false);
  }
  return true;
}

bool
JourneyValidator::sameBookingCode(FarePath& fpath, FareMarket& fm)
{
  std::vector<TravelSeg*>::const_iterator tvlI = fm.travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator tvlE = fm.travelSeg().end();

  const AirSeg* airSeg = nullptr;
  const AirSeg* prevAirSeg = nullptr;
  FareUsage* fu = nullptr;
  uint16_t tvlSegIndex = 0;
  BookingCode firstBc = "";
  BookingCode prevBc = "";
  BookingCode currBc;
  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      continue;
    // do not increment tvlSegIndex value in for loop.
    // This value will be computed by farepathutils::getFareUsage() function
    fu = farepathutils::getFareUsage(fpath, *tvlI, tvlSegIndex);
    if (UNLIKELY(fu == nullptr))
      return false;

    if (flowAvailFailed(*fu))
      return false;

    currBc = bookingCode(fu, *tvlI, fu->segmentStatusRule2()[tvlSegIndex]);

    if (firstBc.empty())
    {
      firstBc = currBc;
      prevBc = currBc;
      prevAirSeg = airSeg;
      continue;
    }
    if (currBc != prevBc)
    {
      if (!BookingCodeUtil::premiumMarriage(_trx, *prevAirSeg, prevBc, currBc))
      {
        return false;
      }
    }
    prevBc = currBc;
    prevAirSeg = airSeg;
  }
  return true;
}
bool
JourneyValidator::flowAvailFailed(FareUsage& fu)
{
  PaxTypeFare::BookingCodeStatus& bcStatus = fu.bookingCodeStatus();
  if ((bcStatus.isSet(PaxTypeFare::BKS_FAIL)) ||
      (!(bcStatus.isSet(PaxTypeFare::BKS_PASS_FLOW_AVAIL) ||
         bcStatus.isSet(PaxTypeFare::BKS_MIXED_FLOW_AVAIL))))
  {
    if (UNLIKELY(processingKeepFare(fu)))
      bcStatus.set(PaxTypeFare::BKS_FAIL_CAT31_KEEP_FARE, true);
    else
      return true;
  }
  return (bcStatus.isSet(PaxTypeFare::BKS_FAIL_FLOW_AVAIL));
}

bool
JourneyValidator::recheckJourneyConnection(const uint16_t nonArunks,
                                           const AirSeg* startAirSeg,
                                           const AirSeg* prevAirSeg,
                                           const AirSeg* airSeg)
{
  return (nonArunks == 2 && startAirSeg != nullptr && prevAirSeg != nullptr && airSeg != nullptr &&
          startAirSeg->carrier().equalToConst("DL") &&
          (startAirSeg->geoTravelType() == GeoTravelType::Domestic ||
           startAirSeg->geoTravelType() == GeoTravelType::Transborder) &&
          (prevAirSeg->geoTravelType() == GeoTravelType::Domestic ||
           prevAirSeg->geoTravelType() == GeoTravelType::Transborder) &&
          airSeg->geoTravelType() != GeoTravelType::Domestic &&
          airSeg->geoTravelType() != GeoTravelType::Transborder);
}

bool
JourneyValidator::checkMarriedConnection()
{
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
                static_cast<const RexPricingTrx*>(&_trx)->trxPhase() !=
                    RexPricingTrx::PRICE_NEWITIN_PHASE))
    return false;

  if (UNLIKELY(!_trx.getRequest()->isLowFareRequested()))
    return false;

  return JourneyUtil::useJourneyLogic(_trx);
}

bool
JourneyValidator::processMarriedConnection(FarePath& fpath)
{
  Itin& itin = *fpath.itin();
  for (FareMarket* fareMarket : itin.fareMarket())
  {
    if (!fareMarket->hasAllMarriedSegs())
      continue;
    if (!validConnection(*fareMarket, fpath))
      return false;
  }
  return true;
}

bool
JourneyValidator::validConnection(FareMarket& fm, FarePath& fp)
{
  bool allTvlSegsNeedRebook = true;
  bool atleastOneTvlSegNeedRebook = false;
  uint16_t tvlSegIndex = 0;
  const AirSeg* airSeg = nullptr;
  FareUsage* fu = nullptr;
  bool allLocalJourneyCarriers = true;
  bool allFlowJourneyCarriers = true;

  std::vector<TravelSeg*>::iterator tvlI = fm.travelSeg().begin();
  std::vector<TravelSeg*>::iterator tvlE = fm.travelSeg().end();
  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      continue;

    if (!airSeg->localJourneyCarrier())
      allLocalJourneyCarriers = false;

    if (!TrxUtil::isIntralineAvailabilityCxr(_trx, airSeg->carrier()))
    {
      if (!airSeg->flowJourneyCarrier())
        allFlowJourneyCarriers = false;
    }

    fu = farepathutils::getFareUsage(fp, *tvlI, tvlSegIndex);
    if (fu == nullptr)
      return false;

    if (fm.travelSeg() == fu->travelSeg())
      return true;

    if (reBookRequired(fu, *tvlI, fu->segmentStatus()[tvlSegIndex]))
      atleastOneTvlSegNeedRebook = true;
    else
      allTvlSegsNeedRebook = false;
  }

  if (allFlowJourneyCarriers)
    return true;

  if (!atleastOneTvlSegNeedRebook)
    adjustAvailBreaks(fm, fp);

  if (!allLocalJourneyCarriers)
    return true;

  if (allTvlSegsNeedRebook || !atleastOneTvlSegNeedRebook)
    return true;

  FareMarket* localFm = nullptr;
  std::vector<TravelSeg*> collectTravelSegs;
  bool startCollect = false;
  Itin& itin = *(fp.itin());
  tvlI = fm.travelSeg().begin();

  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      continue;
    fu = farepathutils::getFareUsage(fp, *tvlI, tvlSegIndex);
    if (fu == nullptr)
      return false;

    PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[tvlSegIndex];

    if (reBookRequired(fu, *tvlI, segStat))
    {
      startCollect = false;
      collectTravelSegs.clear();
      continue;
    }

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
    {
      collectTravelSegs.push_back(*tvlI);
      startCollect = false;
    }
    else
    {
      startCollect = true;
      collectTravelSegs.push_back(*tvlI);
    }

    if (!startCollect)
    {
      localFm = ItinUtil::findMarket(itin, collectTravelSegs);
      if (localFm == nullptr)
        return false;

      if (localFm->travelSeg().size() == 1)
      {
        if (!segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
        {
          if (!realAvail(fu, tvlSegIndex, *localFm, 0, true, false))
            return false;
        }
      }
      else
      {
        fu = farepathutils::getFareUsage(fp, collectTravelSegs.front(), tvlSegIndex);
        if (fu == nullptr)
          return false;

        PaxTypeFare::SegmentStatus& segStat1 = fu->segmentStatus()[tvlSegIndex];

        if (!segStat1._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
        {
          if (!realAvail(fu, tvlSegIndex, *localFm, 0, true, false))
            return false;
        }

        fu = farepathutils::getFareUsage(fp, collectTravelSegs.back(), tvlSegIndex);
        if (fu == nullptr)
          return false;

        PaxTypeFare::SegmentStatus& segStat2 = fu->segmentStatus()[tvlSegIndex];

        if (!segStat2._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
        {
          if (!realAvail(fu, tvlSegIndex, *localFm, 1, true, false))
            return false;
        }
      }
      collectTravelSegs.clear();
    }
  }

  return true;
}

void
JourneyValidator::adjustAvailBreaks(FareMarket& fm, FarePath& fp)
{
  TSE_ASSERT(!fm.travelSeg().empty());

  uint16_t tvlSegIndex = 0;

  std::vector<TravelSeg*>::iterator tvlI = fm.travelSeg().begin();
  std::vector<TravelSeg*>::iterator tvlE = fm.travelSeg().end();
  std::vector<TravelSeg*>::iterator tvlLast = tvlE - 1;

  for (; tvlI != tvlE; tvlI++)
  {
    if (!(*tvlI)->isAir())
      continue;
    FareUsage* fu = farepathutils::getFareUsage(fp, *tvlI, tvlSegIndex);
    if (fu == nullptr)
      return;

    PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[tvlSegIndex];

    if (tvlI == tvlLast)
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);
    else
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, false);
  }
}

bool
JourneyValidator::processOtherSegs(FarePath& fpath, std::vector<TravelSeg*>& segmentsProcessed)
{
  Itin& itin = *fpath.itin();

  AirSeg* airSeg = nullptr;
  FareUsage* fu = nullptr;
  uint16_t tvlSegIndex = 0;
  for (TravelSeg* travelSeg : itin.travelSeg())
  {
    if (segmentsProcessed.end() !=
        std::find(segmentsProcessed.begin(), segmentsProcessed.end(), travelSeg))
      continue;

    segmentsProcessed.push_back(travelSeg);

    airSeg = travelSeg->toAirSeg();

    if (airSeg == nullptr)
      continue;

    if (!airSeg->localJourneyCarrier())
      continue;

    fu = farepathutils::getFareUsage(fpath, travelSeg, tvlSegIndex);
    if (fu == nullptr || !checkLocalAvailStatus(fu, travelSeg, tvlSegIndex))
      return false;

    bool checkBookedAvail = checkBookedClassAvail(travelSeg);
    if (!realAvailLocal(fu, tvlSegIndex, travelSeg, checkBookedAvail))
      return false;
  }
  return true;
}

bool
JourneyValidator::processingKeepFare(const FareUsage& fu)
{
  return _trx.excTrxType() == PricingTrx::AR_EXC_TRX && fu.isKeepFare();
}
} // tse namespace
