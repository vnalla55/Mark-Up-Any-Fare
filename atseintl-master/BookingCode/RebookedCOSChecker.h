//----------------------------------------------------------------------------
//  Copyright Sabre 2014
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

#pragma once

#include "DataModel/PaxTypeFare.h"

namespace tse
{

class AirSeg;
class ClassOfService;
class PricingTrx;
class TravelSeg;
class FareUsage;

class FBCVAvailabilityGetter
{
public:
  virtual ~FBCVAvailabilityGetter() = default;

  virtual const std::vector<ClassOfService*>* getAvailability(TravelSeg* travelSeg,
                                                              PaxTypeFare::SegmentStatus& segStat,
                                                              PaxTypeFare& paxTypeFare,
                                                              const uint16_t segIdx) = 0;
};

class RebookedCOSChecker
{
public:
  RebookedCOSChecker(PricingTrx& trx,
                     PaxTypeFare& paxTfare,
                     FareMarket& mkt,
                     FareUsage* fu,
                     FBCVAvailabilityGetter& fbcValidator,
                     bool statusTypeNotFlowForLocalJrnyCxr)
    : _trx(trx),
      _paxTfare(paxTfare),
      _mkt(mkt),
      _fu(fu),
      _fbcValidator(fbcValidator),
      _statusTypeNotFlowForLocalJrnyCxr(statusTypeNotFlowForLocalJrnyCxr)
  {
  }

  virtual ~RebookedCOSChecker() = default;

  bool checkRebookedCOS();

protected:
  virtual uint16_t getNumSeats();

  virtual PaxTypeFare::SegmentStatus& getSegmentStatus(size_t segIdx);

  virtual const std::vector<ClassOfService*>*
  getCosVec(AirSeg* airSeg, PaxTypeFare::SegmentStatus& segStat, size_t segIdx);

  virtual BookingCode getBookingCode(AirSeg* airSeg, PaxTypeFare::SegmentStatus& segStat);

  friend class RebookedCOSCheckerTest;

private:
  PricingTrx& _trx;
  PaxTypeFare& _paxTfare;
  FareMarket& _mkt;
  FareUsage* _fu = nullptr;
  FBCVAvailabilityGetter& _fbcValidator;
  bool _statusTypeNotFlowForLocalJrnyCxr;
};
} // namespace tse
