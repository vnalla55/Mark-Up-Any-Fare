// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "TestServer/Facades/FallbackServiceServer.h"

namespace tse
{
ATPCO_FALLBACK_DEF(ATPCO_TAX_RevokeDuplicatedOC)
ATPCO_FALLBACK_DEF(ATPCO_TAX_AcceptOCTagPrePaid)
ATPCO_FALLBACK_DEF(ATPCO_TAX_OcCurrencyConversionFix)
ATPCO_FALLBACK_DEF(ATPCO_TAX_useIATARulesRoundingFix)
ATPCO_FALLBACK_DEF(fallbackAtpcoTaxTotalRounding)
ATPCO_FALLBACK_DEF(monetaryDiscountFlatTaxesApplication)
ATPCO_FALLBACK_DEF(markupAnyFareOptimization)
}
