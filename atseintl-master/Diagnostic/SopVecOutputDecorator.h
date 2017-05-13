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

#include <iomanip>

#include "Common/ShoppingUtil.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{

class PricingTrx;

class SopVecOutputDecorator
{
public:
  SopVecOutputDecorator(const PricingTrx& trx, const SopIdVec& intSopVec)
    : _trx(trx), _internalSops(intSopVec), _width(0)
  {
  }

  SopVecOutputDecorator& width(int w)
  {
    _width = w;
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& dc, const SopVecOutputDecorator& dec)
  {
    const bool useWidth = !_debug;

    for (std::size_t leg = 0, numLeg = dec._internalSops.size(); leg < numLeg; ++leg)
    {
      if (useWidth && dec._width)
        dc << std::setw(dec._width / static_cast<int>(numLeg));
      else if (leg > 0)
        dc << ",";

      int intSopId = dec._internalSops[leg];
      dc << ShoppingUtil::findSopId(dec._trx, static_cast<LegId>(leg), intSopId);
    }

    // print also internal sops
    if (_debug)
    {
      dc << "/";
      for (std::size_t leg = 0; leg < dec._internalSops.size(); ++leg)
      {
        if (leg > 0)
          dc << ",";

        int intSopId = dec._internalSops[leg];
        dc << intSopId;
      }
    }

    return dc;
  }

private:
  const PricingTrx& _trx;
  SopIdVec _internalSops;
  int _width;

  static constexpr bool _debug = false;
};

} // ns tse

