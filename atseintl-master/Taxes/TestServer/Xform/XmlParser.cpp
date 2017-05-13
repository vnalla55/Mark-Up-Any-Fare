// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "AtpcoTaxes/DataModel/Services/CarrierFlightSegment.h"
#include "AtpcoTaxes/DomainDataObjects/FlightUsage.h"
#include "AtpcoTaxes/DomainDataObjects/Request.h"
#include "AtpcoTaxes/Rules/XmlParsingError.h"
#include "AtpcoTaxes/ServiceInterfaces/Services.h"
#include "TestServer/Server/InputRequestWithCache.h"
#include "TestServer/Xform/FromString.h"
#include "TestServer/Xform/XmlParser.h"
#include "TestServer/Xform/XmlTagsList.h"

namespace tax
{

using rapidxml::xml_node;
using rapidxml::xml_attribute;

namespace
{

struct DefaultConvert
{
  template <typename T>
  void operator()(const char* asText, T& asValue) const
  {
    asValue = FromString::fromString<T>(asText);
  }
};

template <typename T, typename Conv>
bool
parse_attr(T& destination,
           xml_attribute<>* attribute,
           std::string const& name,
           Conv converter)
{
  if (attribute && (name == attribute->name()))
  {
    try
    {
      converter(attribute->value(), destination);
      return true;
    }
    catch (std::exception const&)
    {
      throw XmlParsingError("Cannot XmlParser::parse attribute value: " +
                            std::string(attribute->name()) + "=" + attribute->value());
    }
  }
  else
  {
    return false;
  }
}

template <typename tag, int L, int H, typename Conv>
bool
parse_attr(Code<tag, L, H>& destination,
           xml_attribute<>* attribute,
           std::string const& name,
           Conv converter)
{
  std::string asString;
  const bool ans = parse_attr(asString, attribute, name, converter);
  if (ans) codeFromString(asString, destination); // otherwise initialize to empty, but return true
  return ans;
}

template <typename T, typename Conv>
bool
parse_attr(CompactOptional<T>& destination,
           xml_attribute<>* attribute,
           std::string const& name,
           Conv converter)
{
  T rawVal;
  const bool ans = parse_attr(rawVal, attribute, name, converter);
  if (ans) destination = rawVal;
  return ans;
}

} // anonymous namespace


XmlParser::XmlParser()
  : _l(0)
{
}

XmlParser::~XmlParser()
{
}

template <typename T>
bool
XmlParser::parse(T& destination, xml_node<>* root)
{
  if (root && (_l->getTagName<T>() == root->name()))
  {
    xml_node<>* child = root->first_node();
    while (child)
    {
      if (!XmlParser::parseAnyChild(destination, child))
        throw XmlParsingError("Unknown XML element: " + std::string(child->name()));
      child = child->next_sibling();
    }

    xml_attribute<>* attribute = root->first_attribute();
    while (attribute)
    {
      if (!XmlParser::parseAnyAttribute(destination, attribute))
        throw XmlParsingError("Unknown XML attribute: " + std::string(attribute->name()));
      attribute = attribute->next_attribute();
    }
    return true;
  }
  else
  {
    return false;
  }
}

template <typename T>
bool
XmlParser::parse(boost::ptr_vector<T>& destination, xml_node<>* root)
{
  destination.push_back(new T());
  T& element = destination.back();
  if (parse(element, root))
  {
    return true;
  }
  else
  {
    destination.pop_back();
    return false;
  }
}

template <typename T>
bool
XmlParser::parse(std::vector<T>& destination, xml_node<>* root)
{
  destination.push_back(T());
  T& element = destination.back();
  if (parse(element, root))
  {
    return true;
  }
  else
  {
    destination.pop_back();
    return false;
  }
}

template <typename T>
bool
XmlParser::parse(boost::optional<T>& destination, xml_node<>* root)
{
  T val;
  if (parse(val, root))
  {
    destination = val;
    return true;
  }
  else
  {
    return false;
  }
}

template <typename T>
bool
XmlParser::parse(T& destination, xml_attribute<>* attribute, std::string const& name)
{
  return parse_attr(destination, attribute, name, DefaultConvert());
}

template <typename T, typename Conv>
bool
XmlParser::parse(T& destination,
                 xml_attribute<>* attribute,
                 std::string const& name,
                 Conv converter)
{
  return parse_attr(destination, attribute, name, converter);
}

// REQUEST //

template <>
bool
XmlParser::parseAnyChild(InputRequestWithCache& requestWithCache, xml_node<>* child)
{
  return parse(requestWithCache.request.pointsOfSale(), child) ||
         parse(requestWithCache.request.geoPaths(), child) ||
         parse(requestWithCache.request.prevTicketGeoPath(), child) ||
         parse(requestWithCache.request.fares(), child) ||
         parse(requestWithCache.request.farePaths(), child) ||
         parse(requestWithCache.request.yqYrs(), child) ||
         parse(requestWithCache.request.yqYrPaths(), child) ||
         parse(requestWithCache.request.optionalServices(), child) ||
         parse(requestWithCache.request.optionalServicePaths(), child) ||
         parse(requestWithCache.request.geoPathMappings(), child) ||
         parse(requestWithCache.request.diagnostic(), child) ||
         parse(requestWithCache.request.itins(), child) ||
         parse(requestWithCache.xmlCache, child) ||
         parse(requestWithCache.request.processing(), child) ||
         parse(requestWithCache.request.flights(), child) ||
         parse(requestWithCache.request.flightPaths(), child) ||
         parse(requestWithCache.request.ticketingOptions(), child) ||
         parse(requestWithCache.request.passengers(), child) ||
         parse(requestWithCache.request.changeFees(), child) ||
         parse(requestWithCache.request.ticketingFees(), child) ||
         parse(requestWithCache.request.ticketingFeePaths(), child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputRequestWithCache& requestWithCache, xml_attribute<>* attribute)
{
  return parse(requestWithCache.request.echoToken(), attribute, _l->getAttributeName_EchoToken());
}

// Processing options

template <>
bool
XmlParser::parseAnyChild(InputProcessingOptions& processingOptions, xml_node<>* child)
{
  return parse(processingOptions.exemptedRules(), child) ||
         parse(processingOptions.calculationRestrictions(), child) ||
         parse(processingOptions.taxDetailsLevel(), child) ||
         parse(processingOptions.getMutableApplicableGroups(), child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputProcessingOptions& processingOptions, xml_attribute<>* attribute)
{
  return parse(processingOptions._tch, attribute, _l->getAttributeName_TCH()) ||
         parse(processingOptions._useRepricing, attribute, _l->getAttributeName_UseRepricing()) ||
         parse(processingOptions._rtw, attribute, _l->getAttributeName_RoundTheWorld()) ||
         parse(processingOptions._geoPathsSize, attribute, _l->getAttributeName_GeoPathsSize()) ||
         parse(processingOptions._geoPathMappingsSize, attribute, _l->getAttributeName_GeoPathMappingsSize()) ||
         parse(processingOptions._faresSize, attribute, _l->getAttributeName_FaresSize()) ||
         parse(processingOptions._farePathsSize, attribute, _l->getAttributeName_FarePathsSize()) ||
         parse(processingOptions._flightsSize, attribute, _l->getAttributeName_FlightsSize()) ||
         parse(processingOptions._flightPathsSize, attribute, _l->getAttributeName_FlightPathsSize()) ||
         parse(processingOptions._yqYrsSize, attribute, _l->getAttributeName_YqYrsSize()) ||
         parse(processingOptions._yqYrPathsSize, attribute, _l->getAttributeName_YqYrPathsSize()) ||
         parse(processingOptions._itinsSize, attribute, _l->getAttributeName_ItinsSize()) ||
         parse(processingOptions._applyUSCAgrouping, attribute, _l->getAttributeName_ApplyUSCAGrouping());
}

// InputApplyOn

template <>
bool
XmlParser::parseAnyChild(InputApplyOn&, xml_node<>*)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputApplyOn& exemptedRule, xml_attribute<>* attribute)
{
  return parse(exemptedRule._group, attribute, _l->getAttributeName_ProcessingGroup());
}

// InputExemptedRule

template <>
bool
XmlParser::parseAnyChild(InputExemptedRule&, xml_node<>*)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputExemptedRule& exemptedRule, xml_attribute<>* attribute)
{
  return parse(exemptedRule._ruleRefId, attribute, _l->getAttributeName_RuleRefId());
}

// InputCalculationRestriction
template <>
bool
XmlParser::parseAnyChild(InputCalculationRestriction& calculationRestriction, xml_node<>* child)
{
  return parse(calculationRestriction.calculationRestrictionTax(), child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputCalculationRestriction& calculationRestriction,
                             xml_attribute<>* attribute)
{
  return parse(calculationRestriction.restrictionType(), attribute, _l->getAttributeName_Type());
}

// TaxDetailsLevel
template <>
bool
XmlParser::parseAnyChild(InputTaxDetailsLevel&, xml_node<>*)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputTaxDetailsLevel& level, xml_attribute<>* attr)
{
  // attribute name shortcuts to fit lines into 100 chars
  std::string attrTaxOnOcDetails = _l->getAttributeName_TaxOnOptionalServicesDetails();
  std::string attrTaxOnExchangeDetails = _l->getAttributeName_TaxOnExchangeReissueDetails();
  std::string attrExchangeDetails = _l->getAttributeName_ExchangeReissueDetails();

  return parse(level.allDetails(), attr, _l->getAttributeName_AllDetails()) ||
         parse(level.geoDetails(), attr, _l->getAttributeName_GeoDetails()) ||
         parse(level.calcDetails(), attr, _l->getAttributeName_CalcDetails()) ||
         parse(level.exchangeReissueDetails(), attr, attrExchangeDetails) ||
         parse(level.taxOnExchangeReissueDetails(), attr, attrTaxOnExchangeDetails) ||
         parse(level.taxOnFaresDetails(), attr, _l->getAttributeName_TaxOnFaresDetails()) ||
         parse(level.taxOnOptionalServiceDetails(), attr, attrTaxOnOcDetails) ||
         parse(level.taxOnTaxDetails(), attr, _l->getAttributeName_TaxOnTaxDetails()) ||
         parse(level.taxOnYQYRDetails(), attr, _l->getAttributeName_TaxOnYQYRDetails());
}

//  CalculationRestrictionTax

template <>
bool
XmlParser::parseAnyChild(InputCalculationRestrictionTax&, xml_node<>*)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputCalculationRestrictionTax& calculationRestrictionTax,
                             xml_attribute<>* attribute)
{
  return parse(calculationRestrictionTax.nationCode(), attribute, _l->getAttributeName_Nation()) ||
         parse(calculationRestrictionTax.taxCode(), attribute, _l->getAttributeName_Code()) ||
         parse(calculationRestrictionTax.taxType(), attribute, _l->getAttributeName_Type());
}

template <>
bool
XmlParser::parseAnyChild(InputTicketingOptions&, xml_node<>*)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputTicketingOptions& ticketingOptions, xml_attribute<>* attribute)
{
  return parse(ticketingOptions.paymentCurrency(),
               attribute,
               _l->getAttributeName_PaymentCurrency()) ||
         parse(ticketingOptions.formOfPayment(), attribute, _l->getAttributeName_FormOfPayment()) ||
         parse(
             ticketingOptions.ticketingPoint(), attribute, _l->getAttributeName_TicketingPoint()) ||
         parse(ticketingOptions.ticketingDate(), attribute, _l->getAttributeName_TicketingDate()) ||
         parse(ticketingOptions.ticketingTime(), attribute, _l->getAttributeName_TicketingTime());
}

// POINT OF SALE //

template <>
bool
XmlParser::parseAnyChild(InputPointOfSale& /*pointOfSale*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputPointOfSale& pointOfSale, xml_attribute<>* attribute)
{
  return parse(pointOfSale.loc(), attribute, _l->getAttributeName_PointOfSaleLoc()) ||
         parse(pointOfSale.id(), attribute, _l->getAttributeName_Id()) ||
         parse(pointOfSale.agentPcc(), attribute, _l->getAttributeName_AgentPCC()) ||
         parse(pointOfSale.vendorCrsCode(), attribute, _l->getAttributeName_VendorCrsCode()) ||
         parse(pointOfSale.carrierCode(), attribute, _l->getAttributeName_CarrierCode()) ||
         parse(pointOfSale.agentDuty(), attribute, _l->getAttributeName_DutyCode()) ||
         parse(pointOfSale.agentFunction(), attribute, _l->getAttributeName_FunctionCode()) ||
         parse(pointOfSale.agentCity(), attribute, _l->getAttributeName_AgentCity()) ||
         parse(pointOfSale.iataNumber(), attribute, _l->getAttributeName_IATANumber()) ||
         parse(pointOfSale.ersp(), attribute, _l->getAttributeName_ERSP()) ||
         parse(
             pointOfSale.agentAirlineDept(), attribute, _l->getAttributeName_AgentAirlineDept()) ||
         parse(pointOfSale.agentOfficeDesignator(),
               attribute,
               _l->getAttributeName_AgentOfficeDesignator());
}

// GEO PATH //
template <>
bool
XmlParser::parseAnyChild(InputGeoPath& geoPath, xml_node<>* child)
{
  return parse(geoPath._geos, child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputGeoPath& geoPath, xml_attribute<>* attribute)
{
  return parse(geoPath._id, attribute, _l->getAttributeName_Id());
}

// PREV TICKET GEO PATH //
template <>
bool
XmlParser::parseAnyChild(InputPrevTicketGeoPath& geoPath, xml_node<>* child)
{
  return parse(geoPath._geos, child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputPrevTicketGeoPath& geoPath, xml_attribute<>* attribute)
{
  return parse(geoPath._id, attribute, _l->getAttributeName_Id());
}

// GEO //
template <>
bool
XmlParser::parseAnyChild(InputGeo& /*geo*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputGeo& geo, xml_attribute<>* attribute)
{
  return parse(geo._loc.tag(), attribute, _l->getAttributeName_Type()) ||
         parse(geo._loc.code(), attribute, _l->getAttributeName_Loc()) ||
         parse(geo._loc.inBufferZone(), attribute, _l->getAttributeName_BufferZoneInd()) ||
         parse(geo._unticketedTransfer, attribute, _l->getAttributeName_UnticketedTransfer());
}

// FARE //

template <>
bool
XmlParser::parseAnyChild(InputFare& /*fare*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputFare& fare, xml_attribute<>* attribute)
{
  return parse(fare._id, attribute, _l->getAttributeName_Id()) ||
         parse(fare._basis, attribute, _l->getAttributeName_BasisCode()) ||
         parse(fare._type, attribute, _l->getAttributeName_TypeCode()) ||
         parse(fare._oneWayRoundTrip, attribute, _l->getAttributeName_OneWayRoundTrip()) ||
         parse(fare._directionality, attribute, _l->getAttributeName_Directionality()) ||
         parse(fare._amount, attribute, _l->getAttributeName_Amount()) ||
         parse(fare._sellAmount, attribute, _l->getAttributeName_SellAmount()) ||
         parse(fare._rule, attribute, _l->getAttributeName_Rule()) ||
         parse(fare._isNetRemitAvailable, attribute, _l->getAttributeName_IsNetRemitAvailable()) ||
         parse(fare._tariff, attribute, _l->getAttributeName_Tariff()) ||
         parse(fare._tariffInd, attribute, _l->getAttributeName_TariffInd()) ||
         parse(fare._outputPtc, attribute, _l->getAttributeName_OutputPassengerCode()) ||
         parse(fare._markupAmount, attribute, _l->getAttributeName_MarkupAmount());
}

// FARE PATH //

template <>
bool
XmlParser::parseAnyChild(InputFarePath& farePath, xml_node<>* child)
{
  return parse(farePath._fareUsages, child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputFarePath& farePath, xml_attribute<>* attribute)
{
  return parse(farePath._id, attribute, _l->getAttributeName_Id()) ||
         parse(farePath._totalAmount, attribute, _l->getAttributeName_TotalAmount()) ||
         parse(farePath._totalAmountBeforeDiscount,
               attribute,
               _l->getAttributeName_TotalAmountBeforeDiscount()) ||
         parse(farePath._validatingCarrier, attribute, _l->getAttributeName_ValidatingCarrier()) ||
         parse(farePath._outputPtc, attribute, _l->getAttributeName_OutputPassengerCode());
}

// FLIGHT PATH
template <>
bool
XmlParser::parseAnyChild(InputFlightPath& flightPath, xml_node<>* child)
{
  return parse(flightPath._flightUsages, child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputFlightPath& flightPath, xml_attribute<>* attribute)
{
  return parse(flightPath._id, attribute, _l->getAttributeName_Id());
}

// FARE USAGE
template <>
bool
XmlParser::parseAnyChild(InputFareUsage& /*fareUsage*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputFareUsage& fareUsage, xml_attribute<>* attribute)
{
  return parse(fareUsage._fareRefId, attribute, _l->getAttributeName_FareRefId());
}

// OPTIONALSERVICE //

template <>
bool
XmlParser::parseAnyChild(InputOptionalService& /*optionalService*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputOptionalService& optionalService, xml_attribute<>* attribute)
{
  return parse(optionalService._id, attribute, _l->getAttributeName_Id()) ||
         parse(optionalService._subCode, attribute, _l->getAttributeName_ServiceSubTypeCode()) ||
         parse(optionalService._type, attribute, _l->getAttributeName_Type()) ||
         parse(optionalService._serviceGroup, attribute, _l->getAttributeName_SvcGroup()) ||
         parse(optionalService._serviceSubGroup, attribute, _l->getAttributeName_SvcSubGroup()) ||
         parse(optionalService._amount, attribute, _l->getAttributeName_Amount()) ||
         parse(optionalService._ownerCarrier, attribute, _l->getAttributeName_OwnerCarrier()) ||
         parse(optionalService._pointOfDeliveryLoc,
               attribute,
               _l->getAttributeName_PointOfDeliveryLoc()) ||
         parse(optionalService._outputPtc, attribute, _l->getAttributeName_OutputPassengerCode());
}

// OPTIONALSERVICE PATH //

template <>
bool
XmlParser::parseAnyChild(InputOptionalServicePath& optionalServicePath, xml_node<>* child)
{
  return parse(optionalServicePath._optionalServiceUsages, child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputOptionalServicePath& optionalServicePath,
                             xml_attribute<>* attribute)
{
  return parse(optionalServicePath._id, attribute, _l->getAttributeName_Id());
}

// OPTIONALSERVICE USAGE //

template <>
bool
XmlParser::parseAnyChild(InputOptionalServiceUsage& /*optionalServiceUsage*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputOptionalServiceUsage& optionalServiceUsage,
                             xml_attribute<>* attribute)
{
  return parse(optionalServiceUsage._optionalServiceRefId,
               attribute,
               _l->getAttributeName_OptionalServiceRefId());
}

//////////////////////

// YQYR //

template <>
bool
XmlParser::parseAnyChild(InputYqYr& /*yqYr*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputYqYr& yqYr, xml_attribute<>* attribute)
{
  return parse(yqYr._id, attribute, _l->getAttributeName_Id()) ||
         parse(yqYr._code, attribute, _l->getAttributeName_Code()) ||
         parse(yqYr._type, attribute, _l->getAttributeName_Type()) ||
         parse(yqYr._amount, attribute, _l->getAttributeName_Amount()) ||
         parse(yqYr._taxIncluded, attribute, _l->getAttributeName_TaxIncluded());
}

// YQYR PATH //

template <>
bool
XmlParser::parseAnyChild(InputYqYrPath& yqYrPath, xml_node<>* child)
{
  return parse(yqYrPath._yqYrUsages, child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputYqYrPath& yqYrPath, xml_attribute<>* attribute)
{
  return parse(yqYrPath._id, attribute, _l->getAttributeName_Id()) ||
         parse(yqYrPath._totalAmount, attribute, _l->getAttributeName_TotalAmount());
}

// YQYR USAGE //

template <>
bool
XmlParser::parseAnyChild(InputYqYrUsage& /*yqYrUsage*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputYqYrUsage& yqYrUsage, xml_attribute<>* attribute)
{
  return parse(yqYrUsage._yqYrRefId, attribute, _l->getAttributeName_YqYrRefId());
}

// GEO PATH MAPPING //

template <>
bool
XmlParser::parseAnyChild(InputGeoPathMapping& geoPathMapping, xml_node<>* child)
{
  return parse(geoPathMapping._mappings, child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputGeoPathMapping& geoPathMapping, xml_attribute<>* attribute)
{
  return parse(geoPathMapping._id, attribute, _l->getAttributeName_Id());
}

// MAPPING //

template <>
bool
XmlParser::parseAnyChild(InputMapping& mapping, xml_node<>* child)
{
  return parse(mapping.maps(), child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputMapping& /*mapping*/, xml_attribute<>* /*attribute*/)
{
  return false;
}

// MAP //

template <>
bool
XmlParser::parseAnyChild(InputMap& /*map*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputMap& map, xml_attribute<>* attribute)
{
  return parse(map._geoRefId, attribute, _l->getAttributeName_GeoRefId());
}

// FLIGHT //

template <>
bool
XmlParser::parseAnyChild(InputFlight& /*map*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputFlight& flight, xml_attribute<>* attribute)
{
  return parse(flight._id, attribute, _l->getAttributeName_Id()) ||
         parse(flight._departureTime, attribute, _l->getAttributeName_DepartureTime()) ||
         parse(flight._arrivalTime, attribute, _l->getAttributeName_ArrivalTime()) ||
         parse(flight._arrivalDateShift, attribute, _l->getAttributeName_ArrivalDateShift()) ||
         parse(flight._marketingCarrierFlightNumber,
               attribute,
               _l->getAttributeName_MarketingCarrierFlightNumber()) ||
         parse(flight._marketingCarrier, attribute, _l->getAttributeName_MarketingCarrier()) ||
         parse(flight._operatingCarrier, attribute, _l->getAttributeName_OperatingCarrier()) ||
         parse(flight._equipment, attribute, _l->getAttributeName_Equipment()) ||
         parse(flight._cabinCode, attribute, _l->getAttributeName_CabinCode()) ||
         parse(flight._reservationDesignator, attribute, _l->getAttributeName_ReservationDesignator());
}

// FLIGHT USAGE //

template <>
bool
XmlParser::parseAnyChild(InputFlightUsage& /*flightUsage*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputFlightUsage& flightUsage, xml_attribute<>* attribute)
{
  return parse(flightUsage._flightRefId, attribute, _l->getAttributeName_FlightRefId()) ||
         parse(flightUsage._bookingCodeType, attribute, _l->getAttributeName_BookingCodeType()) ||
         parse(flightUsage._connectionDateShift,
               attribute,
               _l->getAttributeName_ConnectionDateShift()) ||
         parse(flightUsage._openSegmentIndicator, attribute, _l->getAttributeName_SkipDateTime()) ||
         parse(flightUsage._forcedConnection, attribute, _l->getAttributeName_ForcedConnection());
}

// ITIN //

template <>
bool
XmlParser::parseAnyChild(InputItin& /*itin*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputItin& itin, xml_attribute<>* attribute)
{
  return parse(itin._id, attribute, _l->getAttributeName_Id()) ||
         parse(itin._geoPathRefId, attribute, _l->getAttributeName_GeoPathRefId()) ||
         parse(itin._farePathRefId, attribute, _l->getAttributeName_FarePathRefId()) ||
         parse(itin._flightPathRefId, attribute, _l->getAttributeName_FlightPathRefId()) ||
         parse(itin._yqYrPathRefId, attribute, _l->getAttributeName_YqYrPathRefId()) ||
         parse(itin._optionalServicePathRefId,
               attribute,
               _l->getAttributeName_OptionalServicePathRefId()) ||
         parse(itin._farePathGeoPathMappingRefId,
               attribute,
               _l->getAttributeName_FarePathGeoPathMappingRefId()) ||
         parse(itin._yqYrPathGeoPathMappingRefId,
               attribute,
               _l->getAttributeName_YqYrPathGeoPathMappingRefId()) ||
         parse(itin._optionalServicePathGeoPathMappingRefId,
               attribute,
               _l->getAttributeName_OptionalServicePathGeoPathMappingRefId()) ||
         parse(itin._travelOriginDate, attribute, _l->getAttributeName_TravelOriginDate()) ||
         parse(itin._passengerRefId, attribute, _l->getAttributeName_PassengerRefId()) ||
         parse(itin._pointOfSaleRefId, attribute, _l->getAttributeName_PointOfSaleRefId()) ||
         parse(itin._changeFeeRefId, attribute, _l->getAttributeName_ChangeFeeRefId()) ||
         parse(itin._ticketingFeeRefId, attribute, _l->getAttributeName_TicketingFeeRefId());
}

// DIAGNOSTIC COMMAND

template <>
bool
XmlParser::parseAnyChild(InputDiagnosticCommand& diagnostic, xml_node<>* child)
{
  return parse(diagnostic._parameters, child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputDiagnosticCommand& diagnostic, xml_attribute<>* attribute)
{
  return parse(diagnostic._number, attribute, _l->getAttributeName_Id());
}

// PARAMETER

template <>
bool
XmlParser::parseAnyChild(InputParameter& /*parameter*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputParameter& parameter, xml_attribute<>* attribute)
{
  return parse(parameter.name(), attribute, _l->getAttributeName_Name()) ||
         parse(parameter.value(), attribute, _l->getAttributeName_Value());
}

// PASSENGER

template <>
bool
XmlParser::parseAnyChild(InputPassenger& /*passenger*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputPassenger& passenger, xml_attribute<>* attribute)
{
  return parse(passenger._id, attribute, _l->getAttributeName_Id()) ||
         parse(passenger._code, attribute, _l->getAttributeName_PassengerCode()) ||
         parse(passenger._birthDate, attribute, _l->getAttributeName_DateOfBirth()) ||
         parse(passenger._stateCode, attribute, _l->getAttributeName_StateCode()) ||
         parse(passenger._passengerStatus._nationality,
               attribute,
               _l->getAttributeName_Nationality()) ||
         parse(
             passenger._passengerStatus._residency, attribute, _l->getAttributeName_Residency()) ||
         parse(
             passenger._passengerStatus._employment, attribute, _l->getAttributeName_Employment());
}

// XML CACHE //

template <>
bool
XmlParser::parseAnyChild(XmlCache& xmlCache, xml_node<>* child)
{
  return parse(xmlCache.rulesRecords(), child) || parse(xmlCache.reportingRecords(), child) ||
         parse(xmlCache.nations(), child) || parse(xmlCache.isInLocs(), child) ||
         parse(xmlCache.mileages(), child) || parse(xmlCache.carrierFlights(), child) ||
         parse(xmlCache.carrierApplications(), child) || parse(xmlCache.currencies(), child) ||
         parse(xmlCache.currencyConversions(), child) || parse(xmlCache.serviceBaggage(), child) ||
         parse(xmlCache.passengerTypeCodes(), child) || parse(xmlCache.sectorDetail(), child) ||
         parse(xmlCache.aKHIFactor(), child) || parse(xmlCache.paxTypeMapping(), child) ||
         parse(xmlCache.serviceFeeSecurity(), child) || parse(xmlCache.taxRoundings(), child) ||
         parse(xmlCache.repricingEntries(), child) || parse(xmlCache.customers(), child);
}

template <>
bool
XmlParser::parseAnyAttribute(XmlCache& /*xmlCache*/, xml_attribute<>* /*attribute*/)
{
  return false;
}

// RULESRECORD CACHE //

template <>
bool
XmlParser::parseAnyChild(RulesRecord& /*rulesRecord*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(RulesRecord& rulesRecord, xml_attribute<>* attribute)
{
  type::ConnectionsTag connectionsTag(type::ConnectionsTag::Blank);
  if (parse(connectionsTag, attribute, _l->getAttributeName_CONNECTIONSTAG1()) ||
      parse(connectionsTag, attribute, _l->getAttributeName_CONNECTIONSTAG2()) ||
      parse(connectionsTag, attribute, _l->getAttributeName_CONNECTIONSTAG3()) ||
      parse(connectionsTag, attribute, _l->getAttributeName_CONNECTIONSTAG4()) ||
      parse(connectionsTag, attribute, _l->getAttributeName_CONNECTIONSTAG5()) ||
      parse(connectionsTag, attribute, _l->getAttributeName_CONNECTIONSTAG6()) ||
      parse(connectionsTag, attribute, _l->getAttributeName_CONNECTIONSTAG7()))
  {
    if (connectionsTag != type::ConnectionsTag::Blank)
      rulesRecord.connectionsTags.insert(connectionsTag);
    return true;
  }
  type::TaxableUnitTag taxableUnitTag(type::TaxableUnitTag::Blank);
  type::TaxableUnit taxableUnit = type::TaxableUnit::Blank;
  if (parse(taxableUnitTag, attribute, _l->getAttributeName_TAXABLEUNITTAG1()))
    taxableUnit = type::TaxableUnit::YqYr;
  else if (parse(taxableUnitTag, attribute, _l->getAttributeName_TAXABLEUNITTAG2()))
    taxableUnit = type::TaxableUnit::TicketingFee;
  else if (parse(taxableUnitTag, attribute, _l->getAttributeName_TAXABLEUNITTAG3()))
    taxableUnit = type::TaxableUnit::OCFlightRelated;
  else if (parse(taxableUnitTag, attribute, _l->getAttributeName_TAXABLEUNITTAG4()))
    taxableUnit = type::TaxableUnit::OCTicketRelated;
  else if (parse(taxableUnitTag, attribute, _l->getAttributeName_TAXABLEUNITTAG5()))
    taxableUnit = type::TaxableUnit::OCMerchandise;
  else if (parse(taxableUnitTag, attribute, _l->getAttributeName_TAXABLEUNITTAG6()))
    taxableUnit = type::TaxableUnit::OCFareRelated;
  else if (parse(taxableUnitTag, attribute, _l->getAttributeName_TAXABLEUNITTAG7()))
    taxableUnit = type::TaxableUnit::BaggageCharge;
  else if (parse(taxableUnitTag, attribute, _l->getAttributeName_TAXABLEUNITTAG8()))
    taxableUnit = type::TaxableUnit::TaxOnTax;
  else if (parse(taxableUnitTag, attribute, _l->getAttributeName_TAXABLEUNITTAG9()))
    taxableUnit = type::TaxableUnit::Itinerary;
  else if (parse(taxableUnitTag, attribute, _l->getAttributeName_TAXABLEUNITTAG10()))
    taxableUnit = type::TaxableUnit::ChangeFee;

  if (taxableUnitTag != type::TaxableUnitTag::Blank)
    rulesRecord.applicableTaxableUnits.setTag(taxableUnit);

  if (taxableUnit != type::TaxableUnit::Blank)
    return true;

  return parse(rulesRecord.taxName.nation(), attribute, _l->getAttributeName_NATION()) ||
         parse(rulesRecord.taxName.taxCode(), attribute, _l->getAttributeName_TAXCODE()) ||
         parse(rulesRecord.taxName.taxType(), attribute, _l->getAttributeName_TAXTYPE()) ||
         parse(rulesRecord.taxName.taxPointTag(), attribute, _l->getAttributeName_TAXPOINTTAG()) ||
         parse(rulesRecord.taxName.percentFlatTag(),
               attribute,
               _l->getAttributeName_PERCENTFLATTAG()) ||
         parse(rulesRecord.taxName.taxRemittanceId(),
               attribute,
               _l->getAttributeName_TAXREMITTANCEID()) ||
         parse(rulesRecord.taxName.taxCarrier(), attribute, _l->getAttributeName_TAXCARRIER()) ||
         parse(rulesRecord.seqNo, attribute, _l->getAttributeName_SEQNO()) ||
         parse(rulesRecord.vendor, attribute, _l->getAttributeName_VENDOR()) ||
         parse(rulesRecord.taxAmt, attribute, _l->getAttributeName_TAXAMT()) ||
         parse(rulesRecord.taxCurrency, attribute, _l->getAttributeName_TAXCURRENCY()) ||
         parse(rulesRecord.taxCurDecimals, attribute, _l->getAttributeName_TAXCURDECIMALS()) ||
         parse(rulesRecord.taxPercent, attribute, _l->getAttributeName_TAXPERCENT()) ||
         parse(rulesRecord.rtnToOrig, attribute, _l->getAttributeName_RTNTOORIG()) ||
         parse(rulesRecord.exemptTag, attribute, _l->getAttributeName_EXEMPTTAG()) ||
         parse(rulesRecord.effDate, attribute, _l->getAttributeName_EFFDATE()) ||
         parse(rulesRecord.discDate, attribute, _l->getAttributeName_DISCDATE()) ||
         parse(rulesRecord.expiredDate, attribute, _l->getAttributeName_EXPIREDATE()) ||
         parse(rulesRecord.firstTravelYear, attribute, _l->getAttributeName_TVLFIRSTYEAR()) ||
         parse(rulesRecord.lastTravelYear, attribute, _l->getAttributeName_TVLLASTYEAR()) ||
         parse(rulesRecord.firstTravelMonth, attribute, _l->getAttributeName_TVLFIRSTMONTH()) ||
         parse(rulesRecord.lastTravelMonth, attribute, _l->getAttributeName_TVLLASTMONTH()) ||
         parse(rulesRecord.firstTravelDay, attribute, _l->getAttributeName_TVLFIRSTDAY()) ||
         parse(rulesRecord.lastTravelDay, attribute, _l->getAttributeName_TVLLASTDAY()) ||
         parse(rulesRecord.travelDateTag, attribute, _l->getAttributeName_TRAVELDATEAPPTAG()) ||
         parse(rulesRecord.ticketedPointTag, attribute, _l->getAttributeName_TICKETEDPOINTTAG()) ||
         parse(rulesRecord.jrnyInd, attribute, _l->getAttributeName_JRNYIND()) ||
         parse(rulesRecord.jrnyLocZone1.type(), attribute, _l->getAttributeName_JRNYLOC1TYPE()) ||
         parse(rulesRecord.jrnyLocZone1.code(), attribute, _l->getAttributeName_JRNYLOC1()) ||
         parse(rulesRecord.jrnyLocZone1.code(),
               attribute,
               _l->getAttributeName_JRNYLOC1ZONETBLNO()) ||
         parse(rulesRecord.jrnyLocZone2.type(), attribute, _l->getAttributeName_JRNYLOC2TYPE()) ||
         parse(rulesRecord.jrnyLocZone2.code(), attribute, _l->getAttributeName_JRNYLOC2()) ||
         parse(rulesRecord.jrnyLocZone2.code(),
               attribute,
               _l->getAttributeName_JRNYLOC2ZONETBLNO()) ||
         parse(rulesRecord.trvlWhollyWithin.type(),
               attribute,
               _l->getAttributeName_TRVLWHOLLYWITHINLOCTYPE()) ||
         parse(rulesRecord.trvlWhollyWithin.code(),
               attribute,
               _l->getAttributeName_TRVLWHOLLYWITHINLOC()) ||
         parse(rulesRecord.trvlWhollyWithin.code(),
               attribute,
               _l->getAttributeName_TRVLWHOLLYWITHINLOCZONETBLNO()) ||
         parse(rulesRecord.jrnyIncludes.type(),
               attribute,
               _l->getAttributeName_JRNYINCLUDESLOCTYPE()) ||
         parse(
             rulesRecord.jrnyIncludes.code(), attribute, _l->getAttributeName_JRNYINCLUDESLOC()) ||
         parse(rulesRecord.jrnyIncludes.code(),
               attribute,
               _l->getAttributeName_JRNYINCLUDESLOCZONETBLNO()) ||
         parse(rulesRecord.taxPointLoc1TransferType,
               attribute,
               _l->getAttributeName_TAXPOINTLOC1TRNSFRTYPE()) ||
         parse(rulesRecord.taxPointLoc1StopoverTag,
               attribute,
               _l->getAttributeName_TAXPOINTLOC1STOPOVERTAG()) ||
         parse(rulesRecord.taxPointLoc2StopoverTag,
               attribute,
               _l->getAttributeName_TAXPOINTLOC2STOPOVERTAG()) ||
         parse(
             rulesRecord.histSaleEffDate, attribute, _l->getAttributeName_HISTORICSALEEFFDATE()) ||
         parse(rulesRecord.histSaleDiscDate,
               attribute,
               _l->getAttributeName_HISTORICSALEDISCDATE()) ||
         parse(
             rulesRecord.histTrvlEffDate, attribute, _l->getAttributeName_HISTORICTRVLEFFDATE()) ||
         parse(rulesRecord.histTrvlDiscDate,
               attribute,
               _l->getAttributeName_HISTORICTRVLDISCDATE()) ||
         parse(rulesRecord.taxPointLocZone1.type(),
               attribute,
               _l->getAttributeName_TAXPOINTLOC1TYPE()) ||
         parse(
             rulesRecord.taxPointLocZone1.code(), attribute, _l->getAttributeName_TAXPOINTLOC1()) ||
         parse(rulesRecord.taxPointLocZone1.code(),
               attribute,
               _l->getAttributeName_TAXPOINTLOC1ZONETBLNO()) ||
         parse(rulesRecord.taxPointLocZone2.type(),
               attribute,
               _l->getAttributeName_TAXPOINTLOC2TYPE()) ||
         parse(
             rulesRecord.taxPointLocZone2.code(), attribute, _l->getAttributeName_TAXPOINTLOC2()) ||
         parse(rulesRecord.taxPointLocZone2.code(),
               attribute,
               _l->getAttributeName_TAXPOINTLOC2ZONETBLNO()) ||
         parse(rulesRecord.taxPointLocZone3.type(),
               attribute,
               _l->getAttributeName_TAXPOINTLOC3TYPE()) ||
         parse(
             rulesRecord.taxPointLocZone3.code(), attribute, _l->getAttributeName_TAXPOINTLOC3()) ||
         parse(rulesRecord.taxPointLocZone3.code(),
               attribute,
               _l->getAttributeName_TAXPOINTLOC3ZONETBLNO()) ||
         parse(rulesRecord.taxPointLoc2IntlDomInd,
               attribute,
               _l->getAttributeName_TAXPOINTLOC2INTLDOMIND()) ||
         parse(rulesRecord.stopoverTimeTag, attribute, _l->getAttributeName_STOPOVERTIMETAG()) ||
         parse(rulesRecord.stopoverTimeUnit, attribute, _l->getAttributeName_STOPOVERTIMEUNIT()) ||
         parse(rulesRecord.taxPointLoc1IntlDomInd,
               attribute,
               _l->getAttributeName_TAXPOINTLOC1INTDOMIND()) ||
         parse(rulesRecord.taxPointLoc2Compare,
               attribute,
               _l->getAttributeName_TAXPOINTLOC2COMPARE()) ||
         parse(rulesRecord.taxPointLoc3GeoType,
               attribute,
               _l->getAttributeName_TAXPOINTLOC3GEOTYPE()) ||
         parse(rulesRecord.currencyOfSale, attribute, _l->getAttributeName_CURRENCYOFSALE()) ||
         parse(
             rulesRecord.pointOfTicketing.type(), attribute, _l->getAttributeName_POTKTLOCTYPE()) ||
         parse(rulesRecord.pointOfTicketing.code(), attribute, _l->getAttributeName_POTKTLOC()) ||
         parse(rulesRecord.pointOfTicketing.code(),
               attribute,
               _l->getAttributeName_POTKTLOCZONETBLNO()) ||
         parse(rulesRecord.pointOfSale.type(), attribute, _l->getAttributeName_POSLOCTYPE()) ||
         parse(rulesRecord.pointOfSale.code(), attribute, _l->getAttributeName_POSLOC()) ||
         parse(rulesRecord.pointOfSale.code(), attribute, _l->getAttributeName_POSLOCZONETBLNO()) ||
         parse(rulesRecord.taxApplicationLimit, attribute, _l->getAttributeName_TAXAPPLIMIT()) ||
         parse(rulesRecord.carrierFlightItemBefore,
               attribute,
               _l->getAttributeName_CARRIERFLTITEMNO1()) ||
         parse(rulesRecord.carrierFlightItemAfter,
               attribute,
               _l->getAttributeName_CARRIERFLTITEMNO2()) ||
         parse(rulesRecord.carrierApplicationItem,
               attribute,
               _l->getAttributeName_CARRIERAPPLITEMNO1()) ||
         parse(rulesRecord.minTax, attribute, _l->getAttributeName_MINTAX()) ||
         parse(rulesRecord.maxTax, attribute, _l->getAttributeName_MAXTAX()) ||
         parse(rulesRecord.minMaxCurrency, attribute, _l->getAttributeName_MINMAXCURRENCY()) ||
         parse(rulesRecord.minMaxDecimals, attribute, _l->getAttributeName_MINMAXDECIMALS()) ||
         parse(rulesRecord.tktValApplQualifier,
               attribute,
               _l->getAttributeName_TKTVALAPPLQUALIFIER()) ||
         parse(rulesRecord.tktValCurrency, attribute, _l->getAttributeName_TKTVALCURRENCY()) ||
         parse(rulesRecord.tktValMin, attribute, _l->getAttributeName_TKTVALMIN()) ||
         parse(rulesRecord.tktValMax, attribute, _l->getAttributeName_TKTVALMAX()) ||
         parse(
             rulesRecord.tktValCurrDecimals, attribute, _l->getAttributeName_TKTVALCURDECIMALS()) ||
         parse(rulesRecord.serviceBaggageItemNo,
               attribute,
               _l->getAttributeName_SERVICEBAGGAGEITEMNO()) ||
         parse(rulesRecord.passengerTypeCodeItem,
               attribute,
               _l->getAttributeName_PSGRTYPECODEITEMNO()) ||
         parse(rulesRecord.netRemitApplTag, attribute, _l->getAttributeName_NETREMITAPPLTAG()) ||
         parse(rulesRecord.sectorDetailItemNo,
               attribute,
               _l->getAttributeName_SECTORDETAILITEMNO()) ||
         parse(rulesRecord.alternateRuleRefTag,
               attribute,
               _l->getAttributeName_ALTERNATERULEREFTAG()) ||
         parse(rulesRecord.sectorDetailApplTag,
               attribute,
               _l->getAttributeName_SECTORDETAILAPPLTAG()) ||
         parse(rulesRecord.taxMatchingApplTag,
               attribute,
               _l->getAttributeName_TAXMATCHINGAPPLTAG()) ||
         parse(rulesRecord.serviceBaggageApplTag,
               attribute,
               _l->getAttributeName_SERVICEBAGGAGEAPPLTAG()) ||
         parse(rulesRecord.calcOrder, attribute, _l->getAttributeName_CALCORDER()) ||
         parse(rulesRecord.taxAppliesToTagInd,
               attribute,
               _l->getAttributeName_TAXAPPLIESTOTAGIND()) ||
         parse(
             rulesRecord.paidBy3rdPartyTag, attribute, _l->getAttributeName_PAIDBY3RDPARTYTAG()) ||
         parse(rulesRecord.taxRoundUnit, attribute, _l->getAttributeName_TAXROUNDUNIT()) ||
         parse(rulesRecord.taxRoundDir, attribute, _l->getAttributeName_TAXROUNDDIR()) ||
         parse(rulesRecord.pointOfDelivery.type(),
               attribute,
               _l->getAttributeName_PODELIVERYLOCTYPE()) ||
         parse(
             rulesRecord.pointOfDelivery.code(), attribute, _l->getAttributeName_PODELIVERYLOC()) ||
         parse(
             rulesRecord.taxProcessingApplTag, attribute, _l->getAttributeName_TAXPROCESSINGAPPLTAG()) ||
         parse(rulesRecord.pointOfDelivery.code(),
               attribute,
               _l->getAttributeName_PODELIVERYLOCZONETBLNO()) ||
         parse(rulesRecord.svcFeesSecurityItemNo,
               attribute,
               _l->getAttributeName_SVCFEESSECURITYITEMNO());
}

// ReportingRecord CACHE //

template <>
bool
XmlParser::parseAnyChild(ReportingRecord& reportingRecord, xml_node<>* child)
{
  return parse(reportingRecord.entries, child);
}

template <>
bool
XmlParser::parseAnyAttribute(ReportingRecord& reportingRecord, xml_attribute<>* attribute)
{
  return parse(reportingRecord.vendor, attribute, _l->getAttributeName_VENDOR()) ||
         parse(reportingRecord.nation, attribute, _l->getAttributeName_NATION()) ||
         parse(reportingRecord.taxCarrier, attribute, _l->getAttributeName_TAXCARRIER()) ||
         parse(reportingRecord.taxCode, attribute, _l->getAttributeName_TAXCODE()) ||
         parse(reportingRecord.taxType, attribute, _l->getAttributeName_TAXTYPE());
}

// ReportingRecordEntry CACHE //

template <>
bool
XmlParser::parseAnyChild(ReportingRecordEntry& /*ReportingRecordEntry*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(ReportingRecordEntry& reportingRecordEntry, xml_attribute<>* attribute)
{
  return parse(reportingRecordEntry.taxLabel, attribute, _l->getAttributeName_TAXNAME());
}

// RepricingEntry CACHE //

template <>
bool
XmlParser::parseAnyChild(RepricingEntry& /*repricingEntry*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(RepricingEntry& repricingEntry, xml_attribute<>* attribute)
{
  return parse(repricingEntry.itinId, attribute, _l->getAttributeName_ItinId()) ||
         parse(repricingEntry.taxPointBegin, attribute, _l->getAttributeName_TaxPointBegin()) ||
         parse(repricingEntry.taxPointEnd, attribute, _l->getAttributeName_TaxPointEnd()) ||
         parse(repricingEntry.repricedAmount, attribute, _l->getAttributeName_RepricedAmount());
}

// LOC CACHE //

template <>
bool
XmlParser::parseAnyChild(Nation& /*nation*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(Nation& nation, xml_attribute<>* attribute)
{
  return parse(nation.id(), attribute, _l->getAttributeName_NationId()) ||
         parse(nation.nationCode(), attribute, _l->getAttributeName_NationCode()) ||
         parse(nation.locCode(), attribute, _l->getAttributeName_LocCode()) ||
         parse(nation.cityCode(), attribute, _l->getAttributeName_CityCode()) ||
         parse(nation.alaskaZone(), attribute, _l->getAttributeName_AlaskaZone()) ||
         parse(nation.state(), attribute, _l->getAttributeName_State()) ||
         parse(nation.currency(), attribute, _l->getAttributeName_Currency());
}

// Currency //
template <>
bool
XmlParser::parseAnyChild(Currency& /*currencyConversion*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(Currency& currency, xml_attribute<>* attribute)
{
  return parse(currency.currencyCode, attribute, _l->getAttributeName_Currency()) ||
         parse(currency.currencyDecimals, attribute, _l->getAttributeName_CurrencyDecimals());
}

// CurrencyConversion //
template <>
bool
XmlParser::parseAnyChild(CurrencyConversion& /*currencyConversion*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(CurrencyConversion& currencyConversion, xml_attribute<>* attribute)
{
  return parse(currencyConversion.fromCurrency, attribute, _l->getAttributeName_FromCurrency()) ||
         parse(currencyConversion.toCurrency, attribute, _l->getAttributeName_ToCurrency()) ||
         parse(currencyConversion.bsr, attribute, _l->getAttributeName_BSR());
}

// IsInLoc CACHE //

template <>
bool
XmlParser::parseAnyChild(IsInLoc& isInLoc, xml_node<>* child)
{
  return parse(isInLoc.locUsages(), child) ||
         parse(isInLoc.paxLocUsages(), child);
}

template <>
bool
XmlParser::parseAnyAttribute(IsInLoc& isInLoc, xml_attribute<>* attribute)
{
  return parse(isInLoc.type(), attribute, _l->getAttributeName_LocType()) ||
         parse(isInLoc.code(), attribute, _l->getAttributeName_LocCode()) ||
         parse(isInLoc.vendor(), attribute, _l->getAttributeName_Vendor()) ||
         parse(isInLoc.isPartial(), attribute, _l->getAttributeName_IsPartial());
}

// LocUsage CACHE //

template <>
bool
XmlParser::parseAnyChild(LocUsage& /*locUsage*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(LocUsage& locUsage, xml_attribute<>* attribute)
{
  return parse(locUsage.locRefId(), attribute, _l->getAttributeName_LocRefId()) ||
         parse(locUsage.airportCode(), attribute, _l->getAttributeName_LocCode()) ||
         parse(locUsage.included(), attribute, _l->getAttributeName_Included());
}

template <>
bool
XmlParser::parseAnyChild(PaxLocUsage& /*locUsage*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(PaxLocUsage& locUsage, xml_attribute<>* attribute)
{
  return parse(locUsage.locCode(), attribute, _l->getAttributeName_LocCode()) ||
         parse(locUsage.included(), attribute, _l->getAttributeName_Included());
}

//  CACHE MILEAGE//

template <>
bool
XmlParser::parseAnyChild(MileageGetterServer& mileage, xml_node<>* child)
{
  return parse(mileage.distances(), child);
}

template <>
bool
XmlParser::parseAnyAttribute(MileageGetterServer& mileage, xml_attribute<>* attribute)
{
  return parse(mileage.geoPathRefId(), attribute, _l->getAttributeName_MileageGeoPathRefId());
}

template <>
bool
XmlParser::parseAnyChild(Distance& /*distance*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(Distance& distance, xml_attribute<>* attribute)
{
  return parse(distance.fromGeoRefId(), attribute, _l->getAttributeName_FromGeoRefId()) ||
         parse(distance.toGeoRefId(), attribute, _l->getAttributeName_ToGeoRefId()) ||
         parse(distance.globalDirection(), attribute, _l->getAttributeName_GlobalDirection()) ||
         parse(distance.miles(), attribute, _l->getAttributeName_Miles());
}

// CarrierFlight CACHE //

template <>
bool
XmlParser::parseAnyChild(CarrierFlight& carrierFlight, xml_node<>* child)
{
  return parse(carrierFlight.segments, child);
}

template <>
bool
XmlParser::parseAnyAttribute(CarrierFlight& carrierFlight, xml_attribute<>* attribute)
{
  return parse(carrierFlight.vendor, attribute, _l->getAttributeName_VENDOR()) ||
         parse(carrierFlight.itemNo, attribute, _l->getAttributeName_ITEMNO());
}

// CarrierFlightSegment CACHE //

template <>
bool
XmlParser::parseAnyChild(CarrierFlightSegment& /*carrierFlightSegment*/, xml_node<>* /*child*/)
{
  return false;
}

namespace
{
struct FltNoConverter
{
  template <typename IntT>
  void operator()(std::string const& str, IntT& destination) const
  {
    if (str.empty())
      destination = 0;
    else if (str[0] == '*')
      destination = -1;
    else
      destination = static_cast<IntT>(atoi(str.c_str()));
  }
};

} // anonymous namespace

template <>
bool
XmlParser::parseAnyAttribute(CarrierFlightSegment& carrierFlightSegment, xml_attribute<>* attribute)
{
  bool ans =
      parse(carrierFlightSegment.marketingCarrier,
            attribute,
            _l->getAttributeName_MARKETINGCARRIER()) ||
      parse(carrierFlightSegment.operatingCarrier,
            attribute,
            _l->getAttributeName_OPERATINGCARRIER()) ||
      parse(carrierFlightSegment.flightFrom,
            attribute,
            _l->getAttributeName_FLT1(),
            FltNoConverter()) ||
      parse(
          carrierFlightSegment.flightTo, attribute, _l->getAttributeName_FLT2(), FltNoConverter());

  if (carrierFlightSegment.flightFrom == -1)
  {
    carrierFlightSegment.flightTo = 9999;
  }
  else if (carrierFlightSegment.flightFrom == 0)
  {
    carrierFlightSegment.flightTo = -1; // empty range
  }
  else if (carrierFlightSegment.flightFrom > 0 && carrierFlightSegment.flightTo == 0)
  {
    carrierFlightSegment.flightTo = carrierFlightSegment.flightFrom;
  }
  return ans;
}

// AKHIFactor CACHE

template <>
bool
XmlParser::parseAnyChild(AKHIFactor& /*AKHIfactor*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(AKHIFactor& AKHIfactor, xml_attribute<>* attribute)
{
  return parse(AKHIfactor.locCode, attribute, _l->getAttributeName_LocCode()) ||
         parse(AKHIfactor.hawaiiPercent, attribute, _l->getAttributeName_HawaiiPercent()) ||
         parse(AKHIfactor.zoneAPercent, attribute, _l->getAttributeName_ZoneAPercent()) ||
         parse(AKHIfactor.zoneBPercent, attribute, _l->getAttributeName_ZoneBPercent()) ||
         parse(AKHIfactor.zoneCPercent, attribute, _l->getAttributeName_ZoneCPercent()) ||
         parse(AKHIfactor.zoneDPercent, attribute, _l->getAttributeName_ZoneDPercent());
}

// CarrierApplication CACHE //

template <>
bool
XmlParser::parseAnyChild(CarrierApplication& carrierApplication, xml_node<>* child)
{
  return parse(carrierApplication.entries, child);
}

template <>
bool
XmlParser::parseAnyAttribute(CarrierApplication& carrierApplication, xml_attribute<>* attribute)
{
  return parse(carrierApplication.vendor, attribute, _l->getAttributeName_VENDOR()) ||
         parse(carrierApplication.itemNo, attribute, _l->getAttributeName_ITEMNO());
}

// CarrierApplEntry CACHE //

template <>
bool
XmlParser::parseAnyChild(CarrierApplicationEntry& /*carrierApplicationEntry*/,
                         xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(CarrierApplicationEntry& carrierApplicationEntry,
                             xml_attribute<>* attribute)
{
  return parse(carrierApplicationEntry.carrier, attribute, _l->getAttributeName_CARRIER()) ||
         parse(carrierApplicationEntry.applind, attribute, _l->getAttributeName_APPLIND());
}

// ServiceBaggage CACHE //

template <>
bool
XmlParser::parseAnyChild(ServiceBaggage& serviceBaggage, xml_node<>* child)
{
  return parse(serviceBaggage.entries, child);
}

template <>
bool
XmlParser::parseAnyAttribute(ServiceBaggage& serviceBaggage, xml_attribute<>* attribute)
{
  return parse(serviceBaggage.vendor, attribute, _l->getAttributeName_VENDOR()) ||
         parse(serviceBaggage.itemNo, attribute, _l->getAttributeName_ITEMNO());
}

// ServiceBaggageEntry CACHE //

template <>
bool
XmlParser::parseAnyChild(ServiceBaggageEntry& /*ServiceBaggageEntry*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(ServiceBaggageEntry& serviceBaggageEntry, xml_attribute<>* attribute)
{
  return parse(serviceBaggageEntry.applTag, attribute, _l->getAttributeName_APPLTAG()) ||
         parse(serviceBaggageEntry.taxTypeSubcode,
               attribute,
               _l->getAttributeName_TAXTYPESUBCODE()) ||
         parse(serviceBaggageEntry.taxCode, attribute, _l->getAttributeName_TAXCODE()) ||
         parse(serviceBaggageEntry.optionalServiceTag, attribute, _l->getAttributeName_SVCTYPE()) ||
         parse(serviceBaggageEntry.group, attribute, _l->getAttributeName_ATTRGROUP()) ||
         parse(serviceBaggageEntry.subGroup, attribute, _l->getAttributeName_ATTRSUBGROUP()) ||
         parse(serviceBaggageEntry.feeOwnerCarrier, attribute, _l->getAttributeName_FEEOWNERCXR());
}

// PassengerTypeCode CACHE //

template <>
bool
XmlParser::parseAnyChild(PassengerTypeCode& passengerTypeCode, xml_node<>* child)
{
  return parse(passengerTypeCode.entries, child);
}

template <>
bool
XmlParser::parseAnyAttribute(PassengerTypeCode& passengerTypeCode, xml_attribute<>* attribute)
{
  return parse(passengerTypeCode.vendor, attribute, _l->getAttributeName_VENDOR()) ||
         parse(passengerTypeCode.itemNo, attribute, _l->getAttributeName_ITEMNO());
}

// PassengerTypeCodeItem CACHE //

template <>
bool
XmlParser::parseAnyChild(PassengerTypeCodeItem& /*passengerTypeCodeItem*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(PassengerTypeCodeItem& passengerTypeCodeItem,
                             xml_attribute<>* attribute)
{
  return parse(passengerTypeCodeItem.applTag, attribute, _l->getAttributeName_APPLTAG()) ||
         parse(passengerTypeCodeItem.passengerType, attribute, _l->getAttributeName_PSGRTYPE()) ||
         parse(passengerTypeCodeItem.maximumAge, attribute, _l->getAttributeName_PSGRMAXAGE()) ||
         parse(passengerTypeCodeItem.minimumAge, attribute, _l->getAttributeName_PSGRMINAGE()) ||
         parse(passengerTypeCodeItem.statusTag, attribute, _l->getAttributeName_PSGRSTATUS()) ||
         parse(passengerTypeCodeItem.location.type(), attribute, _l->getAttributeName_LOCTYPE()) ||
         parse(passengerTypeCodeItem.location.code(), attribute, _l->getAttributeName_LOC()) ||
         parse(passengerTypeCodeItem.matchIndicator, attribute, _l->getAttributeName_PTCMATCHIND());
}

// PaxTypeMapping CACHE //
template <>
bool
XmlParser::parseAnyChild(PaxTypeMapping& paxTypeMapping, xml_node<>* child)
{
  return parse(paxTypeMapping.items, child);
}

template <>
bool
XmlParser::parseAnyAttribute(PaxTypeMapping& paxTypeMapping, xml_attribute<>* attribute)
{
  return parse(paxTypeMapping.vendor, attribute, _l->getAttributeName_VENDOR()) ||
         parse(
             paxTypeMapping.validatingCarrier, attribute, _l->getAttributeName_VALIDATINGCARRIER());
}

// PaxTypeMappingItem CACHE //
template <>
bool
XmlParser::parseAnyChild(PaxTypeMappingItem& /*paxTypeMappingItem*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(PaxTypeMappingItem& paxTypeMappingItem, xml_attribute<>* attribute)
{
  return parse(paxTypeMappingItem.paxFrom, attribute, _l->getAttributeName_PAXFROM()) ||
         parse(paxTypeMappingItem.paxTo, attribute, _l->getAttributeName_PAXTO());
}

// SectorDetail CACHE //

template <>
bool
XmlParser::parseAnyChild(SectorDetail& sectorDetail, xml_node<>* child)
{
  return parse(sectorDetail.entries, child);
}

template <>
bool
XmlParser::parseAnyAttribute(SectorDetail& sectorDetail, xml_attribute<>* attribute)
{
  return parse(sectorDetail.vendor, attribute, _l->getAttributeName_VENDOR()) ||
         parse(sectorDetail.itemNo, attribute, _l->getAttributeName_ITEMNO());
}

// SectorDetailEntry CACHE //

template <>
bool
XmlParser::parseAnyChild(SectorDetailEntry& /*SectorDetailEntry*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(SectorDetailEntry& sectorDetailEntry, xml_attribute<>* attribute)
{
  return parse(sectorDetailEntry.applTag, attribute, _l->getAttributeName_APPLTAG()) ||
         parse(sectorDetailEntry.equipmentCode, attribute, _l->getAttributeName_EQUIPMENTCODE()) ||
         parse(
             sectorDetailEntry.fareOwnerCarrier, attribute, _l->getAttributeName_FAREOWNINGCXR()) ||
         parse(sectorDetailEntry.cabinCode, attribute, _l->getAttributeName_CABINCODE()) ||
         parse(sectorDetailEntry.rule, attribute, _l->getAttributeName_RULE()) ||
         parse(sectorDetailEntry.fareType, attribute, _l->getAttributeName_FARETYPE()) ||
         parse(sectorDetailEntry.reservationCodes[0], attribute, _l->getAttributeName_RBD1()) ||
         parse(sectorDetailEntry.reservationCodes[1], attribute, _l->getAttributeName_RBD2()) ||
         parse(sectorDetailEntry.reservationCodes[2], attribute, _l->getAttributeName_RBD3()) ||
         parse(sectorDetailEntry.tariff.asMutableNumber(), attribute, _l->getAttributeName_FARETARIFF()) ||
         parse(sectorDetailEntry.tariff.asMutableCode(), attribute, _l->getAttributeName_FARETARIFFIND()) ||
         parse(sectorDetailEntry.fareBasisTktDesignator,
               attribute,
               _l->getAttributeName_FAREBASISTKTDESIGNATOR()) ||
         parse(sectorDetailEntry.ticketCode, attribute, _l->getAttributeName_TKTCODE());
}

// ServiceFeeSecurity CACHE //
template <>
bool
XmlParser::parseAnyChild(ServiceFeeSecurity& serviceFeeSecurity, xml_node<>* child)
{
  return parse(serviceFeeSecurity.entries, child);
}

template <>
bool
XmlParser::parseAnyAttribute(ServiceFeeSecurity& serviceFeeSecurity, xml_attribute<>* attribute)
{
  return parse(serviceFeeSecurity.vendor, attribute, _l->getAttributeName_VENDOR()) ||
         parse(serviceFeeSecurity.itemNo, attribute, _l->getAttributeName_ITEMNO());
}

// ServiceFeeSecurityItem CACHE //
template <>
bool
XmlParser::parseAnyChild(ServiceFeeSecurityItem& /*serviceFeeSecurityItem*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(ServiceFeeSecurityItem& serviceFeeSecurityItem,
                             xml_attribute<>* attribute)
{
  return parse(serviceFeeSecurityItem.travelAgencyIndicator,
               attribute,
               _l->getAttributeName_TRAVELAGENCYIND()) ||
         parse(
             serviceFeeSecurityItem.carrierGdsCode, attribute, _l->getAttributeName_CXRGDSCODE()) ||
         parse(serviceFeeSecurityItem.dutyFunctionCode,
               attribute,
               _l->getAttributeName_DUTYFUNCTIONCODE()) ||
         parse(serviceFeeSecurityItem.location.type(), attribute, _l->getAttributeName_LOCTYPE()) ||
         parse(serviceFeeSecurityItem.location.code(), attribute, _l->getAttributeName_LOC()) ||
         parse(serviceFeeSecurityItem.codeType, attribute, _l->getAttributeName_CODETYPE()) ||
         parse(serviceFeeSecurityItem.code, attribute, _l->getAttributeName_CODE()) ||
         parse(serviceFeeSecurityItem.viewBookTktInd, attribute, _l->getAttributeName_VIEWBOOKTKTIND());
}

// TaxRounding CACHE //
template <>
bool
XmlParser::parseAnyChild(TaxRounding& /*TaxRounding*/, xml_node<>* /*child*/)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(TaxRounding& taxRounding, xml_attribute<>* attribute)
{
  return parse(taxRounding.taxRoundingUnit(), attribute, _l->getAttributeName_TAXROUNDUNIT()) ||
         parse(
             taxRounding.taxUnitDecimals(), attribute, _l->getAttributeName_TAXROUNDUNITNODEC()) ||
         parse(taxRounding.taxRoundingDir(), attribute, _l->getAttributeName_ROUNDINGRULE()) ||
         parse(taxRounding.nation(), attribute, _l->getAttributeName_TAXNATION());
}

XmlParser&
XmlParser::parse(InputRequest& request,
                 XmlCache& xmlCache,
                 rapidxml::xml_document<> const& xml,
                 const XmlTagsList& tagsList)
{
  _l = &tagsList;
  rapidxml::xml_node<>* rootNode = xml.first_node();
  InputRequestWithCache inputRequestWithCache = { request, xmlCache };
  parse(inputRequestWithCache, rootNode);
  return *this;
}

// ChangeFee //
template <>
bool
XmlParser::parseAnyChild(InputChangeFee&, xml_node<>*)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputChangeFee& changeFee, xml_attribute<>* attribute)
{
  return parse(changeFee._id, attribute, _l->getAttributeName_Id()) ||
         parse(changeFee._amount, attribute, _l->getAttributeName_Amount());
}

// TicketingFee //
template <>
bool
XmlParser::parseAnyChild(InputTicketingFee&, xml_node<>*)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputTicketingFee& ticketingFee, xml_attribute<>* attribute)
{
  return parse(ticketingFee._id, attribute, _l->getAttributeName_Id()) ||
         parse(ticketingFee._amount, attribute, _l->getAttributeName_Amount()) ||
         parse(ticketingFee._taxAmount, attribute, _l->getAttributeName_TaxAmount()) ||
         parse(ticketingFee._subCode, attribute, _l->getAttributeName_ServiceSubTypeCode());
}

// TicketingFeePath //
template <>
bool
XmlParser::parseAnyChild(InputTicketingFeePath& ticketingFeePath, xml_node<>* child)
{
  return parse(ticketingFeePath._ticketingFeeUsages, child);
}

template <>
bool
XmlParser::parseAnyAttribute(InputTicketingFeePath& ticketingFeePath, xml_attribute<>* attribute)
{
  return parse(ticketingFeePath._id, attribute, _l->getAttributeName_Id());
}

// TicketingFeeUsage //
template <>
bool
XmlParser::parseAnyChild(InputTicketingFeeUsage&, xml_node<>*)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(InputTicketingFeeUsage& ticketingFeeUsage, xml_attribute<>* attribute)
{
  return parse(ticketingFeeUsage._ticketingFeeRefId, attribute, _l->getAttributeName_TicketingFeeRefId());
}


// Customer //
template <>
bool
XmlParser::parseAnyChild(Customer&, xml_node<>*)
{
  return false;
}

template <>
bool
XmlParser::parseAnyAttribute(Customer& customer, xml_attribute<>* attribute)
{
  return parse(customer._pcc, attribute, _l->getAttributeName_AgentPCC()) ||
    parse(customer._exemptDuG3, attribute, _l->getAttributeName_EXEMPTDUFORG3()) ||
    parse(customer._exemptDuT4, attribute, _l->getAttributeName_EXEMPTDUFORT4()) ||
    parse(customer._exemptDuJJ, attribute, _l->getAttributeName_EXEMPTDUFORJJ());
}


} // namespace tax
