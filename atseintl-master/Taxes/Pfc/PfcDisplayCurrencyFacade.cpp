

// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordatance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/Pfc/PfcDisplayCurrencyFacade.h"

#include "Common/TrxUtil.h"
#include "Currency/LocalCurrencyDisplay.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/CurrencyTrx.h"
#include <boost/lexical_cast.hpp>

using namespace tse;

const std::string
PfcDisplayCurrencyFacade::FARES_APPL("FARES");

const std::string
PfcDisplayCurrencyFacade::TAX_APPL("TAXES");

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayCurrencyFacade::PfcDisplayCurrencyFacade
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayCurrencyFacade::PfcDisplayCurrencyFacade(const TaxTrx* trx) : _trx(trx) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayCurrencyFacade::~PfcDisplayCurrencyFacade
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayCurrencyFacade::~PfcDisplayCurrencyFacade() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayCurrencyFacade::getEquivalentAmount
//
// Description:  Get equivalent amount.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayCurrencyFacade::getEquivalentAmount(const MoneyAmount amt,
                                              const std::string& targetCurrency,
                                              const DateTime& dateTime) const
{
  std::string equivAmt;

  if (targetCurrency == US_DOLLARS || targetCurrency == EMPTY_STRING())
  {
    return equivAmt;
  }

  PricingRequest request;
  CurrencyTrx currencyTrx;
  currencyTrx.setRequest(&request);

  currencyTrx.commandType() = 'D';
  currencyTrx.eprBDK() = 'T';
  currencyTrx.reciprocal() = 'T';

  currencyTrx.sourceCurrency() = US_DOLLARS;

  currencyTrx.amount() = amt;
  currencyTrx.targetCurrency() = targetCurrency;
  currencyTrx.baseDT() = dateTime.date();
  currencyTrx.pssLocalDate() = DateTime::localTime().date();

  const DateTime dcDate = !currencyTrx.baseDT().isInfinity() ? currencyTrx.baseDT() : currencyTrx.pssLocalDate();
  if (TrxUtil::isIcerActivated(currencyTrx, dcDate)) //Remove BSRDSP keyword logic
  {
    // Keep the same logic as when currencyTrx.eprBDK() = 'T' && currencyTrx.reciprocal() = 'T'
    currencyTrx.reciprocal() = 'F';
  }

  Agent agent;
  agent.tvlAgencyPCC() = trx()->pfcDisplayRequest()->ticketingAgent()->mainTvlAgencyPCC();
  request.ticketingAgent() = &agent;

  request.ticketingAgent()->agentCity() = trx()->pfcDisplayRequest()->ticketingAgent()->agentCity();

  const Loc* loc = trx()->dataHandle().getLoc(
      trx()->pfcDisplayRequest()->ticketingAgent()->agentCity(), currencyTrx.pssLocalDate());

  request.ticketingAgent()->agentLocation() = loc;

  LocalCurrencyDisplay lsd(currencyTrx);

  if (!lsd.convert())
  {
    return equivAmt;
  }

  std::string dc = currencyTrx.response().str();
  // std::cout << "DC:" << dc << std::endl;

  size_t pos1 = dc.rfind(FARES_APPL);
  size_t pos2 = dc.rfind(TAX_APPL, pos1);

  std::string line = dc.substr(pos1 + FARES_APPL.size() + 1, pos2 - pos1 - FARES_APPL.size() - 1);

  std::string currency = line.substr(0, 3);

  equivAmt = line.substr(ROUND_NOTE_POS, line.find(" ", ROUND_NOTE_POS) - ROUND_NOTE_POS);

  try { boost::lexical_cast<MoneyAmount>(equivAmt); }
  catch (boost::bad_lexical_cast& e)
  {
    return std::string(); // PFC equivalent amount not available
  }

  return equivAmt;
}
