
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

#include <boost/bind.hpp>
#include <boost/ref.hpp>

#include "Taxes/TaxInfo/TaxInfoBuilderPFC.h"

using namespace tse;

const TaxCode TaxInfoBuilderPFC::TAX_CODE = "XF";
const std::string TaxInfoBuilderPFC::TAX_NOT_FOUND = "PFC NOT APPLICABLE";
const std::string TaxInfoBuilderPFC::TAX_DESCRIPTION = "PASSENGER FACILITY CHARGE";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderPFC::TaxInfoBuilderPFC
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoBuilderPFC::TaxInfoBuilderPFC()
  : _pfcV(nullptr)
{
  response().initPFC();
  taxCode() = TAX_CODE;
  taxType() = 'F';
  taxNation() = "US";
  taxDescription() = TAX_DESCRIPTION;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderPFC::~TaxInfoBuilderPFC
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoBuilderPFC::~TaxInfoBuilderPFC() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderPFC::buildDetails
//
// Description:  Build PFC Info.
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoBuilderPFC::buildDetails(TaxTrx& trx)
{

  std::for_each(airports()->begin(),
                airports()->end(),
                boost::bind(&TaxInfoBuilderPFC::buildItem, this, boost::ref(trx), _1));
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderPFC::buildItem
//
// Description:  Build response item.
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoBuilderPFC::buildItem(TaxTrx& trx, LocCode& airport)
{

  TaxInfoResponse::AptItem item;

  std::get<TaxInfoResponse::PFC::AIRPORT>(item) = airport;

  if (!isAirport(getLoc(trx, airport)))
  {
    std::get<TaxInfoResponse::PFC::ERROR>(item) = INVALID_AIRPORT;
  }
  else
  {
    const std::vector<PfcPFC*>& pfcV = getPfc(trx, airport);

    if (pfcV.empty())
    {
      std::get<Response::PFC::ERROR>(item) = TAX_NOT_FOUND;
    }
    else
    {
      std::get<Response::PFC::AMOUNT>(item) = amtToString(
          pfcV.front()->pfcAmt1(), pfcV.front()->pfcCur1(), trx.getRequest()->ticketingDT());
      std::get<Response::PFC::CURRENCY>(item) = pfcV.front()->pfcCur1();
      std::get<Response::PFC::EFF_DATE>(item) = dateToString(pfcV.front()->effDate());
      std::get<Response::PFC::DISC_DATE>(item) = dateToString(pfcV.front()->discDate());
    }
  }

  response().aptAttrValues().push_back(item);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderPFC::filterPfcPFC
//
// Description:  Filter PFC records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcPFC*>&
TaxInfoBuilderPFC::getPfc(TaxTrx& trx, const LocCode& airport)
{
  std::vector<PfcPFC*>* pfcAirport;

  trx.dataHandle().get(pfcAirport);

  std::remove_copy_if(
      _pfcV->begin(), _pfcV->end(), back_inserter(*pfcAirport), std::not1(IsValidAirport(airport)));

  return *pfcAirport;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderPFC::validateTax
//
// Description:  Validate tax record.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxInfoBuilderPFC::validateTax(TaxTrx& trx)
{

  trx.dataHandle().get(_pfcV);
  TaxInfoRequest* req = trx.taxInfoRequest();

  const std::vector<PfcPFC*>& rawPfcV = trx.dataHandle().getAllPfcPFC();

  std::remove_copy_if(rawPfcV.begin(),
                      rawPfcV.end(),
                      back_inserter(*_pfcV),
                      std::not1(IsValidDate(req->overrideDate())));

  return !rawPfcV.empty();
}
