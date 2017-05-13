// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Xform/PricingResponseXMLTags.h"
#include "Xform/XMLFareCalcFormatter.h"

namespace tse
{
FALLBACK_DECL(fallbackAMCPhase2);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);

static constexpr uint16_t PERCENT_NO_DEC = 2;

void
XMLFareCalcFormatter::formatCAL(PricingTrx& pricingTrx,
                                CalcTotals& calcTotals,
                                const FareUsage& fareUsage,
                                const PricingUnit& pricingUnit,
                                const CurrencyNoDec& noDecCalc,
                                const FarePath& farePath,
                                uint16_t& segmentOrder,
                                XMLConstruct& construct,
                                std::vector<const FareUsage*>& fuPlusUpsShown,
                                const uint16_t& fcId)
{
  _construct.openElement(xml2::FareCalcInformation);

  if (!fallback::fallbackAMCPhase2(&pricingTrx) && fcId > 0)
    _construct.addAttributeShort(xml2::FareComponentNumber, fcId); // "Q6D",

  _construct.addAttribute(xml2::FareCalcDepartureCity, _model.getFareCalcDepartureCity());
  _construct.addAttribute(xml2::FareCalcDepartureAirport, _model.getFareCalcDepartureAirport());
  _construct.addAttribute(xml2::TrueGoverningCarrier, _model.getTrueGoverningCarrier());
  _construct.addAttribute(xml2::FareCalcArrivalCity, _model.getFareCalcArrivalCity());
  _construct.addAttribute(xml2::FareCalcArrivalAirport, _model.getFareCalcArrivalAirport());

  if (_model.isDiscounted())
  {
    _construct.addAttribute(xml2::DiscountCode, _model.getDiscountCode());
    _construct.addAttributeDouble(xml2::DiscountPercentage, _model.getDiscountPercentage(),
        PERCENT_NO_DEC);
  }

  _construct.addAttributeDouble(xml2::FareAmount, _model.getFareAmount(), noDecCalc);
  _construct.addAttribute(xml2::FareBasisCode, _model.getFareBasisCode().c_str());
  _construct.addAttributeInteger(xml2::FareBasisCodeLength, _model.getFareBasisCode().length());

  if (_model.isNetFUNeeded())
  {
    _construct.addAttributeDouble(xml2::NetTktFareAmount, _model.getNetTktFareAmount(), noDecCalc);
  }

  if (_model.isNetRemitPubFareAmountNeeded())
  {
    _construct.addAttributeDouble(xml2::NetRemitPubFareAmount, _model.getNetRemitPubFareAmount(), noDecCalc);
  }

  if (_model.isNetRemitPubFareBasisCodeNeeded())
  {
    _construct.addAttribute(xml2::NetRemitPubFareBasisCode, _model.getNetRemitPubFareBasisCode());
    _construct.addAttributeInteger(xml2::NetRemitPubFareBasisCodeLength,
        _model.getNetRemitPubFareBasisCode().length());
  }

  _construct.addAttribute(xml2::RequestedPassengerType, _model.getRequestedPassengerType());
  _construct.addAttribute(xml2::GoverningCarrier, _model.getGoverningCarrier());
  _construct.addAttributeChar(xml2::FareCalcCabinCode, _model.getFareCalcCabinCode());
  _construct.addAttribute(xml2::DepartureCountry, _model.getDepartureCountry());
  _construct.addAttribute(xml2::DepartureIATA, _model.getDepartureIATA());

  if (_model.getDepartureStateOrProvince() != " ")
  {
    _construct.addAttribute(xml2::DepartureStateOrProvince, _model.getDepartureStateOrProvince());
  }

  _construct.addAttribute(xml2::ArrivalCountry, _model.getArrivalCountry());
  _construct.addAttribute(xml2::ArrivalIATA, _model.getArrivalIATA());

  if (_model.getArrivalStateOrProvince() != " ")
  {
    _construct.addAttribute(xml2::ArrivalStateOrProvince, _model.getArrivalStateOrProvince());
  }

  _construct.addAttributeDouble(xml2::PublishedFareAmount, _model.getPublishedFareAmount(),
      _model.getPublishedFarePrecision());
  _construct.addAttribute(xml2::FareComponentCurrencyCode, _model.getFareComponentCurrencyCode());
  _construct.addAttributeBoolean(xml2::OneWayFare, _model.isOneWayFare());
  _construct.addAttributeBoolean(xml2::RoundTripFare, _model.isRoundTripFare());
  _construct.addAttribute(xml2::CommencementDate, _model.getCommencementDate());
  _construct.addAttributeChar(xml2::TypeOfFare, _model.getTypeOfFare());
  _construct.addAttributeBoolean(xml2::StopoverSegment, _model.isStopoverSegment());

  if (_model.isIsMileageRouting() && !_model.isDomesticTravel())
  {
    _construct.addAttributeBoolean(xml2::IsMileageRouting, _model.isIsMileageRouting());
    _construct.addAttributeUShort(xml2::MileageSurchargePctg, _model.getMileageSurchargePctg());
  }
  else
  {
    _construct.addAttributeBoolean(xml2::IsMileageRouting, false);
  }

  _construct.addAttributeBoolean(xml2::SpecialFare, _model.isSpecialFare());
  _construct.addAttributeUShort(xml2::PricingUnitCount, _model.getPricingUnitCount());
  _construct.addAttributeChar(xml2::PricingUnitType, _model.getPricingUnitType());
  _construct.addAttribute(xml2::FareCalcDirectionality, _model.getFareCalcDirectionality());

  // PL9761 For SDS requires Itinerary Directionality not Fare Directionality
  //    std::string globalDirStr;
  //    globalDirectionToStr(globalDirStr, fareUsage.paxTypeFare()->globalDirection());
  //    construct.addAttribute(xml2::GlobalDirectionInd, globalDirStr);

  if (!_model.isDummyFare())
  {
    _construct.addAttribute(xml2::GlobalDirectionInd, _model.getGlobalDirectionInd());
  }

  if (_model.hasSideTravels())
  {
    _construct.addAttributeChar(xml2::SideTripIndicator, 'T');
    _construct.addAttributeUShort(xml2::SegmentsCount, _model.getSegmentsCount());
  }

  _construct.addAttribute(xml2::CorpId, _model.getCorpId());
  _construct.addAttribute(xml2::OCFareStat, _model.getOCFareStat());
  _construct.addAttribute(xml2::FCAFareType, _model.getFCAFareType()); // "S53",

  if (_model.isSplitTaxAttributesNeeded())
  {
    _construct.addAttributeDouble(xml2::TotalTaxesPerFareComponent,
        _model.getTotalTaxesPerFareComponent(), noDecCalc);
    _construct.addAttributeDouble(xml2::TotalSurchargesPerFareComponent,
        _model.getTotalSurchargesPerFareComponent(), noDecCalc);
    _construct.addAttributeDouble(xml2::FareComponentPlusTaxesPlusSurcharges,
        _model.getFareComponentPlusTaxesPlusSurcharges(), noDecCalc);
    _construct.addAttributeDouble(xml2::FareComponentInEquivalentCurrency,
        _model.getFareComponentInEquivalentCurrency(), noDecCalc);
  }

  _pricingResponseFormatter.formatElementsInCAL(pricingTrx,
                                                calcTotals,
                                                fareUsage,
                                                pricingUnit,
                                                noDecCalc,
                                                farePath,
                                                segmentOrder,
                                                construct,
                                                fuPlusUpsShown,
                                                *_model.getFareCompInfo());

  _construct.closeElement();
}

}
