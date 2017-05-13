//-------------------------------------------------------------------
//
//  File:        HeaderMsgSection.h
//  Authors:     Doug Batchelor
//  Created:     May 3, 2005
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize a particular set of headers
//               and fields that appear on the Sabre greenscreen.)
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

class HeaderMsgSection : public Section
{
  static const std::string CENTER;
  static const std::string RIGHT;
  static const std::string BLANK_SPACE;

public:
  HeaderMsgSection(FareDisplayTrx& trx) : Section(trx), _preferredCarriers(trx.preferredCarriers())
  {
  }

  void buildDisplay() override;

private:
  void displayAlternateCurrencyMessages(FareDisplayTrx& trx) const;
  std::set<CarrierCode> _preferredCarriers;
};
} // namespace tse
