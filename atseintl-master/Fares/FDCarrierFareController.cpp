//-------------------------------------------------------------------
//
//  File:        FDCarrierFareController.cpp
//  Description: Carrier fare factory for Fare Display
//
//
//  Copyright Sabre 2004
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

#include "Fares/FDCarrierFareController.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ErrorResponseException.h"
#include "Common/FareDisplayUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/Logger.h"
#include "Common/MultiTransportMarkets.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/PricingTrx.h"

#include <vector>

namespace tse
{
namespace
{
ConfigurableValue<bool>
getMultiAirportFares("FARESC_SVC", "GET_MULTIAIRPORT_FARES", false);

Logger
logger("atseintl.Fares.FDCarrierFareController");
}

FDCarrierFareController::FDCarrierFareController(PricingTrx& trx,
                                                 Itin& itin,
                                                 FareMarket& fareMarket)
  : CarrierFareController(trx, itin, fareMarket)
{
}

//-------------------------------------------------------------------
// process()
//-------------------------------------------------------------------
bool
FDCarrierFareController::process(std::vector<Fare*>& constructedFares)
{
  setBoardOffCity(_fareMarket, _fareMarket.travelDate());
  return CarrierFareController::process(constructedFares);
}

//-------------------------------------------------------------------
// resolveTariffInhibits()
//-------------------------------------------------------------------
bool
FDCarrierFareController::resolveTariffInhibits(const Fare& fare) const
{
  const TariffCrossRefInfo& tariffCrossRef = *fare.tariffCrossRefInfo();

  // retrieve a vector of TariffCrossRef candidates
  const Indicator tariffInhibit = _trx.dataHandle().getTariffInhibit(
      tariffCrossRef.vendor(),
      (tariffCrossRef.crossRefType() == INTERNATIONAL ? 'I' : 'D'),
      tariffCrossRef.carrier(),
      tariffCrossRef.fareTariff(),
      tariffCrossRef.ruleTariffCode());

  switch (tariffInhibit)
  {
  case NOT_INHIBIT:
    return true;
  case INHIBIT_PRICING_ONLY:
  {
    // Set "For Display Only" indicator
    return (const_cast<Fare*>(&fare)->setFareValidForDisplayOnly(true));
  }
  case INHIBIT_ALL:
    return false;
  default:
    return true;
  }
}

//-------------------------------------------------------------------
// setBoardOffCity()
//-------------------------------------------------------------------
void
FDCarrierFareController::setBoardOffCity(FareMarket& fareMarket, DateTime& travelDate)
{
  if (_fareMarket.travelSeg().empty())
    return;
  else
  {
    _fareMarket.boardMultiCity() = FareMarketUtil::getMultiCity(_fareMarket.governingCarrier(),
                                                                _fareMarket.origin()->loc(),
                                                                _fareMarket.geoTravelType(),
                                                                travelDate);
    _fareMarket.offMultiCity() = FareMarketUtil::getMultiCity(_fareMarket.governingCarrier(),
                                                              _fareMarket.destination()->loc(),
                                                              _fareMarket.geoTravelType(),
                                                              travelDate);
  }
}

//-------------------------------------------------------------------
// findFares()
//-------------------------------------------------------------------
void
FDCarrierFareController::findFares(const CarrierCode& carrier, std::vector<Fare*>& fares)
{
  if (!isProcessMultiAirport())
  {
    LOG4CXX_DEBUG(logger, "NOT PROCESSING MULTIAIRPORT DUE TO CONFIG FILE ENTRY");
    FareController::findFares(carrier, fares);
  }
  else
  {
    MultiTransportMarkets multiTransportMkts(_fareMarket.origin()->loc(),
                                             _fareMarket.destination()->loc(),
                                             _fareMarket.governingCarrier(),
                                             _fareMarket.geoTravelType(),
                                             _trx.ticketingDate(),
                                             _fareMarket.travelDate(),
                                             &_fareMarket);
    std::vector<MultiTransportMarkets::Market> markets;
    multiTransportMkts.getMarkets(markets);

    try
    {
      LOG4CXX_DEBUG(logger, " Processing MultiAirport Fare Retrieval ");
      std::vector<MultiTransportMarkets::Market>::const_iterator iter = markets.begin();
      std::vector<MultiTransportMarkets::Market>::const_iterator iterEnd = markets.end();
      for (; iter != iterEnd; ++iter)
      {
        bool processMkt = true;

        if (_fdTrx && _fdTrx->marketCarrierMap())
        {
          // Check if this carrier is the set associated with this market
          // ** This map was populated earlier when the unique set of
          //    carriers was determined. It is used to avoid looking for
          //    fares for this market in the db if we know the carrier
          //    does not publish any fare.
          std::map<MultiTransportMarkets::Market, std::set<CarrierCode>>*& mktCxrMap =
              _fdTrx->marketCarrierMap();

          if (!mktCxrMap->empty())
          {
            MultiTransportMarkets::Market mkt = std::make_pair((*iter).first, (*iter).second);

            std::map<MultiTransportMarkets::Market, std::set<CarrierCode>>::const_iterator itr(
                mktCxrMap->end());
            itr = mktCxrMap->find(mkt);

            if (itr != mktCxrMap->end())
            {
              std::set<CarrierCode>::const_iterator iter(itr->second.end());
              iter = itr->second.find(carrier);

              if (iter == itr->second.end())
              {
                processMkt = false;
              }
            }
          }
        }

        if (processMkt)
        {
          LOG4CXX_DEBUG(logger,
                        "getting FARES for " << (*iter).first << " -" << (*iter).second << " -- "
                                             << carrier);

          FareController::findFares(carrier, (*iter).first, (*iter).second, fares);
        }
      }
    }
    catch (ErrorResponseException& ex)
    {
      throw;
    }
  }
}

//-------------------------------------------------------------------
// findCityPairFares()
//-------------------------------------------------------------------
const std::vector<const FareInfo*>&
FDCarrierFareController::findCityPairFares(const LocCode& origin,
                                           const LocCode& destination,
                                           const CarrierCode& carrier)
{
  // retrieve published fareInfo records for governing carrier

  FareDisplayRequest* request = dynamic_cast<FareDisplayRequest*>(_trx.getRequest());

  if (request != nullptr && request->dateRangeLower().isValid() &&
      request->dateRangeUpper().isValid())
  {
    return _trx.dataHandle().getFaresByMarketCxr(
        origin, destination, carrier, request->dateRangeLower(), request->dateRangeUpper());
  }
  else
  {
    return _trx.dataHandle().getFaresByMarketCxr(origin, destination, carrier, _travelDate);
  }
}

bool
FDCarrierFareController::isProcessMultiAirport() const
{
  if (_fdTrx->isShopperRequest())
  {
    return getMultiAirportFares.getValue();
  }
  return true;
}

bool
FDCarrierFareController::isInternationalFare(const Fare& fare) const
{
  if (_fareMarket.boardMultiCity() == _fareMarket.offMultiCity())
  {
    return true; // TariffCrossRef treats this Market as an International Market( ROUND THE WORLD
    // FARE)
  }
  else
    return FareController::isInternationalFare(fare);
}
} // namespace tse
