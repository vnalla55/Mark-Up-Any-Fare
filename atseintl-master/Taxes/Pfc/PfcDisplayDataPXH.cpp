
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/Pfc/PfcDisplayDataPXH.h"

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXH::PfcDisplayDataPXH
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXH::PfcDisplayDataPXH(TaxTrx* trx, PfcDisplayDb* db) : PfcDisplayData(trx, db) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXH::PfcDisplayDataPXH
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXH::~PfcDisplayDataPXH() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXH::getPfcPFCHist()
//
// Description:  Get all historical PXC records
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcPFC*>&
PfcDisplayDataPXH::getPfcPFCHist()
{
  return db()->getDateIndependentPfcPFC(std::get<PfcDisplayRequest::DEPARTURE_AIRPORT>(
      trx()->pfcDisplayRequest()->segments().front()));
}
