//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/ShoppingUtil.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/FosCommonUtil.h"

#include <map>

namespace tse
{

class ShoppingTrx;

class RequiredNonStopsCalculator
{
public:
  RequiredNonStopsCalculator() : _maxNumberOfNS(0u), _numAlreadyGeneratedNS(0u) {}

  void init(const ShoppingTrx& trx);
  void countAlreadyGeneratedNS(const ShoppingTrx& trx, const ShoppingTrx::FlightMatrix& fm);
  void countAlreadyGeneratedNS(const ShoppingTrx& trx, const ShoppingTrx::EstimateMatrix& em);

  // Iterate over all PQs from ShoppingTrx and count all solutions which
  // are non-stop and respect P1W/P41 flags.
  void countAlreadyGeneratedNS(const ShoppingTrx& trx);

  std::size_t calcRequiredNSCount(const ShoppingTrx& trx);

  void countAlreadyGeneratedNSPerCarrier(const ShoppingTrx& trx);

  void calcRequiredNSCountPerCarrier(const ShoppingTrx& trx,
                                     uint32_t totalOptionsRequired,
                                     fos::FosCommonUtil::CarrierMap& requiredMap);

protected:
  template <typename Matrix>
  void countAlreadyGeneratedNSPerCarrier(const ShoppingTrx& trx, const Matrix& em);

private:
  std::size_t _maxNumberOfNS;
  std::size_t _numAlreadyGeneratedNS;
  fos::FosCommonUtil::CarrierMap _numAlreadyGeneratedNSPerCarrier;
};

template <typename Matrix>
void
RequiredNonStopsCalculator::countAlreadyGeneratedNSPerCarrier(const ShoppingTrx& trx,
                                                              const Matrix& em)
{
  if (_maxNumberOfNS == 0)
    return;

  typedef typename Matrix::value_type Solution;
  CarrierCode carrier = Diversity::INTERLINE_CARRIER;

  for (const Solution& sol : em)
  {
    if (!ShoppingUtil::isDirectFlightSolution(trx, sol.first))
      continue;

    carrier = Diversity::INTERLINE_CARRIER;
    if (ShoppingUtil::isOnlineSolution(trx, sol.first))
      carrier = trx.legs()[0].sop()[sol.first[0]].governingCarrier();

    ++_numAlreadyGeneratedNSPerCarrier[carrier];
  }
}

} // tse

