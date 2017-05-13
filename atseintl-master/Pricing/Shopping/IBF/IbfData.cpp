//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Pricing/Shopping/IBF/IbfData.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/IBF/V2IbfManager.h"
#include "Pricing/Shopping/Utils/DiagLogger.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

#include <string>

namespace tse
{
namespace
{
Logger
logger("atseintl.Pricing.Shopping.IBF.IbfData");
ConfigurableValue<uint64_t>
ibfIsQueueIterationsLimit("SHOPPING_OPT", "IBF_IS_QUEUE_ITERATIONS_LIMIT", 1000);
}

IbfData*
IbfData::createInitialIbfDataV2(ShoppingTrx& trx)
{
  const PricingOptions* options = trx.getOptions();
  TSE_ASSERT(nullptr != options);
  const int requestedNbrOfSolutions = options->getRequestedNumberOfSolutions();
  const std::string rcCode = utils::getRequestingCarrierCodeForTrx(trx);

  utils::DiagLogger* logger = new utils::DiagLogger(trx, Diagnostic930);
  logger->setName("V2 IBF MANAGER");
  V2IbfManager* manager = &trx.dataHandle().safe_create<V2IbfManager>(
      trx, requestedNbrOfSolutions, rcCode, ibfIsQueueIterationsLimit.getValue(), logger);

  return &trx.dataHandle().safe_create<IbfData>(manager);
}

} // namespace tse
