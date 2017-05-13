//----------------------------------------------------------------------------
//  File:        Diag851Collector.C
//
//  Description: Diagnostic 851 formatter
//
//  Updates:
//          02/28/04 - DVD - Intitial Development
//
//  Copyright Sabre 2004
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
#include "Diagnostic/Diag851Collector.h"

#include "DBAccess/InterlineTicketCarrierInfo.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "FareCalc/IETValidator.h"


#include <iomanip>

namespace tse
{
const char* Diag851Collector::_SEPARATOR =
    "---------------------------------------------------------------\n";

void
Diag851Collector::printIETHeader(const CarrierCode& validatingCarrier)
{
  (*this) << " \n   INTERLINE TICKET AGREEMENT VALIDATION \n"
          << "          VALIDATING CARRIER - " << validatingCarrier << " \n" << _SEPARATOR;
}

void
Diag851Collector::printIETLine(InterlineTicketCarrierInfo* iet)
{

  Diag851Collector& dc = *this;

  if (_counter > 7)
  {
    _counter = 0;
    dc << "\n ";
  }
  dc << " " << iet->interlineCarrier() << "-";
  if (iet->hostInterline() == 'Y')
  {
    dc << "H  ";
  }
  else if (iet->pseudoInterline() == 'Y')
  {
    dc << "P  ";
  }
  else if (iet->superPseudoInterline() == 'Y')
  {
    dc << "S  ";
  }
  else
  {
    dc << "N  ";
  }
  _counter++;
}

void
Diag851Collector::printAllCarriers(const std::vector<InterlineTicketCarrierInfo*>& interlineInfoVec)
{
  std::vector<InterlineTicketCarrierInfo*>::const_iterator iter = interlineInfoVec.begin();
  std::vector<InterlineTicketCarrierInfo*>::const_iterator iterEnd = interlineInfoVec.end();
  Diag851Collector& dc = *this;

  dc << "\nN - NORMAL   H - HOSTED   P - PSEUDO   S - SUPER PSEUDO \n ";
  _counter = 0;
  for (; iter != iterEnd; ++iter)
  {
    printIETLine((*iter));
  }
}

void
Diag851Collector::printItinCrxLine(const std::vector<CarrierCode>& cxrVec)
{
  Diag851Collector& dc = *this;

  std::vector<CarrierCode>::const_iterator it = cxrVec.begin();
  std::vector<CarrierCode>::const_iterator itEnd = cxrVec.end();

  dc << "CARRIERS IN ITIN: ";

  for (; it != itEnd; ++it)
  {
    dc << (*it) << " ";
  }
  dc << "\n";
}
}
