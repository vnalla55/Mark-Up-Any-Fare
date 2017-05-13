#pragma once

#include "Xform/AncillarySchemaNames.h"

namespace tse
{
namespace ticketingfees
{
enum _TicketingFeesElementIdx_
{ _TicketingFeesRequest = ancillary::_NumberOfElementNames_,
  _BIN, // Form of payment, BIN #
  _DIF, // Differential Dtata
  _NumberOfElementNamesTktFees_ };

enum _TicketingFeesAttributeIdx_
{ _STA = ancillary::_NumberOfAttributeNames_, // Total Amount per Passenger
  _FOP, // BIN# Form of Payment
  _FP2,
  _BJ0, // Diff High Fare FareClass
  _NumberOfAttributeNamesTktFees_ };

extern const char* _TicketingFeesElementNames[];
extern const char* _TicketingFeesAttributeNames[];
}
}
