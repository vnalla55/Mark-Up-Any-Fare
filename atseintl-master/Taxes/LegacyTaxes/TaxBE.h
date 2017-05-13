//---------------------------------------------------------------------------
//  Copyright Sabre 2008
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
#ifndef TAX_BE_H
#define TAX_BE_H

#include "Taxes/LegacyTaxes/Tax.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/Loc.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 7500 for BE Tax
//---------------------------------------------------------------------------

class TaxBE : public Tax
{
  friend class TaxBETest;

  typedef std::vector<const Loc*> HiddenStopsType;

  static const LocCode BRUSSELS_CODE;

public:
  TaxBE();
  virtual ~TaxBE();

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;

private:
  bool isHiddenStopInBRU(const HiddenStopsType& hiddenStops) const;

  TaxBE(const TaxBE&);
  TaxBE& operator=(const TaxBE&);
};

} /* end tse namespace */

#endif /* TAX_BE_H */
