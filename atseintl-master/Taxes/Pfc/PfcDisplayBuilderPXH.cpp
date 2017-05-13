
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

#include "Taxes/Pfc/PfcDisplayBuilderPXH.h"
#include "Taxes/Pfc/PfcDisplayErrorMsg.h"
#include "DBAccess/PfcPFC.h"
#include <vector>

using namespace tse;

const std::string PfcDisplayBuilderPXH::MAIN_HEADER_PXH = "HISTORICAL RECORDS";

const std::string PfcDisplayBuilderPXH::TABLE_HEADER =
    "AIR   EFF     EXPIRE  DISC    PFC      SEG   DATA CREATE USA\n"
    "PORT  DATE    DATE    DATE    CUR/AMT  CNT   CENTRAL TIME ZONE";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXH::PfcDisplayBuilderPXH
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXH::PfcDisplayBuilderPXH(TaxTrx* trx, PfcDisplayData* data)
  : PfcDisplayBuilder(trx, data), _formatter(PfcDisplayFormatterPXH(TABLE_HEADER))
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::PfcDisplayBuilderPXH
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXH::~PfcDisplayBuilderPXH() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXH::buildHeader
//
// Description:  PXH entry header.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXH::buildHeader()
{
  std::string header;
  const std::vector<PfcPFC*>& pfcV = data()->getPfcPFCHist();

  if (pfcV.empty())
  {
    return PfcDisplayErrorMsg::DATA_NOT_FOUND;
  }
  else
  {
    return fmt().center(MAIN_HEADER) + " \n" + fmt().center(MAIN_HEADER_PXH) + " \n" +
           TABLE_HEADER + "\n";
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXH::buildBody
//
// Description:  PXH entry body.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXH::buildBody()
{
  std::string body;

  const std::vector<PfcPFC*>& pfcV = data()->getPfcPFCHist();

  if (pfcV.empty())
  {
    return body;
  }

  body = " \n";

  std::string arpt;
  std::string effDate;
  std::string expireDate;
  std::string discDate;
  std::string amount;
  std::string segCnt;
  std::string createData;

  std::vector<PfcPFC*>::const_iterator it = pfcV.begin();
  std::vector<PfcPFC*>::const_iterator itEnd = pfcV.end();

  for (; it < itEnd; it++)
  {
    arpt = (*it)->pfcAirport();
    effDate = fmt().toString((*it)->effDate());
    expireDate = fmt().toString((*it)->expireDate());
    discDate = fmt().toString((*it)->discDate());
    amount = fmt().pfcAmount((*it)->pfcAmt1());
    segCnt = fmt().counter((*it)->segCnt());
    createData = fmt().dateTime((*it)->createDate());

    body += fmt().line(arpt, effDate, expireDate, discDate, amount, segCnt, createData);
  }

  return body;
}
