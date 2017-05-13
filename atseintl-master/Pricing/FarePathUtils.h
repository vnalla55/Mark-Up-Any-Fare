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

#pragma once

#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/PUPath.h"

namespace tse
{
class DiagCollector;
class DifferentialData;
class FarePath;
class FarePathFactoryFailedPricingUnits;
class FarePathFactoryStorage;
class PaxTypeFare;
class PricingTrx;
class PricingUnit;
class PUPath;

enum JourneyDiagMsgType
{ FAIL_USE_FLOW_AVAIL = 1,
  FAIL_FU_NOT_FOUND,
  FAIL_BKS_FAIL_SET,
  FAIL_NOT_PASS_LOCAL_MIX,
  FAIL_START_FLOW,
  FAIL_FOUND_FLOW,
  PASS_JOURNEY,
  INFO_SAME_BC,
  INFO_FLOW_AVAIL_FAIL,
  INFO_FLOW_AVAIL_PASS,
  INFO_TRY_LOCAL_AVAIL,
  FAIL_JOURNEY_SHOPPING,
  PASS_JOURNEY_SHOPPING,
  FAIL_NEED_SET_REBOOK,
  PASS_OTHER_SEGS,
  FAIL_OTHER_SEGS };

namespace farepathutils
{
bool
failedFareExistsInPU(const PaxTypeFare* const ptf, const PricingUnit& prU);

void
printFarePathToDiag(PricingTrx& trx, DiagCollector& diag, FarePath& farePath, PUPath* puPath);

bool
putDiagMsg(PricingTrx& trx, FPPQItem& fppqItem, DiagCollector& diag, uint32_t fpCombTried);

void
copyPUPathEOEInfo(FarePath& fPath, const PUPath* puPath);

void
clearMainTripSideTripLink(FarePath& farePath);

void
addMainTripSideTripLink(FPPQItem& fppqItem);

void
addMainTripSideTripLink(FarePath& farePath, PUPath* puPath);

void
findPUFactoryIdx(const FarePath& fpath,
                 const FareUsage* const failedSourceFareUsage,
                 const FareUsage* const failedTargetFareUsage,
                 int32_t& puFactIdx1,
                 int32_t& puFactIdx2);

void
jDiagInfo(DiagCollector& diag,
          const FareMarket& fm,
          JourneyDiagMsgType diagMsg,
          const bool journeyDiagOn);
void
jDiagDisplayFlights(DiagCollector& diag, const std::vector<TravelSeg*>& segmentsProcessed);

void
journeyDiagSegStat(PaxTypeFare::SegmentStatus& segStat, DiagCollector& diag, AirSeg* airSeg);

bool
journeyDiagShowStatus(FarePath& fp, DiagCollector& diag, bool beforeJourney);

void
jDiag(DiagCollector& diag,
      std::vector<TravelSeg*>& segmentsProcessed,
      JourneyDiagMsgType diagMsg,
      bool journeyDiagOn);

bool
journeyDiag(PricingTrx& trx, FarePath& fp, DiagCollector& diag);

FareUsage*
getFareUsage(FarePath& fpath, TravelSeg* tvlSeg, uint16_t& tvlSegIndex);

void
setPriority(PricingTrx& trx, const PUPQItem& pupqItem, FPPQItem& fppqItem, Itin* itin);

DifferentialData*
differentialData(const FareUsage* fu, const TravelSeg* tvlSeg);

const PaxTypeFare::SegmentStatus&
diffSegStatus(const DifferentialData* diff, const TravelSeg* tvlSeg);

std::string
displayValCxr(const std::vector<CarrierCode>& vcList);

void
setFailedPricingUnits(const uint16_t puFactIdx,
                      FPPQItem& fppqItem,
                      FarePathFactoryFailedPricingUnits& failedPricingUnits);
void
copyReusablePUToMutablePU(FarePathFactoryStorage& storage, FPPQItem& fppqItem);

uint16_t
fareComponentCount(const FarePath& farePath);

bool
checkMotherAvailability(const FarePath* farePath,
                        const uint16_t numSeatsRequired);

bool
checkSimilarItinAvailability(const FarePath* farePath,
                             const uint16_t numSeatsRequired,
                             const Itin& motherItin);
} // farepathutils namespace
} // tse namespace
