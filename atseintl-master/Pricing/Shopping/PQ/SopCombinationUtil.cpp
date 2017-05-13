/*---------------------------------------------------------------------------
 *  Copyright Sabre 2013
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include "Common/FallbackUtil.h"
#include "DataModel/Diversity.h"
#include "Pricing/Shopping/PQ/SOPInfo.h"

namespace tse
{

const char SopCombinationUtil::DIAG_ONLINE_NON_STOP = 'O';
const char SopCombinationUtil::DIAG_INTERLINE_NON_STOP = 'I';
const char SopCombinationUtil::DIAG_NOT_A_NON_STOP = 'N';

FIXEDFALLBACK_DECL(fallbackShoppingPQCabinClassValid);

CarrierCode
SopCombinationUtil::detectCarrier(const ShoppingTrx::SchedulingOption* outbound,
                                  const ShoppingTrx::SchedulingOption* inbound)
{
  CarrierCode cxr = outbound->governingCarrier();
  if (inbound && inbound->governingCarrier() != cxr)
    cxr = Diversity::INTERLINE_CARRIER;
  return cxr;
}

void
SopCombinationUtil::detectCarrier(const ShoppingTrx::SchedulingOption* outbound,
                                  const ShoppingTrx::SchedulingOption* inbound,
                                  boost::optional<CarrierCode>& carrierCode)
{
  if (UNLIKELY(!carrierCode.is_initialized()))
    carrierCode = detectCarrier(outbound, inbound);
}

CarrierCode
SopCombinationUtil::detectCarrier(const ShoppingTrx& trx, const shpq::SopIdxVecArg sopVec)
{
  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  getSops(trx, sopVec, &outbound, &inbound);

  CarrierCode cxr = detectCarrier(outbound, inbound);
  return cxr;
}

std::pair<CarrierCode, CarrierCode>
SopCombinationUtil::detectCarrierPair(const ShoppingTrx& trx, const shpq::SopIdxVecArg sopVec)
{
  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  getSops(trx, sopVec, &outbound, &inbound);

  std::pair<CarrierCode, CarrierCode> result(outbound->governingCarrier(),
                                             inbound ? inbound->governingCarrier() : CarrierCode());
  return result;
}

SopCombinationUtil::NonStopType
SopCombinationUtil::detectNonStop(const ShoppingTrx::SchedulingOption* outbound,
                                  const ShoppingTrx::SchedulingOption* inbound)
{
  if (outbound->itin()->travelSeg().size() != 1)
    return NOT_A_NON_STOP;

  if (!inbound)
    return ONLINE_NON_STOP;

  if (inbound->itin()->travelSeg().size() != 1)
    return NOT_A_NON_STOP;

  if (outbound->governingCarrier() == inbound->governingCarrier())
    return ONLINE_NON_STOP;

  return INTERLINE_NON_STOP;
}

SopCombinationUtil::NonStopType
SopCombinationUtil::detectNonStop(const ShoppingTrx& trx, const shpq::SopIdxVecArg sopVec)
{
  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  getSops(trx, sopVec, &outbound, &inbound);
  return detectNonStop(outbound, inbound);
}

bool
SopCombinationUtil::mayBeNonStop(const ShoppingTrx::SchedulingOption& sop)
{
  return sop.itin()->travelSeg().size() == 1;
}

int32_t
SopCombinationUtil::getDuration(const ShoppingTrx::SchedulingOption* outbound,
                                const ShoppingTrx::SchedulingOption* inbound)
{
  int32_t duration = outbound->itin()->getFlightTimeMinutes();
  if (inbound)
    duration += inbound->itin()->getFlightTimeMinutes();

  return duration;
}

int32_t
SopCombinationUtil::getDuration(const ShoppingTrx::SchedulingOption& sop)
{
  return sop.itin()->getFlightTimeMinutes();
}

void
SopCombinationUtil::getSops(const ShoppingTrx& trx,
                            const shpq::SopIdxVecArg sopVec,
                            const ShoppingTrx::SchedulingOption** outbound,
                            const ShoppingTrx::SchedulingOption** inbound)
{
  *outbound = nullptr;
  *inbound = nullptr;
  *outbound = &trx.legs()[0].sop()[sopVec[0]];
  if (sopVec.size() > 1)
    *inbound = &trx.legs()[1].sop()[sopVec[1]];
}

const ShoppingTrx::SchedulingOption&
SopCombinationUtil::getSop(const ShoppingTrx& trx, unsigned legId, unsigned sopId)
{
  return trx.legs()[legId].sop()[sopId];
}

char
SopCombinationUtil::getDiagNonStopType(const ShoppingTrx::SchedulingOption* outbound,
                                       const ShoppingTrx::SchedulingOption* inbound)
{
  SopCombinationUtil::NonStopType type = detectNonStop(outbound, inbound);
  return getDiagNonStopType(type);
}

char
SopCombinationUtil::getDiagNonStopType(SopCombinationUtil::NonStopType type)
{
  if (type & ONLINE_NON_STOP)
    return DIAG_ONLINE_NON_STOP;

  if (type & INTERLINE_NON_STOP)
    return DIAG_INTERLINE_NON_STOP;

  return DIAG_NOT_A_NON_STOP;
}

bool
SopCombinationUtil::isValidForCombination(const ShoppingTrx& trx,
                                          unsigned legId,
                                          const SOPInfo& sopInfo)
{
  bool isValid = sopInfo.isStatusValid();
  if (isValid && !fallback::fixed::fallbackShoppingPQCabinClassValid())
  {
    const ShoppingTrx::SchedulingOption& sop =
        SopCombinationUtil::getSop(trx, legId, sopInfo._sopIndex);
    isValid = sop.cabinClassValid();
  }

  return isValid;
}

} // tse
