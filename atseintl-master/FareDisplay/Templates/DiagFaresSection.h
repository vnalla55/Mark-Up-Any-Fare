//-------------------------------------------------------------------
//
//  File:        DiagFaresSection.h
//  Created:     July 24, 2005
//  Authors:     Mike Carroll
//  Description: Used for display of fares and related fare information
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
#include "FareDisplay/Templates/DiagElementField.h"

namespace tse
{
class FareDisplayTrx;

class DiagFaresSection : public Section
{
public:
  DiagFaresSection(FareDisplayTrx& trx) : Section(trx) {}
  void buildDisplay() override;

  //--------------------------------------------------------------------------
  // @function DiagFaresSection::securityCheck
  //
  // Description: This method allows to display fares that failed Cat 15 only
  //              for duty code'$'
  //
  // @param none
  //--------------------------------------------------------------------------

  bool securityCheck(PaxTypeFare* ptf);

  // Accessors
  std::vector<DiagElementField*>& columnFields() { return _columnFields; }
  const std::vector<DiagElementField*>& columnFields() const { return _columnFields; }

private:
  std::vector<DiagElementField*> _columnFields;
};
} // namespace tse
