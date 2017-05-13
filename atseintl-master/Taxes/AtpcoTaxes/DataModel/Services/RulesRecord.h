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

#include <set>

#include "Common/Consts.h"
#include "Common/LocZone.h"
#include "Common/Timestamp.h"
#include "Common/TaxableUnitTagSet.h"
#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"

namespace tax
{

struct RulesRecord
{
  static const int16_t CONNECTIONS_TAGS_SIZE = 8;
  static const int16_t TAXABLEUNIT_TAGS_SIZE = 10;

  TaxName taxName;
  type::Vendor vendor;
  type::SeqNo seqNo{0};
  type::TicketedPointTag ticketedPointTag{type::TicketedPointTag::MatchTicketedPointsOnly};
  type::ExemptTag exemptTag{type::ExemptTag::Blank};
  type::MoneyAmount taxAmt{0};
  type::CurrencyCode taxCurrency{"USD"};
  type::CurDecimals taxCurDecimals{0};
  type::Percent taxPercent{0};
  type::Date createDate{2001, 1, 1};
  type::Date effDate{2001, 1, 1};
  type::Date discDate{type::Date::pos_infinity()};
  type::Timestamp expiredDate{type::Date::pos_infinity(), type::Time::blank_time()};
  int16_t firstTravelYear{99};
  int16_t firstTravelMonth{99};
  int16_t firstTravelDay{99};
  type::Date firstTravelDate{type::Date::blank_date()};
  int16_t lastTravelYear{99};
  int16_t lastTravelMonth{99};
  int16_t lastTravelDay{99};
  type::Date lastTravelDate{type::Date::blank_date()};
  type::Date histSaleEffDate{type::Date::blank_date()};
  type::Date histSaleDiscDate{type::Date::blank_date()};
  type::Date histTrvlEffDate{type::Date::blank_date()};
  type::Date histTrvlDiscDate{type::Date::blank_date()};
  type::TravelDateAppTag travelDateTag{type::TravelDateAppTag::Blank};
  type::RtnToOrig rtnToOrig{type::RtnToOrig::Blank};
  type::JrnyInd jrnyInd{type::JrnyInd::JnyLoc2DestPointForOWOrTurnAroundForRTOrOJ};
  LocZone jrnyLocZone1;
  LocZone jrnyLocZone2;
  LocZone trvlWhollyWithin;
  LocZone jrnyIncludes;
  LocZone taxPointLocZone1;
  LocZone taxPointLocZone2;
  LocZone taxPointLocZone3;
  type::TransferTypeTag taxPointLoc1TransferType{type::TransferTypeTag::Blank};
  type::StopoverTag taxPointLoc1StopoverTag{type::StopoverTag::Blank};
  type::Loc2StopoverTag taxPointLoc2StopoverTag{type::Loc2StopoverTag::Blank};
  type::AdjacentIntlDomInd taxPointLoc1IntlDomInd{type::AdjacentIntlDomInd::Blank};
  type::IntlDomInd taxPointLoc2IntlDomInd{type::IntlDomInd::Blank};
  type::StopoverTimeUnit stopoverTimeUnit{type::StopoverTimeUnit::Blank};
  type::StopoverTimeTag stopoverTimeTag;
  type::TaxPointLoc2Compare taxPointLoc2Compare{type::TaxPointLoc2Compare::Blank};
  type::TaxPointLoc3GeoType taxPointLoc3GeoType{type::TaxPointLoc3GeoType::Blank};
  std::set<type::ConnectionsTag> connectionsTags;
  type::CurrencyCode currencyOfSale{UninitializedCode};
  LocZone pointOfTicketing;
  type::TaxApplicationLimit taxApplicationLimit{type::TaxApplicationLimit::Unlimited};
  type::Index carrierFlightItemBefore{0};
  type::Index carrierFlightItemAfter{0};
  type::Index carrierApplicationItem{0};
  TaxableUnitTagSet applicableTaxableUnits{TaxableUnitTagSet::none()};
  uint32_t minTax{0};
  uint32_t maxTax{0};
  type::CurrencyCode minMaxCurrency{UninitializedCode};
  type::CurDecimals minMaxDecimals{0};
  type::TktValApplQualifier tktValApplQualifier{type::TktValApplQualifier::Blank};
  type::CurrencyCode tktValCurrency{UninitializedCode};
  uint32_t tktValMin{0};
  uint32_t tktValMax{0};
  type::CurDecimals tktValCurrDecimals{0};
  type::Index serviceBaggageItemNo{0};
  type::Index passengerTypeCodeItem{0};
  type::NetRemitApplTag netRemitApplTag{type::NetRemitApplTag::Blank};
  type::Index sectorDetailItemNo{0};
  int16_t alternateRuleRefTag{0};
  type::SectorDetailApplTag sectorDetailApplTag{type::SectorDetailApplTag::Blank};
  type::TaxMatchingApplTag taxMatchingApplTag{UninitializedCode};
  type::TaxAppliesToTagInd taxAppliesToTagInd{type::TaxAppliesToTagInd::Blank};
  type::ServiceBaggageApplTag serviceBaggageApplTag{type::ServiceBaggageApplTag::Blank};
  int16_t calcOrder{1};
  type::PaidBy3rdPartyTag paidBy3rdPartyTag{type::PaidBy3rdPartyTag::Blank};
  type::TaxRoundingUnit taxRoundUnit{type::TaxRoundingUnit::Blank};
  type::TaxRoundingDir taxRoundDir{type::TaxRoundingDir::Blank};
  LocZone pointOfDelivery;
  type::TaxProcessingApplTag taxProcessingApplTag{UninitializedCode};
  type::Index svcFeesSecurityItemNo{0};
  LocZone pointOfSale;
  type::OutputTypeIndicator outputTypeIndicator{type::OutputTypeIndicator::BothTTBSAndRATD};
  type::VatInclusiveInd vatInclusiveInd{type::VatInclusiveInd::Blank};
};

} // namespace tax
