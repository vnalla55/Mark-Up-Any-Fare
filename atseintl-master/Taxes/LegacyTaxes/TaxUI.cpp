// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Taxes/LegacyTaxes/TaxUI.h"

#include "Common/TrxUtil.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/BaseTaxOnTaxCollector.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"


using namespace tse;
using namespace std;

const string
TaxUI::TAX_CODE_FR1("FR1");
const string
TaxUI::TAX_CODE_FR4("FR4");
const string
TaxUI::TAX_CODE_QW("QW");
const string
TaxUI::TAX_CODE_IZ("IZ");

namespace
{

class TaxOnTaxCollector : public BaseTaxOnTaxCollector<TaxUI>
{
public:
  TaxOnTaxCollector(TaxUI& taxUI)
    : Base(taxUI)
    , _taxAmountFR1(0.0)
    , _taxAmountFR4(0.0)
    , _taxAmountQW(0.0)
    , _taxAmountIZ(0.0)
    , _serviceFee(0.0)
    , _isNonFrYqTax(false)
  {}

  MoneyAmount taxAmountFR1() const { return _taxAmountFR1; }
  MoneyAmount taxAmountQW()  const { return _taxAmountQW; }
  MoneyAmount serviceFee()   const { return _serviceFee; }

  MoneyAmount getTaxesSum() const
  {
    return _taxAmountFR1 + _taxAmountQW + _taxAmountFR4 + _taxAmountIZ + _serviceFee;
  }

  bool isNonFrYqTax() const { return _isNonFrYqTax; }

  void collect(const TaxResponse::TaxItemVector& taxItemVec)
  {
    TaxResponse::TaxItemVector::const_iterator taxItemI;
    for (taxItemI = taxItemVec.begin(); taxItemI != taxItemVec.end(); taxItemI++)
    {
      applyTaxItem(_taxAmountFR1, *taxItemI, TaxUI::TAX_CODE_FR1);
      applyTaxItem(_taxAmountFR4, *taxItemI, TaxUI::TAX_CODE_FR4);
      applyTaxItem(_taxAmountQW, *taxItemI, TaxUI::TAX_CODE_QW);
      applyTaxIZ(*taxItemI);
      applyTaxItem(_serviceFee, *taxItemI, YQI);
      applyTaxItem(_serviceFee, *taxItemI, YQF);

      if (TaxUI::isNonFrOrYqTax(*taxItemI))
      {
        _isNonFrYqTax = true;
        break;
      }

      _tax.updateCalculationDetails(*taxItemI);
    }
  }

private:
  void applyTaxIZ(const TaxItem* const taxItem)
  {
    if (taxItem->taxCode().substr(0, 2) == TaxUI::TAX_CODE_IZ)
    {
      _taxAmountIZ += taxItem->taxAmount();
      _mixedTax = true;
    }
  }

private:
  MoneyAmount _taxAmountFR1;
  MoneyAmount _taxAmountFR4;
  MoneyAmount _taxAmountQW;
  MoneyAmount _taxAmountIZ;
  MoneyAmount _serviceFee;
  bool _isNonFrYqTax;
};

} // namespace anonymous

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxUI::TaxUI() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxUI::~TaxUI() {}

// ----------------------------------------------------------------------------
// Description:  applyTaxOnTax
// ----------------------------------------------------------------------------
void
TaxUI::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  TaxOnTaxCollector collector(*this);
  collector.collect(taxResponse.taxItemVector());
  if (TrxUtil::isShoppingTaxRequest(&trx) && taxResponse.farePath())
    collector.collect(taxResponse.farePath()->getExternalTaxes());

  if (collector.isMixedTax())
    _mixedTax = true;

  if (((!collector.taxAmountFR1()) && (!collector.taxAmountQW())) || collector.isNonFrYqTax())
  {
    _taxAmount = 0.0;
    _failCode = TaxDiagnostic::NO_TAX_ADDED;

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_TAX_ADDED, Diagnostic820);

    return;
  }

  _taxableFare = collector.getTaxesSum();
  _taxAmount = _taxableFare * taxCodeReg.taxAmt();

}

// ----------------------------------------------------------------------------
// Description:  isNonFrOrYqTax
// If any other tax than FR1, FR4, QW, IZ, and YQ's price on an itinerary
// the UI tax does not apply.
// ----------------------------------------------------------------------------
bool
TaxUI::isNonFrOrYqTax(const TaxItem* taxItem)
{
  return taxItem->taxCode() != TAX_CODE_FR1 && taxItem->taxCode() != TAX_CODE_FR4 &&
         taxItem->taxCode() != TAX_CODE_QW && taxItem->taxCode().substr(0, 2) != TAX_CODE_IZ &&
         taxItem->taxCode() != YQF && taxItem->taxCode() != YQI && taxItem->taxCode() != YRF &&
         taxItem->taxCode() != YRI && taxItem->taxAmount() != 0;
}
