// --------------------------------------------------------------------------
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

#include "Taxes/Pfc/PfcDisplayBuilderPXT.h"

using namespace tse;

const std::string PfcDisplayBuilderPXT::TABLE_HEADER =
    "     PRIMARY EFF       DISC      COUNTRY CODE/OPTION";
const std::string PfcDisplayBuilderPXT::TABLE_HEADER2 = "CXR  OPTION  DATE      DATE";
const std::string PfcDisplayBuilderPXT::TABLE_HEADER_FORMAT =
    "XXX  XXXXXX  XXXX      XXXX      XXXXXXXXXXXXXXXXXXX";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXT::PfcDisplayBuilderPXT
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXT::PfcDisplayBuilderPXT(TaxTrx* trx, PfcDisplayData* data)
  : PfcDisplayBuilder(trx, data), _formatter(PfcDisplayFormatterPXT(TABLE_HEADER_FORMAT))
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::~PfcDisplayBuilderPXC
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXT::~PfcDisplayBuilderPXT() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::buildHeader
//
// Description:  PXC entry header.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXT::buildHeader()
{
  std::string header;

  if (!data()->getAllCollectMethData().empty())
  {
    header = "                  PASSENGER FACILITY CHARGES\n \n";
    header += "              VALIDATING CARRIER OPTIONS TABLE\n \n";
    header += "   OPTIONS FOR INTERNATIONAL TICKETS ISSUED OUTSIDE THE USA\n";
    header += "            ALL - COLLECT ALL APPLICABLE PFCS /MAX 4/\n";
    header += "            DEP - COLLECT FOR DEPARTURE GATEWAY ONLY\n";
    header += "            BYP - DO NOT COLLECT ANY PFC FEES\n \n";

    header += TABLE_HEADER + "\n" + TABLE_HEADER2 + "\n \n";
  }
  else if (!trx()->pfcDisplayRequest()->carrier1().empty())
  {
    header += "CARRIER HAS NO OPTIONS\n";
  }

  return header;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXCQ:buildBody
//
// Description:  PXC entry body.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXT::buildBody()
{
  /// contains whole body information
  std::string body;

  const std::vector<PfcCollectMeth*>& vec = data()->getAllCollectMethData();

  if (vec.empty())
    return std::string();

  std::vector<PfcCollectMeth*>::const_iterator itBegin = vec.begin();
  std::vector<PfcCollectMeth*>::const_iterator it = itBegin;
  std::vector<PfcCollectMeth*>::const_iterator itEnd = vec.end();

  std::string carrier;
  std::string primaryOption;
  std::string effDate;
  std::string expireDate;
  std::string collectOptionstr;

  /// loop through all items
  for (; it < itEnd; it++)
  {
    carrier = (*it)->carrier();
    primaryOption = fmt().colOptToStr((*it)->collectOption());
    effDate = fmt().toString((*it)->effDate());
    expireDate = fmt().toString((*it)->expireDate());
    std::vector<PfcCollectExcpt*>& collectOption = (*it)->excpts();

    std::vector<PfcCollectExcpt*>::const_iterator colBeg = collectOption.begin();
    std::vector<PfcCollectExcpt*>::const_iterator col = colBeg;
    std::vector<PfcCollectExcpt*>::const_iterator colEnd = collectOption.end();
    ///
    for (; col < colEnd; col++)
    {
      collectOptionstr += (*col)->nation() + "/" + fmt().colOptToStr((*col)->collectOption()) + " ";
    }

    body += fmt().line(carrier, primaryOption, effDate, expireDate, collectOptionstr);
    collectOptionstr.clear();
  }

  return body;
}
