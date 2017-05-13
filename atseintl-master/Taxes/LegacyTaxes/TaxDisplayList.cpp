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

#include "Taxes/LegacyTaxes/TaxDisplayList.h"
#include "DataModel/Trx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "DBAccess/TaxNation.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include <algorithm>

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
TaxDisplayList::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxDisplayList"));

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxDisplayList::TaxDisplayList() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxDisplayList::~TaxDisplayList() {}

// ----------------------------------------------------------------------------
// Description:  Display buildList
// ----------------------------------------------------------------------------

void
TaxDisplayList::buildList(TaxTrx& taxTrx)
{
  LOG4CXX_INFO(_logger, "Started Build List Process()");

  const TaxNation* taxNation = nullptr;
  const TaxNation* taxNation2 = nullptr;
  const TaxNation* taxNation3 = nullptr;

  if (taxTrx.getRequest()->nation().size() < 3)
  {
    taxNation = taxTrx.dataHandle().getTaxNation(taxTrx.getRequest()->nation(),
                                                 taxTrx.getRequest()->ticketingDT());
  }

  if (taxNation == nullptr)
  {
    if (taxTrx.getRequest()->nationDescription().size() < 4)
    {
      const Loc* agentLocation =
          taxTrx.dataHandle().getLoc(taxTrx.getRequest()->nationDescription(), time(nullptr));

      if (agentLocation != nullptr)
      {
        taxNation = taxTrx.dataHandle().getTaxNation(agentLocation->nation(),
                                                     taxTrx.getRequest()->ticketingDT());
      }
    }

    if (taxNation == nullptr)
    {
      const std::vector<Nation*> allNationList =
          taxTrx.dataHandle().getAllNation(taxTrx.getRequest()->ticketingDT());

      std::vector<Nation*>::const_iterator nationsListIter = allNationList.begin();
      std::vector<Nation*>::const_iterator nationsListEnd = allNationList.end();

      if (allNationList.empty())
      {
        LOG4CXX_INFO(_logger,
                     "No All Nation Table Located: " << taxTrx.getRequest()->nationDescription());
        return;
      }

      for (; nationsListIter != nationsListEnd; nationsListIter++)
      {
        if (taxTrx.getRequest()->nationDescription() != (*nationsListIter)->description())
          continue;

        taxNation = taxTrx.dataHandle().getTaxNation((*nationsListIter)->nation(),
                                                     taxTrx.getRequest()->ticketingDT());
        break;
      }

      if (taxNation == nullptr)
      {
        if (storeTaxNationError(taxTrx, allNationList))
          return;
      }
    }
  }

  if (taxNation->taxCodeOrder().empty() && taxNation->collectionNation1().empty())
  {
    LOG4CXX_INFO(_logger, "No Tax Codes for Nation : " << taxTrx.getRequest()->nation());
    return;
  }

  storeTaxDetailInfo(taxTrx, taxNation);

  if (!taxNation->collectionNation1().empty())
  {
    taxNation2 = taxTrx.dataHandle().getTaxNation(taxNation->collectionNation1(),
                                                  taxTrx.getRequest()->ticketingDT());

    if (taxNation2 != nullptr)
    {
      if (!taxNation2->taxCodeOrder().empty())
        storeTaxDetailInfo(taxTrx, taxNation2);
    }
  }

  if (!taxNation->collectionNation2().empty())
  {
    taxNation3 = taxTrx.dataHandle().getTaxNation(taxNation->collectionNation2(),
                                                  taxTrx.getRequest()->ticketingDT());

    if (taxNation3 != nullptr)
    {
      if (!taxNation3->taxCodeOrder().empty())
        storeTaxDetailInfo(taxTrx, taxNation3);
    }
  }
}

// ----------------------------------------------------------------------------
// Description:  Add error when whrong nation is read from request
// ----------------------------------------------------------------------------

bool
TaxDisplayList::storeTaxNationError(TaxTrx& taxTrx, const std::vector<Nation*>& allNationList)
{
  TaxDisplayItem* taxDisplayItem = nullptr;
  taxTrx.dataHandle().get(taxDisplayItem);

  if (taxDisplayItem == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");

    return false;
  }

  taxDisplayItem->errorMsg() = "UNABLE TO IDENTIFY COUNTRY NAME - MODIFY AND REENTER\n";

  if (allNationList.empty())
  {
    LOG4CXX_INFO(_logger, "No All Nation Table Located");
    taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
        taxDisplayItem);

    return true;
  }

  std::string nationFirstChar = taxTrx.getRequest()->nation().substr(0, 1);
  std::vector<std::string> matchedNations;

  std::vector<std::string> matchedNationCodes;
  std::vector<std::string> matchedNationDescriptions;

  std::vector<Nation*>::const_iterator nationsListIter = allNationList.begin();
  std::vector<Nation*>::const_iterator nationsListEnd = allNationList.end();

  for (; nationsListIter != nationsListEnd; nationsListIter++)
  {
    if ((*nationsListIter)->description().substr(0, 1) == nationFirstChar)
    {
      matchedNations.push_back((*nationsListIter)->nation() + std::string(2, ' ') +
                               (*nationsListIter)->description());

      matchedNationCodes.push_back((*nationsListIter)->nation());
      matchedNationDescriptions.push_back((*nationsListIter)->description());
    }
  }

  if (matchedNations.empty() ||
      (std::find(matchedNationCodes.begin(),
                 matchedNationCodes.end(),
                 taxTrx.getRequest()->nation()) != matchedNationCodes.end() ||
       std::find(matchedNationDescriptions.begin(),
                 matchedNationDescriptions.end(),
                 taxTrx.getRequest()->nation()) != matchedNationDescriptions.end()))
  {
    if ((taxTrx.getRequest()->effectiveDate() != DateTime::emptyDate() &&
         taxTrx.getRequest()->travelDate() != DateTime::emptyDate()) ||
        (taxTrx.getRequest()->effectiveDate() != DateTime::emptyDate() &&
         taxTrx.getRequest()->travelDate() == DateTime::emptyDate()))
    {

      taxDisplayItem->errorMsg() = "NO DATA ON FILE FOR REQUESTED NATION - DATE COMBINATION";

      LOG4CXX_INFO(_logger,
                   "No data on file for requested Nation : " << taxTrx.getRequest()->nation());
    }
  }
  else if (!matchedNations.empty())
  {
    std::vector<std::string>::const_iterator matchedNationsListIter = matchedNations.begin();
    std::vector<std::string>::const_iterator matchedNationsListIterEnd = matchedNations.end();

    for (; matchedNationsListIter != matchedNationsListIterEnd; matchedNationsListIter++)
    {
      taxDisplayItem->errorMsg() += *matchedNationsListIter + "\n";
    }
  }

  taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
      taxDisplayItem);

  return true;
}
// ----------------------------------------------------------------------------
// Description:  storeTaxDetailInfo
// ----------------------------------------------------------------------------

void
TaxDisplayList::storeTaxDetailInfo(TaxTrx& taxTrx, const TaxNation* taxNation)
{
  std::vector<TaxCode>::const_iterator taxCodeIter = taxNation->taxCodeOrder().begin();
  std::vector<TaxCode>::const_iterator taxCodeEndIter = taxNation->taxCodeOrder().end();

  for (; taxCodeIter != taxCodeEndIter; taxCodeIter++)
  {
    TaxDisplayItem* taxDisplayItem = nullptr;
    taxTrx.dataHandle().get(taxDisplayItem);

    if (taxDisplayItem == nullptr)
    {
      LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
      return;
    }

    const std::vector<TaxCodeReg*>& taxCodeReg =
        taxTrx.dataHandle().getTaxCode(*taxCodeIter, taxTrx.getRequest()->ticketingDT());

    if (taxCodeReg.empty())
      continue;

    taxDisplayItem->buildTaxDisplayItem(taxTrx, *taxCodeReg.front(), *taxNation);

    taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().push_back(
        taxDisplayItem);
  }
}
