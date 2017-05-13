#pragma once

#include "Xform/TicketingFeesSchemaNames.h"

namespace tse
{
namespace ticketingfeeswpa
{
enum _TicketingFeesWpaElementIdx_
{ _REQ = ticketingfees::_NumberOfElementNamesTktFees_,
  _ABL,
  _ERD,
  _SEG,
  _CAL,
  _OBF,
  _DCX,
  _SUM,
  _NumberOfElementNamesTktFeesWpa_ };

enum _TicketingFeesWpaAttributeIdx_
{ _S89 = ticketingfees::_NumberOfAttributeNamesTktFees_,
  _A11,
  _B08,
  _Q0Z,
  _P2N,
  _Q4J,
  _Q46,
  _PAT,
  _PRS,
  _C43,
  _C5E,
  _C5F,
  _C44,
  _C5C,
  _Q0X,
  _C63,
  _C46,
  _C66,
  _S66,
  _C5D,
  _C5B,
  _N09,
  _C64,
  _P27,
  _P2C,
  _P2B,
  _P2A,
  _Q0W,
  _S02,
  _C62,
  _N0C,
  _N0B,
  _C61,
  _C60,
  _N0A,
  _QV0,
  _P26,
  _P28,
  _P29,
  _P2E,
  _P2D,
  _S84,
  _Q0P,
  _S85,
  _PBS,
  _C71,
  _C70,
  _C72,
  _C73,
  _C56,
  _C6L,
  _S86,
  _VCL,
  _C5A,
  _C65,
  _S83,
  _D00,
  _D54,
  _B03,
  _Q4P,
  _SMX,
  _P6X,
  _OFS,
  _C6I,
  _A12,
  _D71,
  _P2M,
  _FCC,
  _NumberOfAttributeNamesTktFeesWpa_ };

extern const char* _TicketingFeesWpaElementNames[];
extern const char* _TicketingFeesWpaAttributeNames[];
}
}
