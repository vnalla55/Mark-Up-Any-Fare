//-------------------------------------------------------------------
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "RexPricing/FarePathChangeDetermination.h"

#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"

namespace tse
{

bool
FarePathChangeDetermination::changedFC(const PaxTypeFare& ptf) const
{
  return std::find(_changedExcFCs.begin(), _changedExcFCs.end(), &ptf) != _changedExcFCs.end();
}
bool
FarePathChangeDetermination::insideChangedPU(const PaxTypeFare& ptf) const
{
  return std::find(_changedExcPuFCs.begin(), _changedExcPuFCs.end(), &ptf) !=
         _changedExcPuFCs.end();
}
bool
FarePathChangeDetermination::sameInsideExtendedPU(const PaxTypeFare& ptf) const
{
  return std::find(_extendedExcPuSameFCs.begin(), _extendedExcPuSameFCs.end(), &ptf) !=
         _extendedExcPuSameFCs.end();
}

const std::string
FarePathChangeDetermination::applicationStatus(const PaxTypeFare& ptf) const
{
  std::stringstream status;

  status << std::setw(8);
  changedFC(ptf) ? status << "CHANGED" : status << " ";

  status << std::setw(7);
  insideChangedPU(ptf) ? status << "PU CHG" : status << " ";

  status << std::setw(7);
  sameInsideExtendedPU(ptf) ? status << "PU EXT" : status << " ";

  return status.str();
}

void
FarePathChangeDetermination::determineChanges(FarePath& newFP, RexPricingTrx& trx)
{
  _newFP = &newFP;
  _trx = &trx;

  cPUi puIter = _trx->exchangeItin().front()->farePath().front()->pricingUnit().begin();
  cPUi puIterEnd = _trx->exchangeItin().front()->farePath().front()->pricingUnit().end();
  for (; puIter != puIterEnd; ++puIter)
  {
    cFUi fuIter = (*puIter)->fareUsage().begin();
    cFUi fuIterEnd = (*puIter)->fareUsage().end();
    for (; fuIter != fuIterEnd; ++fuIter)
    {
      if (changed(*(**fuIter).paxTypeFare()->fareMarket()) || !foundSameOnNew(**fuIter))
      {
        _changedExcFCs.push_back((**fuIter).paxTypeFare());
        storeAllFCsOfThisPU(**puIter);
      }
    }
  }

  checkRestOfNew();

  if (_changedExcFCs.empty() && _changedExcPuFCs.empty() && _extendedExcPuSameFCs.empty())
    _newFP->ignoreReissueCharges() = true;
}

bool
FarePathChangeDetermination::changed(const FareMarket& fm) const
{
  if (_newFP->rebookClassesExists())
    return fm.isChanged();

  return fm.asBookChangeStatus() == tse::UC;
}

void
FarePathChangeDetermination::storeAllFCsOfThisPU(const PricingUnit& excPU)
{
  cFUi fuIter = excPU.fareUsage().begin();

  // possible multiple call for one pu that's why
  if (!insideChangedPU(*(**fuIter).paxTypeFare()))
    for (; fuIter != excPU.fareUsage().end(); ++fuIter)
      _changedExcPuFCs.push_back((**fuIter).paxTypeFare());
}

bool
FarePathChangeDetermination::foundSameOnNew(const FareUsage& excFC)
{
  cPUi puIter = _newFP->pricingUnit().begin();
  cPUi puIterEnd = _newFP->pricingUnit().end();
  for (; puIter != puIterEnd; ++puIter)
  {
    cFUi fuIter = (*puIter)->fareUsage().begin();
    cFUi fuIterEnd = (*puIter)->fareUsage().end();
    for (; fuIter != fuIterEnd; ++fuIter)
    {
      if (sameFareComponents(*excFC.paxTypeFare(), *(**fuIter).paxTypeFare()))
      {
        _hitNewPUs.insert(*puIter);
        _newToExcFC[*fuIter] = &excFC;
        return true;
      }
    }
  }

  return false;
}

void
FarePathChangeDetermination::checkRestOfNew()
{
  cPUi puIter = _newFP->pricingUnit().begin();
  cPUi puIterEnd = _newFP->pricingUnit().end();
  for (; puIter != puIterEnd; ++puIter)
  {
    cFUi fuIter = (*puIter)->fareUsage().begin();
    cFUi fuIterEnd = (*puIter)->fareUsage().end();
    for (; fuIter != fuIterEnd; ++fuIter)
      if (fcPreviouslyNotHitButInsideHitPu(**fuIter, **puIter))
        storeCommonPartWithExcTicket(**puIter);
  }
}

bool
FarePathChangeDetermination::fcPreviouslyNotHitButInsideHitPu(const FareUsage& newFU,
                                                              const PricingUnit& newPU) const
{
  return _newToExcFC.find(&newFU) == _newToExcFC.end() &&
         _hitNewPUs.find(&newPU) != _hitNewPUs.end();
}

void
FarePathChangeDetermination::storeCommonPartWithExcTicket(const PricingUnit& newPU)
{
  cFUi fuIter = newPU.fareUsage().begin();
  cFUi fuIterEnd = newPU.fareUsage().end();
  for (; fuIter != fuIterEnd; ++fuIter)
    if (_newToExcFC.find(*fuIter) != _newToExcFC.end())
      _extendedExcPuSameFCs.push_back(_newToExcFC[*fuIter]->paxTypeFare());
}

bool
FarePathChangeDetermination::sameFareComponents(const PaxTypeFare& newPtf,
                                                const PaxTypeFare& excPtf) const
{
  return excPtf.fareMarket()->boardMultiCity() == newPtf.fareMarket()->boardMultiCity() &&
         excPtf.fareMarket()->offMultiCity() == newPtf.fareMarket()->offMultiCity() &&
         (fabs(excPtf.fareAmount() - newPtf.fareAmount()) < EPSILON) &&
         sameTravelSegments(newPtf.fareMarket()->travelSeg(), excPtf.fareMarket()->travelSeg()) &&
         excPtf.createFareBasis(_trx) == newPtf.createFareBasis(_trx);
}

bool
FarePathChangeDetermination::sameTravelSegments(const std::vector<TravelSeg*>& newSegs,
                                                const std::vector<TravelSeg*>& excSegs) const
{
  GenericRemoveOpens<std::vector, const TravelSeg*> newWoOpens;
  newWoOpens.remove(newSegs);
  GenericRemoveOpens<std::vector, const TravelSeg*> excWoOpens;
  excWoOpens.remove(excSegs);

  if (newWoOpens.get().size() != excWoOpens.get().size())
    return false;

  return std::equal(
      newWoOpens.get().begin(), newWoOpens.get().end(), excWoOpens.get().begin(), SameSegs());
}
}
