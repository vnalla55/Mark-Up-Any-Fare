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

#include "Taxes/LegacyTaxes/TaxSP46.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxSP46::TaxSP46() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxSP46::~TaxSP46() {}

// ----------------------------------------------------------------------------
// Description:  Override validateTripTypes for From Via To
// ----------------------------------------------------------------------------

bool
TaxSP46::validateTripTypes(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t& startIndex,
                           uint16_t& endIndex)
{
  return true;
}

// ----------------------------------------------------------------------------
// Description:  Override validateLocRestrictions for From Via To
// ----------------------------------------------------------------------------

bool
TaxSP46::validateLocRestrictions(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 uint16_t& startIndex,
                                 uint16_t& endIndex)
{
  return true;
}

// ----------------------------------------------------------------------------
// Description:  TaxCreate
// ----------------------------------------------------------------------------

void
TaxSP46::taxCreate(PricingTrx& trx,
                   TaxResponse& taxResponse,
                   TaxCodeReg& taxCodeReg,
                   uint16_t travelSegStartIndex,
                   uint16_t travelSegEndIndex)
{
  Tax tax;

  tax.taxCreate(trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex);

  _taxAmount = tax.taxAmount();
  _taxableFare = tax.taxableFare();
  _paymentCurrency = tax.paymentCurrency();
  _paymentCurrencyNoDec = tax.paymentCurrencyNoDec();
  _travelSegStartIndex = travelSegStartIndex - 1;
  _travelSegEndIndex = travelSegStartIndex;
  _intermediateCurrency = tax.intermediateCurrency();
  _intermediateNoDec = tax.intermediateNoDec();
  _exchangeRate1 = tax.exchangeRate1();
  _exchangeRate1NoDec = tax.exchangeRate1NoDec();
  _exchangeRate2 = tax.exchangeRate2();
  _exchangeRate2NoDec = tax.exchangeRate2NoDec();
  _intermediateUnroundedAmount = tax.intermediateUnroundedAmount();
  _intermediateAmount = tax.intermediateAmount();
}
