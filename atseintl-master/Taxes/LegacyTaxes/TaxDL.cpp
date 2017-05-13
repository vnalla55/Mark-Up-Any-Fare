// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxDL.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
TaxDL::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxDL"));

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
TaxDL::TaxDL() : _fareUsage(nullptr) {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
TaxDL::~TaxDL() {}

// ----------------------------------------------------------------------------
// Description:  validateTripTypes
// ----------------------------------------------------------------------------

bool
TaxDL::validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)

{
  const AirSeg* airSeg;
  uint16_t segmentOrder = 0;
  bool locOrigin;
  bool locDestination;
  bool ftRequest = false;
  CurrencyConversionFacade ccFacade;

  _paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (!trx.getOptions()->currencyOverride().empty())
  {
    _paymentCurrency = trx.getOptions()->currencyOverride();
  }

  Money targetMoney(_paymentCurrency);

  _paymentCurrencyNoDec = targetMoney.noDec(trx.getRequest()->ticketingDT());

  _taxAmount = taxCodeReg.taxAmt();
  _travelSegStartIndex = 0;
  _travelSegEndIndex = 0;

  if (typeid(trx) == typeid(FareDisplayTrx))
  {
    FareDisplayTrx* fdTrx = dynamic_cast<FareDisplayTrx*>(&trx);
    if (fdTrx->getRequest()->requestType() == FARE_TAX_REQUEST)
      ftRequest = true;
  }

  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    if (segmentOrder >= taxResponse.farePath()->itin()->segmentOrder(*travelSegI))
      continue;

    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    locOrigin = LocUtil::isInLoc(*airSeg->origin(),
                                 taxCodeReg.loc1Type(),
                                 taxCodeReg.loc1(),
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::TAXES,
                                 GeoTravelType::International,
                                 EMPTY_STRING(),
                                 trx.getRequest()->ticketingDT());

    locDestination = LocUtil::isInLoc(*airSeg->destination(),
                                      taxCodeReg.loc2Type(),
                                      taxCodeReg.loc2(),
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    if (!locOrigin || !locDestination)
      continue;

    if (!locateFareUsage(taxResponse, **travelSegI))
    {
      LOG4CXX_DEBUG(_logger, "Fare Not Found *** TaxDL::validateTripTypes ***");

      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic820);

      continue;
    }

    TravelSeg* travelSegFront = _fareUsage->travelSeg().front();
    TravelSeg* travelSegBack = _fareUsage->travelSeg().back();

    segmentOrder = taxResponse.farePath()->itin()->segmentOrder(travelSegBack);

    locOrigin = LocUtil::isInLoc(*travelSegFront->origin(),
                                 taxCodeReg.loc1Type(),
                                 taxCodeReg.loc1(),
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::TAXES,
                                 GeoTravelType::International,
                                 EMPTY_STRING(),
                                 trx.getRequest()->ticketingDT());

    locDestination = LocUtil::isInLoc(*travelSegBack->destination(),
                                      taxCodeReg.loc1Type(),
                                      taxCodeReg.loc1(),
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    if (!locOrigin || !locDestination)
      continue;

    MoneyAmount moneyAmount = _fareUsage->totalFareAmount();

    if (taxResponse.farePath()->calculationCurrency() != taxResponse.farePath()->baseFareCurrency())
    {
      Money targetMoneyOrigination(taxResponse.farePath()->baseFareCurrency());
      targetMoneyOrigination.value() = 0;

      Money sourceMoneyCalculation(moneyAmount, taxResponse.farePath()->calculationCurrency());

      if (!ccFacade.convert(targetMoneyOrigination,
                            sourceMoneyCalculation,
                            trx,
                            taxResponse.farePath()->itin()->useInternationalRounding()))
      {
        LOG4CXX_WARN(_logger, "Currency Convertion Collection *** TaxDL::validateTripTypes ***");

        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic820);
      }
      moneyAmount = targetMoneyOrigination.value();
    }

    if (taxResponse.farePath()->baseFareCurrency() != _paymentCurrency)
    {
      targetMoney.value() = 0;
      Money sourceMoney(moneyAmount, taxResponse.farePath()->baseFareCurrency());

      if (!ccFacade.convert(targetMoney, sourceMoney, trx, false))
      {
        LOG4CXX_WARN(_logger, "Currency Convertion Collection *** TaxDL::validateTripTypes ***");

        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic820);
      }
      moneyAmount = targetMoney.value();
    }
    _taxableFare += moneyAmount;

    if (ftRequest)
      break;
  }

  _taxAmount = _taxableFare * taxCodeReg.taxAmt();

  if (_taxAmount == 0.0)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic820);
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

void
TaxDL::taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex)
{
  return;
}

/// ----------------------------------------------------------------------------
// Description:  locatedFareUsage
// ----------------------------------------------------------------------------

bool
TaxDL::locateFareUsage(TaxResponse& taxResponse, TravelSeg& travelSeg)
{
  FarePath* farePath = taxResponse.farePath();

  std::vector<PricingUnit*>::const_iterator pricingUnitI;
  std::vector<FareUsage*>::iterator fareUsageI;
  std::vector<TravelSeg*>::const_iterator travelSegFuI;

  for (pricingUnitI = farePath->pricingUnit().begin();
       pricingUnitI != farePath->pricingUnit().end();
       pricingUnitI++)
  {
    for (fareUsageI = (*pricingUnitI)->fareUsage().begin();
         fareUsageI != (*pricingUnitI)->fareUsage().end();
         fareUsageI++)
    {
      for (travelSegFuI = (*fareUsageI)->travelSeg().begin();
           travelSegFuI != (*fareUsageI)->travelSeg().end();
           travelSegFuI++)
      {
        if (taxResponse.farePath()->itin()->segmentOrder(*travelSegFuI) !=
            taxResponse.farePath()->itin()->segmentOrder(&travelSeg))
          continue;

        _fareUsage = *fareUsageI;
        return true;
      }
    }
  }
  return false;
}
