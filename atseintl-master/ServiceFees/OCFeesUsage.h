//-------------------------------------------------------------------
//
//  File:        OCFeesUsage.h
//
//  Copyright Sabre 2012
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

#include "Common/SmallBitSet.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "ServiceFees/OCFees.h"

#include <vector>

namespace tse
{
class OCFees;
class FarePath;
class TravelSeg;
class SubCodeInfo;
class OptionalServicesInfo;
class CarrierFlightSeg;
class SvcFeesCxrResultingFCLInfo;
class SvcFeesResBkgDesigInfo;

class OCFeesUsage
{
public:
  // ctors
  OCFeesUsage() : _oCFees(nullptr), _index(0), _padisData(nullptr) {}
  ~OCFeesUsage() {}

  // OCFeesUsage accessors
  const OCFees* oCFees() const { return _oCFees; }
  OCFees*& oCFees() { return _oCFees; }

  size_t getSegIndex() const { return _index; }
  void setSegIndex(size_t index) { _index = index; }

  SvcFeesResBkgDesigInfo*& upgradeT198Sequence() { return _padisData; }
  const SvcFeesResBkgDesigInfo* upgradeT198Sequence() const { return _padisData; }

  std::string& upgradeT198CommercialName() { return _commercialNameForDisplay; }
  const std::string& upgradeT198CommercialName() const { return _commercialNameForDisplay; }

  // OCFees accessors
  const CarrierCode& carrierCode() const { return _oCFees->carrierCode(); }

  const TravelSeg* travelStart() const { return _oCFees->travelStart(); }

  const TravelSeg* travelEnd() const { return _oCFees->travelEnd(); }

  const SubCodeInfo* subCodeInfo() const { return _oCFees->subCodeInfo(); }

  const std::vector<OCFees::TaxItem>& getTaxes() const
  {
    return _oCFees->getSegPtr(_index)->_taxes;
  }

  const uint32_t& baggageTravelIndex() const { return _oCFees->baggageTravelIndex(); }

  const OptionalServicesInfo* optFee() const { return _oCFees->getSegPtr(_index)->_optFee; }

  const MoneyAmount feeAmount() const { return _oCFees->getSegPtr(_index)->_feeAmount; }

  const CurrencyCode& feeCurrency() const { return _oCFees->getSegPtr(_index)->_feeCurrency; }

  const MoneyAmount displayAmount() const { return _oCFees->getSegPtr(_index)->_displayAmount; }

  const CurrencyCode& displayCurrency() const
  {
    return _oCFees->getSegPtr(_index)->_displayCurrency;
  }

  const CurrencyNoDec& feeNoDec() const { return _oCFees->getSegPtr(_index)->_feeNoDec; }

  const FarePath*& farePath() { return _oCFees->getSegPtr(_index)->_farePath; }
  const FarePath* farePath() const { return _oCFees->getSegPtr(_index)->_farePath; }

  bool isDisplayOnly() const { return _oCFees->getSegPtr(_index)->_displayOnly; }

  bool isFeeGuaranteed() const { return _oCFees->getSegPtr(_index)->_feeGuaranteed; }

  DateTime purchaseByDate() const { return _oCFees->getSegPtr(_index)->_purchaseByDate; }

  LocCode matchedOriginAirport() const { return _oCFees->getSegPtr(_index)->_matchOriginAirport; }

  LocCode matchedDestinationAirport() const
  {
    return _oCFees->getSegPtr(_index)->_matchDestinationAirport;
  }

  // soft Match status and accessories

  bool isAnyS7SoftPass() const
  {
    return _oCFees->getSegPtr(_index)->_softMatchS7Status.value() != 0;
  }

  const OCFees::SoftMatchS7Status& softMatchS7Status() const
  {
    return _oCFees->getSegPtr(_index)->_softMatchS7Status;
  }

  const std::vector<SvcFeesCxrResultingFCLInfo*>& softMatchResultingFareClassT171() const
  {
    return _oCFees->getSegPtr(_index)->_resultingFareClass;
  }
  std::vector<SvcFeesCxrResultingFCLInfo*>& softMatchResultingFareClassT171()
  {
    return _oCFees->getSegPtr(_index)->_resultingFareClass;
  }

  const std::vector<CarrierFlightSeg*>& softMatchCarrierFlightT186() const
  {
    return _oCFees->getSegPtr(_index)->_carrierFlights;
  }
  std::vector<CarrierFlightSeg*>& softMatchCarrierFlightT186()
  {
    return _oCFees->getSegPtr(_index)->_carrierFlights;
  }

  const std::vector<SvcFeesResBkgDesigInfo*>& softMatchRBDT198() const
  {
    return _oCFees->getSegPtr(_index)->_rbdData;
  }
  std::vector<SvcFeesResBkgDesigInfo*>& softMatchRBDT198()
  {
    return _oCFees->getSegPtr(_index)->_rbdData;
  }

  const std::vector<OCFees::BaggageItemProperty>& getBaggageProperty() const
  {
    return _oCFees->getSegPtr(_index)->_baggageItemProperties;
  }
  std::vector<OCFees::BaggageItemProperty>& getBaggageProperty()
  {
    return _oCFees->getSegPtr(_index)->_baggageItemProperties;
  }

  const std::vector<SvcFeesResBkgDesigInfo*>& padisData() const
  {
    return _oCFees->getSegPtr(_index)->_padisData;
  }
  std::vector<SvcFeesResBkgDesigInfo*>& padisData()
  {
    return _oCFees->getSegPtr(_index)->_padisData;
  }

  bool
  isBackingOutTaxes() const { return static_cast<bool>(_oCFees->getSegPtr(_index)->isBackingOutTaxes()); }

  const OCFees::BackingOutTaxes&
  getBackingOutTaxes() const { return _oCFees->getSegPtr(_index)->getBackingOutTaxes(); }

  const AncillaryIdentifier&
  getAncillaryPriceIdentifier() const { return _oCFees->getSegPtr(_index)->_ancPriceModification.get().first; }

  const AncillaryPriceModifier&
  getAncillaryPriceModifier() const { return _oCFees->getSegPtr(_index)->_ancPriceModification.get().second; }

  bool hasPriceModification() { return (bool)_oCFees->getSegPtr(_index)->_ancPriceModification; }

private:
  OCFees* _oCFees;
  size_t _index;
  SvcFeesResBkgDesigInfo* _padisData;
  std::string _commercialNameForDisplay;
};

} // tse

