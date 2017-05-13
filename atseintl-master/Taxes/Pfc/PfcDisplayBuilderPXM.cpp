
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

#include "Taxes/Pfc/PfcDisplayBuilderPXM.h"
#include "Taxes/Pfc/PfcDisplayErrorMsg.h"

using namespace tse;

const std::string PfcDisplayBuilderPXM::MAIN_HEADER_PXM = "MULTI-AIRPORT CITY CODE TABLE";

const std::string PfcDisplayBuilderPXM::TABLE_HEADER = "AIRPORT        TYPE           COTERMINAL";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXH::PfcDisplayBuilderPXH
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXM::PfcDisplayBuilderPXM(TaxTrx* trx, PfcDisplayData* data)
  : PfcDisplayBuilder(trx, data), _formatter(PfcDisplayFormatterPXM(TABLE_HEADER))
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
PfcDisplayBuilderPXM::~PfcDisplayBuilderPXM() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXM::buildHeader
//
// Description:  PXM entry header.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXM::buildHeader()
{
  std::string header;
  int segCntMax;
  const std::vector<PfcMultiAirport*>& pfcMultiAirportV = data()->getPfcMultiAirport(segCntMax);

  if (pfcMultiAirportV.empty())
  {
    return PfcDisplayErrorMsg::NO_MULTIAIRPORT_LOCATIONS;
  }
  else
  {
    return fmt().center(MAIN_HEADER) + " \n" + std::string(16, PfcDisplayFormatter::SPACE) +
           MAIN_HEADER_PXM + "\n \n" + TABLE_HEADER + "\n";
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXM::buildBody
//
// Description:  PXM entry body.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXM::buildBody()
{
  std::string body;
  int segCntMax = 0;

  const std::vector<PfcMultiAirport*>& pfcMultiAirportV = data()->getPfcMultiAirport(segCntMax);

  if (pfcMultiAirportV.empty())
  {
    return body;
  }

  body = " \n";

  std::string arpt;
  std::string type;
  std::string coterminal;

  std::vector<PfcMultiAirport*>::const_iterator it = pfcMultiAirportV.begin();
  std::vector<PfcMultiAirport*>::const_iterator itEnd = pfcMultiAirportV.end();

  for (; it < itEnd; it++)
  {
    arpt = (*it)->loc().loc();
    type = locTypeDesc((*it)->loc().locType());
    if (segCntMax)
    {
      coterminal = fmt().toString(segCntMax + 1);
    }
    else
    {
      coterminal = fmt().toString((*it)->segCnt() + 1);
    }
    body += fmt().line(arpt, type, coterminal);
  }

  return body;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXM::locTypeDesc
//
// Description:  MultiAirport description.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXM::locTypeDesc(LocTypeCode locTypeCode) const
{
  if (locTypeCode == 'A')
    return std::string("  AIRPORT");
  else if (locTypeCode == 'B')
    return std::string(" AIRPORT/CITY");
  else if (locTypeCode == 'C')
    return std::string(" CITY");
  else
    return "";
}
