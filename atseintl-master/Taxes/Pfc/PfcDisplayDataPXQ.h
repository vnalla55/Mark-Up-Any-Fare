//----------------------------------------------------------------------------
//  File:           PfcDisplayDataPXQ.h
//  Authors:        Jakub Kubica
//  Created:        4/17/2008
//  Description:    PfcDisplayDBProxy header file for ATSE V2 PFC Display Project.
//                  DB Proxy for PFC Display functionality.
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
#ifndef PFC_DISPLAY_DATA_PXQ_H
#define PFC_DISPLAY_DATA_PXQ_H

#include "Taxes/Pfc/PfcDisplayData.h"

#include "Common/TseConsts.h"

namespace tse
{
class PfcDisplayDataPXQ : public PfcDisplayData
{
public:
  PfcDisplayDataPXQ(TaxTrx* trx, PfcDisplayDb* db);
  virtual ~PfcDisplayDataPXQ();
  const std::vector<PfcEquipTypeExempt*>& getAllEquipmentExempt();
};

} // tse

#endif
