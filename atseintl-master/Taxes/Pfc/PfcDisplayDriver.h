//----------------------------------------------------------------------------
//  File:           PfcDisplayDriver.h
//  Authors:        Piotr Lach
//  Created:        2/18/2008
//  Description:    PfcDisplayDriver header file for ATSE V2 PFC Display Project.
//                  The object of this class builds PXC Dislay entry response.
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
#ifndef PFC_DISPLAY_FACTORY_H
#define PFC_DISPLAY_FACTORY_H

#include "DataModel/TaxTrx.h"

namespace tse
{


class PfcDisplayDriver
{
public:
  PfcDisplayDriver(TaxTrx* trx);
  virtual ~PfcDisplayDriver();

  void buildPfcDisplayResponse();

private:
  const TaxTrx* trx() const { return _trx; }
  TaxTrx* trx() { return _trx; }

  TaxTrx* _trx;
};

} // namespace tse
#endif
