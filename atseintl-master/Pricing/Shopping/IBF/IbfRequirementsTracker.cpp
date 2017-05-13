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

#include "Pricing/Shopping/IBF/IbfRequirementsTracker.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/IBF/LessConnectionsIsBetter.h"
#include "Pricing/Shopping/Swapper/PrioritySwapperFormatter.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

namespace tse
{
const int IbfRequirementsTracker::LESS_CONNECTION_SATISFY_PERCENT = 80;

const int IbfRequirementsTracker::HIGHEST_PLUS_PRIORITY = 60;
const int IbfRequirementsTracker::HIGHEST_PRIORITY = 50;
const int IbfRequirementsTracker::HIGH_PRIORITY = 40;
const int IbfRequirementsTracker::MEDIUM_PLUS_PRIORITY = 35;
const int IbfRequirementsTracker::MEDIUM_PRIORITY = 30;
const int IbfRequirementsTracker::LOW_PRIORITY = 20;
const int IbfRequirementsTracker::LOWEST_PRIORITY = 10;


static Logger
logger("atseintl.Pricing.IBF.IbfRequirementsTracker");

IbfRequirementsTracker::IbfRequirementsTracker(const ShoppingTrx& trx,
                                               unsigned int requestedNrOfSolutions,
                                               const std::string& requestingCarrierCode,
                                               utils::ILogger* logger)
  : _trx(trx),
    _sopUsageTracker(_trx),
    _lockingAppraiser("All direct FOS solutions kept"),
    _allSopsRepresented(_sopUsageTracker),
    _allSOPsOnSpecifiedLeg(_sopUsageTracker),
    _newAllSopsRepresented(_sopUsageTracker),
    _allDirectOptionsRepresented(utils::getNonAsoLegsCount(trx)),
    _mctOptionsAreBetter(_trx),
    _swapper(requestedNrOfSolutions),
    _logger(logger)
{
  _allOnlinesForCarrier =
      &trx.dataHandle().safe_create<AllOnlinesForCarrier>(requestingCarrierCode, trx);
  TSE_ASSERT(_allOnlinesForCarrier != nullptr);

  if(_trx.isNotExchangeTrx())
  {
    prepareAppraisersForIBF();
  }
  else
  {
    prepareAppraisersForEXC();
  }
}

void
IbfRequirementsTracker::prepareAppraisersForIBF()
{
  if(_trx.getRequest()->isAllFlightsRepresented())
  {
    _swapper.addAppraiser(&_newAllSopsRepresented, HIGHEST_PRIORITY);
    if (_trx.getRequest()->isReturnIllogicalFlights())
    {
      _swapper.addAppraiser(&_mctOptionsAreBetter, HIGH_PRIORITY);
    }
    _swapper.addAppraiser(&_allDirectOptionsRepresented, MEDIUM_PRIORITY);
  }
  else
  {
    _swapper.addAppraiser(&_lockingAppraiser, HIGHEST_PRIORITY);
    _swapper.addAppraiser(&_allSopsRepresented, HIGH_PRIORITY);
    if (_trx.getRequest()->isReturnIllogicalFlights())
    {
      _swapper.addAppraiser(&_mctOptionsAreBetter, MEDIUM_PRIORITY);
    }
  }

  _swapper.addAppraiser(_allOnlinesForCarrier, LOW_PRIORITY);
  _swapper.addAppraiser(&_earlierOptionsAreBetter, LOWEST_PRIORITY);
}

void
IbfRequirementsTracker::prepareAppraisersForEXC()
{
  _lessConnectionsIsBetter = &_trx.dataHandle().safe_create<LessConnectionsIsBetter>(_trx);

  _lessConnectionsIsBetter->setTargetCount(
              int((_swapper.getCapacity()*LESS_CONNECTION_SATISFY_PERCENT)/100));

  _swapper.addAppraiser(&_lockingAppraiser, HIGHEST_PRIORITY);
  _swapper.addAppraiser(_allOnlinesForCarrier, HIGH_PRIORITY);
  _swapper.addAppraiser(_lessConnectionsIsBetter, MEDIUM_PRIORITY);
  _swapper.addAppraiser(&_earlierOptionsAreBetter, LOW_PRIORITY);

  if (_trx.getRequest()->getContextShoppingRequestFlag())
  {
    _swapper.addAppraiser(&_allSOPsOnSpecifiedLeg, HIGHEST_PLUS_PRIORITY);
  }
  else
  {
    _swapper.addAppraiser(&_allSopsRepresented, LOWEST_PRIORITY);
  }
}

void
IbfRequirementsTracker::enableSrl(unsigned int srlValue)
{
  TSE_ASSERT(srlValue > 0);

  if (_scheduleRepeatLimit != nullptr)
    return;

  unsigned int requestedSolutionsCount =  _swapper.getCapacity();
  _scheduleRepeatLimit = &_trx.dataHandle().safe_create<ScheduleRepeatLimit>(
      srlValue, requestedSolutionsCount, _sopUsageTracker, _swapper, _logger);

  // MEDIUM_PLUS_PRIORITY > MEDIUM_PRIORITY
  // so for UAF="T" SRL is more important than "All direct options".
  // MEDIUM_PLUS_PRIORITY < HIGH_PRIORITY
  // so for UAF="F" we won't collide with ASR.
  _swapper.addAppraiser(_scheduleRepeatLimit, MEDIUM_PLUS_PRIORITY);
}


void
IbfRequirementsTracker::setRCOnlinesDesiredCount(unsigned int desired)
{
  TSE_ASSERT(desired <= _swapper.getCapacity());
  _allOnlinesForCarrier->setTargetCount(desired);
}

void
IbfRequirementsTracker::newDirectFosSolution(const utils::SopCombination& comb)
{
  _lockingAppraiser.setLocking(true);
  _swapper.addItem(comb);
  _lockingAppraiser.setLocking(false);
}

IbfRequirementsTracker::AddResponse
IbfRequirementsTracker::newQueueSolution(const utils::SopCombination& comb)
{
  return _swapper.addItem(comb);
}

std::string
IbfRequirementsTracker::swapperToString() const
{
  std::ostringstream out;
  out << _swapper;
  return out.str();
}

IbfRequirementsTracker::ImplSwapper&
IbfRequirementsTracker::getSwapper()
{
  return _swapper;
}

} // namespace tse
