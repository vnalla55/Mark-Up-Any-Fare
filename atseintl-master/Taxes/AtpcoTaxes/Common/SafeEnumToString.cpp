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

#include "Common/SafeEnumToString.h"

namespace tax
{
std::string
safeEnumToString(const type::TaxPointTag& tpt)
{
  if (tpt == type::TaxPointTag::Sale)
    return "SALE";
  else if (tpt == type::TaxPointTag::Departure)
    return "DEPARTURE";
  else if (tpt == type::TaxPointTag::Arrival)
    return "ARRIVAL";
  else if (tpt == type::TaxPointTag::Delivery)
    return "DELIVERY";
  else
    return "";
}

std::string
safeEnumToString(const type::OpenSegmentIndicator& osi)
{
  if (osi == type::OpenSegmentIndicator::Fixed)
    return "FIXED";
  else if (osi == type::OpenSegmentIndicator::DateFixed)
    return "DATEFIXED";
  else if (osi == type::OpenSegmentIndicator::Open)
    return "OPEN";
  else
    return "UNKNOWN";
}

std::string
safeEnumToString(const type::StopoverTimeUnit& unit)
{
  if (unit == type::StopoverTimeUnit::Blank)
    return "BLANK";
  else if (unit == type::StopoverTimeUnit::Minutes)
    return "MINS";
  else if (unit == type::StopoverTimeUnit::Hours)
    return "HOURS";
  else if (unit == type::StopoverTimeUnit::Days)
    return "DAYS";
  else if (unit == type::StopoverTimeUnit::Months)
    return "MONTHS";
  else if (unit == type::StopoverTimeUnit::HoursSameDay)
    return "SAME DAY HOURS";
  else
    return "UNKNOWN";
}


std::string
safeEnumToString(const type::TaxOrChargeTag& tag)
{
  switch(tag)
  {
  case type::TaxOrChargeTag::Charge:
    return "CHARGE";
  case type::TaxOrChargeTag::Tax:
    return "TAX";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::RefundableTag& tag)
{
  switch(tag)
  {
  case type::RefundableTag::No:
    return "NO";
  case type::RefundableTag::Yes:
    return "YES";
  case type::RefundableTag::NoButReuseable:
    return "NO BUT RE-USEABLE";
  case type::RefundableTag::Partial:
    return "PARTIAL";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::AccountableDocTag& tag)
{
  switch(tag)
  {
  case type::AccountableDocTag::NonTkdOrSale:
    return "NON TKD/SALE";
  case type::AccountableDocTag::NotReported:
    return "NOT REPORTED";
  case type::AccountableDocTag::ReportBundle:
    return "REPORT/BUNDLE";
  case type::AccountableDocTag::ReportSeparate:
    return "REPORT/SEPARATE";
  default:
    return "UNKNOWN";
  }
}


std::string
safeEnumToString(const type::PercentFlatTag& tag)
{
  switch(tag)
  {
  case type::PercentFlatTag::Flat:
    return "FLAT";
  case type::PercentFlatTag::Percent:
    return "PERCENT";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::TaxRemittanceId& tag)
{
  switch (tag)
  {
  case type::TaxRemittanceId::Sale:
    return "SALE";
  case type::TaxRemittanceId::Use:
    return "USE";
  default:
    return "UNKNOWN";
  }
}


std::string
safeEnumToString(const type::TravelDateAppTag& tag)
{
  switch (tag)
  {
  case type::TravelDateAppTag::Journey:
    return "J-JOURNEY";
  case type::TravelDateAppTag::TaxPoint:
    return "T-TAX POINT";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::VatInclusiveInd& tag)
{
  switch (tag)
  {
  case type::VatInclusiveInd::Excludes:
    return "X-EXCLUDES";
  case type::VatInclusiveInd::Includes:
    return "I-INCLUDES";
  default:
    return "UNKNOWN";
  }
}


std::string
safeEnumToString(const type::LocType& tag)
{
  switch (tag)
  {
  case type::LocType::Area:
    return "A-IATA AREA";
  case type::LocType::City:
    return "C-CITY";
  case type::LocType::Airport:
    return "P-AIRPORT";
  case type::LocType::Nation:
    return "N-NATION";
  case type::LocType::StateProvince:
    return "S-STATE/PROVINCE";
  case type::LocType::Miscellaneous:
    return "M-MISCELLANEOUS";
  case type::LocType::ZoneReserved:
    return "R-ZONE/RESERVED";
  case type::LocType::UserZone178:
    return "T-USER ZONE 178";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::TktValApplQualifier& tag)
{
  switch (tag)
  {
  case type::TktValApplQualifier::BaseFare:
    return "A-FARE/FEE EXCLUDE TAXES AND OTHER SERVICE FEES";
  case type::TktValApplQualifier::FareWithFees:
    return "B-FARE/FEE EXCLUDE TAXES BUT INCLUDE OTHER SERVICE FEES";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::PaidBy3rdPartyTag& tag)
{
  switch (tag)
  {
  case type::PaidBy3rdPartyTag::Government:
    return "G-FORM OF PAYMENT GOVERNMENT";
  case type::PaidBy3rdPartyTag::Miles:
    return "M-FORM OF PAYMENT MILES";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::TaxAppliesToTagInd& tag)
{
  switch (tag)
  {
  case type::TaxAppliesToTagInd::AllBaseFare:
    return "A-ENTIRE FARE/FEE DEFINED BY TAXABLE UNIT TAG";
  case type::TaxAppliesToTagInd::LowestFromFareList:
    return "B-TAX POINTS LOC1/LOC2 WHEN BOTH FARE BREAK POINTS-IF NOT "
           "FARE BREAK THEN DIFFERENT CALCULATION METHOD APPLIED";
  case type::TaxAppliesToTagInd::BetweenFareBreaks:
    return "C-TAX POINTS LOC1/LOC2 WHEN BOTH FARE BREAK POINTS-IF NOT "
           "FARE BREAK THEN NO TAX APPLIED";
  case type::TaxAppliesToTagInd::DefinedByTaxProcessingApplTag:
    return "D-ENTIRE FARE/FEE DEFINED BY TAXABLE UNIT TAG AND "
           "SPECIAL PROCESSING";
  case type::TaxAppliesToTagInd::USDeduct:
    return "E-BASE AMOUNT USING MPM PRORATE DEDUCT METHOD WHEN TAX "
           "POINTS LOC1/LOC2 ARE NOT BOTH FARE BREAK POINTS";
  case type::TaxAppliesToTagInd::FullyApplicableRoundTripFare:
    return "F-FULL APPLICABLE ROUND TRIP FARE";
  case type::TaxAppliesToTagInd::BeforeDiscount:
    return "G-FARE BEFORE DISCOUNT";
  case type::TaxAppliesToTagInd::MatchingSectorDetail:
    return "H-TAX PERCENTAGE APPLIES ON COMBINED FARE COMPONENTS EITHER FOR "
           "ENTIRE ITINERARY OR SECTORS MATCHING DATA IN SECTOR DETAIL TABLE 167";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::NetRemitApplTag& tag)
{
  switch (tag)
  {
  case type::NetRemitApplTag::Applies:
    return "S-PERCENTAGE ON SELLING FARE";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::TicketedPointTag& tag)
{
  switch (tag)
  {
  case type::TicketedPointTag::MatchTicketedAndUnticketedPoints:
    return "U-MATCH TICKETED AND UN-TICKETED POINTS";
  default:
    return "DEFAULT-MATCH TICKETED POINTS ONLY";
  }
}

std::string
safeEnumToString(const type::RtnToOrig& tag)
{
  switch (tag)
  {
  case type::RtnToOrig::ContinuousJourney1stPoint:
    return "C-TAX POINT MUST BE FIRST IN CONTINUOUS JOURNEY FOR TAX CODE CA";
  case type::RtnToOrig::ContinuousJourney2ndPoint:
    return "D-TAX POINT MUST BE SECOND IN CONTINUOUS JOURNEY FOR TAX CODE CA";
  case type::RtnToOrig::NotReturnToOrigin:
    return "N-TRAVEL JOURNEY MUST NOT RETURN TO JOURNEY ORIGIN";
  case type::RtnToOrig::ReturnToOrigin:
    return "Y-TRAVEL JOURNEY MUST RETURN TO JOURNEY ORIGIN";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::JrnyInd& tag)
{
  switch (tag)
  {
  case type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ:
    return "A-JOURNEY LOC2 IS DESTINATION POINT FOR ONE-WAY OR ROUND-TRIP OR OPEN-JAW JOURNEY";
  default:
    return "DEFAULT-JOURNEY LOC2 IS DESTINATION POINT FOR ONE-WAY OR TURNAROUND FOR ROUND-TRIP"
           " OR OPEN-JAW JOURNEY";
  }
}

std::string
safeEnumToString(const type::AdjacentIntlDomInd& tag)
{
  switch (tag)
  {
  case type::AdjacentIntlDomInd::AdjacentDomestic:
    return "E-DOMESTIC. TRAVEL BETWEEN LOC1 AND POINT PRIOR IF DEPARTURE"
           "TAX/AFTER IF ARRIVAL TAX MUST BE DOMESTIC";
  case type::AdjacentIntlDomInd::AdjacentInternational:
    return "J-INTERNATIONAL. TRAVEL BETWEEN LOC1 AND POINT PRIOR IF "
           "DEPARTURE TAX/AFTER IF ARRIVAL TAX MUST BE INTERNATIONAL";
  case type::AdjacentIntlDomInd::AdjacentStopoverDomestic:
    return "D-DOMESTIC STOPOVER. TRAVEL BETWEEN LOC1 AND STOPOVER POINT "
           "PRIOR IF DEPARTURE TAX/AFTER IF ARRIVAL TAX MUST BE DOMESTIC";
  case type::AdjacentIntlDomInd::AdjacentStopoverInternational:
    return "I-INTERNATIONAL STOPOVER. TRAVEL BETWEEN LOC1 AND STOPOVER "
           "POINT PRIOR IF DEPARTURE TAX/AFTER IF ARRIVAL TAX MUST BE INTERNATIONAL";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::TransferTypeTag& tag)
{
  switch (tag)
  {
  case type::TransferTypeTag::Interline:
    return "A-INTERLINE, CHANGE OF CARRIER";
  case type::TransferTypeTag::OnlineWithChangeOfFlightNumber:
    return "B-ONLINE, SAME CARRIER WITH CHANGE OF FLIGHT NUMBER";
  case type::TransferTypeTag::OnlineWithChangeOfGauge:
    return "C-ONLINE, SAME CARRIER WITH EQUIPMENT CHANGE, SAME FLIGHT NBR";
  case type::TransferTypeTag::OnlineWithNoChangeOfGauge:
    return "D-ONLINE, SAME CARRIER WITH NO CHANGE OF FLIGHT NBR/EQUIPMENT";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::StopoverTag& tag)
{
  switch (tag)
  {
  case type::StopoverTag::Connection:
    return "C-CONNECTION";
  case type::StopoverTag::FareBreak:
    return "F-FARE BREAK";
  case type::StopoverTag::NotFareBreak:
    return "N-NOT FARE BREAK";
  case type::StopoverTag::Stopover:
    return "S-STOPOVER";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::IntlDomInd& tag)
{
  switch (tag)
  {
  case type::IntlDomInd::Domestic:
    return "D-DOMESTIC. LOC2 MUST BE IN SAME COUNTRY AS LOC1";
  case type::IntlDomInd::International:
    return "I-INTERNATIONAL. LOC2 MUST NOT BE IN SAME COUNTRY AS LOC1";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::TaxPointLoc2Compare& tag)
{
  switch (tag)
  {
  case type::TaxPointLoc2Compare::Point:
    return "X-LOC2 MUST BE A DIFFERENT CITY FROM THE POINT PRIOR TO LOC1 "
           "FOR DEPARTURE TAX/AFTER LOC1 FOR ARRIVAL TAX";
  case type::TaxPointLoc2Compare::Stopover:
    return "Y-LOC2 MUST BE A DIFFERENT CITY FROM THE STOPOVER POINT PRIOR "
           "TO LOC1 FOR DEPARTURE TAX/AFTER LOC1 FOR ARRIVAL TAX";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::Loc2StopoverTag& tag)
{
  switch (tag)
  {
  case type::Loc2StopoverTag::FareBreak:
    return "F-LOC2 MUST BE FIRST FARE/FEE BREAK POINT AFTER LOC1 FOR "
           "DEPARTURE TAX/PRIOR TO LOC1 FOR ARRIVAL TAX";
  case type::Loc2StopoverTag::Furthest:
    return "T-LOC2 MUST BE FURTHEST POINT IN THE FARE/FEE COMPONENT AFTER "
           "LOC1 FOR DEPARTURE TAX/PRIOR TO LOC1 FOR ARRIVAL TAX";
  case type::Loc2StopoverTag::Stopover:
    return "S-LOC2 MUST BE FIRST STOPOVER POINT AFTER LOC1 FOR DEPARTURE "
           "TAX/PRIOR TO LOC1 FOR ARRIVAL TAX";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::TaxPointLoc3GeoType& tag)
{
  switch (tag)
  {
  case type::TaxPointLoc3GeoType::Point:
    return "P-LOC3 MUST BE FIRST POINT PRIOR TO LOC1 FOR DEPARTURE TAX/AFTER";
  case type::TaxPointLoc3GeoType::Stopover:
    return "N-LOC3 MUST BE FIRST STOPOVER POINT AFTER LOC1 FOR DEPARTURE "
           "TAX/PRIOR TO LOC1 FOR ARRIVAL TAX";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::PassengerStatusTag& tag)
{
  switch (tag)
  {
  case type::PassengerStatusTag::Employee:
    return "E-EMPLOYEE";
  case type::PassengerStatusTag::National:
    return "N-NATIONAL";
  case type::PassengerStatusTag::Resident:
    return "R-RESIDENT";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::PtcMatchIndicator& tag)
{
  switch (tag)
  {
  case type::PtcMatchIndicator::Input:
    return "I-INPUT PTC";
  case type::PtcMatchIndicator::Output:
    return "O-FARE PTC";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::CodeType& tag)
{
  switch (tag)
  {
  case type::CodeType::AgencyNumber:
    return "I-IATA TVL AGENCY";
  case type::CodeType::AgencyPCC:
    return "T-PSEUDO/TVL AGT";
  case type::CodeType::AirlineSpec:
    return "A-AIRLINE";
  case type::CodeType::CarrierDept:
    return "V-CXR/GDS DEPT";
  case type::CodeType::Department:
    return "X-DEPARTMENT/ID";
  case type::CodeType::ElecResServProvider:
    return "E-ERSP NO.";
  case type::CodeType::LNIATA:
    return "L-LNIATA-CRT NO.";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::ViewBookTktInd& tag)
{
  switch (tag)
  {
  case type::ViewBookTktInd::ViewBookTkt:
    return "1-VIEW/BOOK/TKT";
  case type::ViewBookTktInd::ViewOnly:
    return "2-VIEW ONLY";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::ServiceBaggageApplTag& tag)
{
  switch (tag)
  {
  case type::ServiceBaggageApplTag::C:
    return "C-MAY PROCESS AGAINST ANOTHER TAX IF DATA IN SERVICE/BAGGAGE "
           "APPLICATION TABLE";
  case type::ServiceBaggageApplTag::E:
    return "E-MAY PROCESS AGAINST DATA IN TAXABLE UNIT TAGS AND SERVICE/"
           "BAGGAGE APPLICATION TABLE";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::ServiceBaggageAppl& tag)
{
  switch (tag)
  {
  case type::ServiceBaggageAppl::Negative:
    return "N-NO";
  case type::ServiceBaggageAppl::Positive:
    return "Y-YES";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::OptionalServiceTag& tag)
{
  switch (tag)
  {
  case type::OptionalServiceTag::BaggageCharge:
    return "C-BAGGAGE CHARGE";
  case type::OptionalServiceTag::FareRelated:
    return "R-FARE RELATED";
  case type::OptionalServiceTag::FlightRelated:
    return "F-FLIGHT RELATED";
  case type::OptionalServiceTag::Merchandise:
    return "M-MERCHANDISE";
  case type::OptionalServiceTag::PrePaid:
    return "P-PREPAID";
  case type::OptionalServiceTag::TicketRelated:
    return "T-TICKET";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::SectorDetailApplTag& tag)
{
  switch (tag)
  {
  case type::SectorDetailApplTag::AllSectorLoc1Loc2:
    return "N-MATCH TO ALL SECTORS BETWEEN LOC1 AND LOC2";
  case type::SectorDetailApplTag::AnySectorLoc1Loc2:
    return "A-MATCH TO AT LEAST ONE SECTOR BETWEEN LOC1-LOC2";
  case type::SectorDetailApplTag::AnySectorOrginDestination:
    return "B-MATCH ANY SECTOR BETWEEN JOUNEY ORIGIN AND DESTINATION";
  case type::SectorDetailApplTag::EverySectorInItin:
    return "J-MATCHED TO EVERY SECTOR IN THE ITINEARY";
  case type::SectorDetailApplTag::FirstInternationalSector:
    return "I-MATCH FIRST INTERNATIONAL SECTOR IN DIFFERENT COUNTRY CODE";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::CabinCode& tag)
{
  switch (tag)
  {
  case type::CabinCode::Business:
    return "C-BUSINESS";
  case type::CabinCode::Economy:
    return "Y-ECONOMY";
  case type::CabinCode::First:
    return "F-FIRST";
  case type::CabinCode::PremiumBusiness:
    return "J-PREMIUM BUSINESS";
  case type::CabinCode::PremiumEconomy:
    return "P-PREMIUM ECONOMY";
  case type::CabinCode::PremiumFirst:
    return "R-PREMIUM FIRST";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::SectorDetailAppl& tag)
{
  switch (tag)
  {
  case type::SectorDetailAppl::Positive:
    return "Y-YES";
  case type::SectorDetailAppl::Negative:
    return "N-NO";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::ConnectionsTag& tag)
{
  switch (tag)
  {
  case type::ConnectionsTag::TurnaroundPointForConnection:
    return "A-TURNAROUNDPOINT CONSIDERED STOP FOR CONNECTION ITINERARIES";
  case type::ConnectionsTag::TurnaroundPoint:
    return "B-TURNAROUND POINT CONSIDERED STOP FOR MOST ITINERARIES";
  case type::ConnectionsTag::FareBreak:
    return "C-ALL FARE BREAKS CONSIDERED STOP";
  case type::ConnectionsTag::FurthestFareBreak:
    return "D-FURTHEST FARE BREAK CONSIDERED STOP";
  case type::ConnectionsTag::GroundTransport:
    return "E-GROUND TRANSPORTATION TO AIR CONSIDERED STOP";
  case type::ConnectionsTag::DifferentMarketingCarrier:
    return "F-OFFLINE CONNECTION CONSIDERED STOP";
  case type::ConnectionsTag::Multiairport:
    return "G-DIFFERENT AIRPORT CODE SAME CITY CODE CONSIDERED STOP";
  case type::ConnectionsTag::DomesticToInternational:
    return "H-DOMESTIC TO INTERNATIONAL GATEWAY CONSIDERED STOP";
  case type::ConnectionsTag::InternationalToDomestic:
    return "I-INTERNATIONAL TO DOMESTIC GATEWAY CONSIDERED STOP";
  case type::ConnectionsTag::InternationalToInternational:
    return "J-INTERNATIONAL TO INTERNATIONAL GATEWAY CONSIDERED STOP";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::ReissueLocTypeTag& tag)
{
  switch (tag)
  {
  case type::ReissueLocTypeTag::IataArea:
    return "IATA Area";
  case type::ReissueLocTypeTag::Market:
    return "MARKET";
  case type::ReissueLocTypeTag::Nation:
    return "NATION";
  case type::ReissueLocTypeTag::StateProvince:
    return "STATE/PROVINCE";
  case type::ReissueLocTypeTag::SubArea:
    return "SUB AREA";
  case type::ReissueLocTypeTag::Zone:
    return "ZONE";
  default:
    return "UNKNOWN";
  }
}

std::string
safeEnumToString(const type::ReissueRefundableTag& tag)
{
  switch (tag)
  {
  case type::ReissueRefundableTag::NonRefundable:
    return "TAX IS NON REFUNDABLE";
  case type::ReissueRefundableTag::Refundable:
    return "TAX IS REFUNDABLE";
  default:
    return "UNKNOWN";
  }
}
} /* namespace tax */
