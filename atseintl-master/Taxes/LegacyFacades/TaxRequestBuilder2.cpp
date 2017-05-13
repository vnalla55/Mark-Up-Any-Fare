// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TSEException.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RequestResponse/InputGeoPathMapping.h"
#include "DataModel/RequestResponse/InputYqYrPath.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DomainDataObjects/FlightUsage.h"
#include "Rules/RuleConst.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Taxes/AtpcoTaxes/Common/Convert.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/CodeOps.h"
#include "Taxes/AtpcoTaxes/Diagnostic/AtpcoDiagnostic.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Passenger.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Request.h"
#include "Taxes/Common/ReissueExchangeDateSetter.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyFacades/CabinUtils.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/CurrencyServiceV2.h"
#include "Taxes/LegacyFacades/FacadesUtils.h"
#include "Taxes/LegacyFacades/InputPointOfSaleFactory.h"
#include "Taxes/LegacyFacades/OptionalServicesBuilder.h"
#include "Taxes/LegacyFacades/OptionalServicesForOtaTaxReqBuilder.h"
#include "Taxes/LegacyFacades/TaxRequestBuilder2.h"
#include "Taxes/LegacyFacades/V2TrxMappingDetails.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <map>
#include <vector>

using namespace tse;
namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(fallbackAtpcoTaxDefaultAgeForADTCNN);
FALLBACK_DECL(markupAnyFareOptimization);
}

namespace tax
{
namespace
{

template<typename T>
const T& as_const(T& r)
{
  return r;
}

struct SegFareInfo
{
  SegFareInfo(int16_t _index, tse::TravelSeg* _tseTravelSeg,
      tse::FareUsage* _tseFareUsage, InputMapping* _mapping) :
      index(_index), tseTravelSeg(_tseTravelSeg), tseFareUsage(_tseFareUsage), mapping(
          _mapping)
  {
  }
  bool operator<(const SegFareInfo& rhs) const
  {
    return index < rhs.index;
  }

  int16_t index;
  tse::TravelSeg* tseTravelSeg;
  tse::FareUsage* tseFareUsage;
  InputMapping* mapping;
};

Logger _logger("atseintl.AtpcoTaxes.TaxRequestBuilder");

tax::type::Index addGeoPathMapping(tax::InputGeoPathMapping* geoPathMapping,
    tax::InputRequest::InputGeoPathMappings& geoPathMappings)
{
  tax::type::Index foundId = 0;
  if (FacadesUtils::findGeoPathMapping(*geoPathMapping, geoPathMappings,
      foundId))
  {
    delete geoPathMapping;
    return foundId;
  }

  geoPathMappings.push_back(geoPathMapping);
  geoPathMapping->_id = geoPathMappings.size() - 1;

  return geoPathMapping->_id;
}

void processInputTseFareUsage(tse::PricingTrx& trx,
    const tse::CurrencyCode& baseFareCurrency,
    const tse::CurrencyCode& calculationCurrency,
    const tse::CurrencyCode& paymentCurrency, const tse::Itin& tseItin,
    const tse::FareUsage& tseFareUsage, InputFarePath& farePath,
    boost::ptr_vector<InputFare>& fares)
{
  InputFareUsage* fareUsage = new InputFareUsage();
  fareUsage->_fareRefId = fares.size();

  InputFare* fare = new InputFare();
  fares.push_back(fare);
  farePath._fareUsages.push_back(fareUsage);

  if (LIKELY(nullptr != tseFareUsage.paxTypeFare()))
  {
    FareClassCode code =
        tseFareUsage.paxTypeFare()->fare()->fareInfo()->fareClass();
    fare->_basis = code.c_str();

    if (LIKELY(fare->_basis.find('/') == std::string::npos))
    {
      std::string tktDesignator;
      tseFareUsage.paxTypeFare()->createTktDesignator(tktDesignator);
      if (tktDesignator.empty())
      {
        const int16_t segOrd = tseItin.segmentOrder(
            tseFareUsage.travelSeg().front());
        tktDesignator = trx.getRequest()->tktDesignator(segOrd).c_str();
        if (LIKELY(tktDesignator.empty()))
          tktDesignator =
              trx.getRequest()->specifiedTktDesignator(segOrd).c_str();
      }

      if (!tktDesignator.empty())
      {
        fare->_basis += '/';
        fare->_basis.append(tktDesignator);
      }
    }
    fare->_type = tseFareUsage.paxTypeFare()->fcaFareType().c_str();
    fare->_directionality = toTaxDirectionality(
        tseFareUsage.paxTypeFare()->directionality());
    fare->_oneWayRoundTrip = tseFareUsage.paxTypeFare()->owrt();
    fare->_sellAmount = 0;
    fare->_isNetRemitAvailable = false;
    const FareClassAppInfo* fca =
        tseFareUsage.paxTypeFare()->fareClassAppInfo();
    if (LIKELY(fca))
    {
      fare->_tariff = Convert::intToShort(fca->_ruleTariff);
      fare->_rule = fca->_ruleNumber;
    }

    bool isCat19Discount = false;
    if (tseFareUsage.paxTypeFare()->actualPaxType()->paxType() == "ADT"
        && tseFareUsage.paxTypeFare()->isDiscounted())
    {
      try
      {
        if (tseFareUsage.paxTypeFare()->discountInfo().category() == 19)
        {
          isCat19Discount = true;
          fare->_outputPtc = farePath._outputPtc;
        }
      } catch (...)
      {
      } // discountInfo throws if no rule data, that's normal
    }
    if (!isCat19Discount)
      fare->_outputPtc = toTaxPassengerCode(
          tseFareUsage.paxTypeFare()->actualPaxType()->paxType());

    bool noConversion = tseFareUsage.paxTypeFare()->currency()
        == paymentCurrency && baseFareCurrency.empty()
        && calculationCurrency.empty();
    const tse::CurrencyServiceV2 currencyService(trx,
        trx.getRequest()->ticketingDT());
    try
    {
      if (noConversion)
        fare->_amount = tax::doubleToAmount(tseFareUsage.totalFareAmount());
      else
        fare->_amount = currencyService.convert(tseFareUsage.totalFareAmount(),
            baseFareCurrency, calculationCurrency, paymentCurrency,
            CurrencyConversionRequest::FARES,
            tseItin.useInternationalRounding());
    } catch (const std::runtime_error& exception)
    {
      LOG4CXX_ERROR(_logger, exception.what());
    }

    if (tseFareUsage.paxTypeFare()->isNegotiated())
    {
      const NegPaxTypeFareRuleData* negPaxTypeFare =
          tseFareUsage.paxTypeFare()->getNegRuleData();
      if (negPaxTypeFare && negPaxTypeFare->nucNetAmount() > 0)
      {
        try
        {
          if (noConversion)
            fare->_sellAmount = tax::doubleToAmount(
                negPaxTypeFare->netAmount());
          else
            fare->_sellAmount = currencyService.convert(
                negPaxTypeFare->netAmount(), baseFareCurrency,
                calculationCurrency, paymentCurrency,
                CurrencyConversionRequest::FARES,
                tseItin.useInternationalRounding());
        } catch (const std::runtime_error& exception)
        {
          LOG4CXX_ERROR(_logger, exception.what());
        }

        fare->_isNetRemitAvailable = true;
      }
    }
  }
}

void processInputTseTravelSegForFlight(const tse::TravelSeg& tseTravelSeg,
    const tse::TravelSeg* const previousTseTravelSeg, const tse::Itin& tseItin,
    const tse::FareUsage& tseFareUsage, boost::ptr_vector<InputFlight>& flights,
    boost::ptr_vector<InputFlightUsage>& flightUsages,
    type::UnticketedTransfer unticketedTag,
    bool afterHiddenStop = false)
{
  InputFlight* flight = new InputFlight();
  flight->_equipment = tseTravelSeg.equipmentType();

  if (unticketedTag == type::UnticketedTransfer::No)
  {
    flight->_departureTime = boost::posix_time::time_duration(
        tseTravelSeg.departureDT().hours(),
        tseTravelSeg.departureDT().minutes(), 0, 0);
    flight->_arrivalTime = boost::posix_time::time_duration(
        tseTravelSeg.arrivalDT().hours(), tseTravelSeg.arrivalDT().minutes(), 0,
        0);
    flight->_arrivalDateShift =
        Convert::longToShort(
            (tseTravelSeg.arrivalDT().date() - tseTravelSeg.departureDT().date()).days());
  } else
  {
    flight->_departureTime = boost::posix_time::time_duration(
        tseTravelSeg.departureDT().hours(),
        tseTravelSeg.departureDT().minutes(), 0, 0);
    // Make arrival same as departure, like flight to hidden stop took no time
    flight->_arrivalTime = boost::posix_time::time_duration(
        tseTravelSeg.departureDT().hours(),
        tseTravelSeg.departureDT().minutes(), 0, 0);
    flight->_arrivalDateShift = 0;
  }

  const tse::AirSeg* airSeg = tseTravelSeg.toAirSeg();
  if (LIKELY(nullptr != airSeg))
  {
    flight->_operatingCarrier = toTaxCarrierCode(
        airSeg->operatingCarrierCode());
    flight->_marketingCarrier = toTaxCarrierCode(
        airSeg->marketingCarrierCode());
    flight->_marketingCarrierFlightNumber = Convert::intToShort(
        airSeg->marketingFlightNumber());
  }

  const std::pair<BookingCode, tax::type::CabinCode>& bookingInfo =
      CabinUtils::getBookingCodeAndCabin(tseItin, tseFareUsage, &tseTravelSeg);
  flight->_reservationDesignator = bookingInfo.first;
  flight->_cabinCode = bookingInfo.second;

  flights.push_back(flight);

  InputFlightUsage* flightUsage = new InputFlightUsage();
  flightUsage->_flightRefId = flights.size() - 1;

  if (!afterHiddenStop)
  {
    flightUsage->_connectionDateShift = Convert::longToShort(
        (nullptr != previousTseTravelSeg) ?
            (tseTravelSeg.departureDT().date()
                - previousTseTravelSeg->arrivalDT().date()).days() :
            0);
  }

  if (UNLIKELY(tseTravelSeg.isForcedStopOver()))
  {
    flightUsage->_forcedConnection = type::ForcedConnection::Stopover;
  } else if (UNLIKELY(tseTravelSeg.isForcedConx()))
  {
    flightUsage->_forcedConnection = type::ForcedConnection::Connection;
  }

  if (UNLIKELY(tseTravelSeg.isOpenWithoutDate()))
  {
    flightUsage->_openSegmentIndicator = type::OpenSegmentIndicator::Open;
  } else if (UNLIKELY(tseTravelSeg.segmentType() == Open))
  {
    flightUsage->_openSegmentIndicator = type::OpenSegmentIndicator::DateFixed;
  }

  flightUsages.push_back(flightUsage);
}

void createGeo(const tse::TravelSeg& tseTravelSeg, const tse::Loc& loc,
    type::TaxPointTag taxPointTag, type::UnticketedTransfer unticketedTag,
    TaxRequestBuilder::TravelSegGeoItems& items,
    boost::ptr_vector<InputGeo>& geos, boost::ptr_vector<InputMap>& maps)
{
  geos.push_back(new InputGeo());
  InputGeo& geo = geos.back();

  geo._id = geos.size() - 1;
  geo._loc.tag() = taxPointTag;
  geo._loc.code() = tse::toTaxAirportCode(loc.loc());
  geo._loc.inBufferZone() = loc.bufferZoneInd();
  geo._unticketedTransfer = unticketedTag;

  maps.push_back(new InputMap());
  maps.back()._geoRefId = geos.back()._id;

  items[&tseTravelSeg].push_back(&geo);
}

void processInputTseTravelSeg(const tse::Itin& tseItin,
    const tse::FareUsage& tseFareUsage, tse::TravelSeg* travelSeg,
    const tse::TravelSeg* previousTravelSeg, boost::ptr_vector<InputGeo>& geos,
    TaxRequestBuilder::TravelSegGeoItems& items,
    boost::ptr_vector<InputFlight>& flights,
    boost::ptr_vector<InputFlightUsage>& flightUsages, InputMapping& mapping)
{
  // origin
  createGeo(*travelSeg, *travelSeg->origin(), type::TaxPointTag::Departure,
      type::UnticketedTransfer::No, items, geos, mapping.maps());

  bool afterHiddenStop = false;
  for (const tse::Loc* hiddenStop : travelSeg->hiddenStops())
  {
    // one per hidden segment
    processInputTseTravelSegForFlight(*travelSeg, previousTravelSeg, tseItin,
        tseFareUsage, flights, flightUsages, type::UnticketedTransfer::Yes,
        afterHiddenStop);

    // hiddenStop Arrival
    createGeo(*travelSeg, *hiddenStop, type::TaxPointTag::Arrival,
        type::UnticketedTransfer::Yes, items, geos, mapping.maps());

    // hiddenStop Departure
    createGeo(*travelSeg, *hiddenStop, type::TaxPointTag::Departure,
        type::UnticketedTransfer::Yes, items, geos, mapping.maps());

    afterHiddenStop = true;
  }

  // destination
  createGeo(*travelSeg, *travelSeg->destination(), type::TaxPointTag::Arrival,
      type::UnticketedTransfer::No, items, geos, mapping.maps());

  processInputTseTravelSegForFlight(*travelSeg, previousTravelSeg, tseItin,
      tseFareUsage, flights, flightUsages, type::UnticketedTransfer::No,
      afterHiddenStop);
}

void processInputTseItin(tse::PricingTrx& trx, const tse::Itin& tseItin,
    const tse::FarePath& tseFarePath, InputRequest& taxRequest, InputItin& itin,
    InputFarePath& farePath, InputGeoPath& geoPath, InputFlightPath& flightPath,
    TaxRequestBuilder::TravelSegGeoItems& items,
    const type::CarrierCode& validatingCarrier)
{
  farePath._validatingCarrier = validatingCarrier;
  farePath._outputPtc = toTaxPassengerCode(tseFarePath.paxType()->paxType());

  InputGeoPathMapping* geoPathMapping = new InputGeoPathMapping();
  tse::MoneyAmount totalAmountBeforeDiscount = 0;

  // Preprocessing to get all segments in correct order
  std::vector<SegFareInfo> segFareInfos;

  for (tse::PricingUnit* tsePricingUnit : tseFarePath.pricingUnit())
  {
    for (tse::FareUsage* tseFareUsage : tsePricingUnit->fareUsage())
    {
      geoPathMapping->_mappings.push_back(new InputMapping());
      InputMapping& mapping = geoPathMapping->_mappings.back();

      for (tse::TravelSeg* tseTravelSeg : tseFareUsage->travelSeg())
      {
        if (!tseTravelSeg->isAir())
          continue;

        segFareInfos.push_back(
            SegFareInfo(tseItin.segmentOrder(tseTravelSeg), tseTravelSeg,
                tseFareUsage, &mapping));
      }

      tse::PaxTypeFare* ptf = tseFareUsage->paxTypeFare();
      if (LIKELY(nullptr != ptf))
      {
        PaxTypeFare* baseFare = nullptr;
        try
        {
          if (ptf->isFareByRule() && !ptf->isSpecifiedFare()
              && (baseFare = ptf->baseFare(25)) != nullptr)
          {
            totalAmountBeforeDiscount += std::max(baseFare->nucFareAmount(),
                ptf->nucFareAmount());
          } else
          {
            totalAmountBeforeDiscount += ptf->nucFareAmount();
          }
        } catch (const TSEException&)
        {
          totalAmountBeforeDiscount += ptf->nucFareAmount();
        }
      }

      if (LIKELY(!taxUtil::isAnciliaryItin(trx, tseItin)))
      {
        processInputTseFareUsage(trx, (tseFarePath.baseFareCurrency()),
            (tseFarePath.calculationCurrency()),
            toTseCurrencyCode(taxRequest.ticketingOptions().paymentCurrency()),
            tseItin, *tseFareUsage, farePath, taxRequest.fares());
      }
    }
  }
  std::sort(segFareInfos.begin(), segFareInfos.end());

  tse::TravelSeg* prevTravelSeg = nullptr;
  // Conversion
  for (const SegFareInfo& segFareInfo : segFareInfos)
  {
    processInputTseTravelSeg(tseItin, *segFareInfo.tseFareUsage,
        segFareInfo.tseTravelSeg, prevTravelSeg, geoPath._geos, items,
        taxRequest.flights(), flightPath._flightUsages, *segFareInfo.mapping);

    prevTravelSeg = segFareInfo.tseTravelSeg;
  }

  bool noConversion = tseFarePath.baseFareCurrency().empty()
      && tseFarePath.calculationCurrency().empty();
  const tse::CurrencyServiceV2 currencyService(trx,
      trx.getRequest()->ticketingDT());
  try
  {
    if (noConversion)
    {
      farePath._totalAmount = tax::doubleToAmount(
          tseFarePath.getTotalNUCAmount());
      if (!fallback::markupAnyFareOptimization(&trx))
      {
        farePath._totalMarkupAmount = tax::doubleToAmount(
            tseFarePath.getTotalNUCMarkupAmount());
      }
      farePath._totalAmountBeforeDiscount = tax::doubleToAmount(
          totalAmountBeforeDiscount);
    } else
    {
      farePath._totalAmount = currencyService.convert(
          tseFarePath.getTotalNUCAmount(), tseFarePath.baseFareCurrency(),
          tseFarePath.calculationCurrency(),
          toTseCurrencyCode(taxRequest.ticketingOptions().paymentCurrency()),
          CurrencyConversionRequest::FARES, tseItin.useInternationalRounding());

      if (!fallback::markupAnyFareOptimization(&trx))
      {
        farePath._totalMarkupAmount = currencyService.convert(
            tseFarePath.getTotalNUCMarkupAmount(), tseFarePath.baseFareCurrency(),
            tseFarePath.calculationCurrency(),
            toTseCurrencyCode(taxRequest.ticketingOptions().paymentCurrency()),
            CurrencyConversionRequest::FARES,
            tseItin.useInternationalRounding());
      }

      farePath._totalAmountBeforeDiscount = currencyService.convert(
          totalAmountBeforeDiscount, tseFarePath.baseFareCurrency(),
          tseFarePath.calculationCurrency(),
          toTseCurrencyCode(taxRequest.ticketingOptions().paymentCurrency()),
          CurrencyConversionRequest::FARES, tseItin.useInternationalRounding());
    }
  } catch (const std::runtime_error& exception)
  {
    LOG4CXX_ERROR(_logger, exception.what());
  }

  const type::Index geoPathMappingId = addGeoPathMapping(geoPathMapping,
      taxRequest.geoPathMappings());
  itin._farePathGeoPathMappingRefId = geoPathMappingId;
  itin._geoPathRefId = geoPath._id;
  itin._farePathRefId = taxRequest.farePaths().size() - 1;
  itin._passengerRefId = itin._id;

  const bool pccOverride = tseItin.agentPCCOverride() != nullptr
      && !equal(taxRequest.pointsOfSale()[0].agentCity(),
          tseItin.agentPCCOverride()->agentCity());

  if (pccOverride)
  {
    std::unique_ptr<InputPointOfSale> pos =
        InputPointOfSaleFactory::createInputPointOfSale(trx,
            *tseItin.agentPCCOverride(), taxRequest.pointsOfSale().size());

    taxRequest.pointsOfSale().push_back(pos.release());
    itin._pointOfSaleRefId = taxRequest.pointsOfSale().size() - 1;
  }

  tse::DateTime& origin = tseItin.travelSeg()[0]->departureDT();
  itin._travelOriginDate = boost::gregorian::date(origin.year(), origin.month(),
      origin.day());

  try
  {
    itin._label = boost::lexical_cast<std::string>(tseItin.getItinIndex());
  } catch (boost::bad_lexical_cast& e)
  {
    LOG4CXX_ERROR(_logger, e.what());
  }
}

InputApplyOn makeGroup(int i)
{
  InputApplyOn ans;
  ans._group = i;
  return ans;
}

void addChangeFee(const type::MoneyAmount& amount, InputRequest& inputRequest,
    InputItin& itin)
{
  InputChangeFee* inputChangeFee = new InputChangeFee();
  inputChangeFee->_id = inputRequest.changeFees().size();
  inputChangeFee->_amount = amount;

  itin._changeFeeRefId = inputRequest.changeFees().size();
  inputRequest.changeFees().push_back(inputChangeFee);
}

void proccessChangeFee(const tse::FarePath& tseFarePath,
    tax::AtpcoTaxesActivationStatus activationStatus, tse::PricingTrx& trx,
    InputRequest& inputRequest, InputItin& itin)
{
  if (!activationStatus.isTaxOnChangeFee())
  {
    return;
  }

  if ((tseFarePath.reissueCharges() != nullptr)
      && (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      && (tseFarePath.reissueCharges()->changeFee > tse::EPSILON)
      && !tseFarePath.ignoreReissueCharges())
  {
    type::MoneyAmount changeFeeAmount = doubleToAmount(
        tseFarePath.reissueCharges()->changeFee);
    const tse::CurrencyCode& changeFeeCurrency =
        tseFarePath.reissueCharges()->changeFeeCurrency;

    if (inputRequest.ticketingOptions().paymentCurrency()
        != toTaxCurrencyCode(changeFeeCurrency))
    {
      tax::type::Money source;
      source._amount = changeFeeAmount;
      source._currency = toTaxCurrencyCode(changeFeeCurrency);

      const tse::CurrencyServiceV2 currencyService(trx,
          trx.getRequest()->ticketingDT());
      changeFeeAmount = currencyService.convertTo(
          inputRequest.ticketingOptions().paymentCurrency(), source);
    }

    addChangeFee(changeFeeAmount, inputRequest, itin);

    ReissueExchangeDateSetter(trx, tseFarePath);
  }
}

void addPassenger(const tse::PricingTrx& trx, const tse::DateTime& ticketingDT,
    const tse::PricingOptions* options, const tse::PaxType& paxType,
    boost::ptr_vector<InputPassenger>& passengers)
{
  std::unique_ptr<InputPassenger> passenger(new InputPassenger());
  passenger->_id = passengers.size();
  passenger->_code = toTaxPassengerCode(paxType.paxType());
  passenger->_stateCode = paxType.stateCode();
  if (LIKELY(options))
  {
    passenger->_passengerStatus._nationality = tse::toTaxNationCode(
        options->nationality());
    passenger->_passengerStatus._employment = tse::toTaxNationCode(
        options->employment());
    passenger->_passengerStatus._residency = tse::toTaxLocZoneCode(
        options->residency());
  }
  // passenger->_passengerStatus =
  // passenger->_passengersCount = paxType->totalPaxNumber();

  if (paxType.age() != 0)
  {
    passenger->_birthDate = boost::gregorian::date(ticketingDT.year(),
        ticketingDT.month(), ticketingDT.day());

    passenger->_birthDate -= boost::gregorian::years(paxType.age());
  } else // paxType.age() == 0, age unknown (except infants, where this is ok)
  {
    static const std::vector<std::string> infantPtc = { "CBI", "INE", "FBI",
        "FGI", "FNF", "INF", "INR", "INS", "INY", "ISR", "ITF", "ITS", "JNF",
        "JNS", "LIF", "LNS", "MNF", "MNS", "ZEI", "GIF", "GIS", "INX", "MIF",
        "MSS", "TNF", "WBI", "WBS" };
    static const std::vector<std::string> childPtc = { "CBC", "CHR", "CNN",
        "CSB", "DNN", "ECH", "ENN", "FNN", "GNN", "INN", "JNN", "LNN", "MNN",
        "PNN", "TNN", "UNN", "VNN", "YNN", "ZEC", "CCA", "FBC", "FGC", "MDP",
        "MIC", "UNR", "WBC", "CNE" };

    if (!fallback::fallbackAtpcoTaxDefaultAgeForADTCNN(&trx)) // inf = 0, cnn = 2, other = 18
    {
      passenger->_birthDate = boost::gregorian::date(
          Convert::intToShort(ticketingDT.year()), ticketingDT.month(),
          ticketingDT.day());

      if (std::find(childPtc.begin(), childPtc.end(), paxType.paxType())
          != childPtc.end())
        passenger->_birthDate -= boost::gregorian::years(2);
      else if (std::find(infantPtc.begin(), infantPtc.end(), paxType.paxType())
          == infantPtc.end())
        passenger->_birthDate -= boost::gregorian::years(18);
    } else // default age for inf = 0, other blank date, strict matching
    {
      if (std::find(infantPtc.begin(), infantPtc.end(), paxType.paxType())
          != infantPtc.end())
        passenger->_birthDate = boost::gregorian::date(
            Convert::intToShort(ticketingDT.year()), ticketingDT.month(),
            ticketingDT.day());
      else
        passenger->_birthDate = boost::gregorian::date(
            boost::gregorian::not_a_date_time);
    }
  }

  passengers.push_back(passenger.release());
}

void processInputDiagnostic(tse::Diagnostic& tseDiagnostic,
    tax::InputDiagnosticCommand& diagnostic)
{
  diagnostic.number() = static_cast<uint32_t>(tseDiagnostic.diagnosticType());

  if (diagnostic.number() == 0)
  {
    return;
  }

  typedef std::map<std::string, std::string> DiagParamMap;
  typedef DiagParamMap::const_iterator DiagParamMap_iterator;

  DiagParamMap& parameters = tseDiagnostic.diagParamMap();
  DiagParamMap::const_iterator i = parameters.begin();

  for (DiagParamMap_iterator it = parameters.begin(); it != parameters.end();
      it++)
  {
    InputParameter* parameter = new InputParameter();
    parameter->name() = it->first;
    parameter->value() = it->second;
    diagnostic.parameters().push_back(parameter);
  }
}

tax::type::CurrencyCode getRequestedCurrency(tse::PricingTrx& trx)
{
  tse::CurrencyCode ans = trx.getOptions()->currencyOverride();
  if (ans.empty())
    ans = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  return toTaxCurrencyCode(ans);
}

void processInputCommonHeader(tse::PricingTrx& trx, InputRequest& request)
{
  const tse::Agent* ticketingAgent = trx.getRequest()->ticketingAgent();
  if (ticketingAgent == nullptr)
    throw std::runtime_error("No ticketing agent in Pricing Request");

  std::unique_ptr<InputPointOfSale> pointOfSale =
      InputPointOfSaleFactory::createInputPointOfSale(trx, *ticketingAgent,
          request.pointsOfSale().size());
  request.pointsOfSale().push_back(pointOfSale.release());

  // TODO: find out how to fill in request.ticketingOptions().formOfPayment()
  // Paid by government - trx.getRequest()->formOfPaymentGTR() ??
  // Paid by miles - not available ??

  request.ticketingOptions().ticketingPoint() =
      request.pointsOfSale().back().loc();
  if (!trx.getRequest()->ticketPointOverride().empty())
  {
    request.ticketingOptions().ticketingPoint() = toTaxAirportCode(
        trx.getRequest()->ticketPointOverride());
  }
  request.ticketingOptions().paymentCurrency() = getRequestedCurrency(trx);

  request.ticketingOptions().ticketingDate() = boost::gregorian::date(
      trx.ticketingDate().year(), trx.ticketingDate().month(),
      trx.ticketingDate().day());
  request.ticketingOptions().ticketingTime() = boost::posix_time::time_duration(
      trx.ticketingDate().hours(), trx.ticketingDate().minutes(), 0, 0);

  if (!fallback::fallbackValidatingCxrMultiSp(&trx)
      || trx.overrideFallbackValidationCXRMultiSP())
  {
    request.processing()._tch = trx.getRequest()->getSettlementMethod() == "TCH"
        || (trx.countrySettlementPlanInfo()
            && trx.countrySettlementPlanInfo()->getSettlementPlanTypeCode()
                == "TCH");

  } else
    request.processing()._tch = trx.getRequest()->getSettlementMethod()
        == "TCH";

  if (trx.getOptions())
    request.processing()._rtw = trx.getOptions()->isRtw();

  if (trx.getRequest()->isExemptAllTaxes())
  {
    InputCalculationRestriction* cr = new InputCalculationRestriction();
    cr->restrictionType() = type::CalcRestriction::ExemptAllTaxesAndFees;
    request.processing().calculationRestrictions().push_back(cr);
  }
  if (trx.getRequest()->isExemptSpecificTaxes()
      && trx.getRequest()->taxIdExempted().empty())
  {
    InputCalculationRestriction* cr = new InputCalculationRestriction();
    cr->restrictionType() = type::CalcRestriction::ExemptAllTaxes;
    request.processing().calculationRestrictions().push_back(cr);
  }
  if (trx.getRequest()->isExemptSpecificTaxes()
      && !trx.getRequest()->taxIdExempted().empty())
  {
    std::unique_ptr<InputCalculationRestriction> cr(
        new InputCalculationRestriction());
    cr->restrictionType() = type::CalcRestriction::ExemptSpecifiedTaxes;

    InputCalculationRestrictionTax* crt = nullptr;
    for (const std::string& exemptedTaxId : trx.getRequest()->taxIdExempted())
    {
      crt = new InputCalculationRestrictionTax();
      crt->sabreTaxCode() = exemptedTaxId;
      cr->calculationRestrictionTax().push_back(crt);
    }
    request.processing().calculationRestrictions().push_back(cr.release());
  }
}

tse::FarePath&
selectGsaFarePath(tse::FarePath& mainFarePath,
    const tse::CarrierCode& validatingCarrier)
{
  tse::FarePath* alternative = mainFarePath.findTaggedFarePath(
      validatingCarrier);
  return alternative ? *alternative : mainFarePath;
}

std::string address(const void * addr)
{
  std::stringstream ans;
  ans << addr;
  std::string strans = ans.str();
  return strans.substr(strans.size() - 4);
}

const std::string separator = std::string(AtpcoDiagnostic::LINE_LENGTH, '*')
    + std::string(1, '\n');
void logInputValCarriers(std::ostream* gsaDiag, tse::Itin& tseItin,
    tse::FarePath& tseFarePath)
{
  if (gsaDiag)
  {
    *gsaDiag << separator << "STAGE: BUILD REQUEST\n";
    *gsaDiag << "  ITIN: " << address(&tseItin) << "  FPATH: "
        << address(&tseFarePath) << "\n";
    *gsaDiag << "    VAL CARRIERS:";

    for (const CarrierCode& alternateValCxr : tseFarePath.validatingCarriers())
      *gsaDiag << " " << alternateValCxr;

    *gsaDiag << "\n";
  }
}

} // anonymous namespace

void TaxRequestBuilder::process(const ServicesFeesMap& servicesFees,
    V2TrxMappingDetails& v2Mapping, InputRequest& inputRequest,
    std::ostream* gsaDiag)
{
  if (gsaDiag)
    *gsaDiag << separator << std::string(20, ' ') << "GSA DIAGNOSTIC\n";

  processApplicableGroups(inputRequest.processing());
  processInputCommonHeader(_trx, inputRequest);
  processInputDiagnostic(_trx.diagnostic(), inputRequest.diagnostic());

  ItinSelector itinSelector(_trx);

  for (tse::Itin* tseItin : itinSelector.get())
  {
    std::vector<type::Index> arunksBeforeSeg = computeArunksBeforeSeg(
        tseItin->travelSeg());
    std::vector<type::Index> hiddenBeforeSeg = computeHiddenBeforeSeg(
        tseItin->travelSeg());
    for (tse::FarePath* tseFarePath : tseItin->farePath())
    {
      processTopLevelFarePath(servicesFees, *tseItin, *tseFarePath,
          arunksBeforeSeg, hiddenBeforeSeg, v2Mapping, inputRequest, gsaDiag);
    }
  }
}

void TaxRequestBuilder::processTopLevelFarePath(
    const ServicesFeesMap& servicesFees, tse::Itin& tseItin,
    tse::FarePath& tseFarePath, const std::vector<type::Index>& arunksBeforeSeg,
    const std::vector<type::Index>& hiddenBeforeSeg,
    V2TrxMappingDetails& v2Mapping, InputRequest& inputRequest,
    std::ostream* gsaDiag)
{
  const bool gsaLogicEnabled = _trx.isValidatingCxrGsaApplicable()
      && _activationStatus.isTaxOnItinYqYrTaxOnTax();

  const std::vector<tse::CarrierCode>& valCxrs =
      as_const(tseFarePath).validatingCarriers();
  const bool doGsa = gsaLogicEnabled && !valCxrs.empty();

  if (doGsa)
  {
    logInputValCarriers(gsaDiag, tseItin, tseFarePath);

    for (const CarrierCode& alternateValCxr : valCxrs)
    {
      tse::FarePath& workingFarePath = selectGsaFarePath(tseFarePath,
          alternateValCxr);
      processMainAndNetFarePath(servicesFees, tseItin, workingFarePath,
          arunksBeforeSeg, hiddenBeforeSeg, v2Mapping, inputRequest,
          toTaxCarrierCode(alternateValCxr), &tseFarePath);
    }
  } else
  {
    processMainAndNetFarePath(servicesFees, tseItin, tseFarePath,
        arunksBeforeSeg, hiddenBeforeSeg, v2Mapping, inputRequest,
        toTaxCarrierCode(tseItin.validatingCarrier()), nullptr);
  }
}

void TaxRequestBuilder::processMainAndNetFarePath(
    const ServicesFeesMap& servicesFees, tse::Itin& tseItin,
    tse::FarePath& tseFarePath, const std::vector<type::Index>& arunksBeforeSeg,
    const std::vector<type::Index>& hiddenBeforeSeg,
    V2TrxMappingDetails& v2Mapping, InputRequest& inputRequest,
    const type::CarrierCode& validatingCarrier,
    tse::FarePath* mainPath /* can be null */)
{
  processFarePathForItin(servicesFees, tseItin, tseFarePath, arunksBeforeSeg,
      hiddenBeforeSeg, v2Mapping, inputRequest, validatingCarrier, mainPath);

  if (TrxUtil::isCat35TFSFEnabled(_trx) && tseFarePath.netFarePath())
  {
    processFarePathForItin(servicesFees, tseItin, *tseFarePath.netFarePath(),
        arunksBeforeSeg, hiddenBeforeSeg, v2Mapping, inputRequest,
        validatingCarrier, mainPath);
  }

  if (fallback::markupAnyFareOptimization(&_trx))
  {
    if (tseFarePath.adjustedSellingFarePath())
    {
      processFarePathForItin(servicesFees, tseItin,
          *tseFarePath.adjustedSellingFarePath(), arunksBeforeSeg,
          hiddenBeforeSeg, v2Mapping, inputRequest, validatingCarrier,
          mainPath);
    }
  }
}

void TaxRequestBuilder::processFarePathForItin(
    const ServicesFeesMap& servicesFees, tse::Itin& tseItin,
    tse::FarePath& tseFarePath, const std::vector<type::Index>& arunksBeforeSeg,
    const std::vector<type::Index>& hiddenBeforeSeg,
    V2TrxMappingDetails& v2Mapping, InputRequest& inputRequest,
    const type::CarrierCode& validatingCarrier,
    tse::FarePath* mainPath /* can be null */)
{
  InputItin* itin = new InputItin();
  inputRequest.itins().push_back(itin);
  itin->_id = inputRequest.itins().size() - 1;

  if (mainPath) // we are dealing with GSA (alternate validating cxr)
  {
    v2Mapping._farePathMap.push_back(
        FarePathLink(&tseFarePath, mainPath, validatingCarrier, itin->_id));
  }

  inputRequest.flightPaths().push_back(new InputFlightPath);
  itin->_flightPathRefId = inputRequest.flightPaths().size() - 1;

  InputFarePath* farePath = new InputFarePath();
  inputRequest.farePaths().push_back(farePath);
  farePath->_id = inputRequest.farePaths().size() - 1;

  InputGeoPath* geoPath = new InputGeoPath();
  inputRequest.geoPaths().push_back(geoPath);
  geoPath->_id = inputRequest.geoPaths().size() - 1;

  proccessChangeFee(tseFarePath, _activationStatus, _trx, inputRequest, *itin);

  TaxRequestBuilder::InputGeoMap inputGeoMap;

  const tax::ItinFarePathKey key = std::make_tuple(&tseItin, &tseFarePath,
      validatingCarrier, itin->_id);
  v2Mapping._itinFarePathMapping.push_back(key);

  processInputTseItin(_trx, tseItin, tseFarePath, inputRequest, *itin,
      *farePath, *geoPath, inputRequest.flightPaths().back(), inputGeoMap[key],
      validatingCarrier);

  addPassenger(_trx, _trx.getRequest()->ticketingDT(), _trx.getOptions(),
      *tseFarePath.paxType(), inputRequest.passengers());

  tax::ServicesFeesMapKey serviceFeesKey = std::make_tuple(&tseItin,
      &tseFarePath, validatingCarrier);
  tax::ServicesFeesMap::const_iterator it = servicesFees.find(serviceFeesKey);
  if (it != servicesFees.end())
  {
    addYqYrServicesFees(it->second, *itin, inputRequest, arunksBeforeSeg,
        hiddenBeforeSeg, tseItin.travelSeg());
  }

  if (taxUtil::isAnciliaryItin(_trx, tseItin))
  {
    OptionalServicesForOtaTaxReqBuilder optionalServicesForOtaTaxReqBuilder(
        _trx, inputRequest, *itin, tseFarePath);
    optionalServicesForOtaTaxReqBuilder.addOptionalServices();
  } else
  {
    OptionalServicesBuilder optionalServicesBuilder(_trx, inputRequest, *itin,
        inputGeoMap[key], tseFarePath, v2Mapping._optionalServicesRefs,
        v2Mapping._ocTaxInclIndMap);
    if (_activationStatus.isTaxOnOC())
    {
      optionalServicesBuilder.addOptionalServices();
    }

    if (_activationStatus.isTaxOnBaggage())
    {
      optionalServicesBuilder.addBaggage();
    }
  }
}

std::vector<type::Index> TaxRequestBuilder::computeArunksBeforeSeg(
    const std::vector<TravelSeg*>& segments)
{
  std::vector<type::Index> ans;
  ans.reserve(segments.size());
  type::Index count = 0;

  for (TravelSeg* seg : segments)
  {
    ans.push_back(count);
    if (!seg->isAir())
      ++count;
  }

  return ans;
}

std::vector<type::Index> TaxRequestBuilder::computeHiddenBeforeSeg(
    const std::vector<TravelSeg*>& segments)
{
  std::vector<type::Index> ans;
  ans.reserve(segments.size());
  type::Index count = 0;

  for (TravelSeg* seg : segments)
  {
    ans.push_back(count);
    count += seg->hiddenStops().size();
  }

  return ans;
}

InputRequest*
TaxRequestBuilder::buildInputRequest(const ServicesFeesMap& servicesFees,
    V2TrxMappingDetails& v2MappingDetails, std::ostream* gsaDiag)
{
  InputRequest* request = nullptr;
  _trx.dataHandle().get(request);

  process(servicesFees, v2MappingDetails, *request, gsaDiag);

  return request;
}

void TaxRequestBuilder::addYqYrServicesFees(
    const tax::ServicesFeesMapVal& items, tax::InputItin& itin,
    InputRequest& inputRequest, const std::vector<type::Index>& arunksBeforeSeg,
    const std::vector<type::Index>& hiddenBeforeSeg,
    const std::vector<TravelSeg*>& travelSegs)
{
  if (items.empty())
  {
    return;
  }

  // yqYrPath
  tax::InputYqYrPath* yqYrPath = new tax::InputYqYrPath;
  inputRequest.yqYrPaths().push_back(yqYrPath);
  yqYrPath->_id = inputRequest.yqYrPaths().size() - 1;
  yqYrPath->_totalAmount = 0;
  itin._yqYrPathRefId = yqYrPath->_id;

  tax::InputGeoPathMapping* geoPathMapping = new tax::InputGeoPathMapping;
  for (const tse::TaxItem* taxItem : items)
  {
    processYqYrTaxItem(*taxItem, inputRequest, *geoPathMapping, *yqYrPath,
        arunksBeforeSeg, hiddenBeforeSeg, travelSegs);
  }

  itin._yqYrPathGeoPathMappingRefId = addGeoPathMapping(geoPathMapping,
      inputRequest.geoPathMappings());
}

void TaxRequestBuilder::processYqYrTaxItem(const tse::TaxItem& taxItem,
    InputRequest& inputRequest, tax::InputGeoPathMapping& geoPathMapping,
    tax::InputYqYrPath& yqYrPath,
    const std::vector<type::Index>& arunksBeforeSeg,
    const std::vector<type::Index>& hiddenBeforeSeg,
    const std::vector<TravelSeg*>& travelSegs)
{
  if (FacadesUtils::isYqYr(taxItem.taxCode()))
  {
    // yqYr
    tax::type::Index yqyrId = addYqYr(taxItem, inputRequest.yqYrs());
    yqYrPath._totalAmount += inputRequest.yqYrs().at(yqyrId)._amount;

    // yqYrUsage
    tax::InputYqYrUsage* yqYrUsage = new tax::InputYqYrUsage;
    yqYrPath._yqYrUsages.push_back(yqYrUsage);
    yqYrUsage->_yqYrRefId = yqyrId;

    buildYqYrMappings(taxItem, geoPathMapping, arunksBeforeSeg, hiddenBeforeSeg,
        travelSegs);
    // TOTO analyzer geoPathMapping._mappings.size() == yqYrPath->_yqYrUsages.size()
  }
}

tax::type::Index TaxRequestBuilder::addYqYr(const tse::TaxItem& taxItem,
    InputRequest::InputYqYrs& yqYrs)
{
  tax::InputYqYr yqYr;
  yqYr._seqNo = taxItem.seqNo();
  yqYr._amount = tax::doubleToAmount(taxItem.taxAmount());
  yqYr._originalAmount = tax::doubleToAmount(taxItem.taxAmt());
  yqYr._originalCurrency = toTaxCurrencyCode(taxItem.taxCur());
  codeFromString(taxItem.carrierCode().substr(0, 2), yqYr._carrierCode);
  codeFromString(taxItem.taxCode().substr(0, 2), yqYr._code);
  yqYr._type = taxItem.taxCode()[2];
  yqYr._taxIncluded = taxItem.taxIncluded();

  tax::type::Index foundId = 0;
  if (FacadesUtils::findYqyr(yqYr, yqYrs, foundId))
    return foundId;

  yqYrs.push_back(new tax::InputYqYr(yqYr));
  yqYrs.back()._id = yqYrs.size() - 1;

  return yqYrs.back()._id;
}

void TaxRequestBuilder::processApplicableGroups(
    InputProcessingOptions& options)
{
  // TODO magic numbers
  // TODO make new phase for OB

  if (_activationStatus.isTaxOnItinYqYrTaxOnTax())
  {
    options.addApplicableGroup(makeGroup(3));
  }

  if (_activationStatus.isTaxOnChangeFee())
  {
    options.addApplicableGroup(makeGroup(1));
    options.addApplicableGroup(makeGroup(2));
  }

  if (_activationStatus.isTaxOnOC())
  {
    options.addApplicableGroup(makeGroup(0));
  }

  if (_activationStatus.isTaxOnBaggage())
  {
    options.addApplicableGroup(makeGroup(4));
  }
}

void TaxRequestBuilder::buildYqYrMappings(const tse::TaxItem& taxItem,
    tax::InputGeoPathMapping& geoPathMapping,
    const std::vector<type::Index>& arunksBeforeSeg,
    const std::vector<type::Index>& hiddenBeforeSeg,
    const std::vector<TravelSeg*>& travelSegs)
{
  tax::InputMapping* mapping = new tax::InputMapping;

  for (tax::type::Index id = taxItem.travelSegStartIndex();
      id <= taxItem.travelSegEndIndex(); ++id)
  {
    const TravelSeg* travelSeg = travelSegs[id];
    const tse::AirSeg* airSeg = travelSeg->toAirSeg();
    if (LIKELY(nullptr != airSeg))
    {
      tax::type::Index geoId = (id - arunksBeforeSeg[id] + hiddenBeforeSeg[id])
          * 2;

      // Departure
      mapping->maps().push_back(new tax::InputMap);
      mapping->maps().back()._geoRefId = geoId++;

      for (tax::type::Index i = 0; i < airSeg->hiddenStops().size(); ++i)
      {
        mapping->maps().push_back(new tax::InputMap);
        mapping->maps().back()._geoRefId = geoId++;
        mapping->maps().push_back(new tax::InputMap);
        mapping->maps().back()._geoRefId = geoId++;
      }

      // Arrival
      mapping->maps().push_back(new tax::InputMap);
      mapping->maps().back()._geoRefId = geoId++;
    }
  }

  geoPathMapping._mappings.push_back(mapping);
}

} // namespace tax
