
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

#include "Taxes/Pfc/PfcDisplayDriver.h"
#include "Taxes/Pfc/PfcDisplayErrorMsg.h"
#include "Taxes/Pfc/PfcDisplayDb.h"
#include "Taxes/Pfc/PfcDisplayData.h"
#include "Taxes/Pfc/PfcDisplayDataPXC.h"
#include "Taxes/Pfc/PfcDisplayDataPXE.h"
#include "Taxes/Pfc/PfcDisplayDataPXA.h"
#include "Taxes/Pfc/PfcDisplayDataPXM.h"
#include "Taxes/Pfc/PfcDisplayDataPXH.h"
#include "Taxes/Pfc/PfcDisplayDataPXT.h"
#include "Taxes/Pfc/PfcDisplayBuilderHelp.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXC.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXE.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXA.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXM.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXH.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXQ.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXT.h"
#include "Taxes/Pfc/PfcDisplayDataPXQ.h"

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayCmdDecoder::PfcDisplayDriver
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDriver::PfcDisplayDriver(TaxTrx* trx) : _trx(trx) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDriver::~PfcDisplayDriver
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDriver::~PfcDisplayDriver() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDriver::PfcDisplayDriver
//
// Description: Build PFC Display entry response.
//
// </PRE>
// ----------------------------------------------------------------------------
void
PfcDisplayDriver::buildPfcDisplayResponse()
{

  PfcDisplayDb db(trx());

  if (!db.isValidDate())
  {
    DateTime oldestAllowedDate = db.oldestAllowedDate();

    if (db.date() < oldestAllowedDate)
    {
      trx()->response() << PfcDisplayErrorMsg::BEYOND_MAX_HISTORICAL_DATE + " - " +
                               oldestAllowedDate.dateToString(tse::DDMMMYY, nullptr);
    }
  }
  else if (trx()->pfcDisplayRequest()->cmdType() == PfcDisplayRequest::HELP)
  {
    PfcDisplayData da(trx(), &db);
    PfcDisplayBuilderHelp builder(trx(), &da);
    trx()->response() << builder.build();
  }
  else if (trx()->pfcDisplayRequest()->cmdType() == PfcDisplayRequest::PXC)
  {
    PfcDisplayDataPXC data(trx(), &db);
    PfcDisplayBuilderPXC builder(trx(), &data);
    trx()->response() << builder.build();
  }
  else if (trx()->pfcDisplayRequest()->cmdType() == PfcDisplayRequest::PXA)
  {
    PfcDisplayDataPXA data(trx(), &db);

    if (data.isPNR())
    {
      PfcDisplayBuilderPXA_itineraryInfo builder(trx(), &data);
      trx()->response() << builder.build();
    }
    else if (trx()->pfcDisplayRequest()->segments().empty() &&
             trx()->pfcDisplayRequest()->absorptionRecordNumber() == 0)
    {
      PfcDisplayBuilderPXA_generalInfo builder(trx(), &data);
      trx()->response() << builder.build();
    }
    else
    {

      if (!trx()->pfcDisplayRequest()->carrier1().empty() &&
          !trx()->pfcDisplayRequest()->carrier2().empty())
      {

        PfcDisplayBuilderPXA_diffInfo builder(trx(), &data);
        trx()->response() << builder.build();
      }
      else
      {
        PfcDisplayBuilderPXA_detailInfo builder(trx(), &data);
        trx()->response() << builder.build();
      }
    }
  }
  else if (trx()->pfcDisplayRequest()->cmdType() == PfcDisplayRequest::PXH &&
           !trx()->pfcDisplayRequest()->segments().empty())
  {
    PfcDisplayDataPXH data(trx(), &db);
    PfcDisplayBuilderPXH builder(trx(), &data);
    trx()->response() << builder.build();
  }
  else if (trx()->pfcDisplayRequest()->cmdType() == PfcDisplayRequest::PXQ)
  {
    PfcDisplayDataPXQ data(trx(), &db);
    PfcDisplayBuilderPXQ builder(trx(), &data);
    trx()->response() << builder.build();
  }
  else if (trx()->pfcDisplayRequest()->cmdType() == PfcDisplayRequest::PXE)
  {
    PfcDisplayDataPXE data(trx(), &db);
    PfcDisplayBuilderPXE builder(trx(), &data);
    trx()->response() << builder.build();
  }
  else if (trx()->pfcDisplayRequest()->cmdType() == PfcDisplayRequest::PXT)
  {
    PfcDisplayDataPXT data(trx(), &db);
    PfcDisplayBuilderPXT builder(trx(), &data);
    trx()->response() << builder.build();
  }
  else if (trx()->pfcDisplayRequest()->cmdType() == PfcDisplayRequest::PXM)
  {
    PfcDisplayDataPXM data(trx(), &db);
    PfcDisplayBuilderPXM builder(trx(), &data);
    trx()->response() << builder.build();
  }
  else
  {
    trx()->response() << PfcDisplayErrorMsg::INVALID_FORMAT;
  }
}
