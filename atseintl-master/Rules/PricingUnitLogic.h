//----------------------------------------------------------------------------
//  Copyright Sabre 2010
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

namespace tse
{

class FarePath;
class PricingUnit;
class PricingTrx;

class PricingUnitLogic
{
  friend class PricingUnitLogicTest;

public:
  static bool shouldConvertRTtoCT(PricingTrx& trx, PricingUnit& pu, const FarePath& fp);
};

} /* end tse namespace */

