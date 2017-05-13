//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
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

#pragma once

#include "Common/Assert.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"


#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace tse
{

namespace utils
{

inline bool
isBrandedFaresRequest(const ShoppingTrx& trx)
{
  const PricingRequest* pricingRequest = trx.getRequest();
  TSE_ASSERT(nullptr != pricingRequest);
  return pricingRequest->isBrandedFaresRequest();
}

inline bool
isAllFlightsRepresentedDiversity(const ShoppingTrx& trx)
{
  const PricingRequest* pricingRequest = trx.getRequest();
  TSE_ASSERT(nullptr != pricingRequest);
  return pricingRequest->isAllFlightsRepresented();
}

inline bool
isSopDirect(const ::tse::ShoppingTrx::SchedulingOption& sop)
{
  const Itin* itin = sop.itin();
  TSE_ASSERT(itin != nullptr);
  TSE_ASSERT(itin->travelSeg().size() > 0);
  return (itin->travelSeg().size() == 1);
}

// Get the number of non Across-Stopover legs
inline unsigned int
getNonAsoLegsCount(const ShoppingTrx& aTrx)
{
  unsigned int numLegs = 0;
  while ((numLegs < aTrx.legs().size()) && (!aTrx.legs()[numLegs].stopOverLegFlag()))
  {
    ++numLegs;
  }
  return numLegs;
}

std::ostream& operator<<(std::ostream& out, const SopEntry& t);

std::ostream& operator<<(std::ostream& out, const SopCombination& v);

std::ostream& operator<<(std::ostream& out, const SopCombinationList& v);

std::ostream& operator<<(std::ostream& out, const SopCandidate& c);

std::ostream& operator<<(std::ostream& out, const TravelSeg& segment);

std::ostream& operator<<(std::ostream& out, const ::tse::ShoppingTrx::SchedulingOption& sop);

const ShoppingTrx::SchedulingOption&
findSopInTrx(unsigned int legId, unsigned int sopId, const ShoppingTrx& trx);

void
addToFlightMatrix(tse::ShoppingTrx& trx,
                  tse::ItinStatistic& stats,
                  const SopCombinationList& options);

void
addToFlightMatrix(ShoppingTrx::FlightMatrix& matrix, const SopCombinationList& options);

CarrierCode
getRequestingCarrierCodeForTrx(const ShoppingTrx& trx);

std::size_t
hash_value(const SopEntry& se);


inline bool equal(double x, double y, double epsilon = 1e-5)
{
  return std::abs(x - y) <= epsilon * std::max(std::abs(x), std::abs(y));
}


template<typename C>
inline double mean(const C& c)
{
  TSE_ASSERT(!c.empty());
  double sum = 0;
  for (const auto& elem : c)
  {
    sum += elem;
  }
  return sum / c.size();
}


template<typename C>
double variance(const C& c)
{
  const double avg = mean(c);
  double acc = 0;
  for (const auto& elem : c)
  {
    const double diff = elem - avg;
    acc += diff*diff;
  }
  return acc / c.size();
}


template<typename C>
double standard_deviation(const C& c)
{
  return sqrt(variance(c));
}


template<typename C>
double coefficient_of_variation(const C& c)
{
  const double avg = mean(c);
  TSE_ASSERT(avg != 0);
  return standard_deviation(c)/avg;
}


struct CoefficientOfVariation
{
  template<typename C>
  double operator()(const C& c) const
  {
    return coefficient_of_variation(c);
  }
};


} // namespace utils

} // namespace tse

