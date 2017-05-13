//----------------------------------------------------------------------------
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class InterlineTicketCarrierInfo;

class Diag851Collector : public DiagCollector
{
public:
  Diag851Collector() : _counter(0) {}

  void printIETHeader(const CarrierCode& validatingCarrier);
  void printIETLine(InterlineTicketCarrierInfo* iet);
  void printAllCarriers(const std::vector<InterlineTicketCarrierInfo*>& interlineInfoVec);
  void printItinCrxLine(const std::vector<CarrierCode>& cxrVec);

private:
  static const char* _SEPARATOR;
  uint16_t _counter;
};

} // namespace tse

