//-------------------------------------------------------------------
//
//  File:        FaresHeaderSection.h
//  Created:     July 24, 2005
//  Authors:     Mike Carroll
//  Description: Used for display of fares header
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
#include "DBAccess/FareDispTemplateSeg.h"

namespace tse
{
class FareDisplayTrx;

class FaresHeaderSection : public Section
{
public:
  FaresHeaderSection(FareDisplayTrx& trx, std::vector<FareDispTemplateSeg*>* templateSegRecs)
    : Section(trx), _templateSegRecs(templateSegRecs)
  {
  }

  void buildDisplay() override;

private:
  std::vector<FareDispTemplateSeg*>* _templateSegRecs = nullptr; // a vector of segment records
  bool headerBuilt = false;
};
} // namespace tse
