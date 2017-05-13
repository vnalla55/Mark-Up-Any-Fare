//-------------------------------------------------------------------
//  Copyright Sabre 2010
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/PaxType.h"
#include "DBAccess/FreqFlyerStatusSeg.h"

#include <array>
#include <vector>

namespace tse
{
class FarePath;
class Itin;
class PricingTrx;
class TravelSeg;
class BaggageCharge;
class OCFees;
class FreqFlyerStatusSeg;
enum CheckedPointType
{
  CP_AT_ORIGIN,
  CP_AT_DESTINATION
};

using CheckedPoint = std::pair<std::vector<TravelSeg*>::const_iterator, CheckedPointType>;
using CheckedPointVector = std::vector<CheckedPoint>;

using TravelSegPtrVecCI = std::vector<TravelSeg*>::const_iterator;
using ChargeVector = std::vector<BaggageCharge*>;
using EmbargoVector = std::vector<OCFees*>;

template<class T>
using PerBagPiece = std::array<T, MAX_BAG_PIECES>;
using BaggageCharges = PerBagPiece<BaggageCharge*>;
using FreqFlyerStatuses = std::vector<std::pair<const CarrierCode, const uint16_t>>;

struct BaggageTravel
{
  BaggageTravel() { _charges.fill(nullptr); }

  void clone(const BaggageTravel& original, FarePath* farePath = nullptr);

  void setupTravelData(FarePath& fp);
  FarePath* farePath() const { return _farePath; }

  void setItin(Itin& itin) {_itin = &itin; }
  Itin* itin() const { return _itin; }

  void setPaxType(PaxType& paxType) { _paxType = &paxType; }
  PaxType* paxType() const { return _paxType; }

  const TravelSegPtrVecCI& getTravelSegBegin() const { return _travelSegBegin; }
  const TravelSegPtrVecCI& getTravelSegEnd() const { return _travelSegEnd; }
  const size_t getNumTravelSegs() const { return _travelSegEnd - _travelSegBegin; }

  const TravelSeg* getMsSeg(bool usDot) const { return usDot ? *_MSSJourney : *_MSS; }
  const TravelSeg* getFciSeg(bool usDot) const;

  void updateSegmentsRange(const TravelSegPtrVecCI& travelSegBegin,
                           const TravelSegPtrVecCI& travelSegEnd);

  bool shouldAttachToDisclosure() const;

  inline bool isBaggageCharge1stOr2ndBag(const BaggageCharge* bc) const;

  uint16_t getFreqFlyerTierLevel() const { return _frequentFlyerTierLevel; }
  void setFreqFlyerTierLevel(const uint16_t frequentFlyerTierLevel)
  {
    _frequentFlyerTierLevel = frequentFlyerTierLevel;
  }

  CarrierCode getCarrier() const { return _carrier; }
  void setCarrier(const CarrierCode carrier) { _carrier = carrier; }

  PricingTrx* _trx = nullptr;
  TravelSegPtrVecCI _MSS;
  TravelSegPtrVecCI _MSSJourney;
  int64_t _stopOverLength = 0;

  const TravelSeg* _carrierTravelSeg = nullptr;
  CarrierCode _allowanceCxr; // same should be used for charges
  OCFees* _allowance = nullptr;
  BaggageCharges _charges;
  ChargeVector _chargeVector;
  EmbargoVector _embargoVector;

  uint16_t _frequentFlyerTierLevel = FF_LEVEL_NOT_DETERMINED;
  bool _chargeS5Available = false;
  bool _processCharges = true;
  bool _defer = false;

private:
  FarePath* _farePath = nullptr;
  Itin* _itin = nullptr;
  PaxType* _paxType = nullptr;
  TravelSegPtrVecCI _travelSegBegin;
  TravelSegPtrVecCI _travelSegEnd;
  CarrierCode _carrier;
  bool _containsUnflownSegment = true;
  bool _containsSelectedSegment = true;
};

bool
BaggageTravel::isBaggageCharge1stOr2ndBag(const BaggageCharge* bc) const
{
  return bc && ((_charges[0] && bc->optFee() == _charges[0]->optFee()) ||
                (_charges[1] && bc->optFee() == _charges[1]->optFee()));
}

} // tse
