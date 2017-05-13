// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Common/TaxableUnitTagSet.h"
#include "DataModel/Common/CodeIO.h"
#include "Rules/AlternateRefAkHiFactorsRule.h"
#include "Rules/ApplicationTag01Rule.h"
#include "Rules/BaggageRulesBuilder.h"
#include "Rules/BlankLimitRule.h"
#include "Rules/BusinessRule.h"
#include "Rules/BusinessRulesContainer.h"
#include "Rules/CarrierFlightRule.h"
#include "Rules/ChangeFeeRulesBuilder.h"
#include "Rules/ConnectionsTagsRule.h"
#include "Rules/ContinuousJourneyLimitRule.h"
#include "Rules/ContinuousJourneyRule.h"
#include "Rules/CurrencyOfSaleRule.h"
#include "Rules/ExemptTagRule.h"
#include "Rules/FillTimeStopoversRule.h"
#include "Rules/FlatTaxRule.h"
#include "Rules/ItineraryRulesBuilder.h"
#include "Rules/JourneyIncludesRule.h"
#include "Rules/JourneyLoc1AsOriginExRule.h"
#include "Rules/JourneyLoc1AsOriginRule.h"
#include "Rules/JourneyLoc2DestinationTurnAroundRule.h"
#include "Rules/OBRulesBuilder.h"
#include "Rules/OnePerItinLimitRule.h"
#include "Rules/OptionalServicePointOfDeliveryRule.h"
#include "Rules/OptionalServiceRulesBuilder.h"
#include "Rules/OptionalServiceTagsRule.h"
#include "Rules/OutputTypeIndicatorRule.h"
#include "Rules/PassengerTypeCodeRule.h"
#include "Rules/PercentageTaxRule.h"
#include "Rules/PointOfSaleRule.h"
#include "Rules/PointOfTicketingRule.h"
#include "Rules/PointOverlappingForItineraryRule.h"
#include "Rules/PointOverlappingForYqYrRule.h"
#include "Rules/ReportingRecordRule.h"
#include "Rules/RequestLogicError.h"
#include "Rules/ReturnToOriginRule.h"
#include "Rules/RulesBuilder.h"
#include "Rules/RulesBuilder.h"
#include "Rules/SaleDateRule.h"
#include "Rules/SectorDetailRule.h"
#include "Rules/ServiceBaggageRule.h"
#include "Rules/ServiceFeeSecurityRule.h"
#include "Rules/SingleJourneyLimitRule.h"
#include "Rules/SpecialTaxProcessingRule.h"
#include "Rules/TaxCodeConversionRule.h"
#include "Rules/TaxMatchingApplTagRule.h"
#include "Rules/TaxMinMaxValueRule.h"
#include "Rules/TaxOnChangeFeeRule.h"
#include "Rules/TaxOnFareRule.h"
#include "Rules/TaxOnOptionalServiceRule.h"
#include "Rules/TaxOnTaxRule.h"
#include "Rules/TaxOnTicketingFeeRule.h"
#include "Rules/TaxOnYqYrRule.h"
#include "Rules/TaxPointLoc1InternationalDomesticRule.h"
#include "Rules/TaxPointLoc1Rule.h"
#include "Rules/TaxPointLoc1StopoverTagRule.h"
#include "Rules/TaxPointLoc1TransferTypeRule.h"
#include "Rules/TaxPointLoc2CompareRule.h"
#include "Rules/TaxPointLoc2InternationalDomesticRule.h"
#include "Rules/TaxPointLoc2Rule.h"
#include "Rules/TaxPointLoc2StopoverTagRule.h"
#include "Rules/TaxPointLoc3AsNextStopoverRule.h"
#include "Rules/TaxPointLoc3AsPreviousPointRule.h"
#include "Rules/TaxRoundingOCRule.h"
#include "Rules/TaxRoundingRule.h"
#include "Rules/ThirdPartyTagRule.h"
#include "Rules/TicketedPointRule.h"
#include "Rules/TicketMinMaxValueOCRule.h"
#include "Rules/TicketMinMaxValueRule.h"
#include "Rules/TravelDatesRule.h"
#include "Rules/TravelWhollyWithinRule.h"
#include "Rules/UsOneWayAndRoundTripLimitRule.h"
#include "Rules/ValidatingCarrierRule.h"
#include "Rules/YqYrAmountRule.h"

namespace tax
{

namespace
{
static const type::TaxProcessingApplTag taxProcessingApplTag01_alternateRTOJLogic{"01"};
static const type::TaxProcessingApplTag taxProcessingApplTag02_alternateTurnaroundLogic{"02"};
static const type::TaxProcessingApplTag taxProcessingApplTag05_roundToFives{"05"};
static const type::TaxProcessingApplTag taxProcessingApplTag09_surfaceNotAlwaysStop{"09"};

static const type::TaxMatchingApplTag taxMatchingApplTag06_fareBreakMustBeStop{"06"};
static const type::TaxMatchingApplTag taxMatchingApplTag08_jrnyIncludesMustBeStop{"08"};

// Sabre custom processing codes
static const type::TaxProcessingApplTag taxProcessingApplTag55_TCH{"55"};
static const type::TaxProcessingApplTag taxProcessingApplTag56_jrnyLoc1OriginEx{"56"};
static const type::TaxProcessingApplTag taxProcessingApplTag57_customerRestriction{"57"};
static const type::TaxProcessingApplTag taxProcessingApplTag58_parentTaxes{"58"};

type::Date
buildDate(int16_t year, int16_t month, int16_t day)
{
  if ((year == 0) && (month == 0) && (day == 0))
  {
    return type::Date::blank_date();
  }

  if (year == tax::BLANK_YEAR)
  {
    if (month <= 0 && day <= 0)
    {
      return type::Date::blank_date();
    }
    if (month <= 0 || day <= 0)
    {
      return type::Date::invalid_date();
    }
    return type::Date(month, day);
  }
  if (year == 99 && month == 99 && day == 99)
  {
    return type::Date::pos_infinity();
  }
  if (month <= 0 || day <= 0)
  {
    return type::Date::invalid_date();
  }
  if (year < 2000)
  {
    year = int16_t(year + 2000);
  }

  return type::Date(year, month, day);
}

bool
filteredTaxableAmount(type::ServiceBaggageApplTag const& serviceBaggageApplTag,
                      type::PercentFlatTag const& percentFlatTag,
                      type::TaxAppliesToTagInd const& taxAppliesToTagInd)
{
  if (serviceBaggageApplTag == type::ServiceBaggageApplTag::C) // non-fitered
    return false;

  if (taxAppliesToTagInd != type::TaxAppliesToTagInd::Blank &&
      percentFlatTag == type::PercentFlatTag::Percent)
    return true;

  if (taxAppliesToTagInd == type::TaxAppliesToTagInd::Blank &&
      percentFlatTag == type::PercentFlatTag::Flat)
    return true;

  return false;
}

bool
unfilteredTaxableAmount(type::ServiceBaggageApplTag const& serviceBaggageApplTag,
                        type::PercentFlatTag const& percentFlatTag,
                        type::TaxAppliesToTagInd const& taxAppliesToTagInd)
{
  if (serviceBaggageApplTag != type::ServiceBaggageApplTag::E)
    return false;

  if (taxAppliesToTagInd != type::TaxAppliesToTagInd::Blank &&
      percentFlatTag == type::PercentFlatTag::Percent)
    return true;

  if (taxAppliesToTagInd == type::TaxAppliesToTagInd::Blank &&
      percentFlatTag == type::PercentFlatTag::Flat)
    return true;

  return false;
}

} // namespace

void
RulesBuilder::buildRules(const RulesRecord& rulesRecord,
                         const TaxableUnitTagSet& taxableUnits,
                         ValidatorsGroups& validatorsGroups,
                         LimitGroup& limitGroup,
                         CalculatorsGroups& calculatorsGroups)
{
  if (taxableUnits.hasTicketingFeeTags())
  {
    buildRules<OBRulesBuilder>(
        rulesRecord, taxableUnits, validatorsGroups, limitGroup, calculatorsGroups);
  }
  else if (taxableUnits.hasBaggageTags())
  {
    buildRules<BaggageRulesBuilder>(
        rulesRecord, taxableUnits, validatorsGroups, limitGroup, calculatorsGroups);
  }
  else if (taxableUnits.hasAncillaryTags())
  {
    buildRules<OptionalServiceRulesBuilder>(
        rulesRecord, taxableUnits, validatorsGroups, limitGroup, calculatorsGroups);
  }
  else if (taxableUnits.hasChangeFeeTags())
  {
    buildRules<ChangeFeeRulesBuilder>(
        rulesRecord, taxableUnits, validatorsGroups, limitGroup, calculatorsGroups);
  }
  else
  {
    buildRules<ItineraryRulesBuilder>(
        rulesRecord, taxableUnits, validatorsGroups, limitGroup, calculatorsGroups);
  }
}

RulesBuilder::RulesBuilder(const RulesRecord& rulesRecord, const TaxableUnitTagSet& taxableUnits)
  : _rulesRecord(rulesRecord), _taxableUnits(taxableUnits)
{
}

void
RulesBuilder::addLimiters()
{
  buildTaxApplicationLimitRule();
}

void
RulesBuilder::buildSpecialTaxProcessingRule()
{
  if (_rulesRecord.taxProcessingApplTag == taxProcessingApplTag55_TCH)
  {
    _validatorsGroups._processingOptionGroup._specialTaxProcessingRule =
        SpecialTaxProcessingRule(_rulesRecord.taxProcessingApplTag);
  }
}

void
RulesBuilder::buildOutputTypeIndicatorRule()
{
  _validatorsGroups._processingOptionGroup._outputTypeIndicatorRule =
      OutputTypeIndicatorRule(_rulesRecord.outputTypeIndicator);
}

void
RulesBuilder::buildPointOverlappingRule()
{
  _validatorsGroups._processingOptionGroup._pointOverlappingForItineraryRule =
      PointOverlappingForItineraryRule();
  _validatorsGroups._processingOptionGroup._pointOverlappingForYqYrRule =
      PointOverlappingForYqYrRule();
}

void
RulesBuilder::buildCurrencyOfSaleRule()
{
  const type::CurrencyCode& currencyOfSale = _rulesRecord.currencyOfSale;
  if (!currencyOfSale.empty())
    _validatorsGroups._ticketGroup._currencyOfSaleRule = CurrencyOfSaleRule(currencyOfSale);
}

void
RulesBuilder::buildCustomerRestrictionRule()
{
  if (_rulesRecord.taxProcessingApplTag != taxProcessingApplTag57_customerRestriction)
    return;

  const type::CarrierCode& carrierCode = _rulesRecord.taxName.taxCarrier();
  if (carrierCode.empty())
    return;

  if (carrierCode == "YY")
    return;

  _validatorsGroups._ticketGroup._customerRestrictionRule = CustomerRestrictionRule(carrierCode);
}

void
RulesBuilder::buildPreviousTicketRule()
{
  if (_rulesRecord.taxProcessingApplTag == taxProcessingApplTag58_parentTaxes)
    _validatorsGroups._miscGroup._previousTicketRule =
        PreviousTicketRule(_rulesRecord.taxPercent);
}

void
RulesBuilder::buildThirdPartyTagRule()
{
  if (_rulesRecord.paidBy3rdPartyTag != type::PaidBy3rdPartyTag::Blank)
  {
    _validatorsGroups._ticketGroup._thirdPartyTagRule =
        ThirdPartyTagRule(_rulesRecord.paidBy3rdPartyTag);
  }
}

void
RulesBuilder::buildPointOfTicketingRule()
{
  if (_rulesRecord.pointOfTicketing.type() != type::LocType::Blank)
  {
    _validatorsGroups._ticketGroup._pointOfTicketingRule =
        PointOfTicketingRule(_rulesRecord.pointOfTicketing, _rulesRecord.vendor);
  }
}

void
RulesBuilder::buildSaleDateRule()
{
  type::Date effDate = _rulesRecord.effDate;
  type::Date discDate = _rulesRecord.discDate;
  type::Date histEffDate = _rulesRecord.histSaleEffDate;
  type::Date histDiscDate = _rulesRecord.histSaleDiscDate;
  type::Timestamp expireDate = _rulesRecord.expiredDate;

  type::Date eff(effDate);

  if (discDate.is_blank_date())
  {
    discDate = type::Date::pos_infinity();
  }
  else
  {
    discDate = discDate.advance(1);
  }
  if (histDiscDate.is_blank_date())
  {
    histDiscDate = type::Date::pos_infinity();
  }

  type::Timestamp disc(discDate, type::Time(0, 0));

  if (!histEffDate.is_blank_date() && !effDate.is_blank_date() && histEffDate < effDate)
  {
    eff = histEffDate;
  }

  if (histDiscDate < discDate)
  {
    disc.date() = histDiscDate;
  }

  if (!expireDate.date().is_blank_date() && expireDate < disc)
  {
    disc = expireDate;
  }

  if ((!eff.is_blank_date() && !eff.is_neg_infinity()) ||
      (!disc.date().is_blank_date() && !disc.date().is_pos_infinity()))
  {
    _validatorsGroups._ticketGroup._saleDateRule = SaleDateRule(eff, disc);
  }
}

void
RulesBuilder::buildServiceFeeSecurityRule()
{
  if (_rulesRecord.svcFeesSecurityItemNo != 0)
  {
    _validatorsGroups._ticketGroup._serviceFeeSecurityRule =
        ServiceFeeSecurityRule(_rulesRecord.vendor, _rulesRecord.svcFeesSecurityItemNo);
  }
}

void
RulesBuilder::buildPointOfSaleRule()
{
  if (_rulesRecord.pointOfSale.type() != type::LocType::Blank)
  {
    _validatorsGroups._saleGroup._pointOfSaleRule =
        PointOfSaleRule(_rulesRecord.pointOfSale, _rulesRecord.vendor);
  }
}

void
RulesBuilder::buildFillTimeStopoversRule()
{
  if (!_rulesRecord.stopoverTimeTag.empty() ||
      _rulesRecord.stopoverTimeUnit != type::StopoverTimeUnit::Blank)
  {
    _validatorsGroups._fillerGroup._fillTimeStopoversRule =
        FillTimeStopoversRule(_rulesRecord.stopoverTimeTag, _rulesRecord.stopoverTimeUnit);
  }
}

void
RulesBuilder::buildTicketedPointRule()
{
  if (_rulesRecord.ticketedPointTag != type::TicketedPointTag::MatchTicketedAndUnticketedPoints)
  {
    _validatorsGroups._fillerGroup._ticketedPointRule = TicketedPointRule();
  }
}

void
RulesBuilder::buildConnectionsTagsRule()
{
  bool caSurfaceException = (_rulesRecord.taxName.taxCode() == "CA" &&
                             _rulesRecord.taxProcessingApplTag == taxProcessingApplTag09_surfaceNotAlwaysStop);
  if (!_rulesRecord.connectionsTags.empty() ||
      caSurfaceException)
  {
    _validatorsGroups._fillerGroup._connectionsTagsRule =
        ConnectionsTagsRule(_rulesRecord.connectionsTags,
                            _rulesRecord.taxProcessingApplTag == taxProcessingApplTag02_alternateTurnaroundLogic,
                            caSurfaceException);
  }
}

void
RulesBuilder::buildJourneyLoc1AsOriginRule()
{
  if (_rulesRecord.jrnyLocZone1.type() != type::LocType::Blank
      && _rulesRecord.taxProcessingApplTag != taxProcessingApplTag56_jrnyLoc1OriginEx)
  {
    _validatorsGroups._geoPathGroup._journeyLoc1AsOriginRule =
        JourneyLoc1AsOriginRule(_rulesRecord.jrnyLocZone1, _rulesRecord.vendor);
  }
}

void
RulesBuilder::buildJourneyLoc1AsOriginExRule()
{
  if (_rulesRecord.jrnyLocZone1.type() != type::LocType::Blank
      && _rulesRecord.taxProcessingApplTag == taxProcessingApplTag56_jrnyLoc1OriginEx)
  {
    //TODO use base class _journeyLoc1AsOriginRule, then call the reference
    _validatorsGroups._geoPathGroup._journeyLoc1AsOriginExRule =
        JourneyLoc1AsOriginExRule(_rulesRecord.jrnyLocZone1, _rulesRecord.vendor);
  }
}

void
RulesBuilder::buildApplicationTagRule()
{
  const TaxName& taxName = _rulesRecord.taxName;

  if (_rulesRecord.jrnyLocZone2.type() != type::LocType::Blank &&
      _rulesRecord.taxProcessingApplTag == taxProcessingApplTag01_alternateRTOJLogic &&
      taxName.nation() == "US" &&
      taxName.taxCode() == "US" &&
      (taxName.taxType() == "005" || taxName.taxType() == "006"))
  {
    _validatorsGroups._fillerGroup._applicationTagRule = ApplicationTag01Rule(_rulesRecord.vendor);
  }
}

void
RulesBuilder::buildJourneyLoc2DestinationTurnAroundRule()
{
  if (_rulesRecord.jrnyLocZone2.type() != type::LocType::Blank)
  {
    _validatorsGroups._geoPathGroup._journeyLoc2DestinationTurnAroundRule =
        JourneyLoc2DestinationTurnAroundRule(_rulesRecord.jrnyInd,
                                             _rulesRecord.jrnyLocZone2,
                                             _rulesRecord.vendor,
                                             _rulesRecord.taxProcessingApplTag == taxProcessingApplTag02_alternateTurnaroundLogic);
  }
}

void
RulesBuilder::buildTravelWhollyWithinRule()
{
  if (_rulesRecord.trvlWhollyWithin.type() != type::LocType::Blank)
  {
    _validatorsGroups._geoPathGroup._travelWhollyWithinRule = TravelWhollyWithinRule(
        _rulesRecord.ticketedPointTag, _rulesRecord.trvlWhollyWithin, _rulesRecord.vendor);
  }
}

void
RulesBuilder::buildJourneyIncludes()
{
  if (_rulesRecord.jrnyIncludes.type() != type::LocType::Blank)
  {
    _validatorsGroups._geoPathGroup._journeyIncludesRule = JourneyIncludesRule(
        _rulesRecord.ticketedPointTag,
        _rulesRecord.jrnyIncludes,
        _rulesRecord.vendor,
        _rulesRecord.taxMatchingApplTag == taxMatchingApplTag08_jrnyIncludesMustBeStop);
  }
}

void
RulesBuilder::buildReturnToOriginRule()
{
  if (_rulesRecord.rtnToOrig == type::RtnToOrig::ReturnToOrigin ||
      _rulesRecord.rtnToOrig == type::RtnToOrig::NotReturnToOrigin)
  {
    _validatorsGroups._geoPathGroup._returnToOriginRule =
        ReturnToOriginRule(_rulesRecord.rtnToOrig);
  }
}

void
RulesBuilder::buildContinuousJourneyRule()
{
  if (_rulesRecord.rtnToOrig == type::RtnToOrig::ContinuousJourney1stPoint ||
      _rulesRecord.rtnToOrig == type::RtnToOrig::ContinuousJourney2ndPoint)
  {
    _validatorsGroups._geoPathGroup._continuousJourneyRule =
        ContinuousJourneyRule(_rulesRecord.rtnToOrig);
  }
}

void
RulesBuilder::buildOptionalServiceTagsRule()
{
  _validatorsGroups._optionalServiceGroup._optionalServiceTagsRule =
      OptionalServiceTagsRule(_taxableUnits);
}

void
RulesBuilder::buildOptionalServicePointOfDeliveryRule()
{
  if (_rulesRecord.pointOfDelivery.type() != type::LocType::Blank)
  {
    _validatorsGroups._optionalServiceGroup._optionalServicePointOfDeliveryRule =
        OptionalServicePointOfDeliveryRule(_rulesRecord.pointOfDelivery, _rulesRecord.vendor);
  }
}

void
RulesBuilder::buildTaxPointLoc1Rule()
{
  if (_rulesRecord.taxPointLocZone1.type() != type::LocType::Blank)
  {
    _validatorsGroups._fillerGroup._taxPointLoc1Rule =
        TaxPointLoc1Rule(_rulesRecord.taxPointLocZone1, _rulesRecord.vendor);
  }
}

void
RulesBuilder::buildTaxPointLoc1TransferTypeRule()
{
  _validatorsGroups._taxPointBeginGroup._taxPointLoc1TransferTypeRule =
      TaxPointLoc1TransferTypeRule(_rulesRecord.taxPointLoc1TransferType);
}

void
RulesBuilder::buildTaxPointLoc1StopoverTagRule()
{
  if (_rulesRecord.taxPointLoc1StopoverTag != type::StopoverTag::Blank)
  {
    _validatorsGroups._taxPointBeginGroup._taxPointLoc1StopoverTagRule =
        TaxPointLoc1StopoverTagRule(_rulesRecord.taxPointLoc1StopoverTag,
                                    _rulesRecord.ticketedPointTag,
                                    _rulesRecord.taxMatchingApplTag == taxMatchingApplTag06_fareBreakMustBeStop);
  }
}

void
RulesBuilder::buildTaxPointLoc1InternationalDomesticRule()
{
  const type::AdjacentIntlDomInd& adjacentIntlDomInd = _rulesRecord.taxPointLoc1IntlDomInd;

  if (adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentStopoverInternational ||
      adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentStopoverDomestic ||
      adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentDomestic ||
      adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentInternational)
  {
    _validatorsGroups._taxPointBeginGroup._taxPointLoc1InternationalDomesticRule =
        TaxPointLoc1InternationalDomesticRule(adjacentIntlDomInd, _rulesRecord.ticketedPointTag);
  }
}

void
RulesBuilder::buildTaxPointLoc2StopoverTagRule()
{
  if (_rulesRecord.taxPointLoc2StopoverTag != type::Loc2StopoverTag::Blank &&
      _rulesRecord.taxPointLoc3GeoType != type::TaxPointLoc3GeoType::Stopover)
  {
    _validatorsGroups._taxPointEndGroup._taxPointLoc2StopoverTagRule =
        TaxPointLoc2StopoverTagRule(
          _rulesRecord.taxPointLoc2StopoverTag,
          _rulesRecord.taxMatchingApplTag,
          _rulesRecord.taxName.taxPointTag(),
          _rulesRecord.taxProcessingApplTag,
          _rulesRecord.taxPointLocZone1,
          _rulesRecord.taxPointLocZone2);
  }
}

void
RulesBuilder::buildTaxPointLoc2Rule()
{
  if (_rulesRecord.taxPointLoc2StopoverTag != type::Loc2StopoverTag::Blank ||
      _rulesRecord.taxPointLocZone2.type() != type::LocType::Blank)
  {
    _validatorsGroups._taxPointEndGroup._taxPointLoc2Rule =
        TaxPointLoc2Rule(_rulesRecord.taxPointLocZone2, _rulesRecord.vendor);
  }
}

void
RulesBuilder::buildTaxPointLoc2InternationalDomesticRule()
{
  const type::IntlDomInd& intlDomInd = _rulesRecord.taxPointLoc2IntlDomInd;

  if (intlDomInd == type::IntlDomInd::International || intlDomInd == type::IntlDomInd::Domestic)
  {
    _validatorsGroups._taxPointEndGroup._taxPointLoc2InternationalDomesticRule =
        TaxPointLoc2InternationalDomesticRule(intlDomInd, _rulesRecord.ticketedPointTag);
  }
}

void
RulesBuilder::buildTaxLoc2CompareRule()
{
  const type::TaxPointLoc2Compare& taxPointLoc2Compare = _rulesRecord.taxPointLoc2Compare;

  if (taxPointLoc2Compare != type::TaxPointLoc2Compare::Blank)
  {
    _validatorsGroups._taxPointEndGroup._taxPointLoc2CompareRule =
        TaxPointLoc2CompareRule(taxPointLoc2Compare, _rulesRecord.ticketedPointTag);
  }
}

void
RulesBuilder::buildTaxPointLoc3GeoRule()
{
  const type::TaxPointLoc3GeoType& taxPointLoc3GeoType = _rulesRecord.taxPointLoc3GeoType;
  const LocZone& taxPointLocZone3 = _rulesRecord.taxPointLocZone3;
  const type::Vendor& vendor = _rulesRecord.vendor;

  if (taxPointLoc3GeoType == type::TaxPointLoc3GeoType::Stopover)
  {
    _validatorsGroups._taxPointEndGroup._taxPointLoc3AsNextStopoverRule =
        TaxPointLoc3AsNextStopoverRule(taxPointLocZone3, vendor);
  }
  else if (taxPointLoc3GeoType == type::TaxPointLoc3GeoType::Point)
  {
    _validatorsGroups._taxPointEndGroup._taxPointLoc3AsPreviousPointRule =
        TaxPointLoc3AsPreviousPointRule(taxPointLocZone3, vendor);
  }
}

void
RulesBuilder::buildTravelDatesRule()
{
  if (_rulesRecord.travelDateTag == type::TravelDateAppTag::Blank)
    return;

  type::Date first(buildDate(
      _rulesRecord.firstTravelYear, _rulesRecord.firstTravelMonth, _rulesRecord.firstTravelDay));
  type::Date last(buildDate(
      _rulesRecord.lastTravelYear, _rulesRecord.lastTravelMonth, _rulesRecord.lastTravelDay));

  if (first.is_invalid())
  {
    throw RequestLogicError() << "Rules record " << _rulesRecord.taxName.nation()
                              << _rulesRecord.taxName.taxCode() << _rulesRecord.taxName.taxType()
                              << ": invalid TVLFIRST... date!";
  }

  if (last.is_invalid())
  {
    throw RequestLogicError() << "Rules record " << _rulesRecord.taxName.nation()
                              << _rulesRecord.taxName.taxCode() << _rulesRecord.taxName.taxType()
                              << ": invalid TVLLAST... date!";
  }
  if (first.is_pos_infinity())
  {
    first = type::Date::neg_infinity();
  }

  type::Date const& histFirst = _rulesRecord.histTrvlEffDate;
  type::Date const& histLast = _rulesRecord.histTrvlDiscDate;

  if (!histFirst.is_blank_date() && !first.is_blank_date() && histFirst < first)
  {
    first = histFirst;
  }

  if (!histLast.is_blank_date() && !last.is_blank_date() && histLast < last)
  {
    last = histLast;
  }

  if ((first.is_blank_date() || first.is_neg_infinity()) &&
      (last.is_blank_date() || last.is_pos_infinity()))
  {
    return;
  }

  if (_rulesRecord.travelDateTag == type::TravelDateAppTag::Journey)
  {
    _validatorsGroups._itinGroup._travelDatesJourneyRule = TravelDatesJourneyRule(first, last);
  }
  else
  {
    _validatorsGroups._itinGroup._travelDatesTaxPointRule = TravelDatesTaxPointRule(first, last);
  }
}

void
RulesBuilder::buildValidatingCarrierRule()
{
  if (_rulesRecord.carrierApplicationItem != 0)
  {
    _validatorsGroups._itinGroup._validatingCarrierRule =
        ValidatingCarrierRule(_rulesRecord.carrierApplicationItem, _rulesRecord.vendor);
  }
}

void
RulesBuilder::buildCarrierFlightRule()
{
  type::Index const& carrierFlightItemBefore = _rulesRecord.carrierFlightItemBefore;
  type::Index const& carrierFlightItemAfter = _rulesRecord.carrierFlightItemAfter;
  type::TaxPointTag const& taxPointTag = _rulesRecord.taxName.taxPointTag();
  type::Vendor const& vendor = _rulesRecord.vendor;

  if (carrierFlightItemBefore != 0 || carrierFlightItemAfter != 0)
  {
    _validatorsGroups._itinGroup._carrierFlightRule =
        CarrierFlightRule(carrierFlightItemBefore, carrierFlightItemAfter, taxPointTag, vendor);
  }
}

void
RulesBuilder::buildPassengerTypeCodeRule()
{
  if (_rulesRecord.passengerTypeCodeItem != 0)
  {
    _validatorsGroups._itinGroup._passengerTypeCodeRule =
        PassengerTypeCodeRule(_rulesRecord.vendor,
                              _rulesRecord.passengerTypeCodeItem,
                              _rulesRecord.taxName.taxRemittanceId());
  }
}

void
RulesBuilder::buildSectorDetailRule()
{
  const type::SectorDetailApplTag& sectorDetailApplTag = _rulesRecord.sectorDetailApplTag;
  const type::Index& sectorDetailItemNo = _rulesRecord.sectorDetailItemNo;
  const type::Vendor& vendor = _rulesRecord.vendor;

  if (sectorDetailApplTag != type::SectorDetailApplTag::Blank && sectorDetailItemNo != 0 &&
      !vendor.empty())
  {
    _validatorsGroups._miscGroup._sectorDetailRule =
        SectorDetailRule(sectorDetailApplTag, sectorDetailItemNo, vendor);
  }
}

void
RulesBuilder::buildServiceBaggageRule()
{
  if (_rulesRecord.serviceBaggageApplTag == type::ServiceBaggageApplTag::C &&
      _rulesRecord.serviceBaggageItemNo != 0)
  {
    _validatorsGroups._miscGroup._serviceBaggageRule =
        ServiceBaggageRule(_rulesRecord.serviceBaggageItemNo, _rulesRecord.vendor);
  }
}

void
RulesBuilder::buildTicketMinMaxValueRule()
{
  if (_rulesRecord.tktValApplQualifier != type::TktValApplQualifier::Blank &&
      !_rulesRecord.tktValCurrency.empty())
  {
    _validatorsGroups._miscGroup._ticketMinMaxValueRule =
        TicketMinMaxValueRule(_rulesRecord.tktValApplQualifier,
                              _rulesRecord.tktValCurrency,
                              _rulesRecord.tktValMin,
                              _rulesRecord.tktValMax,
                              _rulesRecord.tktValCurrDecimals);
  }
}

void
RulesBuilder::buildTicketMinMaxValueOCRule()
{
  if (_rulesRecord.tktValApplQualifier != type::TktValApplQualifier::Blank &&
      !_rulesRecord.tktValCurrency.empty())
  {
    _validatorsGroups._miscGroup._ticketMinMaxValueOCRule =
        TicketMinMaxValueOCRule(_rulesRecord.tktValApplQualifier,
                                _rulesRecord.tktValCurrency,
                                _rulesRecord.tktValMin,
                                _rulesRecord.tktValMax,
                                _rulesRecord.tktValCurrDecimals);
  }
}

void
RulesBuilder::buildTaxOnTaxRule()
{
  if (_taxableUnits.hasTag(type::TaxableUnit::TaxOnTax))
  {
    if (filteredTaxableAmount(_rulesRecord.serviceBaggageApplTag,
                              _rulesRecord.taxName.percentFlatTag(),
                              _rulesRecord.taxAppliesToTagInd))
    {
      _validatorsGroups._miscGroup._taxOnTaxRule =
          TaxOnTaxRule(_rulesRecord.serviceBaggageItemNo, _rulesRecord.vendor);
    }
  }
}

void
RulesBuilder::buildTaxOnYqYrRule()
{
  if (_taxableUnits.hasTag(type::TaxableUnit::YqYr))
  {
    if (filteredTaxableAmount(_rulesRecord.serviceBaggageApplTag,
                              _rulesRecord.taxName.percentFlatTag(),
                              _rulesRecord.taxAppliesToTagInd))
    {
      _validatorsGroups._miscGroup._taxOnYqYrRule = TaxOnYqYrRule(
          _rulesRecord.serviceBaggageItemNo, _rulesRecord.vendor, _rulesRecord.taxAppliesToTagInd);
    }
  }
}

void
RulesBuilder::buildYqYrAmountRule()
{
  if (_taxableUnits.hasTag(type::TaxableUnit::YqYr))
  {
    if (filteredTaxableAmount(_rulesRecord.serviceBaggageApplTag,
                              _rulesRecord.taxName.percentFlatTag(),
                              _rulesRecord.taxAppliesToTagInd))
    {
      _calculatorsGroups._applyGroup._yqYrAmountRule = YqYrAmountRule();
    }
  }
}

void
RulesBuilder::buildTaxOnOptionalServiceRule()
{
  bool anyOptionalService = _taxableUnits.hasTag(type::TaxableUnit::OCFlightRelated) ||
                            _taxableUnits.hasTag(type::TaxableUnit::OCTicketRelated) ||
                            _taxableUnits.hasTag(type::TaxableUnit::OCMerchandise) ||
                            _taxableUnits.hasTag(type::TaxableUnit::OCFareRelated) ||
                            _taxableUnits.hasTag(type::TaxableUnit::BaggageCharge);

  if (anyOptionalService)
  {
    if (unfilteredTaxableAmount(_rulesRecord.serviceBaggageApplTag,
                                _rulesRecord.taxName.percentFlatTag(),
                                _rulesRecord.taxAppliesToTagInd))
    {
      _validatorsGroups._miscGroup._taxOnOptionalServiceRule =
          TaxOnOptionalServiceRule(_rulesRecord.serviceBaggageItemNo, _rulesRecord.vendor);
    }
  }
}

void
RulesBuilder::buildFlatTaxRule()
{
  const type::PercentFlatTag& percentFlatTag = _rulesRecord.taxName.percentFlatTag();
  const type::MoneyAmount& taxAmt = _rulesRecord.taxAmt;
  const type::CurrencyCode& taxCurrency = _rulesRecord.taxCurrency;
  const TaxableUnitTagSet& applicableTaxableUnits = _taxableUnits;

  if (percentFlatTag != type::PercentFlatTag::Percent && taxAmt != 0 && !taxCurrency.empty())
  {
    _calculatorsGroups._applyGroup._flatTaxRule = FlatTaxRule(applicableTaxableUnits);
  }
}

void
RulesBuilder::buildAlternateRefAkHiFactorsRule()
{
  const int16_t& alternateRuleRefTag = _rulesRecord.alternateRuleRefTag;
  if (alternateRuleRefTag == 1)
  {
    _validatorsGroups._miscGroup._alternateRefAkHiFactorsRule = AlternateRefAkHiFactorsRule();
  }
}

void
RulesBuilder::buildTaxOnFareRule()
{
  if (_taxableUnits.hasTag(type::TaxableUnit::Itinerary))
  {
    _validatorsGroups._miscGroup._taxOnFareRule = TaxOnFareRule(_rulesRecord.netRemitApplTag,
                                                                _rulesRecord.taxAppliesToTagInd,
                                                                _rulesRecord.sectorDetailItemNo,
                                                                _rulesRecord.vendor,
                                                                _rulesRecord.taxProcessingApplTag);
  }
}

void
RulesBuilder::buildPercentageTaxRule()
{
  if (_rulesRecord.taxName.percentFlatTag() != type::PercentFlatTag::Flat)
  {
    _calculatorsGroups._applyGroup._percentageTaxRule =
        PercentageTaxRule(_rulesRecord.taxPercent,
                          _rulesRecord.taxCurrency,
                          _taxableUnits,
                          _rulesRecord.serviceBaggageItemNo,
                          _rulesRecord.vendor,
                          _rulesRecord.serviceBaggageApplTag);
  }
}

void
RulesBuilder::buildTaxMinMaxValueRule()
{
  if (!_rulesRecord.minMaxCurrency.empty())
  {
    _calculatorsGroups._applyGroup._taxMinMaxValueRule =
        TaxMinMaxValueRule(_rulesRecord.minMaxCurrency,
                           _rulesRecord.minTax,
                           _rulesRecord.maxTax,
                           _rulesRecord.minMaxDecimals);
  }
}

namespace
{
bool
isLimitValid(const type::TaxApplicationLimit& limit)
{
  return limit == type::TaxApplicationLimit::OnceForItin ||
         limit == type::TaxApplicationLimit::FirstTwoPerContinuousJourney ||
         limit == type::TaxApplicationLimit::OncePerSingleJourney ||
         limit == type::TaxApplicationLimit::FirstTwoPerUSRoundTrip ||
         limit == type::TaxApplicationLimit::Unlimited;
}
}

void
RulesBuilder::buildTaxApplicationLimitRule()
{
  type::TaxApplicationLimit taxApplicationLimit = _rulesRecord.taxApplicationLimit;

  if (!isLimitValid(taxApplicationLimit))
    taxApplicationLimit = type::TaxApplicationLimit::Unlimited;

  _limitGroup._limitType = taxApplicationLimit;

  _limitGroup._blankLimitRule = BlankLimitRule();
  _limitGroup._onePerItinLimitRule = OnePerItinLimitRule();
  _limitGroup._usOneWayAndRoundTripLimitRule = UsOneWayAndRoundTripLimitRule();
  _limitGroup._continuousJourneyLimitRule = ContinuousJourneyLimitRule();
  _limitGroup._singleJourneyLimitRule = SingleJourneyLimitRule();
}

void
RulesBuilder::buildApplicationLimitRule()
{
  if (_rulesRecord.taxName.taxCode() == "AY" &&
      _rulesRecord.taxApplicationLimit == type::TaxApplicationLimit::FirstTwoPerUSRoundTrip)
  {
    _validatorsGroups._processingOptionGroup._applicationLimitRule =
        ApplicationLimitRule(_rulesRecord.taxApplicationLimit);
  }
}

void
RulesBuilder::buildTaxMatchingApplTagRule()
{
  if (!_rulesRecord.taxMatchingApplTag.empty())
  {
    _validatorsGroups._miscGroup._taxMatchingApplTagRule =
        TaxMatchingApplTagRule(_rulesRecord.taxMatchingApplTag,
                               _rulesRecord.taxProcessingApplTag == taxProcessingApplTag02_alternateTurnaroundLogic);
  }
}

void
RulesBuilder::buildTaxOnChangeFeeRule()
{
  if (_taxableUnits.hasTag(type::TaxableUnit::ChangeFee))
  {
    _calculatorsGroups._applyGroup._taxOnChangeFeeRule =
        TaxOnChangeFeeRule(_rulesRecord.taxName.percentFlatTag());
  }
}

void
RulesBuilder::buildTaxOnTicketingFeeRule()
{
  if (_taxableUnits.hasTicketingFeeTags())
  {
    _calculatorsGroups._applyGroup._taxOnTicketingFeeRule =
        TaxOnTicketingFeeRule(_rulesRecord.taxName.percentFlatTag());
  }
}

void
RulesBuilder::buildTaxRoundingRule()
{
  _calculatorsGroups._applyGroup._taxRoundingRule =
      TaxRoundingRule(_rulesRecord.taxRoundUnit, _rulesRecord.taxRoundDir,
          _rulesRecord.taxProcessingApplTag == taxProcessingApplTag05_roundToFives
              ? type::TaxRoundingPrecision::ToFives
              : type::TaxRoundingPrecision::ToUnits);
}

void
RulesBuilder::buildTaxRoundingOCRule()
{
  _calculatorsGroups._applyGroup._taxRoundingOCRule =
      TaxRoundingOCRule(_rulesRecord.taxRoundUnit, _rulesRecord.taxRoundDir);
}

void
RulesBuilder::buildReportingRecordRule()
{
  _validatorsGroups._miscGroup._reportingRecordRule =
      ReportingRecordRule(_rulesRecord.vendor,
                          _rulesRecord.taxName.nation(),
                          _rulesRecord.taxName.taxCarrier(),
                          _rulesRecord.taxName.taxCode(),
                          _rulesRecord.taxName.taxType());
}

void
RulesBuilder::buildExemptTagRule()
{
  if (_rulesRecord.exemptTag == type::ExemptTag::Exempt)
    _validatorsGroups._miscGroup._exemptTagRule = ExemptTagRule();
}

void
RulesBuilder::buildTaxCodeConversionRule()
{
  _calculatorsGroups._finalGroup._taxCodeConversionRule = TaxCodeConversionRule();
}

} /* namespace tax */
