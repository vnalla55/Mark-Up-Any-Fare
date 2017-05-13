//----------------------------------------------------------------------------
//  File:           PfcDisplayCurrencyFacade.h
//  Authors:        Piotr Lach
//  Created:        5/9/2008
//  Description:    PfcDisplayCurrencyFacade header file for ATSE V2 PFC Display Project.
//                  Facade for LocalCurrencyDisplay class.
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#ifndef PFC_DISPLAY_CURRENCY_FACADE_H
#define PFC_DISPLAY_CURRENCY_FACADE_H

#include "DataModel/TaxTrx.h"
#include "Taxes/Pfc/PfcDisplayDb.h"
#include <string>

namespace tse
{

class PfcDisplayCurrencyFacade
{
public:
  PfcDisplayCurrencyFacade(const TaxTrx* trx);
  virtual ~PfcDisplayCurrencyFacade();

  virtual std::string getEquivalentAmount(const MoneyAmount amt,
                                          const std::string& targetCurrency,
                                          const DateTime& dateTime) const;

  static const std::string FARES_APPL;
  static const std::string TAX_APPL;
  static const size_t ROUND_NOTE_POS = 11;

private:
  const TaxTrx* trx() const { return _trx; }

  const TaxTrx* _trx;
};

} // namespace tse
#endif
