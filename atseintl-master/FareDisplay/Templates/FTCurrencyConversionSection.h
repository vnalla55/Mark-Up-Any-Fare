//-------------------------------------------------------------------
//
//  File:        FTCurrencyConversionSection.h
//  Created:     Jan 23, 2006
//  Authors:     LeAnn Perez
//  Description: Currency conversion display section
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

#include "FareDisplay/Templates/CurrencyConversionSection.h"
namespace tse
{
class FareDisplayTrx;

class FTCurrencyConversionSection : public CurrencyConversionSection
{
public:
  FTCurrencyConversionSection(FareDisplayTrx& trx) : CurrencyConversionSection(trx) {}

  void buildDisplay() override;
};
} // namespace tse
