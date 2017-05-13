//----------------------------------------------------------------------------
//  File:           RoundTripFareSelection.cpp
//  Created:        10/12/04
//  Authors:        Quan Ta
//
//  Description:    A class to capture the round trip fare selection process.
//
//
//  Updates:
//
//  Copyright  Sabre 2004
//
//          The copyright to the computer program(s) herein is the
//          property of Sabre.
//
//          The program(s) may be used and/or copied only with the
//          written permission of Sabre or in accordance with the
//          terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//----------------------------------------------------------------------------

#include "MinFares/RoundTripFareSelection.h"

#include "Common/TrxUtil.h"
#include "Common/Logger.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/DiagCollector.h"
#include "MinFares/MinFareLogic.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "MinFares/MinFareSpecialFareSelection.h"
#include "MinFares/MinimumFare.h"

namespace tse
{
static Logger
logger("atseintl.MinFares.RoundTripFareSelection");

RoundTripFareSelection::RoundTripFareSelection(MinimumFareModule module,
                                               DiagCollector* diag,
                                               PricingTrx& trx,
                                               const FarePath& farePath,
                                               const PricingUnit& pu,
                                               const std::vector<TravelSeg*>& obTvlSegs,
                                               const std::vector<TravelSeg*>& ibTvlSegs,
                                               CabinType cabin,
                                               const PaxType* paxType,
                                               const DateTime& travelDate,
                                               const PaxTypeFare* obThruFare,
                                               const PaxTypeFare* ibThruFare,
                                               const MinFareAppl* minFareAppl,
                                               const MinFareDefaultLogic* minFareDefLogic,
                                               const RepricingTrx* obRepricingTrx,
                                               const RepricingTrx* ibRepricingTrx,
                                               FMDirection selectFareForDirection,
                                               FMDirection selectfareUsageDirection)
  : _module(module),
    _diag(diag),
    _trx(trx),
    _farePath(farePath),
    _pu(pu),
    _obTvlSegs(obTvlSegs),
    _ibTvlSegs(ibTvlSegs),
    _lowestCabin(cabin),
    _paxType(paxType),
    _travelDate(travelDate),
    _obThruFare(obThruFare),
    _ibThruFare(ibThruFare),
    _minFareAppl(minFareAppl),
    _minFareDefLogic(minFareDefLogic),
    _obRepricingTrx(obRepricingTrx),
    _ibRepricingTrx(ibRepricingTrx),
    _selectFareForDirection(selectFareForDirection),
    _selectfareUsageDirection(selectfareUsageDirection)
{
  if (obTvlSegs.size() > 0 && ibTvlSegs.size() > 0 && paxType)
  {
    try
    {
      _obGi = MinFareLogic::getGlobalDirection(_trx, _obTvlSegs, _travelDate);
      _ibGi = MinFareLogic::getGlobalDirection(_trx, _ibTvlSegs, _travelDate);
      LOG4CXX_DEBUG(logger, "OB GI: " << _obGi << ", IB GI: " << _ibGi);

      processRtFareSelection();
      reuseThruFare();
      _status = true;
    }
    catch (const ErrorResponseException& ex)
    {
      LOG4CXX_ERROR(logger, "ErrorResponseException: (" << ex.code() << ") " << ex.what());
      throw;
    }
    catch (const std::exception& ex)
    {
      LOG4CXX_ERROR(logger, "std::exception: " << ex.what());
      throw;
    }
    catch (...)
    {
      LOG4CXX_ERROR(logger, "Unknown Exception: ");
      throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION,
                                   "RoundTripFareSelection - Caught Unknown Exception");
    }
  }
} // lint !e1541

RoundTripFareSelection::~RoundTripFareSelection()
{
  // clean up - make sure we are not holding any ref. counted object.
  _roundTripFare.clear();
}

void
RoundTripFareSelection::reuseThruFare()
{
  if (_module == CTM && _pu.fareUsage().size() > 1)
  {
    const PaxTypeFare* reuseThruFare = nullptr;
    const PaxTypeFare* puObFare = _pu.fareUsage().front()->paxTypeFare();
    if (puObFare != nullptr)
    {
      const std::vector<TravelSeg*>& puObTvlSegs = puObFare->fareMarket()->travelSeg();
      if (_obTvlSegs.front() == puObTvlSegs.front() && _obTvlSegs.back() == puObTvlSegs.back())
        reuseThruFare = puObFare;
    }

    const PaxTypeFare* puIbFare = _pu.fareUsage().back()->paxTypeFare();
    if (puIbFare != nullptr)
    {
      const std::vector<TravelSeg*>& puIbTvlSegs = puIbFare->fareMarket()->travelSeg();
      if (_ibTvlSegs.front() == puIbTvlSegs.front() && _ibTvlSegs.back() == puIbTvlSegs.back())
      {
        if (reuseThruFare == nullptr ||
            (reuseThruFare->nucFareAmount() - puIbFare->nucFareAmount()) > EPSILON)
          reuseThruFare = puIbFare;
      }
    }

    if (reuseThruFare != nullptr && !isThruFareValidForReUse(*reuseThruFare))
      return;

    if (reuseThruFare)
    {
      bool thruFareUsed = false;
      if (_constructedFare) // Uses constructed fare
      {
        if ((_constFareAmt - reuseThruFare->nucFareAmount()) > EPSILON)
        {
          _constructedFare = false;
          _constructionPoint = nullptr;
          thruFareUsed = true;
        }
      }
      else
      {
        if (_roundTripFare.empty() || // No fare selected
            (((_roundTripFare.front()->nucFareAmount() - reuseThruFare->nucFareAmount()) >
              EPSILON) &&
             ((_roundTripFare.back()->nucFareAmount() - reuseThruFare->nucFareAmount()) > EPSILON)))
        {
          thruFareUsed = true;
        }
      }

      if (thruFareUsed)
      {
        _roundTripFare.clear();
        _roundTripFare.push_back(reuseThruFare);
        _roundTripFare.push_back(reuseThruFare);

        if (_diag)
          MinimumFare::printFareInfo(*reuseThruFare, *_diag, _module, "  ");
      }
    }
  }
}

bool
RoundTripFareSelection::isThruFareValidForReUse(const PaxTypeFare& ptf)
{
  std::vector<PricingUnit*> pricingUnits;
  pricingUnits.push_back(const_cast<PricingUnit*>(&_pu));

  if (!ptf.isNormal())
  {
    MinFareSpecialFareSelection minFareFareSelection(_module,
                                                     MinFareFareSelection::HALF_ROUND_TRIP,
                                                     MinFareFareSelection::OUTBOUND,
                                                     ptf.fcaFareType(),
                                                     _trx,
                                                     *_farePath.itin(),
                                                     _obTvlSegs,
                                                     pricingUnits,
                                                     _paxType,
                                                     _travelDate,
                                                     &_farePath,
                                                     _obThruFare,
                                                     _minFareAppl,
                                                     _minFareDefLogic,
                                                     _obRepricingTrx);

    return minFareFareSelection.passRuleLevelExclusion(ptf);
  }
  else
  {
    MinFareNormalFareSelection minFareFareSelection(_module,
                                                    MinFareFareSelection::HALF_ROUND_TRIP,
                                                    MinFareFareSelection::OUTBOUND,
                                                    _lowestCabin,
                                                    _trx,
                                                    *_farePath.itin(),
                                                    _obTvlSegs,
                                                    pricingUnits,
                                                    _paxType,
                                                    _travelDate,
                                                    &_farePath,
                                                    _obThruFare,
                                                    _minFareAppl,
                                                    _minFareDefLogic,
                                                    _obRepricingTrx);

    return minFareFareSelection.passRuleLevelExclusion(ptf);
  }
}

void
RoundTripFareSelection::processRtFareSelection()
{
  std::vector<PricingUnit*> pricingUnits;
  pricingUnits.push_back(const_cast<PricingUnit*>(&_pu));

  const MinFareAppl* minFareAppl = nullptr;
  const MinFareDefaultLogic* minFareDefLogic = nullptr;

  if (_selectfareUsageDirection == FMDirection::UNKNOWN ||
      _selectfareUsageDirection == FMDirection::OUTBOUND)
  {
    minFareAppl = _minFareAppl;
    minFareDefLogic = _minFareDefLogic;
  }

  if (_obThruFare && !_obThruFare->isNormal())
  {

    MinFareSpecialFareSelection obFareSel(_module,
                                          MinFareFareSelection::HALF_ROUND_TRIP,
                                          MinFareFareSelection::OUTBOUND,
                                          _obThruFare->fcaFareType(),
                                          _trx,
                                          *_farePath.itin(),
                                          _obTvlSegs,
                                          pricingUnits,
                                          _paxType,
                                          _travelDate,
                                          &_farePath,
                                          _obThruFare,
                                          minFareAppl,
                                          minFareDefLogic,
                                          _obRepricingTrx);

    doSelectFares(obFareSel);
  }
  else
  {
    MinFareNormalFareSelection obFareSel(_module,
                                         MinFareFareSelection::HALF_ROUND_TRIP,
                                         MinFareFareSelection::OUTBOUND,
                                         _lowestCabin,
                                         _trx,
                                         *_farePath.itin(),
                                         _obTvlSegs,
                                         pricingUnits,
                                         _paxType,
                                         _travelDate,
                                         &_farePath,
                                         _obThruFare,
                                         _minFareAppl,
                                         _minFareDefLogic,
                                         _obRepricingTrx);

    doSelectFares(obFareSel);
  }
}

template <class ObFareSelector>
void
RoundTripFareSelection::doSelectFares(ObFareSelector& obFareSel)
{
  std::vector<PricingUnit*> pricingUnits;
  pricingUnits.push_back(const_cast<PricingUnit*>(&_pu));

  const MinFareAppl* minFareAppl = nullptr;
  const MinFareDefaultLogic* minFareDefLogic = nullptr;

  if (_selectfareUsageDirection == FMDirection::UNKNOWN ||
      _selectfareUsageDirection == FMDirection::INBOUND)
  {
    minFareAppl = _minFareAppl;
    minFareDefLogic = _minFareDefLogic;
  }

  if (_ibThruFare && !_ibThruFare->isNormal())
  {
    MinFareSpecialFareSelection ibFareSel(_module,
                                          MinFareFareSelection::HALF_ROUND_TRIP,
                                          MinFareFareSelection::INBOUND,
                                          _ibThruFare->fcaFareType(),
                                          _trx,
                                          *_farePath.itin(),
                                          _ibTvlSegs,
                                          pricingUnits,
                                          _paxType,
                                          _travelDate,
                                          &_farePath,
                                          _ibThruFare,
                                          minFareAppl,
                                          minFareDefLogic,
                                          _ibRepricingTrx);

    doSelectFares(obFareSel, ibFareSel);
  }
  else
  {
    MinFareNormalFareSelection ibFareSel(_module,
                                         MinFareFareSelection::HALF_ROUND_TRIP,
                                         MinFareFareSelection::INBOUND,
                                         _lowestCabin,
                                         _trx,
                                         *_farePath.itin(),
                                         _ibTvlSegs,
                                         pricingUnits,
                                         _paxType,
                                         _travelDate,
                                         &_farePath,
                                         _ibThruFare,
                                         _minFareAppl,
                                         _minFareDefLogic,
                                         _ibRepricingTrx);

    doSelectFares(obFareSel, ibFareSel);
  }
}

template <class ObFareSelector, class IbFareSelector>
void
RoundTripFareSelection::selectFareSameGI(ObFareSelector& obFareSel, IbFareSelector& ibFareSel)
{
  PaxTypeStatus paxTypeStatus = MinFareLogic::paxTypeStatus(_pu);

  for (;; paxTypeStatus = PaxTypeUtil::nextPaxTypeStatus(paxTypeStatus))
  {
    selectFareSameGI(obFareSel, ibFareSel, paxTypeStatus);

    if (_roundTripFare.size() > 0 || paxTypeStatus == PAX_TYPE_STATUS_UNKNOWN ||
        paxTypeStatus == PAX_TYPE_STATUS_ADULT)
    {
      break;
    }
  }
}

template <class ObFareSelector, class IbFareSelector>
void
RoundTripFareSelection::selectFareSameGI(ObFareSelector& obFareSel,
                                         IbFareSelector& ibFareSel,
                                         PaxTypeStatus paxTypeStatus)
{
  LOG4CXX_DEBUG(logger, "selectFareSameGI");

  _roundTripFare.clear();

  // Regular fare selection:
  //
  if (!_obFare && (_selectFareForDirection != FMDirection::INBOUND))
    _obFare = obFareSel.selectFare(paxTypeStatus, true);
  if (!_ibFare && (_selectFareForDirection != FMDirection::OUTBOUND))
    _ibFare = ibFareSel.selectFare(paxTypeStatus, true);

  LOG4CXX_DEBUG(logger, "Outbound Fare: " << _obFare << ", Inbound Fare: " << _ibFare);
  if (_diag && _diag->isActive())
  {
    if (_obFare)
      MinimumFare::printFareInfo(*_obFare, *_diag, _module, "  ");
    else
    {
      if (_selectFareForDirection != FMDirection::INBOUND)
        MinimumFare::printExceptionInfo(
            *_diag, _obTvlSegs, MinFareFareSelection::OUTBOUND, "NO FARE FOUND\n");
    }
    if (_ibFare)
      MinimumFare::printFareInfo(*_ibFare, *_diag, _module, "  ");
    else
    {
      if (_selectFareForDirection != FMDirection::OUTBOUND)
        MinimumFare::printExceptionInfo(
            *_diag, _ibTvlSegs, MinFareFareSelection::INBOUND, "NO FARE FOUND\n");
    }
  }

  if (_obFare && _ibFare)
  {
    _roundTripFare.push_back(_obFare);
    _roundTripFare.push_back(_ibFare);
  }
  else if (_obFare)
  {
    _roundTripFare.push_back(_obFare);
  }
  else if (_ibFare)
  {
    _roundTripFare.push_back(_ibFare);
  }
}

template <class ObFareSelector, class IbFareSelector>
void
RoundTripFareSelection::selectFareDiffGI(ObFareSelector& obFareSel, IbFareSelector& ibFareSel)
{
  PaxTypeStatus paxTypeStatus = MinFareLogic::paxTypeStatus(_pu);

  for (;; paxTypeStatus = PaxTypeUtil::nextPaxTypeStatus(paxTypeStatus))
  {
    selectFareDiffGI(obFareSel, ibFareSel, paxTypeStatus);

    if (_roundTripFare.size() > 0 || paxTypeStatus == PAX_TYPE_STATUS_UNKNOWN ||
        paxTypeStatus == PAX_TYPE_STATUS_ADULT)
    {
      break;
    }
  }
}

template <class ObFareSelector, class IbFareSelector>
void
RoundTripFareSelection::selectFareDiffGI(ObFareSelector& obFareSel,
                                         IbFareSelector& ibFareSel,
                                         PaxTypeStatus paxTypeStatus)
{
  LOG4CXX_DEBUG(logger, "selectFareDiffGI");

  _roundTripFare.clear();

  if (!_obFare && _obGi != GlobalDirection::XX)
  {
    _obFare = obFareSel.selectFare(paxTypeStatus, true);
  }

  if (!_ibFare && _ibGi != GlobalDirection::XX)
  {
    _ibFare = ibFareSel.selectFare(paxTypeStatus, true);
  }

  LOG4CXX_DEBUG(logger, "OB Fare: " << _obFare << ", IB Fare: " << _ibFare);
  if (_diag && _diag->isActive())
  {
    if (_obFare)
      MinimumFare::printFareInfo(*_obFare, *_diag, _module, "  ");
    else
    {
      MinimumFare::printExceptionInfo(
          *_diag, _obTvlSegs, MinFareFareSelection::OUTBOUND, "NO FARE FOUND\n");
      if (_obGi == GlobalDirection::XX)
      {
        MinimumFare::printExceptionInfo(*_diag,
                                        _obTvlSegs,
                                        MinFareFareSelection::OUTBOUND,
                                        "INVALID GI - NO FARE CONSTRUCTION\n");
      }
    }
    if (_ibFare)
      MinimumFare::printFareInfo(*_ibFare, *_diag, _module, "  ");
    else
    {
      MinimumFare::printExceptionInfo(
          *_diag, _obTvlSegs, MinFareFareSelection::INBOUND, "NO FARE FOUND\n");
      if (_ibGi == GlobalDirection::XX)
      {
        MinimumFare::printExceptionInfo(*_diag,
                                        _obTvlSegs,
                                        MinFareFareSelection::INBOUND,
                                        "INVALID GI - NO FARE CONSTRUCTION\n");
      }
    }
  }

  if (_obFare && _ibFare)
  {
    _roundTripFare.push_back(_obFare);
    _roundTripFare.push_back(_ibFare);
    return;
  }
  else
  {
    if (_obFare)
    {

      _roundTripFare.push_back(_obFare);
    }
    else if (_ibFare)
    {

      _roundTripFare.push_back(_ibFare);
    }
  }
}

template <class ObFareSelector, class IbFareSelector>
void
RoundTripFareSelection::doSelectFares(ObFareSelector& obFareSel, IbFareSelector& ibFareSel)
{
  if ((_obGi == _ibGi) && (_obGi != GlobalDirection::XX))
  {
    selectFareSameGI(obFareSel, ibFareSel);
  }
  else
  {
    selectFareDiffGI(obFareSel, ibFareSel);
  }
}
}
