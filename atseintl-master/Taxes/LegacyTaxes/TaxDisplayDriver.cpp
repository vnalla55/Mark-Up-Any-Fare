// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Taxes/LegacyTaxes/TaxDisplayDriver.h"
#include "DBAccess/TaxReissue.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"
#include "DataModel/Trx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TaxRequest.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayCAT.h"
#include "DBAccess/TaxNation.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/Itin.h"
#include "Taxes/LegacyTaxes/TaxPercentageUS.h"
#include "DataModel/Billing.h"

#include <algorithm>

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
TaxDisplayDriver::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxDisplayDriver"));

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxDisplayDriver::TaxDisplayDriver() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxDisplayDriver::~TaxDisplayDriver() {}

// ----------------------------------------------------------------------------
// Description:  Tax Display Driver
// ----------------------------------------------------------------------------

bool
TaxDisplayDriver::buildTaxDisplay(TaxTrx& taxTrx)
{
  LOG4CXX_INFO(_logger, "Started Build Tax DisplayDriver Process()");
  taxTrx.billing()->updateServiceNames(Billing::SVC_TAX);

  if (taxTrx.getRequest()->help())
    return true;

  if (!setUp(taxTrx))
    return true;

  // needed here probably because in future havaian shoudl support historical also?
  if (taxTrx.getRequest()->effectiveDate() != DateTime::emptyDate())
  {
    taxTrx.getRequest()->ticketingDT() = taxTrx.getRequest()->effectiveDate();
  }
  else if (taxTrx.getRequest()->travelDate() != DateTime::emptyDate())
  {
    taxTrx.getRequest()->ticketingDT() = taxTrx.getRequest()->travelDate();
  }

  taxTrx.dataHandle().setTicketDate(taxTrx.getRequest()->ticketingDT());

  if (TaxPercentageUS::validHiAkConUSTrip(taxTrx.getRequest()) ||
      TaxPercentageUS::validConUSTrip(taxTrx.getRequest()))
    return true;

  if (!taxTrx.getRequest()->nation().empty())
  {
    TaxDisplayList taxDisplayList;
    taxDisplayList.buildList(taxTrx);
    return true;
  }

  if (!buildTaxCodeInfo(taxTrx))
    return true;

  TaxDisplayCAT taxDisplayCAT;
  taxDisplayCAT.buildCats(taxTrx);

  return true;
}

// ----------------------------------------------------------------------------
// Description:  Display SetUp
// ----------------------------------------------------------------------------

bool
TaxDisplayDriver::setUp(TaxTrx& taxTrx)
{
  LOG4CXX_INFO(_logger, "Started Set Up Process()");

  TaxResponse* taxResponse = nullptr;
  taxTrx.dataHandle().get(taxResponse);
  Itin* itin = nullptr;
  taxTrx.dataHandle().get(itin);
  taxTrx.itin().push_back(itin);
  taxTrx.itin().front()->mutableTaxResponses().push_back(taxResponse);

  return true;
}

// ----------------------------------------------------------------------------
// Description:  Display buildTaxCodeInfo
// ----------------------------------------------------------------------------

bool
TaxDisplayDriver::buildTaxCodeInfo(TaxTrx& taxTrx)
{
  LOG4CXX_INFO(_logger, "Started Build Driver Process()");

  TaxDisplayItem* taxDisplayItem = nullptr;
  taxTrx.dataHandle().get(taxDisplayItem);

  const std::vector<TaxCodeReg*>& taxCodeReg = taxTrx.dataHandle().getTaxCode(
      taxTrx.getRequest()->taxCode(), taxTrx.getRequest()->ticketingDT());

  if (taxCodeReg.empty())
  {

    if ((taxTrx.getRequest()->effectiveDate() != DateTime::emptyDate() &&
         taxTrx.getRequest()->travelDate() != DateTime::emptyDate()) ||
        (taxTrx.getRequest()->effectiveDate() != DateTime::emptyDate() &&
         taxTrx.getRequest()->travelDate() == DateTime::emptyDate()))
    {

      taxDisplayItem->errorMsg() = "NO DATA ON FILE FOR REQUESTED TAX CODE - DATE COMBINATION";
      taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
          taxDisplayItem);

      LOG4CXX_INFO(_logger,
                   "No data on file for requested Tax Code : " << taxTrx.getRequest()->taxCode());
      return false;
    }

    taxDisplayItem->errorMsg() = "TAX NOT FOUND - MODIFY AND REENTER OR REFER TO TXNHELP";
    taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
        taxDisplayItem);

    LOG4CXX_INFO(_logger,
                 "No Tax Code Table Located for Tax Code : " << taxTrx.getRequest()->taxCode());
    return false;
  }

  const TaxNation* taxNation = taxTrx.dataHandle().getTaxNation(taxCodeReg.front()->nation(),
                                                                taxTrx.getRequest()->ticketingDT());

  if (taxNation == nullptr)
  {
    taxDisplayItem->errorMsg() = "NATION NOT FOUND - MODIFY AND REENTER OR REFER TO TXNHELP";
    taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
        taxDisplayItem);

    LOG4CXX_INFO(_logger, "No Nation Table Located for Nation : " << taxCodeReg.front()->nation());
    return false;
  }

  if (!taxCodeReg.empty() && taxTrx.getRequest()->getReissue())
  {

    const std::vector<TaxReissue*>& taxReissueVec = taxTrx.dataHandle().getTaxReissue(
        taxCodeReg.front()->taxCode(), taxTrx.getRequest()->ticketingDT());

    std::vector<TaxReissue*>::const_iterator reissueI = taxReissueVec.begin();
    std::vector<TaxReissue*>::const_iterator reissueEndI = taxReissueVec.end();

    if (taxTrx.getRequest()->sequenceNumber() == TaxRequest::EMPTY_SEQUENCE)
    {
      if (!taxReissueVec.empty())
      {
        std::vector<TaxCodeReg*>::const_iterator taxCodeRegIter = taxCodeReg.begin();
        for (; reissueI != reissueEndI; reissueI++)
        {
          TaxDisplayItem* taxDisplayItem = nullptr;
          taxTrx.dataHandle().get(taxDisplayItem);
          taxDisplayItem->buildTaxDisplayItem(taxTrx, **taxCodeRegIter, *taxNation);
          taxDisplayItem->taxReissue() = *reissueI;
          taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
              taxDisplayItem);
        }
        return true;
      }
      taxDisplayItem->errorMsg() =
          "NO REISSUE DATA ON FILE FOR REQUESTED TAX CODE - DATE COMBINATION";
      taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
          taxDisplayItem);
      LOG4CXX_INFO(
          _logger,
          "No reissue data on file for requested Tax Code : " << taxTrx.getRequest()->taxCode());
      return false;
    }
    else
    {
      reissueI = taxReissueVec.begin();
      for (; reissueI != reissueEndI; reissueI++)
      {
        if (static_cast<uint32_t>((*reissueI)->seqNo()) == taxTrx.getRequest()->sequenceNumber())
        {
          TaxDisplayItem* taxDisplayItem = nullptr;
          taxTrx.dataHandle().get(taxDisplayItem);
          std::vector<TaxCodeReg*>::const_iterator taxCodeRegIter = taxCodeReg.begin();
          taxDisplayItem->buildTaxDisplayItem(taxTrx, **taxCodeRegIter, *taxNation);
          taxDisplayItem->taxReissue() = *reissueI;
          taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
              taxDisplayItem);
          return true;
        }
      }
    }
    taxDisplayItem->errorMsg() =
        "REISSUE SEQUENCE NOT FOUND - MODIFY AND REENTER OR REFER TO TXNHELP";
    taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
        taxDisplayItem);

    LOG4CXX_INFO(_logger,
                 "Reissue sequence not found for Tax Code : " << taxTrx.getRequest()->taxCode());
    return false;
  }
  // reissue end

  if (!taxCodeReg.empty() && taxTrx.getRequest()->sequenceNumber() != TaxRequest::EMPTY_SEQUENCE)
  {
    std::vector<TaxCodeReg*>::const_iterator taxCodeRegIter = taxCodeReg.begin();
    std::vector<TaxCodeReg*>::const_iterator taxCodeRegEndIter = taxCodeReg.end();

    bool isSeqenceNumber = false;

    for (; taxCodeRegIter != taxCodeRegEndIter; taxCodeRegIter++)
    {
      if (taxTrx.getRequest()->sequenceNumber() ==
          static_cast<uint32_t>((*taxCodeRegIter)->seqNo()))
      {
        isSeqenceNumber = true;
        break;
      }
    }
    if (!isSeqenceNumber)
    {
      taxDisplayItem->errorMsg() = "SEQUENCE NOT FOUND - MODIFY AND REENTER OR REFER TO TXNHELP";
      taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
          taxDisplayItem);
      LOG4CXX_INFO(_logger, "Sequence not found for Tax Code : " << taxTrx.getRequest()->taxCode());
      return false;
    }
  }

  if (!taxCodeReg.empty() && taxTrx.getRequest()->sequenceNumber2() != TaxRequest::EMPTY_SEQUENCE)
  {
    std::vector<TaxCodeReg*>::const_iterator taxCodeRegIter = taxCodeReg.begin();
    std::vector<TaxCodeReg*>::const_iterator taxCodeRegEndIter = taxCodeReg.end();

    bool isSeqenceNumber2 = false;

    for (; taxCodeRegIter != taxCodeRegEndIter; taxCodeRegIter++)
    {
      if (taxTrx.getRequest()->sequenceNumber2() ==
          static_cast<uint32_t>((*taxCodeRegIter)->seqNo()))
      {
        isSeqenceNumber2 = true;
        break;
      }
    }
    if (!isSeqenceNumber2)
    {

      taxDisplayItem->errorMsg() = "SEQUENCE NOT FOUND - MODIFY AND REENTER OR REFER TO TXNHELP";
      taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
          taxDisplayItem);
      LOG4CXX_INFO(_logger, "Sequence not found for Tax Code : " << taxTrx.getRequest()->taxCode());
      return false;
    }
  }

  if (!taxTrx.getRequest()->categoryVec().empty())
  {
    std::sort(taxTrx.getRequest()->categoryVec().begin(), taxTrx.getRequest()->categoryVec().end());

    if (taxTrx.getRequest()->categoryVec().front() < 1 ||
        taxTrx.getRequest()->categoryVec().back() > 18)
    {
      taxDisplayItem->errorMsg() =
          "INVALID CATEGORY 01-18 ONLY - MODIFY AND REENTER - REFER TO TXNHELP";
      taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
          taxDisplayItem);
      LOG4CXX_INFO(_logger, "Categories not exist.");
      return false;
    }
  }

  std::vector<TaxCodeReg*>::const_iterator taxCodeRegIter = taxCodeReg.begin();
  std::vector<TaxCodeReg*>::const_iterator taxCodeRegEndIter = taxCodeReg.end();

  bool rejected = false;
  for (; taxCodeRegIter != taxCodeRegEndIter; taxCodeRegIter++)
  {
    if (taxTrx.getRequest()->sequenceNumber() != TaxRequest::EMPTY_SEQUENCE)
    {
      if (taxTrx.getRequest()->sequenceNumber() !=
          static_cast<uint32_t>((*taxCodeRegIter)->seqNo()))
      {
        if (taxTrx.getRequest()->sequenceNumber2() !=
            static_cast<uint32_t>((*taxCodeRegIter)->seqNo()))
          continue;
      }
    }

    if (taxTrx.getRequest()->travelDate().isValid())
    {
      if (taxTrx.getRequest()->travelDate() > (*taxCodeRegIter)->discDate())
        continue;

      if ((*taxCodeRegIter)->firstTvlDate().isValid())
      {
        if (taxTrx.getRequest()->travelDate() < (*taxCodeRegIter)->firstTvlDate())
        {
          rejected = true;
          continue;
        }
      }

      if ((*taxCodeRegIter)->lastTvlDate().isValid())
      {
        if (taxTrx.getRequest()->travelDate() > (*taxCodeRegIter)->lastTvlDate())
        {
          rejected = true;
          continue;
        }
      }
    }

    TaxDisplayItem* taxDisplayItem = nullptr;
    taxTrx.dataHandle().get(taxDisplayItem);
    taxDisplayItem->buildTaxDisplayItem(taxTrx, **taxCodeRegIter, *taxNation);
    taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
        taxDisplayItem);
    rejected = false;
  }

  if (rejected)
  {
    TaxDisplayItem* taxDisplayItem = nullptr;
    taxTrx.dataHandle().get(taxDisplayItem);
    taxDisplayItem->errorMsg() = "NO DATA ON FILE FOR REQUESTED TAX CODE - DATE COMBINATION";
    taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
        taxDisplayItem);
    return false;
  }

  if (taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().empty())
    return false;

  return true;
}
