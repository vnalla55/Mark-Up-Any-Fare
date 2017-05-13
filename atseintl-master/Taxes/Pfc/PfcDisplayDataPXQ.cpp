
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

#include "Taxes/Pfc/PfcDisplayDataPXQ.h"

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
PfcDisplayDataPXQ::PfcDisplayDataPXQ(TaxTrx* trx, PfcDisplayDb* db) : PfcDisplayData(trx, db) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXQ::~PfcDisplayDataPXQ
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXQ::~PfcDisplayDataPXQ() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXQ::~PfcDisplayDataPXQ
//
// Description:  Destructor
//
// </PRE>
// ---------------------------------------------------------------------------

const std::vector<PfcEquipTypeExempt*>&
PfcDisplayDataPXQ::getAllEquipmentExempt()
{
  return db()->getAllPfcEquipTypeExempt();
}
