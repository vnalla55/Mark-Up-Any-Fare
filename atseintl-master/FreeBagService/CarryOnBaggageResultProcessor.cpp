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
#include "FreeBagService/CarryOnBaggageResultProcessor.h"

#include "Common/TrxUtil.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxText.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FreeBagService/CarryOnBaggageTextFormatter.h"
#include "ServiceFees/OCFees.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <tuple>

#include <boost/foreach.hpp>
#include <boost/regex.hpp>

namespace tse
{
CarryOnBaggageResultProcessor::CarryOnBaggageResultProcessor(PricingTrx& trx) : _trx(trx) {}

void
CarryOnBaggageResultProcessor::processAllowance(const std::vector<FarePath*>& farePaths) const
{
  for (FarePath* farePath : farePaths)
    farePath->baggageResponse() += formatAllowanceText(farePath->baggageTravelsPerSector());
}

void
CarryOnBaggageResultProcessor::processCharges(const std::vector<FarePath*>& farePaths) const
{
  for (FarePath* farePath : farePaths)
    farePath->baggageResponse() += formatChargesText(farePath->baggageTravelsPerSector());
}

namespace
{

class TaxTblItemNoSetComparator : std::unary_function<const std::set<uint32_t>, bool>
{
  uint32_t _taxTblItemNo;

public:
  TaxTblItemNoSetComparator(uint32_t taxTblItemNo) : _taxTblItemNo(taxTblItemNo) {}

  bool operator()(const std::set<uint32_t> taxTblItemNoSet)
  {
    return find(taxTblItemNoSet.begin(), taxTblItemNoSet.end(), _taxTblItemNo) !=
           taxTblItemNoSet.end();
  }
};

class SubCodeMapComparator : std::binary_function<std::map<ServiceSubTypeCode, int>*,
                                                  std::map<ServiceSubTypeCode, int>*,
                                                  bool>
{

public:
  bool operator()(const std::map<ServiceSubTypeCode, int>& first,
                  const std::map<ServiceSubTypeCode, int>& second) const
  {
    return first < second;
  }
};

class AllowanceComparator
    : std::unary_function<const std::pair<const BaggageTravel*, std::string>&, bool>
{
  const BaggageTravel* _baggageTravel;
  std::vector<std::set<uint32_t> > _taxTblNoSets;

public:
  AllowanceComparator(const BaggageTravel* baggageTravel,
                      const std::vector<std::set<uint32_t> >& taxTblNoSets)
    : _baggageTravel(baggageTravel), _taxTblNoSets(taxTblNoSets)
  {
  }

  bool operator()(const CarryOnBaggageTextFormatter::TravelDisclosureData& travelData) const
  {
    if (travelData.getBagTravel() == nullptr || _baggageTravel == nullptr)
      return false;

    return compareOperatingCarrier(travelData.getBagTravel()) &&
           compareAllowance(travelData.getBagTravel());
  }

  bool compareOperatingCarrier(const BaggageTravel* bgTravel) const
  {
    const AirSeg* airSeg1 = static_cast<const AirSeg*>(*(_baggageTravel->_MSS));
    const AirSeg* airSeg2 = static_cast<const AirSeg*>(*(bgTravel->_MSS));

    if (airSeg1 && airSeg2)
      return airSeg1->operatingCarrierCode() == airSeg2->operatingCarrierCode();
    else
      return false;
  }

  bool isAllowanceS7(const BaggageTravel* bgTravel) const
  {
    return bgTravel->_allowance != nullptr && bgTravel->_allowance->optFee() != nullptr;
  }

  bool compareAllowance(const BaggageTravel* bgTravel) const
  {
    if (_baggageTravel->_allowance && _baggageTravel->_allowance->optFee() &&
        bgTravel->_allowance && bgTravel->_allowance->optFee())
    {
      const OptionalServicesInfo* s7first = _baggageTravel->_allowance->optFee();
      const OptionalServicesInfo* s7second = bgTravel->_allowance->optFee();

      return s7first->freeBaggagePcs() == s7second->freeBaggagePcs() &&
             s7first->baggageWeight() == s7second->baggageWeight() &&
             compareTaxText(s7first, s7second);
    }
    else
      return !isAllowanceS7(_baggageTravel) && !isAllowanceS7(bgTravel);
  }

  bool
  compareTaxText(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second) const
  {
    if (s7first->taxTblItemNo() == s7second->taxTblItemNo())
      return true;

    std::vector<std::set<uint32_t> >::const_iterator it =
        std::find_if(_taxTblNoSets.begin(),
                     _taxTblNoSets.end(),
                     TaxTblItemNoSetComparator(s7first->taxTblItemNo()));

    if (it == _taxTblNoSets.end())
      return false;

    return it->find(s7second->taxTblItemNo()) != it->end();
  }
};
}

void
CarryOnBaggageResultProcessor::getTaxTblNoSets(
    const std::vector<const BaggageTravel*>& baggageTravels,
    std::vector<std::set<uint32_t> >& taxTblItemNoSets) const
{
  if (baggageTravels.empty())
    return;

  std::map<uint32_t, const OptionalServicesInfo*> taxTblItemNos;
  for (const BaggageTravel* bgTravel : baggageTravels)
  {
    if (bgTravel && bgTravel->_allowance)
    {
      const OptionalServicesInfo* s7 = bgTravel->_allowance->optFee();
      if (s7)
        taxTblItemNos[s7->taxTblItemNo()] = s7;
    }
  }

  PricingTrx* trx = baggageTravels.front()->_trx;
  CarryOnBaggageTextFormatter carryOnTextFormatter(*trx);

  std::map<uint32_t, std::string> taxTblItemNoToCarryOnTextMap;

  const OptionalServicesInfo* s7;
  uint32_t taxTblItemNo;
  BOOST_FOREACH(std::tie(taxTblItemNo, s7), taxTblItemNos)
  {
    taxTblItemNoToCarryOnTextMap[taxTblItemNo] = carryOnTextFormatter.getDescriptionsText(s7);
  }

  std::map<std::string, std::set<uint32_t> > carryOnTextToItemNoMap;
  std::string carryOnText;
  BOOST_FOREACH(std::tie(taxTblItemNo, carryOnText), taxTblItemNoToCarryOnTextMap)
  {
    carryOnTextToItemNoMap[carryOnText].insert(taxTblItemNo);
  }

  std::set<uint32_t> taxTblItemNoSet;
  BOOST_FOREACH(std::tie(carryOnText, taxTblItemNoSet), carryOnTextToItemNoMap)
  {
    taxTblItemNoSets.push_back(taxTblItemNoSet);
  }
}

std::string
CarryOnBaggageResultProcessor::formatAllowanceText(
    const std::vector<const BaggageTravel*>& baggageTravels) const
{
  typedef std::vector<CarryOnBaggageTextFormatter::TravelDisclosureData> TravelForDisclosureList;

  TravelForDisclosureList travelForDisclosure;
  travelForDisclosure.reserve(baggageTravels.size());
  std::vector<std::set<uint32_t> > taxTblNoSets;
  getTaxTblNoSets(baggageTravels, taxTblNoSets);

  uint32_t checkedPortion = 1;
  for (const BaggageTravel* baggageTravel : baggageTravels)
  {
    if (baggageTravel->shouldAttachToDisclosure())
    {
      TravelForDisclosureList::iterator it =
          std::find_if(travelForDisclosure.begin(),
                       travelForDisclosure.end(),
                       AllowanceComparator(baggageTravel, taxTblNoSets));
      if (it == travelForDisclosure.end())
        travelForDisclosure.push_back(CarryOnBaggageTextFormatter::TravelDisclosureData(
            baggageTravel,
            CarryOnBaggageTextFormatter::getTravelText(baggageTravel),
            checkedPortion));
      else
        it->appendTravelTextAndPortion(
            CarryOnBaggageTextFormatter::SPACE +
                CarryOnBaggageTextFormatter::getTravelText(baggageTravel),
            checkedPortion);
    }
    ++checkedPortion;
  }

  if (travelForDisclosure.empty())
    return EMPTY_STRING();

  Diag852Collector* diag(nullptr);
  if (TrxUtil::isBaggage302GlobalDisclosureActivated(_trx))
  {
    DiagManager diagMgr(_trx, Diagnostic852);
    if (diagMgr.isActive())
    {
      diag = &static_cast<Diag852Collector&>(diagMgr.collector());
    }
  }

  std::string allowanceText =
      BaggageTextFormatter::CARRY_ON_ALLOWANCE + BaggageTextFormatter::NEW_LINE;
  CarryOnBaggageTextFormatter textFormatter(_trx);
  for (const CarryOnBaggageTextFormatter::TravelDisclosureData& data : travelForDisclosure)
    allowanceText += data.getTravelText() + textFormatter.formatCarryOnAllowanceText(data, diag);

  return allowanceText;
}

namespace
{
struct ChargesDisclosureDataEqual
    : std::binary_function<const BaggageCharge*, const BaggageCharge*, bool>
{
  bool operator()(const BaggageCharge* fstCharge, const BaggageCharge* sndCharge) const
  {
    if (fstCharge->subCodeInfo() == sndCharge->subCodeInfo())
    {
      const OptionalServicesInfo* fstS7 = fstCharge->optFee();
      const OptionalServicesInfo* sndS7 = sndCharge->optFee();
      if (fstS7 && sndS7)
      {
        return fstCharge->feeAmount() == sndCharge->feeAmount() &&
               fstCharge->feeCurrency() == sndCharge->feeCurrency() &&
               fstS7->baggageOccurrenceFirstPc() == sndS7->baggageOccurrenceFirstPc() &&
               fstS7->baggageOccurrenceLastPc() == sndS7->baggageOccurrenceLastPc() &&
               fstS7->baggageWeight() == sndS7->baggageWeight() &&
               fstS7->baggageWeightUnit() == sndS7->baggageWeightUnit() &&
               fstS7->notAvailNoChargeInd() == sndS7->notAvailNoChargeInd() &&
               fstS7->frequentFlyerMileageAppl() == sndS7->frequentFlyerMileageAppl();
      }
      else if (!fstS7 && !sndS7)
        return true;
    }
    return false;
  }
};

struct BaggageTravelsEqual : std::unary_function<const BaggageTravel*, bool>
{
  BaggageTravelsEqual(const BaggageTravel* baggageTravel) : _baggageTravel(baggageTravel) {}

  bool operator()(const std::pair<const BaggageTravel*, std::string>& baggageTravelPair) const
  {
    if (_baggageTravel->_chargeVector.empty() && baggageTravelPair.first->_chargeVector.empty())
    {
      return (CarryOnBaggageTextFormatter::getOperatingCarrierText(_baggageTravel) ==
              CarryOnBaggageTextFormatter::getOperatingCarrierText(baggageTravelPair.first));
    }
    else
    {
      return (
          (baggageTravelPair.first->_chargeVector.size() == _baggageTravel->_chargeVector.size()) &&
          std::equal(_baggageTravel->_chargeVector.begin(),
                     _baggageTravel->_chargeVector.end(),
                     baggageTravelPair.first->_chargeVector.begin(),
                     ChargesDisclosureDataEqual()));
    }
  }

private:
  const BaggageTravel* _baggageTravel;
};
}

std::string
CarryOnBaggageResultProcessor::formatChargesText(
    const std::vector<const BaggageTravel*>& baggageTravels) const
{
  typedef std::vector<std::pair<const BaggageTravel*, std::string> > TravelForDisclosureList;
  TravelForDisclosureList travelForDisclosure;
  travelForDisclosure.reserve(baggageTravels.size());
  for (const BaggageTravel* baggageTravel : baggageTravels)
  {
    if (baggageTravel->_processCharges && baggageTravel->shouldAttachToDisclosure())
    {
      TravelForDisclosureList::iterator it = std::find_if(travelForDisclosure.begin(),
                                                          travelForDisclosure.end(),
                                                          BaggageTravelsEqual(baggageTravel));
      if (it == travelForDisclosure.end())
        travelForDisclosure.push_back(std::make_pair(
            baggageTravel, CarryOnBaggageTextFormatter::getTravelText(baggageTravel)));
      else
        it->second += CarryOnBaggageTextFormatter::SPACE +
                      CarryOnBaggageTextFormatter::getTravelText(baggageTravel);
    }
  }

  if (travelForDisclosure.empty())
    return EMPTY_STRING();

  std::string chargesText = BaggageTextFormatter::CARRY_ON_CHARGES + BaggageTextFormatter::NEW_LINE;
  CarryOnBaggageTextFormatter textFormatter(_trx);
  const BaggageTravel* baggageTravel;
  std::string travelText;
  BOOST_FOREACH(std::tie(baggageTravel, travelText), travelForDisclosure)
  {
    chargesText += travelText + BaggageTextFormatter::DASH +
                   CarryOnBaggageTextFormatter::getOperatingCarrierText(baggageTravel);
    if (!baggageTravel->_chargeVector.empty())
    {
      std::map<const SubCodeInfo*, size_t> subCodeQuantities;
      gatherSubCodeQuantities(baggageTravel->_chargeVector, subCodeQuantities);
      chargesText += BaggageTextFormatter::NEW_LINE;
      for (const BaggageCharge* baggageCharge : baggageTravel->_chargeVector)
      {
        const bool singleS7Matched = subCodeQuantities[baggageCharge->subCodeInfo()] == 1;
        chargesText += textFormatter.formatCarryOnChargeText(baggageCharge, singleS7Matched) +
                       BaggageTextFormatter::NEW_LINE;
      }
    }
    else
    {
      chargesText += BaggageTextFormatter::DASH +
                     CarryOnBaggageTextFormatter::CARRY_ON_CHARGES_UNKNOWN +
                     BaggageTextFormatter::NEW_LINE;
    }
  }

  return chargesText;
}

void
CarryOnBaggageResultProcessor::gatherSubCodeQuantities(
    const ChargeVector& charges, std::map<const SubCodeInfo*, size_t>& quantities) const
{
  std::map<const SubCodeInfo*, size_t>::iterator it;
  for (const BaggageCharge* baggageCharge : charges)
  {
    it = quantities.find(baggageCharge->subCodeInfo());
    if (it != quantities.end())
      it->second++;
    else
      quantities.insert(std::make_pair(baggageCharge->subCodeInfo(), 1));
  }
}
} // tse
