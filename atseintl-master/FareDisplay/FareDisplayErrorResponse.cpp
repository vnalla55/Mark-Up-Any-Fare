//-------------------------------------------------------------------
//
//  File: FareDisplayErrorResponse.cpp
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/FareDisplayErrorResponse.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplayErrorResponse");

FareDisplayErrorResponse::FareDisplayErrorResponse(FareDisplayTrx& trx)
  : _trx(trx), _request(*trx.getRequest()), _fareMarket(*trx.fareMarket().front())

{
  _trx.errorResponse().clear();
}

FareDisplayErrorResponse::~FareDisplayErrorResponse() {}

void
FareDisplayErrorResponse::process()
{
  LOG4CXX_INFO(logger, "Entering FareDisplayErrorResponse::process() ");

  if (_request.globalDirection() != GlobalDirection::ZZ)
  {
    getGlobalDirErrorResponse(_fareMarket);
  }

  else if (_trx.allPaxTypeFare().empty())
  {
    return;
  }

  else if (_trx.getOptions()->isPrivateFares())
  {
    if (_trx.getOptions()->isAllCarriers())
    {
      _trx.errorResponse() << "NO QUOTABLE FARES FOUND" << std::endl;
    }
    else
    {
      CarrierCode carrier = _trx.requestedCarrier();

      _trx.errorResponse() << "*** NO APPLICABLE " << carrier << " PRIVATE TYPE FARES FOUND "
                           << _fareMarket.origin()->loc() << "-" << _fareMarket.destination()->loc()
                           << " ***" << std::endl;

      _trx.errorResponse() << "NO FARES FOR CARRIER ";

      std::set<CarrierCode>& preferredCarrier = _trx.preferredCarriers();
      uint16_t preferredCarriersSize = preferredCarrier.size();
      uint16_t i = 0;

      std::set<CarrierCode>::const_iterator cxrSetItr = _trx.preferredCarriers().begin();
      std::set<CarrierCode>::const_iterator cxrSetEnd = _trx.preferredCarriers().end();

      for (; cxrSetItr != cxrSetEnd; cxrSetItr++, i++)
      {
        _trx.errorResponse() << *cxrSetItr;
        if ((i + 1) < preferredCarriersSize)
        {
          _trx.errorResponse() << "/";
        }
      }

      /*for (uint16_t i = 0; i < preferredCarriersSize; ++i)
      {
          _trx.errorResponse() << preferredCarrier[i].c_str();
          if ((i + 1) < preferredCarriersSize)
          {
              _trx.errorResponse() << "/";
          }
      }*/

      _trx.errorResponse() << std::endl;
    }
  }
  LOG4CXX_INFO(logger, "Leaving FareDisplayErrorResponse::process() ");
}

//------------------------------------------------------
// FareDisplayErrorResponse::getGlobalDirErrorResponse()
//------------------------------------------------------

void
FareDisplayErrorResponse::getGlobalDirErrorResponse(const FareMarket& fm)
{
  const GlobalDirection& requestedGD(_request.globalDirection());

  if (requestedGD != GlobalDirection::ZZ)
  {
    const std::vector<const FareInfo*> fares = _trx.dataHandle().getFaresByMarketCxr(
        fm.origin()->loc(), fm.destination()->loc(), fm.governingCarrier(), _trx.travelDate());
    if (fares.empty())
    {

      _trx.dataHandle().getFaresByMarketCxr(
          fm.boardMultiCity(), fm.offMultiCity(), fm.governingCarrier(), _trx.travelDate());
    }

    std::set<GlobalDirection> validGlobals;
    std::vector<const FareInfo*>::const_iterator i(fares.begin());
    for (; i != fares.end(); ++i)
    {
      if ((*i)->globalDirection() != GlobalDirection::ZZ)
        validGlobals.insert((*i)->globalDirection());
    }

    if (!validGlobals.empty())
    {
      appendMessage(_trx.errorResponse(), validGlobals, requestedGD);
    }
  }
}

//------------------------------------------------------
// FareDisplayErrorResponse::appendMessage()
//------------------------------------------------------

void
FareDisplayErrorResponse::appendMessage(std::ostringstream& response,
                                        std::set<GlobalDirection>& validGlobals,
                                        const GlobalDirection& requestedGD)
{
  std::set<GlobalDirection>::iterator i(validGlobals.begin()), j(validGlobals.end()),
      end(validGlobals.end());
  j = find(i, end, requestedGD);

  if (j == end)
  {
    std::string gd;
    globalDirectionToStr(gd, requestedGD);
    response << " GLOBAL INDICATOR " << gd << " NOT AVAILABLE - "
             << "TRY";
    for (; i != end; ++i)
    {
      std::string availableGD;
      globalDirectionToStr(availableGD, *i);
      response << " " << availableGD;
    }
  }
  else if (_trx.allPaxTypeFare().empty() && !_trx.isRD() && _trx.isShopperRequest())
  {
    _trx.errResponse();
  }
}
}
