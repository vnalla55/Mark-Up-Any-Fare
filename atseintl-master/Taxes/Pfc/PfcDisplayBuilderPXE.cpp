
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

#include "Taxes/Pfc/PfcDisplayBuilderPXE.h"
#include "Taxes/Pfc/PfcDisplayErrorMsg.h"
#include "Taxes/Pfc/PfcDisplayDb.h"
#include "DBAccess/PfcEssAirSvcProv.h"
#include <vector>

using namespace tse;

const std::string PfcDisplayBuilderPXE::MAIN_HEADER_PXE = "ESSENTIAL AIR SERVICE EXEMPTIONS";

const std::string PfcDisplayBuilderPXE::TABLE_HEADER =
    "FROM     TO      CXR     FLT RANGE     EFF DATE     DISC DATE";

const std::string PfcDisplayBuilderPXE::IATA_SPECIAL_CARRIER_SIGN_DB = "$$";
const std::string PfcDisplayBuilderPXE::IATA_SPECIAL_CARRIER_SIGN_DISPLAY = "YY";

const std::string PfcDisplayBuilderPXE::ALL_FLIGHTS_AVAILABLE = "ALL";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXE::PfcDisplayBuilderPXE
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXE::PfcDisplayBuilderPXE(TaxTrx* trx, PfcDisplayData* data)
  : PfcDisplayBuilder(trx, data), _formatter(PfcDisplayFormatterPXE(TABLE_HEADER))
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::PfcDisplayBuilderPXE
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXE::~PfcDisplayBuilderPXE() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXE::buildHeader
//
// Description:  PXE entry header.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXE::buildHeader()
{
  std::string header;
  const std::vector<PfcEssAirSvc*>& pfcEssAirSvcV = data()->getAllPfcEssAirSvc();

  if (pfcEssAirSvcV.empty())
  {
    return PfcDisplayErrorMsg::NO_AIRPORT_EAS_LOCATIONS;
  }
  else
  {
    return fmt().center(MAIN_HEADER) + " \n" + fmt().center(MAIN_HEADER_PXE) + " \n" +
           TABLE_HEADER + "\n";
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXE::buildBody
//
// Description:  PXE entry body.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXE::buildBody()
{
  std::string body;

  const std::vector<PfcEssAirSvc*>& pfcEssAirSvcV = data()->getAllPfcEssAirSvc();

  if (pfcEssAirSvcV.empty())
  {
    return body;
  }

  body = " \n";

  std::string from;
  std::string to;
  std::string cxr;
  std::string fltRange;
  std::string effDate;
  std::string discDate;

  std::string fromPrev;

  std::vector<PfcEssAirSvcProv*> asProvs;

  std::vector<PfcEssAirSvcProv*>::const_iterator itAsProvs;
  std::vector<PfcEssAirSvcProv*>::const_iterator itEndAsProvs;

  std::vector<PfcEssAirSvc*>::const_iterator it = pfcEssAirSvcV.begin();
  std::vector<PfcEssAirSvc*>::const_iterator itEnd = pfcEssAirSvcV.end();

  for (; it < itEnd; it++)
  {
    if ((*it)->easHubArpt() != fromPrev)
    {
      from = (*it)->easHubArpt();
    }

    to = (*it)->easArpt();
    effDate = fmt().toString((*it)->effDate());
    discDate = fmt().toString((*it)->expireDate());

    asProvs = (*it)->asProvs();

    std::sort(asProvs.begin(),
              asProvs.end(),
              std::not2(PfcDisplayDb::Greater<PfcEssAirSvcProv, CarrierCode>(
                  &PfcEssAirSvcProv::easCarrier)));

    itAsProvs = asProvs.begin();
    itEndAsProvs = asProvs.end();

    for (; itAsProvs < itEndAsProvs; itAsProvs++)
    {
      cxr = (*itAsProvs)->easCarrier();

      if (cxr == IATA_SPECIAL_CARRIER_SIGN_DB)
      {
        cxr = IATA_SPECIAL_CARRIER_SIGN_DISPLAY;
      }

      if ((*itAsProvs)->flt1() == 0 && (*itAsProvs)->flt2() == 0)
      {
        fltRange = ALL_FLIGHTS_AVAILABLE;
      }
      else if ((*itAsProvs)->flt1() != 0 && (*itAsProvs)->flt2() == 0)
      {
        fltRange = (*itAsProvs)->flt1();
      }
      else
      {
        fltRange = (*itAsProvs)->flt1() + " - " + (*itAsProvs)->flt2();
      }

      body += fmt().line(from, to, cxr, fltRange, effDate, discDate);

      from = "";
    }

    fromPrev = (*it)->easHubArpt();
  }

  return body;
}
