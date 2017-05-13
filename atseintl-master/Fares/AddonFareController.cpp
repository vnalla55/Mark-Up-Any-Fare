//-------------------------------------------------------------------
//
//  File:        AddonFareController.cpp
//  Created:     Apr 09, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Add-on construction factory
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

#include "Fares/AddonFareController.h"

#include "AddonConstruction/AddonConstructionOrchestrator.h"
#include "AddonConstruction/ConstructedFareInfoResponse.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"

namespace tse
{
static Logger
logger("atseintl.Fares.AddonFareController");

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

AddonFareController::AddonFareController(PricingTrx& trx,
                                         Itin& itin,
                                         FareMarket& fareMarket,
                                         SpecifiedFareCache* specFareCache)
: FareController(trx, itin, fareMarket), _specFareCache(specFareCache)
{ }

#else

AddonFareController::AddonFareController(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
  : FareController(trx, itin, fareMarket)
{
}

#endif

bool
AddonFareController::process(std::vector<Fare*>& cxrFares, std::vector<Fare*>& yyFares)
{
  LOG4CXX_INFO(logger, "Entered AddonFareController::process()");

  TSELatencyData metrics(_trx, "FCO AOFC PROCESS");

  // define add-on construction preferences for governing carrier

  const bool singleOverDouble =
      (_fareMarket.governingCarrierPref()->applysingleaddonconstr() == 'Y');

  // run construction process

  const LocCode& origin = _fareMarket.origin()->loc();

  ConstructedFareInfoResponse response(_trx);

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  const bool ok = AddonConstructionOrchestrator::process(
      _trx,
      getVendorForCat31(),
      _fareMarket.governingCarrier(),
      origin,
      _fareMarket.boardMultiCity(),
      _fareMarket.destination()->loc(),
      _fareMarket.offMultiCity(),
      _fareMarket.getGlobalDirection(),
      singleOverDouble,
      response,
      _trx.excTrxType() == PricingTrx::AR_EXC_TRX ? _travelDate : _fareMarket.travelDate(),
      _specFareCache);
#else
  const bool ok = AddonConstructionOrchestrator::process(
      _trx,
      getVendorForCat31(),
      _fareMarket.governingCarrier(),
      origin,
      _fareMarket.boardMultiCity(),
      _fareMarket.destination()->loc(),
      _fareMarket.offMultiCity(),
      singleOverDouble,
      response,
      _trx.excTrxType() == PricingTrx::AR_EXC_TRX ? _travelDate : _fareMarket.travelDate());
#endif

  postprocess<ConstructedFareInfoResponse::ConstructedFareInfoHashSet>(
      cxrFares, yyFares, response.responseHashSet());

  return ok;
}

template <class ContainerType>
void
AddonFareController::postprocess(std::vector<Fare*>& cxrFares,
                                 std::vector<Fare*>& yyFares,
                                 ContainerType& responseHashSet)
{
  typename ContainerType::iterator i = responseHashSet.begin();
  typename ContainerType::iterator ie = responseHashSet.end();

  bool sfaTag = false;
  std::vector<FareClassCode> fareClasses;
  sfaTag = sfaDiagRequested(_trx, fareClasses);

  if (sfaTag)
  {
    for (; i != ie; ++i)
    {
      ConstructedFareInfo* constrFareInfo = *i;

      uint16_t fareNum = 0;
      for (; fareNum != fareClasses.size(); ++fareNum)
      {
        if (constrFareInfo->fareInfo().fareClass() == fareClasses[fareNum])
          break;
      }
      if (fareNum >= fareClasses.size())
        continue;
      if (constrFareInfo->fareInfo().fareClass() != fareClasses[fareNum])
        continue;

      Fare* fare = createFare(&(constrFareInfo->fareInfo()),
                              _fareMarket.origin()->loc(),
                              Fare::FS_ConstructedFare,
                              constrFareInfo);

      if (fare != nullptr)
      {
        if (constrFareInfo->fareInfo().carrier() != INDUSTRY_CARRIER)
          cxrFares.push_back(fare);
        else
          yyFares.push_back(fare);
      }
    }
  }
  else
  {
    for (; i != ie; ++i)
    {
      ConstructedFareInfo* constrFareInfo = *i;

      Fare* fare = createFare(&(constrFareInfo->fareInfo()),
                              _fareMarket.origin()->loc(),
                              Fare::FS_ConstructedFare,
                              constrFareInfo);

      if (fare != nullptr)
      {
        if (constrFareInfo->fareInfo().carrier() != INDUSTRY_CARRIER)
          cxrFares.push_back(fare);
        else
          yyFares.push_back(fare);
      }
    }
  }
}
}
