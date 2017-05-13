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
#pragma once

#include "TestServer/Xform/XmlTagsList.h"

namespace tax
{
class NaturalXmlTagsList : public XmlTagsList
{
public:
  std::string getTagName(InputRequestWithCache*) const override { return "TaxRq"; }
  std::string getTagName(InputRequest*) const override { return "TaxRq"; }

  std::string getTagName(InputApplyOn*) const override { return "ApplyOn"; }
  std::string getTagName(InputCalculationRestriction*) const override { return "CalcRestrictions"; }
  std::string getTagName(InputChangeFee*) const override { return "ChangeFee"; }
  std::string getTagName(InputDiagnosticCommand*) const override { return "Diagnostic"; }
  std::string getTagName(InputExemptedRule*) const override { return "ExemptedRule"; }
  std::string getTagName(InputFare*) const override { return "Fare"; }
  std::string getTagName(InputFarePath*) const override { return "FarePath"; }
  std::string getTagName(InputFareUsage*) const override { return "FareUsage"; }
  std::string getTagName(InputFlight*) const override { return "Flight"; }
  std::string getTagName(InputFlightPath*) const override { return "FlightPath"; }
  std::string getTagName(InputFlightUsage*) const override { return "FlightUsage"; }
  std::string getTagName(InputGeo*) const override { return "Geo"; }
  std::string getTagName(InputGeoPath*) const override { return "GeoPath"; }
  std::string getTagName(InputPrevTicketGeoPath*) const override { return "PrevTicketGeoPath"; }
  std::string getTagName(InputGeoPathMapping*) const override { return "GeoPathMapping"; }
  std::string getTagName(InputItin*) const override { return "Itin"; }
  std::string getTagName(InputMap*) const override { return "Map"; }
  std::string getTagName(InputMapping*) const override { return "Mapping"; }
  std::string getTagName(InputOptionalService*) const override { return "OptionalService"; }
  std::string getTagName(InputOptionalServicePath*) const override { return "OptionalServicePath"; }
  std::string getTagName(InputOptionalServiceUsage*) const override
  {
    return "OptionalServiceUsage";
  }
  std::string getTagName(InputParameter*) const override { return "Parameter"; }
  std::string getTagName(InputPassenger*) const override { return "Passenger"; }
  std::string getTagName(InputPointOfSale*) const override { return "PointOfSale"; }
  std::string getTagName(InputProcessingOptions*) const override { return "ProcessingOptions"; }
  std::string getTagName(InputCalculationRestrictionTax*) const override { return "Tax"; }
  std::string getTagName(InputTaxDetailsLevel*) const override { return "TaxDetailsLevel"; }
  std::string getTagName(InputTicketingFee*) const override { return "TicketingFee"; }
  std::string getTagName(InputTicketingFeePath*) const override { return "TicketingFeePath"; }
  std::string getTagName(InputTicketingFeeUsage*) const override { return "TicketingFeeUsage"; }
  std::string getTagName(InputTicketingOptions*) const override { return "TicketingOptions"; }
  std::string getTagName(InputYqYr*) const override { return "YqYr"; }
  std::string getTagName(InputYqYrPath*) const override { return "YqYrPath"; }
  std::string getTagName(InputYqYrUsage*) const override { return "YqYrUsage"; }

  std::string getAttributeName_AgentAirlineDept() const override { return "AgentAirlineDept"; }
  std::string getAttributeName_AgentCity() const override { return "AgentCity"; }
  std::string getAttributeName_AgentOfficeDesignator() const override
  {
    return "AgentOfficeDesignator";
  }
  std::string getAttributeName_AgentPCC() const override { return "AgentPCC"; }
  std::string getAttributeName_AllDetails() const override { return "AllDetails"; }
  std::string getAttributeName_Amount() const override { return "Amount"; }
  std::string getAttributeName_ArrivalDateShift() const override { return "ArrivalDateShift"; }
  std::string getAttributeName_ArrivalTime() const override { return "ArrivalTime"; }
  std::string getAttributeName_BasisCode() const override { return "BasisCode"; }
  std::string getAttributeName_BookingCodeType() const override { return "BookingCodeType"; }
  std::string getAttributeName_BufferZoneInd() const override { return "BufferZoneInd"; }
  std::string getAttributeName_CabinCode() const override { return "CabinCode"; }
  std::string getAttributeName_CalcDetails() const override { return "CalcDetails"; }
  std::string getAttributeName_CarrierCode() const override { return "CarrierCode"; }
  std::string getAttributeName_Code() const override { return "Code"; }
  std::string getAttributeName_ConnectionDateShift() const override
  {
    return "ConnectionDateShift";
  }
  std::string getAttributeName_DateOfBirth() const override { return "DateOfBirth"; }
  std::string getAttributeName_DepartureTime() const override { return "DepartureTime"; }
  std::string getAttributeName_Directionality() const override { return "Directionality"; }
  std::string getAttributeName_DutyCode() const override { return "DutyCode"; }
  std::string getAttributeName_EchoToken() const override { return "EchoToken"; }
  std::string getAttributeName_Employment() const override { return "Employment"; }
  std::string getAttributeName_Equipment() const override { return "Equipment"; }
  std::string getAttributeName_ExchangeReissueDetails() const override
  {
    return "ExchangeReissueDetails";
  }
  std::string getAttributeName_FarePathGeoPathMappingRefId() const override
  {
    return "FarePathGeoPathMappingRefId";
  }
  std::string getAttributeName_FarePathRefId() const override { return "FarePathRefId"; }
  std::string getAttributeName_FareRefId() const override { return "FareRefId"; }
  std::string getAttributeName_FlightRefId() const override { return "FlightRefId"; }
  std::string getAttributeName_FlightPathRefId() const override { return "FlightPathRefId"; }
  std::string getAttributeName_FormOfPayment() const override { return "FormOfPayment"; }
  std::string getAttributeName_FunctionCode() const override { return "FunctionCode"; }
  std::string getAttributeName_GeoDetails() const override { return "GeoDetails"; }
  std::string getAttributeName_GeoPathRefId() const override { return "GeoPathRefId"; }
  std::string getAttributeName_GeoRefId() const override { return "GeoRefId"; }
  std::string getAttributeName_IATANumber() const override { return "IATANumber"; }
  std::string getAttributeName_Id() const override { return "Id"; }
  std::string getAttributeName_IsNetRemitAvailable() const override
  {
    return "IsNetRemitAvailable";
  }
  std::string getAttributeName_Loc() const override { return "Loc"; }
  std::string getAttributeName_MarketingCarrier() const override { return "MarketingCarrier"; }
  std::string getAttributeName_MarketingCarrierFlightNumber() const override
  {
    return "MarketingCarrierFlightNumber";
  }
  std::string getAttributeName_MarkupAmount() const override { return "MarkupAmount"; }
  std::string getAttributeName_Name() const override { return "Name"; }
  std::string getAttributeName_Nation() const override { return "Nation"; }
  std::string getAttributeName_Nationality() const override { return "Nationality"; }
  std::string getAttributeName_OneWayRoundTrip() const override { return "OneWayRoundTrip"; }
  std::string getAttributeName_OperatingCarrier() const override { return "OperatingCarrier"; }
  std::string getAttributeName_OptionalServicePathGeoPathMappingRefId() const override
  {
    return "OptionalServicePathGeoPathMappingRefId";
  }
  std::string getAttributeName_OptionalServicePathRefId() const override
  {
    return "OptionalServicePathRefId";
  }
  std::string getAttributeName_OptionalServiceRefId() const override
  {
    return "OptionalServiceRefId";
  }
  std::string getAttributeName_OutputPassengerCode() const override
  {
    return "OutputPassengerCode";
  }
  std::string getAttributeName_OwnerCarrier() const override { return "OwnerCarrier"; }
  std::string getAttributeName_PartitionId() const override { return "PartitionId"; }
  std::string getAttributeName_PassengerCode() const override { return "PassengerCode"; }
  std::string getAttributeName_PassengerRefId() const override { return "PassengerRefId"; }
  std::string getAttributeName_PaymentCurrency() const override { return "PaymentCurrency"; }
  std::string getAttributeName_PointOfDeliveryLoc() const override { return "PointOfDeliveryLoc"; }
  std::string getAttributeName_PointOfSaleLoc() const override { return "PointOfSaleLoc"; }
  std::string getAttributeName_PointOfSaleRefId() const override { return "PointOfSaleRefId"; }
  std::string getAttributeName_ProcessingGroup() const override { return "ProcessingGroup"; }
  std::string getAttributeName_ReservationDesignator() const override
  {
    return "ReservationDesignator";
  }
  std::string getAttributeName_Residency() const override { return "Residency"; }
  std::string getAttributeName_Rule() const override { return "Rule"; }
  std::string getAttributeName_RuleRefId() const override { return "RuleRefId"; }
  std::string getAttributeName_SellAmount() const override { return "SellAmount"; }
  std::string getAttributeName_ServiceSubTypeCode() const override { return "ServiceSubTypeCode"; }
  std::string getAttributeName_SkipDateTime() const override { return "SkipDateTime"; }
  std::string getAttributeName_StateCode() const override { return "StateCode"; }
  std::string getAttributeName_SvcGroup() const override { return "SvcGroup"; }
  std::string getAttributeName_SvcSubGroup() const override { return "SvcSubGroup"; }
  std::string getAttributeName_Tariff() const override { return "Tariff"; }
  std::string getAttributeName_TariffInd() const override { return "TariffInd"; }
  std::string getAttributeName_TaxAmount() const override { return "TaxAmount"; }
  std::string getAttributeName_TaxIncluded() const override { return "TaxIncluded"; }
  std::string getAttributeName_TaxOnExchangeReissueDetails() const override
  {
    return "ExchangeReissueDetails";
  }
  std::string getAttributeName_TaxOnFaresDetails() const override { return "TaxOnFaresDetails"; }
  std::string getAttributeName_TaxOnOptionalServicesDetails() const override
  {
    return "TaxOnOptionalServiceDetails";
  }
  std::string getAttributeName_TaxOnTaxDetails() const override { return "TaxOnTaxDetails"; }
  std::string getAttributeName_TaxOnYQYRDetails() const override { return "TaxOnYQYRDetails"; }
  std::string getAttributeName_TicketingDate() const override { return "TicketingDate"; }
  std::string getAttributeName_TicketingPoint() const override { return "TicketingPoint"; }
  std::string getAttributeName_TicketingTime() const override { return "TicketingTime"; }
  std::string getAttributeName_TotalAmount() const override { return "TotalAmount"; }
  std::string getAttributeName_TotalAmountBeforeDiscount() const override
  {
    return "TotalAmountBeforeDiscount";
  }
  std::string getAttributeName_TravelOriginDate() const override { return "TravelOriginDate"; }
  std::string getAttributeName_Type() const override { return "Type"; }
  std::string getAttributeName_TypeCode() const override { return "TypeCode"; }
  std::string getAttributeName_UnticketedTransfer() const override { return "UnticketedTransfer"; }
  std::string getAttributeName_ValidatingCarrier() const override { return "ValidatingCarrier"; }
  std::string getAttributeName_Value() const override { return "Value"; }
  std::string getAttributeName_VendorCrsCode() const override { return "VendorCrsCode"; }
  std::string getAttributeName_YqYrPathGeoPathMappingRefId() const override
  {
    return "YqYrPathGeoPathMappingRefId";
  }
  std::string getAttributeName_YqYrPathRefId() const override { return "YqYrPathRefId"; }
  std::string getAttributeName_YqYrRefId() const override { return "YqYrRefId"; }
  std::string getAttributeName_ForcedConnection() const override { return "ForcedConnection"; }
  std::string getAttributeName_ChangeFeeRefId() const override { return "ChangeFeeRefId"; }
  std::string getAttributeName_TicketingFeeRefId() const override { return "TicketingFeeRefId"; }

  std::string getAttributeName_GeoPathsSize() const override { return "GeoPathsSize"; }
  std::string getAttributeName_GeoPathMappingsSize() const override
  {
    return "GeoPathMappingsSize";
  }
  std::string getAttributeName_FaresSize() const override { return "FaresSize"; }
  std::string getAttributeName_FarePathsSize() const override { return "FarePathsSize"; }
  std::string getAttributeName_FlightsSize() const override { return "FlightsSize"; }
  std::string getAttributeName_FlightPathsSize() const override { return "FlightPathsSize"; }
  std::string getAttributeName_YqYrsSize() const override { return "YqYrsSize"; }
  std::string getAttributeName_YqYrPathsSize() const override { return "YqYrPathsSize"; }
  std::string getAttributeName_ItinsSize() const override { return "ItinsSize"; }
  std::string getAttributeName_ApplyUSCAGrouping() const override { return "ApplyUSCAGrouping"; }

  std::string getTagName(OutputBaggageDetails*) const override { return "TaxOnBaggageDetails"; }
  std::string getTagName(OutputCalcDetails*) const override { return "CalcDetails"; }
  std::string getTagName(DiagnosticResponse*) const override { return "Diagnostic"; }
  std::string getTagName(OutputDiagnostic*) const override { return "Diagnostic"; }
  std::string getTagName(ErrorMessage*) const override { return "Error"; }
  std::string getTagName(OutputError*) const override { return "Error"; }
  std::string getTagName(OutputFaresDetails*) const override { return "TaxOnFaresDetails"; }
  std::string getTagName(OutputGeoDetails*) const override { return "GeoDetails"; }
  std::string getTagName(ItinPayments*) const override { return "Itin"; }
  std::string getTagName(OutputItin*) const override { return "Itin"; }
  std::string getTagName(ItinsPayments*) const override { return "Itins"; }
  std::string getTagName(OutputItins*) const override { return "Itins"; }
  std::string getTagName(Message*) const override { return "Message"; }
  std::string getTagName(OutputMessage*) const override { return "Message"; }
  std::string getTagName(OutputOBDetails*) const override { return "OBDetails"; }
  std::string getTagName(OutputOCDetails*) const override { return "TaxOnOptionalServiceDetails"; }
  std::string getTagName(OutputOptionalServiceDetails*) const override
  {
    return "OptionalServiceDetails";
  }
  std::string getTagName(OutputTax*) const override { return "Tax"; }
  std::string getTagName(Payment*) const override { return "Tax"; }
  std::string getTagName(PaymentDetail*) const override { return "TaxDetail"; }
  std::string getTagName(OutputTaxDetails*) const override { return "TaxDetails"; }
  std::string getTagName(OutputTaxDetailsRef*) const override { return "ElementRef"; }
  std::string getTagName(OutputTaxGroup*) const override { return "TaxGroup"; }
  std::string getTagName(OutputTaxOnTaxDetails*) const override { return "TaxOnTaxDetails"; }
  std::string getTagName(OutputResponse*) const override { return "TaxRs"; }
  std::string getTagName(Response*) const override { return "TaxRs"; }
  std::string getTagName(OutputYQYRDetails*) const override { return "TaxOnYQYRDetails"; }

  std::string getTagName(BCHOutputResponse*) const override { return "BCHTaxRs"; }
  std::string getTagName(BCHOutputItin*) const override { return "BCHItin"; }
  std::string getTagName(BCHOutputItinPaxDetail*) const override { return "BCHPax"; }
  std::string getTagName(BCHOutputTaxDetail*) const override { return "BCHTaxDetail"; }

  std::string getOutputAttributeName_Carrier() const override { return "Carrier"; }
  std::string getOutputAttributeName_Code() const override { return "Code"; }
  std::string getOutputAttributeName_Content() const override { return "Content"; }
  std::string getOutputAttributeName_EchoToken() const override { return "EchoToken"; }
  std::string getOutputAttributeName_ElementRefId() const override { return "ElementRefId"; }
  std::string getOutputAttributeName_GST() const override { return "GST"; }
  std::string getOutputAttributeName_Id() const override { return "Id"; }
  std::string getOutputAttributeName_JourneyLoc1() const override { return "JourneyLoc1"; }
  std::string getOutputAttributeName_JourneyLoc2() const override { return "JourneyLoc2"; }
  std::string getOutputAttributeName_Name() const override { return "Name"; }
  std::string getOutputAttributeName_Nation() const override { return "Nation"; }
  std::string getOutputAttributeName_OptionalServiceType() const override
  {
    return "OptionalServiceType";
  }
  std::string getOutputAttributeName_PaymentAmt() const override { return "PaymentAmt"; }
  std::string getOutputAttributeName_PaymentCur() const override { return "PaymentCur"; }
  std::string getOutputAttributeName_PercentFlatTag() const override { return "PercentFlatTag"; }
  std::string getOutputAttributeName_PercentageFlatTag() const override
  {
    return "PercentageFlatTag";
  }
  std::string getOutputAttributeName_PointOfDeliveryLoc() const override
  {
    return "PointOfDeliveryLoc";
  }
  std::string getOutputAttributeName_PointOfSaleLoc() const override { return "PointOfSaleLoc"; }
  std::string getOutputAttributeName_PointOfTicketingLoc() const override
  {
    return "PointOfTicketingLoc";
  }
  std::string getOutputAttributeName_PublishedAmt() const override { return "PublishedAmt"; }
  std::string getOutputAttributeName_PublishedCur() const override { return "PublishedCur"; }
  std::string getOutputAttributeName_SabreCode() const override { return "SabreCode"; }
  std::string getOutputAttributeName_SeqNo() const override { return "SeqNo"; }
  std::string getOutputAttributeName_ServiceSubTypeCode() const override
  {
    return "ServiceSubTypeCode";
  }
  std::string getOutputAttributeName_SvcGroup() const override { return "SvcGroup"; }
  std::string getOutputAttributeName_SvcSubGroup() const override { return "SvcSubGroup"; }
  std::string getOutputAttributeName_TaxAmt() const override { return "TaxAmt"; }
  std::string getOutputAttributeName_TaxCode() const override { return "TaxCode"; }
  std::string getOutputAttributeName_TaxCurToPaymentCurBSR() const override
  {
    return "TaxCurToPaymentCurBSR";
  }
  std::string getOutputAttributeName_TaxRoundingUnit() const override { return "TaxRoundingUnit"; }
  std::string getOutputAttributeName_TaxRoundingDir() const override { return "TaxRoundingDir"; }
  std::string getOutputAttributeName_TaxEquivAmt() const override { return "TaxEquivAmt"; }
  std::string getOutputAttributeName_TaxEquivCurr() const override { return "TaxEquivCurr"; }
  std::string getOutputAttributeName_TaxGroupId() const override { return "TaxGroupRefId"; }
  std::string getOutputAttributeName_TaxGroupType() const override { return "TaxGroupType"; }
  std::string getOutputAttributeName_TaxLabel() const override { return "TaxLabel"; }
  std::string getOutputAttributeName_TaxOnOptionalServiceGroupRefId() const override
  {
    return "TaxOnOptionalServiceGroupRefId";
  }
  std::string getOutputAttributeName_TaxOnBaggageGroupRefId() const override
  {
    return "TaxOnBaggageGroupRefId";
  }
  std::string getOutputAttributeName_TaxOnChangeFeeGroupRefId() const override
  {
    return "TaxOnChangeFeeGroupRefId";
  }
  std::string getOutputAttributeName_TaxPointIndexBegin() const override
  {
    return "TaxPointIndexBegin";
  }
  std::string getOutputAttributeName_TaxPointIndexEnd() const override
  {
    return "TaxPointIndexEnd";
  }
  std::string getOutputAttributeName_TaxPointLoc1() const override { return "TaxPointLoc1"; }
  std::string getOutputAttributeName_TaxPointLoc1Index() const override
  {
    return "TaxPointLoc1Index";
  }
  std::string getOutputAttributeName_TaxPointLoc2() const override { return "TaxPointLoc2"; }
  std::string getOutputAttributeName_TaxPointLoc2Index() const override
  {
    return "TaxPointLoc2Index";
  }
  std::string getOutputAttributeName_TaxPointLoc3() const override { return "TaxPointLoc3"; }
  std::string getOutputAttributeName_TaxPointLocBegin() const override
  {
    return "TaxPointLocBegin";
  }
  std::string getOutputAttributeName_TaxPointLocEnd() const override { return "TaxPointLocEnd"; }
  std::string getOutputAttributeName_TaxPointTag() const override { return "TaxPointTag"; }
  std::string getOutputAttributeName_TaxType() const override { return "TaxType"; }
  std::string getOutputAttributeName_TotalAmt() const override { return "TotalAmt"; }
  std::string getOutputAttributeName_Type() const override { return "Type"; }
  std::string getOutputAttributeName_TaxableUnitTags() const override { return "TaxableUnitTags"; }
  std::string getOutputAttributeName_UnticketedPoint() const override { return "UnticketedPoint"; }

  std::string getOutputAttributeName_BCHItinId() const override { return "Id"; }
  std::string getOutputAttributeName_BCHPaxType() const override { return "Code"; }
  std::string getOutputAttributeName_BCHPaxCount() const override { return "Count"; }
  std::string getOutputAttributeName_BCHPaxTotalAmount() const override { return "TotalAmount"; }
  std::string getOutputAttributeName_BCHPaxTaxes() const override { return "Taxes"; }
  std::string getOutputAttributeName_BCHTaxId() const override { return "Id"; }
  std::string getOutputAttributeName_BCHTaxCode() const override { return "Code"; }
  std::string getOutputAttributeName_BCHTaxAmount() const override { return "Amount"; }
  std::string getOutputAttributeName_BCHTaxAmountAdjusted() const override { return "AmountAdjusted"; }
  std::string getOutputAttributeName_BCHTaxDescription() const override { return "Description"; }
};

} // namespace tax
