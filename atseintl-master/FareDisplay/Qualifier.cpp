//////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------
//
//  File:        Qualifier.cpp
//  Created:     Feb 6, 2006
//  Authors:     J.D. Batchelor
//
//  Updates:
//
//  Copyright Sabre 2006
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Qualifier.h"

#include "Common/Logger.h"

namespace tse
{
log4cxx::LoggerPtr
Qualifier::_logger(log4cxx::Logger::getLogger("atseintl.FareDisplay.Qualifier"));

const PaxTypeFare::FareDisplayState
Qualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  return PaxTypeFare::FD_Valid;
}
}
