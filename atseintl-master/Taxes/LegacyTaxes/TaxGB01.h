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
#ifndef TAXGB01_H
#define TAXGB01_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/TaxGB.h"
#include <vector>

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxGB01 : public TaxGB
{

public:
  TaxGB01();
  virtual ~TaxGB01();

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

protected:
  virtual bool isStopOver(const TravelSeg* pTvlSeg,
                          const TravelSeg* pTvlSegPrev,
                          bool isTravelWithinUK) override;

  virtual bool isForcedStopOver(const TravelSeg* pTvlSeg, const TravelSeg* pTvlSegPrev) override;

private:
  TaxGB01(const TaxGB01& map);
  TaxGB01& operator=(const TaxGB01& map);
};

} /* end tse namespace */

#endif /* TAXGB01_H */
