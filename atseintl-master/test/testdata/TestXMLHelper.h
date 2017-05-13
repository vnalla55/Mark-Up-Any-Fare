#ifndef TEST_XMLHELPER_FACTORY_H
#define TEST_XMLHELPER_FACTORY_H

/**
 * This is a helper to read attributes from XML sections.
 * The reason we do this is to handle default values for items.
 **/

#include "Common/TseBoostStringTypes.h"
#include "Common/TseEnums.h"
#include "Common/DateTime.h"
#include "test/testdata/tinyxml/tinyxml.h"

#include "DataModel/DifferentialData.h"
#include "Common/ErrorResponseException.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingUnit.h"
#include "Common/SmallBitSet.h"
#include "Common/CabinType.h"
#include "Common/FareTypeDesignator.h"

class TestXMLHelper
{
public:
  // For those classes whose accessor returns a const, we take the const reference
  // (and, given it's C++, we conveniently cast away the constness).

  static void Attribute(TiXmlElement* element, const char* name, const std::string& str);
  static void Attribute(TiXmlElement* element, const char* name, const BoostString& str);
  static void Attribute(TiXmlElement* element, const char* name, const int8_t& value);
  static void Attribute(TiXmlElement* element, const char* name, const int16_t& value);
  static void Attribute(TiXmlElement* element, const char* name, const int32_t& value);
  static void Attribute(TiXmlElement* element, const char* name, const int64_t& value);
  static void Attribute(TiXmlElement* element, const char* name, const uint8_t& value);
  static void Attribute(TiXmlElement* element, const char* name, const uint16_t& value);
  static void Attribute(TiXmlElement* element, const char* name, const uint32_t& value);
  static void Attribute(TiXmlElement* element, const char* name, const uint64_t& value);
  static void Attribute(TiXmlElement* element, const char* name, const double& value);
  static void Attribute(TiXmlElement* element, const char* name, const char& value);
  static void Attribute(TiXmlElement* element, const char* name, const bool& value);
  static void Attribute(TiXmlElement* element, const char* name, const tse::GlobalDirection& value);
  static void Attribute(TiXmlElement* element, const char* name, const tse::LocType& value);
  static void Attribute(TiXmlElement* element,
                        const char* name,
                        const tse::DifferentialData::STATUS_TYPE& value);
  static void Attribute(TiXmlElement* element, const char* name, const tse::GeoTravelType& value);
  static void
  Attribute(TiXmlElement* element, const char* name, const tse::FCASegDirectionality& value);
  static void Attribute(TiXmlElement* element, const char* name, const tse::Directionality& value);
  static void
  Attribute(TiXmlElement* element, const char* name, const tse::PricingUnit::Type& value);
  static void
  Attribute(TiXmlElement* element, const char* name, const tse::PricingUnit::PUSubType& value);
  static void
  Attribute(TiXmlElement* element, const char* name, const tse::PricingUnit::PUFareType& value);
  static void Attribute(TiXmlElement* element, const char* name, const tse::RecordScope& value);
  static void Attribute(TiXmlElement* element,
                        const char* name,
                        const tse::SmallBitSet<uint8_t, tse::FMTravelBoundary>& value);
  static void Attribute(TiXmlElement* element,
                        const char* name,
                        const tse::ErrorResponseException::ErrorResponseCode& value);
  static void
  Attribute(TiXmlElement* element, const char* name, const tse::FMDirection& value);
  static void Attribute(TiXmlElement* element, const char* name, const Code<15>& value);
  static void Attribute(TiXmlElement* element, const char* name, const Code<8>& value);
  static void Attribute(TiXmlElement* element, const char* name, const Code<7>& value);
  static void Attribute(TiXmlElement* element, const char* name, const Code<6>& value);
  static void Attribute(TiXmlElement* element, const char* name, const Code<5>& value);
  static void Attribute(TiXmlElement* element, const char* name, const Code<4>& value);
  static void Attribute(TiXmlElement* element, const char* name, const Code<3>& value);
  static void Attribute(TiXmlElement* element, const char* name, const Code<2>& value);
  static void Attribute(TiXmlElement* element, const char* name, const tse::DateTime& value);

  // The attribute setting methods where all the work takes place.

  static void Attribute(TiXmlElement* element, const char* name, std::string& str);
  static void Attribute(TiXmlElement* element, const char* name, BoostString& str);
  static void Attribute(TiXmlElement* element, const char* name, int8_t& value);
  static void Attribute(TiXmlElement* element, const char* name, int16_t& value);
  static void Attribute(TiXmlElement* element, const char* name, int32_t& value);
  static void Attribute(TiXmlElement* element, const char* name, int64_t& value);
  static void Attribute(TiXmlElement* element, const char* name, uint8_t& value);
  static void Attribute(TiXmlElement* element, const char* name, uint16_t& value);
  static void Attribute(TiXmlElement* element, const char* name, uint32_t& value);
  static void Attribute(TiXmlElement* element, const char* name, uint64_t& value);
  static void Attribute(TiXmlElement* element, const char* name, double& value);
  static void Attribute(TiXmlElement* element, const char* name, char& value);
  static void Attribute(TiXmlElement* element, const char* name, bool& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::GlobalDirection& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::LocType& value);
  static void
  Attribute(TiXmlElement* element, const char* name, tse::DifferentialData::STATUS_TYPE& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::GeoTravelType& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::FCASegDirectionality& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::Directionality& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::PricingUnit::Type& value);
  static void
  Attribute(TiXmlElement* element, const char* name, tse::PricingUnit::PUSubType& value);
  static void
  Attribute(TiXmlElement* element, const char* name, tse::PricingUnit::PUFareType& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::RecordScope& value);
  static void Attribute(TiXmlElement* element,
                        const char* name,
                        tse::SmallBitSet<uint8_t, tse::FMTravelBoundary>& value);
  static void Attribute(TiXmlElement* element,
                        const char* name,
                        tse::ErrorResponseException::ErrorResponseCode& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::FMDirection& value);
  static void Attribute(TiXmlElement* element, const char* name, Code<15>& value);
  static void Attribute(TiXmlElement* element, const char* name, Code<8>& value);
  static void Attribute(TiXmlElement* element, const char* name, Code<7>& value);
  static void Attribute(TiXmlElement* element, const char* name, Code<6>& value);
  static void Attribute(TiXmlElement* element, const char* name, Code<5>& value);
  static void Attribute(TiXmlElement* element, const char* name, Code<4>& value);
  static void Attribute(TiXmlElement* element, const char* name, Code<3>& value);
  static void Attribute(TiXmlElement* element, const char* name, Code<2>& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::DateTime& value);

  static void Attribute(TiXmlElement* element, const char* name, tse::CabinType& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::FareTypeDesignator& value);
  static void Attribute(TiXmlElement* element, const char* name, tse::RoundingRule& value);

  // Format the data for output.

  static std::string format(const std::string& str);
  static std::string format(const BoostString& str);
  static std::string format(const int8_t& value);
  static std::string format(const int16_t& value);
  static std::string format(const int32_t& value);
  static std::string format(const int64_t& value);
  static std::string format(const uint8_t& value);
  static std::string format(const uint16_t& value);
  static std::string format(const uint32_t& value);
  static std::string format(const uint64_t& value);
  static std::string format(const double& value);
  static std::string format(const char& value);
  static std::string format(const char* value);
  static std::string format(const bool& value);
  static std::string format(const tse::GlobalDirection& value);
  static std::string format(const tse::LocType& value);
  static std::string format(const tse::DifferentialData::STATUS_TYPE& value);
  static std::string format(const tse::GeoTravelType& value);
  static std::string format(const tse::FCASegDirectionality& value);
  static std::string format(const tse::Directionality& value);
  static std::string format(tse::PricingUnit::Type value);
  static std::string format(const tse::PricingUnit::PUSubType& value);
  static std::string format(const tse::PricingUnit::PUFareType& value);
  static std::string format(const tse::RecordScope& value);
  static std::string
  format(const tse::SmallBitSet<uint8_t, tse::FMTravelBoundary>& value);
  static std::string format(const tse::ErrorResponseException::ErrorResponseCode& value);
  static std::string format(const tse::FMDirection& value);
  static std::string format(const Code<15>& value);
  static std::string format(const Code<8>& value);
  static std::string format(const Code<7>& value);
  static std::string format(const Code<6>& value);
  static std::string format(const Code<5>& value);
  static std::string format(const Code<4>& value);
  static std::string format(const Code<3>& value);
  static std::string format(const Code<2>& value);

  static std::string format(const tse::DateTime& value);
  static std::string format(const tse::CabinType& value);
  static std::string format(const tse::FareTypeDesignator& value);

  static bool AttributeExists(TiXmlElement* element, const char* name)
  {
    return element->Attribute(name) != NULL;
  }

private:
  TestXMLHelper();

  template <typename CodeType>
  static void AttributeCode(TiXmlElement*, const char*, CodeType&);

  template <typename CodeType>
  static std::string formatCode(const CodeType&);
};

template <typename CodeType>
void
TestXMLHelper::AttributeCode(TiXmlElement* element, const char* name, CodeType& loc)
{
  const char* value = element->Attribute(name);
  if (value != NULL)
  {
    loc = value;
  }
  else
  {
    loc = "";
  }
}

template <typename CodeType>
std::string
TestXMLHelper::formatCode(const CodeType& loc)
{
  std::string s("\"");
  s = s + loc;
  s = s + "\"";

  return s;
}

#endif
