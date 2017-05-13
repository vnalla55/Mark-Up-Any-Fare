//-------------------------------------------------------------------
//
//  File:           ERDFareComp.cpp
//  Created:        16 September 2008
//  Authors:        Konrad Koch
//
//  Description:    Wrapper to ERD section data from WPRD request.
//
//  Copyright Sabre 2008
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

#include "DataModel/ERDFareComp.h"

#include "Common/FareMarketUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/PaxTypeUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ERDFltSeg.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"

namespace tse
{

FALLBACK_DECL(fallbackWPRDASLFix);

void
ERDFareComp::updateFRRInfo(FareDisplayTrx& trx,
                           FareDisplayOptions& options,
                           FareDisplayRequest& request)
{
  if (fallback::fallbackWPRDASLFix(&trx))
    return;

  for (const HPUSection& hpuSection : _hpuInfo)
  {
    if ((hpuSection.markupType == "AJ") && !options.isPDOForFRRule())
    {
      options.setSourcePCCForASL(hpuSection.sourcePCC);
      request.calcFareAmount() = hpuSection.totalFareAmount;
    }

    if (hpuSection.markupType == "NT")
    {
      options.setSourcePCCForCat35(hpuSection.sourcePCC);
      // Note that in this case existing calcFareAmount is good.
    }
  }
}

void
ERDFareComp::select(FareDisplayTrx& trx)
{
  FareDisplayOptions* options = trx.getOptions();
  FareDisplayRequest* request = trx.getRequest();

  if (!options || !request)
    return;

  options->lineNumber() = 0; // This can be positive only for short RD

  // Fill common data from fare component
  storePassengers(*request);

  request->fareBasisCode() = _fareBasis;
  request->uniqueFareBasisCode() = _uniqueFareBasis;
  request->calcFareAmount() = _fareAmount;

  updateFRRInfo(trx, *options, *request);

  options->baseFareCurrency() = _fareCurrency;
  request->bookingCode() = _bookingCode;
  options->carrierCode() = _trueGoverningCarrier;
  trx.preferredCarriers().insert(_trueGoverningCarrier);

  if (!_validatingCarrier.empty())
  {
    trx.preferredCarriers().insert(_validatingCarrier);
  }

  options->fareTariff() = _fareTariff;
  options->vendorCode() = _vendor;
  options->ruleNumber() = _ruleNumber;
  options->linkNumber() = _linkNumber;
  options->sequenceNumber() = _sequenceNumber;
  options->createDate() = _createDate;
  options->createTime() = _createTime;
  options->fareType() = _fareType;
  options->fareClass() = _fareClass;
  options->FDcat35Type() = _cat35Type[0];
  options->isErdAfterCmdPricing() = _wasCommandPricing;
  request->accountCode() = _accountCode.c_str();
  request->ticketDesignator() = _ticketDesignator;

  if (_cat35Type == NET_QUALIFIER || _cat35Type == NET_QUALIFIER_WITH_UNIQUE_FBC)
  {
    request->inclusionCode() = FD_NET;
    request->requestedInclusionCode() = FD_NET;
  }

  // Check for Fare by Rule fare
  if (_cat25Values.isNonPublishedFare())
  {
    storeNonPublishedValues(_cat25Values, options->cat25Values(), *options);
  }

  // Check for Negotiated fare
  if (_cat35Values.isNonPublishedFare())
  {
    storeNonPublishedValues(_cat35Values, options->cat35Values(), *options);
  }

  // Check for Discounted fare
  if (_discountedValues.isNonPublishedFare())
  {
    storeNonPublishedValues(_discountedValues, options->discountedValues(), *options);
  }

  // Check for Constructed fare
  if (_constructedAttributes.isConstructedFare())
  {
    storeConstructedAttributes(*options);
  }

  //check rtw
  if (!_arrivalCity.empty() && _departureCity == _arrivalCity)
  {
    options->setRtw(true);
  }

  AirSeg* firstTvlSeg = dynamic_cast<AirSeg*>(trx.travelSeg().front());

  if (!firstTvlSeg)
    return;

  setTravelDates(firstTvlSeg);

  if ((_directionality == "FR" && !options->swappedDirectionality()) ||
      (_directionality == "TO" && options->swappedDirectionality()))
  {
    setMultiAirportCities(trx, firstTvlSeg, _departureAirport, _arrivalAirport);
  }
  else
  {
    setMultiAirportCities(trx, firstTvlSeg, _arrivalAirport, _departureAirport);
  }
}

//----------------------------------------------------------------------------
// ERDFareComp::setMultiAirportCities
//----------------------------------------------------------------------------
void
ERDFareComp::setMultiAirportCities(FareDisplayTrx& trx,
                                   AirSeg* airSeg,
                                   const LocCode& origAirport,
                                   const LocCode& destAirport)
{
  // Set origin and destination in the first segment of itinerary
  airSeg->origAirport() = origAirport;
  airSeg->destAirport() = destAirport;

  const Loc* origin = trx.dataHandle().getLoc(airSeg->origAirport(), DateTime::localTime());
  const Loc* destination = trx.dataHandle().getLoc(airSeg->destAirport(), DateTime::localTime());

  if (origin == nullptr || destination == nullptr)
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_CITY_AIRPORT_CODE);

  airSeg->origin() = origin;
  airSeg->destination() = destination;
  airSeg->carrier() = _trueGoverningCarrier;

  GeoTravelType geoType =
      LocUtil::isInternational(*(airSeg->origin()), *(airSeg->destination())) == true
          ? GeoTravelType::International
          : GeoTravelType::Domestic;

  airSeg->boardMultiCity() = FareMarketUtil::getMultiCity(
      airSeg->carrier(), airSeg->origAirport(), geoType, airSeg->departureDT());
  airSeg->offMultiCity() = FareMarketUtil::getMultiCity(
      airSeg->carrier(), airSeg->destAirport(), geoType, airSeg->departureDT());
}

//----------------------------------------------------------------------------
// ERDFareComp::setTravelDates
//----------------------------------------------------------------------------
void
ERDFareComp::setTravelDates(AirSeg* airSeg)
{
  // Set travel dates
  airSeg->departureDT() = _departureDT;
  airSeg->earliestDepartureDT() = _departureDT;
  airSeg->latestDepartureDT() = _departureDT;

  // Change time from 00:00:00 to 23:59:59
  airSeg->latestDepartureDT() = airSeg->latestDepartureDT().addSeconds(23 * 60 * 60 + 59 * 60 + 59);
  airSeg->earliestArrivalDT() = _departureDT;
  airSeg->latestArrivalDT() = _departureDT;

  // Change time from 00:00:00 to 23:59:59
  airSeg->latestArrivalDT() = airSeg->latestArrivalDT().addSeconds(23 * 60 * 60 + 59 * 60 + 59);
}

void
ERDFareComp::setAddonAttributes(AddOnAttributes& destAttrs, const AddOnAttributes& srcAttrs) const
{
  destAttrs.isAddOn() = true;
  destAttrs.addonFootNote1() = srcAttrs.addonFootNote1();
  destAttrs.addonFootNote2() = srcAttrs.addonFootNote2();
  destAttrs.addonFareClass() = srcAttrs.addonFareClass();
  destAttrs.addonTariff() = srcAttrs.addonTariff();
  destAttrs.addonRouting() = srcAttrs.addonRouting();
  destAttrs.addonAmount() = srcAttrs.addonAmount();
  destAttrs.addonCurrency() = srcAttrs.addonCurrency();
  destAttrs.oWRT() = srcAttrs.oWRT();
}

bool
ERDFareComp::MatchFareBasis::
operator()(const ERDFareComp* erdFareComp) const
{
  if (erdFareComp->uniqueFareBasis().length())
  {
    return _fareBasis == erdFareComp->uniqueFareBasis();
  }

  if (erdFareComp->cat35FareBasis().length() && _fareBasis == erdFareComp->cat35FareBasis())
  {
    return true;
  }

  return _fareBasis == erdFareComp->fareBasis();
}

bool
ERDFareComp::MatchConditionalFareBasis::
operator()(const ERDFareComp* erdFareComp) const
{
  if (erdFareComp->uniqueFareBasis().length())
  {
    return _fareBasis == erdFareComp->uniqueFareBasis() ||
           _fareBasis == erdFareComp->uniqueFareBasis().substr(0, erdFareComp->fareClassLength());
  }

  if (erdFareComp->cat35FareBasis().length() && _fareBasis == erdFareComp->cat35FareBasis())
  {
    return true;
  }

  return _fareBasis == erdFareComp->fareBasis() ||
         _fareBasis == erdFareComp->fareBasis().substr(0, erdFareComp->fareClassLength());
}

bool
ERDFareComp::MatchPaxTypeCode::
operator()(const ERDFareComp* erdFareComp) const
{
  boost::cmatch results;

  if (_paxType != erdFareComp->paxTypeCode() &&
      _paxType[0] == erdFareComp->paxTypeCode().c_str()[0] &&
      boost::regex_match(erdFareComp->paxTypeCode().c_str(), results, PaxTypeUtil::PAX_WITH_SPECIFIED_AGE) &&
      boost::regex_match(_paxType.c_str(), results, PaxTypeUtil::PAX_WITH_UNSPECIFIED_AGE))
  {
    return true;
  }
  return _paxType == erdFareComp->paxTypeCode();
}

bool
ERDFareComp::MatchSegmentNumbers::
operator()(const ERDFareComp* erdFareComp) const
{
  std::vector<uint16_t>::const_iterator snIter = _segmentNumbers.begin();
  std::vector<uint16_t>::const_iterator snEnd = _segmentNumbers.end();

  for (; snIter != snEnd; ++snIter)
  {
    std::vector<ERDFltSeg*>::const_iterator fsIterBegin = erdFareComp->segments().begin();
    std::vector<ERDFltSeg*>::const_iterator fsIterEnd = erdFareComp->segments().end();

    if (std::find_if(fsIterBegin, fsIterEnd, ERDFltSeg::MatchSegmentNumber(*snIter)) != fsIterEnd)
      return true;
  }
  return false;
}

//--------------------------------------------------------------------------
// ERDFareComp::completeFareBreak
//--------------------------------------------------------------------------
bool
ERDFareComp::completeFareBreak(const ERDFareComp& second)
{
  if (this->fareBreak() == TRUE_INDICATOR || second.fareBreak() == FALSE_INDICATOR ||
      this->pricingUnitNumber() != second.pricingUnitNumber() ||
      this->fareBasis() != second.fareBasis() ||
      this->trueGoverningCarrier() != second.trueGoverningCarrier() ||
      this->nucFareAmount() != second.nucFareAmount() ||
      this->directionality() != second.directionality() ||
      this->departureCity() != second.departureCity() ||
      this->arrivalCity() != second.arrivalCity())
  {
    return false;
  }

  this->_vendor = second._vendor;
  this->_fareTariff = second._fareTariff;
  this->_ruleNumber = second._ruleNumber;

  this->_fareBreak = TRUE_INDICATOR;

  return true;
}

//--------------------------------------------------------------------------
// ERDFareComp::storePassengers
//--------------------------------------------------------------------------
void
ERDFareComp::storePassengers(FareDisplayRequest& request) const
{
  request.displayPassengerTypes().push_back(_actualPaxTypeCode);
  request.displayPassengerTypes().push_back(_paxTypeCode);
  request.inputPassengerTypes().push_back(_actualPaxTypeCode);
  request.inputPassengerTypes().push_back(_paxTypeCode);
}

//--------------------------------------------------------------------------
// ERDFareComp::storeNonPublishedValues
//--------------------------------------------------------------------------
void
ERDFareComp::storeNonPublishedValues(const NonPublishedValues& source,
                                     NonPublishedValues& destination,
                                     FareDisplayOptions& options) const
{
  destination.isNonPublishedFare() = true;
  destination.vendorCode() = source.vendorCode();
  destination.itemNo() = source.itemNo();
  destination.createDate() = source.createDate();
  if (!source.directionality().empty() && source.directionality() != _directionality)
  {
    options.swappedDirectionality() = true;
  }
}

//--------------------------------------------------------------------------
// ERDFareComp::storeConstructedAttributes
//--------------------------------------------------------------------------
void
ERDFareComp::storeConstructedAttributes(FareDisplayOptions& options) const
{
  options.constructedAttributes().isConstructedFare() = true;
  options.constructedAttributes().gateway1() = _constructedAttributes.gateway1();
  options.constructedAttributes().gateway2() = _constructedAttributes.gateway2();
  options.constructedAttributes().constructionType() = _constructedAttributes.constructionType();
  options.constructedAttributes().specifiedFareAmount() =
      _constructedAttributes.specifiedFareAmount();
  options.constructedAttributes().constructedNucAmount() =
      _constructedAttributes.constructedNucAmount();

  // Check for origin addon
  if (_origAddonAttributes.isAddOn())
    setAddonAttributes(options.origAttributes(), _origAddonAttributes);

  // Check for destination addon
  if (_destAddonAttributes.isAddOn())
    setAddonAttributes(options.destAttributes(), _destAddonAttributes);
}
} //tse
