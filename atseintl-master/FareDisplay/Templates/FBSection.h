//-------------------------------------------------------------------
//
//  File:        FBSection.h
//  Authors:     Doug Batchelor
//  Created:     May 4, 2005
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize a FB template (the set of labels
//               and fields that appear for a FB diagnostic on the
//               Sabre greenscreen.)
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

#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Templates/Section.h"

namespace tse
{

class FBSection : public Section
{
  friend class FBSectionTest;
public:
  FBSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;

private:
  void addCategoryLine(const CatNumber& catNumber);
  FBCategoryRuleRecord* _catRuleRecord = nullptr;
};
} // namespace tse
