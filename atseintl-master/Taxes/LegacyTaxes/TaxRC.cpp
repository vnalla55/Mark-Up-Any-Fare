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

#include "Taxes/LegacyTaxes/TaxRC.h"

#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/BaseTaxOnTaxCollector.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"

using namespace tse;
using namespace std;

const string
TaxRC::TAX_CODE_CA1("CA1");
const string
TaxRC::TAX_CODE_SQ2("SQ2");

namespace
{

class TaxOnTaxCollector : public BaseTaxOnTaxCollector<TaxRC>
{
public:
  TaxOnTaxCollector(TaxRC& taxRC)
    : Base(taxRC)
    , _taxAmountCA1(0.0)
    , _taxAmountSQ2(0.0)
  {}

  void collect(const TaxResponse::TaxItemVector& taxItemVec)
  {
    TaxResponse::TaxItemVector::const_iterator taxItemI;

    for (taxItemI = taxItemVec.begin(); taxItemI != taxItemVec.end(); taxItemI++)
    {
      if (applyTaxItem(_taxAmountCA1, *taxItemI, TaxRC::TAX_CODE_CA1))
      {
        if (_tax.zeroesBaseFare() || _tax.specialTaxRCInd())
          continue;
      }
      applyTaxItem(_taxAmountSQ2, *taxItemI, TaxRC::TAX_CODE_SQ2);
      _tax.updateCalculationDetails(*taxItemI);
    }

    if (_tax.zeroesBaseFare() || _tax.specialTaxRCInd())
    {
      _tax.setTaxableFare(0.0);
      _tax.setTaxableFareAdjusted(0.0);
      _taxAmountCA1 = 0.0;
      _tax.applyFeeOnTax() = false;
    }

  }

  MoneyAmount taxAmountCA1() const { return _taxAmountCA1; }
  MoneyAmount taxAmountSQ2() const { return _taxAmountSQ2; }
private:
  MoneyAmount _taxAmountCA1;
  MoneyAmount _taxAmountSQ2;
};

} // namespace anonymous

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxRC::TaxRC()
  : _canadianPt(false),
    _origCAMaritime(false),
    _origCANotMaritime(false),
    _usPoint(false),
    _otherIntl(false),
    _zeroesBaseFare(false)
{
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxRC::~TaxRC() {}

// ----------------------------------------------------------------------------
// Description:  validateLocation
// ----------------------------------------------------------------------------

bool
TaxRC::validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex)
{
  return true;
}

// ----------------------------------------------------------------------------
// Description:  TripTypesValidator
// ----------------------------------------------------------------------------

bool
TaxRC::validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)
{
  std::vector<TravelSeg*>::iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();
  endIndex = taxResponse.farePath()->itin()->travelSeg().size() - 1;

  _canadianPt = false;
  _origCAMaritime = false;
  _usPoint = false;
  _otherIntl = false;

  uint16_t index = 0;
  bool toStPierre = false;

  // Check for the origin Maritime Canada - NewBrunswick/NovaScotia/NewFoundLand
  if (LocUtil::isCanada(*(*travelSegI)->origin()))
  {
    if (((*travelSegI)->origin()->state() == NEW_BRUNSWICK) ||
        ((*travelSegI)->origin()->state() == NOVA_SCOTIA) ||
        ((*travelSegI)->origin()->state() == NEW_FOUNDLAND))
    {
      _origCAMaritime = true;
    }
  } // end board is in Canada.

  for (; index <= endIndex; ++index, ++travelSegI)
  {
    const Loc& orig = *(*travelSegI)->origin();
    const Loc& dest = *(*travelSegI)->destination();

    // Check for wholly in Canada -- true if any brd/off in Canada.
    if (LocUtil::isCanada(orig) || LocUtil::isCanada(dest))
    {
      _canadianPt = true;
    }
    // Check for wholly in US -- true if any brd/off in USA.
    if (LocUtil::isUS(orig) || LocUtil::isUS(dest))
    {
      _usPoint = true;
    }
    // Check for International
    if ((!LocUtil::isUS(orig) && !LocUtil::isCanada(orig) && !LocUtil::isStPierreMiquelon(orig)) ||
        (!LocUtil::isUS(dest) && !LocUtil::isCanada(dest) && !LocUtil::isStPierreMiquelon(dest)))
    {
      _otherIntl = true;
    }
    if (LocUtil::isUSTerritoryOnly(orig) || LocUtil::isUSTerritoryOnly(dest))
    {
      _otherIntl = true;
    }
    if (LocUtil::isStPierreMiquelon(dest))
    {
      toStPierre = true;
    }
  } // Loop TravelSeg set Indicators

  // If there is no Canada City, no RC tax applies
  if (!_canadianPt)
    return false;

  if ((_usPoint) || (_otherIntl) || (!_origCAMaritime))
  {
    _zeroesBaseFare = true;
  }

  TripTypesValidator tripTypesValidator;

  bool rcFromTo =
      tripTypesValidator.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);

  if ((rcFromTo == false) && (toStPierre == false))
  {
    _specialTaxRCInd = true;
  }

  return true;
} // end of validateTrip

// ----------------------------------------------------------------------------
// Description:  applyTaxOnTax
// ----------------------------------------------------------------------------
void
TaxRC::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  _applyFeeOnTax = true;

  TaxOnTaxCollector collector(*this);
  collector.collect(taxResponse.taxItemVector());
  if (TrxUtil::isShoppingTaxRequest(&trx) && taxResponse.farePath())
    collector.collect(taxResponse.farePath()->getExternalTaxes());

  if (collector.isMixedTax())
    _mixedTax = true;

  _taxableFare += collector.taxAmountCA1() + collector.taxAmountSQ2();
  _taxAmount = _taxableFare * taxCodeReg.taxAmt();

  _taxableFareAdjusted += collector.taxAmountCA1() + collector.taxAmountSQ2();
  _taxAmountAdjusted = _taxableFareAdjusted * taxCodeReg.taxAmt();

  if (_taxAmount == 0.0)
  {
    _failCode = TaxDiagnostic::NO_TAX_CODE;

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_TAX_CODE, Diagnostic820);
  }
}
