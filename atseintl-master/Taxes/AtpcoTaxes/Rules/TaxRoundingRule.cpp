// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include "Common/MoneyUtil.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/Services.h"
#include "AtpcoTaxes/Common/SafeEnumToString.h"
#include "AtpcoTaxes/DataModel/Common/CodeIO.h"
#include "TaxRoundingApplicator.h"
#include "TaxRoundingRule.h"

namespace tax
{

TaxRoundingRule::TaxRoundingRule(const type::TaxRoundingUnit& taxRoundingUnit,
                                 const type::TaxRoundingDir& taxRoundingDir,
                                 const type::TaxRoundingPrecision& taxRoundingPrecision)
  : _taxRoundingUnit(taxRoundingUnit), _taxRoundingDir(taxRoundingDir),
    _taxRoundingPrecision(taxRoundingPrecision)
{
}

TaxRoundingRule::~TaxRoundingRule() {}

std::string
TaxRoundingRule::getDescription(Services&) const
{
  std::ostringstream str;
  str << "TAX VALUE WILL BE ROUNDED TO ";
  if (_taxRoundingUnit != type::TaxRoundingUnit::Blank)
  {
    str << "UNIT '" << _taxRoundingUnit
        << "' (VALUE: " << amountToDouble(TaxRoundingApplicator::unitValue(_taxRoundingUnit))
        << ")";
  }
  else
  {
    str << "DEFAULT UNIT OF ITS NATION";
  }

  str << ", DIRECTION ";
  if (_taxRoundingDir != type::TaxRoundingDir::Blank)
  {
    str << "'" << _taxRoundingDir << "'";
  }
  else
  {
    str << "DEFAULT FOR ITS NATION";
  }
  return str.str();
}

TaxRoundingRule::ApplicatorType
TaxRoundingRule::createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const
{
  type::Index posIndex = request.getItinByIndex(itinIndex).pointOfSaleRefId();
  const PointOfSale& pos = request.pointsOfSale()[posIndex];
  const type::Nation& posNation = services.locService().getNation(pos.loc());
  return ApplicatorType(this,
                        services,
                        posNation);
}

}
