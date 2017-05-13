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
class Xml2TagsList : public XmlTagsList
{
public:
  std::string getTagName(InputRequestWithCache*) const override { return "TAX"; }
  std::string getTagName(InputRequest*) const override { return "TAX"; }

  std::string getTagName(InputApplyOn*) const override { return "APP"; }
  std::string getTagName(InputCalculationRestriction*) const override { return "CA1"; }
  std::string getTagName(InputChangeFee*) const override { return "CH1"; }
  std::string getTagName(InputDiagnosticCommand*) const override { return "DI1"; }
  std::string getTagName(InputExemptedRule*) const override { return "EX1"; }
  std::string getTagName(InputFare*) const override { return "FA1"; }
  std::string getTagName(InputFarePath*) const override { return "FA2"; }
  std::string getTagName(InputFareUsage*) const override { return "FA3"; }
  std::string getTagName(InputFlight*) const override { return "FL1"; }
  std::string getTagName(InputFlightPath*) const override { return "FL2"; }
  std::string getTagName(InputFlightUsage*) const override { return "FL3"; }
  std::string getTagName(InputGeo*) const override { return "GE1"; }
  std::string getTagName(InputGeoPath*) const override { return "GE2"; }
  std::string getTagName(InputPrevTicketGeoPath*) const override { return "GE4"; }
  std::string getTagName(InputGeoPathMapping*) const override { return "GE3"; }
  std::string getTagName(InputItin*) const override { return "IT1"; }
  std::string getTagName(InputMap*) const override { return "MA1"; }
  std::string getTagName(InputMapping*) const override { return "MA2"; }
  std::string getTagName(InputOptionalService*) const override { return "OP1"; }
  std::string getTagName(InputOptionalServicePath*) const override { return "OP2"; }
  std::string getTagName(InputOptionalServiceUsage*) const override { return "OP3"; }
  std::string getTagName(InputParameter*) const override { return "PA1"; }
  std::string getTagName(InputPassenger*) const override { return "PA2"; }
  std::string getTagName(InputPointOfSale*) const override { return "PO1"; }
  std::string getTagName(InputProcessingOptions*) const override { return "PR1"; }
  std::string getTagName(InputCalculationRestrictionTax*) const override { return "TA1"; }
  std::string getTagName(InputTaxDetailsLevel*) const override { return "TAL"; }
  std::string getTagName(InputTicketingFee*) const override { return "TF2"; }
  std::string getTagName(InputTicketingFeePath*) const override { return "TF3"; }
  std::string getTagName(InputTicketingFeeUsage*) const override { return "TF4"; }
  std::string getTagName(InputTicketingOptions*) const override { return "TI1"; }
  std::string getTagName(InputYqYr*) const override { return "YQ1"; }
  std::string getTagName(InputYqYrPath*) const override { return "YQ2"; }
  std::string getTagName(InputYqYrUsage*) const override { return "YQ3"; }

  std::string getAttributeName_AgentAirlineDept() const override { return "A80"; }
  std::string getAttributeName_AgentCity() const override { return "AG1"; }
  std::string getAttributeName_AgentOfficeDesignator() const override { return "AE1"; }
  std::string getAttributeName_AgentPCC() const override { return "AG2"; }
  std::string getAttributeName_AllDetails() const override { return "AL1"; }
  std::string getAttributeName_Amount() const override { return "AM1"; }
  std::string getAttributeName_ArrivalDateShift() const override { return "AR1"; }
  std::string getAttributeName_ArrivalTime() const override { return "AR2"; }
  std::string getAttributeName_BasisCode() const override { return "BA1"; }
  std::string getAttributeName_BookingCodeType() const override { return "BO1"; }
  std::string getAttributeName_BufferZoneInd() const override { return "BZ1"; }
  std::string getAttributeName_CabinCode() const override { return "CA1"; }
  std::string getAttributeName_CalcDetails() const override { return "CA2"; }
  std::string getAttributeName_CarrierCode() const override { return "CA4"; }
  std::string getAttributeName_Code() const override { return "CO1"; }
  std::string getAttributeName_ConnectionDateShift() const override { return "CO2"; }
  std::string getAttributeName_DateOfBirth() const override { return "DA1"; }
  std::string getAttributeName_DepartureTime() const override { return "DE1"; }
  std::string getAttributeName_Directionality() const override { return "DI1"; }
  std::string getAttributeName_DutyCode() const override { return "DU1"; }
  std::string getAttributeName_EchoToken() const override { return "ET1"; }
  std::string getAttributeName_Employment() const override { return "EM1"; }
  std::string getAttributeName_Equipment() const override { return "EQ1"; }
  std::string getAttributeName_ExchangeReissueDetails() const override { return "EXC"; }
  std::string getAttributeName_FarePathGeoPathMappingRefId() const override { return "FA1"; }
  std::string getAttributeName_FarePathRefId() const override { return "FA2"; }
  std::string getAttributeName_FareRefId() const override { return "FA3"; }
  std::string getAttributeName_FlightRefId() const override { return "FL1"; }
  std::string getAttributeName_FlightPathRefId() const override { return "FL4"; }
  std::string getAttributeName_FormOfPayment() const override { return "FO1"; }
  std::string getAttributeName_FunctionCode() const override { return "FU1"; }
  std::string getAttributeName_GeoDetails() const override { return "GE0"; }
  std::string getAttributeName_GeoPathRefId() const override { return "GE1"; }
  std::string getAttributeName_GeoRefId() const override { return "GE2"; }
  std::string getAttributeName_IATANumber() const override { return "IA1"; }
  std::string getAttributeName_Id() const override { return "ID1"; }
  std::string getAttributeName_IsNetRemitAvailable() const override { return "IS1"; }
  std::string getAttributeName_Loc() const override { return "LO1"; }
  std::string getAttributeName_MarketingCarrier() const override { return "MA1"; }
  std::string getAttributeName_MarketingCarrierFlightNumber() const override { return "MA2"; }
  std::string getAttributeName_MarkupAmount() const override { return "MA3"; }
  std::string getAttributeName_Name() const override { return "NA1"; }
  std::string getAttributeName_Nation() const override { return "NA2"; }
  std::string getAttributeName_Nationality() const override { return "NA3"; }
  std::string getAttributeName_OneWayRoundTrip() const override { return "ON1"; }
  std::string getAttributeName_OperatingCarrier() const override { return "OP1"; }
  std::string getAttributeName_OptionalServicePathGeoPathMappingRefId() const override
  {
    return "OP2";
  }
  std::string getAttributeName_OptionalServicePathRefId() const override { return "OP3"; }
  std::string getAttributeName_OptionalServiceRefId() const override { return "OP4"; }
  std::string getAttributeName_OutputPassengerCode() const override { return "OU1"; }
  std::string getAttributeName_OwnerCarrier() const override { return "CA3"; }
  std::string getAttributeName_PartitionId() const override { return "PA1"; }
  std::string getAttributeName_PassengerCode() const override { return "PA2"; }
  std::string getAttributeName_PassengerRefId() const override { return "PA3"; }
  std::string getAttributeName_PaymentCurrency() const override { return "PA4"; }
  std::string getAttributeName_PointOfDeliveryLoc() const override { return "PO1"; }
  std::string getAttributeName_PointOfSaleLoc() const override { return "PO2"; }
  std::string getAttributeName_PointOfSaleRefId() const override { return "PO4"; }
  std::string getAttributeName_ProcessingGroup() const override { return "PR2"; }
  std::string getAttributeName_ReservationDesignator() const override { return "RE1"; }
  std::string getAttributeName_Residency() const override { return "RE2"; }
  std::string getAttributeName_Rule() const override { return "RU1"; }
  std::string getAttributeName_RuleRefId() const override { return "RU2"; }
  std::string getAttributeName_SellAmount() const override { return "SE1"; }
  std::string getAttributeName_ServiceSubTypeCode() const override { return "SU1"; }
  std::string getAttributeName_SkipDateTime() const override { return "SK1"; }
  std::string getAttributeName_StateCode() const override { return "ST2"; }
  std::string getAttributeName_SvcGroup() const override { return "SG1"; }
  std::string getAttributeName_SvcSubGroup() const override { return "SG2"; }
  std::string getAttributeName_Tariff() const override { return "TA1"; }
  std::string getAttributeName_TariffInd() const override { return "TA2"; }
  std::string getAttributeName_TaxAmount() const override { return "TA3"; }
  std::string getAttributeName_TaxIncluded() const override { return "TX1"; }
  std::string getAttributeName_TaxOnExchangeReissueDetails() const override { return "TD1"; }
  std::string getAttributeName_TaxOnFaresDetails() const override { return "TD2"; }
  std::string getAttributeName_TaxOnOptionalServicesDetails() const override { return "TD3"; }
  std::string getAttributeName_TaxOnTaxDetails() const override { return "TD4"; }
  std::string getAttributeName_TaxOnYQYRDetails() const override { return "TD5"; }
  std::string getAttributeName_TicketingDate() const override { return "TI1"; }
  std::string getAttributeName_TicketingPoint() const override { return "TI2"; }
  std::string getAttributeName_TicketingTime() const override { return "TI3"; }
  std::string getAttributeName_TotalAmount() const override { return "TO3"; }
  std::string getAttributeName_TotalAmountBeforeDiscount() const override { return "TO4"; }
  std::string getAttributeName_TravelOriginDate() const override { return "TR1"; }
  std::string getAttributeName_Type() const override { return "TY1"; }
  std::string getAttributeName_TypeCode() const override { return "TY2"; }
  std::string getAttributeName_UnticketedTransfer() const override { return "UN1"; }
  std::string getAttributeName_ValidatingCarrier() const override { return "VA1"; }
  std::string getAttributeName_Value() const override { return "VA2"; }
  std::string getAttributeName_VendorCrsCode() const override { return "VE2"; }
  std::string getAttributeName_YqYrPathGeoPathMappingRefId() const override { return "YQ1"; }
  std::string getAttributeName_YqYrPathRefId() const override { return "YQ2"; }
  std::string getAttributeName_YqYrRefId() const override { return "YQ3"; }
  std::string getAttributeName_ForcedConnection() const override { return "FC1"; }
  std::string getAttributeName_ChangeFeeRefId() const override { return "CF1"; }
  std::string getAttributeName_TicketingFeeRefId() const override { return "TF1"; }
  std::string getAttributeName_GeoPathsSize() const override { return "GP1"; }
  std::string getAttributeName_GeoPathMappingsSize() const override { return "GM1"; }
  std::string getAttributeName_FaresSize() const override { return "FA4"; }
  std::string getAttributeName_FarePathsSize() const override { return "FP1"; }
  std::string getAttributeName_FlightsSize() const override { return "FL5"; }
  std::string getAttributeName_FlightPathsSize() const override { return "FT1"; }
  std::string getAttributeName_YqYrsSize() const override { return "YY1"; }
  std::string getAttributeName_YqYrPathsSize() const override { return "YP1"; }
  std::string getAttributeName_ItinsSize() const override { return "IT5"; }
  std::string getAttributeName_ApplyUSCAGrouping() const override { return "AUG"; }

  std::string getTagName(OutputResponse*) const override { return "TAX"; }

  std::string getTagName(OutputBaggageDetails*) const override { return "BA1"; }
  std::string getTagName(OutputCalcDetails*) const override { return "CA1"; }
  std::string getTagName(DiagnosticResponse*) const override { return "DI1"; }
  std::string getTagName(OutputDiagnostic*) const override { return "DI2"; }
  std::string getTagName(ErrorMessage*) const override { return "ER1"; }
  std::string getTagName(OutputError*) const override { return "ER2"; }
  std::string getTagName(OutputFaresDetails*) const override { return "FA1"; }
  std::string getTagName(OutputGeoDetails*) const override { return "GE1"; }
  std::string getTagName(ItinPayments*) const override { return "IT1"; }
  std::string getTagName(OutputItin*) const override { return "IT2"; }
  std::string getTagName(ItinsPayments*) const override { return "IT3"; }
  std::string getTagName(OutputItins*) const override { return "IT4"; }
  std::string getTagName(Message*) const override { return "ME1"; }
  std::string getTagName(OutputMessage*) const override { return "ME2"; }
  std::string getTagName(OutputOBDetails*) const override { return "OB1"; }
  std::string getTagName(OutputOCDetails*) const override { return "OC1"; }
  std::string getTagName(OutputOptionalServiceDetails*) const override { return "OP1"; }
  std::string getTagName(OutputTax*) const override { return "TA1"; }
  std::string getTagName(Payment*) const override { return "TA2"; }
  std::string getTagName(PaymentDetail*) const override { return "TA3"; }
  std::string getTagName(OutputTaxDetails*) const override { return "TA4"; }
  std::string getTagName(OutputTaxDetailsRef*) const override { return "TA5"; }
  std::string getTagName(OutputTaxGroup*) const override { return "TA6"; }
  std::string getTagName(OutputTaxOnTaxDetails*) const override { return "TA7"; }
  std::string getTagName(Response*) const override { return "TA9"; }
  std::string getTagName(OutputYQYRDetails*) const override { return "YQ1"; }

  std::string getTagName(BCHOutputResponse*) const override { return "TRS"; }
  std::string getTagName(BCHOutputItin*) const override { return "COM"; }
  std::string getTagName(BCHOutputItinPaxDetail*) const override { return "PXI"; }
  std::string getTagName(BCHOutputTaxDetail*) const override { return "TAX"; }

  std::string getOutputAttributeName_Carrier() const override { return "CA1"; }
  std::string getOutputAttributeName_Code() const override { return "CO1"; }
  std::string getOutputAttributeName_Content() const override { return "CO2"; }
  std::string getOutputAttributeName_EchoToken() const override { return "ET1"; }
  std::string getOutputAttributeName_ElementRefId() const override { return "REF"; }
  std::string getOutputAttributeName_GST() const override { return "GST"; }
  std::string getOutputAttributeName_Id() const override { return "ID1"; }
  std::string getOutputAttributeName_JourneyLoc1() const override { return "JO1"; }
  std::string getOutputAttributeName_JourneyLoc2() const override { return "JO2"; }
  std::string getOutputAttributeName_Name() const override { return "NA1"; }
  std::string getOutputAttributeName_Nation() const override { return "NA2"; }
  std::string getOutputAttributeName_OptionalServiceType() const override { return "OP1"; }
  std::string getOutputAttributeName_PaymentAmt() const override { return "PA1"; }
  std::string getOutputAttributeName_PaymentCur() const override { return "PA2"; }
  std::string getOutputAttributeName_PercentFlatTag() const override { return "PA3"; }
  std::string getOutputAttributeName_PercentageFlatTag() const override { return "PA4"; }
  std::string getOutputAttributeName_PointOfDeliveryLoc() const override { return "PO1"; }
  std::string getOutputAttributeName_PointOfSaleLoc() const override { return "PO2"; }
  std::string getOutputAttributeName_PointOfTicketingLoc() const override { return "PO3"; }
  std::string getOutputAttributeName_PublishedAmt() const override { return "PU1"; }
  std::string getOutputAttributeName_PublishedCur() const override { return "PU2"; }
  std::string getOutputAttributeName_SabreCode() const override { return "SA1"; }
  std::string getOutputAttributeName_SeqNo() const override { return "SE1"; }
  std::string getOutputAttributeName_ServiceSubTypeCode() const override { return "SU1"; }
  std::string getOutputAttributeName_SvcGroup() const override { return "SG1"; }
  std::string getOutputAttributeName_SvcSubGroup() const override { return "SG2"; }
  std::string getOutputAttributeName_TaxAmt() const override { return "TA1"; }
  std::string getOutputAttributeName_TaxCode() const override { return "TA2"; }
  std::string getOutputAttributeName_TaxCurToPaymentCurBSR() const override { return "TA3"; }
  std::string getOutputAttributeName_TaxRoundingUnit() const override { return "TRU"; }
  std::string getOutputAttributeName_TaxRoundingDir() const override { return "TRD"; }
  std::string getOutputAttributeName_TaxEquivAmt() const override { return "TA4"; }
  std::string getOutputAttributeName_TaxEquivCurr() const override { return "TA5"; }
  std::string getOutputAttributeName_TaxGroupId() const override { return "TA6"; }
  std::string getOutputAttributeName_TaxGroupType() const override { return "TA7"; }
  std::string getOutputAttributeName_TaxLabel() const override { return "TA8"; }
  std::string getOutputAttributeName_TaxOnOptionalServiceGroupRefId() const override
  {
    return "TA9";
  }
  std::string getOutputAttributeName_TaxOnBaggageGroupRefId() const override { return "TB0"; }
  std::string getOutputAttributeName_TaxOnChangeFeeGroupRefId() const override { return "TA0"; }
  std::string getOutputAttributeName_TaxPointIndexBegin() const override { return "TAA"; }
  std::string getOutputAttributeName_TaxPointIndexEnd() const override { return "TAB"; }
  std::string getOutputAttributeName_TaxPointLoc1() const override { return "TAC"; }
  std::string getOutputAttributeName_TaxPointLoc1Index() const override { return "TAD"; }
  std::string getOutputAttributeName_TaxPointLoc2() const override { return "TAE"; }
  std::string getOutputAttributeName_TaxPointLoc2Index() const override { return "TAJ"; }
  std::string getOutputAttributeName_TaxPointLoc3() const override { return "TAF"; }
  std::string getOutputAttributeName_TaxPointLocBegin() const override { return "TAG"; }
  std::string getOutputAttributeName_TaxPointLocEnd() const override { return "TAH"; }
  std::string getOutputAttributeName_TaxPointTag() const override { return "TAI"; }
  std::string getOutputAttributeName_TaxType() const override { return "TAJ"; }
  std::string getOutputAttributeName_TotalAmt() const override { return "TO1"; }
  std::string getOutputAttributeName_Type() const override { return "TY1"; }
  std::string getOutputAttributeName_TaxableUnitTags() const override { return "TUT"; }
  std::string getOutputAttributeName_UnticketedPoint() const override { return "UNT"; }

  std::string getOutputAttributeName_BCHItinId() const override { return "Q1D"; }
  std::string getOutputAttributeName_BCHPaxType() const override { return "B70"; }
  std::string getOutputAttributeName_BCHPaxCount() const override { return "Q0W"; }
  std::string getOutputAttributeName_BCHPaxTotalAmount() const override { return "C65"; }
  std::string getOutputAttributeName_BCHPaxTaxes() const override { return "TID"; }
  std::string getOutputAttributeName_BCHTaxId() const override { return "Q1B"; }
  std::string getOutputAttributeName_BCHTaxCode() const override { return "BC0"; }
  std::string getOutputAttributeName_BCHTaxAmount() const override { return "C6B"; }
  std::string getOutputAttributeName_BCHTaxAmountAdjusted() const override { return "C6M"; }
  std::string getOutputAttributeName_BCHTaxDescription() const override { return "S04"; }
};

} // namespace tax
