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
#include "Taxes/LegacyTaxes/TaxApplyYQ.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "Rules/RuleUtil.h"
#include "Taxes/Common/PricingTrxOps.h"
#include "Taxes/LegacyTaxes/ServiceFeeYQ.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;
using namespace tse::YQYR;

log4cxx::LoggerPtr
tse::YQYR::TaxApply::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.YQYR.TaxApply"));

namespace tse
{
namespace YQYR
{

void
TaxApply::initializeTaxItem(PricingTrx& trx,
                            tse::YQYR::ServiceFee& tax,
                            TaxResponse& taxResponse,
                            tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator)
{
  TaxItem* pTaxItem = nullptr;

  trx.dataHandle().get(pTaxItem);

  if (UNLIKELY(pTaxItem == nullptr))
  {
    LOG4CXX_WARN(_logger, "NO MEMORY AVAILABLE ***** TaxApply::initializeTaxItem *****");

    TaxDiagnostic::collectErrors(trx,
                                 serviceFeeRec1Validator,
                                 taxResponse,
                                 TaxDiagnostic::BAD_DATA_HANDLER_POINTER,
                                 Diagnostic809);

    return;
  }

  if (taxResponse.taxItemVector().empty())
    addUniqueTaxResponse(taxResponse, trx);

  pTaxItem->buildTaxItem(trx, tax, *taxResponse.farePath(), serviceFeeRec1Validator); // lint !e413
  taxResponse.taxItemVector().push_back(pTaxItem);
}
}
}

