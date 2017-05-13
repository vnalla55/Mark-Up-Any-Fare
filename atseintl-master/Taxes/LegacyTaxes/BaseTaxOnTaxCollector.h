#ifndef BaseTaxOnTaxCollector_h
#define BaseTaxOnTaxCollector_h

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

namespace tse
{

template<class Tax>
class BaseTaxOnTaxCollector
{
 public:
  BaseTaxOnTaxCollector(Tax& tax)
    : _tax(tax)
    , _mixedTax(false)
  {}

  bool isMixedTax() const { return _mixedTax; }

 protected:
  bool applyTaxItem(MoneyAmount& resultMoney, const TaxItem* const taxItem, const TaxCode& code)
  {
    if (taxItem->taxCode() == code)
    {
      resultMoney += taxItem->taxAmount();
      _mixedTax = true;
      return true;
    }
    return false;
  }

 protected:
  typedef BaseTaxOnTaxCollector<Tax> Base;

  Tax& _tax;
  bool _mixedTax;
};

}

#endif
