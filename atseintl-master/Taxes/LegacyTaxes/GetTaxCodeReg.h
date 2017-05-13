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

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"

#include <list>
#include <memory>
#include <vector>

namespace tse
{
class PricingTrx;
class TaxCodeReg;

class GetTaxCodeReg
{
public:
  static std::unique_ptr<GetTaxCodeReg>
  create(PricingTrx& trx, const TaxCode& taxCode, std::list<TaxCode>* pChangeFeeTaxes);

  const std::vector<TaxCodeReg*>* taxCodeReg() const
  {
    return _taxCodeReg;
  }

  DateTime ticketingDate() const
  {
    return _ticketDate;
  }

protected:
  const std::vector<TaxCodeReg*>* _taxCodeReg = nullptr;
  DateTime _ticketDate;
};

class DefaultGetTaxCodeReg : public GetTaxCodeReg
{
public:
  DefaultGetTaxCodeReg(PricingTrx& trx, const TaxCode& taxCode);
};

class CanadaGetTaxCodeReg : public GetTaxCodeReg
{
public:
  CanadaGetTaxCodeReg(PricingTrx& trx, const TaxCode& taxCode);
};

class Cat33GetTaxCodeReg : public GetTaxCodeReg
{
public:
  Cat33GetTaxCodeReg(PricingTrx& trx, const TaxCode& taxCode);
};

} // end of tse namespace
