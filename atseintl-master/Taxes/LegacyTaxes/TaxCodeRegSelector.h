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

#include <vector>

namespace tse
{
class PricingTrx;
class TaxCodeReg;

class TaxCodeRegSelector
{
  std::vector<TaxCodeReg*> _itinTaxCodeReg;
  std::vector<TaxCodeReg*> _ocTaxCodeReg;
  std::vector<TaxCodeReg*> _changeFeeTaxCodeReg;

  std::vector<TaxCodeReg*>* selectVectorNewItin(PricingTrx& trx, TaxCodeReg* taxCodeReg);
  std::vector<TaxCodeReg*>* selectVectorExcItin(PricingTrx& trx, TaxCodeReg* taxCodeReg);

public:
  TaxCodeRegSelector(PricingTrx& trx, std::vector<TaxCodeReg*> taxCodeRegVector);
  std::vector<TaxCodeReg*> getItin() const { return _itinTaxCodeReg; }
  std::vector<TaxCodeReg*> getOC() const { return _ocTaxCodeReg; }
  std::vector<TaxCodeReg*> getChangeFee() const { return _changeFeeTaxCodeReg; }
  bool hasItin() const { return !_itinTaxCodeReg.empty(); }
  bool hasOC() const { return !_ocTaxCodeReg.empty(); }
  bool hasChangeFee() const { return !_changeFeeTaxCodeReg.empty(); }
};

} // end of tse namespace
