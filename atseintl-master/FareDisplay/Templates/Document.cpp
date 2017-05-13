//-------------------------------------------------------------------
//
//  File:        Document.cpp
//  Authors:     Mike Carroll
//  Created:     July 24, 2005
//  Description: Base class for a document.
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

#include "FareDisplay/Templates/Document.h"

#include "FareDisplay/Templates/Section.h"

namespace tse
{
void
Document::buildDisplay()
{
  for (const auto section : _sections)
    section->buildDisplay();
}
}
