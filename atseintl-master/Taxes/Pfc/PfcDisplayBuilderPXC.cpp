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

#include "Taxes/Pfc/PfcDisplayBuilderPXC.h"
#include "Taxes/Pfc/PfcDisplayErrorMsg.h"
#include "Common/Money.h"

using namespace tse;

const std::string PfcDisplayBuilderPXC::ITINERARY_HEADER =
    "MAXIMUM OF FOUR PFCS PER PASSENGER TICKET";

const std::string PfcDisplayBuilderPXC::TABLE_HEADER =
    "ARPT     EFF DATE   DISC DATE  AMOUNT      EQUIV AMT    NOTE";

const std::string PfcDisplayBuilderPXC::CONNEX_CITY_LOCAL_ABSORPTION_SIGN = "$";
const std::string PfcDisplayBuilderPXC::ESSENTIAL_AIR_SVC_EXEMPTION_SIGN = "*";
const std::string PfcDisplayBuilderPXC::AIR_TAX_EXEMPTION_SIGN = "/";
const std::string PfcDisplayBuilderPXC::CHARTER_EXEMPTION_SIGN = "-";
const std::string PfcDisplayBuilderPXC::FREQUENT_FLYER_EXEMPTION_SIGN = "@";

const std::string PfcDisplayBuilderPXC::CONNEX_CITY_LOCAL_ABSORPTION_DESCRIPTION =
    CONNEX_CITY_LOCAL_ABSORPTION_SIGN +
    " CONNEX CITY/LOCAL ABSORPTION MAY APPLY-ENTER PXA* FOR LIST\n";

const std::string PfcDisplayBuilderPXC::ESSENTIAL_AIR_SVC_EXEMPTION_DESCRIPTION =
    ESSENTIAL_AIR_SVC_EXEMPTION_SIGN +
    " ESSENTIAL AIR SVC EXEMPTION MAY APPLY-ENTER PXE* FOR LIST\n";

const std::string PfcDisplayBuilderPXC::AIR_TAX_EXEMPTION_DESCRIPTION =
    AIR_TAX_EXEMPTION_SIGN + " AIR TAXI EXEMPTION APPLIES\n";

const std::string PfcDisplayBuilderPXC::CHARTER_EXEMPTION_DESCRIPTION =
    CHARTER_EXEMPTION_SIGN + " CHARTER EXEMPTION APPLIES\n";

const std::string PfcDisplayBuilderPXC::FREQUENT_FLYER_EXEMPTION_DESCRIPTION =
    FREQUENT_FLYER_EXEMPTION_SIGN + " FREQUENT FLYER EXEMPTION APPLIES\n";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::PfcDisplayBuilderPXC
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXC::PfcDisplayBuilderPXC(TaxTrx* trx, PfcDisplayData* data)
  : PfcDisplayBuilder(trx, data), _formatter(PfcDisplayFormatterPXC(TABLE_HEADER))
{
  this->data()->getPfcPFC(_pfcV, _warningMap);
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
PfcDisplayBuilderPXC::~PfcDisplayBuilderPXC() {}

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
PfcDisplayBuilderPXC::buildHeader()
{
  if (pfcV().empty())
  {
    return PfcDisplayErrorMsg::DATA_NOT_FOUND;
  }

  std::string header = fmt().center(MAIN_HEADER) + " \n";

  std::vector<PfcPFC*>::const_iterator it = pfcV().begin();
  std::vector<PfcPFC*>::const_iterator itEnd = pfcV().end();

  PfcDisplayDataPXC::WarningMap::iterator warningI;

  for (; it < itEnd; it++)
  {
    warningI = pfcWarning().find((*it)->pfcAirport());

    if (warningI != pfcWarning().end())
    {
      continue;
    }

    if (!data()->getEquivalentAmount((*it)->pfcAmt1()).empty())
    {
      header +=
          fmt().center("AMOUNT CONVERTED TO " + data()->getCustomerCurrency() + " USING BSR") +
          " \n";
      break;
    }
  }

  if (data()->isPNR())
  {
    header += fmt().center(ITINERARY_HEADER) + "\n \n";
  }

  header += TABLE_HEADER + " \n";

  return header;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::buildBody
//
// Description:  PXC entry body.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXC::buildBody()
{
  std::string body;

  if (pfcV().empty())
  {
    return body;
  }

  body = " \n";

  std::string arpt;
  std::string effDate;
  std::string discDate;
  std::string amount;
  std::string equivAmt;
  std::string note;

  uint32_t noteCounter = 0;

  std::vector<PfcPFC*>::const_iterator itBegin = pfcV().begin();
  std::vector<PfcPFC*>::const_iterator it = itBegin;
  std::vector<PfcPFC*>::const_iterator itEnd = pfcV().end();

  PfcDisplayDataPXC::WarningMap::iterator warningI;

  for (; it < itEnd; it++)
  {
    arpt = (*it)->pfcAirport() + fmt().toString(specialSigns(**it));

    if (arpt.empty())
    {
      continue;
    }

    warningI = pfcWarning().find((*it)->pfcAirport());

    if (warningI != pfcWarning().end())
    {
      body += fmt().warningLine((*it)->pfcAirport(), warningI->second);
      continue;
    }

    effDate = fmt().toString((*it)->effDate());
    discDate = fmt().toString((*it)->expireDate());
    amount = fmt().pfcAmount((*it)->pfcAmt1());
    equivAmt = data()->getEquivalentAmount((*it)->pfcAmt1());

    if (!(*it)->cxrExcpts().empty())
    {
      ++noteCounter;
      note = fmt().counter(noteCounter);
    }
    else
    {
      note = std::string();
    }

    body += fmt().line(arpt, effDate, discDate, amount, equivAmt, note);
  }

  return body;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::buildFootnote
//
// Description:  PXC entry footnote.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXC::buildFootnote()
{
  std::string footnote = specialSignsDescription();

  if (footnote.empty())
  {
    return footnote;
  }

  footnote = footnote + " \n \n";

  uint32_t noteCounter = 0;
  std::vector<PfcCxrExcpt*>::const_iterator cxrExcptsItBegin;
  std::vector<PfcCxrExcpt*>::const_iterator cxrExcptsIt;
  std::vector<PfcCxrExcpt*>::const_iterator cxrExcptsItEnd;

  PfcDisplayDataPXC::WarningMap::iterator warningI;

  std::vector<PfcPFC*>::const_iterator it = pfcV().begin();
  std::vector<PfcPFC*>::const_iterator itEnd = pfcV().end();

  for (; it < itEnd; it++)
  {
    warningI = pfcWarning().find((*it)->pfcAirport());

    if (warningI != pfcWarning().end())
    {
      continue;
    }

    std::vector<PfcCxrExcpt*>& cxrExcptsV = (*it)->cxrExcpts();

    if (!cxrExcptsV.empty())
    {
      cxrExcptsItBegin = cxrExcptsV.begin();
      cxrExcptsIt = cxrExcptsV.begin();
      cxrExcptsItEnd = cxrExcptsV.end();

      ++noteCounter;
      footnote += "NOTES:" + fmt().counter(noteCounter);

      for (; cxrExcptsIt < cxrExcptsItEnd; cxrExcptsIt++)
      {
        if (cxrExcptsIt - cxrExcptsItBegin > 0)
        {
          footnote += std::string(9, PfcDisplayFormatter::SPACE);
        }

        footnote += " DOES NOT APPLY TO " + (*cxrExcptsIt)->excpCarrier() + " FLTS " +
                    fmt().flight((*cxrExcptsIt)->flt1()) + " - " +
                    fmt().flight((*cxrExcptsIt)->flt2()) + "\n";
      }
    }
  }
  return footnote;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::specialSignsDescription
//
// Description:  Airport special signs descripton.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXC::specialSignsDescription()
{
  std::string desc;

  if (pfcV().empty())
  {
    return desc;
  }

  std::vector<std::string> signV(SPECIAL_SIGN_NUMBER, "");
  PfcDisplayDataPXC::WarningMap::iterator warningI;
  std::vector<PfcPFC*>::const_iterator it = pfcV().begin();
  std::vector<PfcPFC*>::const_iterator itEnd = pfcV().end();

  for (; it < itEnd; it++)
  {
    warningI = pfcWarning().find((*it)->pfcAirport());

    if (warningI != pfcWarning().end())
    {
      continue;
    }

    for (uint32_t i = 0; i < SPECIAL_SIGN_NUMBER; i++)
    {
      if (!specialSigns(**it)[i].empty())
      {
        signV[i] = specialSigns(**it)[i];
      }
    }

    if (signV[0] == CONNEX_CITY_LOCAL_ABSORPTION_SIGN &&
        signV[1] == ESSENTIAL_AIR_SVC_EXEMPTION_SIGN && signV[2] == AIR_TAX_EXEMPTION_SIGN &&
        signV[3] == CHARTER_EXEMPTION_SIGN && signV[4] == FREQUENT_FLYER_EXEMPTION_SIGN)
    {
      break;
    }
  }

  if (signV[0] == CONNEX_CITY_LOCAL_ABSORPTION_SIGN)
  {
    desc += CONNEX_CITY_LOCAL_ABSORPTION_DESCRIPTION;
  }
  if (signV[1] == ESSENTIAL_AIR_SVC_EXEMPTION_SIGN)
  {
    desc += ESSENTIAL_AIR_SVC_EXEMPTION_DESCRIPTION;
  }
  if (signV[2] == AIR_TAX_EXEMPTION_SIGN)
  {
    desc += AIR_TAX_EXEMPTION_DESCRIPTION;
  }
  if (signV[3] == CHARTER_EXEMPTION_SIGN)
  {
    desc += CHARTER_EXEMPTION_DESCRIPTION;
  }
  if (signV[4] == FREQUENT_FLYER_EXEMPTION_SIGN)
  {
    desc += FREQUENT_FLYER_EXEMPTION_DESCRIPTION;
  }

  if (!desc.empty())
  {
    desc = " \n" + desc;
  }

  return desc;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::pecialSignss
//
// Description:  Airport special signs for the PFC record.
//
// </PRE>
// ----------------------------------------------------------------------------

std::vector<std::string>
PfcDisplayBuilderPXC::specialSigns(PfcPFC& pfc)
{
  std::vector<std::string> signV(SPECIAL_SIGN_NUMBER, "");

  if (data()->isPfcAbsorb(pfc.pfcAirport()))
  {
    signV[0] = CONNEX_CITY_LOCAL_ABSORPTION_SIGN;
  }
  if (data()->isPfcEssAirSvc(pfc.pfcAirport()))
  {
    signV[1] = ESSENTIAL_AIR_SVC_EXEMPTION_SIGN;
  }
  if (pfc.pfcAirTaxExcp() == YES)
  {
    signV[2] = AIR_TAX_EXEMPTION_SIGN;
  }
  if (pfc.pfcCharterExcp() == YES)
  {
    signV[3] = CHARTER_EXEMPTION_SIGN;
  }
  if (pfc.freqFlyerInd() == YES)
  {
    signV[4] = FREQUENT_FLYER_EXEMPTION_SIGN;
  }

  return signV;
}
