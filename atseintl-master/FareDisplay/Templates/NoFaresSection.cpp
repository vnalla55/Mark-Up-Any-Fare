//-------------------------------------------------------------------
//
//  File:        NoFaresSection.cpp
//  Authors:     Svetlana Tsarkova
//  Created:     October 18, 2005
//  Description: NoFaresSection will build a display when none published fares
//
//
//  Copyright Sabre 2003
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

#include "FareDisplay/Templates/NoFaresSection.h"

#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{
void
NoFaresSection::buildDisplay()
{
  if (_trx.getRequest()->globalDirection() != GlobalDirection::ZZ)
  {
    return;
  }

  _trx.errResponse();
}
} // tse namespace
