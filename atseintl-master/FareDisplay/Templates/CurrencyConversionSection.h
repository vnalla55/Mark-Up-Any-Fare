//-------------------------------------------------------------------
//
//  File:        CurrencyConversionSection.h
//  Created:     July 24, 2005
//  Authors:     Mike Carroll
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

#include "FareDisplay/Templates/Section.h"

namespace tse
{
class FareDisplayTrx;

class CurrencyConversionSection : public Section
{
public:
  CurrencyConversionSection(FareDisplayTrx& trx) : Section(trx) {}

  bool getConversionInfo(CurrencyCode& sourceCurrency,
                         CurrencyCode& targetCurrency,
                         CurrencyCode& intermediateCurrency,
                         ExchRate& exchangRate1,
                         ExchRate& exchangeRate2,
                         CurrencyNoDec& exchangRate1NoDec);
};
} // namespace tse
