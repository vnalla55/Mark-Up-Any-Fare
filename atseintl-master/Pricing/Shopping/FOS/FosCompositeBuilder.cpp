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

#include "Pricing/Shopping/FOS/FosCompositeBuilder.h"

#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag910Collector.h"
#include "Pricing/Shopping/FOS/CustomFilter.h"
#include "Pricing/Shopping/FOS/CustomValidator.h"
#include "Pricing/Shopping/FOS/DiamondValidator.h"
#include "Pricing/Shopping/FOS/DirectFamilyGroupingInserter.h"
#include "Pricing/Shopping/FOS/FosBaseGenerator.h"
#include "Pricing/Shopping/FOS/FosCommonUtil.h"
#include "Pricing/Shopping/FOS/FosFilterComposite.h"
#include "Pricing/Shopping/FOS/FosMatrixInserter.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"
#include "Pricing/Shopping/FOS/FosTaskScope.h"
#include "Pricing/Shopping/FOS/FosTypes.h"
#include "Pricing/Shopping/FOS/FosValidatorComposite.h"
#include "Pricing/Shopping/FOS/LongConxValidator.h"
#include "Pricing/Shopping/FOS/NonRestrictionFilter.h"
#include "Pricing/Shopping/FOS/NonStopFilter.h"
#include "Pricing/Shopping/FOS/NonStopValidator.h"
#include "Pricing/Shopping/FOS/OnlineValidator.h"
#include "Pricing/Shopping/FOS/RestrictionFilter.h"
#include "Pricing/Shopping/FOS/SnowmanValidator.h"
#include "Pricing/Shopping/FOS/TriangleValidator.h"

#include <map>

namespace tse
{

namespace fos
{
void
FosCompositeBuilder::buildFosFilterComposite(const FosTaskScope& task,
                                             FosFilterComposite& filterComposite)
{
  DataHandle& dataHandle = _trx.dataHandle();

  const bool isOW = _trx.legs().size() == 1;
  const bool isRestrictionExcOW =
      isOW && (_trx.flightMatrix().size() + _trx.estimateMatrix().size() == 0);
  const bool isApplicableForRestriction =
      !task.pqConditionOverride() && (!isOW || isRestrictionExcOW);

  if (isApplicableForRestriction || !task.getNumFos())
  {
    if (isApplicableForRestriction)
    {
      RestrictionFilter* filter = &dataHandle.safe_create<RestrictionFilter>(_trx, task);
      filter->addExistingFareMarkets(_trx.fareMarket());
      filterComposite.addFilter(*filter);
      if (_dc910)
        _dc910->printFilterAdded(filter->getType());
    }

    if (task.getNumCustomFos() >= 0)
    {
      CustomFilter* filter = &dataHandle.safe_create<CustomFilter>(_trx);
      filterComposite.addFilter(*filter);
      if (_dc910)
        _dc910->printFilterAdded(filter->getType());
    }

    if (task.getNumDirectFos())
    {
      NonStopFilter* filter = &dataHandle.safe_create<NonStopFilter>(_trx);
      filterComposite.addFilter(*filter);
      if (_dc910)
        _dc910->printFilterAdded(filter->getType());
    }
  }

  if (task.getNumFos())
  {
    const std::size_t numFos =
        std::max(static_cast<int32_t>(task.getNumFos()), task.getNumCustomFos());

    if (isOW && FosCommonUtil::calcNumOfValidSops(_trx) > numFos)
    {
      NonRestrictionFilter* owFilter = &dataHandle.safe_create<NonRestrictionFilter>(_trx, task);
      owFilter->setNumTSRestrictions(2u, 0);
      filterComposite.addFilter(*owFilter);
      if (_dc910)
        _dc910->printFilterAdded(owFilter->getType());
    }

    if (!isOW)
    {
      NonRestrictionFilter* filter = nullptr;

      if (!task.pqConditionOverride())
      {
        filter = &dataHandle.safe_create<NonRestrictionFilter>(_trx, task);
        filter->setNumTSRestrictions(4u, 0u);
      }
      else if (task.checkConnectingFlights())
      {
        filter = &dataHandle.safe_create<NonRestrictionFilter>(_trx, task);
        filter->setNumTSRestrictions(0, 1u);
      }

      if (filter)
      {
        filterComposite.addFilter(*filter);
        if (_dc910)
          _dc910->printFilterAdded(filter->getType());
      }
    }

    NonRestrictionFilter* weakFilter = &dataHandle.safe_create<NonRestrictionFilter>(_trx, task);
    filterComposite.addFilter(*weakFilter);
    if (_dc910)
      _dc910->printFilterAdded(weakFilter->getType());
  }
}

void
FosCompositeBuilder::buildFosValidatorComposite(const FosTaskScope& task,
                                                FosBaseGenerator& generator,
                                                FosStatistic& stats,
                                                FosValidatorComposite& validatorComposite)
{
  typedef std::map<CarrierCode, uint32_t> NumFosPerCarrier;

  DataHandle& dataHandle = _trx.dataHandle();

  if (task.getNumOnlineFos())
  {
    stats.setCounterLimit(VALIDATOR_ONLINE, task.getNumOnlineFos());
    validatorComposite.addValidator(
        dataHandle.safe_create<OnlineValidator>(_trx, generator, stats));

    const NumFosPerCarrier& numFosPerCarrier = task.getNumFosPerCarrier();
    for (const auto& elem : numFosPerCarrier)
      stats.getCarrierCounter(elem.first).limit = elem.second;

    if (_dc910)
      _dc910->printValidatorAdded(VALIDATOR_ONLINE);
  }

  if (task.getNumSnowmanFos())
  {
    stats.setCounterLimit(VALIDATOR_SNOWMAN, task.getNumSnowmanFos());
    validatorComposite.addValidator(
        dataHandle.safe_create<SnowmanValidator>(_trx, generator, stats));

    if (_dc910)
      _dc910->printValidatorAdded(VALIDATOR_SNOWMAN);
  }

  if (task.getNumDiamondFos())
  {
    stats.setCounterLimit(VALIDATOR_DIAMOND, task.getNumDiamondFos());
    validatorComposite.addValidator(
        dataHandle.safe_create<DiamondValidator>(_trx, generator, stats));

    if (_dc910)
      _dc910->printValidatorAdded(VALIDATOR_DIAMOND);
  }

  if (task.getNumTriangleFos())
  {
    stats.setCounterLimit(VALIDATOR_TRIANGLE, task.getNumTriangleFos());
    validatorComposite.addValidator(
        dataHandle.safe_create<TriangleValidator>(_trx, generator, stats));

    if (_dc910)
      _dc910->printValidatorAdded(VALIDATOR_TRIANGLE);
  }

  if (task.getNumCustomFos() >= 0)
  {
    stats.setCounterLimit(VALIDATOR_CUSTOM, static_cast<uint32_t>(task.getNumCustomFos()));
    validatorComposite.addValidator(
        dataHandle.safe_create<CustomValidator>(_trx, generator, stats));

    if (_dc910)
      _dc910->printValidatorAdded(VALIDATOR_CUSTOM);
  }

  if (task.getNumLongConxFos() >= 0)
  {
    stats.setCounterLimit(VALIDATOR_LONGCONX, static_cast<uint32_t>(task.getNumLongConxFos()));
    validatorComposite.addValidator(
        dataHandle.safe_create<LongConxValidator>(_trx, generator, stats));

    if (_dc910)
      _dc910->printValidatorAdded(VALIDATOR_LONGCONX);
  }

  if (task.getNumDirectFos())
  {
    stats.setCounterLimit(VALIDATOR_NONSTOP, task.getNumDirectFos());
    NonStopValidator* nsValidator =
        &dataHandle.safe_create<NonStopValidator>(_trx, generator, stats);
    nsValidator->setDeferredProcessing(task.isDeferredAdditionaNSProcessingEnabled());
    validatorComposite.addValidator(*nsValidator);

    const NumFosPerCarrier& numDirectFosPerCarrier = task.getNumDirectFosPerCarrier();
    for (const auto& elem : numDirectFosPerCarrier)
      stats.getDirectCarrierCounter(elem.first).limit = elem.second;

    if (_dc910)
      _dc910->printValidatorAdded(VALIDATOR_NONSTOP);
  }
}

FosMatrixInserter*
FosCompositeBuilder::createFosMatrixInserter(const FosTaskScope& task)
{
  DataHandle& dataHandle = _trx.dataHandle();
  if (task.getNumDirectFos())
    return &dataHandle.safe_create<DirectFamilyGroupingInserter>(_trx, _dc910);
  else
    return &dataHandle.safe_create<FosMatrixInserter>(_trx, _dc910);
}

} // fos
} // tse
