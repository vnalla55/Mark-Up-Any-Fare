// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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
#include "Pricing/Shopping/PQ/AltDatesStatistic.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

namespace tse
{
namespace
{
ConfigurableValue<double>
fareLevelDelta("SHOPPING_DIVERSITY", "ALT_DATE_DIVERSITY_FARE_LEVEL_DELTA");
}
const char* const AltDatesStatistic::DIAG_EMPTY_MSG = "NO SOLUTIONS HAVE BEEN GENERATED YET";
const char* const AltDatesStatistic::DIAG_EMPTY_FINAL_MSG = "NO SOLUTIONS WERE GENERATED";

Logger
AltDatesStatistic::_logger("atseintl.ShoppingPQ.AltDatesStatistic");

AltDatesStatistic::AltDatesStatistic(ShoppingTrx& trx, ItinStatistic& itinStatistic)
  : _trx(trx), _itinStatistic(itinStatistic), _fareLevelDelta(5.)
{
  TSE_ASSERT(trx.isAltDates());
  _fareLevelDelta = fareLevelDelta.getValue();
}
}
