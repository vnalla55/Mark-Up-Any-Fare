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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include <boost/optional.hpp>

namespace tse
{
struct SOPInfo;

class SopCombinationUtil
{
public:
  enum NonStopType
  {
    ONLINE_NON_STOP = 0x01,
    INTERLINE_NON_STOP = 0x02,
    NON_STOP = 0x03,
    NOT_A_NON_STOP = 0x0
  };

  static const char DIAG_ONLINE_NON_STOP;
  static const char DIAG_INTERLINE_NON_STOP;
  static const char DIAG_NOT_A_NON_STOP;

  /**
   * Returns empty carrier code for interline Diversity solution.
   *
   * A solution is considered as interline for Diversity,
   * if outbound and inbound options have different governing carrier.
   */
  static CarrierCode detectCarrier(const ShoppingTrx::SchedulingOption* outbound,
                                   const ShoppingTrx::SchedulingOption* inbound);

  static void detectCarrier(const ShoppingTrx::SchedulingOption* outbound,
                            const ShoppingTrx::SchedulingOption* inbound,
                            boost::optional<CarrierCode>& carrierCode);

  static CarrierCode detectCarrier(const ShoppingTrx& trx, const shpq::SopIdxVecArg sopVec);

  /**
   * Detect OB/IB governing carrier
   *
   * For one-way pair.second is empty
   */
  static std::pair<CarrierCode, CarrierCode>
  detectCarrierPair(const ShoppingTrx& trx, const shpq::SopIdxVecArg sopVec);

  static NonStopType detectNonStop(const ShoppingTrx::SchedulingOption* outbound,
                                   const ShoppingTrx::SchedulingOption* inbound);

  static NonStopType detectNonStop(const ShoppingTrx& trx, shpq::SopIdxVecArg sopVec);

  static bool mayBeNonStop(const ShoppingTrx::SchedulingOption& sop);

  static int32_t getDuration(const ShoppingTrx::SchedulingOption* outbound,
                             const ShoppingTrx::SchedulingOption* inbound);

  static int32_t getDuration(const ShoppingTrx::SchedulingOption& sop);

  static void getSops(const ShoppingTrx& trx,
                      const shpq::SopIdxVecArg sopVec,
                      const ShoppingTrx::SchedulingOption** outbound,
                      const ShoppingTrx::SchedulingOption** inbound);

  static const ShoppingTrx::SchedulingOption&
  getSop(const ShoppingTrx& trx, unsigned legId, unsigned sopId);

  static char getDiagNonStopType(SopCombinationUtil::NonStopType type);

  static char getDiagNonStopType(const ShoppingTrx::SchedulingOption* outbound,
                                 const ShoppingTrx::SchedulingOption* inbound);

  static bool isValidForCombination(const ShoppingTrx& trx, unsigned legId, const SOPInfo& sopInfo);
};

} // tse

