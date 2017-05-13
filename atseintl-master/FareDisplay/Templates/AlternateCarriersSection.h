//-------------------------------------------------------------------
//
//  File:        AlternateCarriersSection.h
//  Authors:     Doug Batchelor
//  Created:     May 10, 2005
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize the Alternate Carriers that
//               appear on the Sabre greenscreen.)
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

#pragma once

#include "FareDisplay/Templates/Section.h"

namespace tse
{
class FareDisplayTrx;

class AlternateCarriersSection : public Section
{
public:
  AlternateCarriersSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;

private:
  bool isIncludeAddonTable() const;
};
}
