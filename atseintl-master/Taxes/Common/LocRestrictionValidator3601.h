//---------------------------------------------------------------------------
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

#ifndef LOC_RESTRICTION_VALIDATOR_3601_H
#define LOC_RESTRICTION_VALIDATOR_3601_H

#include "Taxes/Common/LocRestrictionValidator.h"

namespace tse
{

class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class FarePath;
class Itin;

class LocRestrictionValidator3601 : public LocRestrictionValidator
{
  friend class TaxSP3601Test;

public:
  LocRestrictionValidator3601()
    : _farthestSegIndex(0), _farthestPointFound(false), _fareBreaksFound(false)
  {
  }
  virtual void findFareBreaks(const FarePath& farePath);
  virtual void findFarthestPoint(PricingTrx& trx, const Itin& itin, const uint16_t& startIndex);
  bool validateDestination(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t& startIndex,
                           uint16_t& endIndex) override;
  bool fareBreaksFound() const { return _fareBreaksFound; }
  uint16_t getFarthestSegIndex() const { return _farthestSegIndex; }

private:
  uint16_t _farthestSegIndex;
  bool _farthestPointFound;
  bool _fareBreaksFound;
  std::set<uint16_t> _fareBreaksSet;
};

}
#endif
