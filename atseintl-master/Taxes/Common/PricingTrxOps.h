// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
  class FarePath;
  class Itin;
  class PricingTrx;
  class TaxResponse;
}

namespace tse
{

void addTaxResponseInAdvance(tse::TaxResponse& taxResponse, tse::PricingTrx& trx, tse::Itin& itin);

void addUniqueTaxResponse(tse::TaxResponse& taxResponse, tse::PricingTrx& trx);

void addUniqueTaxResponse(tse::TaxResponse& taxResponse, tse::PricingTrx& trx, FarePath& farePath);

}

