//-----------------------------------------------------------------------------
//
//  File:     Diag430Collector.cpp
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2004
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "Diagnostic/Diag430Collector.h"

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function  Diag430Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//          BookingCode Exception Data Record Diagnostic Display.
//
// @param  BookingCodeException - Specific  BookingCode Exception Data Information
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag430Collector&
Diag430Collector::operator << (const BookingCodeExceptionSequence& bceSequence)
{
  if (!_active)
  {
    return *this;
  }

  DiagCollector& dc = (DiagCollector&)*this;

  dc << "\n \nSEQUENCE NUMBER   : " << bceSequence.seqNo() << "\n"
     << "   IF TAG          : " << bceSequence.ifTag()
     << "   SEGMENT COUNT   : " << bceSequence.segCnt() << "\n";
  return *this;
}
}
