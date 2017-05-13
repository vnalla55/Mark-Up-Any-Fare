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

#include "Common/DateTime.h"
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/AtpcoTaxes/Common/Timestamp.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include <boost/format.hpp>

namespace tse
{

namespace
{

Logger logger("atseintl.Taxes.LegacyFacades");

std::string extractRange(int lo, int hi)
{
  if (lo == hi)
    return str(boost::format("%1%") % lo);
  else
    return str(boost::format("%1% to %2%") % lo % hi);
}

template <typename tag, int L, int H>
std::string extractRange(tax::Code<tag, L, H>)
{
  return extractRange(L, H); // to prevent code bloat
}


template <typename TaxCode, typename TseCode, int N>
void convert(TaxCode& taxCode, const TseCode& tseCode, const char (&codeName)[N])
{
  if (tseCode.empty()) return; // empty to empty

  bool ans = codeFromString(tseCode.c_str(), taxCode);
  if (BOOST_UNLIKELY(!ans))
  {
    static const char* const message = "Expected %1% characters for %2%. Got \"%3%\".";
    LOG4CXX_ERROR(logger, boost::format(message) % extractRange(taxCode) % codeName % tseCode);
  }
}

} // anonymous namespace

tse::NationCode toTseNationCode(tax::type::Nation taxNationCode)
{
  return tse::NationCode(taxNationCode.asString());
}

tse::TaxCode toTseTaxCode(tax::type::TaxCode taxTaxCode)
{
  return tse::TaxCode(taxTaxCode.rawArray(), taxTaxCode.length());
}

tse::TaxType toTseTaxType(tax::type::TaxType taxTaxType)
{
  return tse::TaxType(taxTaxType.rawArray(), taxTaxType.length());
}

tse::CarrierCode toTseCarrierCode(tax::type::CarrierCode taxCarrierCode)
{
  return tse::CarrierCode(taxCarrierCode.rawArray(), taxCarrierCode.length());
}

tse::CurrencyCode toTseCurrencyCode(tax::type::CurrencyCode taxCurrencyCode)
{
  return tse::CurrencyCode(taxCurrencyCode.rawArray(), taxCurrencyCode.length());
}

tse::VendorCode toTseVendorCode(tax::type::Vendor taxVendorCode)
{
  return tse::VendorCode(taxVendorCode.rawArray(), taxVendorCode.length());
}

tse::LocCode toTseLocZoneCode(tax::type::LocZoneText taxLocCode)
{
  return tse::LocCode(taxLocCode.rawArray(), taxLocCode.length());
}

tse::LocCode toTseAirportCode(tax::type::AirportCode taxPortCode)
{
  return tse::LocCode(taxPortCode.rawArray(), taxPortCode.length());
}

tse::LocCode toTseAirportOrCityCode(tax::type::AirportOrCityCode taxPortCode)
{
  return tse::LocCode(taxPortCode.rawArray(), taxPortCode.length());
}

tse::PaxTypeCode toTsePassengerCode(tax::type::PassengerCode taxPaxCode)
{
  return tse::PaxTypeCode(taxPaxCode.rawArray(), taxPaxCode.length());
}

tse::PseudoCityCode toTsePseudoCityCode(tax::type::PseudoCityCode code)
{
  return tse::PseudoCityCode(code.rawArray(), code.length());
}

tax::type::Nation toTaxNationCode(const tse::NationCode& tseNationCode)
{
  tax::type::Nation ans;
  convert(ans, tseNationCode, "Nation Code");
  return ans;
}

tax::type::TaxCode toTaxCode(const tse::TaxCode& tseTaxCode)
{
  tax::type::TaxCode ans;
  convert(ans, tseTaxCode, "Tax Code");
  return ans;
}

tax::type::TaxType toTaxType(const tse::TaxType& tseTaxType)
{
  tax::type::TaxType ans;
  convert(ans, tseTaxType, "Tax Type");
  return ans;
}

tax::type::CarrierCode toTaxCarrierCode(const tse::CarrierCode& tseCarrierCode)
{
  tax::type::CarrierCode ans;
  convert(ans, tseCarrierCode, "Carrier Code");
  return ans;
}

tax::type::CurrencyCode toTaxCurrencyCode(const tse::CurrencyCode& tseCurrencyCode)
{
  tax::type::CurrencyCode ans;
  convert(ans, tseCurrencyCode, "Currency Code");
  return ans;
}

tax::type::Vendor toTaxVendorCode(const tse::VendorCode& tseVendorCode)
{
  tax::type::Vendor ans;
  convert(ans, tseVendorCode, "Vendor Code");
  return ans;
}

tax::type::LocZoneText toTaxLocZoneCode(const tse::LocCode& tseLocZoneCode)
{
  tax::type::LocZoneText ans;
  convert(ans, tseLocZoneCode, "Location/Zone Code");
  return ans;
}

tax::type::CityCode toTaxCityCode(const tse::LocCode& cityCode)
{
  tax::type::CityCode ans;
  convert(ans, cityCode, "City Code");
  return ans;
}

tax::type::AirportCode toTaxAirportCode(const tse::LocCode& portCode)
{
  tax::type::AirportCode ans;
  convert(ans, portCode, "Airport Code");
  return ans;
}

tax::type::PseudoCityCode toTaxPseudoCityCode(const tse::PseudoCityCode& pcc)
{
  tax::type::PseudoCityCode ans;
  convert(ans, pcc, "Pseudo City Code");
  return ans;
}

tax::type::PassengerCode toTaxPassengerCode(const tse::PaxTypeCode& paxTypeCode)
{
  tax::type::PassengerCode ans;
  convert(ans, paxTypeCode, "Passenger Type Code");
  return ans;
}

tax::type::OcSubCode toTaxOcSubCode(const tse::ServiceSubTypeCode& ocSubCode)
{
  tax::type::OcSubCode ans;
  convert(ans, ocSubCode, "OC Sub-Code");
  return ans;
}

tax::type::FareTariffInd toTaxFareTariff(const tse::FareTariffInd& fareTariffCode)
{
  tax::type::FareTariffInd ans;
  convert(ans, fareTariffCode, "Fare Tariff Indicator");
  return ans;
}

tax::type::TaxMatchingApplTag toTaxMatchApplTag(const ::Code<2>& tag)
{
  tax::type::TaxMatchingApplTag ans;
  convert(ans, tag, "Tax Matching Appl Tag");
  return ans;
}

tax::type::TaxProcessingApplTag toTaxApplicationTag(const ::Code<2>& tag)
{
  tax::type::TaxProcessingApplTag ans;
  convert(ans, tag, "Tax Application Tag");
  return ans;
}

tax::type::StopoverTimeTag toTaxStopoverTimeTag(const ::Code<3>& tag)
{
  tax::type::StopoverTimeTag ans;
  convert(ans, tag, "Tax Stopover Time Tag");
  return ans;
}

tax::type::Directionality toTaxDirectionality(const tse::Directionality dir)
{
  switch (dir)
  {
  case FROM:
    return tax::type::Directionality::From;
  case TO:
    return tax::type::Directionality::To;
  case BETWEEN:
    return tax::type::Directionality::Between;
  case WITHIN:
    return tax::type::Directionality::Within;
  case BOTH:
    return tax::type::Directionality::Both;
  case ORIGIN:
    return tax::type::Directionality::Origin;
  case TERMINATE:
    return tax::type::Directionality::Terminate;
  default:
    return tax::type::Directionality::Blank;
  }
}

tax::type::Timestamp toTimestamp(const tse::DateTime& date)
{
  tax::type::Date d (date.year(), date.month(), date.day());
  tax::type::Time t (date.hours(), date.minutes());
  return {d, t};
}

DateTime toDateTime(const tax::type::Timestamp& date)
{
  return DateTime(date.year(),
                  date.month(),
                  date.day(),
                  date.hour(),
                  date.min());
}

unsigned char signedToUnsigned(char value)
{
  if (BOOST_UNLIKELY(int(value) < 0 || int(value) >= 127))
  {
    LOG4CXX_ERROR(logger, "Got an invalid tag value \"" << value << "\".");
    return static_cast<unsigned char>(' ');
  }
  return static_cast<unsigned char>(value);
}

} // namespace tse

