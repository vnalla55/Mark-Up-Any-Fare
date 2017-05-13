
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

#include "Taxes/Pfc/PfcDisplayDataPXT.h"

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXA::PfcDisplayDataPXA
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXT::PfcDisplayDataPXT(TaxTrx* trx, PfcDisplayDb* db) : PfcDisplayData(trx, db) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXT::~PfcDisplayDataPXT
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXT::~PfcDisplayDataPXT() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXT::~PfcDisplayDataPXT
//
// Description:  Destructor
//
// </PRE>
// ---------------------------------------------------------------------------

const std::vector<PfcCollectMeth*>&
PfcDisplayDataPXT::getAllCollectMethData()
{
  // req
  if (trx()->pfcDisplayRequest()->carrier1().empty())
  {
    return db()->getAllPfcCollectMethData();
  }
  else
  {
    return db()->getPfcCollectMethData(trx()->pfcDisplayRequest()->carrier1());
  }
}
