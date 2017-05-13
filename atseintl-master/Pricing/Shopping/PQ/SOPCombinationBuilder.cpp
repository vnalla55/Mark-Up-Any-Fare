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

#include "Pricing/Shopping/PQ/SOPCombinationBuilder.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/MultiDimensionalPQ.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/Diag941Collector.h"
#include "Pricing/Shopping/Diversity/DiversityUtil.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include <tr1/functional>

namespace tse
{

FIXEDFALLBACK_DECL(fallbackShoppingPQCabinClassValid);

namespace shpq
{

SOPCombinationBuilder::SOPCombinationBuilder(const SoloPQItemPtr& pqItem,
                                             const CxrKeyPerLeg& cxrKey,
                                             bool processDirectFlightsOnly,
                                             DiversityModel& dm,
                                             Diag941Collector* dc,
                                             ShoppingTrx& trx)
  : _pqItem(pqItem),
    _farePath(*pqItem->getFarePath()),
    _processDirectFlightsOnly(processDirectFlightsOnly),
    _dm(dm),
    _dc(dc),
    _trx(trx),
    _carrierKeys(cxrKey),
    _isCombListCreated(false),
    _sopsByDateByLeg(trx.legs().size()),
    _combinations(trx),
    _sopIdxStdVecAdapter(trx.legs().size())
{
}

DiversityModel::SOPCombinationList&
SOPCombinationBuilder::getOrCreateSOPCombinationList()
{
  if (UNLIKELY(_isCombListCreated))
    return _combinations;

  collectSOPInfo();
  collectApplicableCombinations();
  _isCombListCreated = true;
  return _combinations;
}

SopIdxVec
SOPCombinationBuilder::getFlbIdxVec(SopIdxVecArg intSopVec) const
{
  std::map<SopIdxVec, SopIdxVec>::const_iterator findIt = _oSopVecToSopVec.find(intSopVec);
  TSE_ASSERT(findIt != _oSopVecToSopVec.end());

  return findIt->second;
}

void
SOPCombinationBuilder::detectCarriers()
{
  // Check that we have all flight bitmaps for detected carriers
  for (const PricingUnit* pu : _farePath.pricingUnit())
  {
    for (const FareUsage* fareUsage : pu->fareUsage())
    {
      const PaxTypeFare* ptf = fareUsage->paxTypeFare();
      const uint16_t legIdx = ptf->fareMarket()->legIndex();
      TSE_ASSERT(!_trx.isAltDates() ||
                 ptf->durationFlightBitmapPerCarrier().find(_carrierKeys[legIdx]) !=
                     ptf->durationFlightBitmapPerCarrier().end());
      TSE_ASSERT(_trx.isAltDates() ||
                 ptf->flightBitmapPerCarrier().find(_carrierKeys[legIdx]) !=
                     ptf->flightBitmapPerCarrier().end());
    }
  }

  if (UNLIKELY(_dc))
  {
    for (size_t puIdx = 0; puIdx < _farePath.pricingUnit().size(); ++puIdx)
    {
      PricingUnit* pu = _farePath.pricingUnit()[puIdx];
      for (size_t fuIdx = 0; fuIdx < pu->fareUsage().size(); ++fuIdx)
        _dc->printCarrierLine(puIdx, fuIdx, pu->fareUsage()[fuIdx]->paxTypeFare());
    }

    _dc->printCarrierTotals(_carrierKeys);
  }
}

void
SOPCombinationBuilder::collectSOPInfo()
{
  TSE_ASSERT(_sopInfoZones.empty());
  TSE_ASSERT(_sopInfos.empty()); // for debug only

  _sopInfos.resize(_trx.legs().size());
  detectCarriers();

  for (size_t puIdx = 0; puIdx < _farePath.pricingUnit().size(); ++puIdx)
  {
    PricingUnit* pu = _farePath.pricingUnit()[puIdx];

    for (size_t fuIdx = 0; fuIdx < pu->fareUsage().size(); ++fuIdx)
    {
      FareUsage* fu = pu->fareUsage()[fuIdx];
      PaxTypeFare* ptf = fu->paxTypeFare();
      uint16_t legIndex = ptf->fareMarket()->legIndex();
      std::vector<SOPInfo>& sopInfoVec = _sopInfos[legIndex];
      const ApplicableSOP* asops = ptf->fareMarket()->getApplicableSOPs();
      TSE_ASSERT(asops != nullptr);
      ApplicableSOP::const_iterator asopIt = asops->find(_carrierKeys[legIndex]);
      TSE_ASSERT(asopIt != asops->end());
      const SOPUsages& sopUsages = asopIt->second;

      ptf->setComponentValidationForCarrier(
          _carrierKeys[legIndex], _trx.isAltDates(), _trx.mainDuration());
      if (UNLIKELY(_dc))
      {
        _dc->printSOPLine(puIdx, fuIdx, ptf);

        if (_trx.isAltDates())
          _dc->printDurationFltBitmap(_carrierKeys[legIndex], ptf);
      }

      const PaxTypeFare::FlightBitmap& flbm = ptf->flightBitmap();
      TSE_ASSERT(flbm.size() == sopUsages.size());

      if (sopInfoVec.empty())
      {
        if (_processDirectFlightsOnly)
        {
          std::size_t endIdxForNonStop = detectEndIdxForNonStop(_trx, legIndex, sopUsages);
          sopInfoVec.resize(endIdxForNonStop);
        }
        else
          sopInfoVec.resize(flbm.size());

        TSE_ASSERT(sopInfoVec.size() <= flbm.size()); // "<=" if _processDirectFlightsOnly
        for (size_t flbIdx = 0; flbIdx < sopInfoVec.size(); ++flbIdx)
          sopInfoVec[flbIdx].initialize(
            _trx, ptf, legIndex, flbIdx, _carrierKeys[legIndex], sopUsages);
      }
      else
      {
        TSE_ASSERT(sopInfoVec.size() <= flbm.size()); // "<=" if _processDirectFlightsOnly
        for (size_t flbIdx = 0; flbIdx < sopInfoVec.size(); ++flbIdx)
          sopInfoVec[flbIdx].update(_trx, ptf, flbIdx, _carrierKeys[legIndex]);
      }
    }
  }

  SOPInfos thrownSopInfos;
  _sopInfosFiltered = _dm.filterSolutionsPerLeg(
      _farePath.getTotalNUCAmount(), _sopInfos, _dc ? &thrownSopInfos : nullptr);

  if (UNLIKELY(_dc && _sopInfosFiltered.get()))
  {
    TSE_ASSERT(thrownSopInfos.size() == _sopInfos.size());
    if (thrownSopInfos.size() == 2)
    {
      printThrownCombinations(thrownSopInfos.at(0), &_sopInfos.at(1));
      printThrownCombinations(_sopInfos.at(0), &thrownSopInfos.at(1));
    }
    else
    {
      printThrownCombinations(thrownSopInfos.at(0), nullptr);
    }
  }

  if (UNLIKELY(_dc))
  {
    for (size_t i = 0; i < _sopInfos.size(); ++i)
      _dc->printSOPTotalsPerLeg(i, _sopInfos[i]);
  }

  collectSOPInfoZones();
}

void
SOPCombinationBuilder::collectSOPInfoZones()
{
  _sopInfoZones.resize(_trx.legs().size());

  const std::vector<std::vector<SOPInfo> >& sopInfos = getSOPInfosForCombinations();
  for (size_t legIdx = 0; legIdx < sopInfos.size(); ++legIdx)
  {
    SOPZoneMap sopZoneMap;

    for (const SOPInfo& sopInfo : sopInfos[legIdx])
    {
      if (sopInfo.getStatus() == INVALID_STATUS)
        continue;
      const Itin& itin = *_trx.legs()[legIdx].sop()[sopInfo.getSOPValidIndex()].itin();
      const uint16_t segCount = itin.travelSeg().size();
      // TODO Possibly refactor into vector*'s to avoid copying if performance hit
      sopZoneMap[segCount].push_back(sopInfo);
    }

    flush(sopZoneMap, _sopInfoZones[legIdx]);
  }
}

bool
SOPCombinationBuilder::isAlreadyGenerated(SopIdxVecArg oSopVec) const
{
  // With attempt to hint compiler to generate code,
  // which shall be quick due to branch prediction
  switch (_sopIdxStdVecAdapter.size())
  {
  case 1:
    std::copy(oSopVec.begin(), oSopVec.begin() + 1, _sopIdxStdVecAdapter.begin());
    break;
  case 2:
    std::copy(oSopVec.begin(), oSopVec.begin() + 2, _sopIdxStdVecAdapter.begin());
    break;
  default:
    TSE_ASSERT(!"Unsupported number of legs");
  }

  return _trx.flightMatrix().find(_sopIdxStdVecAdapter) != _trx.flightMatrix().end();
}

bool
SOPCombinationBuilder::isInterlineSolution(SopIdxVecArg sopVec) const
{
  CarrierCode sopGovCxr("");
  CarrierCode tvlSegGovCxr("");

  for (size_t idx = 0, sopSize = sopVec.size(); idx < sopSize; ++idx)
  {
    const ShoppingTrx::SchedulingOption& sop = _trx.legs()[idx].sop()[sopVec[idx]];
    // if sops have different cxr, we don't have look further
    if (sopGovCxr.empty())
      sopGovCxr = sop.governingCarrier();
    else if (sopGovCxr != sop.governingCarrier())
      return true;

    const std::vector<TravelSeg*>& segs = sop.itin()->travelSeg();
    for (const auto seg : segs)
    {
      const AirSeg* const airSeg = seg->toAirSeg();
      if (airSeg)
      {
        if (tvlSegGovCxr.empty())
          tvlSegGovCxr = airSeg->marketingCarrierCode();
        else if (tvlSegGovCxr != airSeg->marketingCarrierCode())
          return true;
      }
    }
  }
  return false;
}

bool
SOPCombinationBuilder::checkMinConnectionTime(SopIdxVecArg sopVec) const
{
  switch (_trx.legs().size())
  {
  case 1:
    return true;
  case 2:
  {
    const int OB = 0;
    const int IB = 1;

    const ShoppingTrx::SchedulingOption& sop1 = _trx.legs()[OB].sop().at(sopVec[OB]);
    const ShoppingTrx::SchedulingOption& sop2 = _trx.legs()[IB].sop().at(sopVec[IB]);

    const TravelSeg* const seg1 = sop1.itin()->travelSeg().back();
    const TravelSeg* const seg2 = sop2.itin()->travelSeg().front();

    return ShoppingUtil::checkMinConnectionTime(seg1, seg2, _trx.getOptions());
  }
  default:
    TSE_ASSERT(!"Unsupported number of legs");
    return false;
  }
}

size_t
SOPCombinationBuilder::detectEndIdxForNonStop(const ShoppingTrx& trx,
                                              uint16_t legIndex,
                                              const SOPUsages& sopUsages) const
{
  size_t result = 0;
  for (; result < sopUsages.size(); ++result)
  {
    int origSopId = sopUsages[result].origSopId_;
    if (origSopId < 0)
      continue;

    bool isNonStopSOP =
        (ShoppingUtil::getTravelSegCountForExternalSopID(trx, legIndex, origSopId) == 1);
    if (!isNonStopSOP)
      break;
  }

  return result;
}

void
SOPCombinationBuilder::collectApplicableCombinations()
{
  TSE_ASSERT(getCombinations().empty());
  TSE_ASSERT(_oSopVecToSopVec.empty());

  MultiDimensionalPQ<SOPInfoZone, uint16_t> multiPQ(_sopInfoZones);

  std::vector<std::vector<SOPInfo>*> sopInfos(_trx.legs().size());
  std::vector<SOPInfoZone> combination;
  while (!(combination = multiPQ.next()).empty())
  {
    for (size_t i = 0; i < combination.size(); ++i)
      sopInfos[i] = &combination[i].sopInfos();
    collectApplicableCombinationsImpl(sopInfos);
  }
}

void
SOPCombinationBuilder::addCombination(const DiversityModel::SOPCombination& combination,
                                      const shpq::SopIdxVec& sopVec)
{
  if (_trx.isAltDates() && combination.datePair)
  {
    _sopsByDateByLeg[0][combination.datePair->first].insert(combination.oSopVec[0]);
    if (_trx.legs().size() == 2)
      _sopsByDateByLeg[1][combination.datePair->second].insert(combination.oSopVec[1]);
  }

  _combinations.push_back(combination);
  _oSopVecToSopVec.insert(std::make_pair(cref(combination.oSopVec), cref(sopVec)));
}

const std::vector<std::vector<SOPInfo> >&
SOPCombinationBuilder::getSOPInfosForCombinations() const
{
  if (_sopInfosFiltered.get())
  {
    return *_sopInfosFiltered;
  }
  else
  {
    return _sopInfos;
  }
}

void
SOPCombinationBuilder::printThrownCombinations(const std::vector<SOPInfo>& inboundSops,
                                               const std::vector<SOPInfo>* outboundSops)
{
  TSE_ASSERT(_dc);
  for (const SOPInfo& inboundSop : inboundSops)
  {
    if (!SopCombinationUtil::isValidForCombination(_trx, 0, inboundSop))
      continue;

    if (outboundSops)
    {
      SopIdxVec sopIdxVec(_trx.legs().size());
      sopIdxVec[0] = inboundSop._sopIndex;
      for (const SOPInfo& outboundSop : *outboundSops)
      {
        if (!SopCombinationUtil::isValidForCombination(_trx, 1, outboundSop))
          continue;

        sopIdxVec[1] = outboundSop._sopIndex;
        _dc->addCombinationResult(sopIdxVec, Diag941Collector::DIVERSITY);
      }
    }
    else
    {
      SopIdxVec sopIdxVec(_trx.legs().size());
      sopIdxVec[0] = inboundSop._sopIndex;
      _dc->addCombinationResult(sopIdxVec, Diag941Collector::DIVERSITY);
    }
  }
}

// sopZoneMapIn will be consumend for performance reasons and cannot be const
void
SOPCombinationBuilder::flush(SOPZoneMap& sopZoneMapIn,
                                   std::vector<SOPInfoZone>& sopInfoZonesLeg)
{
  SOPInfoZone zone;
  for (SOPZoneMap::value_type& sopZone : sopZoneMapIn)
  {
    zone.queueRank() = sopZone.first;
    zone.sopInfos().swap(sopZone.second);
    sopInfoZonesLeg.push_back(zone);
  }
}


void
SOPCombinationBuilder::collectApplicableCombinationsImpl(
    const std::vector<std::vector<SOPInfo>*>& sopInfos)
{
  std::vector<SOPInfo> combination(_trx.legs().size());
  shpq::SopIdxVec sopVec(_trx.legs().size());
  const bool isRoundTrip(_trx.legs().size() == 2);

  for (const SOPInfo& ob : *sopInfos[0])
  {
    combination[0] = ob;

    if (isRoundTrip)
    {
      for (const SOPInfo& ib : *sopInfos[1])
      {
        combination[1] = ib;
        checkAddApplicableCombination(combination, sopVec, isRoundTrip);
      }
    }
    else
    {
      checkAddApplicableCombination(combination, sopVec, isRoundTrip);
    }
  }
}

void
SOPCombinationBuilder::checkAddApplicableCombination(const std::vector<SOPInfo>& combination,
                                                           shpq::SopIdxVec& sopVec,
                                                           const bool isRoundTrip)
{
  if (UNLIKELY(combination[0].getStatus() == INVALID_STATUS))
    return;
  if (UNLIKELY(isRoundTrip && combination[1].getStatus() == INVALID_STATUS))
    return;

  if (!fallback::fixed::fallbackShoppingPQCabinClassValid())
  {
    if (!isCombinationCabinValid(combination, isRoundTrip))
      return;
  }

  DiversityModel::SOPCombination sopCombination(_trx, *_pqItem, combination);
  if (!isCombinationValid(sopCombination))
    return;

  if (!_dm.getIsNewCombinationDesired(sopCombination, _farePath.getTotalNUCAmount()))
  {
    if (UNLIKELY(_dc))
      _dc->addCombinationResult(sopCombination.oSopVec, Diag941Collector::DIVERSITY);
    return;
  }

  sopVec[0] = combination[0]._flightBitmapIndex;
  if (isRoundTrip)
    sopVec[1] = combination[1]._flightBitmapIndex;

  addCombination(sopCombination, sopVec);
}

bool
SOPCombinationBuilder::isCombinationCabinValid(const std::vector<SOPInfo>& combination,
                                               const bool isRoundTrip) const
{
  if (!_trx.legs()[0].sop()[combination[0]._sopIndex].cabinClassValid())
    return false;
  if (isRoundTrip && !_trx.legs()[1].sop()[combination[1]._sopIndex].cabinClassValid())
    return false;

  return true;
}

bool
SOPCombinationBuilder::isCombinationValid(DiversityModel::SOPCombination& combination) const
{
  if (isAlreadyGenerated(combination.oSopVec))
  {
    if (UNLIKELY(_dc))
      _dc->addCombinationResult(combination.oSopVec, Diag941Collector::EXISTS);
    return false;
  }

  if (UNLIKELY(_trx.interlineSolutionsOnly() && !isInterlineSolution(combination.oSopVec)))
  {
    if (_dc)
      _dc->addCombinationResult(combination.oSopVec, Diag941Collector::INTERLINE_ONLY);
    return false;
  }

  if (_trx.isAltDates() && combination.status == INVALID_STATUS)
  {
    if (UNLIKELY(_dc))
      _dc->addCombinationResult(combination.oSopVec, Diag941Collector::ALT_DATES);
    return false;
  }

  if (combination.oSopVec.size() == 2)
  {
    // check MCT only for round trips
    if (!checkMinConnectionTime(combination.oSopVec))
    {
      if (UNLIKELY(_dc))
        _dc->addCombinationResult(combination.oSopVec, Diag941Collector::MCT);
      return false;
    }
  }

  return true;
}
} /* ns shpq */
} /* ns tse */
