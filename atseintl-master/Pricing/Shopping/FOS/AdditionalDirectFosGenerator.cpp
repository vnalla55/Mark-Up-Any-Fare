// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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
#include "Pricing/Shopping/FOS/AdditionalDirectFosGenerator.h"

#include "Diagnostic/Diag910Collector.h"
#include "Pricing/Shopping/FiltersAndPipes/IFilterObserver.h"
#include "Pricing/Shopping/FOS/CxrOverridePredicate.h"
#include "Pricing/Shopping/FOS/CxrRestrictionsPredicate.h"
#include "Pricing/Shopping/FOS/ForcedConnectionPredicate.h"
#include "Pricing/Shopping/FOS/FosFilterComposite.h"
#include "Pricing/Shopping/FOS/InterlineFlightPredicate.h"
#include "Pricing/Shopping/FOS/OnlineFlightPredicate.h"
#include "Pricing/Shopping/FOS/OnlineSolutionPredicate.h"
#include "Pricing/Shopping/FOS/SolutionInFlightMatricesPredicate.h"
#include "Pricing/Shopping/Predicates/InterlineTicketingAgreement.h"
#include "Pricing/Shopping/Predicates/MinimumConnectTime.h"

namespace tse
{
namespace fos
{

namespace
{
class AdditionalDirectFosObserver : public utils::IFilterObserver<SopCombination>
{
public:
  AdditionalDirectFosObserver(ShoppingTrx& trx, Diag910Collector& dc910) : _trx(trx), _diag(dc910)
  {
  }

  void elementInvalid(const SopCombination& comb,
                      const utils::INamedPredicate<SopCombination>& failedPredicate) override
  {
    _diag.printFosFailedPredicate(_trx, comb, failedPredicate);
  }

private:
  const ShoppingTrx& _trx;
  Diag910Collector& _diag;
};
}

void
AdditionalDirectFosGenerator::initGenerators()
{
  utils::FosGenerator* generator = createFosGenerator();
  _fosGenerators.push_back(generator);

  for (LegId legIdx = 0; legIdx < _trx.legs().size(); ++legIdx)
  {
    if (_trx.legs()[legIdx].stopOverLegFlag())
      continue;

    for (uint32_t sopIdx = 0; sopIdx < _trx.legs()[legIdx].sop().size(); ++sopIdx)
    {
      const ShoppingTrx::SchedulingOption& sop = _trx.legs()[legIdx].sop()[sopIdx];
      if (!sop.cabinClassValid())
        continue;

      if (sop.itin()->travelSeg().size() > 1)
        continue;

      generator->addSop(legIdx, sopIdx);
    }
  }
}

void
AdditionalDirectFosGenerator::addPredicates()
{
  SolutionInFlightMatricesPredicate* alreadyGen = nullptr;
  utils::MinimumConnectTime* mct = nullptr;
  ForcedConnectionPredicate* forcedCxn = nullptr;
  CxrOverridePredicate* cxrOverride = nullptr;
  CxrRestrictionsPredicate* cxrRestrictions = nullptr;
  utils::InterlineTicketingAgreement* vita = nullptr;
  InterlineFlightPredicate* interline = nullptr;
  utils::IPredicate<SopCombination>* online = nullptr;

  alreadyGen = &_trx.dataHandle().safe_create<SolutionInFlightMatricesPredicate>(_trx);
  mct = &_trx.dataHandle().safe_create<utils::MinimumConnectTime>(_trx);
  vita = &_trx.dataHandle().safe_create<utils::InterlineTicketingAgreement>(_trx);
  cxrRestrictions = &_trx.dataHandle().safe_create<CxrRestrictionsPredicate>(_trx);

  if (_trx.excTrxType() == PricingTrx::EXC_IS_TRX)
    forcedCxn = &_trx.dataHandle().safe_create<ForcedConnectionPredicate>(_trx);

  if (_trx.getTrxType() == PricingTrx::IS_TRX && _trx.getRequest()->cxrOverride() != BLANK_CODE)
    cxrOverride = &_trx.dataHandle().safe_create<CxrOverridePredicate>(_trx);

  if (_trx.onlineSolutionsOnly())
  {
    if (_trx.noDiversity())
      online = &_trx.dataHandle().safe_create<OnlineFlightPredicate>(_trx);
    else
      online = &_trx.dataHandle().safe_create<OnlineSolutionPredicate>(_trx);
  }

  if (_trx.interlineSolutionsOnly())
    interline = &_trx.dataHandle().safe_create<InterlineFlightPredicate>(_trx);

  AdditionalDirectFosObserver* observer = nullptr;
  Diag910Collector* dc = getDiagCollector910();
  if (dc && _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "FOS" &&
      _trx.diagnostic().diagParamMapItem(Diagnostic::FOS_GENERATOR) == "PREDICATES")
  {
    observer = &_trx.dataHandle().safe_create<AdditionalDirectFosObserver>(_trx, *dc);
  }

  for (utils::FosGenerator* generator : _fosGenerators)
  {
    if (observer)
      generator->addObserver(observer);

    generator->addPredicate(alreadyGen, "SolutionInFlightMatrices");
    generator->addPredicate(mct, "MinimumConnectTime");

    if (forcedCxn)
      generator->addPredicate(forcedCxn, "ForcedConnectionPredicate");

    if (cxrOverride)
      generator->addPredicate(cxrOverride, "CxrOverridePredicate");

    if (online)
      generator->addPredicate(online, "OnlineFlightPredicate");

    if (interline)
      generator->addPredicate(interline, "InterlineFlightPredicate");

    generator->addPredicate(vita, "InterlineTicketingAgreement");
    generator->addPredicate(cxrRestrictions, "CxrRestrictionsPredicate");
  }
}

} // fos
} // tse
