//-------------------------------------------------------------------
//
//  File:        BrandingService.cpp
//  Created:     April 2013
//  Authors:
//
//  Description:
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

#include "Common/FallbackUtil.h"
#include "BrandedFares/BrandingService.h"

#include "BrandedFares/BrandingRequestResponseHandler.h"
#include "BrandedFares/MarketRequest.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
FALLBACK_DECL(fallbackBrandedServiceInterface);

BrandingService::BrandingService(PricingTrx& trx)
  : _trx(trx), _getOnlyXmlData(false), _brandingCriteria(nullptr)
{
}

bool
BrandingService::getBrandedFares()
{

  if (_trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX)
  {
    return false;
  }

  const FareDisplayRequest* fqReq = static_cast<const FareDisplayRequest*>(_trx.getRequest());
  LocCode depAirportCode = _trx.travelSeg().front()->origAirport();
  LocCode arrivalAirportCode = _trx.travelSeg().front()->destAirport();
  DateTime travelDate = _trx.travelDate();
  std::vector<PaxType*> paxType = _trx.paxType();
  std::vector<PaxTypeCode> paxTypes;
  if (fqReq && !fqReq->displayPassengerTypes().empty())
  {
    std::vector<PaxTypeCode>::const_iterator paxTypeBeg = fqReq->displayPassengerTypes().begin();
    std::vector<PaxTypeCode>::const_iterator paxTypeEnd = fqReq->displayPassengerTypes().end();
    for (; paxTypeBeg != paxTypeEnd; paxTypeBeg++)
    {
      paxTypes.push_back(*paxTypeBeg);
    }
  }
  else
  {
    std::vector<PaxType*>::const_iterator paxTypeBeg = _trx.paxType().begin();
    std::vector<PaxType*>::const_iterator paxTypeEnd = _trx.paxType().end();
    for (; paxTypeBeg != paxTypeEnd; paxTypeBeg++)
    {
      paxTypes.push_back((*paxTypeBeg)->paxType());
    }
  }
  MarketRequest::CarrierList carriers;
  std::vector<FareMarket*>::const_iterator fareMarketBeg = _trx.fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fareMarketEnd = _trx.fareMarket().end();
  for (; fareMarketBeg != fareMarketEnd; fareMarketBeg++)
  {
    carriers.push_back((*fareMarketBeg)->governingCarrier());
  }

  GlobalDirection gd = _trx.fareMarket().front()->getGlobalDirection();
  // FQ direction always OUT per BA on 1Oct24/2013
  AlphaCode direction = "OT";

  BrandingRequestResponseHandler brandingRequestResponseHandler(_trx);
  if (fallback::fallbackBrandedServiceInterface(&_trx))
    brandingRequestResponseHandler.setClientId(BR_CLIENT_FQ);
  else
  {
    brandingRequestResponseHandler.setClientId(BR_CLIENT_FQ);
    brandingRequestResponseHandler.setActionCode(BR_ACTION_FQ);
  }

  brandingRequestResponseHandler.buildMarketRequest(depAirportCode,
                                                    arrivalAirportCode,
                                                    travelDate,
                                                    paxTypes,
                                                    carriers,
                                                    _trx.fareMarket(),
                                                    gd,
                                                    direction);

  if (_getOnlyXmlData)
  {
    brandingRequestResponseHandler.getOnlyXmlData() = _getOnlyXmlData;
  }
  _brandingCriteria = brandingRequestResponseHandler.brandingCriteria();

  bool rc = brandingRequestResponseHandler.getBrandedFares();
  _xmlData = brandingRequestResponseHandler.xmlData();
  return rc;
}
}
