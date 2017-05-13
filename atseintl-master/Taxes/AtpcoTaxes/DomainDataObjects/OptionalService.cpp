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
//#include "DomainDataObjects/FallbackService.h"
#include "DomainDataObjects/OptionalService.h"
#include "Rules/BusinessRule.h"

#include <exception>

//namespace tse
//{
//ATPCO_FALLBACK_DECL(ATPCO_TAX_AcceptOCTagP)
//}

namespace tax
{

OptionalService::OptionalService(void)
  : _index(0),
    _amount(0),
    _taxAmount(),
    _type(type::OptionalServiceTag::Blank),
    _taxPointBegin(nullptr),
    _taxPointEnd(nullptr),
    _taxPointLoc2(nullptr),
    _taxPointLoc3(nullptr),
    _failedRule(nullptr),
    _taxInclInd(false),
    _isDuplicated(false),
    _quantity(1)
{
}

OptionalService::~OptionalService(void)
{
}

void
OptionalService::setTaxPointBegin(const Geo& begin)
{
  if (_taxPointBegin)
  {
    throw std::logic_error("TaxPointBegin in Optional Service should be set only once!");
  }
  _taxPointBegin = &begin;
}

void
OptionalService::setTaxPointEnd(const Geo& end)
{
  if (_taxPointEnd)
  {
    throw std::logic_error("TaxPointEnd in Optional Service should be set only once!");
  }
  _taxPointEnd = &end;
}

void
OptionalService::setTaxPointLoc2(const Geo& geo)
{
  _taxPointLoc2 = &geo;
}

void
OptionalService::setTaxPointLoc3(const Geo& geo)
{
  _taxPointLoc3 = &geo;
}

const Geo&
OptionalService::getTaxPointBegin() const
{
  if (!_taxPointBegin)
  {
    throw std::logic_error("TaxPointBegin not set in Optional Service!");
  }
  return *_taxPointBegin;
}

const Geo&
OptionalService::getTaxPointEnd() const
{
  if (!_taxPointEnd)
  {
    throw std::logic_error("TaxPointEnd not set in Optional Service!");
  }
  return *_taxPointEnd;
}

const Geo&
OptionalService::getTaxPointLoc2() const
{
  if (!_taxPointLoc2)
  {
    throw std::logic_error("TaxPointLoc2 not set in Optional Service!");
  }
  return *_taxPointLoc2;
}

std::string
OptionalService::getFailureReason(Services& services) const
{
  if (_failedRule)
    return _failedRule->getDescription(services);
  else if(_isDuplicated)
    return "DUPLICATED";
  else
    return "";
}

std::ostream&
OptionalService::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */)
    const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "OPTIONALSERVICEAMOUNT: " << _amount << "\n";

  return out;
}

void
OptionalService::includeTax(const TaxAndRounding& taxAndRounding,
                            const TaxRoundingInfoService& taxRoundingInfoService)
{
  if (!_taxInclInd)
    return;

  if (!_taxInclIndProcessor)
    _taxInclIndProcessor = TaxInclIndProcessor(_amount);

  _taxInclIndProcessor->includeTax(taxAndRounding);
  _amount = _taxInclIndProcessor->getAmount(taxRoundingInfoService);
  _taxAmount = _taxInclIndProcessor->getTax(_taxAndRounding, taxRoundingInfoService);
}

} // namespace tax
