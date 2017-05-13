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

#include "DataModel/Common/SafeEnums.h"

#include <ostream>
#include <string>
#include <type_traits>

namespace tax
{
std::string
safeEnumToString(const type::TaxPointTag& tpt);
std::string
safeEnumToString(const type::OpenSegmentIndicator& osi);
std::string
safeEnumToString(const type::StopoverTimeUnit& unit);
std::string
safeEnumToString(const type::TaxOrChargeTag& tag);
std::string
safeEnumToString(const type::RefundableTag& tag);
std::string
safeEnumToString(const type::AccountableDocTag& tag);
std::string
safeEnumToString(const type::PercentFlatTag& tag);
std::string
safeEnumToString(const type::TaxRemittanceId& tag);
std::string
safeEnumToString(const type::TravelDateAppTag& tag);
std::string
safeEnumToString(const type::VatInclusiveInd& tag);
std::string
safeEnumToString(const type::LocType& tag);
std::string
safeEnumToString(const type::TktValApplQualifier& tag);
std::string
safeEnumToString(const type::PaidBy3rdPartyTag& tag);
std::string
safeEnumToString(const type::TaxAppliesToTagInd& tag);
std::string
safeEnumToString(const type::NetRemitApplTag& tag);
std::string
safeEnumToString(const type::TicketedPointTag& tag);
std::string
safeEnumToString(const type::RtnToOrig& tag);
std::string
safeEnumToString(const type::JrnyInd& tag);
std::string
safeEnumToString(const type::AdjacentIntlDomInd& tag);
std::string
safeEnumToString(const type::TransferTypeTag& tag);
std::string
safeEnumToString(const type::StopoverTag& tag);
std::string
safeEnumToString(const type::IntlDomInd& tag);
std::string
safeEnumToString(const type::TaxPointLoc2Compare& tag);
std::string
safeEnumToString(const type::Loc2StopoverTag& tag);
std::string
safeEnumToString(const type::TaxPointLoc3GeoType& tag);
std::string
safeEnumToString(const type::PassengerStatusTag& tag);
std::string
safeEnumToString(const type::PtcMatchIndicator& tag);
std::string
safeEnumToString(const type::CodeType& tag);
std::string
safeEnumToString(const type::ViewBookTktInd& tag);
std::string
safeEnumToString(const type::ServiceBaggageApplTag& tag);
std::string
safeEnumToString(const type::ServiceBaggageAppl& tag);
std::string
safeEnumToString(const type::OptionalServiceTag& tag);
std::string
safeEnumToString(const type::SectorDetailApplTag& tag);
std::string
safeEnumToString(const type::CabinCode& tag);
std::string
safeEnumToString(const type::SectorDetailAppl& tag);
std::string
safeEnumToString(const type::ConnectionsTag& tag);
std::string
safeEnumToString(const type::ReissueLocTypeTag& tag);
std::string
safeEnumToString(const type::ReissueRefundableTag& tag);

namespace type {

template <typename T>
typename std::enable_if<std::is_enum<T>::value && sizeof(T) == 1, std::ostream&>::type
operator<<(std::ostream& out, const T& val)
{
  return out << static_cast<unsigned char>(val);
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value && sizeof(T) == sizeof(int), std::ostream&>::type
operator<<(std::ostream& out, const T& val)
{
  return out << static_cast<int>(val);
}

}

}

