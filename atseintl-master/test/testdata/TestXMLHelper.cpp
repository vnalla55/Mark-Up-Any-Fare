#include "test/testdata/TestXMLHelper.h"

#include <sstream>
#include <boost/algorithm/string/predicate.hpp>

using namespace tse;

// **************************************************************************************************
// Methods that take a const. There isn't really any work that happens here; we just cast away
// the constness and invoke the method that can modify the incoming attribute.
// **************************************************************************************************

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const std::string& str)
{
  TestXMLHelper::Attribute(element, name, const_cast<std::string&>(str));
}

void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const BoostString& str)
{
  TestXMLHelper::Attribute(element, name, const_cast<BoostString&>(str));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const int8_t& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<int8_t&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const int16_t& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<int16_t&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const int32_t& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<int32_t&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const int64_t& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<int64_t&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const uint8_t& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<uint8_t&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const uint16_t& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<uint16_t&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const uint32_t& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<uint32_t&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const uint64_t& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<uint64_t&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const double& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<double&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const char& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<char&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const bool& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<bool&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const tse::GlobalDirection& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::GlobalDirection&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const tse::LocType& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::LocType&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         const tse::DifferentialData::STATUS_TYPE& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::DifferentialData::STATUS_TYPE&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const tse::GeoTravelType& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::GeoTravelType&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         const tse::FCASegDirectionality& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::FCASegDirectionality&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const tse::Directionality& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::Directionality&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         const tse::PricingUnit::Type& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::PricingUnit::Type&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         const tse::PricingUnit::PUSubType& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::PricingUnit::PUSubType&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         const tse::PricingUnit::PUFareType& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::PricingUnit::PUFareType&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const tse::RecordScope& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::RecordScope&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         const tse::SmallBitSet<uint8_t, tse::FMTravelBoundary>& value)
{
  TestXMLHelper::Attribute(
    element, name, const_cast<tse::SmallBitSet<uint8_t, tse::FMTravelBoundary>&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         const tse::ErrorResponseException::ErrorResponseCode& value)
{
  TestXMLHelper::Attribute(
    element, name, const_cast<tse::ErrorResponseException::ErrorResponseCode&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         const tse::FMDirection& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<tse::FMDirection&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const Code<15>& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<Code<15>&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const Code<8>& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<Code<8>&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const Code<7>& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<Code<7>&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const Code<6>& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<Code<6>&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const Code<5>& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<Code<5>&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const Code<4>& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<Code<4>&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const Code<3>& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<Code<3>&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const Code<2>& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<Code<2>&>(value));
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, const DateTime& value)
{
  TestXMLHelper::Attribute(element, name, const_cast<DateTime&>(value));
}

// **************************************************************************************************
// All the work of setting the attributes is done here. The incoming attribute is modified based on
// the element & name passed in (the TiXmlElement is used to retrieve the value of the given name).
// **************************************************************************************************

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, std::string& str)
{
  const char* value = element->Attribute(name);
  if (value != NULL)
  {
    str = value;
  }
  else
  {
    str = "";
  }
}

void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, BoostString& str)
{
  const char* value = element->Attribute(name);
  if (value != NULL)
  {
    str = value;
  }
  else
  {
    str = "";
  }
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, int8_t& value)
{
  int value2;
  Attribute(element, name, value2);
  value = static_cast<int8_t>(value2);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, int16_t& value)
{
  int value2;
  Attribute(element, name, value2);
  value = static_cast<int16_t>(value2);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, int32_t& value)
{
  value = 0;
  element->Attribute(name, &value);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, int64_t& value)
{
  value = strtol(element->Attribute(name), 0, 10);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, uint8_t& value)
{
  int value2;
  Attribute(element, name, value2);
  value = static_cast<uint8_t>(value2);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, uint16_t& value)
{
  int value2;
  Attribute(element, name, value2);
  value = static_cast<uint16_t>(value2);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, uint32_t& value)
{
  int value2;
  Attribute(element, name, value2);
  value = value2;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, uint64_t& value)
{
  value = strtoul(element->Attribute(name), 0, 10);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, double& value)
{
  value = 0.0;
  element->Attribute(name, &value);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, char& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s); // Get string value
  if (s == "")
    value = 0;

  // If the entry is a string, we can convert to certain values
  // Unfortunately, this isn't a 2-way street. We can convert the full string to a char,
  // but not the other way around.

  if (s == "ALL_WAYS")
    value = ' ';
  else if (s == "ONE_WAY_MAY_BE_DOUBLED")
    value = '1';
  else if (s == "ROUND_TRIP_MAYNOT_BE_HALVED")
    value = '2';
  else if (s == "ONE_WAY_MAYNOT_BE_DOUBLED")
    value = '3';
  else if (boost::algorithm::starts_with(s, "\\x"))
  {
    std::istringstream tmp(std::string(s.begin() + 2, s.end()));
    int v = 0;
    tmp >> std::hex >> v;
    value = static_cast<char>(v);
  }
  else if (s != "")
    value = s[0]; // default behavior
}

//-------------------------------------------------------------------------------------------

// For convenience, we take boolean values as a textual "t" or "true" (case insensitive)
// and "f", or "false", and convert to the appropriate boolean value.

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, bool& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);
  if (s != "" && (s == "t" || s == "T" || s == "true" || s == "TRUE"))
  {
    value = true;
  }
  else
  {
    value = false;
  }
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, tse::GlobalDirection& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);
  if (s == "AF")
    value = GlobalDirection::AF;
  else if (s == "AL")
    value = GlobalDirection::AL;
  else if (s == "AP")
    value = GlobalDirection::AP;
  else if (s == "AT")
    value = GlobalDirection::AT;
  else if (s == "CA")
    value = GlobalDirection::CA;
  else if (s == "CT")
    value = GlobalDirection::CT;
  else if (s == "DI")
    value = GlobalDirection::DI;
  else if (s == "DO")
    value = GlobalDirection::DO;
  else if (s == "DU")
    value = GlobalDirection::DU;
  else if (s == "EH")
    value = GlobalDirection::EH;
  else if (s == "EM")
    value = GlobalDirection::EM;
  else if (s == "EU")
    value = GlobalDirection::EU;
  else if (s == "FE")
    value = GlobalDirection::FE;
  else if (s == "IN")
    value = GlobalDirection::IN;
  else if (s == "ME")
    value = GlobalDirection::ME;
  else if (s == "NA")
    value = GlobalDirection::NA;
  else if (s == "NP")
    value = GlobalDirection::NP;
  else if (s == "PA")
    value = GlobalDirection::PA;
  else if (s == "PE")
    value = GlobalDirection::PE;
  else if (s == "PN")
    value = GlobalDirection::PN;
  else if (s == "PO")
    value = GlobalDirection::PO;
  else if (s == "PV")
    value = GlobalDirection::PV;
  else if (s == "RU")
    value = GlobalDirection::RU;
  else if (s == "RW")
    value = GlobalDirection::RW;
  else if (s == "SA")
    value = GlobalDirection::SA;
  else if (s == "SN")
    value = GlobalDirection::SN;
  else if (s == "SP")
    value = GlobalDirection::SP;
  else if (s == "TB")
    value = GlobalDirection::TB;
  else if (s == "TS")
    value = GlobalDirection::TS;
  else if (s == "TT")
    value = GlobalDirection::TT;
  else if (s == "US")
    value = GlobalDirection::US;
  else if (s == "WH")
    value = GlobalDirection::WH;
  else if (s == "XX")
    value = GlobalDirection::XX;
  else if (s == "ZZ")
    value = GlobalDirection::ZZ;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, tse::LocType& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);
  if (s == "UNKNOWN_LOC")
    value = UNKNOWN_LOC;
  else if (s == "IATA_AREA")
    value = IATA_AREA;
  else if (s == "SUBAREA")
    value = SUBAREA;
  else if (s == "MARKET")
    value = MARKET;
  else if (s == "NATION")
    value = NATION;
  else if (s == "STATE_PROVINCE")
    value = STATE_PROVINCE;
  else if (s == "ZONE")
    value = ZONE;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         tse::DifferentialData::STATUS_TYPE& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s == "NOT_PROCESSED_YET")
    value = tse::DifferentialData::SC_NOT_PROCESSED_YET;
  else if (s == "PASSED")
    value = tse::DifferentialData::SC_PASSED;
  else if (s == "FAILED")
    value = tse::DifferentialData::SC_FAILED;
  else if (s == "MATCH_1A")
    value = tse::DifferentialData::SC_MATCH_1A;
  else if (s == "MATCH_1B")
    value = tse::DifferentialData::SC_MATCH_1B;
  else if (s == "CONSOLIDATED_PASS")
    value = tse::DifferentialData::SC_CONSOLIDATED_PASS;
  else if (s == "CONSOLIDATED_FAIL")
    value = tse::DifferentialData::SC_CONSOLIDATED_FAIL;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, tse::GeoTravelType& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s == "UnknownGeoTravelType")
    value = GeoTravelType::UnknownGeoTravelType;
  else if (s == "Domestic")
    value = GeoTravelType::Domestic;
  else if (s == "International")
    value = GeoTravelType::International;
  else if (s == "Transborder")
    value = GeoTravelType::Transborder;
  else if (s == "ForeignDomestic")
    value = GeoTravelType::ForeignDomestic;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, tse::FCASegDirectionality& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s == "DIR_IND_NOT_DEFINED")
    value = tse::DIR_IND_NOT_DEFINED;
  else if (s == "ORIGINATING_LOC1")
    value = tse::ORIGINATING_LOC1;
  else if (s == "ORIGINATING_LOC2")
    value = tse::ORIGINATING_LOC2;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, tse::Directionality& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s == "FROM")
    value = tse::FROM;
  else if (s == "TO")
    value = tse::TO;
  else if (s == "BETWEEN")
    value = tse::BETWEEN;
  else if (s == "WITHIN")
    value = tse::WITHIN;
  else if (s == "BOTH")
    value = tse::BOTH;
  else if (s == "ORIGIN")
    value = tse::ORIGIN;
  else if (s == "TERMINATE")
    value = tse::TERMINATE;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, tse::PricingUnit::Type& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s == "UNKNOWN")
    value = tse::PricingUnit::Type::UNKNOWN;
  else if (s == "OPENJAW")
    value = tse::PricingUnit::Type::OPENJAW;
  else if (s == "ROUNDTRIP")
    value = tse::PricingUnit::Type::ROUNDTRIP;
  else if (s == "CIRCLETRIP")
    value = tse::PricingUnit::Type::CIRCLETRIP;
  else if (s == "ONEWAY")
    value = tse::PricingUnit::Type::ONEWAY;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         tse::PricingUnit::PUSubType& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s == "UNKNOWN_SUBTYPE")
    value = tse::PricingUnit::UNKNOWN_SUBTYPE;
  else if (s == "DEST_OPENJAW")
    value = tse::PricingUnit::DEST_OPENJAW;
  else if (s == "ORIG_OPENJAW")
    value = tse::PricingUnit::ORIG_OPENJAW;
  else if (s == "DOUBLE_OPENJAW")
    value = tse::PricingUnit::DOUBLE_OPENJAW;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         tse::PricingUnit::PUFareType& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s == "NL")
    value = tse::PricingUnit::NL;
  else if (s == "SP")
    value = tse::PricingUnit::SP;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, tse::RecordScope& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s == "DOMESTIC")
    value = tse::DOMESTIC;
  else if (s == "INTERNATIONAL")
    value = tse::INTERNATIONAL;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         tse::SmallBitSet<uint8_t, tse::FMTravelBoundary>& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s.find("TravelWithinUSCA") != std::string::npos)
    value.set(FMTravelBoundary::TravelWithinUSCA, true);
  if (s.find("TravelWithinSameCountryExceptUSCA") != std::string::npos)
    value.set(FMTravelBoundary::TravelWithinSameCountryExceptUSCA, true);
  if (s.find("TravelWithinOneIATA") != std::string::npos)
    value.set(FMTravelBoundary::TravelWithinOneIATA, true);
  if (s.find("TravelWithinTwoIATA") != std::string::npos)
    value.set(FMTravelBoundary::TravelWithinTwoIATA, true);
  if (s.find("TravelWithinAllIATA") != std::string::npos)
    value.set(FMTravelBoundary::TravelWithinAllIATA, true);
  if (s.find("TravelWithinSubIATA11") != std::string::npos)
    value.set(FMTravelBoundary::TravelWithinSubIATA11, true);
  if (s.find("TravelWithinSubIATA21") != std::string::npos)
    value.set(FMTravelBoundary::TravelWithinSubIATA21, true);
  if (s.find("TravelWithinSameSubIATAExcept21And11") != std::string::npos)
    value.set(FMTravelBoundary::TravelWithinSameSubIATAExcept21And11, true);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element,
                         const char* name,
                         tse::ErrorResponseException::ErrorResponseCode& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s == "NO_ERROR")
    value = ErrorResponseException::NO_ERROR;
  else if (s == "NO_FARE_REQUESTED")
    value = ErrorResponseException::NO_FARE_REQUESTED;
  else if (s == "PRICING_REST_BY_GOV")
    value = ErrorResponseException::PRICING_REST_BY_GOV;
  else if (s == "NO_FARE_FOR_CLASS_USED")
    value = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
  else if (s == "CITY_PAIR_RESTRICTED_BY_FBM")
    value = ErrorResponseException::CITY_PAIR_RESTRICTED_BY_FBM;
  else if (s == "CANNOT_COMPUTE_TAX")
    value = ErrorResponseException::CANNOT_COMPUTE_TAX;
  else if (s == "AIRPORT_CODE_NOT_IN_SYS")
    value = ErrorResponseException::AIRPORT_CODE_NOT_IN_SYS;
  else if (s == "FARE_NOT_IN_SYS")
    value = ErrorResponseException::FARE_NOT_IN_SYS;
  else if (s == "CHECK_DATE_TIME_CONTINUITY_OF_FLIGHTS")
    value = ErrorResponseException::CHECK_DATE_TIME_CONTINUITY_OF_FLIGHTS;
  else if (s == "JT_RESTRICTED_RTG_INVALID")
    value = ErrorResponseException::JT_RESTRICTED_RTG_INVALID;
  else if (s == "TRIP_EXCEEDS_MPM_SOM")
    value = ErrorResponseException::TRIP_EXCEEDS_MPM_SOM;
  else if (s == "MAX_NUMBER_COMBOS_EXCEEDED")
    value = ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED;
  else if (s == "FREE_SURCHARGED_STOPOVER_MAY_APPLY")
    value = ErrorResponseException::FREE_SURCHARGED_STOPOVER_MAY_APPLY;
  else if (s == "TOO_MANY_SURFACE_SEGS")
    value = ErrorResponseException::TOO_MANY_SURFACE_SEGS;
  else if (s == "NO_DATE_IN_OPEN_SEG")
    value = ErrorResponseException::NO_DATE_IN_OPEN_SEG;
  else if (s == "CHECK_LINE_OF_FLIGHT")
    value = ErrorResponseException::CHECK_LINE_OF_FLIGHT;
  else if (s == "STOPOVER_SURCHARGE_MAY_APPLY")
    value = ErrorResponseException::STOPOVER_SURCHARGE_MAY_APPLY;
  else if (s == "NO_RULES_FOR_PSGR_TYPE_OR_CLASS")
    value = ErrorResponseException::NO_RULES_FOR_PSGR_TYPE_OR_CLASS;
  else if (s == "NO_THRU_CLASS_FOR_ENROUTE_CHANGE_FLT")
    value = ErrorResponseException::NO_THRU_CLASS_FOR_ENROUTE_CHANGE_FLT;
  else if (s == "FIRST_SEG_OPEN")
    value = ErrorResponseException::FIRST_SEG_OPEN;
  else if (s == "RETRY_IN_ONE_MINUTE")
    value = ErrorResponseException::RETRY_IN_ONE_MINUTE;
  else if (s == "NO_TIMES_IN_OA_SEG")
    value = ErrorResponseException::NO_TIMES_IN_OA_SEG;
  else if (s == "SYSTEM_ERROR")
    value = ErrorResponseException::SYSTEM_ERROR;
  else if (s == "FARE_RESTRICTED_FROM_PRICING")
    value = ErrorResponseException::FARE_RESTRICTED_FROM_PRICING;
  else if (s == "INVALID_ROUTING_OR_SEG_BETWEEN_CO_TERM")
    value = ErrorResponseException::INVALID_ROUTING_OR_SEG_BETWEEN_CO_TERM;
  else if (s == "PRICING_AT_PEAK_USAGE")
    value = ErrorResponseException::PRICING_AT_PEAK_USAGE;
  else if (s == "MIXED_CLASS_TRIP")
    value = ErrorResponseException::MIXED_CLASS_TRIP;
  else if (s == "MAX_PERMITTED_MILEAGE_NOT_AVAIL")
    value = ErrorResponseException::MAX_PERMITTED_MILEAGE_NOT_AVAIL;
  else if (s == "MAX_SEGS_EXCEEDED")
    value = ErrorResponseException::MAX_SEGS_EXCEEDED;
  else if (s == "TOTAL_FARE_TOO_LARGE")
    value = ErrorResponseException::TOTAL_FARE_TOO_LARGE;
  else if (s == "SYSTEM_DATA_ERROR")
    value = ErrorResponseException::SYSTEM_DATA_ERROR;
  else if (s == "FAILED_DUE_TO_COMBO_RESTRICTIONS")
    value = ErrorResponseException::FAILED_DUE_TO_COMBO_RESTRICTIONS;
  else if (s == "NO_COMBINABLE_FARES_FOR_CLASS")
    value = ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS;
  else if (s == "FARE_BASIS_EXCEED_7CHAR_FOR_TAT_TM")
    value = ErrorResponseException::FARE_BASIS_EXCEED_7CHAR_FOR_TAT_TM;
  else if (s == "MAX_COMBOS_EXCEEDED_FOR_AUTO_PRICING")
    value = ErrorResponseException::MAX_COMBOS_EXCEEDED_FOR_AUTO_PRICING;
  else if (s == "CANNOT_FORMAT_TAX")
    value = ErrorResponseException::ErrorResponseException::CANNOT_FORMAT_TAX;
  else if (s == "NBR_PSGRS_EXCEEDS_OA_AVAIL")
    value = ErrorResponseException::ErrorResponseException::NBR_PSGRS_EXCEEDS_OA_AVAIL;
  else if (s == "NO_FARE_VALID_FOR_PSGR_TYPE")
    value = ErrorResponseException::ErrorResponseException::NO_FARE_VALID_FOR_PSGR_TYPE;
  else if (s == "EXCEEDS_ALLOWED_SEGS_CHANGED_CLASS_FLIGHTS")
    value =
        ErrorResponseException::ErrorResponseException::EXCEEDS_ALLOWED_SEGS_CHANGED_CLASS_FLIGHTS;
  else if (s == "CHECK_FLIGHT")
    value = ErrorResponseException::ErrorResponseException::CHECK_FLIGHT;
  else if (s == "FARE_CALC_TOO_LARGE_FOR_ATB")
    value = ErrorResponseException::ErrorResponseException::FARE_CALC_TOO_LARGE_FOR_ATB;
  else if (s == "CANNOT_CALC_SD_FARE")
    value = ErrorResponseException::ErrorResponseException::CANNOT_CALC_SD_FARE;
  else if (s == "TICKET_DESIGNATOR_NOT_ALLOWED")
    value = ErrorResponseException::ErrorResponseException::TICKET_DESIGNATOR_NOT_ALLOWED;
  else if (s == "WPNJ_NOT_VALID_ON_THIS_ITIN")
    value = ErrorResponseException::ErrorResponseException::WPNJ_NOT_VALID_ON_THIS_ITIN;
  else if (s == "CANNOT_CALCULATE_CURRENCY")
    value = ErrorResponseException::CANNOT_CALCULATE_CURRENCY;
  else if (s == "CANNOT_ROUND_CURRENCY")
    value = ErrorResponseException::CANNOT_ROUND_CURRENCY;
  else if (s == "BREAK_FARE_INVALID")
    value = ErrorResponseException::BREAK_FARE_INVALID;
  else if (s == "RETRY")
    value = ErrorResponseException::RETRY;
  else if (s == "NEED_COMMISSION")
    value = ErrorResponseException::NEED_COMMISSION;
  else if (s == "OPEN_JAW_MAY_APPLY")
    value = ErrorResponseException::OPEN_JAW_MAY_APPLY;
  else if (s == "FLT_CHK_DATE_TIME_CONTINUITY_OF_FLTS")
    value = ErrorResponseException::FLT_CHK_DATE_TIME_CONTINUITY_OF_FLTS;
  else if (s == "FARE_BASIS_NOT_AVAIL")
    value = ErrorResponseException::FARE_BASIS_NOT_AVAIL;
  else if (s == "WORLDFARE_AT_PEAK_USE")
    value = ErrorResponseException::WORLDFARE_AT_PEAK_USE;
  else if (s == "WORLDFARE_UNAVAILABLE")
    value = ErrorResponseException::WORLDFARE_UNAVAILABLE;
  else if (s == "TKT_DES_FAILE_RULES_CHECK")
    value = ErrorResponseException::TKT_DES_FAILE_RULES_CHECK;
  else if (s == "TKT_DES_RECORD_RETRIEVAL_ERROR")
    value = ErrorResponseException::TKT_DES_RECORD_RETRIEVAL_ERROR;
  else if (s == "PROCESSING_ERROR_DETECTED")
    value = ErrorResponseException::PROCESSING_ERROR_DETECTED;
  else if (s == "PENALTY_DATA_INCOMPLETE")
    value = ErrorResponseException::PENALTY_DATA_INCOMPLETE;
  else if (s == "LOW_FARE_NOT_ALLOWED_FOR_CLASS_USED")
    value = ErrorResponseException::LOW_FARE_NOT_ALLOWED_FOR_CLASS_USED;
  else if (s == "WRONG_NUMBER_OF_FARES")
    value = ErrorResponseException::WRONG_NUMBER_OF_FARES;
  else if (s == "DISCOUNT_AMT_EXCEEDS_FARE_COMPONENT_TOTAL")
    value = ErrorResponseException::DISCOUNT_AMT_EXCEEDS_FARE_COMPONENT_TOTAL;
  else if (s == "ENTRIES_NOT_COMBINABLE_WITH_DISCOUNT")
    value = ErrorResponseException::ENTRIES_NOT_COMBINABLE_WITH_DISCOUNT;
  else if (s == "OPEN_RETURN_REQUIRED")
    value = ErrorResponseException::OPEN_RETURN_REQUIRED;
  else if (s == "NO_FARE_BASIS_FOR_AIRPORT_PAIR_AGENCY")
    value = ErrorResponseException::NO_FARE_BASIS_FOR_AIRPORT_PAIR_AGENCY;
  else if (s == "CANNOT_COMPUTE_TAX_COMPUTE_MANUALLY")
    value = ErrorResponseException::CANNOT_COMPUTE_TAX_COMPUTE_MANUALLY;
  else if (s == "CANNOT_TAX_INSERT_TAX_AFTER_TX")
    value = ErrorResponseException::CANNOT_TAX_INSERT_TAX_AFTER_TX;
  else if (s == "MULTI_DISCOUNTS_EXIST")
    value = ErrorResponseException::MULTI_DISCOUNTS_EXIST;
  else if (s == "NO_VALID_DISCOUNT_PUB_FARE")
    value = ErrorResponseException::NO_VALID_DISCOUNT_PUB_FARE;
  else if (s == "SCHEDULES_NOT_AVAILABLE_FOR_CARRIER_DATE")
    value = ErrorResponseException::SCHEDULES_NOT_AVAILABLE_FOR_CARRIER_DATE;
  else if (s == "DATA_ERROR_DETECTED")
    value = ErrorResponseException::DATA_ERROR_DETECTED;
  else if (s == "NO_PNR_CREATED")
    value = ErrorResponseException::NO_PNR_CREATED;
  else if (s == "NO_FARE_FOR_CLASS")
    value = ErrorResponseException::NO_FARE_FOR_CLASS;
  else if (s == "CANNOT_PRICE_AS_REQUESTED")
    value = ErrorResponseException::ErrorResponseCode::CANNOT_PRICE_AS_REQUESTED;
  else if (s == "NO_FARES_RBD_CARRIER")
    value = ErrorResponseException::NO_FARES_RBD_CARRIER;
  else if (s == "NEGOTIATED_FARES_APPLY")
    value = ErrorResponseException::NEGOTIATED_FARES_APPLY;
  else if (s == "NO_FARES_FOUND_FOR_FARE_COMPONENT")
    value = ErrorResponseException::NO_FARES_FOUND_FOR_FARE_COMPONENT;
  else if (s == "CODESHARE_PROCESSING_ERROR")
    value = ErrorResponseException::CODESHARE_PROCESSING_ERROR;
  else if (s == "NO_MATCH_FOR_FARE_COMPONENT")
    value = ErrorResponseException::NO_MATCH_FOR_FARE_COMPONENT;
  else if (s == "INVALID_FARE_BASIS_FOR_CARRIER")
    value = ErrorResponseException::INVALID_FARE_BASIS_FOR_CARRIER;
  else if (s == "REQ_CARRIER_HAS_NO_FARES")
    value = ErrorResponseException::REQ_CARRIER_HAS_NO_FARES;
  else if (s == "NEGOTIATED_FARES_APPLY")
    value = ErrorResponseException::NEGOTIATED_FARES_APPLY;
  else if (s == "NO_FARES_FOUND_FOR_FARE_COMPONENT")
    value = ErrorResponseException::NO_FARES_FOUND_FOR_FARE_COMPONENT;
  else if (s == "CODESHARE_PROCESSING_ERROR")
    value = ErrorResponseException::CODESHARE_PROCESSING_ERROR;
  else if (s == "NO_MATCH_FOR_FARE_COMPONENT")
    value = ErrorResponseException::NO_MATCH_FOR_FARE_COMPONENT;
  else if (s == "INVALID_FARE_BASIS_FOR_CARRIER")
    value = ErrorResponseException::INVALID_FARE_BASIS_FOR_CARRIER;
  else if (s == "REQ_CARRIER_HAS_NO_FARES")
    value = ErrorResponseException::REQ_CARRIER_HAS_NO_FARES;
  else if (s == "INVALID_JOURNEY_RECORD")
    value = ErrorResponseException::INVALID_JOURNEY_RECORD;
  else if (s == "INVALID_INPUT")
    value = ErrorResponseException::INVALID_INPUT;
  else if (s == "INVALID_DUTY_CODE")
    value = ErrorResponseException::INVALID_DUTY_CODE;
  else if (s == "NO_ITIN_SEGS_FOUND")
    value = ErrorResponseException::NO_ITIN_SEGS_FOUND;
  else if (s == "PNR_DATABASE_ERROR")
    value = ErrorResponseException::PNR_DATABASE_ERROR;
  else if (s == "NEED_PREFERRED_CARRIER")
    value = ErrorResponseException::NEED_PREFERRED_CARRIER;
  else if (s == "MAX_PREF_CARRIERS_EXCEEDED")
    value = ErrorResponseException::MAX_PREF_CARRIERS_EXCEEDED;
  else if (s == "MAX_PASSENGERS_EXCEEDED")
    value = ErrorResponseException::MAX_PASSENGERS_EXCEEDED;
  else if (s == "INVALID_SAME_PREF_AND_NON_PREF_CARRIER")
    value = ErrorResponseException::INVALID_SAME_PREF_AND_NON_PREF_CARRIER;
  else if (s == "ENTER_ACTIVE_SEGS")
    value = ErrorResponseException::ENTER_ACTIVE_SEGS;
  else if (s == "CONFLICTING_OPTIONS")
    value = ErrorResponseException::CONFLICTING_OPTIONS;
  else if (s == "INVALID_SEG_TYPE_IN_PNR_RETIEVAL")
    value = ErrorResponseException::INVALID_SEG_TYPE_IN_PNR_RETIEVAL;
  else if (s == "PNR_SEGS_NOT_FOUND")
    value = ErrorResponseException::PNR_SEGS_NOT_FOUND;
  else if (s == "CHECK_SEG_CONTINUITY")
    value = ErrorResponseException::CHECK_SEG_CONTINUITY;
  else if (s == "INVALID_TERMINAL_TYPE")
    value = ErrorResponseException::INVALID_TERMINAL_TYPE;
  else if (s == "MULTI_CONNECT")
    value = ErrorResponseException::MULTI_CONNECT;
  else if (s == "DUPLICATE_X_AND_O_QUALIFIER")
    value = ErrorResponseException::DUPLICATE_X_AND_O_QUALIFIER;
  else if (s == "CANNOT_USE_SEG_NUMBER")
    value = ErrorResponseException::CANNOT_USE_SEG_NUMBER;
  else if (s == "BOOK_SEPARATE_PNR")
    value = ErrorResponseException::BOOK_SEPARATE_PNR;
  else if (s == "MAX_FOUR_SEATS")
    value = ErrorResponseException::MAX_FOUR_SEATS;
  else if (s == "TRY_DIAGNOSTIC_ENTRY_10_20")
    value = ErrorResponseException::TRY_DIAGNOSTIC_ENTRY_10_20;
  else if (s == "TRY_DIAGNOSTIC_ENTRY_1_6")
    value = ErrorResponseException::TRY_DIAGNOSTIC_ENTRY_1_6;
  else if (s == "SPECIFY_PUBLIC_OR_PRIVATE")
    value = ErrorResponseException::SPECIFY_PUBLIC_OR_PRIVATE;
  else if (s == "NO_DIAGNOSTIC_TO_DISPLAY")
    value = ErrorResponseException::NO_DIAGNOSTIC_TO_DISPLAY;
  else if (s == "NO_CONVERTED_FARE_BASIS_CODE")
    value = ErrorResponseException::NO_CONVERTED_FARE_BASIS_CODE;
  else if (s == "NO_PQ_ITEMS_FOR_GIVEN_ORDINAL")
    value = ErrorResponseException::NO_PQ_ITEMS_FOR_GIVEN_ORDINAL;
  else if (s == "FARENET_PROCESSING_NOT_YET_AVAIL")
    value = ErrorResponseException::FARENET_PROCESSING_NOT_YET_AVAIL;
  else if (s == "CALL_DIRECT")
    value = ErrorResponseException::CALL_DIRECT;
  else if (s == "NO_FLIGHTS_FOUND")
    value = ErrorResponseException::NO_FLIGHTS_FOUND;
  else if (s == "INVALID_CLASS")
    value = ErrorResponseException::INVALID_CLASS;
  else if (s == "NO_COMBOS")
    value = ErrorResponseException::NO_COMBOS;
  else if (s == "INVALID_FORMAT")
    value = ErrorResponseException::INVALID_FORMAT;
  else if (s == "NO_TRIP")
    value = ErrorResponseException::NO_TRIP;
  else if (s == "CANNOT_SELL_SEG")
    value = ErrorResponseException::CANNOT_SELL_SEG;
  else if (s == "MUST_BE_PRICED_FIRST")
    value = ErrorResponseException::MUST_BE_PRICED_FIRST;
  else if (s == "D1_NO_FLIGHT_ITEM_BLOCKS")
    value = ErrorResponseException::D1_NO_FLIGHT_ITEM_BLOCKS;
  else if (s == "D1_FIND_ERR")
    value = ErrorResponseException::D1_FIND_ERR;
  else if (s == "D2_NO_FLIGHT_ITEM_SORT_LIST_BLOCKS")
    value = ErrorResponseException::D2_NO_FLIGHT_ITEM_SORT_LIST_BLOCKS;
  else if (s == "D3_NO_FLIGHT_COMB_BLOCKS")
    value = ErrorResponseException::D3_NO_FLIGHT_COMB_BLOCKS;
  else if (s == "D3_INVALID_JRFCT_PARMS")
    value = ErrorResponseException::D3_INVALID_JRFCT_PARMS;
  else if (s == "QJRA_TRAN_VECTOR_ERR")
    value = ErrorResponseException::QJRA_TRAN_VECTOR_ERR;
  else if (s == "QJRB_TRAN_VECTOR_ERR")
    value = ErrorResponseException::QJRB_TRAN_VECTOR_ERR;
  else if (s == "NO_DIRECTS_NONSTOPS")
    value = ErrorResponseException::NO_DIRECTS_NONSTOPS;
  else if (s == "TOO_MANY_COMBOS")
    value = ErrorResponseException::TOO_MANY_COMBOS;
  else if (s == "CORPORATE_PRICING_ACTIVE")
    value = ErrorResponseException::CORPORATE_PRICING_ACTIVE;
  else if (s == "ALT_CITIES_INVALID_FOR_ARNK")
    value = ErrorResponseException::ALT_CITIES_INVALID_FOR_ARNK;
  else if (s == "ALT_DATES_INVALID_FOR_ARNK")
    value = ErrorResponseException::ALT_DATES_INVALID_FOR_ARNK;
  else if (s == "TRIP_DURATION_INVALID_FOR_ARNK")
    value = ErrorResponseException::TRIP_DURATION_INVALID_FOR_ARNK;
  else if (s == "MAX_CONNECTION_TIME_INVALID_FOR_ARNK")
    value = ErrorResponseException::MAX_CONNECTION_TIME_INVALID_FOR_ARNK;
  else if (s == "MAX_TRAVEL_TIME_INVALID_FOR_ARNK")
    value = ErrorResponseException::MAX_TRAVEL_TIME_INVALID_FOR_ARNK;
  else if (s == "ALT_CITIES_NOT_ALLOWED")
    value = ErrorResponseException::ALT_CITIES_NOT_ALLOWED;
  else if (s == "INVALID_SEG_NUMBER")
    value = ErrorResponseException::INVALID_SEG_NUMBER;
  else if (s == "INVALID_CITY_AIRPORT_CODE")
    value = ErrorResponseException::INVALID_CITY_AIRPORT_CODE;
  else if (s == "NEED_SEPARATOR")
    value = ErrorResponseException::NEED_SEPARATOR;
  else if (s == "SEG_DOES_NOT_MATCH_CITY_AIRPORT")
    value = ErrorResponseException::SEG_DOES_NOT_MATCH_CITY_AIRPORT;
  else if (s == "NO_CITY_AIRPORT_FOR_ALTS")
    value = ErrorResponseException::NO_CITY_AIRPORT_FOR_ALTS;
  else if (s == "ALT_CITIES_AIRPORTS_INVALID_FOR_CONNECT")
    value = ErrorResponseException::ALT_CITIES_AIRPORTS_INVALID_FOR_CONNECT;
  else if (s == "ALT_CITY_AIRPORT_AND_MILEAGE_RANGE")
    value = ErrorResponseException::ALT_CITY_AIRPORT_AND_MILEAGE_RANGE;
  else if (s == "MILEAGE_RANGE_INVALID")
    value = ErrorResponseException::MILEAGE_RANGE_INVALID;
  else if (s == "INVALID_ALT_CITY_AIRPORT_CODE")
    value = ErrorResponseException::INVALID_ALT_CITY_AIRPORT_CODE;
  else if (s == "INTERNATIONAL_ALT_CITY_AS_PSEUDO")
    value = ErrorResponseException::INTERNATIONAL_ALT_CITY_AS_PSEUDO;
  else if (s == "MAX_ALT_AIRPORT_CITIES_EXCEEDED")
    value = ErrorResponseException::MAX_ALT_AIRPORT_CITIES_EXCEEDED;
  else if (s == "DUPLICATE_BOARD_OFF_POINT_IN_SEG")
    value = ErrorResponseException::DUPLICATE_BOARD_OFF_POINT_IN_SEG;
  else if (s == "SPECIFY_SEGS_FOR_NONCONSECUTIVE_ALT_DATES")
    value = ErrorResponseException::SPECIFY_SEGS_FOR_NONCONSECUTIVE_ALT_DATES;
  else if (s == "INCLUDE_CONNECTING_SEGS")
    value = ErrorResponseException::INCLUDE_CONNECTING_SEGS;
  else if (s == "INVALID_NUMBER_FOR_ALT_DATES")
    value = ErrorResponseException::INVALID_NUMBER_FOR_ALT_DATES;
  else if (s == "COMBINED_CONSEC_NONCONSEC_ALT_DATES")
    value = ErrorResponseException::COMBINED_CONSEC_NONCONSEC_ALT_DATES;
  else if (s == "INVALID_DATE_ENTRY_FOR_ALT_TRAVEL_DATES")
    value = ErrorResponseException::INVALID_DATE_ENTRY_FOR_ALT_TRAVEL_DATES;
  else if (s == "MAX_NUMBER_ALT_TRAVEL_DATES_EXCEEDED")
    value = ErrorResponseException::MAX_NUMBER_ALT_TRAVEL_DATES_EXCEEDED;
  else if (s == "DURATION_ONLY_VALID_FOR_ALT_DATES")
    value = ErrorResponseException::DURATION_ONLY_VALID_FOR_ALT_DATES;
  else if (s == "DURATION_ONLY_VALID_FOR_CONSEC_ALT_DATES")
    value = ErrorResponseException::DURATION_ONLY_VALID_FOR_CONSEC_ALT_DATES;
  else if (s == "DURATION_IS_INVALID")
    value = ErrorResponseException::DURATION_IS_INVALID;
  else if (s == "DURATION_ONLY_VALID_FOR_CONSEC_STOPOVERS")
    value = ErrorResponseException::DURATION_ONLY_VALID_FOR_CONSEC_STOPOVERS;
  else if (s == "MAX_CONNECTION_TIME_EXCEEDED")
    value = ErrorResponseException::MAX_CONNECTION_TIME_EXCEEDED;
  else if (s == "MAX_TRAVEL_TIME_EXCEEDED")
    value = ErrorResponseException::MAX_TRAVEL_TIME_EXCEEDED;
  else if (s == "PRIOR_DATE_DEPART_NOT_CHECKED")
    value = ErrorResponseException::PRIOR_DATE_DEPART_NOT_CHECKED;
  else if (s == "NETWORK_EXCEPTION")
    value = ErrorResponseException::NETWORK_EXCEPTION;
  else if (s == "MEMORY_EXCEPTION")
    value = ErrorResponseException::MEMORY_EXCEPTION;
  else if (s == "DCA_CORBA_EXCEPTION")
    value = ErrorResponseException::DCA_CORBA_EXCEPTION;
  else if (s == "DB_CURSOR_OPEN_ERROR")
    value = ErrorResponseException::DB_CURSOR_OPEN_ERROR;
  else if (s == "DB_CURSOR_FETCH_ERROR")
    value = ErrorResponseException::DB_CURSOR_FETCH_ERROR;
  else if (s == "DB_CURSOR_CLOSE_ERROR")
    value = ErrorResponseException::DB_CURSOR_CLOSE_ERROR;
  else if (s == "DB_INSERT_ERROR")
    value = ErrorResponseException::DB_INSERT_ERROR;
  else if (s == "DB_UPDATE_ERROR")
    value = ErrorResponseException::DB_UPDATE_ERROR;
  else if (s == "DB_SELECT_ERROR")
    value = ErrorResponseException::DB_SELECT_ERROR;
  else if (s == "DB_BEGIN_WORK_ERROR")
    value = ErrorResponseException::DB_BEGIN_WORK_ERROR;
  else if (s == "DB_COMMIT_WORK_ERROR")
    value = ErrorResponseException::DB_COMMIT_WORK_ERROR;
  else if (s == "SSG_FILE_OPEN_ERROR")
    value = ErrorResponseException::SSG_FILE_OPEN_ERROR;
  else if (s == "SSG_DATABASE_ERROR")
    value = ErrorResponseException::SSG_DATABASE_ERROR;
  else if (s == "SSG_RECORD_ID_ERROR")
    value = ErrorResponseException::SSG_RECORD_ID_ERROR;
  else if (s == "SSG_PROCESSING_STARTED")
    value = ErrorResponseException::SSG_PROCESSING_STARTED;
  else if (s == "SSG_PROCESSING_COMPLETED")
    value = ErrorResponseException::SSG_PROCESSING_COMPLETED;
  else if (s == "CNP_PROCESSING_COMPLETED_EOF")
    value = ErrorResponseException::CNP_PROCESSING_COMPLETED_EOF;
  else if (s == "EQP_PROCESSING_FILE_NUMBER")
    value = ErrorResponseException::EQP_PROCESSING_FILE_NUMBER;
  else if (s == "CNP_DELETE_OLD_FAILED")
    value = ErrorResponseException::CNP_DELETE_OLD_FAILED;
  else if (s == "SSG_DELETE_FAILED")
    value = ErrorResponseException::SSG_DELETE_FAILED;
  else if (s == "SSG_ITEM_DELETED")
    value = ErrorResponseException::SSG_ITEM_DELETED;
  else if (s == "SSG_UPDATE_FAILED")
    value = ErrorResponseException::SSG_UPDATE_FAILED;
  else if (s == "SSG_ITEM_UPDATED")
    value = ErrorResponseException::SSG_ITEM_UPDATED;
  else if (s == "SSG_ADD_FAILED")
    value = ErrorResponseException::SSG_ADD_FAILED;
  else if (s == "SSG_ITEM_ADDED")
    value = ErrorResponseException::SSG_ITEM_ADDED;
  else if (s == "SSG_DUPLICATE_FOUND")
    value = ErrorResponseException::SSG_DUPLICATE_FOUND;
  else if (s == "SSG_SYSTEM_SUSPENDED")
    value = ErrorResponseException::SSG_SYSTEM_SUSPENDED;
  else if (s == "SSG_PROCESSING_RESTARTED")
    value = ErrorResponseException::SSG_PROCESSING_RESTARTED;
  else if (s == "SSG_INPUT_FILE_MISSING")
    value = ErrorResponseException::SSG_INPUT_FILE_MISSING;
  else if (s == "CNP_PROCESSING_COMPLETED_NEF")
    value = ErrorResponseException::CNP_PROCESSING_COMPLETED_NEF;
  else if (s == "SSG_CHECKPOINT_FAILED")
    value = ErrorResponseException::SSG_CHECKPOINT_FAILED;
  else if (s == "SSG_MCT_MCTREGIONS_SQL_ERROR")
    value = ErrorResponseException::SSG_MCT_MCTREGIONS_SQL_ERROR;
  else if (s == "SSG_MCT_SYSTEMPARAMETERS_SQL_ERROR")
    value = ErrorResponseException::SSG_MCT_SYSTEMPARAMETERS_SQL_ERROR;
  else if (s == "SSG_MCT_MINIMUMCONNECTTIME_SQL_ERROR")
    value = ErrorResponseException::SSG_MCT_MINIMUMCONNECTTIME_SQL_ERROR;
  else if (s == "SSG_MCT_INVALID_ENTRY_CODE")
    value = ErrorResponseException::SSG_MCT_INVALID_ENTRY_CODE;
  else if (s == "SSG_MCT_INVALID_ACTION_CODE")
    value = ErrorResponseException::SSG_MCT_INVALID_ACTION_CODE;
  else if (s == "SSG_MCT_FILES_SWITCHED")
    value = ErrorResponseException::SSG_MCT_FILES_SWITCHED;
  else if (s == "SSG_MCT_FILES_NOT_SWITCHED")
    value = ErrorResponseException::SSG_MCT_FILES_NOT_SWITCHED;
  else if (s == "SSG_MCT_SWITCH_ERROR")
    value = ErrorResponseException::SSG_MCT_SWITCH_ERROR;
  else if (s == "SSG_MCT_CACHE_NOTIFY_ERROR")
    value = ErrorResponseException::SSG_MCT_CACHE_NOTIFY_ERROR;
  else if (s == "SSG_MCT_MVS_LOAD_STARTED")
    value = ErrorResponseException::SSG_MCT_MVS_LOAD_STARTED;
  else if (s == "SSG_MCT_MVS_LOAD_COMPLETE")
    value = ErrorResponseException::SSG_MCT_MVS_LOAD_COMPLETE;
  else if (s == "SSG_MCT_MVS_LOAD_ERROR")
    value = ErrorResponseException::SSG_MCT_MVS_LOAD_ERROR;
  else if (s == "SSG_MCT_MVSFILE_LOADER_SUSPENDED")
    value = ErrorResponseException::SSG_MCT_MVSFILE_LOADER_SUSPENDED;
  else if (s == "SSG_SCHEDULE_BATCH_LOAD_STARTED")
    value = ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_STARTED;
  else if (s == "SSG_SCHEDULE_BATCH_LOAD_START_FAILED")
    value = ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_START_FAILED;
  else if (s == "SSG_SCHEDULE_BATCH_LOAD_FAILED")
    value = ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_FAILED;
  else if (s == "SSG_SCHEDULE_BATCH_LOAD_COMPLETED")
    value = ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_COMPLETED;
  else if (s == "SSG_SCHEDULE_BATCH_FILE_FORMAT_ERROR")
    value = ErrorResponseException::SSG_SCHEDULE_BATCH_FILE_FORMAT_ERROR;
  else if (s == "SSG_SCHEDULE_BATCH_PROCESSING_ERROR")
    value = ErrorResponseException::SSG_SCHEDULE_BATCH_PROCESSING_ERROR;
  else if (s == "SSG_SCHEDULE_INVALID_ARGS_ERROR")
    value = ErrorResponseException::SSG_SCHEDULE_INVALID_ARGS_ERROR;
  else if (s == "SSG_SCHEDULE_APPLICATION_INIT_ERROR")
    value = ErrorResponseException::SSG_SCHEDULE_APPLICATION_INIT_ERROR;
  else if (s == "SSG_SCHEDULE_APPLICATION_LOGIC_ERROR")
    value = ErrorResponseException::SSG_SCHEDULE_APPLICATION_LOGIC_ERROR;
  else if (s == "SSG_SCHEDULE_PROCESS_RECORD0_STARTED")
    value = ErrorResponseException::SSG_SCHEDULE_PROCESS_RECORD0_STARTED;
  else if (s == "SSG_SCHEDULE_PROCESS_RECORD0_COMPLETED")
    value = ErrorResponseException::SSG_SCHEDULE_PROCESS_RECORD0_COMPLETED;
  else if (s == "SSG_SCHEDULE_PROCESS_RECORD0_FAILED")
    value = ErrorResponseException::SSG_SCHEDULE_PROCESS_RECORD0_FAILED;
  else if (s == "SSG_SCHEDULE_TAPE_NOT_LOADED_ERROR")
    value = ErrorResponseException::SSG_SCHEDULE_TAPE_NOT_LOADED_ERROR;
  else if (s == "SSG_SCHEDULE_DYNAMIC_UPDATE_ERROR")
    value = ErrorResponseException::SSG_SCHEDULE_DYNAMIC_UPDATE_ERROR;
  else if (s == "SSG_QUERY_CHECKPOINT_FAILED")
    value = ErrorResponseException::SSG_QUERY_CHECKPOINT_FAILED;
  else if (s == "SSG_UPDATE_CHECKPOINT_FAILED")
    value = ErrorResponseException::SSG_UPDATE_CHECKPOINT_FAILED;
  else if (s == "BSR_INVALID_DATA")
    value = ErrorResponseException::BSR_INVALID_DATA;
  else if (s == "BSR_PROCESSING_START")
    value = ErrorResponseException::BSR_PROCESSING_START;
  else if (s == "BSR_ACTION_CODE")
    value = ErrorResponseException::BSR_ACTION_CODE;
  else if (s == "BSR_PROCESSING_END")
    value = ErrorResponseException::BSR_PROCESSING_END;
  else if (s == "BSR_EMU_NATION_IGNORED")
    value = ErrorResponseException::BSR_EMU_NATION_IGNORED;
  else if (s == "BSR_DB_ERROR")
    value = ErrorResponseException::BSR_DB_ERROR;
  else if (s == "FARE_CURRENCY_OVERRIDE_NOT_VALID")
    value = ErrorResponseException::FARE_CURRENCY_OVERRIDE_NOT_VALID;
  else if (s == "LMT_ISSUE_SEP_TKTS_INTL_SURFACE_RESTR")
    value = ErrorResponseException::LMT_ISSUE_SEP_TKTS_INTL_SURFACE_RESTR;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, tse::FMDirection& value)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s);

  if (s == "UNKNOWN")
    value = tse::FMDirection::UNKNOWN;
  else if (s == "INBOUND")
    value = tse::FMDirection::INBOUND;
  else if (s == "OUTBOUND")
    value = tse::FMDirection::OUTBOUND;
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, Code<15>& loc)
{
  AttributeCode(element, name, loc);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, Code<8>& loc)
{
  AttributeCode(element, name, loc);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, Code<7>& loc)
{
  AttributeCode(element, name, loc);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, Code<6>& loc)
{
  AttributeCode(element, name, loc);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, Code<5>& loc)
{
  AttributeCode(element, name, loc);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, Code<4>& loc)
{
  AttributeCode(element, name, loc);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, Code<3>& loc)
{
  AttributeCode(element, name, loc);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, Code<2>& loc)
{
  AttributeCode(element, name, loc);
}

//-------------------------------------------------------------------------------------------

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, DateTime& loc)
{
  std::string s;
  TestXMLHelper::Attribute(element, name, s); // Get string value
  loc = DateTime(s);
}

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, CabinType& cabin)
{
  Indicator s;
  TestXMLHelper::Attribute(element, name, s); // Get string value
  cabin.setClass(s);
}

/* static */
void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, FareTypeDesignator& ftd)
{
  int s;
  TestXMLHelper::Attribute(element, name, s); // Get string value
  ftd.setFareTypeDesignator(s);
}

void
TestXMLHelper::Attribute(TiXmlElement* element, const char* name, tse::RoundingRule& rr)
{
  int s;
  TestXMLHelper::Attribute(element, name, s);
  rr = static_cast<tse::RoundingRule>(s);
}

//-------------------------------------------------------------------------------------------

// **************************************************************************************************
// **************************************************************************************************
// **************************************************************************************************
// **************************************************************************************************

/* static */
std::string
TestXMLHelper::format(const std::string& str)
{
  std::string s("\"");
  s = s + str;
  s = s + "\"";

  return s;
}

std::string
TestXMLHelper::format(const BoostString& str)
{
  std::string s("\"");
  s = s + str;
  s = s + "\"";

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const int8_t& value)
{
  std::stringstream str;
  str << value;
  return str.str();
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const int16_t& value)
{
  std::stringstream str;
  str << value;
  return str.str();
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const int32_t& value)
{
  std::stringstream str;
  str << value;
  return str.str();
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const int64_t& value)
{
  std::stringstream str;
  str << value;
  return str.str();
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const uint8_t& value)
{
  std::stringstream str;
  str << value;
  return str.str();
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const uint16_t& value)
{
  std::stringstream str;
  str << value;
  return str.str();
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const uint32_t& value)
{
  std::stringstream str;
  str << value;
  return str.str();
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const uint64_t& value)
{
  std::stringstream str;
  str << value;
  return str.str();
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const double& value)
{
  std::stringstream str;
  str << value;
  return str.str();
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const char& value)
{
  std::string s("\"");
  if (value == 0)
    s = s + "\"";
  else
    s = s + value + "\"";

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const char* value)
{
  return TestXMLHelper::format(std::string(value));
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const bool& value)
{
  std::string s;

  if (value == true)
  {
    s = "\"t\"";
  }
  else
  {
    s = "\"f\"";
  }

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::GlobalDirection& value)
{
  std::string s;

  if (value == GlobalDirection::AF)
    s = TestXMLHelper::format("AF");
  else if (value == GlobalDirection::AL)
    s = TestXMLHelper::format("AL");
  else if (value == GlobalDirection::AP)
    s = TestXMLHelper::format("AP");
  else if (value == GlobalDirection::AT)
    s = TestXMLHelper::format("AT");
  else if (value == GlobalDirection::CA)
    s = TestXMLHelper::format("CA");
  else if (value == GlobalDirection::CT)
    s = TestXMLHelper::format("CT");
  else if (value == GlobalDirection::DI)
    s = TestXMLHelper::format("DI");
  else if (value == GlobalDirection::DO)
    s = TestXMLHelper::format("DO");
  else if (value == GlobalDirection::DU)
    s = TestXMLHelper::format("DU");
  else if (value == GlobalDirection::EH)
    s = TestXMLHelper::format("EH");
  else if (value == GlobalDirection::EM)
    s = TestXMLHelper::format("EM");
  else if (value == GlobalDirection::EU)
    s = TestXMLHelper::format("EU");
  else if (value == GlobalDirection::FE)
    s = TestXMLHelper::format("FE");
  else if (value == GlobalDirection::IN)
    s = TestXMLHelper::format("IN");
  else if (value == GlobalDirection::ME)
    s = TestXMLHelper::format("ME");
  else if (value == GlobalDirection::NA)
    s = TestXMLHelper::format("NA");
  else if (value == GlobalDirection::NP)
    s = TestXMLHelper::format("NP");
  else if (value == GlobalDirection::PA)
    s = TestXMLHelper::format("PA");
  else if (value == GlobalDirection::PE)
    s = TestXMLHelper::format("PE");
  else if (value == GlobalDirection::PN)
    s = TestXMLHelper::format("PN");
  else if (value == GlobalDirection::PO)
    s = TestXMLHelper::format("PO");
  else if (value == GlobalDirection::PV)
    s = TestXMLHelper::format("PV");
  else if (value == GlobalDirection::RU)
    s = TestXMLHelper::format("RU");
  else if (value == GlobalDirection::RW)
    s = TestXMLHelper::format("RW");
  else if (value == GlobalDirection::SA)
    s = TestXMLHelper::format("SA");
  else if (value == GlobalDirection::SN)
    s = TestXMLHelper::format("SN");
  else if (value == GlobalDirection::SP)
    s = TestXMLHelper::format("SP");
  else if (value == GlobalDirection::TB)
    s = TestXMLHelper::format("TB");
  else if (value == GlobalDirection::TS)
    s = TestXMLHelper::format("TS");
  else if (value == GlobalDirection::TT)
    s = TestXMLHelper::format("TT");
  else if (value == GlobalDirection::US)
    s = TestXMLHelper::format("US");
  else if (value == GlobalDirection::WH)
    s = TestXMLHelper::format("WH");
  else if (value == GlobalDirection::XX)
    s = TestXMLHelper::format("XX");
  else if (value == GlobalDirection::ZZ)
    s = TestXMLHelper::format("ZZ");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::LocType& value)
{
  std::string s;

  if (value == UNKNOWN_LOC)
    s = TestXMLHelper::format("UNKNOWN_LOC");
  else if (value == IATA_AREA)
    s = TestXMLHelper::format("IATA_AREA");
  else if (value == SUBAREA)
    s = TestXMLHelper::format("SUBAREA");
  else if (value == MARKET)
    s = TestXMLHelper::format("MARKET");
  else if (value == NATION)
    s = TestXMLHelper::format("NATION");
  else if (value == STATE_PROVINCE)
    s = TestXMLHelper::format("STATE_PROVINCE");
  else if (value == ZONE)
    s = TestXMLHelper::format("ZONE");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::DifferentialData::STATUS_TYPE& value)
{
  std::string s;

  if (value == tse::DifferentialData::SC_NOT_PROCESSED_YET)
    s = TestXMLHelper::format("NOT_PROCESSED_YET");
  else if (value == tse::DifferentialData::SC_PASSED)
    s = TestXMLHelper::format("PASSED");
  else if (value == tse::DifferentialData::SC_FAILED)
    s = TestXMLHelper::format("FAILED");
  else if (value == tse::DifferentialData::SC_MATCH_1A)
    s = TestXMLHelper::format("MATCH_1A");
  else if (value == tse::DifferentialData::SC_MATCH_1B)
    s = TestXMLHelper::format("MATCH_1B");
  else if (value == tse::DifferentialData::SC_CONSOLIDATED_PASS)
    s = TestXMLHelper::format("CONSOLIDATED_PASS");
  else if (value == tse::DifferentialData::SC_CONSOLIDATED_FAIL)
    s = TestXMLHelper::format("CONSOLIDATED_FAIL");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::GeoTravelType& value)
{
  std::string s;

  if (value == GeoTravelType::UnknownGeoTravelType)
    s = TestXMLHelper::format("UnknownGeoTravelType");
  else if (value == GeoTravelType::Domestic)
    s = TestXMLHelper::format("Domestic");
  else if (value == GeoTravelType::International)
    s = TestXMLHelper::format("International");
  else if (value == GeoTravelType::Transborder)
    s = TestXMLHelper::format("Transborder");
  else if (value == GeoTravelType::ForeignDomestic)
    s = TestXMLHelper::format("ForeignDomestic");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::FCASegDirectionality& value)
{
  std::string s;

  if (value == tse::DIR_IND_NOT_DEFINED)
    s = TestXMLHelper::format("DIR_IND_NOT_DEFINED");
  else if (value == tse::ORIGINATING_LOC1)
    s = TestXMLHelper::format("ORIGINATING_LOC1");
  else if (value == tse::ORIGINATING_LOC2)
    s = TestXMLHelper::format("ORIGINATING_LOG2");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::Directionality& value)
{
  std::string s;

  if (value == tse::FROM)
    s = TestXMLHelper::format("FROM");
  else if (value == tse::TO)
    s = TestXMLHelper::format("TO");
  else if (value == tse::BETWEEN)
    s = TestXMLHelper::format("BETWEEN");
  else if (value == tse::WITHIN)
    s = TestXMLHelper::format("WITHIN");
  else if (value == tse::BOTH)
    s = TestXMLHelper::format("BOTH");
  else if (value == tse::ORIGIN)
    s = TestXMLHelper::format("ORIGIN");
  else if (value == tse::TERMINATE)
    s = TestXMLHelper::format("TERMINATE");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(tse::PricingUnit::Type value)
{
  std::string s;

  if (value == tse::PricingUnit::Type::UNKNOWN)
    s = TestXMLHelper::format("UNKNOWN");
  else if (value == tse::PricingUnit::Type::OPENJAW)
    s = TestXMLHelper::format("OPENJAW");
  else if (value == tse::PricingUnit::Type::ROUNDTRIP)
    s = TestXMLHelper::format("ROUNDTRIP");
  else if (value == tse::PricingUnit::Type::CIRCLETRIP)
    s = TestXMLHelper::format("CIRCLETRIP");
  else if (value == tse::PricingUnit::Type::ONEWAY)
    s = TestXMLHelper::format("ONEWAY");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::PricingUnit::PUSubType& value)
{
  std::string s;

  if (value == tse::PricingUnit::UNKNOWN_SUBTYPE)
    s = TestXMLHelper::format("UNKNOWN_SUBTYPE");
  else if (value == tse::PricingUnit::DEST_OPENJAW)
    s = TestXMLHelper::format("DEST_OPENJAW");
  else if (value == tse::PricingUnit::ORIG_OPENJAW)
    s = TestXMLHelper::format("ORIG_OPENJAW");
  else if (value == tse::PricingUnit::DOUBLE_OPENJAW)
    s = TestXMLHelper::format("DOUBLE_OPENJAW");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::PricingUnit::PUFareType& value)
{
  std::string s;

  if (value == tse::PricingUnit::NL)
    s = TestXMLHelper::format("NL");
  else if (value == tse::PricingUnit::SP)
    s = TestXMLHelper::format("SP");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::RecordScope& value)
{
  std::string s;

  if (value == tse::DOMESTIC)
    s = TestXMLHelper::format("DOMESTIC");
  else if (value == tse::INTERNATIONAL)
    s = TestXMLHelper::format("INTERNATIONAL");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::SmallBitSet<uint8_t, tse::FMTravelBoundary>& value)
{
  std::string s("");
  bool oneSet = false;

  if (value.isSet(tse::FMTravelBoundary::TravelWithinUSCA))
  {
    s += "TravelWithinUSCA";
    oneSet = true;
  }
  if (value.isSet(tse::FMTravelBoundary::TravelWithinSameCountryExceptUSCA))
  {
    if (oneSet)
    {
      s += "|";
    }
    s += "TravelWithinSameCountryExceptUSCA";
    oneSet = true;
  }
  if (value.isSet(tse::FMTravelBoundary::TravelWithinOneIATA))
  {
    if (oneSet)
    {
      s += "|";
    }
    s += "TravelWithinOneIATA";
    oneSet = true;
  }
  if (value.isSet(tse::FMTravelBoundary::TravelWithinTwoIATA))
  {
    if (oneSet)
    {
      s += "|";
    }
    s += "TravelWithinTwoIATA";
    oneSet = true;
  }
  if (value.isSet(tse::FMTravelBoundary::TravelWithinAllIATA))
  {
    if (oneSet)
    {
      s += "|";
    }
    s += "TravelWithinAllIATA";
    oneSet = true;
  }
  if (value.isSet(tse::FMTravelBoundary::TravelWithinSubIATA11))
  {
    if (oneSet)
    {
      s += "|";
    }
    s += "TravelWithinSubIATA11";
    oneSet = true;
  }
  if (value.isSet(tse::FMTravelBoundary::TravelWithinSubIATA21))
  {
    if (oneSet)
    {
      s += "|";
    }
    s += "TravelWithinSubIATA21";
    oneSet = true;
  }
  if (value.isSet(tse::FMTravelBoundary::TravelWithinSameSubIATAExcept21And11))
  {
    if (oneSet)
    {
      s += "|";
    }
    s += "TravelWithinSameSubIATAExcept21And11";
    oneSet = true;
  }

  return TestXMLHelper::format(s);
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::ErrorResponseException::ErrorResponseCode& value)
{
  std::string s;

  if (value == ErrorResponseException::NO_ERROR)
    s = TestXMLHelper::format("NO_ERROR");
  else if (value == ErrorResponseException::NO_FARE_REQUESTED)
    s = TestXMLHelper::format("NO_FARE_REQUESTED");
  else if (value == ErrorResponseException::PRICING_REST_BY_GOV)
    s = TestXMLHelper::format("PRICING_REST_BY_GOV");
  else if (value == ErrorResponseException::NO_FARE_FOR_CLASS_USED)
    s = TestXMLHelper::format("NO_FARE_FOR_CLASS_USED");
  else if (value == ErrorResponseException::CITY_PAIR_RESTRICTED_BY_FBM)
    s = TestXMLHelper::format("CITY_PAIR_RESTRICTED_BY_FBM");
  else if (value == ErrorResponseException::CANNOT_COMPUTE_TAX)
    s = TestXMLHelper::format("CANNOT_COMPUTE_TAX");
  else if (value == ErrorResponseException::AIRPORT_CODE_NOT_IN_SYS)
    s = TestXMLHelper::format("AIRPORT_CODE_NOT_IN_SYS");
  else if (value == ErrorResponseException::FARE_NOT_IN_SYS)
    s = TestXMLHelper::format("FARE_NOT_IN_SYS");
  else if (value == ErrorResponseException::CHECK_DATE_TIME_CONTINUITY_OF_FLIGHTS)
    s = TestXMLHelper::format("CHECK_DATE_TIME_CONTINUITY_OF_FLIGHTS");
  else if (value == ErrorResponseException::JT_RESTRICTED_RTG_INVALID)
    s = TestXMLHelper::format("JT_RESTRICTED_RTG_INVALID");
  else if (value == ErrorResponseException::TRIP_EXCEEDS_MPM_SOM)
    s = TestXMLHelper::format("TRIP_EXCEEDS_MPM_SOM");
  else if (value == ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED)
    s = TestXMLHelper::format("MAX_NUMBER_COMBOS_EXCEEDED");
  else if (value == ErrorResponseException::FREE_SURCHARGED_STOPOVER_MAY_APPLY)
    s = TestXMLHelper::format("FREE_SURCHARGED_STOPOVER_MAY_APPLY");
  else if (value == ErrorResponseException::TOO_MANY_SURFACE_SEGS)
    s = TestXMLHelper::format("TOO_MANY_SURFACE_SEGS");
  else if (value == ErrorResponseException::NO_DATE_IN_OPEN_SEG)
    s = TestXMLHelper::format("NO_DATE_IN_OPEN_SEG");
  else if (value == ErrorResponseException::CHECK_LINE_OF_FLIGHT)
    s = TestXMLHelper::format("CHECK_LINE_OF_FLIGHT");
  else if (value == ErrorResponseException::STOPOVER_SURCHARGE_MAY_APPLY)
    s = TestXMLHelper::format("STOPOVER_SURCHARGE_MAY_APPLY");
  else if (value == ErrorResponseException::NO_RULES_FOR_PSGR_TYPE_OR_CLASS)
    s = TestXMLHelper::format("NO_RULES_FOR_PSGR_TYPE_OR_CLASS");
  else if (value == ErrorResponseException::NO_THRU_CLASS_FOR_ENROUTE_CHANGE_FLT)
    s = TestXMLHelper::format("NO_THRU_CLASS_FOR_ENROUTE_CHANGE_FLT");
  else if (value == ErrorResponseException::FIRST_SEG_OPEN)
    s = TestXMLHelper::format("FIRST_SEG_OPEN");
  else if (value == ErrorResponseException::RETRY_IN_ONE_MINUTE)
    s = TestXMLHelper::format("RETRY_IN_ONE_MINUTE");
  else if (value == ErrorResponseException::NO_TIMES_IN_OA_SEG)
    s = TestXMLHelper::format("NO_TIMES_IN_OA_SEG");
  else if (value == ErrorResponseException::SYSTEM_ERROR)
    s = TestXMLHelper::format("SYSTEM_ERROR");
  else if (value == ErrorResponseException::FARE_RESTRICTED_FROM_PRICING)
    s = TestXMLHelper::format("FARE_RESTRICTED_FROM_PRICING");
  else if (value == ErrorResponseException::INVALID_ROUTING_OR_SEG_BETWEEN_CO_TERM)
    s = TestXMLHelper::format("INVALID_ROUTING_OR_SEG_BETWEEN_CO_TERM");
  else if (value == ErrorResponseException::PRICING_AT_PEAK_USAGE)
    s = TestXMLHelper::format("PRICING_AT_PEAK_USAGE");
  else if (value == ErrorResponseException::MIXED_CLASS_TRIP)
    s = TestXMLHelper::format("MIXED_CLASS_TRIP");
  else if (value == ErrorResponseException::MAX_PERMITTED_MILEAGE_NOT_AVAIL)
    s = TestXMLHelper::format("MAX_PERMITTED_MILEAGE_NOT_AVAIL");
  else if (value == ErrorResponseException::MAX_SEGS_EXCEEDED)
    s = TestXMLHelper::format("MAX_SEGS_EXCEEDED");
  else if (value == ErrorResponseException::TOTAL_FARE_TOO_LARGE)
    s = TestXMLHelper::format("TOTAL_FARE_TOO_LARGE");
  else if (value == ErrorResponseException::SYSTEM_DATA_ERROR)
    s = TestXMLHelper::format("SYSTEM_DATA_ERROR");
  else if (value == ErrorResponseException::FAILED_DUE_TO_COMBO_RESTRICTIONS)
    s = TestXMLHelper::format("FAILED_DUE_TO_COMBO_RESTRICTIONS");
  else if (value == ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS)
    s = TestXMLHelper::format("NO_COMBINABLE_FARES_FOR_CLASS");
  else if (value == ErrorResponseException::FARE_BASIS_EXCEED_7CHAR_FOR_TAT_TM)
    s = TestXMLHelper::format("FARE_BASIS_EXCEED_7CHAR_FOR_TAT_TM");
  else if (value == ErrorResponseException::MAX_COMBOS_EXCEEDED_FOR_AUTO_PRICING)
    s = TestXMLHelper::format("MAX_COMBOS_EXCEEDED_FOR_AUTO_PRICING");
  else if (value == ErrorResponseException::CANNOT_FORMAT_TAX)
    s = TestXMLHelper::format("CANNOT_FORMAT_TAX");
  else if (value == ErrorResponseException::NBR_PSGRS_EXCEEDS_OA_AVAIL)
    s = TestXMLHelper::format("NBR_PSGRS_EXCEEDS_OA_AVAIL");
  else if (value == ErrorResponseException::NO_FARE_VALID_FOR_PSGR_TYPE)
    s = TestXMLHelper::format("NO_FARE_VALID_FOR_PSGR_TYPE");
  else if (value == ErrorResponseException::EXCEEDS_ALLOWED_SEGS_CHANGED_CLASS_FLIGHTS)
    s = TestXMLHelper::format("EXCEEDS_ALLOWED_SEGS_CHANGED_CLASS_FLIGHTS");
  else if (value == ErrorResponseException::CHECK_FLIGHT)
    s = TestXMLHelper::format("CHECK_FLIGHT");
  else if (value == ErrorResponseException::FARE_CALC_TOO_LARGE_FOR_ATB)
    s = TestXMLHelper::format("FARE_CALC_TOO_LARGE_FOR_ATB");
  else if (value == ErrorResponseException::CANNOT_CALC_SD_FARE)
    s = TestXMLHelper::format("CANNOT_CALC_SD_FARE");
  else if (value == ErrorResponseException::TICKET_DESIGNATOR_NOT_ALLOWED)
    s = TestXMLHelper::format("TICKET_DESIGNATOR_NOT_ALLOWED");
  else if (value == ErrorResponseException::WPNJ_NOT_VALID_ON_THIS_ITIN)
    s = TestXMLHelper::format("WPNJ_NOT_VALID_ON_THIS_ITIN");
  else if (value == ErrorResponseException::CANNOT_CALCULATE_CURRENCY)
    s = TestXMLHelper::format("CANNOT_CALCULATE_CURRENCY");
  else if (value == ErrorResponseException::CANNOT_ROUND_CURRENCY)
    s = TestXMLHelper::format("CANNOT_ROUND_CURRENCY");
  else if (value == ErrorResponseException::BREAK_FARE_INVALID)
    s = TestXMLHelper::format("BREAK_FARE_INVALID");
  else if (value == ErrorResponseException::RETRY)
    s = TestXMLHelper::format("RETRY");
  else if (value == ErrorResponseException::NEED_COMMISSION)
    s = TestXMLHelper::format("NEED_COMMISSION");
  else if (value == ErrorResponseException::OPEN_JAW_MAY_APPLY)
    s = TestXMLHelper::format("OPEN_JAW_MAY_APPLY");
  else if (value == ErrorResponseException::FLT_CHK_DATE_TIME_CONTINUITY_OF_FLTS)
    s = TestXMLHelper::format("FLT_CHK_DATE_TIME_CONTINUITY_OF_FLTS");
  else if (value == ErrorResponseException::FARE_BASIS_NOT_AVAIL)
    s = TestXMLHelper::format("FARE_BASIS_NOT_AVAIL");
  else if (value == ErrorResponseException::WORLDFARE_AT_PEAK_USE)
    s = TestXMLHelper::format("WORLDFARE_AT_PEAK_USE");
  else if (value == ErrorResponseException::WORLDFARE_UNAVAILABLE)
    s = TestXMLHelper::format("WORLDFARE_UNAVAILABLE");
  else if (value == ErrorResponseException::TKT_DES_FAILE_RULES_CHECK)
    s = TestXMLHelper::format("TKT_DES_FAILE_RULES_CHECK");
  else if (value == ErrorResponseException::TKT_DES_RECORD_RETRIEVAL_ERROR)
    s = TestXMLHelper::format("TKT_DES_RECORD_RETRIEVAL_ERROR");
  else if (value == ErrorResponseException::PROCESSING_ERROR_DETECTED)
    s = TestXMLHelper::format("PROCESSING_ERROR_DETECTED");
  else if (value == ErrorResponseException::PENALTY_DATA_INCOMPLETE)
    s = TestXMLHelper::format("PENALTY_DATA_INCOMPLETE");
  else if (value == ErrorResponseException::LOW_FARE_NOT_ALLOWED_FOR_CLASS_USED)
    s = TestXMLHelper::format("LOW_FARE_NOT_ALLOWED_FOR_CLASS_USED");
  else if (value == ErrorResponseException::WRONG_NUMBER_OF_FARES)
    s = TestXMLHelper::format("WRONG_NUMBER_OF_FARES");
  else if (value == ErrorResponseException::DISCOUNT_AMT_EXCEEDS_FARE_COMPONENT_TOTAL)
    s = TestXMLHelper::format("DISCOUNT_AMT_EXCEEDS_FARE_COMPONENT_TOTAL");
  else if (value == ErrorResponseException::ENTRIES_NOT_COMBINABLE_WITH_DISCOUNT)
    s = TestXMLHelper::format("ENTRIES_NOT_COMBINABLE_WITH_DISCOUNT");
  else if (value == ErrorResponseException::OPEN_RETURN_REQUIRED)
    s = TestXMLHelper::format("OPEN_RETURN_REQUIRED");
  else if (value == ErrorResponseException::NO_FARE_BASIS_FOR_AIRPORT_PAIR_AGENCY)
    s = TestXMLHelper::format("NO_FARE_BASIS_FOR_AIRPORT_PAIR_AGENCY");
  else if (value == ErrorResponseException::CANNOT_COMPUTE_TAX_COMPUTE_MANUALLY)
    s = TestXMLHelper::format("CANNOT_COMPUTE_TAX_COMPUTE_MANUALLY");
  else if (value == ErrorResponseException::CANNOT_TAX_INSERT_TAX_AFTER_TX)
    s = TestXMLHelper::format("CANNOT_TAX_INSERT_TAX_AFTER_TX");
  else if (value == ErrorResponseException::MULTI_DISCOUNTS_EXIST)
    s = TestXMLHelper::format("MULTI_DISCOUNTS_EXIST");
  else if (value == ErrorResponseException::NO_VALID_DISCOUNT_PUB_FARE)
    s = TestXMLHelper::format("NO_VALID_DISCOUNT_PUB_FARE");
  else if (value == ErrorResponseException::SCHEDULES_NOT_AVAILABLE_FOR_CARRIER_DATE)
    s = TestXMLHelper::format("SCHEDULES_NOT_AVAILABLE_FOR_CARRIER_DATE");
  else if (value == ErrorResponseException::DATA_ERROR_DETECTED)
    s = TestXMLHelper::format("DATA_ERROR_DETECTED");
  else if (value == ErrorResponseException::NO_PNR_CREATED)
    s = TestXMLHelper::format("NO_PNR_CREATED");
  else if (value == ErrorResponseException::NO_FARE_FOR_CLASS)
    s = TestXMLHelper::format("NO_FARE_FOR_CLASS");
  else if (value == ErrorResponseException::ErrorResponseCode::CANNOT_PRICE_AS_REQUESTED)
    s = TestXMLHelper::format("CANNOT_PRICE_AS_REQUESTED");
  else if (value == ErrorResponseException::NO_FARES_RBD_CARRIER)
    s = TestXMLHelper::format("NO_FARES_RBD_CARRIER");
  else if (value == ErrorResponseException::NEGOTIATED_FARES_APPLY)
    s = TestXMLHelper::format("NEGOTIATED_FARES_APPLY");
  else if (value == ErrorResponseException::NO_FARES_FOUND_FOR_FARE_COMPONENT)
    s = TestXMLHelper::format("NO_FARES_FOUND_FOR_FARE_COMPONENT");
  else if (value == ErrorResponseException::CODESHARE_PROCESSING_ERROR)
    s = TestXMLHelper::format("CODESHARE_PROCESSING_ERROR");
  else if (value == ErrorResponseException::NO_MATCH_FOR_FARE_COMPONENT)
    s = TestXMLHelper::format("NO_MATCH_FOR_FARE_COMPONENT");
  else if (value == ErrorResponseException::INVALID_FARE_BASIS_FOR_CARRIER)
    s = TestXMLHelper::format("INVALID_FARE_BASIS_FOR_CARRIER");
  else if (value == ErrorResponseException::REQ_CARRIER_HAS_NO_FARES)
    s = TestXMLHelper::format("REQ_CARRIER_HAS_NO_FARES");
  else if (value == ErrorResponseException::NEGOTIATED_FARES_APPLY)
    s = TestXMLHelper::format("NEGOTIATED_FARES_APPLY");
  else if (value == ErrorResponseException::NO_FARES_FOUND_FOR_FARE_COMPONENT)
    s = TestXMLHelper::format("NO_FARES_FOUND_FOR_FARE_COMPONENT");
  else if (value == ErrorResponseException::CODESHARE_PROCESSING_ERROR)
    s = TestXMLHelper::format("CODESHARE_PROCESSING_ERROR");
  else if (value == ErrorResponseException::NO_MATCH_FOR_FARE_COMPONENT)
    s = TestXMLHelper::format("NO_MATCH_FOR_FARE_COMPONENT");
  else if (value == ErrorResponseException::INVALID_FARE_BASIS_FOR_CARRIER)
    s = TestXMLHelper::format("INVALID_FARE_BASIS_FOR_CARRIER");
  else if (value == ErrorResponseException::REQ_CARRIER_HAS_NO_FARES)
    s = TestXMLHelper::format("REQ_CARRIER_HAS_NO_FARES");
  else if (value == ErrorResponseException::INVALID_JOURNEY_RECORD)
    s = TestXMLHelper::format("INVALID_JOURNEY_RECORD");
  else if (value == ErrorResponseException::INVALID_INPUT)
    s = TestXMLHelper::format("INVALID_INPUT");
  else if (value == ErrorResponseException::INVALID_DUTY_CODE)
    s = TestXMLHelper::format("INVALID_DUTY_CODE");
  else if (value == ErrorResponseException::NO_ITIN_SEGS_FOUND)
    s = TestXMLHelper::format("NO_ITIN_SEGS_FOUND");
  else if (value == ErrorResponseException::PNR_DATABASE_ERROR)
    s = TestXMLHelper::format("PNR_DATABASE_ERROR");
  else if (value == ErrorResponseException::NEED_PREFERRED_CARRIER)
    s = TestXMLHelper::format("NEED_PREFERRED_CARRIER");
  else if (value == ErrorResponseException::MAX_PREF_CARRIERS_EXCEEDED)
    s = TestXMLHelper::format("MAX_PREF_CARRIERS_EXCEEDED");
  else if (value == ErrorResponseException::MAX_PASSENGERS_EXCEEDED)
    s = TestXMLHelper::format("MAX_PASSENGERS_EXCEEDED");
  else if (value == ErrorResponseException::INVALID_SAME_PREF_AND_NON_PREF_CARRIER)
    s = TestXMLHelper::format("INVALID_SAME_PREF_AND_NON_PREF_CARRIER");
  else if (value == ErrorResponseException::ENTER_ACTIVE_SEGS)
    s = TestXMLHelper::format("ENTER_ACTIVE_SEGS");
  else if (value == ErrorResponseException::CONFLICTING_OPTIONS)
    s = TestXMLHelper::format("CONFLICTING_OPTIONS");
  else if (value == ErrorResponseException::INVALID_SEG_TYPE_IN_PNR_RETIEVAL)
    s = TestXMLHelper::format("INVALID_SEG_TYPE_IN_PNR_RETIEVAL");
  else if (value == ErrorResponseException::PNR_SEGS_NOT_FOUND)
    s = TestXMLHelper::format("PNR_SEGS_NOT_FOUND");
  else if (value == ErrorResponseException::CHECK_SEG_CONTINUITY)
    s = TestXMLHelper::format("CHECK_SEG_CONTINUITY");
  else if (value == ErrorResponseException::INVALID_TERMINAL_TYPE)
    s = TestXMLHelper::format("INVALID_TERMINAL_TYPE");
  else if (value == ErrorResponseException::MULTI_CONNECT)
    s = TestXMLHelper::format("MULTI_CONNECT");
  else if (value == ErrorResponseException::DUPLICATE_X_AND_O_QUALIFIER)
    s = TestXMLHelper::format("DUPLICATE_X_AND_O_QUALIFIER");
  else if (value == ErrorResponseException::CANNOT_USE_SEG_NUMBER)
    s = TestXMLHelper::format("CANNOT_USE_SEG_NUMBER");
  else if (value == ErrorResponseException::BOOK_SEPARATE_PNR)
    s = TestXMLHelper::format("BOOK_SEPARATE_PNR");
  else if (value == ErrorResponseException::MAX_FOUR_SEATS)
    s = TestXMLHelper::format("MAX_FOUR_SEATS");
  else if (value == ErrorResponseException::TRY_DIAGNOSTIC_ENTRY_10_20)
    s = TestXMLHelper::format("TRY_DIAGNOSTIC_ENTRY_10_20");
  else if (value == ErrorResponseException::TRY_DIAGNOSTIC_ENTRY_1_6)
    s = TestXMLHelper::format("TRY_DIAGNOSTIC_ENTRY_1_6");
  else if (value == ErrorResponseException::SPECIFY_PUBLIC_OR_PRIVATE)
    s = TestXMLHelper::format("SPECIFY_PUBLIC_OR_PRIVATE");
  else if (value == ErrorResponseException::NO_DIAGNOSTIC_TO_DISPLAY)
    s = TestXMLHelper::format("NO_DIAGNOSTIC_TO_DISPLAY");
  else if (value == ErrorResponseException::NO_CONVERTED_FARE_BASIS_CODE)
    s = TestXMLHelper::format("NO_CONVERTED_FARE_BASIS_CODE");
  else if (value == ErrorResponseException::NO_PQ_ITEMS_FOR_GIVEN_ORDINAL)
    s = TestXMLHelper::format("NO_PQ_ITEMS_FOR_GIVEN_ORDINAL");
  else if (value == ErrorResponseException::FARENET_PROCESSING_NOT_YET_AVAIL)
    s = TestXMLHelper::format("FARENET_PROCESSING_NOT_YET_AVAIL");
  else if (value == ErrorResponseException::CALL_DIRECT)
    s = TestXMLHelper::format("CALL_DIRECT");
  else if (value == ErrorResponseException::NO_FLIGHTS_FOUND)
    s = TestXMLHelper::format("NO_FLIGHTS_FOUND");
  else if (value == ErrorResponseException::INVALID_CLASS)
    s = TestXMLHelper::format("INVALID_CLASS");
  else if (value == ErrorResponseException::NO_COMBOS)
    s = TestXMLHelper::format("NO_COMBOS");
  else if (value == ErrorResponseException::INVALID_FORMAT)
    s = TestXMLHelper::format("INVALID_FORMAT");
  else if (value == ErrorResponseException::NO_TRIP)
    s = TestXMLHelper::format("NO_TRIP");
  else if (value == ErrorResponseException::CANNOT_SELL_SEG)
    s = TestXMLHelper::format("CANNOT_SELL_SEG");
  else if (value == ErrorResponseException::MUST_BE_PRICED_FIRST)
    s = TestXMLHelper::format("MUST_BE_PRICED_FIRST");
  else if (value == ErrorResponseException::D1_NO_FLIGHT_ITEM_BLOCKS)
    s = TestXMLHelper::format("D1_NO_FLIGHT_ITEM_BLOCKS");
  else if (value == ErrorResponseException::D1_FIND_ERR)
    s = TestXMLHelper::format("D1_FIND_ERR");
  else if (value == ErrorResponseException::D2_NO_FLIGHT_ITEM_SORT_LIST_BLOCKS)
    s = TestXMLHelper::format("D2_NO_FLIGHT_ITEM_SORT_LIST_BLOCKS");
  else if (value == ErrorResponseException::D3_NO_FLIGHT_COMB_BLOCKS)
    s = TestXMLHelper::format("D3_NO_FLIGHT_COMB_BLOCKS");
  else if (value == ErrorResponseException::D3_INVALID_JRFCT_PARMS)
    s = TestXMLHelper::format("D3_INVALID_JRFCT_PARMS");
  else if (value == ErrorResponseException::QJRA_TRAN_VECTOR_ERR)
    s = TestXMLHelper::format("QJRA_TRAN_VECTOR_ERR");
  else if (value == ErrorResponseException::QJRB_TRAN_VECTOR_ERR)
    s = TestXMLHelper::format("QJRB_TRAN_VECTOR_ERR");
  else if (value == ErrorResponseException::NO_DIRECTS_NONSTOPS)
    s = TestXMLHelper::format("NO_DIRECTS_NONSTOPS");
  else if (value == ErrorResponseException::TOO_MANY_COMBOS)
    s = TestXMLHelper::format("TOO_MANY_COMBOS");
  else if (value == ErrorResponseException::CORPORATE_PRICING_ACTIVE)
    s = TestXMLHelper::format("CORPORATE_PRICING_ACTIVE");
  else if (value == ErrorResponseException::ALT_CITIES_INVALID_FOR_ARNK)
    s = TestXMLHelper::format("ALT_CITIES_INVALID_FOR_ARNK");
  else if (value == ErrorResponseException::ALT_DATES_INVALID_FOR_ARNK)
    s = TestXMLHelper::format("ALT_DATES_INVALID_FOR_ARNK");
  else if (value == ErrorResponseException::TRIP_DURATION_INVALID_FOR_ARNK)
    s = TestXMLHelper::format("TRIP_DURATION_INVALID_FOR_ARNK");
  else if (value == ErrorResponseException::MAX_CONNECTION_TIME_INVALID_FOR_ARNK)
    s = TestXMLHelper::format("MAX_CONNECTION_TIME_INVALID_FOR_ARNK");
  else if (value == ErrorResponseException::MAX_TRAVEL_TIME_INVALID_FOR_ARNK)
    s = TestXMLHelper::format("MAX_TRAVEL_TIME_INVALID_FOR_ARNK");
  else if (value == ErrorResponseException::ALT_CITIES_NOT_ALLOWED)
    s = TestXMLHelper::format("ALT_CITIES_NOT_ALLOWED");
  else if (value == ErrorResponseException::INVALID_SEG_NUMBER)
    s = TestXMLHelper::format("INVALID_SEG_NUMBER");
  else if (value == ErrorResponseException::INVALID_CITY_AIRPORT_CODE)
    s = TestXMLHelper::format("INVALID_CITY_AIRPORT_CODE");
  else if (value == ErrorResponseException::NEED_SEPARATOR)
    s = TestXMLHelper::format("NEED_SEPARATOR");
  else if (value == ErrorResponseException::SEG_DOES_NOT_MATCH_CITY_AIRPORT)
    s = TestXMLHelper::format("SEG_DOES_NOT_MATCH_CITY_AIRPORT");
  else if (value == ErrorResponseException::NO_CITY_AIRPORT_FOR_ALTS)
    s = TestXMLHelper::format("NO_CITY_AIRPORT_FOR_ALTS");
  else if (value == ErrorResponseException::ALT_CITIES_AIRPORTS_INVALID_FOR_CONNECT)
    s = TestXMLHelper::format("ALT_CITIES_AIRPORTS_INVALID_FOR_CONNECT");
  else if (value == ErrorResponseException::ALT_CITY_AIRPORT_AND_MILEAGE_RANGE)
    s = TestXMLHelper::format("ALT_CITY_AIRPORT_AND_MILEAGE_RANGE");
  else if (value == ErrorResponseException::MILEAGE_RANGE_INVALID)
    s = TestXMLHelper::format("MILEAGE_RANGE_INVALID");
  else if (value == ErrorResponseException::INVALID_ALT_CITY_AIRPORT_CODE)
    s = TestXMLHelper::format("INVALID_ALT_CITY_AIRPORT_CODE");
  else if (value == ErrorResponseException::INTERNATIONAL_ALT_CITY_AS_PSEUDO)
    s = TestXMLHelper::format("INTERNATIONAL_ALT_CITY_AS_PSEUDO");
  else if (value == ErrorResponseException::MAX_ALT_AIRPORT_CITIES_EXCEEDED)
    s = TestXMLHelper::format("MAX_ALT_AIRPORT_CITIES_EXCEEDED");
  else if (value == ErrorResponseException::DUPLICATE_BOARD_OFF_POINT_IN_SEG)
    s = TestXMLHelper::format("DUPLICATE_BOARD_OFF_POINT_IN_SEG");
  else if (value == ErrorResponseException::SPECIFY_SEGS_FOR_NONCONSECUTIVE_ALT_DATES)
    s = TestXMLHelper::format("SPECIFY_SEGS_FOR_NONCONSECUTIVE_ALT_DATES");
  else if (value == ErrorResponseException::INCLUDE_CONNECTING_SEGS)
    s = TestXMLHelper::format("INCLUDE_CONNECTING_SEGS");
  else if (value == ErrorResponseException::INVALID_NUMBER_FOR_ALT_DATES)
    s = TestXMLHelper::format("INVALID_NUMBER_FOR_ALT_DATES");
  else if (value == ErrorResponseException::COMBINED_CONSEC_NONCONSEC_ALT_DATES)
    s = TestXMLHelper::format("COMBINED_CONSEC_NONCONSEC_ALT_DATES");
  else if (value == ErrorResponseException::INVALID_DATE_ENTRY_FOR_ALT_TRAVEL_DATES)
    s = TestXMLHelper::format("INVALID_DATE_ENTRY_FOR_ALT_TRAVEL_DATES");
  else if (value == ErrorResponseException::MAX_NUMBER_ALT_TRAVEL_DATES_EXCEEDED)
    s = TestXMLHelper::format("MAX_NUMBER_ALT_TRAVEL_DATES_EXCEEDED");
  else if (value == ErrorResponseException::DURATION_ONLY_VALID_FOR_ALT_DATES)
    s = TestXMLHelper::format("DURATION_ONLY_VALID_FOR_ALT_DATES");
  else if (value == ErrorResponseException::DURATION_ONLY_VALID_FOR_CONSEC_ALT_DATES)
    s = TestXMLHelper::format("DURATION_ONLY_VALID_FOR_CONSEC_ALT_DATES");
  else if (value == ErrorResponseException::DURATION_IS_INVALID)
    s = TestXMLHelper::format("DURATION_IS_INVALID");
  else if (value == ErrorResponseException::DURATION_ONLY_VALID_FOR_CONSEC_STOPOVERS)
    s = TestXMLHelper::format("DURATION_ONLY_VALID_FOR_CONSEC_STOPOVERS");
  else if (value == ErrorResponseException::MAX_CONNECTION_TIME_EXCEEDED)
    s = TestXMLHelper::format("MAX_CONNECTION_TIME_EXCEEDED");
  else if (value == ErrorResponseException::MAX_TRAVEL_TIME_EXCEEDED)
    s = TestXMLHelper::format("MAX_TRAVEL_TIME_EXCEEDED");
  else if (value == ErrorResponseException::PRIOR_DATE_DEPART_NOT_CHECKED)
    s = TestXMLHelper::format("PRIOR_DATE_DEPART_NOT_CHECKED");
  else if (value == ErrorResponseException::NETWORK_EXCEPTION)
    s = TestXMLHelper::format("NETWORK_EXCEPTION");
  else if (value == ErrorResponseException::MEMORY_EXCEPTION)
    s = TestXMLHelper::format("MEMORY_EXCEPTION");
  else if (value == ErrorResponseException::DCA_CORBA_EXCEPTION)
    s = TestXMLHelper::format("DCA_CORBA_EXCEPTION");
  else if (value == ErrorResponseException::DB_CURSOR_OPEN_ERROR)
    s = TestXMLHelper::format("DB_CURSOR_OPEN_ERROR");
  else if (value == ErrorResponseException::DB_CURSOR_FETCH_ERROR)
    s = TestXMLHelper::format("DB_CURSOR_FETCH_ERROR");
  else if (value == ErrorResponseException::DB_CURSOR_CLOSE_ERROR)
    s = TestXMLHelper::format("DB_CURSOR_CLOSE_ERROR");
  else if (value == ErrorResponseException::DB_INSERT_ERROR)
    s = TestXMLHelper::format("DB_INSERT_ERROR");
  else if (value == ErrorResponseException::DB_UPDATE_ERROR)
    s = TestXMLHelper::format("DB_UPDATE_ERROR");
  else if (value == ErrorResponseException::DB_SELECT_ERROR)
    s = TestXMLHelper::format("DB_SELECT_ERROR");
  else if (value == ErrorResponseException::DB_BEGIN_WORK_ERROR)
    s = TestXMLHelper::format("DB_BEGIN_WORK_ERROR");
  else if (value == ErrorResponseException::DB_COMMIT_WORK_ERROR)
    s = TestXMLHelper::format("DB_COMMIT_WORK_ERROR");
  else if (value == ErrorResponseException::SSG_FILE_OPEN_ERROR)
    s = TestXMLHelper::format("SSG_FILE_OPEN_ERROR");
  else if (value == ErrorResponseException::SSG_DATABASE_ERROR)
    s = TestXMLHelper::format("SSG_DATABASE_ERROR");
  else if (value == ErrorResponseException::SSG_RECORD_ID_ERROR)
    s = TestXMLHelper::format("SSG_RECORD_ID_ERROR");
  else if (value == ErrorResponseException::SSG_PROCESSING_STARTED)
    s = TestXMLHelper::format("SSG_PROCESSING_STARTED");
  else if (value == ErrorResponseException::SSG_PROCESSING_COMPLETED)
    s = TestXMLHelper::format("SSG_PROCESSING_COMPLETED");
  else if (value == ErrorResponseException::CNP_PROCESSING_COMPLETED_EOF)
    s = TestXMLHelper::format("CNP_PROCESSING_COMPLETED_EOF");
  else if (value == ErrorResponseException::EQP_PROCESSING_FILE_NUMBER)
    s = TestXMLHelper::format("EQP_PROCESSING_FILE_NUMBER");
  else if (value == ErrorResponseException::CNP_DELETE_OLD_FAILED)
    s = TestXMLHelper::format("CNP_DELETE_OLD_FAILED");
  else if (value == ErrorResponseException::SSG_DELETE_FAILED)
    s = TestXMLHelper::format("SSG_DELETE_FAILED");
  else if (value == ErrorResponseException::SSG_ITEM_DELETED)
    s = TestXMLHelper::format("SSG_ITEM_DELETED");
  else if (value == ErrorResponseException::SSG_UPDATE_FAILED)
    s = TestXMLHelper::format("SSG_UPDATE_FAILED");
  else if (value == ErrorResponseException::SSG_ITEM_UPDATED)
    s = TestXMLHelper::format("SSG_ITEM_UPDATED");
  else if (value == ErrorResponseException::SSG_ADD_FAILED)
    s = TestXMLHelper::format("SSG_ADD_FAILED");
  else if (value == ErrorResponseException::SSG_ITEM_ADDED)
    s = TestXMLHelper::format("SSG_ITEM_ADDED");
  else if (value == ErrorResponseException::SSG_DUPLICATE_FOUND)
    s = TestXMLHelper::format("SSG_DUPLICATE_FOUND");
  else if (value == ErrorResponseException::SSG_SYSTEM_SUSPENDED)
    s = TestXMLHelper::format("SSG_SYSTEM_SUSPENDED");
  else if (value == ErrorResponseException::SSG_PROCESSING_RESTARTED)
    s = TestXMLHelper::format("SSG_PROCESSING_RESTARTED");
  else if (value == ErrorResponseException::SSG_INPUT_FILE_MISSING)
    s = TestXMLHelper::format("SSG_INPUT_FILE_MISSING");
  else if (value == ErrorResponseException::CNP_PROCESSING_COMPLETED_NEF)
    s = TestXMLHelper::format("CNP_PROCESSING_COMPLETED_NEF");
  else if (value == ErrorResponseException::SSG_CHECKPOINT_FAILED)
    s = TestXMLHelper::format("SSG_CHECKPOINT_FAILED");
  else if (value == ErrorResponseException::SSG_MCT_MCTREGIONS_SQL_ERROR)
    s = TestXMLHelper::format("SSG_MCT_MCTREGIONS_SQL_ERROR");
  else if (value == ErrorResponseException::SSG_MCT_SYSTEMPARAMETERS_SQL_ERROR)
    s = TestXMLHelper::format("SSG_MCT_SYSTEMPARAMETERS_SQL_ERROR");
  else if (value == ErrorResponseException::SSG_MCT_MINIMUMCONNECTTIME_SQL_ERROR)
    s = TestXMLHelper::format("SSG_MCT_MINIMUMCONNECTTIME_SQL_ERROR");
  else if (value == ErrorResponseException::SSG_MCT_INVALID_ENTRY_CODE)
    s = TestXMLHelper::format("SSG_MCT_INVALID_ENTRY_CODE");
  else if (value == ErrorResponseException::SSG_MCT_INVALID_ACTION_CODE)
    s = TestXMLHelper::format("SSG_MCT_INVALID_ACTION_CODE");
  else if (value == ErrorResponseException::SSG_MCT_FILES_SWITCHED)
    s = TestXMLHelper::format("SSG_MCT_FILES_SWITCHED");
  else if (value == ErrorResponseException::SSG_MCT_FILES_NOT_SWITCHED)
    s = TestXMLHelper::format("SSG_MCT_FILES_NOT_SWITCHED");
  else if (value == ErrorResponseException::SSG_MCT_SWITCH_ERROR)
    s = TestXMLHelper::format("SSG_MCT_SWITCH_ERROR");
  else if (value == ErrorResponseException::SSG_MCT_CACHE_NOTIFY_ERROR)
    s = TestXMLHelper::format("SSG_MCT_CACHE_NOTIFY_ERROR");
  else if (value == ErrorResponseException::SSG_MCT_MVS_LOAD_STARTED)
    s = TestXMLHelper::format("SSG_MCT_MVS_LOAD_STARTED");
  else if (value == ErrorResponseException::SSG_MCT_MVS_LOAD_COMPLETE)
    s = TestXMLHelper::format("SSG_MCT_MVS_LOAD_COMPLETE");
  else if (value == ErrorResponseException::SSG_MCT_MVS_LOAD_ERROR)
    s = TestXMLHelper::format("SSG_MCT_MVS_LOAD_ERROR");
  else if (value == ErrorResponseException::SSG_MCT_MVSFILE_LOADER_SUSPENDED)
    s = TestXMLHelper::format("SSG_MCT_MVSFILE_LOADER_SUSPENDED");
  else if (value == ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_STARTED)
    s = TestXMLHelper::format("SSG_SCHEDULE_BATCH_LOAD_STARTED");
  else if (value == ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_START_FAILED)
    s = TestXMLHelper::format("SSG_SCHEDULE_BATCH_LOAD_START_FAILED");
  else if (value == ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_FAILED)
    s = TestXMLHelper::format("SSG_SCHEDULE_BATCH_LOAD_FAILED");
  else if (value == ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_COMPLETED)
    s = TestXMLHelper::format("SSG_SCHEDULE_BATCH_LOAD_COMPLETED");
  else if (value == ErrorResponseException::SSG_SCHEDULE_BATCH_FILE_FORMAT_ERROR)
    s = TestXMLHelper::format("SSG_SCHEDULE_BATCH_FILE_FORMAT_ERROR");
  else if (value == ErrorResponseException::SSG_SCHEDULE_BATCH_PROCESSING_ERROR)
    s = TestXMLHelper::format("SSG_SCHEDULE_BATCH_PROCESSING_ERROR");
  else if (value == ErrorResponseException::SSG_SCHEDULE_INVALID_ARGS_ERROR)
    s = TestXMLHelper::format("SSG_SCHEDULE_INVALID_ARGS_ERROR");
  else if (value == ErrorResponseException::SSG_SCHEDULE_APPLICATION_INIT_ERROR)
    s = TestXMLHelper::format("SSG_SCHEDULE_APPLICATION_INIT_ERROR");
  else if (value == ErrorResponseException::SSG_SCHEDULE_APPLICATION_LOGIC_ERROR)
    s = TestXMLHelper::format("SSG_SCHEDULE_APPLICATION_LOGIC_ERROR");
  else if (value == ErrorResponseException::SSG_SCHEDULE_PROCESS_RECORD0_STARTED)
    s = TestXMLHelper::format("SSG_SCHEDULE_PROCESS_RECORD0_STARTED");
  else if (value == ErrorResponseException::SSG_SCHEDULE_PROCESS_RECORD0_COMPLETED)
    s = TestXMLHelper::format("SSG_SCHEDULE_PROCESS_RECORD0_COMPLETED");
  else if (value == ErrorResponseException::SSG_SCHEDULE_PROCESS_RECORD0_FAILED)
    s = TestXMLHelper::format("SSG_SCHEDULE_PROCESS_RECORD0_FAILED");
  else if (value == ErrorResponseException::SSG_SCHEDULE_TAPE_NOT_LOADED_ERROR)
    s = TestXMLHelper::format("SSG_SCHEDULE_TAPE_NOT_LOADED_ERROR");
  else if (value == ErrorResponseException::SSG_SCHEDULE_DYNAMIC_UPDATE_ERROR)
    s = TestXMLHelper::format("SSG_SCHEDULE_DYNAMIC_UPDATE_ERROR");
  else if (value == ErrorResponseException::SSG_QUERY_CHECKPOINT_FAILED)
    s = TestXMLHelper::format("SSG_QUERY_CHECKPOINT_FAILED");
  else if (value == ErrorResponseException::SSG_UPDATE_CHECKPOINT_FAILED)
    s = TestXMLHelper::format("SSG_UPDATE_CHECKPOINT_FAILED");
  else if (value == ErrorResponseException::BSR_INVALID_DATA)
    s = TestXMLHelper::format("BSR_INVALID_DATA");
  else if (value == ErrorResponseException::BSR_PROCESSING_START)
    s = TestXMLHelper::format("BSR_PROCESSING_START");
  else if (value == ErrorResponseException::BSR_ACTION_CODE)
    s = TestXMLHelper::format("BSR_ACTION_CODE");
  else if (value == ErrorResponseException::BSR_PROCESSING_END)
    s = TestXMLHelper::format("BSR_PROCESSING_END");
  else if (value == ErrorResponseException::BSR_EMU_NATION_IGNORED)
    s = TestXMLHelper::format("BSR_EMU_NATION_IGNORED");
  else if (value == ErrorResponseException::BSR_DB_ERROR)
    s = TestXMLHelper::format("BSR_DB_ERROR");
  else if (value == ErrorResponseException::FARE_CURRENCY_OVERRIDE_NOT_VALID)
    s = TestXMLHelper::format("FARE_CURRENCY_OVERRIDE_NOT_VALID");
  else if (value == ErrorResponseException::LMT_ISSUE_SEP_TKTS_INTL_SURFACE_RESTR)
    s = TestXMLHelper::format("LMT_ISSUE_SEP_TKTS_INTL_SURFACE_RESTR");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::FMDirection& value)
{
  std::string s;

  if (value == tse::FMDirection::UNKNOWN)
    s = TestXMLHelper::format("UNKNOWN");
  else if (value == tse::FMDirection::INBOUND)
    s = TestXMLHelper::format("INBOUND");
  else if (value == tse::FMDirection::OUTBOUND)
    s = TestXMLHelper::format("OUTBOUND");

  return s;
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const Code<15>& loc)
{
  return formatCode(loc);
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const Code<8>& loc)
{
  return formatCode(loc);
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const Code<7>& loc)
{
  return formatCode(loc);
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const Code<6>& loc)
{
  return formatCode(loc);
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const Code<5>& loc)
{
  return formatCode(loc);
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const Code<4>& loc)
{
  return formatCode(loc);
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const Code<3>& loc)
{
  return formatCode(loc);
}

//-------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const Code<2>& loc)
{
  return formatCode(loc);
}

//--------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::DateTime& loc)
{
  return format(loc.dateToIsoString());
}

//--------------------------------------------------------------------------------------------

/* static */
std::string
TestXMLHelper::format(const tse::CabinType& c)
{
  return format(c.getCabinIndicator());
}
/* static */
std::string
TestXMLHelper::format(const tse::FareTypeDesignator& ftd)
{
  return format(ftd.fareTypeDesig());
}
