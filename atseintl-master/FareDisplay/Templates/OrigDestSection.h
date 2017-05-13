//-------------------------------------------------------------------
//
//  File:        OrigDestSection.h
//  Created:     July 24, 2005
//  Authors:     Mike Carroll
//  Description: Generally, the first line of FareDisplay PSS response
//
//  Updates:
//
//  Copyright Sabre 2005
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

#pragma once

#include "FareDisplay/Templates/Section.h"

namespace tse
{
class FareDisplayTrx;

class OrigDestSection : public Section
{
public:
  OrigDestSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;
};
} // namespace tse
