//----------------------------------------------------------------------------
//  File:           PfcDisplayDataPXH.h
//  Authors:        Piotr Lach
//  Created:        6/23/2008
//  Description:    PfcDisplayDataPXH header file for ATSE V2 PFC Display Project.
//                  Data filter for PXH entry.
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
#ifndef PFC_DISPLAY_DATA_PXH_H
#define PFC_DISPLAY_DATA_PXH_H

#include "Taxes/Pfc/PfcDisplayData.h"

namespace tse
{

class PfcDisplayDataPXH : public PfcDisplayData
{
public:
  PfcDisplayDataPXH(TaxTrx* trx, PfcDisplayDb* db);
  virtual ~PfcDisplayDataPXH();

  const std::vector<PfcPFC*>& getPfcPFCHist();
};

} // namespace tse
#endif
