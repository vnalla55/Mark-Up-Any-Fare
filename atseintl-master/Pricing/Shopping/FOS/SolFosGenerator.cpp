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

#include "Pricing/Shopping/FOS/SolFosGenerator.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Diagnostic/Diag910Collector.h"
#include "Pricing/Shopping/FOS/AlreadyGeneratedSolutionPredicate.h"
#include "Pricing/Shopping/FOS/CxrOverridePredicate.h"
#include "Pricing/Shopping/FOS/ForcedConnectionPredicate.h"
#include "Pricing/Shopping/FOS/FosFilterComposite.h"
#include "Pricing/Shopping/FOS/InterlineFlightPredicate.h"
#include "Pricing/Shopping/FOS/InterlineSolutionPredicate.h"
#include "Pricing/Shopping/Predicates/InterlineTicketingAgreement.h"
#include "Pricing/Shopping/Predicates/MinimumConnectTime.h"

#include <map>

namespace tse
{

namespace fos
{

namespace
{
class TracedFosObserver : public utils::IFilterObserver<SopCombination>
{
public:
  TracedFosObserver(const ShoppingTrx& trx, Diag910Collector& dc910) : _trx(trx), _dc910(dc910) {}

  void elementInvalid(const SopCombination& comb,
                      const utils::INamedPredicate<SopCombination>& failedPredicate) override
  {
    if (!failedPredicate.getName().empty())
      _dc910.printTracedFosFailedPredicate(_trx, comb, failedPredicate);
  }

private:
  const ShoppingTrx& _trx;
  Diag910Collector& _dc910;
};
}

static Logger
logger("atseintl.pricing.FOS.SolFosGenerator");

void
SolFosGenerator::initGenerators()
{
  typedef std::map<CarrierCode, utils::FosGenerator*> CxrGenMap;
  CxrGenMap cxrGenerators;

  // create interline generator
  if (!_trx.onlineSolutionsOnly())
  {
    _interlineGenerator = createFosGenerator();
    _fosGenerators.push_back(_interlineGenerator);
  }

  for (LegId legIdx = 0; legIdx < _trx.legs().size(); ++legIdx)
  {
    for (uint32_t sopIdx = 0; sopIdx < _trx.legs()[legIdx].sop().size(); ++sopIdx)
    {
      const ShoppingTrx::SchedulingOption& sop = _trx.legs()[legIdx].sop()[sopIdx];
      if (!sop.cabinClassValid())
        continue;

      // add SOP to interline generator
      if (_interlineGenerator)
        _interlineGenerator->addSop(legIdx, sopIdx);

      // add SOP to carrier generator
      CxrGenMap::iterator cxrGenerator = cxrGenerators.find(sop.governingCarrier());
      if (cxrGenerator != cxrGenerators.end())
        cxrGenerator->second->addSop(legIdx, sopIdx);
      else
      {
        utils::FosGenerator* newGenerator = createFosGenerator();
        _fosGenerators.push_back(newGenerator);
        cxrGenerators.insert(std::make_pair(sop.governingCarrier(), newGenerator));
        newGenerator->addSop(legIdx, sopIdx);
      }
    }
  }
}

void
SolFosGenerator::addPredicates()
{
  AlreadyGeneratedSolutionPredicate* alreadyGen =
      &_trx.dataHandle().safe_create<AlreadyGeneratedSolutionPredicate>(_trx);
  utils::MinimumConnectTime* mct = &_trx.dataHandle().safe_create<utils::MinimumConnectTime>(_trx);
  utils::InterlineTicketingAgreement* vita =
      &_trx.dataHandle().safe_create<utils::InterlineTicketingAgreement>(_trx);
  ForcedConnectionPredicate* forcedCxn =
      &_trx.dataHandle().safe_create<ForcedConnectionPredicate>(_trx);
  CxrOverridePredicate* cxrOverride = &_trx.dataHandle().safe_create<CxrOverridePredicate>(_trx);

  InterlineSolutionPredicate* interline = nullptr;
  InterlineFlightPredicate* interlineFlight = nullptr;
  if (_interlineGenerator)
    interline = &_trx.dataHandle().safe_create<InterlineSolutionPredicate>(_trx);
  if (_trx.interlineSolutionsOnly())
    interlineFlight = &_trx.dataHandle().safe_create<InterlineFlightPredicate>(_trx);

  TracedFosObserver* tracedFosObserver = nullptr;
  Diag910Collector* dc = getDiagCollector910();
  if (dc)
    tracedFosObserver = &_trx.dataHandle().safe_create<TracedFosObserver>(_trx, *dc);

  for (auto gen : _fosGenerators)
  {
    if (gen == _interlineGenerator)
    {
      gen->addPredicate(interline, "");
    }
    else if (_trx.interlineSolutionsOnly())
      gen->addPredicate(interlineFlight, "InterlineFlightPredicate");

    gen->addPredicate(alreadyGen, "AlreadyGeneratedSolutionPredicate");
    gen->addPredicate(mct, "MinimumConnectTime");
    gen->addPredicate(vita, "InterlineTicketingAgreement");
    gen->addPredicate(forcedCxn, "ForcedConnectionPredicate");
    gen->addPredicate(cxrOverride, "CxrOverridePredicate");

    if (tracedFosObserver)
      gen->addObserver(tracedFosObserver);
  }
}
} // fos
} // tse
