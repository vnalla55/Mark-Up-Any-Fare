//----------------------------------------------------------------------------
//  File:           PfcDisplayDataPXM.h
//  Authors:        Piotr Lach
//  Created:        7/10/2008
//  Description:    PfcDisplayDataPXM header file for ATSE V2 PFC Display Project.
//                  Data filter for PXM entry.
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
#ifndef PFC_DISPLAY_DATA_PXM_H
#define PFC_DISPLAY_DATA_PXM_H

#include "Taxes/Pfc/PfcDisplayData.h"

namespace tse
{

class PfcDisplayDataPXM : public PfcDisplayData
{
public:
  PfcDisplayDataPXM(TaxTrx* trx, PfcDisplayDb* db);
  virtual ~PfcDisplayDataPXM();

  const std::vector<PfcMultiAirport*>& getPfcMultiAirport(int& segCntMax);

private:
  void
  storePfcMultiAirportData(const LocCode& loc, std::vector<PfcMultiAirport*>* pmaV, int& segCntMax);
};

} // namespace tse
#endif
