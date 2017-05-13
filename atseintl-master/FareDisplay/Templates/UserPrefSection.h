//-------------------------------------------------------------------
//
//  File:        UserPrefSection.h
//  Authors:     Svetlana Tsarkova
//  Created:     September 15, 2005
//  Description: Display Diagnostic for UserPreferences
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

class UserPrefSection : public Section
{

public:
  UserPrefSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;

private:
  void displayUserPref(FareDisplayTrx& trx) const;
  void displayYesNo(FareDisplayTrx& trx, Indicator ind) const;
};
} // namespace tse
