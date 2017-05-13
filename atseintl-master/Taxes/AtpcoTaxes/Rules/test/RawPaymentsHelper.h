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
#include "DataModel/Common/Types.h"
#include "Rules/RawPayments.h"

#include <boost/core/noncopyable.hpp>

namespace tax
{

class GeoPath;
class PaymentDetail;
class TaxName;

class RawPaymentsHelper : boost::noncopyable
{
  GeoPath* _geoPath;

public:
  explicit RawPaymentsHelper() : _geoPath(0) {}
  void setGeoPath(GeoPath& gp) { _geoPath = &gp; }

  PaymentDetail&
  emplace(RawPayments& rawPayments, type::Index id, TaxName& taxName,
          type::TicketedPointTag ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly);

  PaymentDetail&
  emplace(RawPayments& rawPayments, type::Index idB, type::Index idE, TaxName& taxName,
          type::TicketedPointTag ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly);
};

} // namespace tax

