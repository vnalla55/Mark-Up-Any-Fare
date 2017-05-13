//-------------------------------------------------------------------
//
//  File:        UserInputSection.h
//  Authors:     Svetlana Tsarkova
//  Created:     September 29, 2005
//  Description: Display Diagnostic for User Input
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
class Agent;
class Loc;

class UserInputSection : public Section
{

public:
  UserInputSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;

private:
  void displayUserInput(FareDisplayTrx& trx) const;
  void displayYesNo(FareDisplayTrx& trx, Indicator ind) const;
  void displayLocInfo(FareDisplayTrx& trx) const;
  void displayAgentInfo(FareDisplayTrx& trx) const;
  void displayRequestInfo(FareDisplayTrx& trx) const;
  void displaySecondaryCxr(FareDisplayTrx& trx) const;
  void displaySecondaryCity1(FareDisplayTrx& trx) const;
  void displaySecondaryCity2(FareDisplayTrx& trx) const;
  void displayPsgTypes(FareDisplayTrx& trx) const;
  void displayDispPsgTypes(FareDisplayTrx& trx) const;
  void displayRec8PsgTypes(FareDisplayTrx& trx) const;
  void displayFareDispOptions(FareDisplayTrx& trx) const;
  void displayRuleCats(FareDisplayTrx& trx) const;
  void displayAlphaCodes(FareDisplayTrx& trx) const;
  void displayCombinCodes(FareDisplayTrx& trx) const;
  void displayTravelSegs(FareDisplayTrx& trx) const;
  void displayMultiAccountCodeCorpIds(FareDisplayTrx& trx) const;
};
} // namespace tse
