//-------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Fares/FareUtil.h"

#include "Common/ShoppingUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareClassAppSegInfo.h"

#include <limits>

namespace tse
{

bool
FareUtil::isNegFareCarrierValid(const CarrierCode& carrier,
                                const Indicator& tktAppl,
                                const Itin& itin,
                                const bool isFareDisplay)
{
  bool ret = true;

  if (!carrier.empty() && !isFareDisplay)
  {
    const CarrierCode& validatingCarrier = itin.validatingCarrier();

    if ((tktAppl == ' ' && carrier != validatingCarrier) ||
        (tktAppl == 'X' && carrier == validatingCarrier))
    {
      ret = false;
    }
  }

  return ret;
}
bool
FareUtil::isNegFareCarrierValid(const CarrierCode& carrier,
                                const Indicator& tktAppl,
                                const CarrierCode& cxr,
                                const bool isFareDisplay)
{
  bool ret = true;

  if (!carrier.empty() && !isFareDisplay)
  {
    if ((tktAppl == ' ' && carrier != cxr) ||
        (tktAppl == 'X' && carrier == cxr))
    {
      ret = false;
    }
  }

  return ret;
}

bool
FareUtil::isNegFareCarrierValid(const CarrierCode& carrier,
                                const Indicator& tktAppl,
                                PaxTypeFare* paxTypeFare,
                                const bool isFareDisplay)
{
  if (!carrier.empty() && !isFareDisplay)
  {
    bool vcrEmpty = paxTypeFare->validatingCarriers().empty();
    for (const CarrierCode& vcr : paxTypeFare->fareMarket()->validatingCarriers())
    {
      if ((tktAppl == ' ' && carrier == vcr) ||
          (tktAppl == 'X' && carrier != vcr))
      {
        if (vcrEmpty)
            paxTypeFare->validatingCarriers().push_back(vcr);
      }
      else
      {
         std::vector<CarrierCode>::iterator vcrIt = find(paxTypeFare->validatingCarriers().begin(),
                                                         paxTypeFare->validatingCarriers().end(),
                                                         vcr);
         if(vcrIt != paxTypeFare->validatingCarriers().end())
         {
            paxTypeFare->validatingCarriers().erase(vcrIt);
         }
      }
    }
    return paxTypeFare->validatingCarriers().size() > 0;
  }

  if (paxTypeFare->validatingCarriers().empty())
      paxTypeFare->validatingCarriers() = paxTypeFare->fareMarket()->validatingCarriers();

  return true;
}
bool
FareUtil::failFareClassAppSegDirectioanlity(const FareClassAppSegInfo& fcas,
                                            const PricingTrx& trx,
                                            bool flipGeo)
{
  if (UNLIKELY(trx.getOptions()->isRtw()))
    return false;

  return ((fcas._directionality == ORIGINATING_LOC1 && flipGeo) ||
          (fcas._directionality == ORIGINATING_LOC2 && !flipGeo));
}

// This function should only be called if delay validator is disabled
// As it relies on flight bitmaps that are not fully available with Delay Validation

void
FareUtil::invalidateFailedSops(ShoppingTrx& trx,
                               FareMarket* fareMarket,
                               JourneyItinWrapper& journeyItinWrapper)
{
  if (journeyItinWrapper.getLeg().stopOverLegFlag())
    return;

  ItinIndex& carrierIndex = journeyItinWrapper.getLeg().carrierIndex();
  ItinIndex::ItinIndexIterator ItinIter = carrierIndex.beginRow(journeyItinWrapper.getCarrierKey());
  ItinIndex::ItinIndexIterator IterEnd = carrierIndex.endRow();

  uint32_t legId = journeyItinWrapper.getLegId();

  invalidateFailedSops_impl(trx, fareMarket, ItinIter, IterEnd, legId);
}

// Same as above but for use in a regular IS ( not NGS )
void
FareUtil::invalidateFailedSops(ShoppingTrx& trx,
                               FareMarket* fareMarket,
                               const ItinIndex::Key& cxrKey,
                               ShoppingTrx::Leg& curLeg,
                               uint32_t legId)
{
  if (curLeg.stopOverLegFlag())
    return;

  ItinIndex& carrierIndex = curLeg.carrierIndex();
  ItinIndex::ItinIndexIterator ItinIter = carrierIndex.beginRow(cxrKey);
  ItinIndex::ItinIndexIterator IterEnd = carrierIndex.endRow();

  invalidateFailedSops_impl(trx, fareMarket, ItinIter, IterEnd, legId);
}

void
FareUtil::invalidateFailedSops_impl(ShoppingTrx& trx,
                                    FareMarket* fareMarket,
                                    ItinIndex::ItinIndexIterator& ItinIter,
                                    ItinIndex::ItinIndexIterator& IterEnd,
                                    uint32_t legId)
{
  uint32_t bitValue = getIndexOfTheFirstSopCoveredByValidFare(fareMarket);

  if (bitValue == 0)
    return;

  std::vector<int> invalidSops =
      markDirectSopsUpToIndexAsInvalid(bitValue, trx, ItinIter, IterEnd, legId);

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic911 && !invalidSops.empty()))
  {
    DiagManager diag(trx, trx.diagnostic().diagnosticType());
    printInvalidatedSopsToDiag911(fareMarket, invalidSops, diag);
  }
}

// Returns index of the first SOP covered by a valid fare. Returns std::limits<uint32_t>::max() if
// no valid SOP was found
uint32_t
FareUtil::getIndexOfTheFirstSopCoveredByValidFare(FareMarket* fareMarket)
{
  const uint32_t SUFFICIENTLY_LARGE = std::numeric_limits<uint32_t>::max();

  std::vector<uint32_t> indexesPerPaxVec;

  std::vector<PaxTypeBucket>::const_iterator cortegeIt = fareMarket->paxTypeCortege().begin();

  // For each PAX Type
  for (; cortegeIt != fareMarket->paxTypeCortege().end(); ++cortegeIt)
  {
    uint32_t bitValue = SUFFICIENTLY_LARGE;
    // If we don't have any fares for this PAX we decide that all SOPs are invalid for our purposes
    // as we need to return a complete response with all required pax types
    if (cortegeIt->paxTypeFare().empty())
    {
      return SUFFICIENTLY_LARGE;
    }

    std::vector<PaxTypeFare*>::const_iterator fareIt = cortegeIt->paxTypeFare().begin();

    // Foreach PaxTypeFare
    for (; fareIt != cortegeIt->paxTypeFare().end(); ++fareIt)
    {
      // Flight Bitmap holds one bit for every SOP
      const uint32_t flightBitmapSize = (*fareIt)->getFlightBitmapSize();

      for (unsigned int bitIndex = 0; bitIndex < flightBitmapSize; ++bitIndex)
      {
        if (bitIndex >= bitValue)
          break;

        if ((*fareIt)->isFlightValid(bitIndex))
        {
          bitValue = bitIndex;
        }
      }
      if (bitValue == 0)
        break;
    }
    indexesPerPaxVec.push_back(bitValue);
  }

  // returning the index of the first sop that has a potential of being valid for all PAX types
  std::vector<uint32_t>::iterator maxElement =
           std::max_element(indexesPerPaxVec.begin(),indexesPerPaxVec.end());

  if (maxElement != indexesPerPaxVec.end())
  {
    return *maxElement;
  }

  return 0;
}

std::vector<int>
FareUtil::markDirectSopsUpToIndexAsInvalid(uint32_t bitValue,
                                           ShoppingTrx& trx,
                                           ItinIndex::ItinIndexIterator& ItinIter,
                                           ItinIndex::ItinIndexIterator& IterEnd,
                                           uint32_t legId)
{
  std::vector<int> invalidSops;

  for (uint32_t bitId = 0; ItinIter != IterEnd; ++ItinIter, ++bitId)
  {
    //  Nothing more to invalidate as we already reached the first valid sop
    if (bitId >= bitValue)
      break;

    // We only invalidate direct sops as they are not shared among itins
    // and the fact that a particular sop is not covered by any fare within this fare market
    // makes it safe to deem it invalid globally
    // Direct itins come first so once we faound something that has more segments we can stop
    // looking
    if (ItinIter->second->travelSeg().size() > 1)
      break;

    ItinIndex::ItinCellInfo& curCellInfo = ItinIter->first;
    uint32_t sopId = ShoppingUtil::findSopId(trx, legId, curCellInfo.sopIndex());

    trx.legs()[legId].sop()[curCellInfo.sopIndex()].cabinClassValid() = false;

    invalidSops.push_back(sopId);
  }

  return invalidSops;
}

void
FareUtil::printInvalidatedSopsToDiag911(FareMarket* fareMarket,
                                        std::vector<int>& invalidSop,
                                        DiagManager& diag)
{
  diag << "SOPS MARKED IN FVO AS INVALID FOR ADDITIONAL NON STOPS FOS GENERATION:\n";
  diag << fareMarket->boardMultiCity() << "-" << fareMarket->offMultiCity() << ":"
       << fareMarket->governingCarrier() << ":"
       << "SOP ID - ";

  for (const auto& elem : invalidSop)
    diag << elem << " ";

  diag << "\n";
}

bool
FareUtil::postCheckOutboundFare(const bool hasRemoveOutboundFares,
                                const bool isReversed,
                                const Directionality directionality)
{
  return (hasRemoveOutboundFares &&
          ((directionality == TO && isReversed) || (directionality == FROM && !isReversed)));
}

} // namespace tse
