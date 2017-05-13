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
#pragma once
#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Codes.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/SafeEnums.h"

namespace tax { namespace type {

  class Timestamp;

}} // namespace tax::type

namespace tse
{

tse::NationCode toTseNationCode(tax::type::Nation taxNationCode);
tse::TaxCode toTseTaxCode(tax::type::TaxCode taxTaxCode);
tse::TaxType toTseTaxType(tax::type::TaxType taxTaxType);
tse::CarrierCode toTseCarrierCode(tax::type::CarrierCode taxCarrierCode);
tse::CurrencyCode toTseCurrencyCode(tax::type::CurrencyCode taxCurrencyCode);
tse::VendorCode toTseVendorCode(tax::type::Vendor taxVendorCode);
tse::LocCode toTseLocZoneCode(tax::type::LocZoneText taxLocCode);
tse::LocCode toTseAirportCode(tax::type::AirportCode taxPortCode);
tse::LocCode toTseAirportOrCityCode(tax::type::AirportOrCityCode taxPortCode);
tse::PaxTypeCode toTsePassengerCode(tax::type::PassengerCode taxPaxCode);
tse::PseudoCityCode toTsePseudoCityCode(tax::type::PseudoCityCode code);

tax::type::Nation toTaxNationCode(const tse::NationCode& tseNationCode);
tax::type::TaxCode toTaxCode(const tse::TaxCode& tseTaxCode);
tax::type::TaxType toTaxType(const tse::TaxType& tseTaxType);
tax::type::CarrierCode toTaxCarrierCode(const tse::CarrierCode& tseCarrierCode);
tax::type::CurrencyCode toTaxCurrencyCode(const tse::CurrencyCode& tseCurrencyCode);
tax::type::Vendor toTaxVendorCode(const tse::VendorCode& tseVendorCode);
tax::type::LocZoneText toTaxLocZoneCode(const tse::LocCode& tseLocZoneCode);
tax::type::CityCode toTaxCityCode(const tse::LocCode& cityCode);
tax::type::AirportCode toTaxAirportCode(const tse::LocCode& portCode);
tax::type::PseudoCityCode toTaxPseudoCityCode(const tse::PseudoCityCode& pcc);
tax::type::PassengerCode toTaxPassengerCode(const tse::PaxTypeCode& paxTypeCode);
tax::type::OcSubCode toTaxOcSubCode(const tse::ServiceSubTypeCode& ocSubCode);
tax::type::FareTariffInd toTaxFareTariff(const tse::FareTariffInd& fareTariffCode);
tax::type::TaxMatchingApplTag toTaxMatchApplTag(const ::Code<2>& tag);
tax::type::TaxProcessingApplTag toTaxApplicationTag(const ::Code<2>& tag);
tax::type::StopoverTimeTag toTaxStopoverTimeTag(const ::Code<3>& tag);

tax::type::Directionality toTaxDirectionality(const tse::Directionality dir);

tax::type::Timestamp toTimestamp(const tse::DateTime& date);
DateTime toDateTime(const tax::type::Timestamp& date);

unsigned char signedToUnsigned(char value);
template <typename T> unsigned char signedToUnsigned(T) = delete;

template <typename ET, typename UT>
void setTaxEnumValue(ET& enum_, UT value)
{
  enum_ = static_cast<ET>(value);
}

template <typename ET>
void setTaxEnumValue(ET& enum_, char value)
{
  unsigned char ch = signedToUnsigned(value);
  setTaxEnumValue(enum_, ch);
}

} // namespace tse

