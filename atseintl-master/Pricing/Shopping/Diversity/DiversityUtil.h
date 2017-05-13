// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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
#pragma once

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseDateTimeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include <iostream>
#include <utility>
#include <vector>

#include <stdint.h>

namespace tse
{

class DiagCollector;

class DiversityUtil
{
public:
  typedef ShoppingTrx::FlightMatrix::value_type FlightMatrixSolution;

  /**
   * Distinguish interline options using OB/IB carrier combination in DiversityModelAltDates.
   * This utility class encapsulates carrier combination into CompoundCarrier abstraction.
   *
   * For one-way request, only OB governing carrier is taken into account,
   * i.e. we have only online options in OW request.
   *
   * AltDatesStatistic1 neither check nor initializes this as OB/IB carrier combination.
   * So, in this case, only outbound carrier is checked.
   * Thus constructor is adopted for AltDatesStatistic1 as well.
   */
  class CompoundCarrier
  {
  public:
    CompoundCarrier(CarrierCode ob = "", CarrierCode ib = "");
    CompoundCarrier(std::pair<CarrierCode, CarrierCode> cxrPair)
    {
      *this = CompoundCarrier(cxrPair.first, cxrPair.second);
    }

    bool isOnline() const;
    bool isInterline() const { return !isOnline(); }

    CarrierCode toCarrierCode() const;

    // to be a map key
    bool operator<(CompoundCarrier rhs) const
    {
      return std::make_pair(_ob, _ib) < std::make_pair(rhs._ob, rhs._ib);
    }

    bool operator==(CompoundCarrier rhs) const { return (_ob == rhs._ob) && (_ib == rhs._ib); }

    friend std::ostream& operator<<(std::ostream& os, CompoundCarrier cxr);
    friend DiagCollector& operator<<(DiagCollector& dc, CompoundCarrier cxr);

  private:
    CarrierCode _ob;
    CarrierCode _ib; // must be empty when the fallback is activated
  };

  /**
   * ADSolultionKind is used to sort options in AltDate Tax and group in AltDatesStatistic
   */
  enum ADSolultionKind
  {
    Online = 0x1,
    Interline = 0x2,
    Snowman = 0x4, // this is currently used to identify a snowman by mask
    OnlineSnowman = (Online | Snowman),
    InterlineSnowman = (Interline | Snowman),
  };

  static ADSolultionKind
  getSolutionKind(const ShoppingTrx&, const FlightMatrixSolution& sol, CompoundCarrier carrier);

  static ADSolultionKind getSolutionKind(CompoundCarrier carrier, bool isSnowman);

  static bool detectSnowman(const ShoppingTrx&, shpq::SopIdxVecArg sops);

  /**
   * This is a variant of ShoppingUtil::IsSolutionSimilar, which
   * - does not check departure date thru travel segments
   * - unconditionally checks marketing carrier
   */
  static bool
  isSolutionSimilar(const ShoppingTrx& trx, shpq::SopIdxVecArg lhSops, shpq::SopIdxVecArg rhSops);

  static std::size_t getTravelSegCount(const ShoppingTrx& trx, const shpq::SopIdxVecArg sops)
  {
    return ShoppingUtil::getTravelSegCount(trx, sops);
  }

  static DatePair getDatePairSops(const ShoppingTrx& trx, shpq::SopIdxVecArg sops);

  static int32_t getBestDuration(const ShoppingTrx& trx);
};

std::ostream& operator<<(std::ostream& s, DiversityUtil::ADSolultionKind);

} // ns tse
