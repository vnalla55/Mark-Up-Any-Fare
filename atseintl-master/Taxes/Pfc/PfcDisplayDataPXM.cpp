
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

#include "Taxes/Pfc/PfcDisplayDataPXM.h"

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXC::PfcDisplayDataPXC
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXM::PfcDisplayDataPXM(TaxTrx* trx, PfcDisplayDb* db) : PfcDisplayData(trx, db) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXC::~PfcDisplayDataPXC
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXM::~PfcDisplayDataPXM() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXM::getPfcMultiAirport
//
// Description:  Get PFC MultiAirport Data.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcMultiAirport*>&
PfcDisplayDataPXM::getPfcMultiAirport(int& segCntMax)
{
  segCntMax = 0;

  if (trx()->pfcDisplayRequest()->segments().empty())
  {
    return db()->getAllPfcMultiAirport();
  }
  else
  {
    std::vector<PfcMultiAirport*>* pmaV;
    trx()->dataHandle().get(pmaV);

    PfcDisplayRequest::Segment segment = trx()->pfcDisplayRequest()->segments().front();
    LocCode loc = std::get<PfcDisplayRequest::DEPARTURE_AIRPORT>(segment);
    const PfcMultiAirport* pma = db()->getPfcMultiAirport(loc);

    if (pma)
    {
      storePfcMultiAirportData(loc, pmaV, segCntMax);

      std::vector<PfcCoterminal*> coterminals = pma->coterminals();
      if (coterminals.empty())
      {
        return *pmaV;
      }

      std::vector<PfcCoterminal*>::const_iterator itPfcCoterminal = coterminals.begin();
      std::vector<PfcCoterminal*>::const_iterator itEndPfcCoterminal = coterminals.end();

      for (; itPfcCoterminal < itEndPfcCoterminal; itPfcCoterminal++)
      {
        loc = (*itPfcCoterminal)->cotermLoc();
        storePfcMultiAirportData(loc, pmaV, segCntMax);
      }
    }
    return *pmaV;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXM::storePfcMultiAirportData
//
// Description:  Store PFC MultiAirport Data.
//
// </PRE>
// ----------------------------------------------------------------------------
void
PfcDisplayDataPXM::storePfcMultiAirportData(const LocCode& loc,
                                            std::vector<PfcMultiAirport*>* pmaV,
                                            int& segCntMax)
{
  PfcMultiAirport* pma = const_cast<PfcMultiAirport*>(db()->getPfcMultiAirport(loc));

  if (pma)
  {
    if (pma->segCnt() > segCntMax)
    {
      segCntMax = pma->segCnt();
    }

    pmaV->push_back(pma);
  }
}
