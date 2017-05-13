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
#ifndef TAX_UI_H
#define TAX_UI_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 50 for French Value Added Adjustment
//---------------------------------------------------------------------------

class TaxUI : public Tax
{
  friend class TaxUITest;

public:
  static const std::string TAX_CODE_FR1;
  static const std::string TAX_CODE_FR4;
  static const std::string TAX_CODE_QW;
  static const std::string TAX_CODE_IZ;

  TaxUI();
  virtual ~TaxUI();

  void applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  static bool isNonFrOrYqTax(const TaxItem* taxItem);

private:

  TaxUI(const TaxUI& tax);
  TaxUI& operator=(const TaxUI& tax);
};

} /* end tse namespace */

#endif /* TAX_UI_H */
