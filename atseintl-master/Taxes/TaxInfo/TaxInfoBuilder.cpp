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
#include <algorithm>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/lexical_cast.hpp>

#include "Common/LocUtil.h"
#include "Common/Money.h"
#include "DataModel/TaxInfoRequest.h"
#include "Taxes/TaxInfo/GetTaxRulesRecords.h"
#include "Taxes/TaxInfo/TaxInfoBuilder.h"
#include "Taxes/TaxInfo/TaxInfoBuilderPFC.h"

using namespace tse;


const std::string TaxInfoBuilder::NO_DATA_ON_FILE = "NO DATA ON FILE FOR REQUESTED DATE";
const std::string TaxInfoBuilder::MISSING_TAX_CODE = "MISSING TAX CODE";
const std::string TaxInfoBuilder::TAX_NOT_FOUND = "TAX NOT FOUND";
const std::string TaxInfoBuilder::MISSING_AIRPORTS = "MISSING AIRPORT CODES";
const std::string TaxInfoBuilder::INVALID_AIRPORT = "INVALID AIRPORT CODE";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::TaxInfoBuilder
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoBuilder::TaxInfoBuilder()
  : _taxCodeReg(nullptr), _taxCode(""), _taxType('\0'), _taxNation(""), _airports(nullptr)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::~TaxInfoBuilder
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoBuilder::~TaxInfoBuilder()
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::build
//
// Description:  Build Tax Info.
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoBuilder::build(TaxTrx& trx)
{
  if (taxCode().empty())
  {
    std::get<Response::TAX::ERROR>(response().taxAttrValues()) = TaxInfoBuilder::MISSING_TAX_CODE;
    return;
  }

  std::get<Response::TAX::CODE>(response().taxAttrValues()) = taxCode();

  initTicketingDT(trx);

  if (!validateTax(trx))
  {
    return;
  }

  if (!validateAirport(trx))
  {
    std::get<Response::TAX::ERROR>(response().taxAttrValues()) = MISSING_AIRPORTS;
    return;
  }

  initTaxDescription(trx);
  buildDetails(trx);

  if (std::get<Response::TAX::ERROR>(response().taxAttrValues()).empty())
  {
    std::get<Response::TAX::TYPE>(response().taxAttrValues()) = taxType();
    std::get<Response::TAX::NATION>(response().taxAttrValues()) = taxNation();
    std::get<Response::TAX::DESCRIPTION>(response().taxAttrValues()) = taxDescription();
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::buildDetails
//
// Description:  Build Details info.
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoBuilder::buildDetails(TaxTrx& trx)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::initTaxDescription
//
// Description:  .
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoBuilder::initTaxDescription(TaxTrx& trx) // initTaxData
{
  if (taxType() == '\0')
    taxType() = taxCodeReg()->taxType();

  if (taxDescription().empty() &&
       ( (taxCodeReg() && !taxCodeReg()->taxCodeGenTexts().empty())
      ))
    taxDescription() = taxCodeReg()->taxCodeGenTexts().front()->txtMsgs().front();

  if (taxNation().empty())
    taxNation() = taxCodeReg()->nation();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::validateTax
//
// Description:  Validate tax records.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxInfoBuilder::validateTax(TaxTrx& trx)
{
  const std::vector<TaxCodeReg*>& taxCodeReg = getTaxCodeReg(trx);

  std::vector<TaxCodeReg*>::const_iterator taxCodeRegIt = taxCodeReg.begin();
  std::vector<TaxCodeReg*>::const_iterator taxCodeRegEndIt = taxCodeReg.end();

  for (; taxCodeRegIt != taxCodeRegEndIt; taxCodeRegIt++)
  {
    if (validateTravelDate(trx, *taxCodeRegIt))
    {
      _taxCodeReg = *taxCodeRegIt;
      return true;
    }
  }

  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::validateTravelDate
//
// Description: Validate travel date.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxInfoBuilder::validateTravelDate(TaxTrx& trx, TaxCodeReg* taxCodeReg)
{
  bool isHistorical = trx.dataHandle().isHistorical();

  if (taxCodeReg->firstTvlDate().isValid())
  {
    if (!isHistorical)
    {
      if (ticketingDT() < taxCodeReg->firstTvlDate())
      {
        return false;
      }
    }
    else
    {
      if (ticketingDT().date() < taxCodeReg->firstTvlDate().date())
      {
        return false;
      }
    }
  }

  if (taxCodeReg->lastTvlDate().isValid())
  {
    if (!isHistorical)
    {
      if (ticketingDT() > taxCodeReg->lastTvlDate())
      {
        return false;
      }
    }
    else
    {
      if (ticketingDT().date() > taxCodeReg->lastTvlDate().date())
      {
        return false;
      }
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::validateAirport
//
// Description: Validate airports.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxInfoBuilder::validateAirport(TaxTrx& trx)
{
  TaxInfoRequest* req = trx.taxInfoRequest();

  std::vector<TaxInfoItem>::iterator item =
      std::find_if(req->taxItems().begin(),
                   req->taxItems().end(),
                   boost::bind(static_cast<const tse::TaxCode& (tse::TaxInfoItem::*)() const>(
                                   &TaxInfoItem::taxCode),
                               _1) == taxCode());

  if (item != req->taxItems().end() && !item->airports().empty())
    airports() = &item->airports();

  return airports();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::initTicketingDT
//
// Description:  Initialize date and time.
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoBuilder::initTicketingDT(TaxTrx& trx)
{
  TaxInfoRequest* req = trx.taxInfoRequest();

  if (req->overrideDate().isValid())
    ticketingDT() = req->overrideDate();
  else
    ticketingDT() = DateTime::localTime();

  trx.dataHandle().setTicketDate(ticketingDT());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder:::amtToString
//
// Description:  Tax amount format.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
TaxInfoBuilder::amtToString(const MoneyAmount& amount,
                            const CurrencyCode& ccode,
                            const DateTime& ticketingDT)
{
  Money moneyPayment(ccode);
  std::ostringstream formattedAmount;
  formattedAmount.setf(std::ios::fixed);
  formattedAmount.precision(moneyPayment.noDec(ticketingDT));
  formattedAmount << amount;
  return formattedAmount.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder:::percentageToString
//
// Description:  Tax amount format.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
TaxInfoBuilder::percentageToString(MoneyAmount percentage)
{
  std::ostringstream formattedPercentage;

  formattedPercentage.precision(2);
  formattedPercentage << percentage * 100;

  return formattedPercentage.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::dateToString
//
// Description:  Date format.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
TaxInfoBuilder::dateToString(const DateTime& dt)
{
  if (!dt.isValid())
  {
    return "";
  }

  return dt.dateToString(tse::YYYYMMDD, "-");
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::isAirport
//
// Description:
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TaxInfoBuilder::isAirport(const Loc* loc)
{
  return loc && loc->transtype() == 'P';
}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::getLoc
//
// Description:  DataHandle adapter for getLoc.
//
// </PRE>
// ----------------------------------------------------------------------------
const Loc*
TaxInfoBuilder::getLoc(TaxTrx& trx, const LocCode& locCode)
{
  return trx.dataHandle().getLoc(locCode, ticketingDT());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::getTaxCodeReg
//
// Description:  DataHandle adapter for geTaxCodeReg.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<TaxCodeReg*>&
TaxInfoBuilder::getTaxCodeReg(TaxTrx& trx)
{
  // TODO check AtpcoTaxOrchestrator::process() TaxInfoDriver driver(&trx);
  if (trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
    return getTaxRulesRecords(_regListStorage, trx.dataHandle(), taxCode(), ticketingDT());
  else
    return trx.dataHandle().getTaxCode(taxCode(), ticketingDT());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::getCustomer
//
// Description:  DataHandle adapter for getCustomer.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<Customer*>&
TaxInfoBuilder::getCustomer(TaxTrx& trx, const PseudoCityCode& pcc)
{
  return trx.dataHandle().getCustomer(pcc);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::getNation
//
// Description:  DataHandle adapter for getNation.
//
// </PRE>
// ----------------------------------------------------------------------------
const Nation*
TaxInfoBuilder::getNation(TaxTrx& trx, const NationCode& countryCode)
{
  return trx.dataHandle().getNation(countryCode, ticketingDT());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilder::getZone
//
// Description:  DataHandle adapter for getZone.
//
// </PRE>
// ----------------------------------------------------------------------------
const ZoneInfo*
TaxInfoBuilder::getZone(TaxTrx& trx, const Zone& zone)
{
  return trx.dataHandle().getZone(Vendor::SABRE, zone, MANUAL, ticketingDT());
}
