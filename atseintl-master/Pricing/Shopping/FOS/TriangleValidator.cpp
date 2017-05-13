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

#include "Pricing/Shopping/FOS/TriangleValidator.h"

#include "Common/Assert.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/FosBaseGenerator.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"

namespace tse
{

namespace fos
{
TriangleValidator::TriangleValidator(ShoppingTrx& trx,
                                     FosBaseGenerator& generator,
                                     FosStatistic& stats)
  : BaseValidator(trx, generator, stats), _cheapestThruFM(nullptr)
{
  prepareCandidates();
}

BaseValidator::ValidationResult
TriangleValidator::validate(const SopIdVec& combination) const
{
  TSE_ASSERT(combination.size() == 2);

  uint32_t count = getStats().getCounter(getType());
  uint32_t limit = getStats().getCounterLimit(getType());

  if (count >= limit || !_cheapestThruFM)
    return INVALID;
  if (ShoppingUtil::isDirectFlightSolution(getTrx(), combination))
    return INVALID;

  uint16_t cheapestLegId = _cheapestThruFM->legIndex();
  uint16_t otherLegId = cheapestLegId == 0 ? 1 : 0;

  if (_triangleCandidates.find(combination[cheapestLegId]) == _triangleCandidates.end())
    return INVALID;

  int cheapestSopId = combination[cheapestLegId];
  ShoppingTrx::SchedulingOption& cheapestSop = getTrx().legs()[cheapestLegId].sop()[cheapestSopId];

  const SopDetailsPtrVec detailsVec =
      getGenerator().getSopDetails(otherLegId, combination[otherLegId]);

  for (const SopDetails* details : detailsVec)
  {
    if (details->cxrCode[0] == cheapestSop.governingCarrier())
      return VALID;
  }

  return INVALID_SOP_DETAILS;
}

bool
TriangleValidator::isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const
{
  return false;
}

void
TriangleValidator::prepareCandidates()
{
  _triangleCandidates.clear();
  _cheapestThruFM = nullptr;

  MoneyAmount amount = 0.0;
  MoneyAmount cheapestAmount = 0.0;
  PaxTypeFare* cheapestPTF = nullptr;

  for (FareMarket* fareMarket : getTrx().fareMarket())
  {
    if (FareMarket::SOL_FM_THRU == fareMarket->getFmTypeSol() && fareMarket->getApplicableSOPs())
    {
      PaxTypeFare* ptf = getCheapestPaxTypeFare(fareMarket);
      if (!ptf)
        continue;

      amount = ptf->totalFareAmount();
      if (!_cheapestThruFM || amount < cheapestAmount)
      {
        _cheapestThruFM = fareMarket;
        cheapestAmount = amount;
        cheapestPTF = ptf;
      }
    }
  }

  if (!_cheapestThruFM)
    return;

  const ApplicableSOP* sops(_cheapestThruFM->getApplicableSOPs());

  for (const auto& sop : *sops)
  {
    cheapestPTF->setComponentValidationForCarrier(sop.first, getTrx().isAltDates(), 0);

    const SOPUsages& sopUsages = sop.second;

    SOPUsages::const_iterator sopIt = sopUsages.begin();
    for (uint32_t bitIndex = 0; sopIt != sopUsages.end(); ++sopIt, ++bitIndex)
    {
      if (sopIt->applicable_ && cheapestPTF->isFlightValid(bitIndex))
        _triangleCandidates.insert(ShoppingUtil::findInternalSopId(
            getTrx(), _cheapestThruFM->legIndex(), sopIt->origSopId_));
    }
  }
}

PaxTypeFare*
TriangleValidator::getCheapestPaxTypeFare(FareMarket* fareMarket) const
{
  if (!fareMarket->getCheapestOW() && !fareMarket->getCheapestHRT())
    return nullptr;

  if (!fareMarket->getCheapestOW())
    return fareMarket->getCheapestHRT();
  else if (!fareMarket->getCheapestHRT())
    return fareMarket->getCheapestOW();
  else if (fareMarket->getCheapestOW()->totalFareAmount() <=
           fareMarket->getCheapestHRT()->totalFareAmount())
    return fareMarket->getCheapestOW();
  return fareMarket->getCheapestHRT();
}

} // fos
} // tse
