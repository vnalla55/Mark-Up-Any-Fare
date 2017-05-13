// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#ifndef S7BUILDER_H
#define S7BUILDER_H

#include "DBAccess/OptionalServicesInfo.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class S7Builder
{
  OptionalServicesInfo* _s7;
  TestMemHandle* _memHandle;

public:
  S7Builder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _s7 = _memHandle->create<OptionalServicesInfo>();
  }

  S7Builder& withBaggagePcs(int32_t pieces)
  {
    _s7->freeBaggagePcs() = pieces;
    return *this;
  }

  S7Builder& withBaggageWeight(int32_t weight, const Indicator& unit)
  {
    _s7->baggageWeight() = weight;
    _s7->baggageWeightUnit() = unit;
    return *this;
  }

  S7Builder& withBaggageWeight(int32_t weight)
  {
    _s7->baggageWeight() = weight;
    return *this;
  }

  S7Builder& withBaggageWeightUnit(const Indicator& unit)
  {
    _s7->baggageWeightUnit() = unit;
    return *this;
  }

  S7Builder& withAndOr(const Indicator& ind)
  {
    _s7->andOrInd() = ind;
    return *this;
  }

  S7Builder& withSequenceNo(uint32_t seqNo)
  {
    _s7->seqNo() = seqNo;
    return *this;
  }

  S7Builder& withTaxTblItemNo(uint32_t taxTblItemNo)
  {
    _s7->taxTblItemNo() = taxTblItemNo;
    return *this;
  }

  S7Builder& withFirstOccurrence(int32_t firstPiece)
  {
    _s7->baggageOccurrenceFirstPc() = firstPiece;
    return *this;
  }

  S7Builder& withBaggageOccurrence(int32_t firstPiece, int32_t lastPiece)
  {
    _s7->baggageOccurrenceFirstPc() = firstPiece;
    _s7->baggageOccurrenceLastPc() = lastPiece;
    return *this;
  }

  S7Builder& withOccurrence(uint16_t first, uint16_t last)
  {
    _s7->firstOccurence() = first;
    _s7->lastOccurence() = last;
    return *this;
  }

  S7Builder& withApplication(const Indicator& application)
  {
    _s7->frequentFlyerMileageAppl() = application;
    return *this;
  }

  S7Builder& withStatus(uint16_t status)
  {
    _s7->frequentFlyerStatus() = status;
    return *this;
  }

  S7Builder& withNotAvailNoCharge(const Indicator notAvail)
  {
    _s7->notAvailNoChargeInd() = notAvail;
    return *this;
  }

  S7Builder& withAllowanceMatched(const CarrierCode& carrier, const Indicator& notAvailNoChargeInd);

  S7Builder& withFltTktMerchInd(const Indicator& ftmi)
  {
    _s7->fltTktMerchInd() = ftmi;
    return *this;
  }

  S7Builder& withVendor(const VendorCode& vendor)
  {
    _s7->vendor() = vendor;
    return *this;
  }

  S7Builder& withCarrier(const CarrierCode& carrier)
  {
    _s7->carrier() = carrier;
    return *this;
  }

  S7Builder& withBTA(const Indicator& bta)
  {
    _s7->baggageTravelApplication() = bta;
    return *this;
  }

  S7Builder& withVendCarr(const VendorCode& vendor, const CarrierCode& carrier)
  {
    _s7->vendor() = vendor;
    _s7->carrier() = carrier;
    return *this;
  }

  S7Builder& withSubTypeCode(const ServiceSubTypeCode& subType)
  {
    _s7->serviceSubTypeCode() = subType;
    return *this;
  }

  S7Builder& withStartDate(int32_t y, int32_t m, int32_t d)
  {
    _s7->tvlStartYear() = y;
    _s7->tvlStartMonth() = m;
    _s7->tvlStartDay() = d;
    return *this;
  }

  S7Builder& withStopDate(int32_t y, int32_t m, int32_t d)
  {
    _s7->tvlStopYear() = y;
    _s7->tvlStopMonth() = m;
    _s7->tvlStopDay() = d;
    return *this;
  }

  S7Builder& withStartTime(int16_t start)
  {
    _s7->startTime() = start;
    return *this;
  }

  S7Builder& withStopTime(int16_t stop)
  {
    _s7->stopTime() = stop;
    return *this;
  }

  S7Builder& withTime(int16_t start, int16_t stop)
  {
    _s7->startTime() = start;
    _s7->stopTime() = stop;
    return *this;
  }

  S7Builder& withDow(const DayOfWeekCode& dow)
  {
    _s7->dayOfWeek() = dow;
    return *this;
  }

  S7Builder& withDiscDate(Year y, Month m, Day d)
  {
    _s7->ticketDiscDate() = DateTime(y, m, d);
    return *this;
  }

  S7Builder& withEffDate(Year y, Month m, Day d)
  {
    _s7->ticketEffDate() = DateTime(y, m, d);
    return *this;
  }

  S7Builder& withEquipment(const EquipmentType& code)
  {
    _s7->equipmentCode() = code;
    return *this;
  }

  S7Builder& withTour(const TourCode& code)
  {
    _s7->tourCode() = code;
    return *this;
  }

  S7Builder& withTimeApplication(const Indicator time)
  {
    _s7->timeApplication() = time;
    return *this;
  }

  S7Builder& withSectorPortion(const Indicator ind)
  {
    _s7->sectorPortionInd() = ind;
    return *this;
  }

  S7Builder& withViaLoc(const LocKey& vialoc)
  {
    _s7->viaLoc() = vialoc;
    return *this;
  }

  S7Builder& withLocations(const LocCode& loc1, const LocCode& loc2, const LocCode& vialoc)
  {
    _s7->loc1().loc() = loc1;
    _s7->loc2().loc() = loc2;
    _s7->viaLoc().loc() = vialoc;
    return *this;
  }

  S7Builder& withStopCnxDest(const Indicator& t)
  {
    _s7->stopCnxDestInd() = t;
    return *this;
  }

  S7Builder& withStopoverTime(const StopoverTime& t)
  {
    _s7->stopoverTime() = t;
    return *this;
  }

  S7Builder& withAppFee(uint32_t n)
  {
    _s7->applicationFee() = n;
    return *this;
  }

  S7Builder& withFeesSec(uint32_t n)
  {
    _s7->serviceFeesSecurityTblItemNo() = n;
    return *this;
  }

  S7Builder& withFeesCurr(uint32_t n)
  {
    _s7->serviceFeesCurrencyTblItemNo() = n;
    return *this;
  }

  S7Builder& withAdvPurch(const Indicator& t)
  {
    _s7->advPurchTktIssue() = t;
    return *this;
  }

  S7Builder& withTaxIncl(const Indicator& t)
  {
    _s7->taxInclInd() = t;
    return *this;
  }

  S7Builder& withAvail(const Indicator& t)
  {
    _s7->availabilityInd() = t;
    return *this;
  }

  S7Builder& withRuleBuster(const RuleBusterFcl& t)
  {
    _s7->ruleBusterFcl() = t;
    return *this;
  }

  S7Builder& withPubPriv(const Indicator& ind)
  {
    _s7->publicPrivateInd() = ind;
    return *this;
  }

  OptionalServicesInfo* build() const { return _s7; }

  OptionalServicesInfo& buildRef() const { return *_s7; }
};
} // tse
#endif // S7BUILDER_H
