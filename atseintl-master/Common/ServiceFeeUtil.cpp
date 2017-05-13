//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "Common/ServiceFeeUtil.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSEException.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Currency.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/Nation.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/PaxTypeMatrix.h"
#include "DBAccess/SeatCabinCharacteristicInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "DBAccess/TaxNation.h"
#include "FareCalc/CalcTotals.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include <algorithm>

namespace tse
{
FALLBACK_DECL(fallbackSSDMPS242);
FALLBACK_DECL(ocFeesAmountRoundingRefactoring);

namespace
{
Logger logger("atseintl.Common.ServiceFeeUtil");
}

std::string
getDescription(std::pair<BookingCode, std::string> element)
{
  return element.second;
}

TaxItemFeeCurrencyConverter::TaxItemFeeCurrencyConverter(ServiceFeeUtil& util) : _util(util) {}

MoneyAmount
TaxItemFeeCurrencyConverter::
operator()(const OCFees::TaxItem& taxItem) const
{
  return _util.convertMoney(taxItem.getTaxAmount(),
                            taxItem.getCurrency(),
                            _util.getSellingCurrency(),
                            CurrencyConversionRequest::TAXES).value();
}

void
ServiceFeeUtil::fill(std::set<BookingCode>& bookingCode, const SvcFeesResBkgDesigInfo* padis)
{
  if (!padis->bookingCode1().empty())
    bookingCode.insert(padis->bookingCode1());
  if (!padis->bookingCode2().empty())
    bookingCode.insert(padis->bookingCode2());
  if (!padis->bookingCode3().empty())
    bookingCode.insert(padis->bookingCode3());
  if (!padis->bookingCode4().empty())
    bookingCode.insert(padis->bookingCode4());
  if (!padis->bookingCode5().empty())
    bookingCode.insert(padis->bookingCode5());
}

void
ServiceFeeUtil::getPadisDescriptions(
    const std::set<BookingCode>& bookingCodeSet,
    const std::vector<SeatCabinCharacteristicInfo*>& seatCabinForCarrier,
    const std::vector<SeatCabinCharacteristicInfo*>& seatCabinAll,
    std::map<BookingCode, std::string>& codeDescriptionMap,
    std::map<BookingCode, std::string>& abbreviatedDescriptionMap)
{
  for (SeatCabinCharacteristicInfo* seatCabinCharacteristic : seatCabinAll)
  {
    if (bookingCodeSet.find(seatCabinCharacteristic->seatCabinCode()) != bookingCodeSet.end())
    {
      codeDescriptionMap[seatCabinCharacteristic->seatCabinCode()] =
          seatCabinCharacteristic->codeDescription();
      abbreviatedDescriptionMap[seatCabinCharacteristic->seatCabinCode()] =
          seatCabinCharacteristic->abbreviatedDescription();
    }
  }

  for (SeatCabinCharacteristicInfo* seatCabinCharacteristic : seatCabinForCarrier)
  {
    if (bookingCodeSet.find(seatCabinCharacteristic->seatCabinCode()) != bookingCodeSet.end())
    {
      codeDescriptionMap[seatCabinCharacteristic->seatCabinCode()] =
          seatCabinCharacteristic->codeDescription();
      abbreviatedDescriptionMap[seatCabinCharacteristic->seatCabinCode()] =
          seatCabinCharacteristic->abbreviatedDescription();
    }
  }
}

void
ServiceFeeUtil::getPadisDisplayDescriptions(
    const std::set<BookingCode>& bookingCodeSet,
    const std::vector<SeatCabinCharacteristicInfo*>& seatCabinForCarrier,
    const std::vector<SeatCabinCharacteristicInfo*>& seatCabinAll,
    std::map<BookingCode, std::string>& displayDescriptionMap,
    std::map<BookingCode, std::string>& abbreviatedDescriptionMap)
{
  for (SeatCabinCharacteristicInfo* seatCabinCharacteristic : seatCabinAll)
  {
    if (bookingCodeSet.find(seatCabinCharacteristic->seatCabinCode()) != bookingCodeSet.end())
    {
      displayDescriptionMap[seatCabinCharacteristic->seatCabinCode()] =
          seatCabinCharacteristic->displayDescription();
      abbreviatedDescriptionMap[seatCabinCharacteristic->seatCabinCode()] =
          seatCabinCharacteristic->abbreviatedDescription();
    }
  }

  for (SeatCabinCharacteristicInfo* seatCabinCharacteristic : seatCabinForCarrier)
  {
    if (bookingCodeSet.find(seatCabinCharacteristic->seatCabinCode()) != bookingCodeSet.end())
    {
      displayDescriptionMap[seatCabinCharacteristic->seatCabinCode()] =
          seatCabinCharacteristic->displayDescription();
      abbreviatedDescriptionMap[seatCabinCharacteristic->seatCabinCode()] =
          seatCabinCharacteristic->abbreviatedDescription();
    }
  }
}

std::string
ServiceFeeUtil::getPadisDescriptions(
    PricingTrx& pricingTrx,
    const std::set<BookingCode>& bookingCodeSet,
    const std::vector<SeatCabinCharacteristicInfo*>& seatCabinForCarrier,
    const std::vector<SeatCabinCharacteristicInfo*>& seatCabinAll)
{
  std::map<BookingCode, std::string> codeDescriptionMap;
  std::map<BookingCode, std::string> abbreviatedDescriptionMap;

  getPadisDisplayDescriptions(bookingCodeSet,
                              seatCabinForCarrier,
                              seatCabinAll,
                              codeDescriptionMap,
                              abbreviatedDescriptionMap);

  std::ostringstream codeDescriptionStream;
  std::ostringstream abbreviatedDescriptionStream;

  std::transform(codeDescriptionMap.begin(),
                 codeDescriptionMap.end(),
                 std::ostream_iterator<std::string>(codeDescriptionStream, " "),
                 getDescription);

  std::transform(abbreviatedDescriptionMap.begin(),
                 abbreviatedDescriptionMap.end(),
                 std::ostream_iterator<std::string>(abbreviatedDescriptionStream, " "),
                 getDescription);

  std::string codeDescription = codeDescriptionStream.str();
  boost::algorithm::trim(codeDescription);
  std::string abbreviatedDescription = abbreviatedDescriptionStream.str();
  boost::algorithm::trim(abbreviatedDescription);

  if (codeDescription.length() <= 30)
    return codeDescription;
  else
    return abbreviatedDescription;
}

bool
PaxBucketComparator::
operator()(const PaxTypeBucketItem& lhs, const PaxTypeBucketItem& rhs) const
{
  // Sort by :
  // 1. Number of items (desc)
  // 2. Money amount (asc)
  // 3. Adult pax type indicator (adult pax types first)
  // 4. Pax type code (alfabeticaly asc)

  if (lhs.numberOfItems > rhs.numberOfItems)
  {
    return true;
  }

  if (lhs.numberOfItems == rhs.numberOfItems)
  {
    if (lhs.moneyAmount < rhs.moneyAmount)
    {
      return true;
    }

    if (lhs.moneyAmount == rhs.moneyAmount)
    {
      if ((true == ServiceFeeUtil::isAdultPaxType(lhs.paxType)) &&
          (false == ServiceFeeUtil::isAdultPaxType(rhs.paxType)))
      {
        return true;
      }

      if (ServiceFeeUtil::isAdultPaxType(lhs.paxType) ==
          ServiceFeeUtil::isAdultPaxType(rhs.paxType))
      {
        if (lhs.paxType.paxType() < rhs.paxType.paxType())
        {
          return true;
        }
      }
    }
  }

  return false;
}

bool
TravelSegmentOrderComparator::
operator()(const TravelSeg* lhs, const TravelSeg* rhs) const
{
  return _itin.segmentOrder(lhs) < _itin.segmentOrder(rhs);
}

bool
FPOCFeesComparatorR7::
operator()(const PaxType::TktInfo* t1, const PaxType::TktInfo* t2) const
{
  return t1->tktRefNumber() < t2->tktRefNumber();
}

bool
FPOCFeesComparatorR7::
operator()(const FPOCFees& fee1, const FPOCFees& fee2) const
{
  int fee1Seg1Pos = fee1.fp()->itin()->segmentPnrOrder(fee1.fees()->travelStart());
  int fee1Seg2Pos = fee1.fp()->itin()->segmentPnrOrder(fee1.fees()->travelEnd());
  int fee2Seg1Pos = fee2.fp()->itin()->segmentPnrOrder(fee2.fees()->travelStart());
  int fee2Seg2Pos = fee2.fp()->itin()->segmentPnrOrder(fee2.fees()->travelEnd());

  if (fee1Seg1Pos != fee2Seg1Pos) // the same first seg
  {
    return fee1Seg1Pos < fee2Seg1Pos;
  }

  if (fee1Seg2Pos != fee2Seg2Pos) // the same second seg
  {
    return fee1Seg2Pos < fee2Seg2Pos;
  }

  if (fee1.fees()->subCodeInfo()->serviceSubTypeCode() !=
      fee2.fees()->subCodeInfo()->serviceSubTypeCode())
    return fee1.fees()->subCodeInfo()->serviceSubTypeCode() <
           fee2.fees()->subCodeInfo()->serviceSubTypeCode();

  // if any tkt info rpesent
  if (!fee1.fp()->paxType()->psgTktInfo().empty() || !fee2.fp()->paxType()->psgTktInfo().empty())
  {
    // if both fees with tkt info need perform comparsion
    if (!fee1.fp()->paxType()->psgTktInfo().empty() && !fee2.fp()->paxType()->psgTktInfo().empty())
    {
      std::string t1((*std::min_element(fee1.fp()->paxType()->psgTktInfo().begin(),
                                        fee1.fp()->paxType()->psgTktInfo().end(),
                                        *this))->tktRefNumber());
      std::string t2((*std::min_element(fee2.fp()->paxType()->psgTktInfo().begin(),
                                        fee2.fp()->paxType()->psgTktInfo().end(),
                                        *this))->tktRefNumber());
      if (t1 != t2)
        return t1 < t2;
    }
    else
      // first without tkt ?
      return fee1.fp()->paxType()->psgTktInfo().size() < !fee2.fp()->paxType()->psgTktInfo().size();
  }

  return fee1.fp()->paxType()->paxType() < fee2.fp()->paxType()->paxType();
}

bool
FPOCFeesComparator::
operator()(const FPOCFees& fee1, const FPOCFees& fee2) const
{
  int fee1Seg1Pos = fee1.fp()->itin()->segmentOrder(fee1.fees()->travelStart());
  int fee1Seg2Pos = fee1.fp()->itin()->segmentOrder(fee1.fees()->travelEnd());
  int fee2Seg1Pos = fee2.fp()->itin()->segmentOrder(fee2.fees()->travelStart());
  int fee2Seg2Pos = fee2.fp()->itin()->segmentOrder(fee2.fees()->travelEnd());

  if (fee1Seg1Pos != fee2Seg1Pos) // the same first seg
  {
    return fee1Seg1Pos < fee2Seg1Pos;
  }

  if (fee1Seg2Pos != fee2Seg2Pos) // the same second seg
  {
    return fee1Seg2Pos < fee2Seg2Pos;
  }

  if (fee1.fees()->subCodeInfo()->serviceSubTypeCode() ==
      fee2.fees()->subCodeInfo()->serviceSubTypeCode())
  {
    return fee1.fp()->paxType()->paxType() < fee2.fp()->paxType()->paxType();
  }
  else
  {
    return fee1.fees()->subCodeInfo()->serviceSubTypeCode() <
           fee2.fees()->subCodeInfo()->serviceSubTypeCode();
  }
}

// These methods below are for the OCFeesUsage development at Response build time

bool
FPOCFeesUsageComparatorR7::
operator()(const PaxType::TktInfo* t1, const PaxType::TktInfo* t2) const
{
  return t1->tktRefNumber() < t2->tktRefNumber();
}

bool
FPOCFeesUsageComparatorR7::
operator()(const FPOCFeesUsages& fee1, const FPOCFeesUsages& fee2) const
{
  int fee1Seg1Pos = fee1.fp()->itin()->segmentPnrOrder(fee1.fees()->travelStart());
  int fee1Seg2Pos = fee1.fp()->itin()->segmentPnrOrder(fee1.fees()->travelEnd());
  int fee2Seg1Pos = fee2.fp()->itin()->segmentPnrOrder(fee2.fees()->travelStart());
  int fee2Seg2Pos = fee2.fp()->itin()->segmentPnrOrder(fee2.fees()->travelEnd());

  if (fee1Seg1Pos != fee2Seg1Pos) // the same first seg
  {
    return fee1Seg1Pos < fee2Seg1Pos;
  }

  if (fee1Seg2Pos != fee2Seg2Pos) // the same second seg
  {
    return fee1Seg2Pos < fee2Seg2Pos;
  }

  if (fee1.fees()->subCodeInfo()->serviceSubTypeCode() !=
      fee2.fees()->subCodeInfo()->serviceSubTypeCode())
    return fee1.fees()->subCodeInfo()->serviceSubTypeCode() <
           fee2.fees()->subCodeInfo()->serviceSubTypeCode();

  // if any tkt info rpesent
  if (!fee1.fp()->paxType()->psgTktInfo().empty() || !fee2.fp()->paxType()->psgTktInfo().empty())
  {
    // if both fees with tkt info need perform comparsion
    if (!fee1.fp()->paxType()->psgTktInfo().empty() && !fee2.fp()->paxType()->psgTktInfo().empty())
    {
      std::string t1((*std::min_element(fee1.fp()->paxType()->psgTktInfo().begin(),
                                        fee1.fp()->paxType()->psgTktInfo().end(),
                                        *this))->tktRefNumber());
      std::string t2((*std::min_element(fee2.fp()->paxType()->psgTktInfo().begin(),
                                        fee2.fp()->paxType()->psgTktInfo().end(),
                                        *this))->tktRefNumber());
      if (t1 != t2)
        return t1 < t2;
    }
    else
      // first without tkt ?
      return fee1.fp()->paxType()->psgTktInfo().size() < !fee2.fp()->paxType()->psgTktInfo().size();
  }

  return fee1.fp()->paxType()->paxType() < fee2.fp()->paxType()->paxType();
}

bool
FPOCFeesUsageComparator::
operator()(const FPOCFeesUsages& fee1, const FPOCFeesUsages& fee2) const
{
  int fee1Seg1Pos = fee1.fp()->itin()->segmentOrder(fee1.fees()->travelStart());
  int fee1Seg2Pos = fee1.fp()->itin()->segmentOrder(fee1.fees()->travelEnd());
  int fee2Seg1Pos = fee2.fp()->itin()->segmentOrder(fee2.fees()->travelStart());
  int fee2Seg2Pos = fee2.fp()->itin()->segmentOrder(fee2.fees()->travelEnd());

  if (fee1Seg1Pos != fee2Seg1Pos) // the same first seg
  {
    return fee1Seg1Pos < fee2Seg1Pos;
  }

  if (fee1Seg2Pos != fee2Seg2Pos) // the same second seg
  {
    return fee1Seg2Pos < fee2Seg2Pos;
  }

  if (fee1.fees()->subCodeInfo()->serviceSubTypeCode() ==
      fee2.fees()->subCodeInfo()->serviceSubTypeCode())
  {
    return fee1.fp()->paxType()->paxType() < fee2.fp()->paxType()->paxType();
  }
  else
  {
    return fee1.fees()->subCodeInfo()->serviceSubTypeCode() <
           fee2.fees()->subCodeInfo()->serviceSubTypeCode();
  }
}

//

ServiceFeeUtil::ServiceFeeUtil(PricingTrx& trx) : _trx(trx) {}

bool
ServiceFeeUtil::isInternationalJourneyType(const Itin& itin)
{
  bool isInternational = false;

  TravelSeg* tsFirst = itin.travelSeg().front();
  TravelSegVecIC curSeg = itin.travelSeg().begin();
  TravelSegVecIC endS = itin.travelSeg().end();

  for (; curSeg != endS; curSeg++)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*curSeg);
    if (!airSeg || airSeg->segmentType() == Arunk || airSeg->segmentType() == Surface)
      continue;

    if (tsFirst->origin()->nation() != airSeg->destination()->nation() ||
        airSeg->origin()->nation() != airSeg->destination()->nation())
    {
      isInternational = true;
      break;
    }
  }
  return isInternational;
}

bool
ServiceFeeUtil::isRoundTripJourneyType(const Itin& itin,
                                       const CarrierCode& validatingCarrier,
                                       bool isInternational) const
{
  bool isRT = false;

  TravelSeg* tsFirst = itin.travelSeg().front();
  TravelSeg* endS = itin.travelSeg().back();

  if ((isInternational && tsFirst->origin()->nation() == endS->destination()->nation()))
    isRT = true;

  if (!isInternational)
  {
    if (tsFirst->origin()->loc() == endS->destination()->loc())
      isRT = true;
    else
      isRT = checkMultiTransport(tsFirst->origin()->loc(),
                                 endS->destination()->loc(),
                                 validatingCarrier,
                                 itin.geoTravelType());
  }
  return isRT;
}

ServiceFeeUtil::TravelSegVecIC
ServiceFeeUtil::getMlgTurnAround_old(const FarePath& farePath,
                                 std::map<uint32_t, int, std::greater<uint32_t> >& mlgMap) const
{
  const Itin* itin = farePath.itin();
  TravelSegVecIC ret = itin->travelSeg().end();
  mlgMap.clear(); // Fresh Start

  std::set<TravelSeg*> stopoversAndFareBreaks;
  getStopoversAndFareBreaks_old(farePath, stopoversAndFareBreaks);

  TravelSegVecIC travelSegIter;
  std::set<TravelSeg*>::iterator curValidSegIt = stopoversAndFareBreaks.begin();
  TravelSeg* firstTs = itin->travelSeg().front();
  std::vector<TravelSeg*> partialTravelSegs;

  for (; curValidSegIt != stopoversAndFareBreaks.end(); ++curValidSegIt)
  { // Check Global Direction
    TravelSeg* curTs = *curValidSegIt;

    partialTravelSegs.clear();
    travelSegIter = itin->travelSeg().begin();

    for (; travelSegIter != itin->travelSeg().end(); ++travelSegIter)
    {
      partialTravelSegs.push_back(*travelSegIter);

      if (curTs == *travelSegIter)
        break;
    }

    if (*(firstTs->origin()) == *(curTs->destination()))
      continue; // Why bother? Always gonna be 0

    const uint32_t mlg =
        getTPM(*(firstTs->origin()), *(curTs->destination()), partialTravelSegs, farePath);
    mlgMap.insert(std::map<uint32_t, int, std::greater<uint32_t> >::value_type(
        mlg, itin->segmentOrder(curTs)));
  } // for (validSegs iteration)

  if (mlgMap.empty())
    return ret;

  // Got ourselves a legitimate TurnAround Point
  int segmentOrderTurnAround = (*mlgMap.begin()).second;

  TravelSegVecIC curSegIt = itin->travelSeg().begin();
  for (; curSegIt != itin->travelSeg().end(); ++curSegIt)
    if (itin->segmentOrder(*curSegIt) == segmentOrderTurnAround)
      return curSegIt;

  return ret;
}

ServiceFeeUtil::TravelSegVecIC
ServiceFeeUtil::getMlgTurnAround(const FarePath& farePath,
                                 std::map<uint32_t, int, std::greater<uint32_t> >& mlgMap) const
{
  const Itin* itin = farePath.itin();
  TravelSegVecIC ret = itin->travelSeg().end();
  mlgMap.clear(); // Fresh Start

  TravelSegmentOrderComparator travelSegComparator(*itin);
  std::set<TravelSeg*, TravelSegmentOrderComparator> stopoversAndFareBreaks(travelSegComparator);
  getStopoversAndFareBreaks(farePath, stopoversAndFareBreaks);

  TravelSegVecIC travelSegIter;
  std::set<TravelSeg*>::iterator curValidSegIt = stopoversAndFareBreaks.begin();
  TravelSeg* firstTs = itin->travelSeg().front();
  std::vector<TravelSeg*> partialTravelSegs;

  for (; curValidSegIt != stopoversAndFareBreaks.end(); ++curValidSegIt)
  { // Check Global Direction
    TravelSeg* curTs = *curValidSegIt;

    partialTravelSegs.clear();
    travelSegIter = itin->travelSeg().begin();

    for (; travelSegIter != itin->travelSeg().end(); ++travelSegIter)
    {
      partialTravelSegs.push_back(*travelSegIter);

      if (curTs == *travelSegIter)
        break;
    }

    if (*(firstTs->origin()) == *(curTs->destination()))
      continue; // Why bother? Always gonna be 0

    const uint32_t mlg =
        getTPM(*(firstTs->origin()), *(curTs->destination()), partialTravelSegs, farePath);
    mlgMap.insert(std::map<uint32_t, int, std::greater<uint32_t> >::value_type(
        mlg, itin->segmentOrder(curTs)));
  } // for (validSegs iteration)

  if (mlgMap.empty())
    return ret;

  // Got ourselves a legitimate TurnAround Point
  int segmentOrderTurnAround = (*mlgMap.begin()).second;

  TravelSegVecIC curSegIt = itin->travelSeg().begin();
  for (; curSegIt != itin->travelSeg().end(); ++curSegIt)
    if (itin->segmentOrder(*curSegIt) == segmentOrderTurnAround)
      return curSegIt;

  return ret;
}

ServiceFeeUtil::TravelSegVecIC
ServiceFeeUtil::getJourneyDestination(const FarePath& farePath,
                                      const Loc*& journeyDestination,
                                      bool isRT,
                                      bool isIntl) const
{
  const Itin* itin = farePath.itin();
  TravelSegVecIC ret = itin->travelSeg().end();

  if (isRT)
  {
    std::map<uint32_t, int, std::greater<uint32_t> > mlgMap;

    TravelSegVecIC journeyTurnAroundTs;

    if (fallback::fallbackSSDMPS242(&_trx))
      journeyTurnAroundTs = getMlgTurnAround_old(farePath, mlgMap);
    else
      journeyTurnAroundTs = getMlgTurnAround(farePath, mlgMap);

    if (ret == journeyTurnAroundTs) // not found
      return ret;

    const Loc* journeyTurnAroundPoint = (*journeyTurnAroundTs)->destination();
    ret = journeyTurnAroundTs;
    if (isIntl)
    {
      if (journeyTurnAroundPoint->nation() !=
          farePath.itin()->travelSeg().front()->origin()->nation())
      {
        journeyDestination = journeyTurnAroundPoint;
      }
      else // determine the furthest point outside the origin country
      {
        std::map<uint32_t, int, std::greater<uint32_t> >::iterator mlgCurIt = mlgMap.begin();
        std::map<uint32_t, int, std::greater<uint32_t> >::iterator mlgEndIt = mlgMap.end();
        ++mlgCurIt;
        for (; mlgCurIt != mlgEndIt; ++mlgCurIt)
        {
          int segOrder = (*mlgCurIt).second;

          TravelSegVecIC curSegIt = itin->travelSeg().begin();
          for (; curSegIt != itin->travelSeg().end(); ++curSegIt)
          {
            if (itin->segmentOrder(*curSegIt) == segOrder &&
                (*curSegIt)->destination()->nation() !=
                    farePath.itin()->travelSeg().front()->origin()->nation())
            {
              journeyDestination = (*curSegIt)->destination();
              return curSegIt;
            }
          }
        }
      }
    }
    else // domestic
    {
      journeyDestination = journeyTurnAroundPoint;
    }
  }
  else // one way
    journeyDestination = farePath.itin()->travelSeg().back()->destination();

  return ret;
}

void
ServiceFeeUtil::getStopoversAndFareBreaks_old(const FarePath& farePath, std::set<TravelSeg*>& result)
    const
{
  result.clear(); // Fresh Start

  std::vector<PricingUnit*>::const_iterator puBegin = farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puEnd = farePath.pricingUnit().end();
  std::vector<PricingUnit*>::const_iterator puCur = puBegin;
  for (; puCur != puEnd; puCur++)
  {
    if ((*puCur)->isSideTripPU())
      continue;

    std::vector<FareUsage*>::iterator curFu = (*puCur)->fareUsage().begin();
    std::vector<FareUsage*>::iterator fuEnd = (*puCur)->fareUsage().end();
    for (; curFu != fuEnd; curFu++)
    {
      // Load up the FareBreaks first ...
      result.insert((*curFu)->travelSeg().back());

      // Now find the Stopovers
      TravelSegVecIC prevTs = (*curFu)->travelSeg().begin();
      TravelSegVecIC curTs = prevTs + 1;
      for (; curTs != (*curFu)->travelSeg().end(); ++curTs)
      {
        if ((*prevTs)->isForcedConx())
        {
          prevTs = curTs;
          continue; // Already in the list or a Forced Connection
        }

        if ((*curTs)->segmentType() == Arunk || (*prevTs)->isForcedStopOver() ||
            isStopOver(*prevTs, *curTs, *curFu))
          result.insert(*prevTs);
        prevTs = curTs; // On to the next pair
      } // for (TS iteration)
    } // for (FU iteration)
  } // for (PU iteration)
}

void
ServiceFeeUtil::getStopoversAndFareBreaks(const FarePath& farePath, std::set<TravelSeg*, TravelSegmentOrderComparator>& result)
    const
{
  result.clear(); // Fresh Start

  std::vector<PricingUnit*>::const_iterator puBegin = farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puEnd = farePath.pricingUnit().end();
  std::vector<PricingUnit*>::const_iterator puCur = puBegin;
  for (; puCur != puEnd; puCur++)
  {
    if ((*puCur)->isSideTripPU())
      continue;

    std::vector<FareUsage*>::iterator curFu = (*puCur)->fareUsage().begin();
    std::vector<FareUsage*>::iterator fuEnd = (*puCur)->fareUsage().end();
    for (; curFu != fuEnd; curFu++)
    {
      // Load up the FareBreaks first ...
      result.insert((*curFu)->travelSeg().back());

      // Now find the Stopovers
      TravelSegVecIC prevTs = (*curFu)->travelSeg().begin();
      TravelSegVecIC curTs = prevTs + 1;
      for (; curTs != (*curFu)->travelSeg().end(); ++curTs)
      {
        if ((*prevTs)->isForcedConx())
        {
          prevTs = curTs;
          continue; // Already in the list or a Forced Connection
        }

        if ((*curTs)->segmentType() == Arunk || (*prevTs)->isForcedStopOver() ||
            isStopOver(*prevTs, *curTs, *curFu))
          result.insert(*prevTs);
        prevTs = curTs; // On to the next pair
      } // for (TS iteration)
    } // for (FU iteration)
  } // for (PU iteration)
}

bool
ServiceFeeUtil::isStopOverPoint(const TravelSeg* travelSeg,
                                const TravelSeg* travelSegTo,
                                const FareUsage* fareUsage) const
{
  if (fareUsage->paxTypeFare() && fareUsage->paxTypeFare()->fareMarket() &&
      travelSegTo->isStopOver(
          travelSeg, fareUsage->paxTypeFare()->fareMarket()->geoTravelType(), TravelSeg::OTHER))
    return true;

  if (fareUsage->stopovers().find(travelSeg) != fareUsage->stopovers().end())
    return true; // stopovers rule override existed

  return false;
}

bool
ServiceFeeUtil::isStopOver(const TravelSeg* travelSeg,
                           const TravelSeg* travelSegTo,
                           const FareUsage* fareUsage) const
{
  bool isStopOver = isStopOverPoint(travelSeg, travelSegTo, fareUsage);

  if (!isStopOver)
  {
    const TravelSeg* lastTvlSeg = fareUsage->travelSeg().back();
    const TravelSeg* lastAirSeg = TravelSegUtil::lastAirSeg(fareUsage->travelSeg());

    isStopOver = !(travelSeg != lastAirSeg || lastAirSeg == lastTvlSeg);
  }
  return isStopOver;
}

bool
ServiceFeeUtil::checkMultiTransport(const LocCode& locCode1,
                                    const LocCode& locCode2,
                                    const CarrierCode& carrierCode,
                                    GeoTravelType tvlType) const
{
  return LocUtil::getMultiTransportCity(locCode1, carrierCode, tvlType, _trx.ticketingDate()) ==
         LocUtil::getMultiTransportCity(locCode2, carrierCode, tvlType, _trx.ticketingDate());
}

uint32_t
ServiceFeeUtil::getTPM(const Loc& market1,
                       const Loc& market2,
                       const std::vector<TravelSeg*>& travelSegs,
                       const FarePath& farePath) const
{
  GlobalDirection gd = GlobalDirection::XX;

  GlobalDirectionFinderV2Adapter::getGlobalDirection(
      &_trx, farePath.itin()->travelDate(), travelSegs, gd);

  return (gd == GlobalDirection::XX ? TseUtil::greatCircleMiles(market1, market2)
                                    : // check only for performence reason
              LocUtil::getTPM(
                  market1, market2, gd, _trx.getRequest()->ticketingDT(), _trx.dataHandle()));
}

bool
ServiceFeeUtil::checkIsDateBetween(const DateTime& startDate,
                                   const DateTime& endDate,
                                   const DateTime& betweenDate)
{
  const DateTime& start = startDate.isInfinity() ? betweenDate : startDate;
  const DateTime& end = endDate.isInfinity() ? betweenDate : endDate;

  return betweenDate >= start && betweenDate <= end;
}

bool
ServiceFeeUtil::isStartDateSpecified(const OptionalServicesInfo& optSrvInfo)
{
  return optSrvInfo.tvlStartYear() || optSrvInfo.tvlStartMonth() || optSrvInfo.tvlStartDay();
}

bool
ServiceFeeUtil::isStopDateSpecified(const OptionalServicesInfo& optSrvInfo)
{
  return optSrvInfo.tvlStopYear() || optSrvInfo.tvlStopMonth() || optSrvInfo.tvlStopDay();
}

bool
ServiceFeeUtil::matchPaxType(const CarrierCode& validatingCarrier,
                             const PaxType& farePathPaxType,
                             const PaxTypeCode& paxTypeCode) const
{
  if (!paxTypeCode.empty() && farePathPaxType.paxType() != paxTypeCode) // not exact match
  {
    return matchSabrePaxList(farePathPaxType.paxType(), paxTypeCode) ||
           paxTypeMappingMatch(validatingCarrier, farePathPaxType, paxTypeCode);
  }

  return true;
}

bool
ServiceFeeUtil::matchSabrePaxList(const PaxTypeCode& farePathPtc, const PaxTypeCode& svcFeePtc)
    const
{
  std::list<PaxTypeCode> ptcList;

  const std::vector<const PaxTypeMatrix*>& sabrePaxTypes = getSabrePaxTypes(farePathPtc);

  std::transform(sabrePaxTypes.begin(),
                 sabrePaxTypes.end(),
                 std::back_inserter(ptcList),
                 [](const PaxTypeMatrix* ptm)
                 { return ptm->atpPaxType(); });

  return std::find(ptcList.begin(), ptcList.end(), svcFeePtc) != ptcList.end();
}

const std::vector<const PaxTypeMatrix*>&
ServiceFeeUtil::getSabrePaxTypes(const PaxTypeCode& farePathPtc) const
{
  return _trx.dataHandle().getPaxTypeMatrix(farePathPtc);
}

const Currency*
ServiceFeeUtil::getCurrency_old(const CurrencyCode& currencyCode) const
{
  return _trx.dataHandle().getCurrency( currencyCode );
}

const TaxNation*
ServiceFeeUtil::getTaxNation_old(const NationCode& nationCode) const
{
  return _trx.dataHandle().getTaxNation(nationCode, _trx.ticketingDate());
}

bool
ServiceFeeUtil::convertCurrency(Money& target,
                                const Money& source,
                                CurrencyConversionRequest::ApplicationType applType) const
{
  CurrencyConversionFacade converter;
  return converter.convert(target, source, _trx, false, applType);
}

bool
ServiceFeeUtil::paxTypeMappingMatch(const CarrierCode& validatingCarrier,
                                    const PaxType& paxChk,
                                    const PaxTypeCode& paxRef) const
{
  const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxTypes = paxChk.actualPaxType();

  if (UNLIKELY(paxTypeMappingMatch(actualPaxTypes, validatingCarrier, paxRef)))
    return true;

  return paxTypeMappingMatch(actualPaxTypes, ANY_CARRIER, paxRef);
}

bool
ServiceFeeUtil::paxTypeMappingMatch(
    const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxTypes,
    const CarrierCode& carrier,
    const PaxTypeCode& paxRef) const
{
  std::map<CarrierCode, std::vector<PaxType*>*>::const_iterator grpI = actualPaxTypes.find(carrier);

  std::vector<PaxType*>* grpPaxTypes;
  std::vector<PaxType*>::const_iterator paxTypesI, paxTypesEnd;

  if (grpI != actualPaxTypes.end())
  {
    grpPaxTypes = (*grpI).second;
    paxTypesI = grpPaxTypes->begin();
    paxTypesEnd = grpPaxTypes->end();

    for (; paxTypesI != paxTypesEnd; ++paxTypesI)
    {
      PaxType& curPaxType = **paxTypesI;
      if (curPaxType.paxType() == paxRef)
        return true;
    }
  }

  return false;
}

void
ServiceFeeUtil::getSortedFees(const ServiceFeesGroup& sfg, std::vector<FPOCFees>& sortedFees)
{
  typedef std::map<const FarePath*, std::vector<OCFees*> >::value_type FPFeeValueType;

  for (const FPFeeValueType& fpOCFees : sfg.ocFeesMap())
  {
    sortedFees.reserve(fpOCFees.second.size());

    for (OCFees* const fees : fpOCFees.second)
    {
      fees->farePath() = fpOCFees.first;
      sortedFees.push_back(FPOCFees(fpOCFees.first, fees));
    }
  }
}

std::vector<PaxOCFees>
ServiceFeeUtil::getSortedFees(const ServiceFeesGroup& sfg,
                              const std::vector<PaxType*>& reqPaxTypes,
                              bool removeDuplicates)
{
  std::vector<FPOCFees> sortedFees;

  getSortedFees(sfg, sortedFees);

  FPOCFeesComparator feesCmp;
  std::sort(sortedFees.begin(), sortedFees.end(), feesCmp);

  return groupPaxTypes<PaxOCFees>(sortedFees, reqPaxTypes, removeDuplicates);
}

std::vector<PaxOCFees>
ServiceFeeUtil::getFees(const ServiceFeesGroup& sfg,
                        const std::vector<PaxType*>& reqPaxTypes,
                        bool removeDuplicates)
{
  std::vector<FPOCFees> fees;
  getSortedFees(sfg, fees);

  std::vector<PaxOCFees> groupedFees;
  groupedFees.reserve(fees.size());
  std::copy(fees.begin(), fees.end(), std::back_inserter(groupedFees));
  return groupedFees;
}

std::vector<PaxR7OCFees>
ServiceFeeUtil::getSortedFeesForR7(const std::vector<const ServiceFeesGroup*>& sfgs,
                                   const std::vector<PaxType*>& reqPaxTypes,
                                   bool removeDuplicates)
{
  std::vector<FPOCFees> sortedFees;

  for (const ServiceFeesGroup* sfg : sfgs)
    getSortedFees(*sfg, sortedFees);

  FPOCFeesComparatorR7 feesCmp;
  std::sort(sortedFees.begin(), sortedFees.end(), feesCmp);

  return groupPaxTypes<PaxR7OCFees>(sortedFees, reqPaxTypes, removeDuplicates);
}

template <class T>
std::vector<T>
ServiceFeeUtil::groupPaxTypes(std::vector<FPOCFees>& sortedFees,
                              const std::vector<PaxType*>& reqPaxTypes,
                              bool removeDuplicates)
{
  std::vector<T> groupedFees;
  groupedFees.reserve(sortedFees.size());

  if (reqPaxTypes.size() <= 1 || !removeDuplicates)
  {
    std::copy(sortedFees.begin(), sortedFees.end(), std::back_inserter(groupedFees));
    return groupedFees;
  }

  // Remove the same fees
  std::vector<FPOCFees>::iterator currentFee = sortedFees.begin();
  std::vector<FPOCFees>::iterator endFee = sortedFees.end();

  std::vector<FPOCFees> newSortedFees;

  while (currentFee != endFee)
  {
    std::vector<FPOCFees>::iterator const similiarsEnd =
        std::partition(currentFee,
                       endFee,
                       boost::bind(&ServiceFeeUtil::isSimilarAndSamePaxType, _1, (*currentFee)));

    newSortedFees.push_back(*currentFee);

    currentFee = similiarsEnd;
  }

  // Group remaining fees
  currentFee = newSortedFees.begin();
  endFee = newSortedFees.end();

  while (currentFee != endFee)
  {
    std::vector<FPOCFees>::iterator const similiarsEnd = std::partition(
        currentFee, endFee, boost::bind(&ServiceFeeUtil::isSimilar, _1, (*currentFee)));

    reducePaxTypes(currentFee, similiarsEnd, reqPaxTypes, groupedFees);

    currentFee = similiarsEnd;
  }

  return groupedFees;
}

template <class T>
void
ServiceFeeUtil::reducePaxTypes(std::vector<FPOCFees>::const_iterator begin,
                               std::vector<FPOCFees>::const_iterator end,
                               const std::vector<PaxType*>& reqPaxTypes,
                               std::vector<T>& groupedFees)
{
  if (static_cast<size_t>(std::distance(begin, end)) == reqPaxTypes.size())
  {
    groupedFees.push_back(*begin);
    groupedFees.back().setPaxType("ALL");
  }
  else
  {
    std::copy(begin, end, std::back_inserter(groupedFees));
  }
}

bool
ServiceFeeUtil::isSimilar(const FPOCFees& fee1, const FPOCFees& fee2)
{

  return (fee1.fees()->feeAmount() == fee2.fees()->feeAmount() &&
          fee1.fees()->carrierCode() == fee2.fees()->carrierCode() &&
          fee1.fees()->travelStart()->origin()->loc() ==
              fee2.fees()->travelStart()->origin()->loc() &&
          fee1.fees()->travelEnd()->destination()->loc() ==
              fee2.fees()->travelEnd()->destination()->loc() &&
          fee1.fp()->itin()->segmentPnrOrder(fee1.fees()->travelStart()) ==
              fee2.fp()->itin()->segmentPnrOrder(fee2.fees()->travelStart()) &&
          fee1.fp()->itin()->segmentPnrOrder(fee1.fees()->travelEnd()) ==
              fee2.fp()->itin()->segmentPnrOrder(fee2.fees()->travelEnd()) &&
          fee1.fees()->subCodeInfo()->serviceSubTypeCode() ==
              fee2.fees()->subCodeInfo()->serviceSubTypeCode() &&
          fee1.fees()->subCodeInfo()->commercialName() ==
              fee2.fees()->subCodeInfo()->commercialName() &&
          fee1.fees()->getTaxes().size() == fee2.fees()->getTaxes().size() &&
          std::equal(fee1.fees()->getTaxes().begin(),
                     fee1.fees()->getTaxes().end(),
                     fee2.fees()->getTaxes().begin(),
                     OCFees::TaxItemComparator()));
}

bool
ServiceFeeUtil::isSimilarAndSamePaxType(const FPOCFees& fee1, const FPOCFees& fee2)
{
  if (fee1.fp()->paxType() == fee2.fp()->paxType())
  {
    return isSimilar(fee1, fee2);
  }
  else
  {
    return false;
  }
}

// These methods below are for the OCFeesUsage development at Response build time

void
ServiceFeeUtil::getSortedFees(const ServiceFeesGroup& sfg, std::vector<FPOCFeesUsages>& sortedFees)
{
  typedef std::map<const FarePath*, std::vector<OCFees*> >::value_type FPFeeValueType;

  for (const FPFeeValueType& fpOCFees : sfg.ocFeesMap())
  {
    for (OCFees* const fees : fpOCFees.second)
    {
      fees->pointToFirstOCFee();
      for (const auto feesUsage : fees->ocfeeUsage())
      {
        feesUsage->farePath() = fpOCFees.first;
        sortedFees.push_back(FPOCFeesUsages(fpOCFees.first, feesUsage));
      }
    }
  }
}


std::vector<PaxOCFeesUsages>
ServiceFeeUtil::getSortedFeesUsages(const ServiceFeesGroup& sfg,
                                    const std::vector<PaxType*>& reqPaxTypes,
                                    bool removeDuplicates)
{
  std::vector<FPOCFeesUsages> sortedFees;

  getSortedFees(sfg, sortedFees);

  FPOCFeesUsageComparator feesCmp;
  std::sort(sortedFees.begin(), sortedFees.end(), feesCmp);

  return groupPaxTypesU<PaxOCFeesUsages>(sortedFees, reqPaxTypes, removeDuplicates);
}

ServiceFeeUtil::OcFeesUsagesMerger::OcFeesUsagesMerger(const std::vector<PaxType*>& reqPaxTypes,
                                                       unsigned maxFeesCount) :
                                                           _reqPaxTypes(reqPaxTypes),
                                                           _maxFeesCount(maxFeesCount),
                                                           _maxFeesCountReached(false)
{
  // Empty
}

struct ServiceFeeUtil::OcFeesUsagesMerger::SimilarOcFeesUsages
{
  SimilarOcFeesUsages(const FarePath* fp, OCFeesUsage* fee) :
    _fee(fee)
  {
    _similars.emplace_back(fp, fee);
  }

  bool hasAtLeastOneFeeForEachPaxType(const std::vector<PaxType*>& paxTypes);

  bool operator==(const SimilarOcFeesUsages& rhs) { return isSimilarOCFeesUsage(*_fee, *rhs._fee); }
  bool operator==(const OCFeesUsage* rhs) { return isSimilarOCFeesUsage(*_fee, *rhs); }

  OCFeesUsage* _fee;
  std::vector<FPOCFeesUsages> _similars;
};

const std::map<std::string, std::set<int>>&
ServiceFeeUtil::OcFeesUsagesMerger::getGroupedOCFeeIdsForFarePath(const FarePath* fp)
{
  auto iret = _ocFeesIdBySfgByFarePath.find(fp);
  if (iret != _ocFeesIdBySfgByFarePath.end())
  {
    return iret->second;
  }
  else
  {
    static std::map<std::string, std::set<int>> empty;
    return empty;
  }
}

FPOCFeesUsages*
ServiceFeeUtil::OcFeesUsagesMerger::findFirstUsageWithPaxTypeAndAddAllSimilarsToMapping(PaxType* paxType,
                                                                                       ServiceFeeUtil::OcFeesUsagesMerger::SimilarOcFeesUsages& group,
                                                                                       const ServiceGroup& groupCode)
{
  FPOCFeesUsages* firstMatchingOcFeesForThisPaxType = nullptr;
  for (auto farePathOcFees : group._similars)
  {
    if (farePathOcFees.fp()->paxType() == paxType)
    {
      if (!firstMatchingOcFeesForThisPaxType)
        firstMatchingOcFeesForThisPaxType = &farePathOcFees;
      _ocFeesIdBySfgByFarePath[farePathOcFees.fp()][groupCode].insert(_ocs.size());
    }
  }
  return firstMatchingOcFeesForThisPaxType;
}

std::vector<PaxOCFeesUsages>
ServiceFeeUtil::OcFeesUsagesMerger::mergeFeesUsagesForSfg(const ServiceFeesGroup& sfg)
{
  std::vector<PaxOCFeesUsages> result;

  if (_maxFeesCountReached)
    return result;

  // Group OCFees by similarity and then check if each group is available for all passenger types
  // If a group is available for all passenger types - add a single PaxOCFeesUsages
  // to the result vector.
  // If a group is NOT available for all apssenger types - add a PaxOCFeesUsages
  // for each of the fees in the group.
  for (auto group : getFeesGroupedBySimilarity(sfg))
  {
    if (group.hasAtLeastOneFeeForEachPaxType(_reqPaxTypes))
    {
      // This OCFee is available for all the passenger types - add as single PaxOCFeesUsage
      PaxOCFeesUsages usages(group._similars[0], /* id= */ _ocs.size());
      if (_reqPaxTypes.size() > 1)
        usages.setPaxType("ALL");

      for (auto farePathOcFees : group._similars)
        _ocFeesIdBySfgByFarePath[farePathOcFees.fp()][sfg.groupCode()].insert(_ocs.size());

      result.push_back(usages);
      _ocs.push_back(usages);
      if (_maxFeesCount && _ocs.size() >= _maxFeesCount)
      {
        _maxFeesCountReached = true;
        return result;
      }
    }
    else
    {
      // This OCFee is not available for all passenger types - add a PaxOCFeesUsage
      // for each passenger type
      for (auto paxType : _reqPaxTypes)
      {
        if (auto foundOcFee = findFirstUsageWithPaxTypeAndAddAllSimilarsToMapping(paxType, group, sfg.groupCode()))
        {
          PaxOCFeesUsages usages(*foundOcFee, /* id= */ _ocs.size());

          result.push_back(usages);
          _ocs.push_back(usages);
          if (_maxFeesCount && _ocs.size() >= _maxFeesCount)
          {
            _maxFeesCountReached = true;
            return result;
          }
        }
      }
    }
  }

  return result;
}

std::vector<ServiceFeeUtil::OcFeesUsagesMerger::SimilarOcFeesUsages>
ServiceFeeUtil::OcFeesUsagesMerger::getFeesGroupedBySimilarity(const ServiceFeesGroup& sfg)
{
  std::vector<SimilarOcFeesUsages> groupedOcFees;

  for (auto fpOCFees : sfg.ocFeesMap())
  {
    const FarePath* farePath = fpOCFees.first;
    for (OCFees* fees : fpOCFees.second)
    {
      fees->pointToFirstOCFee();
      for (const auto feesUsage : fees->ocfeeUsage())
      {
        feesUsage->farePath() = farePath;
        auto groupAlreadyInVector = std::find(groupedOcFees.begin(), groupedOcFees.end(), feesUsage);
        if (groupAlreadyInVector != groupedOcFees.end())
          groupAlreadyInVector->_similars.emplace_back(fpOCFees.first, feesUsage);
        else
          groupedOcFees.emplace_back(fpOCFees.first, feesUsage);
      }
    }
  }

  return groupedOcFees;
}

bool
ServiceFeeUtil::OcFeesUsagesMerger::SimilarOcFeesUsages::hasAtLeastOneFeeForEachPaxType(const std::vector<PaxType*>& paxTypes)
{
  return std::all_of(paxTypes.begin(), paxTypes.end(),
                     [&] (const PaxType* paxTypeToCheck)
                     {
                       return std::any_of(this->_similars.begin(), this->_similars.end(),
                           [&](const FPOCFeesUsages& farePathOcFeesUsages)
                           {
                             return farePathOcFeesUsages.fp()->paxType() == paxTypeToCheck;
                           });
                     });
}

std::string
ServiceFeeUtil::OcFeesUsagesMerger::toString()
{
  std::ostringstream os;
  os << "OCs[" << _ocs.size() << "]:\n";
  for (auto oc : _ocs)
  {
    os  << "\t" << oc.getId() << ": "
        << oc.paxType() << " " << oc.fees()->feeAmount() << " " << oc.fees()->feeCurrency()
        << " SG=" << oc.fees()->subCodeInfo()->serviceGroup()
        << " SSG=" << oc.fees()->subCodeInfo()->serviceSubGroup()
        << " SSTC=" << oc.fees()->subCodeInfo()->serviceSubTypeCode()
        << " DESCR=\"" << oc.fees()->subCodeInfo()->description1() << "\""
        << " DispOnly=" << oc.fees()->isDisplayOnly()
        << " guaranteed=" << oc.fees()->isFeeGuaranteed()
        << "\n";
  }

  for (auto fpToSfgsToIds : _ocFeesIdBySfgByFarePath)
  {
    os << "FarePath(" << fpToSfgsToIds.first << "):\n";
    for (auto sfgToIds : fpToSfgsToIds.second)
    {
      os << "  SFG(" << sfgToIds.first << "): ";
      for (auto id : sfgToIds.second)
      {
        os << id << " ";
      }
      os << "\n";
    }
  }
  return os.str();
}

std::vector<PaxOCFeesUsages>
ServiceFeeUtil::getFeesUsages(const ServiceFeesGroup& sfg,
                              const std::vector<PaxType*>& reqPaxTypes,
                              bool removeDuplicates)
{
  std::vector<FPOCFeesUsages> fees;
  getSortedFees(sfg, fees);

  std::vector<PaxOCFeesUsages> groupedFees;
  groupedFees.reserve(fees.size());
  std::copy(fees.begin(), fees.end(), std::back_inserter(groupedFees));
  return groupedFees;
}

std::vector<PaxR7OCFeesUsages>
ServiceFeeUtil::getSortedFeesForR7Usages(const std::vector<const ServiceFeesGroup*>& sfgs,
                                         const std::vector<PaxType*>& reqPaxTypes,
                                         bool removeDuplicates)
{
  std::vector<FPOCFeesUsages> sortedFees;

  for (const ServiceFeesGroup* sfg : sfgs)
    getSortedFees(*sfg, sortedFees);

  FPOCFeesUsageComparatorR7 feesCmp;
  std::sort(sortedFees.begin(), sortedFees.end(), feesCmp);

  return groupPaxTypesU<PaxR7OCFeesUsages>(sortedFees, reqPaxTypes, removeDuplicates);
}

template <class T>
std::vector<T>
ServiceFeeUtil::groupPaxTypesU(std::vector<FPOCFeesUsages>& sortedFees,
                               const std::vector<PaxType*>& reqPaxTypes,
                               bool removeDuplicates)
{
  std::vector<T> groupedFees;
  groupedFees.reserve(sortedFees.size());

  std::vector<FPOCFeesUsages>::iterator currentFee = sortedFees.begin();
  std::vector<FPOCFeesUsages>::iterator endFee = sortedFees.end();

  if (currentFee != endFee && (*currentFee).fees()->subCodeInfo()->serviceGroup().equalToConst("SA") &&
      reqPaxTypes.size() <= 1)
  {
    while (currentFee != endFee)
    {
      std::vector<FPOCFeesUsages>::iterator const similiarsEnd = std::partition(
          currentFee, endFee, boost::bind(&ServiceFeeUtil::isSimilarU, _1, (*currentFee)));

      groupedFees.push_back(*currentFee);

      currentFee = similiarsEnd;
    }

    return groupedFees;
  }
  else if (reqPaxTypes.size() <= 1 || !removeDuplicates)
  {
    std::copy(sortedFees.begin(), sortedFees.end(), std::back_inserter(groupedFees));
    return groupedFees;
  }

  // Remove the same fees
  std::vector<FPOCFeesUsages> newSortedFees;

  while (currentFee != endFee)
  {
    std::vector<FPOCFeesUsages>::iterator const similiarsEnd =
        std::partition(currentFee,
                       endFee,
                       boost::bind(&ServiceFeeUtil::isSimilarAndSamePaxTypeU, _1, (*currentFee)));

    newSortedFees.push_back(*currentFee);

    currentFee = similiarsEnd;
  }

  // Group remaining fees
  currentFee = newSortedFees.begin();
  endFee = newSortedFees.end();

  while (currentFee != endFee)
  {
    std::vector<FPOCFeesUsages>::iterator const similiarsEnd = std::partition(
        currentFee, endFee, boost::bind(&ServiceFeeUtil::isSimilarU, _1, (*currentFee)));

    reducePaxTypesU(currentFee, similiarsEnd, reqPaxTypes, groupedFees);

    currentFee = similiarsEnd;
  }

  return groupedFees;
}

template <class T>
void
ServiceFeeUtil::reducePaxTypesU(std::vector<FPOCFeesUsages>::const_iterator begin,
                                std::vector<FPOCFeesUsages>::const_iterator end,
                                const std::vector<PaxType*>& reqPaxTypes,
                                std::vector<T>& groupedFees)
{
  if (static_cast<size_t>(std::distance(begin, end)) == reqPaxTypes.size())
  {
    groupedFees.push_back(*begin);
    groupedFees.back().setPaxType("ALL");
  }
  else
  {
    std::copy(begin, end, std::back_inserter(groupedFees));
  }
}

bool
ServiceFeeUtil::isSimilarOCFeesUsage(const OCFeesUsage& left, const OCFeesUsage& right)
{
  return (left.feeAmount() == right.feeAmount()
          && left.optFee()->notAvailNoChargeInd() == right.optFee()->notAvailNoChargeInd()
          && left.carrierCode() == right.carrierCode()
          && left.travelStart()->origin()->loc() ==
              right.travelStart()->origin()->loc()
          && left.travelEnd()->destination()->loc() ==
              right.travelEnd()->destination()->loc()
          && left.farePath()->itin()->segmentPnrOrder(left.travelStart()) ==
              right.farePath()->itin()->segmentPnrOrder(right.travelStart())
          && left.farePath()->itin()->segmentPnrOrder(left.travelEnd()) ==
              right.farePath()->itin()->segmentPnrOrder(right.travelEnd())
          && left.subCodeInfo()->serviceSubTypeCode() ==
              right.subCodeInfo()->serviceSubTypeCode()
          && left.subCodeInfo()->commercialName() ==
              right.subCodeInfo()->commercialName()
          && left.upgradeT198CommercialName() == right.upgradeT198CommercialName()
          && left.getTaxes().size() == right.getTaxes().size()
          && std::equal(left.getTaxes().begin(),
                        left.getTaxes().end(),
                        right.getTaxes().begin(),
                        OCFees::TaxItemComparator()));
}

bool
ServiceFeeUtil::isSimilarU(const FPOCFeesUsages& fee1, const FPOCFeesUsages& fee2)
{

  return (fee1.fees()->feeAmount() == fee2.fees()->feeAmount() &&
          fee1.fees()->carrierCode() == fee2.fees()->carrierCode() &&
          fee1.fees()->travelStart()->origin()->loc() ==
              fee2.fees()->travelStart()->origin()->loc() &&
          fee1.fees()->travelEnd()->destination()->loc() ==
              fee2.fees()->travelEnd()->destination()->loc() &&
          fee1.fp()->itin()->segmentPnrOrder(fee1.fees()->travelStart()) ==
              fee2.fp()->itin()->segmentPnrOrder(fee2.fees()->travelStart()) &&
          fee1.fp()->itin()->segmentPnrOrder(fee1.fees()->travelEnd()) ==
              fee2.fp()->itin()->segmentPnrOrder(fee2.fees()->travelEnd()) &&
          fee1.fees()->subCodeInfo()->serviceSubTypeCode() ==
              fee2.fees()->subCodeInfo()->serviceSubTypeCode() &&
          fee1.fees()->subCodeInfo()->commercialName() ==
              fee2.fees()->subCodeInfo()->commercialName() &&
          fee1.fees()->upgradeT198CommercialName() == fee2.fees()->upgradeT198CommercialName() &&
          fee1.fees()->getTaxes().size() == fee2.fees()->getTaxes().size() &&
          std::equal(fee1.fees()->getTaxes().begin(),
                     fee1.fees()->getTaxes().end(),
                     fee2.fees()->getTaxes().begin(),
                     OCFees::TaxItemComparator()));
}

bool
ServiceFeeUtil::isSimilarAndSamePaxTypeU(const FPOCFeesUsages& fee1, const FPOCFeesUsages& fee2)
{
  if (fee1.fp()->paxType() == fee2.fp()->paxType())
  {
    return isSimilarU(fee1, fee2);
  }
  else
  {
    return false;
  }
}
// The methods above are for the OCFeesUsage development at Response build time

bool
ServiceFeeUtil::getFeeRounding_old(const CurrencyCode& currencyCode,
                               RoundingFactor& roundingFactor,
                               CurrencyNoDec& roundingNoDec,
                               RoundingRule& roundingRule)
{
  const Currency* currency = getCurrency_old(currencyCode);

  if (UNLIKELY(!currency))
  {
    LOG4CXX_ERROR(logger, "DBAccess getCurrency returned null currency pointer");
    return false;
  }

  if (currency->taxOverrideRoundingUnit() > 0)
  {
    roundingFactor = currency->taxOverrideRoundingUnit();
    roundingNoDec = currency->taxOverrideRoundingUnitNoDec();
    roundingRule = currency->taxOverrideRoundingRule();

    return true;
  }

  const std::string controllingEntityDesc = currency->controllingEntityDesc();
  LOG4CXX_INFO(logger, "Currency country description: " << currency->controllingEntityDesc());

  bool foundNationalCurrency = false;
  bool foundNation = false;
  NationCode nationWithMatchingNationalCurrency;
  NationCode nationCode;

  CurrencyUtil::getControllingNationCode(_trx,
                                         controllingEntityDesc,
                                         nationCode,
                                         foundNation,
                                         foundNationalCurrency,
                                         nationWithMatchingNationalCurrency,
                                         _trx.ticketingDate(),
                                         currencyCode);

  if (LIKELY(foundNation))
  {
    const TaxNation* taxNation = getTaxNation_old(nationCode);

    if (LIKELY(taxNation))
    {
      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();

      return true;
    }
  }
  else if (foundNationalCurrency)
  {
    const TaxNation* taxNation = getTaxNation_old(nationWithMatchingNationalCurrency);

    if (taxNation)
    {
      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();

      return true;
    }
  }
  return false;
}

CurrencyCode
ServiceFeeUtil::getSellingCurrency() const
{
  return ServiceFeeUtil::getSellingCurrency(_trx);
}

CurrencyCode
ServiceFeeUtil::getSellingCurrency(const PricingTrx& trx)
{
  CurrencyCode sellingCurrency;

  if (false == trx.getOptions()->currencyOverride().empty())
  {
    sellingCurrency = trx.getOptions()->currencyOverride();
  }
  else
  {
    sellingCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  }

  return sellingCurrency;
}

void
ServiceFeeUtil::convertServiceFeesGroupCurrency(ServiceFeesGroup* group)
{
  CurrencyCode code = getSellingCurrency(_trx);

  typedef std::map<const FarePath*, std::vector<OCFees*> >::value_type FPFeeValueType;
  for (const FPFeeValueType& fpOCFees : group->ocFeesMap())
  {
    for (OCFees* const fees : fpOCFees.second)
    {
      Money converted = convertOCFeeCurrency(code, fees);
      fees->feeAmount() = converted.value();
      fees->feeCurrency() = converted.code();
    }
  }
}

Money
ServiceFeeUtil::convertOCFeeCurrency(const CalcTotals& calcTotals, const OCFees* fee)
{
  CurrencyCode code(calcTotals.equivCurrencyCode);

  if (calcTotals.equivCurrencyCode.empty() &&
      fee->feeCurrency() != calcTotals.convertedBaseFareCurrencyCode)
  {
    code = calcTotals.convertedBaseFareCurrencyCode;
  }

  return convertOCFeeCurrency(code, fee);
}

Money
ServiceFeeUtil::convertOCFeeCurrency(const OCFees* fee)
{
  return convertOCFeeCurrency(getSellingCurrency(_trx), fee);
}

Money
ServiceFeeUtil::convertBaggageFeeCurrency(const OCFees* fee)
{
  return convertMoney(fee->feeAmount(),
                      fee->feeCurrency(),
                      getSellingCurrency(_trx),
                      CurrencyConversionRequest::FARES);
}

Money
ServiceFeeUtil::convertBaggageFeeCurrency(const OCFeesUsage& ocFeeUsage)
{
  return convertMoney(ocFeeUsage.feeAmount(),
                      ocFeeUsage.feeCurrency(),
                      getSellingCurrency(_trx),
                      CurrencyConversionRequest::FARES);
}

Money
ServiceFeeUtil::convertBaggageFeeCurrency(const MoneyAmount sourceAmount,
                                          const CurrencyCode& sourceCurrency,
                                          const CurrencyCode& targetCurrency)
{
  if (sourceCurrency.empty())
    return Money(sourceAmount, targetCurrency);

  Money source(sourceAmount, sourceCurrency);
  Money target(targetCurrency);

  if (sourceCurrency != targetCurrency)
  {
    CurrencyConversionFacade convertFacade;

    if (!convertFacade.convertWithCurrentDate(target, source, _trx))
    {
      throw ErrorResponseException(ErrorResponseException::FEE_CURRENCY_CONVERSION_FAILED);
    }
  }
  else
  {
    target.value() = sourceAmount;

    if (!fallback::ocFeesAmountRoundingRefactoring(&_trx))
    {
      OCFees::BaggageAmountRounder baggageAmountRounder(_trx);
      target.value() = baggageAmountRounder.getRoundedFeeAmount(target);
    }
    else
    {
      CurrencyRoundingUtil roundingUtil;
      roundingUtil.round(target, _trx);
    }
  }
  return target;
}

Money
ServiceFeeUtil::convertOCFeeCurrency(const OCFeesUsage& ocFeeUsage)
{
  return convertMoney(ocFeeUsage.feeAmount(), ocFeeUsage.feeCurrency(), getSellingCurrency(_trx));
}

Money
ServiceFeeUtil::convertMoney(const MoneyAmount sourceAmount,
                             const CurrencyCode& sourceCurrency,
                             const CurrencyCode& targetCurrency,
                             CurrencyConversionRequest::ApplicationType applType)
{
  if (sourceCurrency.empty())
    return Money(sourceAmount, targetCurrency);

  Money source(sourceAmount, sourceCurrency);
  if (sourceCurrency == targetCurrency)
    return source;

  Money result(targetCurrency);
  convertCurrency(result, source, applType);

  if (!fallback::ocFeesAmountRoundingRefactoring(&_trx))
  {
    OCFees::OcAmountRounder ocAmountRounder(_trx);
    result.value() = ocAmountRounder.getRoundedFeeAmount(result);
  }
  else
  {
    RoundingFactor roundingFactor = 0;
    CurrencyNoDec roundingNoDec = 0;
    RoundingRule roundingRule = NONE;

    if (LIKELY(getFeeRounding_old(result.code(), roundingFactor, roundingNoDec, roundingRule)))
    {
      CurrencyConverter curConverter;
      curConverter.round(result, roundingFactor, roundingRule);
    }
  }
  return result;
}

Money
ServiceFeeUtil::convertOCFeeCurrencyWithTaxes(const CurrencyCode sellingCurrency, const OCFees* fee)
{
  return convertFeeWithTaxes(
      fee->feeAmount(), fee->getTaxes(), fee->feeCurrency(), sellingCurrency);
}

Money
ServiceFeeUtil::convertOCFeeCurrencyWithTaxes(const CurrencyCode sellingCurrency,
                                              const OCFeesUsage* fee)
{
  return convertFeeWithTaxes(
      fee->feeAmount(), fee->getTaxes(), fee->feeCurrency(), sellingCurrency);
}

Money
ServiceFeeUtil::convertFeeWithTaxes(const MoneyAmount& fee,
                                    const std::vector<tse::OCFees::TaxItem>& taxes,
                                    const CurrencyCode& sourceCurrency,
                                    const CurrencyCode& targetCurrency,
                                    CurrencyConversionRequest::ApplicationType applType)
{
  Money result(targetCurrency);
  result.value() += convertMoney(fee, sourceCurrency, targetCurrency, applType).value();
  static const size_t TAX_ON_OC_BUFF_SIZE = 15; // TODO !!!! Remove hardcode !!!!
  size_t count = 0;
  for (const tse::OCFees::TaxItem& taxItem : taxes)
  {
    Money tax =
        convertMoney(taxItem.getTaxAmount(), taxItem.getCurrency(), targetCurrency, applType);
    result.value() += tax.value();
    if (++count >= TAX_ON_OC_BUFF_SIZE)
      break;
  }
  return result;
}

Money
ServiceFeeUtil::convertOCFeeCurrency(const CurrencyCode& sellingCurrency, const OCFees* fee)
{
  return convertMoney(fee->feeAmount(), fee->feeCurrency(), sellingCurrency);
}

const OcFeeGroupConfig*
ServiceFeeUtil::getGroupConfigurationForCode(
    const std::vector<OcFeeGroupConfig>& groupSummaryConfigVec, const ServiceGroup& group)
{
  const OcFeeGroupConfig* feeGroupConfigPtr = nullptr;

  std::vector<OcFeeGroupConfig>::const_iterator feeGroupConfigIter = groupSummaryConfigVec.begin();
  std::vector<OcFeeGroupConfig>::const_iterator feeGroupConfigIterEnd = groupSummaryConfigVec.end();

  for (; feeGroupConfigIter != feeGroupConfigIterEnd; ++feeGroupConfigIter)
  {
    const OcFeeGroupConfig& feeGroupConfig = (*feeGroupConfigIter);

    if (feeGroupConfig.groupCode() == group)
    {
      feeGroupConfigPtr = &feeGroupConfig;
      break;
    }
  }

  return feeGroupConfigPtr;
}

const RequestedOcFeeGroup*
ServiceFeeUtil::getRequestedOcFeeGroupData(const std::vector<RequestedOcFeeGroup>& serviceGroupsVec,
                                           const ServiceGroup& group)
{
  const RequestedOcFeeGroup* ocFeeGroupPtr = nullptr;

  std::vector<RequestedOcFeeGroup>::const_iterator ocFeeGroupIter = serviceGroupsVec.begin();
  std::vector<RequestedOcFeeGroup>::const_iterator ocFeeGroupIterEnd = serviceGroupsVec.end();

  for (; ocFeeGroupIter != ocFeeGroupIterEnd; ++ocFeeGroupIter)
  {
    const RequestedOcFeeGroup& ocFeeGroup = (*ocFeeGroupIter);

    if (ocFeeGroup.groupCode() == group)
    {
      ocFeeGroupPtr = &ocFeeGroup;
      break;
    }
  }

  return ocFeeGroupPtr;
}

Money
ServiceFeeUtil::getOCFeesSummary(const Itin* itin)
{
  CurrencyCode sellingCurrency = getSellingCurrency(_trx);
  Money outMoney(0.0, sellingCurrency);

  typedef std::map<const FarePath*, std::vector<OCFees*> >::value_type FPFeeValueType;

  for (ServiceFeesGroup* sfg : itin->ocFeesGroup())
  {
    if (sfg->ocFeesMap().empty())
    {
      continue;
    }

    for (const FPFeeValueType& fpOCFees : sfg->ocFeesMap())
      for (OCFees* const fees : fpOCFees.second)
        fees->farePath() = fpOCFees.first;

    const OcFeeGroupConfig* feeGroupConfig =
        getGroupConfigurationForCode(_trx.getOptions()->groupsSummaryConfigVec(), sfg->groupCode());

    if (nullptr == feeGroupConfig)
      continue;

    const RequestedOcFeeGroup* ocFeeGroup =
        getRequestedOcFeeGroupData(_trx.getOptions()->serviceGroupsVec(), sfg->groupCode());

    if (nullptr == ocFeeGroup)
      continue;

    const size_t noOfItemsRequested = ocFeeGroup->numberOfItems();

    // BG logic
    if (feeGroupConfig->commandName().empty())
    {
      Money feeGroupMoney(0.0, sellingCurrency);

      std::vector<PaxTypeBucketItem> bucketsVec =
          constructPaxTypeBuckets(sfg, feeGroupConfig, noOfItemsRequested, itin, sellingCurrency);

      std::vector<PaxType> paxTypeRemVec;
      std::vector<bool> paxTypeFeesInitialized;
      size_t noOfPaxRemaining = 0;

      for (PaxType* const paxType : _trx.paxType())
      {
        noOfPaxRemaining += paxType->number();
        paxTypeRemVec.push_back(*paxType);
        paxTypeFeesInitialized.push_back(false);
      }

      size_t feesRemaining = noOfItemsRequested;

      for (PaxTypeBucketItem& paxTypeBucketItem : bucketsVec)
      {
        int position = 0;

        for (PaxType& remPaxType : paxTypeRemVec)
        {
          bool bit = paxTypeFeesInitialized.at(position);

          while ((remPaxType.number() > 0) &&
                 (paxTypeBucketItem.numberOfItems <=
                  getMaxNoOfFeesAllowed(feesRemaining, noOfPaxRemaining)) &&
                 pickBucketItem(remPaxType, paxTypeBucketItem, feeGroupMoney, feesRemaining, bit))
          {
            noOfPaxRemaining--;
            paxTypeFeesInitialized.at(position) = bit;
          }

          ++position;
        }
      }

      removeFeesForRemainingPaxTypes(sfg, paxTypeRemVec, paxTypeFeesInitialized);

      outMoney.value() += feeGroupMoney.value();
    }
    else if (feeGroupConfig->applyTo() == 'I') // Fees other than BG - Fees per Itin
    {
      Money cheapestPaxTypeMoney(0.0, sellingCurrency);
      const PaxType* cheapestPaxType = nullptr;

      for (FPFeeValueType& fpOCFees : sfg->ocFeesMap())
      {
        const FarePath* farePath = fpOCFees.first;

        std::vector<OCFees*>& ocFeesVec = fpOCFees.second;

        getOCFeesSummary(itin, 1, feeGroupConfig, ocFeesVec, sellingCurrency);

        recalculateOCFeesVector(ocFeesVec, noOfItemsRequested);

        Money curMoney = getMoneyFromVec(ocFeesVec, sellingCurrency);

        if (cheapestPaxType != nullptr)
        {
          if (curMoney.value() < cheapestPaxTypeMoney.value())
          {
            cheapestPaxTypeMoney = curMoney;
            cheapestPaxType = farePath->paxType();
          }
          else if (curMoney.value() == cheapestPaxTypeMoney.value())
          {
            if ((true == ServiceFeeUtil::isAdultPaxType(*(farePath->paxType()))) &&
                (false == ServiceFeeUtil::isAdultPaxType(*cheapestPaxType)))
            {
              cheapestPaxTypeMoney = curMoney;
              cheapestPaxType = farePath->paxType();
            }
            else if (ServiceFeeUtil::isAdultPaxType(*(farePath->paxType())) ==
                     ServiceFeeUtil::isAdultPaxType(*cheapestPaxType))
            {
              if (farePath->paxType()->paxType() < cheapestPaxType->paxType())
              {
                cheapestPaxTypeMoney = curMoney;
                cheapestPaxType = farePath->paxType();
              }
            }
          }
        }
        else
        {
          cheapestPaxTypeMoney = curMoney;
          cheapestPaxType = farePath->paxType();
        }
      }

      if (cheapestPaxType != nullptr)
      {
        clearExpensivePaxOCFees(sfg, cheapestPaxType->paxType());

        outMoney.value() += cheapestPaxTypeMoney.value();
      }
    }
    else if (feeGroupConfig->applyTo() == 'P') // Fees other than BG - Fees per Pax
    {
      for (const PaxType* paxType : _trx.paxType())
      {
        for (FPFeeValueType& fpOCFees : sfg->ocFeesMap())
        {
          const FarePath* farePath = fpOCFees.first;

          if (farePath->paxType()->paxType() != paxType->paxType())
            continue;

          std::vector<OCFees*>& ocFeesVec = fpOCFees.second;

          getOCFeesSummary(itin, 1, feeGroupConfig, ocFeesVec, sellingCurrency);

          recalculateOCFeesVector(ocFeesVec, noOfItemsRequested * paxType->number());

          Money curMoney = getMoneyFromVec(ocFeesVec, sellingCurrency);
          outMoney.value() += curMoney.value();
        }
      }
    }
  }

  return outMoney;
}

size_t
ServiceFeeUtil::getMaxNoOfFeesAllowed(size_t feesRemaining, size_t noOfPaxRemaining)
{
  if ((feesRemaining != 0) && (noOfPaxRemaining != 0))
    return (feesRemaining / noOfPaxRemaining) + (feesRemaining % noOfPaxRemaining);

  return 0;
}

void
ServiceFeeUtil::recalculateOCFeesVector(std::vector<OCFees*>& ocFeesVec, size_t noOfItemsRequested)
{
  std::vector<OCFees*> finalOCFeesVec;

  for (OCFees* fee : ocFeesVec)
  {
    if (fee == nullptr)
      continue;

    for (size_t cnt = noOfItemsRequested; cnt > 0; --cnt)
      finalOCFeesVec.push_back(fee);
  }

  ocFeesVec.swap(finalOCFeesVec);
}

std::vector<PaxTypeBucketItem>
ServiceFeeUtil::constructPaxTypeBuckets(ServiceFeesGroup* sfg,
                                        const OcFeeGroupConfig* feeGroupConfig,
                                        size_t noOfItemsRequested,
                                        const Itin* itin,
                                        const CurrencyCode& sellingCurrency)
{
  std::vector<PaxTypeBucketItem> bucketsVec = getPaxBuckets(noOfItemsRequested, sfg);

  for (PaxTypeBucketItem& paxTypeBucketItem : bucketsVec)
  {
    getOCFeesSummary(itin,
                     paxTypeBucketItem.numberOfItems,
                     feeGroupConfig,
                     paxTypeBucketItem.ocFees,
                     sellingCurrency);

    paxTypeBucketItem.moneyAmount =
        (getMoneyFromVec(paxTypeBucketItem.ocFees, sellingCurrency)).value();
  }

  std::sort(bucketsVec.begin(), bucketsVec.end(), PaxBucketComparator());

  return bucketsVec;
}

void
ServiceFeeUtil::clearExpensivePaxOCFees(ServiceFeesGroup* sfg, const PaxTypeCode& paxType)
{
  typedef std::map<const FarePath*, std::vector<OCFees*> >::value_type FPFeeValueType;

  for (FPFeeValueType& fpOCFees : sfg->ocFeesMap())
  {
    const FarePath* farePath = fpOCFees.first;

    if ((farePath->paxType()->paxType()) == paxType)
      continue;

    fpOCFees.second.clear();
  }
}

void
ServiceFeeUtil::removeFeesForRemainingPaxTypes(ServiceFeesGroup* sfg,
                                               std::vector<PaxType>& paxTypeRemVec,
                                               std::vector<bool>& paxTypeFeesInitialized)
{
  typedef std::map<const FarePath*, std::vector<OCFees*> >::value_type FPFeeValueType;

  int position = 0;
  for (PaxType& remPaxType : paxTypeRemVec)
  {
    if (false == paxTypeFeesInitialized.at(position))
    {
      for (FPFeeValueType& fpOCFees : sfg->ocFeesMap())
      {
        const FarePath* farePath = fpOCFees.first;

        if ((farePath->paxType()->paxType()) == remPaxType.paxType())
          fpOCFees.second.clear();
      }
    }

    ++position;
  }
}

bool
ServiceFeeUtil::pickBucketItem(PaxType& remPaxType,
                               PaxTypeBucketItem& paxTypeBucketItem,
                               Money& outMoney,
                               size_t& feesRemaining,
                               bool& paxTypeFeesInitialized)
{
  if (remPaxType.paxType() == paxTypeBucketItem.paxType.paxType())
  {
    if (feesRemaining >= paxTypeBucketItem.numberOfItems)
    {
      if (!paxTypeFeesInitialized)
      {
        if (paxTypeBucketItem.originalOcFees != nullptr)
          paxTypeBucketItem.originalOcFees->clear();

        paxTypeFeesInitialized = true;
      }

      if (paxTypeBucketItem.originalOcFees != nullptr)
        paxTypeBucketItem.originalOcFees->insert(paxTypeBucketItem.originalOcFees->end(),
                                                 paxTypeBucketItem.ocFees.begin(),
                                                 paxTypeBucketItem.ocFees.end());

      feesRemaining -= paxTypeBucketItem.numberOfItems;

      remPaxType.number()--;

      outMoney.value() += paxTypeBucketItem.moneyAmount;
      return true;
    }
  }

  return false;
}

std::vector<PaxTypeBucketItem>
ServiceFeeUtil::getPaxBuckets(size_t numberOfItems, ServiceFeesGroup* sfg)
{
  size_t totalNoOfPax = 0;
  std::vector<PaxTypeBucketItem> retVec;
  std::vector<PaxTypeBucketItem> tempVec;
  typedef std::map<const FarePath*, std::vector<OCFees*> >::value_type FPFeeValueType;

  for (PaxType* const paxType : _trx.paxType())
  {
    PaxTypeBucketItem paxTypeBucketItem;
    paxTypeBucketItem.paxType = *paxType;

    for (FPFeeValueType& fpOCFees : sfg->ocFeesMap())
    {
      const FarePath* farePath = fpOCFees.first;

      if (farePath->paxType()->paxType() == paxType->paxType())
      {
        paxTypeBucketItem.ocFees = fpOCFees.second;
        paxTypeBucketItem.originalOcFees = &(fpOCFees.second);
        break;
      }
    }

    totalNoOfPax += paxType->number();

    tempVec.push_back(paxTypeBucketItem);
  }

  size_t noForPax = numberOfItems / totalNoOfPax;
  size_t noRemaining = ((numberOfItems % totalNoOfPax) != 0) ? 1 : 0;

  if (noForPax != 0 && noRemaining > 0)
    ++noForPax;

  for (PaxTypeBucketItem& paxTypeBucketItem : tempVec)
  {
    if (noForPax != 0)
    {
      for (size_t i = noForPax; i > noRemaining; --i)
      {
        paxTypeBucketItem.numberOfItems = i;

        retVec.push_back(paxTypeBucketItem);
      }
    }

    if (noRemaining != 0)
    {
      paxTypeBucketItem.numberOfItems = 1;

      retVec.push_back(paxTypeBucketItem);
    }
  }
  return retVec;
}

Money
ServiceFeeUtil::getMoneyFromVec(const std::vector<OCFees*>& ocFeesVec,
                                const CurrencyCode& sellingCurrency)
{
  Money outMoney(0.0, sellingCurrency);

  if (_trx.getTrxType() == PricingTrx::MIP_TRX) // calculate ancillary taxes only for MIP service
    for (OCFees* const pOCFees : ocFeesVec)
      outMoney.value() += convertOCFeeCurrencyWithTaxes(sellingCurrency, pOCFees).value();
  else
    for (OCFees* const pOCFees : ocFeesVec)
      outMoney.value() += convertOCFeeCurrency(sellingCurrency, pOCFees).value();

  return outMoney;
}

void
ServiceFeeUtil::getOCFeesSummary(const Itin* itin,
                                 size_t numberOfItems,
                                 const OcFeeGroupConfig* feeGroupConfig,
                                 std::vector<OCFees*>& ocFeesVec,
                                 const CurrencyCode& sellingCurrency)
{
  if (feeGroupConfig->commandName() == "#LO-EX")
    removeRestrictedSubCodes(feeGroupConfig, ocFeesVec);

  if (feeGroupConfig->commandName().empty())
  {
    getFeeDependOnSubCodesOrder(itin, feeGroupConfig, sellingCurrency, numberOfItems, ocFeesVec);
  }
  else
  {
    getLowestFee(itin, sellingCurrency, ocFeesVec);
  }
}

namespace
{

class RestrictedSubCodes
{
public:
  RestrictedSubCodes(const ServiceSubTypeCode& subTypeCode) : _subTypeCode(&subTypeCode) {}

  bool operator()(OCFees* ocFees) const
  {
    return (ocFees->subCodeInfo()->serviceSubTypeCode() == (*_subTypeCode));
  }

private:
  const ServiceSubTypeCode* _subTypeCode;
};
}

void
ServiceFeeUtil::removeRestrictedSubCodes(const OcFeeGroupConfig* feeGroupConfig,
                                         std::vector<OCFees*>& outFeesVec)
{
  std::vector<ServiceSubTypeCode>::const_iterator subTypeCodeIter =
      feeGroupConfig->subTypeCodes().begin();
  std::vector<ServiceSubTypeCode>::const_iterator subTypeCodeIterEnd =
      feeGroupConfig->subTypeCodes().end();

  for (; subTypeCodeIter != subTypeCodeIterEnd; ++subTypeCodeIter)
  {
    const ServiceSubTypeCode& subTypeCode = (*subTypeCodeIter);

    outFeesVec.erase(
        std::remove_if(outFeesVec.begin(), outFeesVec.end(), RestrictedSubCodes(subTypeCode)),
        outFeesVec.end());
  }
}

Money
ServiceFeeUtil::getFeeDependOnSubCodesOrder(const Itin* itin,
                                            const OcFeeGroupConfig* feeGroupConfig,
                                            const CurrencyCode& sellingCurrency,
                                            size_t numberOfItems,
                                            std::vector<OCFees*>& feesVec)
{
  std::map<TravelSeg*, std::vector<OCFees*> > feesForTravelSegMap;

  std::vector<TravelSeg*>::const_iterator travelSegIter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator travelSegIterEnd = itin->travelSeg().end();

  for (; travelSegIter != travelSegIterEnd; ++travelSegIter)
  {
    TravelSeg* travelSeg = (*travelSegIter);

    size_t noOfSubCodesProcessed = 0;
    size_t noOfFeesFoundForSeg = 0;

    if (feesForTravelSegMap.find(travelSeg) != feesForTravelSegMap.end())
    {
      noOfFeesFoundForSeg = (feesForTravelSegMap[travelSeg]).size();
    }

    std::vector<ServiceSubTypeCode>::const_iterator subTypeCodeIter =
        feeGroupConfig->subTypeCodes().begin();
    std::vector<ServiceSubTypeCode>::const_iterator subTypeCodeIterEnd =
        feeGroupConfig->subTypeCodes().end();

    for (; subTypeCodeIter != subTypeCodeIterEnd; ++subTypeCodeIter, ++noOfSubCodesProcessed)
    {
      if (noOfFeesFoundForSeg >= numberOfItems)
        break;

      if (noOfSubCodesProcessed >= numberOfItems)
        break;

      const ServiceSubTypeCode& subTypeCode = (*subTypeCodeIter);

      OCFees* fee = getFeeForSubCode(itin, travelSeg, feesVec, subTypeCode);

      if (fee != nullptr)
      {
        if (addFeeToMap(fee, travelSeg, feesForTravelSegMap))
          noOfFeesFoundForSeg++;
      }
    }
  }

  return getSummaryFromMap(feesForTravelSegMap, sellingCurrency, feesVec);
}

Money
ServiceFeeUtil::getLowestFee(const Itin* itin,
                             const CurrencyCode& sellingCurrency,
                             std::vector<OCFees*>& feesVec)
{
  std::map<TravelSeg*, std::vector<OCFees*> > feesForTravelSegMap;

  std::vector<TravelSeg*>::const_iterator travelSegIter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator travelSegIterEnd = itin->travelSeg().end();

  for (; travelSegIter != travelSegIterEnd; ++travelSegIter)
  {
    TravelSeg* travelSeg = (*travelSegIter);

    Money outMoney(0.0, sellingCurrency);
    bool feeFound = false;
    OCFees* lowestFee = nullptr;

    std::vector<OCFees*>::iterator ocFeesIter = feesVec.begin();
    std::vector<OCFees*>::iterator ocFeesIterEnd = feesVec.end();

    for (; ocFeesIter != ocFeesIterEnd; ++ocFeesIter)
    {
      OCFees* ocFees = (*ocFeesIter);

      if (!feeValidForTravelSeg(itin, ocFees, travelSeg))
        continue;

      Money currFeeMoney = convertOCFeeCurrency(sellingCurrency, ocFees);

      if (feeFound)
      {
        if (currFeeMoney.value() < outMoney.value())
        {
          outMoney = currFeeMoney;
          lowestFee = ocFees;
        }
      }
      else
      {
        outMoney = currFeeMoney;
        lowestFee = ocFees;
      }

      feeFound = true;
    }

    if (lowestFee != nullptr)
    {
      std::vector<OCFees*> newFeesVec;
      newFeesVec.push_back(lowestFee);

      feesForTravelSegMap[travelSeg] = newFeesVec;
    }
  }

  return getSummaryFromMap(feesForTravelSegMap, sellingCurrency, feesVec);
}

OCFees*
ServiceFeeUtil::getFeeForSubCode(const Itin* itin,
                                 TravelSeg* travelSeg,
                                 std::vector<OCFees*>& feesVec,
                                 const ServiceSubTypeCode& subTypeCode)
{
  std::vector<OCFees*>::iterator ocFeesIter = feesVec.begin();
  std::vector<OCFees*>::iterator ocFeesIterEnd = feesVec.end();

  for (; ocFeesIter != ocFeesIterEnd; ++ocFeesIter)
  {
    OCFees* ocFees = (*ocFeesIter);

    if (ocFees->subCodeInfo()->serviceSubTypeCode() == subTypeCode)
    {
      if (feeValidForTravelSeg(itin, ocFees, travelSeg))
        return ocFees;
    }
  }

  return nullptr;
}

bool
ServiceFeeUtil::feeValidForTravelSeg(const Itin* itin, OCFees* ocFees, TravelSeg* travelSeg)
{
  if ((itin->segmentOrder(ocFees->travelStart()) <= itin->segmentOrder(travelSeg)) &&
      (itin->segmentOrder(ocFees->travelEnd()) >= itin->segmentOrder(travelSeg)))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool
ServiceFeeUtil::addFeeToMap(OCFees* ocFees,
                            TravelSeg* travelSeg,
                            std::map<TravelSeg*, std::vector<OCFees*> >& feesForSegmentMap)
{
  if (feesForSegmentMap.find(travelSeg) != feesForSegmentMap.end())
  {
    std::vector<OCFees*>& feesVec = feesForSegmentMap[travelSeg];

    if (std::find(feesVec.begin(), feesVec.end(), ocFees) == feesVec.end())
    {
      feesVec.push_back(ocFees);
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    std::vector<OCFees*> newFeesVec;
    newFeesVec.push_back(ocFees);

    feesForSegmentMap[travelSeg] = newFeesVec;
    return true;
  }
}

Money
ServiceFeeUtil::getSummaryFromMap(
    const std::map<TravelSeg*, std::vector<OCFees*> >& feesForSegmentMap,
    const CurrencyCode& sellingCurrency,
    std::vector<OCFees*>& feesVec)
{
  Money outMoney(0.0, sellingCurrency);

  feesVec.clear();

  std::map<TravelSeg*, std::vector<OCFees*> >::const_iterator mapIter = feesForSegmentMap.begin();
  std::map<TravelSeg*, std::vector<OCFees*> >::const_iterator mapIterEnd = feesForSegmentMap.end();

  for (; mapIter != mapIterEnd; ++mapIter)
  {
    const std::vector<OCFees*>& fees = mapIter->second;

    std::vector<OCFees*>::const_iterator feesIter = fees.begin();
    std::vector<OCFees*>::const_iterator feesIterEnd = fees.end();

    for (; feesIter != feesIterEnd; ++feesIter)
    {
      OCFees* ocFees = (*feesIter);

      if (std::find(feesVec.begin(), feesVec.end(), ocFees) == feesVec.end())
      {
        feesVec.push_back(ocFees);

        Money currFeeMoney = convertOCFeeCurrency(sellingCurrency, ocFees);
        outMoney.value() += currFeeMoney.value();
      }
    }
  }

  return outMoney;
}

bool
ServiceFeeUtil::isAdultPaxType(const PaxType& paxType)
{
  if ((paxType.paxTypeInfo().childInd() != 'Y') && (paxType.paxTypeInfo().infantInd() != 'Y'))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool
ServiceFeeUtil::feesFoundForItin(const Itin* itin)
{
  bool feesFound = false;

  if (!(itin->ocFeesGroup().empty()))
  {
    std::vector<ServiceFeesGroup*>::const_iterator sfgIter = itin->ocFeesGroup().begin();
    std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd = itin->ocFeesGroup().end();

    for (; sfgIter != sfgIterEnd; ++sfgIter)
    {
      ServiceFeesGroup* sfg = (*sfgIter);

      cleanupOcFeesMap(sfg->ocFeesMap());

      if (!(sfg->ocFeesMap().empty()))
      {
        feesFound = true;
      }
    }
  }

  return feesFound;
}

void
ServiceFeeUtil::cleanupOcFeesMap(std::map<const FarePath*, std::vector<OCFees*> >& map)
{
  if (!map.empty())
  {
    std::map<const FarePath*, std::vector<OCFees*> >::iterator ocFeesIter = map.begin();
    std::map<const FarePath*, std::vector<OCFees*> >::iterator ocFeesIterEnd = map.end();
    std::map<const FarePath*, std::vector<OCFees*> >::iterator ocFeesIterTemp;

    while (ocFeesIter != ocFeesIterEnd)
    {
      if ((ocFeesIter->second).empty())
      {
        ocFeesIterTemp = ocFeesIter;
        ++ocFeesIter;
        map.erase(ocFeesIterTemp);
      }
      else
      {
        ++ocFeesIter;
      }
    }
  }
}

//----------------------------------------------------------------------------
//  function to save some PaxTypeFare type info
void
ServiceFeeUtil::fareStatToString(const PaxTypeFare& paxTypeFare, std::string& outStr)
{
  uint8_t ocFareStats = PTF_Normal;

  if (isDiscounted19_22(paxTypeFare))
  {
    ocFareStats |= PTF_Discounted;
  }
  if (paxTypeFare.isNegotiated())
  {
    ocFareStats |= PTF_Negotiated;
  }
  if (paxTypeFare.isFareByRule())
  {
    ocFareStats |= PTF_FareByRule;
  }
  if (paxTypeFare.tcrTariffCat() == 1) // RuleConst::PRIVATE_TARIFF
  {
    ocFareStats |= PTF_TariffCatPrivate;
  }
  char buffer[4];
  sprintf(buffer, "%02X", ocFareStats);
  outStr = buffer;
}

bool
ServiceFeeUtil::isDiscounted19_22(const PaxTypeFare& ptf)
{
  if (ptf.isDiscounted())
  {
    try
    {
      if (ptf.discountInfo().category() == DiscountInfo::CHILD ||
          ptf.discountInfo().category() == DiscountInfo::OTHERS)
      {
        return true;
      }
    }
    catch (...) {}
  }
  return false;
}

//----------------------------------------------------------------------------
//  function to restore some PaxTypeFare type info
void
ServiceFeeUtil::setFareStat(PaxTypeFare& paxTypeFare, const OCFareTypeCode& ocFareType)
{
  unsigned int ocFareTypeBitMap = 0;
  sscanf(ocFareType.c_str(), "%X", &ocFareTypeBitMap);

  if (ocFareTypeBitMap & PTF_Discounted)
  {
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted);
  }
  if (ocFareTypeBitMap & PTF_Negotiated)
  {
    paxTypeFare.status().set(PaxTypeFare::PTF_Negotiated);
  }
  if (ocFareTypeBitMap & PTF_FareByRule)
  {
    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule);
  }
  if (ocFareTypeBitMap & PTF_TariffCatPrivate)
  {
    paxTypeFare.setTcrTariffCatPrivate();
  }
}

void
ServiceFeeUtil::setFareIndicator(PaxTypeFare& paxTypeFare, uint16_t fareTypeInd)
{
  switch (fareTypeInd)
  {
  case 19:
  case 20:
  case 21:
  case 22:
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted);
    break;
  case 25:
    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule);
    break;
  case 35:
    paxTypeFare.status().set(PaxTypeFare::PTF_Negotiated);
    break;
  }
}

void
ServiceFeeUtil::collectSegmentStatus(const FarePath& farePath, Ts2ss& ts2ss)
{
  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      for (size_t i = 0; i < fu->travelSeg().size() && i < fu->segmentStatus().size(); ++i)
      {
        TravelSeg* ts = fu->travelSeg()[i];
        ts2ss[ts] = std::make_pair(&fu->segmentStatus()[i], fu->paxTypeFare());
      }
    }
  }
}

bool
ServiceFeeUtil::checkServiceGroupForAcs(const ServiceGroup& serviceGroup)
{
  return (serviceGroup.equalToConst("BG") || serviceGroup.equalToConst("PT"));
}

bool
ServiceFeeUtil::isServiceGroupInvalidForAcs(const ServiceGroup& serviceGroup)
{
  return !serviceGroup.equalToConst("SA");
}

bool
ServiceFeeUtil::isFeeFarePercentage(const OptionalServicesInfo& info)
{
  return (info.frequentFlyerMileageAppl() == 'C') || (info.frequentFlyerMileageAppl() == 'P') ||
         (info.frequentFlyerMileageAppl() == 'H');
}

void
ServiceFeeUtil::createOCFeesUsagesforR7(const std::vector<ServiceFeesGroup*>& sfgs, PricingTrx& trx)
{
  for (const ServiceFeesGroup* sfg : sfgs)
    createOCFeesUsages(*sfg, trx);
}

void
ServiceFeeUtil::fillOutOCFeeUsageByPadisDescription(PricingTrx& pricingTrx,
                                                    OCFees& ocfee,
                                                    size_t i,
                                                    SvcFeesResBkgDesigInfo& padis)
{
  OCFeesUsage* ocfUsage = nullptr;
  pricingTrx.dataHandle().get(ocfUsage);
  if (!ocfUsage)
    return;
  ocfUsage->oCFees() = &ocfee;
  ocfUsage->setSegIndex(i);
  // For the current Padis code, call addSeatCabinCharacteristic().
  addSeatCabinCharacteristic(pricingTrx, *ocfUsage, padis);

  ocfee.ocfeeUsage().push_back(ocfUsage);
}

bool
ServiceFeeUtil::isPortionOfTravelSelected(PricingTrx& trx, TravelSeg* first, TravelSeg* last)
{
  if (trx.diagnostic().isActive())
  {
    std::map<std::string, std::string>::iterator beginD =
        trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);

    if (beginD != trx.diagnostic().diagParamMap().end())
    {
      std::string& specifiedFM = (*beginD).second;
      if (!(specifiedFM.empty()))
      {
        const std::string boardCity(specifiedFM.substr(0, 3));
        const std::string offCity(specifiedFM.substr(3, 3));

        if (((boardCity != first->boardMultiCity()) && (boardCity != first->origin()->loc())) ||
            ((offCity != last->offMultiCity()) && (offCity != last->destination()->loc())))
          return false;
      }
    }
  }
  return true;
}

void
ServiceFeeUtil::createOCFeesUsageForSingleS7(PricingTrx& pricingTrx,
                                             OCFees& ocfee,
                                             size_t i,
                                             bool ignorePadis)
{
  std::vector<SvcFeesResBkgDesigInfo*>::const_iterator padisI, padisE;
  if (ignorePadis || ocfee.getSeg(i)._padisData.empty())
  {
    OCFeesUsage* ocfUsage = nullptr;
    pricingTrx.dataHandle().get(ocfUsage);

    if (!ocfUsage)
      return;
    ocfUsage->oCFees() = &ocfee;
    ocfUsage->setSegIndex(i);
    ocfee.ocfeeUsage().push_back(ocfUsage);
  }
  else
  {
    padisI = ocfee.getSeg(i)._padisData.begin();
    padisE = ocfee.getSeg(i)._padisData.end();
    for (; padisI != padisE; padisI++)
      fillOutOCFeeUsageByPadisDescription(pricingTrx, ocfee, i, **padisI);
  }
}

void
ServiceFeeUtil::createOCFeesUsages(const ServiceFeesGroup& sfg,
                                   PricingTrx& pricingTrx,
                                   bool ignorePadis)
{
  AncillaryPricingTrx* ancTrx = dynamic_cast<AncillaryPricingTrx*>(&pricingTrx);
  bool isSecondCallForMonetaryDiscount = ancTrx && ancTrx->isSecondCallForMonetaryDiscount();
  for (const auto& pair : sfg.ocFeesMap())
    for (const auto ocfee : pair.second)
      if (ocfee->ocfeeUsage().empty())
        for (size_t i = 0; i != ocfee->segCount(); ++i)
          if (UNLIKELY(!isSecondCallForMonetaryDiscount || ocfee->getSegPtr(i)->_ancPriceModification))
            createOCFeesUsageForSingleS7(pricingTrx, *ocfee, i, ignorePadis);
}

void
ServiceFeeUtil::addSeatCabinCharacteristic(PricingTrx& pricingTrx,
                                           OCFeesUsage& ocfUsage,
                                           SvcFeesResBkgDesigInfo& padis)
{
  ocfUsage.upgradeT198Sequence() = &padis;
  if (ocfUsage.farePath())
    ocfUsage.upgradeT198CommercialName() =
        getTranslatedPadisDescription(pricingTrx,
                                      ocfUsage.carrierCode(),
                                      ocfUsage.farePath()->itin()->travelDate(),
                                      *(ocfUsage.upgradeT198Sequence()));
}

std::string
ServiceFeeUtil::getTranslatedPadisDescription(PricingTrx& pricingTrx,
                                              const CarrierCode& carrier,
                                              const DateTime& travelDate,
                                              const SvcFeesResBkgDesigInfo& padis)
{
  const std::vector<SeatCabinCharacteristicInfo*>& seatCabinForCarrier =
      pricingTrx.dataHandle().getSeatCabinCharacteristicInfo(carrier, 'S', travelDate);

  const std::vector<SeatCabinCharacteristicInfo*>& seatCabinAll =
      pricingTrx.dataHandle().getSeatCabinCharacteristicInfo("**", 'S', travelDate);

  std::set<BookingCode> bookingCodeSet;
  fill(bookingCodeSet, &padis);

  return tse::ServiceFeeUtil::getPadisDescriptions(
      pricingTrx, bookingCodeSet, seatCabinForCarrier, seatCabinAll);
}

void
ServiceFeeUtil::getTranslatedPadisDescription(
    PricingTrx& pricingTrx,
    const CarrierCode& carrier,
    const DateTime& travelDate,
    const SvcFeesResBkgDesigInfo& padis,
    std::map<BookingCode, std::string>& codeDescriptionMap,
    std::map<BookingCode, std::string>& abbreviatedDescriptionMap)
{
  const std::vector<SeatCabinCharacteristicInfo*>& seatCabinForCarrier =
      pricingTrx.dataHandle().getSeatCabinCharacteristicInfo(carrier, 'S', travelDate);

  const std::vector<SeatCabinCharacteristicInfo*>& seatCabinAll =
      pricingTrx.dataHandle().getSeatCabinCharacteristicInfo("**", 'S', travelDate);

  std::set<BookingCode> bookingCodeSet;
  fill(bookingCodeSet, &padis);

  tse::ServiceFeeUtil::getPadisDisplayDescriptions(bookingCodeSet,
                                                   seatCabinForCarrier,
                                                   seatCabinAll,
                                                   codeDescriptionMap,
                                                   abbreviatedDescriptionMap);
}

std::string
ServiceFeeUtil::getFareBasisRoot(PricingTrx& trx, std::string fareBasis)
{
  std::string::size_type posBasis = fareBasis.find("/");
  if (posBasis != std::string::npos)
    fareBasis.erase(posBasis);

  return fareBasis;
}

bool
ServiceFeeUtil::isPerformEMDCheck(const PricingTrx& trx, const SubCodeInfo& subCode)
{
  if (subCode.serviceGroup().equalToConst("99"))
    return false;

  if (!trx.activationFlags().isEmdForFlightRelatedServiceAndPrepaidBaggage())
    return false;

  if(trx.billing() && (trx.billing()->requestPath() == AEBSO_PO_ATSE_PATH) && TrxUtil::isRequestFromAS(trx))
    return true;

  if (trx.activationFlags().isEmdForCharges())
    return true;

  if (isRequestFromTN(trx) &&
    trx.diagnostic().diagnosticType() == Diagnostic875 &&
    trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO" &&
    trx.diagnostic().diagParamMapItem(Diagnostic::ALL_VALID) == "EMDIA")
    return true;

  return false;
}

bool
ServiceFeeUtil::isPerformEMDCheck_old(const PricingTrx& trx)
{
  if (!trx.activationFlags().isEmdForFlightRelatedServiceAndPrepaidBaggage())
    return false;

  if(trx.billing() && (trx.billing()->requestPath() == AEBSO_PO_ATSE_PATH) && TrxUtil::isRequestFromAS(trx))
    return true;

  if (trx.activationFlags().isEmdForCharges())
    return true;

  if (isRequestFromTN(trx) &&
    trx.diagnostic().diagnosticType() == Diagnostic875 &&
    trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO" &&
    trx.diagnostic().diagParamMapItem(Diagnostic::ALL_VALID) == "EMDIA")
    return true;

  return false;
}

bool
ServiceFeeUtil::isRequestFromTN(const PricingTrx& trx)
{
  if (trx.getRequest()->ticketingAgent() && trx.getRequest()->ticketingAgent()->agentTJR() != nullptr &&
      (trx.getRequest()->ticketingAgent()->abacusUser() ||
       trx.getRequest()->ticketingAgent()->axessUser() ||
       trx.getRequest()->ticketingAgent()->agentTJR()->crsCarrier() == INFINI_MULTIHOST_ID ||
       trx.getRequest()->ticketingAgent()->agentTJR()->hostName() == INFINI_USER ||
       trx.getRequest()->ticketingAgent()->agentTJR()->crsCarrier() == SABRE_MULTIHOST_ID ||
       trx.getRequest()->ticketingAgent()->agentTJR()->hostName() == SABRE_USER ||
       trx.getRequest()->ticketingAgent()->cwtUser()))
    return true;

  return false;
}


} // end namespace tse
