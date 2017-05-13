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

#include "Pricing/Shopping/Diversity/DsciDiversityItinerarySwapperAdapter.h"

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

namespace tse
{

int
DsciDiversityItinerarySwapperAdapter::isBetterByTODDist(size_t newTODBucket,
                                                        const Operand& lhs,
                                                        const Operand& rhs)
{
  _isTodDistanceResultLastUsed = true;

  size_t lhsTODBucket = lhs._outbound->itin()->getTODBucket(_trx.diversity().getTODRanges());
  size_t rhsTODBucket = rhs._outbound->itin()->getTODBucket(_trx.diversity().getTODRanges());

  if (lhsTODBucket == rhsTODBucket)
    return 0;

  float lhsTODDist = calcTODDistance(lhsTODBucket, newTODBucket);
  float rhsTODDist = calcTODDistance(rhsTODBucket, newTODBucket);

  if (fabsf(lhsTODDist - rhsTODDist) < EPSILON)
    return 0;

  if (rhsTODDist - lhsTODDist >= EPSILON)
    return 1;

  return -1;
}

float
DsciDiversityItinerarySwapperAdapter::calcTODDistance(size_t decreasedBkt, size_t increasedBkt)
    const
{
  float result = 0.0;
  for (size_t bkt = 0; bkt < _trx.diversity().getTODDistribution().size(); bkt++)
  {
    size_t bktCount = _stats.getTODBucketSize(bkt);
    if (bkt == decreasedBkt)
      bktCount--;
    if (bkt == increasedBkt)
      bktCount++;

    size_t totalCount = _stats.getTotalOptionsCount();
    if (!_stats.considerAdditionalNsInTodAndOIPairing())
      totalCount -= _stats.getAdditionalNonStopsCount();

    float bktDif = (float)(bktCount) / totalCount - _trx.diversity().getTODDistribution()[bkt];
    result += bktDif * bktDif;
  }
  return sqrtf(result);
}

int
DsciDiversityItinerarySwapperAdapter::isBetterBySolutionScore(Diversity::BucketType bucket,
                                                              const Operand& lhs,
                                                              const Operand& rhs)
{
  DsciSwapperScore lhsScore = calcSolutionScore(bucket, lhs._price, lhs._outbound, lhs._inbound);
  DsciSwapperScore rhsScore = calcSolutionScore(bucket, rhs._price, rhs._outbound, rhs._inbound);

  return compareScores(lhsScore, rhsScore);
}

int
DsciDiversityItinerarySwapperAdapter::compareScores(const DsciSwapperScore& lhsScore,
                                                    const DsciSwapperScore& rhsScore)
{
  int result = -1; // default result when lhsScore is less than rhsScore
  double scoreDiff = lhsScore._primaryScore - rhsScore._primaryScore;

  if (fabs(scoreDiff) < EPSILON)
  {
    scoreDiff = lhsScore._secondaryScore - rhsScore._secondaryScore;
    if (fabs(scoreDiff) < EPSILON)
    {
      result = 0;
    }
    else if (scoreDiff > 0)
    {
      result = 1;
    }
  }
  else if (scoreDiff > 0)
  {
    result = 1;
  }

  return result;
}

DsciSwapperScore
DsciDiversityItinerarySwapperAdapter::calcSolutionScore(
    Diversity::BucketType bucket,
    MoneyAmount price,
    const ShoppingTrx::SchedulingOption* outbound,
    const ShoppingTrx::SchedulingOption* inbound) const
{
  DsciSwapperScore result;
  int32_t duration = SopCombinationUtil::getDuration(outbound, inbound);

  switch (bucket)
  {
  case Diversity::GOLD:
  case Diversity::JUNK:
    result._primaryScore = (price / _stats.getAvgPrice()) * (duration / _stats.getAvgDuration());
    break;
  case Diversity::LUXURY:
    result._primaryScore = duration;
    result._secondaryScore = price;
    break;
  case Diversity::UGLY:
    result._primaryScore = price;
    result._secondaryScore = duration;
    break;
  default:
    break;
  }
  return result;
}

int
DsciDiversityItinerarySwapperAdapter::isBetterByIBFPreference(const Operand& lhs,
                                                              const Operand& rhs)
{
  CarrierCode requestingCarrier = _stats.getRequestingCarrier();
  bool isLhsRCOnline =
      ShoppingUtil::isOnlineOptionForCarrier(lhs._outbound, lhs._inbound, requestingCarrier);
  bool isRhsRCOnline =
      ShoppingUtil::isOnlineOptionForCarrier(rhs._outbound, rhs._inbound, requestingCarrier);

  if (isLhsRCOnline == isRhsRCOnline)
    return 0;

  if (isRhsRCOnline && !isLhsRCOnline)
    return 1;

  return -1;
}

} // tse
