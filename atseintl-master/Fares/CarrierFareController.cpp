//-------------------------------------------------------------------
//
//  File:        CarrierFareController.cpp
//  Created:     Apr 09, 2004
//  Authors:     Abu Islam, Mark Kasprowicz, Vadim Nikushin
//
//  Description: Carrier fare factory
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

#include "Fares/CarrierFareController.h"

#include "AddonConstruction/FareDup.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag204Collector.h"
#include "Util/Algorithm/Container.h"

namespace tse
{
namespace
{
Logger
_logger("atseintl.Pricing.CarrierFareController");

} // unnamed namespace

CarrierFareController::CarrierFareController(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
  : FareController(trx, itin, fareMarket)
{
}

bool
CarrierFareController::process(std::vector<Fare*>& constructedFares)
{
  std::vector<Fare*> publishedFares;

    if (_trx.getTrxType() == PricingTrx::IS_TRX && _fareMarket.getMultiAirportInfo())
    {
      findFaresMultiAirport(publishedFares);
    }
    else
    {
      findFares(publishedFares);
    }

  if (publishedFares.empty())
  {
    // create PaxTypeFares from constructed fares
    return createAllPTFs(constructedFares);
  }

  if (UNLIKELY(FareDisplayUtil::shortRequestForPublished(_trx)))
  {
    selectFareForShortRequest(publishedFares);
    return createAllPTFs(publishedFares);
  }
  else
  {
    // eliminate duplicates from constructed and published and create
    // PaxTypeFares
    if (constructedFares.empty())
      return createAllPTFs(publishedFares);
    else
    {
      std::vector<Fare*> resultFares;
      eliminateDuplicates(constructedFares, publishedFares, resultFares);
      return createAllPTFs(resultFares);
    }
  }
}

bool
CarrierFareController::processESV(std::vector<Fare*>& publishedFares)
{
  if (!findFaresESV(publishedFares))
  {
    return false;
  }

  return createAllPaxTypeFaresESV(publishedFares);
}

//---------------------------------------------------------------------------
// This method selects one published fare for short request from the vector
// --------------------------------------------------------------------------
void
CarrierFareController::selectFareForShortRequest(std::vector<Fare*>& publishedFares)
{
  FareDisplayOptions& fdo = *(_fdTrx->getOptions());

  LinkNumber linkNum = fdo.linkNumber(); // Q46
  SequenceNumber seqNumber = fdo.sequenceNumber(); // Q1K
  DateTime crDate = fdo.createDate(); // D12
  std::string crTime = fdo.createTime(); // D55

  CarrierCode carrierCode = fdo.carrierCode(); // B00
  FareClassCode fareClass = fdo.fareClass(); // BJ0
  TariffNumber fareTariff = fdo.fareTariff(); // Q3W
  VendorCode vendor = fdo.vendorCode(); // S37

  std::vector<Fare*>::iterator ptfItr = publishedFares.begin();
  while (ptfItr != publishedFares.end())
  {
    Fare* p = (*ptfItr);
    if (p == nullptr)
    {
      ptfItr++;
      continue;
    }

    Fare& refFare = **ptfItr;

    // Select the Fare for the short request
    if (_fdTrx->isERD())
    {
      if ((refFare.carrier() != carrierCode) || (refFare.fareClass() != fareClass) ||
          (refFare.fareTariff() != fareTariff) || (refFare.vendor() != vendor))
      {
        ptfItr = publishedFares.erase(ptfItr);
      }
      else
      {
        ptfItr++;
      }
    }
    else
    {
      if ((refFare.linkNumber() != linkNum) || (refFare.sequenceNumber() != seqNumber) ||
          (refFare.carrier() != carrierCode) || (refFare.fareClass() != fareClass) ||
          (refFare.fareTariff() != fareTariff) || (refFare.vendor() != vendor) ||
          (refFare.createDate().dateToString(DDMMMYY, "") != crDate.dateToString(DDMMMYY, "")) ||
          (refFare.createDate().timeToSimpleString() != crTime))
      {
        ptfItr = publishedFares.erase(ptfItr);
      }
      else
      {
        ptfItr++;
      }
    }
  }
}

void
CarrierFareController::findFaresMultiAirport(std::vector<Fare*>& fares)
{
  const CarrierCode governingCxr = _fareMarket.governingCarrier();
  const LocCode& boardMultiCity = _fareMarket.boardMultiCity();
  const LocCode& offMultiCity = _fareMarket.offMultiCity();

  std::vector<LocCode> origins(_fareMarket.getMultiAirportInfo()->origin());
  std::vector<LocCode> destinations(_fareMarket.getMultiAirportInfo()->destination());

  if (!alg::contains(origins, boardMultiCity))
    origins.push_back(boardMultiCity);

  if (!alg::contains(destinations, offMultiCity))
    destinations.push_back(offMultiCity);

  for (const auto& origin : origins)
    for (const auto& destination : destinations)
      findFares(governingCxr, origin, destination, fares);
}

} // namespace tse
