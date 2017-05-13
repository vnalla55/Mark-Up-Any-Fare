// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

namespace tse
{
class Trx;

class RexPricingPhase
{
private:
  bool _isExcPhase = false;
  bool _isNewPhase = false;

  RexPricingPhase(const RexPricingPhase&) = delete;
  RexPricingPhase& operator=(const RexPricingPhase&) = delete;

public:
  RexPricingPhase(Trx* trx);
  bool isExcPhase() const;
  bool isNewPhase() const;
};

} // end of tse namespace
