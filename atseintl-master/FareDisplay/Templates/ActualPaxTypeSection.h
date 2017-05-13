//-------------------------------------------------------------------
//
//  File:        ActualPaxTypeSection.h
//  Author:      Svetlana Tsarkova
//  Created:     October 10, 2005
//  Description: Diagnostic for Actual Passenger Type Map
//
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

class ActualPaxTypeSection : public Section
{
public:
  ActualPaxTypeSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;

private:
  void displayActualPaxTypeMap() const;
};
} // namespace tse
