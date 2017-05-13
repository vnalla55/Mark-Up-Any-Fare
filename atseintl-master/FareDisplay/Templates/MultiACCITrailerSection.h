//-------------------------------------------------------------------
//
//  File:        MultiACCITrailerSection.h
//  Description: Builds the trailer message section for multi account
//               code and/or corporate ID's FareDisplay response
//
//  Copyright Sabre 2008
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
class MultiACCITrailerSection : public Section
{
public:
  MultiACCITrailerSection(FareDisplayTrx& trx) : Section(trx) {}
  void buildDisplay() override;
};
}
