
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

#include "Taxes/Pfc/PfcDisplayDataPXE.h"

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXE::PfcDisplayDataPXE
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXE::PfcDisplayDataPXE(TaxTrx* trx, PfcDisplayDb* db) : PfcDisplayData(trx, db) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXE::PfcDisplayDataPXE
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXE::~PfcDisplayDataPXE() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXE::getPfcPFCHist()
//
// Description:  Get all PXE records
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcEssAirSvc*>&
PfcDisplayDataPXE::getAllPfcEssAirSvc()
{

  if (trx()->pfcDisplayRequest()->segments().empty())
  {
    return db()->getAllPfcEssAirSvc();
  }
  else
  {
    PfcDisplayRequest::Segment segment = trx()->pfcDisplayRequest()->segments().front();

    return db()->getPfcEssAirSvc(std::get<PfcDisplayRequest::DEPARTURE_AIRPORT>(segment));
  }
}
