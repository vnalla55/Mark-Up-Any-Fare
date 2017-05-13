// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/Timestamp.h"
#include "DataModel/Common/Codes.h"
#include "DataModel/Common/Types.h"
#include "TaxDisplay/Common/Types.h"

#include <boost/optional.hpp>

#include <vector>

namespace tax
{
namespace display
{

struct TaxDisplayRequest
{
  enum class EntryType
  {
    ENTRY_HELP,
    ENTRY_HELP_CALCULATION_BREAKDOWN,
    REPORTINGRECORD_ENTRY_BY_NATION,
    RULESRECORD_ENTRY_BY_NATION,
    REPORTINGRECORD_ENTRY_BY_TAX,
    RULESRECORD_ENTRY_BY_TAX,
    UNSET_ENTRY
  };

  enum class UserType
  {
    SABRE,
    AS,
    TN,
    UNSET_USER
  };

  bool isUserSabre() const { return userType == UserType::SABRE; }
  bool isUserTN() const { return userType == UserType::TN; }
  bool hasSetAnyCarrierCode() const { return !carrierCode1.empty(); } // no carrier codes should be set if first is empty
  bool hasCarrierCode(const type::CarrierCode& inputCarrier) const
  {
    return carrierCode1 == inputCarrier || carrierCode2 == inputCarrier || carrierCode3 == inputCarrier;
  }

  EntryType entryType{EntryType::UNSET_ENTRY};
  UserType userType{UserType::UNSET_USER};
  DetailLevels detailLevels;

  type::AirportCode airportCode;
  type::CarrierCode carrierCode1;
  type::CarrierCode carrierCode2;
  type::CarrierCode carrierCode3;
  type::Nation nationCode;
  type::NationName nationName;
  bool isReissued{false};
  type::SeqNo seqNo{0};
  type::TaxCode taxCode;
  type::TaxType taxType;
  type::Timestamp requestDate;
  type::Timestamp travelDate;
  std::vector<bool> x1categories;
  std::vector<bool> x2categories;
  bool showCategoryForEachTax{true};
};

} // namespace display
} // namespace tax

