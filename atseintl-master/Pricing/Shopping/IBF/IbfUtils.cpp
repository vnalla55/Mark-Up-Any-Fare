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

#include "Pricing/Shopping/IBF/IbfUtils.h"

#include "Common/Logger.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FiltersAndPipes/NamedPredicateWrapper.h"
#include "Pricing/Shopping/Predicates/InterlineTicketingAgreement.h"
#include "Pricing/Shopping/Predicates/MinimumConnectTime.h"
#include "Pricing/Shopping/Predicates/SopCabinClassValid.h"
#include "Pricing/Shopping/Utils/SopDistributor.h"

namespace tse
{

using namespace utils;

namespace
{
Logger
logger("atseintl.Pricing.IBF.IbfUtils");
}

void
IbfUtils::loadSopsFromTrx(ICollector<utils::SopCandidate>& collector, const ShoppingTrx& trx)
{
  SopDistributor distributor;
  distributor.addCollector(&collector);
  distributor.distributeSops(trx);
}

void
IbfUtils::collectCabinClassValidSops(ShoppingTrx& trx,
                                     ICollector<utils::SopCandidate>& collector,
                                     utils::IFilterObserver<utils::SopCandidate>* observer)
{
  LOG4CXX_INFO(logger, "Starting collecting valid SOPs for IBF");

  CollectingFilter<SopCandidate> filter(collector);
  filter.addPredicate(utils::wrapPredicateWithName(
      &trx.dataHandle().safe_create<utils::SopCabinClassValid>(trx), "Cabin class validity", trx));

  if (observer != nullptr)
  {
    filter.addObserver(observer);
  }
  loadSopsFromTrx(filter, trx);
}

void
IbfUtils::loadGeneratorWithSops(FosPipelineGenerator& generator,
                                const std::vector<SopCandidate>& validSops,
                                ICollector<SopCandidate>* passedSopsObserver)
{
  if (passedSopsObserver != nullptr)
  {
    generator.addObserverForPassedSops(passedSopsObserver);
  }

  for (auto& validSop : validSops)
  {
    generator.collect(validSop);
  }
}

} // namespace tse
