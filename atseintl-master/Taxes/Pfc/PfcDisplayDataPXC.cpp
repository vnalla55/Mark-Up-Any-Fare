
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

#include "Taxes/Pfc/PfcDisplayDataPXC.h"
#include "Common/Money.h"
#include "DataModel/Agent.h"
#include "Taxes/Pfc/PfcDisplayCurrencyFacade.h"
#include "Taxes/Pfc/PfcDisplayErrorMsg.h"
#include <algorithm>

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
PfcDisplayDataPXC::PfcDisplayDataPXC(TaxTrx* trx, PfcDisplayDb* db) : PfcDisplayData(trx, db) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXC::~PfcDisplayDataPXC
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXC::~PfcDisplayDataPXC() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::getCustomerCurrency
//
// Description:  Get customer default currency.
//
// </PRE>
// ----------------------------------------------------------------------------
CurrencyCode
PfcDisplayDataPXC::getCustomerCurrency() const
{
  const Agent* agent = trx()->pfcDisplayRequest()->ticketingAgent();

  if (!agent)
  {
    return CurrencyCode();
  }

  const Customer* customer = db()->getCustomer(agent->mainTvlAgencyPCC());

  if (!customer)
  {
    const Loc* loc = db()->getLoc(agent->agentCity());

    if (!loc)
    {
      return CurrencyCode();
    }
    else
    {
      const Nation* nation = db()->getNation(loc->nation());

      if (!nation)
      {
        return CurrencyCode();
      }

      return nation->primeCur();
    }
  }

  return customer->defaultCur();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::getCustomerCity
//
// Description:  get customer default city.
//
// </PRE>
// ----------------------------------------------------------------------------
LocCode
PfcDisplayDataPXC::getCustomerCity() const
{
  const Agent* agent = trx()->pfcDisplayRequest()->ticketingAgent();

  if (!agent)
  {
    return LocCode();
  }

  const Customer* customer = db()->getCustomer(agent->mainTvlAgencyPCC());

  if (!customer)
  {
    return LocCode();
  }

  return customer->aaCity();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayData::getPfcPFC
//
// Description:  Get PFC records.
//
// </PRE>
// ----------------------------------------------------------------------------
void
PfcDisplayDataPXC::getPfcPFC(std::vector<PfcPFC*>& pfcV, WarningMap& warningMap) const
{
  ////////////////////////
  // PXC* entry
  ////////////////////////
  if (trx()->pfcDisplayRequest()->segments().empty())
  {
    pfcV = db()->getAllPfcPFC();
    return;
  }

  LocCode arpt;
  std::vector<PfcPFC*> pfcRawV;

  PfcDisplayRequest::Segments::const_iterator itS = trx()->pfcDisplayRequest()->segments().begin();
  PfcDisplayRequest::Segments::const_iterator itSEnd = trx()->pfcDisplayRequest()->segments().end();

  /////////////////////////////////////
  // PXC* I, PXC*Sn, PXC*Sn-Sm entries
  /////////////////////////////////////
  if (trx()->pfcDisplayRequest()->isPNR())
  {
    for (; itS < itSEnd; itS++)
    {
      arpt = std::get<PfcDisplayRequest::DEPARTURE_AIRPORT>(*itS);
      pfcRawV = db()->getPfcPFC(arpt);

      pushBackPfcData(arpt, pfcRawV, pfcV, warningMap);
    }
    return;
  }

  /////////////////////////////////////
  // PXC*CCC, PXC*CCCDDD.. entries
  /////////////////////////////////////

  std::vector<PfcCoterminal*> coterminals;
  std::vector<PfcCoterminal*>::const_iterator itPfcCoterminal;
  std::vector<PfcCoterminal*>::const_iterator itEndPfcCoterminal;

  for (; itS < itSEnd; itS++)
  {
    arpt = std::get<PfcDisplayRequest::DEPARTURE_AIRPORT>(*itS);
    pfcRawV = db()->getPfcPFC(arpt);

    PfcMultiAirport* pfcMultiAirport = const_cast<PfcMultiAirport*>(db()->getPfcMultiAirport(arpt));

    if (pfcMultiAirport)
    {
      if (pfcMultiAirport->loc().locType() != LOCTYPE_CITY)
      {
        pushBackPfcData(arpt, pfcRawV, pfcV, warningMap);
      }

      if (pfcMultiAirport->loc().locType() == LOCTYPE_AIRPORT ||
          !(pfcMultiAirport->loc().locType() == LOCTYPE_CITY ||
            pfcMultiAirport->loc().locType() == LOCTYPE_AIRPORT_AND_CITY))
      {
        continue;
      }

      coterminals = pfcMultiAirport->coterminals();
      itPfcCoterminal = coterminals.begin();
      itEndPfcCoterminal = coterminals.end();

      for (; itPfcCoterminal < itEndPfcCoterminal; itPfcCoterminal++)
      {
        arpt = (*itPfcCoterminal)->cotermLoc();
        pfcRawV = db()->getPfcPFC(arpt);

        pushBackPfcData(arpt, pfcRawV, pfcV, warningMap);
      }
    }
    else
    {
      pushBackPfcData(arpt, pfcRawV, pfcV, warningMap);
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXC::isPfcAbsorb
//
// Description:  Returns true if PFC Absorption applied for the pfcAirport.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
PfcDisplayDataPXC::isPfcAbsorb(LocCode& pfcAirport, uint32_t segmentPos) const
{
  const std::vector<PfcAbsorb*>& absorbV = db()->getAllPfcAbsorb();

  std::vector<PfcAbsorb*>::const_iterator it = absorbV.begin();
  std::vector<PfcAbsorb*>::const_iterator itEnd = absorbV.end();

  for (; it < itEnd; it++)
  {
    if ((*it)->pfcAirport() == pfcAirport)
    {
      return true;
    }
  }

  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXC::isPfcEssAirSvc
//
// Description:  Returns true if PFC ESS Air svc applied  for the pfcAirport.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
PfcDisplayDataPXC::isPfcEssAirSvc(LocCode& pfcAirport, uint32_t segmentPos) const
{
  const std::vector<PfcEssAirSvc*>& easV = db()->getAllPfcEssAirSvc();

  std::vector<PfcEssAirSvc*>::const_iterator it = easV.begin();
  std::vector<PfcEssAirSvc*>::const_iterator itEnd = easV.end();

  for (; it < itEnd; it++)
  {
    if ((*it)->easHubArpt() == pfcAirport)
    {
      return true;
    }
  }

  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXC::getEquivalentAmount
//
// Description:  Convert base USD amount to equivalent amount.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayDataPXC::getEquivalentAmount(MoneyAmount& amt) const
{
  PfcDisplayCurrencyFacade converter(trx());

  return converter.getEquivalentAmount(amt, getCustomerCurrency(), db()->date());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXC::setWarning
//
// Description:  Set warning if PFC not applicabble.
//
// </PRE>
// ----------------------------------------------------------------------------
void
PfcDisplayDataPXC::updateWarningMap(LocCode& arpt,
                                    std::vector<PfcPFC*>& pfcV,
                                    WarningMap& warningMap) const
{
  PfcPFC* pfc;
  trx()->dataHandle().get(pfc);
  pfc->pfcAirport() = arpt;
  pfcV.push_back(pfc);

  std::vector<PfcPFC*> pfcDateIndependentV = db()->getDateIndependentPfcPFC(arpt);

  if (pfcDateIndependentV.empty())
  {
    warningMap[arpt] = PfcDisplayErrorMsg::PFC_NOT_APPLICABLE;
  }
  else
  {
    warningMap[arpt] = PfcDisplayErrorMsg::DATA_NOT_FOUND;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXC::pushBackPfcData
//
// Description:  Update PFC data/warnings vector
//
// </PRE>
// ----------------------------------------------------------------------------
void
PfcDisplayDataPXC::pushBackPfcData(LocCode& arpt,
                                   std::vector<PfcPFC*>& pfcRawV,
                                   std::vector<PfcPFC*>& pfcV,
                                   WarningMap& warningMap) const
{
  if (!pfcRawV.empty())
  {
    std::copy(pfcRawV.begin(), pfcRawV.end(), back_inserter(pfcV));
  }
  else
  {
    updateWarningMap(arpt, pfcV, warningMap);
  }
}
