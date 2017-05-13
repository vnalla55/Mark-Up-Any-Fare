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
#include "TaxRoundingOCApplicator.h"
#include "TaxRoundingOCRule.h"

namespace tax
{

TaxRoundingOCRule::TaxRoundingOCRule(const type::TaxRoundingUnit& taxRoundingUnit,
                                     const type::TaxRoundingDir& taxRoundingDir)
    : TaxRoundingRule(taxRoundingUnit, taxRoundingDir, type::TaxRoundingPrecision::ToUnits)
{
}

TaxRoundingOCRule::ApplicatorType
TaxRoundingOCRule::createApplicator(type::Index const& itinIndex,
                                    const Request& request,
                                    Services& services,
                                    RawPayments& rawPayments) const
{
  type::Index posIndex = request.getItinByIndex(itinIndex).pointOfSaleRefId();
  const PointOfSale& pos = request.pointsOfSale()[posIndex];
  const type::Nation& posNation = services.locService().getNation(pos.loc());

  return ApplicatorType(this,
                        services,
                        posNation,
                        rawPayments,
                        request);
}

} // end of tax namespace
