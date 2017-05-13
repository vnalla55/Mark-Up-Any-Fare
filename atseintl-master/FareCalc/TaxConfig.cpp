// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "FareCalc/TaxConfig.h"

namespace tse
{
namespace FareCalcTaxConfig
{
ConfigurableValue<ConfigSet<TaxCode>> flatRateTaxes("SHOPPING_OPT", "FLAT_RATE_TAXES");
ConfigurableValue<ConfigSet<TaxCode>> fareAmountTaxes("SHOPPING_OPT", "FARE_AMOUNT_BASED_TAXES");
ConfigurableValue<ConfigSet<TaxCode>> segmentBasedTaxes("SHOPPING_OPT", "SEGMENT_BASED_TAXES");
ConfigurableValue<ConfigSet<TaxCode>> taxOnTaxSpecialTaxes("SHOPPING_OPT", "TAX_ON_TAX_SPECIAL_TAXES");
}
}
