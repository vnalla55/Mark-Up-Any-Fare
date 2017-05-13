//----------------------------------------------------------------------------
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
#pragma once

#include "DataModel/PaxTypeFare.h"


namespace tse
{

class PricingTrx;
class FareMarket;
class Itin;
class TravelSeg;
class FareBookingCodeValidator;
class DiagCollector;
class FarePath;
class FareUsage;
class PricingUnit;
class CarrierPreference;

using SegmentStatusVec = std::vector<PaxTypeFare::SegmentStatus>;
using SegmentStatusVecI = std::vector<PaxTypeFare::SegmentStatus>::iterator;
using SegmentStatusVecCI = std::vector<PaxTypeFare::SegmentStatus>::const_iterator;

using PaxTypeFarePtrVec = std::vector<PaxTypeFare*>;
using PaxTypeFarePtrVecI = std::vector<PaxTypeFare*>::iterator;
using PaxTypeFarePtrVecIC = std::vector<PaxTypeFare*>::const_iterator;

using COSPtrVec = std::vector<std::vector<ClassOfService*>*>;
using COSPtrVecI = std::vector<std::vector<ClassOfService*>*>::iterator;
using COSPtrVecIC = std::vector<std::vector<ClassOfService*>*>::const_iterator;

using COSInnerPtrVecIC = std::vector<ClassOfService*>::const_iterator;
using TravelSegPtrVecIC = std::vector<tse::TravelSeg*>::const_iterator;

/**
 *      This is a driver for the differential processing and class of service processing
 *  on the international markets when the RBD validation was failed.
 *      It will loop through all fares on the FareMarket and check all flight sectors for "fail"
 *  status of RBD validation. And, it will determine which of these applicatios must be
 *  processed and the order in which these applications must occur.
 *
 */

class MixedClassController
{

  friend class MixedClassControllerTest;

public:
  MixedClassController(PricingTrx& trx) : _mccTrx(trx) {}
  virtual ~MixedClassController() = default;

  MixedClassController(const MixedClassController&) = delete;
  MixedClassController& operator=(const MixedClassController&) = delete;

  enum FailMix
  { FAIL_NONE,
    FAIL_DIFF };

  /*---------------------------------------------------------------------------
   * Analize Failed Booking Code Return Type
   *-------------------------------------------------------------------------*/
  enum FailedBkgCodeEnum
  { DIFFERENTIAL = 1,
    HARD_FAIL,
    ERROR };

  //  The MixedClass validation process for the International Fare Market invokes by this method.
  //  This is the general validation method called by FVO for the international FareMarket only.

  void validate(FareMarket& mkt, Itin& itin);

  bool validate(FarePath& fp, PricingUnit& pu);

  FailedBkgCodeEnum serviceDetermination(FareMarket*, PaxTypeFare*);

  const PaxTypeFare* paxTfare() const { return _paxTfare; }
  const FailMix failReason() const { return _failMixed; }

private:
  enum MccDiagMsgs
  { INVALID_CABIN = 1,
    DOMESTIC_FM,
    PRIME_RBD_FAIL,
    PRIME_RBD_PASS,
    PRIME_RBD_PASS2,
    MIX_PASS,
    PRIME_RBD_FAIL1,
    CAT31_KEEP_IGNORE_FAIL,
    CAT31_KEEP_PROCESSED_EARLIER,
    CAT31_PRIME_RBD_FC,
    CAT31_PRIME_RBD_PU };

  enum ProcessMixReturnType
  { FAIL_FARE_PATH = 1,
    CHECK_NEXT_FU,
    PROCESS_MIX_CLASS };

  bool validate(FarePath& fp, PricingUnit& pu, Itin& itin);

  // Mixed class Booking Code validation for the Normal or Special Fare being validated
  // (International).

  // Before go to validate for Differential, analize Failed
  // Booking
  // codes for the cabin

  FailedBkgCodeEnum serviceDetermination() const;

  ProcessMixReturnType shouldProcessMixClass();

  ProcessMixReturnType shouldProcessMixClassCat31() const;

  bool processSingleEntity();

  bool allValidCabins() const;

  void fuPass();

  void createDiag();
  void diagFm() const;
  void diag(MccDiagMsgs diagMsg) const;
  void diagFuFail(FareUsage& fu) const;
  void endDiag();

  PaxTypeFare::BookingCodeStatus& bookingCodeStatus() const;

  bool validatingCat31KeepFare() const;

  bool allSegsPassBookingCode() const;

  void collectHighCabin(CabinType cabin) const;

  bool validForDiffCalculation();

  DiagCollector* _diag = nullptr;

protected:
  virtual const CarrierPreference* getCarrierPreference();

  PricingTrx& _mccTrx;

  FareMarket* _mkt = nullptr;
  PaxTypeFare* _paxTfare = nullptr;

  PaxType* _paxType = nullptr;
  Itin* _itin = nullptr;
  FarePath* _farePath = nullptr;
  FareUsage* _fareUsage = nullptr;
  PricingUnit* _pricingUnit = nullptr;
  FailMix _failMixed = FailMix::FAIL_NONE;
  FareBookingCodeValidator* _fbcv = nullptr;

  mutable bool _isHighCabin4 = false;
  mutable bool _isHighCabin7 = false;
};
} // end tse namespace
