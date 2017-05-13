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

#include "Common/Tariff.h"
#include "DataModel/Common/Types.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{

struct SectorDetailEntry
{
  static const int16_t RESERVATION_CODES_SIZE = 3;

  type::SectorDetailAppl applTag {type::SectorDetailAppl::Blank};
  type::EquipmentCode equipmentCode {};
  type::CarrierCode fareOwnerCarrier {UninitializedCode};
  type::CabinCode cabinCode {type::CabinCode::Blank};
  type::FareTypeCode fareType {};
  type::FareRuleCode rule {};
  std::vector<type::ReservationDesignatorCode> reservationCodes {RESERVATION_CODES_SIZE, " "};
  Tariff tariff {};
  type::FareBasisCode fareBasisTktDesignator {};
  type::TicketCode ticketCode {};
  int seqNo{0};
};

struct SectorDetail
{
  typedef SectorDetailEntry entry_type;
  type::Vendor vendor;
  type::Index itemNo;
  boost::ptr_vector<entry_type> entries;
};

} // namespace tax

