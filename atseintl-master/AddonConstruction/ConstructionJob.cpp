//-------------------------------------------------------------------
//
//  File:        ConstructionJob.cpp
//  Created:     May 3, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description:
//
//  Copyright Sabre 2005
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

#include "AddonConstruction/ConstructionJob.h"

#include "AddonConstruction/DiagRequest.h"
#include "Common/FallbackUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/ACDiagCollector.h"
#include "Diagnostic/Diag251Collector.h"
#include "Diagnostic/Diag252Collector.h"
#include "Diagnostic/Diag253Collector.h"
#include "Diagnostic/Diag254Collector.h"
#include "Diagnostic/Diag255Collector.h"
#include "Diagnostic/Diag256Collector.h"
#include "Diagnostic/Diag257Collector.h"
#include "Diagnostic/Diag259Collector.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
FALLBACK_DECL(fallbackConstructedFareEffectiveDate)

ConstructionJob::ConstructionJob(PricingTrx& theTrx,
                                 const DateTime& aTravelDate,
                                 const DateTime& aTicketingDate,
                                 const LocCode& orig,
                                 const LocCode& boardCity,
                                 const LocCode& dest,
                                 const LocCode& offCity,
                                 const GlobalDirection globalDir,
                                 const bool isSingleOverDouble,
                                 SpecifiedFareCache* specifiedCache)
  : _trx(theTrx),
    _isHistorical(theTrx.dataHandle().isHistorical()),
    _travelDate(aTravelDate),
    _ticketingDate(aTicketingDate),
    _globalDir(globalDir),
    _singleOverDouble(isSingleOverDouble),
    _specifiedCache(specifiedCache),
    _fallbackConstructedFareEffectiveDate(fallback::fallbackConstructedFareEffectiveDate(&theTrx)),
    _response(_trx)
{
  init(orig, boardCity, dest, offCity);
}
#else
ConstructionJob::ConstructionJob(PricingTrx& theTrx,
                                 const DateTime& aTravelDate,
                                 const DateTime& aTicketingDate,
                                 const LocCode& orig,
                                 const LocCode& boardCity,
                                 const LocCode& dest,
                                 const LocCode& offCity,
                                 const bool isSingleOverDouble)
  : _trx(theTrx),
    _isHistorical(theTrx.dataHandle().isHistorical()),
    _travelDate(aTravelDate),
    _ticketingDate(aTicketingDate),
    _singleOverDouble(isSingleOverDouble),
    _response(_trx)
{
  init(orig, boardCity, dest, offCity);
}
#endif

void
ConstructionJob::init(const LocCode& orig,
                      const LocCode& boardCity,
                      const LocCode& dest,
                      const LocCode& offCity)
{
  // add-on construction allways works in Fare Fisplay mode

  _dataHandle.setIsFareDisplay(true);
  _dataHandle.setTicketDate(_ticketingDate);

  // there is a rule of thumb for any fare in ATSE:
  //             market1 < market2
  // so, first define origin and destination of add-on construction

  _reversedFareMarket = (dest < orig);
  if (UNLIKELY(_reversedFareMarket))
  {
    _origin = dest;
    _boardMultiCity = offCity;

    _destination = orig;
    _offMultiCity = boardCity;
  }
  else
  {
    _origin = orig;
    _boardMultiCity = boardCity;

    _destination = dest;
    _offMultiCity = offCity;
  }

  // define origin and destination Loc objects.

  _originLoc = _trx.dataHandle().getLoc(_origin, _travelDate);

  VALIDATE_OR_THROW(_originLoc != nullptr,
                    INVALID_INPUT,
                    "Could not get location " << _origin.c_str() << " for " << _travelDate);

  _destinationLoc = _trx.dataHandle().getLoc(_destination, _travelDate);

  VALIDATE_OR_THROW(_destinationLoc != nullptr,
                    INVALID_INPUT,
                    "Could not get location " << _destination.c_str() << " for " << _travelDate);
}

// lint -e{1554}
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
ConstructionJob::ConstructionJob(const ConstructionJob& rhs)
  : _trx(rhs._trx),
    _isHistorical(rhs._isHistorical),
    _vendorCode(rhs._vendorCode),
    _carrier(rhs._carrier),
    _travelDate(rhs._travelDate),
    _ticketingDate(rhs._ticketingDate),
    _origin(rhs._origin),
    _boardMultiCity(rhs._boardMultiCity),
    _destination(rhs._destination),
    _offMultiCity(rhs._offMultiCity),
    _globalDir(rhs._globalDir),
    _smfProcess(rhs._smfProcess),
    _reversedFareMarket(rhs._reversedFareMarket),
    _singleOverDouble(rhs._singleOverDouble),
    _originLoc(rhs._originLoc),
    _destinationLoc(rhs._destinationLoc),
    _specifiedCache(rhs._specifiedCache),
    _fallbackConstructedFareEffectiveDate(rhs._fallbackConstructedFareEffectiveDate),
    _response(_trx)
{
  // add-on construction allways works in Fare Fisplay mode

  _dataHandle.setIsFareDisplay(true);
  _dataHandle.setTicketDate(_ticketingDate);
}
#else
ConstructionJob::ConstructionJob(const ConstructionJob& rhs)
  : _trx(rhs._trx),
    _isHistorical(rhs._isHistorical),
    _vendorCode(rhs._vendorCode),
    _carrier(rhs._carrier),
    _travelDate(rhs._travelDate),
    _ticketingDate(rhs._ticketingDate),
    _origin(rhs._origin),
    _boardMultiCity(rhs._boardMultiCity),
    _destination(rhs._destination),
    _offMultiCity(rhs._offMultiCity),
    _smfProcess(rhs._smfProcess),
    _reversedFareMarket(rhs._reversedFareMarket),
    _singleOverDouble(rhs._singleOverDouble),
    _originLoc(rhs._originLoc),
    _destinationLoc(rhs._destinationLoc),
    _response(_trx)
{
  // add-on construction allways works in Fare Fisplay mode

  _dataHandle.setIsFareDisplay(true);
  _dataHandle.setTicketDate(_ticketingDate);
}
#endif

void
ConstructionJob::reclaimDiagCollector()
{
  if (UNLIKELY(_diagRequest))
  {
    Diag253Collector* dc{ nullptr };
    if (!fallback::removeDynamicCastForAddonConstruction(&trx()))
    {
      dc = this->diagnostic<Diag253Collector>();
    }
    else
    {
      dc = diag253();
    }
    if (dc)
      dc->writeCombFareClassFooter();
    _diagRequest->reclaimDiagCollector(_diagCollector);
  }
}

void
ConstructionJob::createDiagCollector()
{
  // create instances of DiagRequest and ACDiagCollector

  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();

  if (UNLIKELY((diagType == Diagnostic251) || (diagType == Diagnostic252) ||
               (diagType == Diagnostic253) || (diagType == Diagnostic254) ||
               (diagType == Diagnostic255) || (diagType == Diagnostic256) ||
               (diagType == Diagnostic257)))
  {
    _diagRequest = DiagRequest::instance(*this);
    if (_diagRequest)
    {
      _diagCollector = _diagRequest->createDiagCollector(*this);
      _forceAddonConstruction = _diagRequest->forceAddonConstruction();
    }
  }
  else if (UNLIKELY(diagType == Diagnostic259))
  {
    _diagRequest = DiagRequest::instance259(*this);
    _diagCollector = _diagRequest->createDiagCollector(*this);
    _constructionCacheFlush = true;
  }
}

// accessors

Diag251Collector*
ConstructionJob::diag251()
{
  return dynamic_cast<Diag251Collector*>(_diagCollector);
}

Diag252Collector*
ConstructionJob::diag252()
{
  return dynamic_cast<Diag252Collector*>(_diagCollector);
}

Diag253Collector*
ConstructionJob::diag253()
{
  return dynamic_cast<Diag253Collector*>(_diagCollector);
}

Diag254Collector*
ConstructionJob::diag254()
{
  return dynamic_cast<Diag254Collector*>(_diagCollector);
}

Diag255Collector*
ConstructionJob::diag255()
{
  return dynamic_cast<Diag255Collector*>(_diagCollector);
}

Diag257Collector*
ConstructionJob::diag257()
{
  return dynamic_cast<Diag257Collector*>(_diagCollector);
}

Diag259Collector*
ConstructionJob::diag259()
{
  return dynamic_cast<Diag259Collector*>(_diagCollector);
}
}
