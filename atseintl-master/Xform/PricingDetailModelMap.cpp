//----------------------------------------------------------------------------
//
//  File:  PricingDetailModelMap.cpp
//  Description: See PricingDetailModelMap.h file
//  Created:  May 17, 2005
//  Authors:  Andrea Yang
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Xform/PricingDetailModelMap.h"

#include "Common/Config/ConfigMan.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/CurrencyDetail.h"
#include "DataModel/DifferentialDetail.h"
#include "DataModel/FareCalcDetail.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/PaxDetail.h"
#include "DataModel/PlusUpDetail.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/SegmentDetail.h"
#include "DataModel/SurchargeDetail.h"
#include "DataModel/SellingFareData.h"
#include "DataModel/TaxBreakdown.h"
#include "DataModel/TaxDetail.h"
#include "DataModel/Trx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MultiTransport.h"

#include <set>
#include <string>
#include <vector>

#include <time.h>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackWpdfEnhancedRuleDisplay);
FALLBACK_DECL(purgeBookingCodeOfNonAlpha);

static Logger
logger("atseintl.Xform.PricingDetailModelMap");

PricingDetailModelMap::~PricingDetailModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

bool
PricingDetailModelMap::createMap()
{
  using NV = ConfigMan::NameValue;
  using NVVector = std::vector<NV>;
  using NVVectorIter = std::vector<NV>::const_iterator;

  NVVector classVector;
  NVVector memberVector;

  if (_config.getValues(classVector, "Classes") == false)
  {
    LOG4CXX_ERROR(logger, "Cannot find config section classes");
    return false;
  }

  NVVectorIter iter = classVector.begin();
  if (iter == classVector.end())
  {
    LOG4CXX_ERROR(logger, "No Classes information in configuration file");
    return false;
  }
  else
  {
    for (; iter != classVector.end(); ++iter)
    {
      NV entry = *iter;
      std::string tagName = entry.name;
      // std::string tagValue = entry.value;
      Mapping* mapping = new Mapping();

      LOG4CXX_INFO(logger, "Tag name: " << tagName);

      // Recognized class tags
      if (tagName == "DTL")
      {
        mapping->func = &tse::PricingDetailModelMap::storePricingDetailInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::savePricingDetailInformation;
      }
      else if (tagName == "PRICINGRESPONSE" || tagName == "OCM" || tagName == "OCH" ||
               tagName == "OCG" || tagName == "OCP" || tagName == "OCT" || tagName == "OBF" ||
               tagName == "FRT" || tagName == "OCF")
      {
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "UPSELLRESPONSE")
      {
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "AGI")
      {
        mapping->func = &tse::PricingDetailModelMap::storeAgentInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveAgentInformation;
      }
      else if (tagName == "BIL")
      {
        mapping->func = &tse::PricingDetailModelMap::storeBillingInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveBillingInformation;
      }
      else if (tagName == "SUM")
      {
        mapping->func = &tse::PricingDetailModelMap::storeSummaryInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveSummaryInformation;
      }
      else if (tagName == "PXI")
      {
        mapping->func = &tse::PricingDetailModelMap::storePassengerInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::savePassengerInformation;
      }
      else if (tagName == "TAX")
      {
        mapping->func = &tse::PricingDetailModelMap::storeTaxInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveTaxInformation;
      }
      else if (tagName == "TBD")
      {
        mapping->func = &tse::PricingDetailModelMap::storeTaxBreakdown;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveTaxBreakdown;
      }
      else if (tagName == "CCD")
      {
        mapping->func = &tse::PricingDetailModelMap::storeCurrencyConversion;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveCurrencyConversion;
      }
      else if (tagName == "FIR")
      {
        mapping->func = &tse::PricingDetailModelMap::storeFareIATARate;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveFareIATARate;
      }
      else if (tagName == "P45")
      {
        mapping->func = &tse::PricingDetailModelMap::storeFareBankerSellRate;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveFareBankerSellRate;
      }
      else if (tagName == "TBR")
      {
        mapping->func = &tse::PricingDetailModelMap::storeTaxBankerSellRate;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveTaxBankerSellRate;
      }
      else if (tagName == "PUP")
      {
        mapping->func = &tse::PricingDetailModelMap::storePlusUp;
        mapping->trxFunc = &tse::PricingDetailModelMap::savePlusUp;
      }
      else if (tagName == "CAL")
      {
        mapping->func = &tse::PricingDetailModelMap::storeFareCalcInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveFareCalcInformation;
      }
      else if (tagName == "ABL")
      {
        // confirm with Alexander Z if the abacus billing data need to be returned
        // on WPn or WPDF entry. If yes the more code to save that data need to be added here
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "SUR")
      {
        mapping->func = &tse::PricingDetailModelMap::storeSurchargeInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveSurchargeInformation;
      }
      else if (tagName == "CAT")
      {
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "SEG")
      {
        mapping->func = &tse::PricingDetailModelMap::storeSegmentInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveSegmentInformation;
      }
      else if (tagName == "HIP" || tagName == "ZDF")
      {
        mapping->func = &tse::PricingDetailModelMap::storeHIPInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveHIPInformation;
      }
      else if (tagName == "MSG")
      {
        mapping->func = &tse::PricingDetailModelMap::storeMessageInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveMessageInformation;
      }
      else if (tagName == "NET")
      {
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "MCH")
      {
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "PVD")
      {
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "DYN")
      {
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "TXT")
      {
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "ERD" || tagName == "ECN" || tagName == "C25" || tagName == "C35" ||
               tagName == "DFI" || tagName == "OAO" || tagName == "DAO" || tagName == "BDI" ||
               tagName == "Q00")
      {
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if ( tagName == "VCL" || tagName == "DCX" || tagName == "ACX" || tagName == "PCX" ||
                tagName == "P3L" || tagName == "SM0" || tagName == "VC0" || tagName == "MNV" )
      {
         mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
         mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "ITR" || tagName == "PFF" )
      {
        mapping->func = &tse::PricingDetailModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::PricingDetailModelMap::nullSaveFunction;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::PricingDetailModelMap::storeDynamicConfigOverride;
        mapping->trxFunc = nullptr;
      }
      else if (tagName == "SFD")
      {
        mapping->func = &tse::PricingDetailModelMap::storeSellingFareInformation;
        mapping->trxFunc = &tse::PricingDetailModelMap::saveSellingFareInformation;
      }
      else
      {
        LOG4CXX_ERROR(logger, "Function for class name tag " << tagName << ", not mapped");
        delete mapping;
        return false;
      }

      // Get the members
      std::string memberTag = tagName + "_MEMBERS";
      if (_config.getValues(memberVector, memberTag) == false)
      {
        LOG4CXX_ERROR(logger,
                      "Cannot find config section " << memberTag << " associated with class tag "
                                                    << tagName);
        delete mapping;
        return false;
      }

      // Set the members
      for (NVVectorIter memIter = memberVector.begin(); memIter != memberVector.end(); ++memIter)
      {
        NV memEntry = *memIter;
        std::string memberName = memEntry.name;
        // std::string memberPosition = memEntry.value;
        mapping->members[SDBMHash(memberName)] = atoi(memEntry.value.c_str());
        LOG4CXX_DEBUG(logger,
                      "Created hash (" << SDBMHash(memberName) << ") to member <" << memberName
                                       << "> mapping for tag <" << tagName << ">");
      }

      // Set the map
      _classMap[SDBMHash(tagName)] = mapping;
      LOG4CXX_DEBUG(logger,
                    "Stored mapping for tag <" << tagName << "> under hash (" << SDBMHash(tagName)
                                               << ")");
    }
  }
  return true;
}

bool
PricingDetailModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
{
  if (tagName == "NET")
  {
    LOG4CXX_DEBUG(logger, ">>>>> START NET");
    _isInNetRemit = true;
  }

  if (tagName == "SFD")
    _isInAsl = true;

  unsigned int hashedTagName = SDBMHash(tagName);

  LOG4CXX_DEBUG(logger,
                "Beginning mapping for tag <" << tagName << ">, hashCode = " << hashedTagName);

  // Find the mapping
  Mapping* mapping = (Mapping*)_classMap[hashedTagName];
  if (mapping && mapping->func)
  {
    _currentMapEntry = (void*)mapping;
    _currentMapEntryStack.push(_currentMapEntry);

    if (!_ignoreNetRemit || !_isInNetRemit)
    {
      if (!fallback::fixed::fallbackWpdfEnhancedRuleDisplay())
        (this->*mapping->func)(atts);
      else if (!_isInAsl)
        (this->*mapping->func)(atts);
    }
  }
  else
  {
    LOG4CXX_WARN(logger, "Tag name: " << tagName << ", not mapped!");
    return false;
  }
  return true;
}

void
PricingDetailModelMap::saveMapEntry(std::string& tagName)
{
  unsigned int hashedTagName = SDBMHash(tagName);

  // Find the mapping
  Mapping* mapping = (Mapping*)_classMap[hashedTagName];

  // Some mappings don't interact directly with a Trx object
  LOG4CXX_DEBUG(logger, "tag=" << tagName << ", mapping=" << mapping);

  if (mapping && mapping->trxFunc)
  {
    if (!_ignoreNetRemit || !_isInNetRemit)
    {
      if (!fallback::fixed::fallbackWpdfEnhancedRuleDisplay())
        (this->*mapping->trxFunc)();
      else if (!_isInAsl)
        (this->*mapping->trxFunc)();
    }
    _currentMapEntryStack.pop();
    if (_currentMapEntryStack.size() > 0)
    {
      _currentMapEntry = _currentMapEntryStack.top();
    }
  }
  if (tagName == "NET")
  {
    _isInNetRemit = false;
    LOG4CXX_DEBUG(logger, ">>>>> END NET");
  }

  if (tagName == "SFD")
    _isInAsl = false;
}

void
PricingDetailModelMap::storePricingDetailInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePricingDetailInformation");

  // Get a Pricing Detail Trx
  if (_pricingDetailTrx == nullptr)
  {
    _dataHandle.get(_pricingDetailTrx);
    _trx = _pricingDetailTrx;
    _pricingDetailTrx->selectionChoice() = 0;

    PricingRequest* request;
    _pricingDetailTrx->dataHandle().get(request);
    _pricingDetailTrx->setRequest(request);

    _pricingDetailTrx->getRequest()->ticketingAgent() = &_pricingDetailTrx->ticketingAgent();
  }
  LOG4CXX_DEBUG(logger, "Finished in storePricingDetailInformation");
}

void
PricingDetailModelMap::savePricingDetailInformation()
{
  LOG4CXX_DEBUG(logger, "In savePricingDetailInformation");
}

void
PricingDetailModelMap::nullStoreFunction(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In nullStoreFunction");

  LOG4CXX_DEBUG(logger, "Finished in nullStoreFunction");
}

void
PricingDetailModelMap::nullSaveFunction()
{
  LOG4CXX_DEBUG(logger, "In nullSaveFunction");
}

void
PricingDetailModelMap::storeBillingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeBillingInformation");

  Billing& billing = *_pricingDetailTrx->billing();
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A20 - User pseudoCity code
      billing.userPseudoCityCode() = xmlValue.c_str();
      break;
    case 2: // Q03 - User station
      billing.userStation() = xmlValue.c_str();
      break;
    case 3: // Q02 - User branch
      billing.userBranch() = xmlValue.c_str();
      break;
    case 4: // AE0 - Partition ID
      billing.partitionID() = xmlValue.c_str();
      break;
    case 5: // AD0 - User set address
      billing.userSetAddress() = xmlValue.c_str();
      break;
    case 6: // C20 - Parent service name
      billing.parentServiceName() = xmlValue.c_str();
      break;
    case 7: // A22 - aaaCity
      billing.aaaCity() = xmlValue.c_str();
      break;
    case 8: // AA0 - agentSine
      billing.aaaSine() = xmlValue.c_str();
      break;
    case 9: // A70 - action code
      billing.actionCode() = xmlValue.c_str();
      break;
    case 10: // C01
      billing.clientTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    case 11: // C00
      billing.parentTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    case 12: // C21 - Not parse it
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeBillingInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }

  if (_wpdfType != WPDF_NONE)
  {
    billing.serviceName() = "INTLWPDF";

    if (billing.parentServiceName() == "INTLWQPR")
      billing.parentServiceName() = "INTLWPIQ";
    else if (_wpdfType != WPDF_AFTER_WP)
      billing.parentServiceName() = "INTLWPIA";
  }
  else if (billing.parentServiceName() == "INTLWQPR")
    billing.serviceName() = "INTLWPIQ";
  else
    billing.serviceName() = "INTLWPIA";

  billing.updateTransactionIds(_pricingDetailTrx->transactionId());
  billing.updateServiceNames(Billing::SVC_PRICING_DETAIL);

  LOG4CXX_DEBUG(logger, "Finished in storeBillingInformation");
}

void
PricingDetailModelMap::saveBillingInformation()
{
  LOG4CXX_DEBUG(logger, "In saveBillingInformation");
}

void
PricingDetailModelMap::storeSummaryInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeSummaryInformation");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // C56 - Total price all
      _pricingDetailTrx->totalPriceAll() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 2: // Q07 - Total number decimals
      _pricingDetailTrx->totalNumberDecimals() = atoi(xmlValue.c_str());
      ;
      break;
    case 3: // C40 - Total currency code
      _pricingDetailTrx->totalCurrencyCode() = xmlValue.c_str();
      break;
    case 4: // B00 - Validating carrier
      _pricingDetailTrx->validatingCarrier() = xmlValue.c_str();
      break;
    case 5: // D13 - Last sales date
      break;
    case 6: // D30 - Last sales time
      break;
    case 7: // D14 - Advanced purchase date
      _pricingDetailTrx->advancePurchaseDate() = convertDate(xmlValue.c_str());
      break;
    case 8: // S69 - IATA sales code
      _pricingDetailTrx->iataSalesCode() = xmlValue.c_str();
      break;
    case 9: // AO0 - Sales location
      _pricingDetailTrx->salesLocation() = xmlValue.c_str();
      break;
    case 10: // C74 - Consolidator Plus Up Currency Code
      _pricingDetailTrx->consolidatorPlusUpCurrencyCode() = xmlValue.c_str();
      break;
    case 11: // C6Z - Consolidator Plus Up Amount
      _pricingDetailTrx->consolidatorPlusUpFareCalcAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 12: // D07 - Ticketing date
      _pricingDetailTrx->ticketingDate() = convertDate(xmlValue.c_str());
      break;
    case 13: // D54 - Ticketing time
    {
      int16_t ticketingTime = atoi(xmlValue.c_str());
      _pricingDetailTrx->ticketingDate() = _pricingDetailTrx->ticketingDate() +
                                           tse::Hours(ticketingTime / 60) +
                                           tse::Minutes(ticketingTime % 60) + tse::Seconds(0);
      _pricingDetailTrx->dataHandle().setTicketDate(_pricingDetailTrx->ticketingDate());
    }
    break;
    case 14: // SFY - Carrier Imposed surcharge
      _pricingDetailTrx->segmentFeeApplied() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeSummaryInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storeSummaryInformation");
}

void
PricingDetailModelMap::saveSummaryInformation()
{
  LOG4CXX_DEBUG(logger, "In saveSummaryInformation");
}

void
PricingDetailModelMap::storePassengerInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePassengerInformation");

  // Create a PaxDetail record is necessary
  if (_paxDetail == nullptr)
  {
    _trx->dataHandle().get(_paxDetail);

    // equivalent amount information might not be provided in pricing response, so provide default
    // information
    _paxDetail->equivalentCurrencyCode() = "";
    _paxDetail->equivalentAmount() = 0;
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B70 - PassengerType
      _paxDetail->paxType() = xmlValue.c_str();
      break;
    case 2: // C43 - Construction currency code
      _paxDetail->constructionCurrencyCode() = xmlValue.c_str();
      break;
    case 3: // C5E - Construction total amount
      _paxDetail->constructionTotalAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 4: // C40 - Base currency code
      _paxDetail->baseCurrencyCode() = xmlValue.c_str();
      break;
    case 5: // C5A - Base fare amount
      _paxDetail->baseFareAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 6: // C45 - Equivalent currency code
      _paxDetail->equivalentCurrencyCode() = xmlValue.c_str();
      break;
    case 7: // C5F - Equivalent amount
      _paxDetail->equivalentAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 8: // C44 - Currency code minimum
      _paxDetail->currencyCodeMinimum() = xmlValue.c_str();
      break;
    case 9: // C5C - Currency code minimum amount
      _paxDetail->currencyCodeMinimumAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 10: // Q0X - Stop over count
      _paxDetail->stopoverCount() = atoi(xmlValue.c_str());
      break;
    case 11: // C63 - Stop over charges
      _paxDetail->stopoverCharges() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 12: // C46 - Fare currency code
      _paxDetail->paxFareCurrencyCode() = xmlValue.c_str();
      break;
    case 13: // C66 - Total per passenger
      _paxDetail->totalPerPassenger() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 14: // C65 - Total taxes
      _paxDetail->totalTaxes() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 15: // S66 - Fare calc line
      _paxDetail->fareCalcLine() = xmlValue.c_str();
      break;
    case 16: // C5D - Commission percentage
      _paxDetail->commissionPercentage() = atoi(xmlValue.c_str());
      break;
    case 17: // C5B - Commission amount
      _paxDetail->commissionAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 18: // N09 - Commission cap
      _paxDetail->commissionCap() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 19: // C64 - Travel agency tax
      _paxDetail->travelAgencyTax() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 20: // P27 - Non refundable
      _paxDetail->nonRefundable() = xmlValue.c_str()[0];
      break;
    case 21: // P2C - Negotiated with corp
      _paxDetail->negotiatedWithCorp() = xmlValue.c_str()[0];
      break;
    case 22: // P2B - Negotiated without corp
      _paxDetail->negotiatedWithoutCorp() = xmlValue.c_str()[0];
      break;
    case 23: // P2A - Local currency
      _paxDetail->localCurrency() = xmlValue.c_str();
      break;
    case 24: // Q0W - Fare passenger number
      _paxDetail->paxFarePassengerNumber() = atoi(xmlValue.c_str());
      break;
    case 25: // S02 - Tour code description
      _paxDetail->tourCodeDescription() = xmlValue.c_str();
      break;
    case 26: // C62 - Net fare amount
      _paxDetail->netFareAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 27: // N0C - Tour indicator
      _paxDetail->tourIndicator() = xmlValue.c_str()[0];
      break;
    case 28: // S01 - Text box
      _paxDetail->textBox() = xmlValue.c_str();
      break;
    case 29: // N0B - Net gross
      _paxDetail->netGross() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 30: // C61 - Cat35 commission percent
      _paxDetail->cat35CommissionPercent() = atoi(xmlValue.c_str());
      break;
    case 31: // C60 - Cat35 commission amount
      _paxDetail->cat35CommissionAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 32: // N0A - BSP method type
      _paxDetail->bspMethodType() = xmlValue.c_str();
      break;
    case 33: // Q0V - Cat35 warning
      _paxDetail->cat35Warning() = xmlValue.c_str();
      break;
    case 34: // P26 - Cat35 used
      _paxDetail->cat35Used() = xmlValue.c_str()[0];
      break;
    case 35: // P28 - Override Cat35
      _paxDetail->overrideCat35() = xmlValue.c_str()[0];
      break;
    case 36: // P29 - Ticket restricted
      _paxDetail->ticketRestricted() = xmlValue.c_str()[0];
      break;
    case 37: // P2E - Paper ticket surcharge may apply
      _paxDetail->paperTicketSurchargeMayApply() = xmlValue.c_str()[0];
      break;
    case 38: // P2D - Paper ticket surcharge included
      _paxDetail->paperTicketSurchargeIncluded() = xmlValue.c_str()[0];
      break;
    case 39: // S84 - WPn Details
      _paxDetail->wpnDetails() = xmlValue.c_str();
      break;
    case 40: // Q4P = WPn Option Number
      _paxDetail->wpnOptionNumber() = atoi(xmlValue.c_str());
      break;
    case 41: // S85 - Acc Tvl Restriction Data
      _paxDetail->accTvlData() = xmlValue.c_str();
      break;
    case 42: // PBS - Req Acc Tvl Restriction
      _paxDetail->reqAccTvl() = xmlValue.c_str()[0];
      break;
    case 43: // C71 - Transfer count
      _paxDetail->transferCount() = atoi(xmlValue.c_str());
      break;
    case 44: // C70 - Transfer charges
      _paxDetail->transferCharges() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 45: // C72 - Transfer Published Currency
      _paxDetail->transferPubCurrency() = xmlValue.c_str();
      break;
    case 46: // C73 - Stopover Published Currency
      _paxDetail->stopOverPubCurrency() = xmlValue.c_str();
      break;
    case 47: // C56 - Total per passenger + imposed carrier surcharges
      _paxDetail->totalPerPaxPlusImposedSrg() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 48: // C6L - Total taxes + imposed carrier surcharges
      _paxDetail->totalTaxesPlusImposedSrg() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 49: // S86
      _paxDetail->baggageResponse() = xmlValue.c_str();
      break;
    case 50: // USI
       break;
    case 51: // VCL
      _paxDetail->vclInfo() = xmlValue.c_str();
    case 52: // PY6 - Cat35TFSFWithNet - Not parse it
       break;
    case 53: // PY7 - Cat35BlankCommission - Not parse it
       break;
    case 54: // PY8 - CommissionBaseAmount - Not parse it
       break;
    default:
      LOG4CXX_WARN(logger,
                   "storePassengerInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storePassengerInformation");
}

void
PricingDetailModelMap::savePassengerInformation()
{
  LOG4CXX_DEBUG(logger, "In savePassengerInformation");
  if (_pricingDetailTrx && _paxDetail)
  {
    copyTaxOverrideTbd2Tax(_paxDetail);

    _pricingDetailTrx->paxDetails().push_back(_paxDetail);
    _paxDetail = nullptr;
  }
}

void
PricingDetailModelMap::copyTaxOverrideTbd2Tax(PaxDetail* paxDetail)
{
  if (paxDetail == nullptr)
    return;

  std::vector<TaxDetail*> taxDetails;

  std::vector<TaxBreakdown*>& taxBreakdowns = paxDetail->taxBreakdowns();
  std::vector<TaxBreakdown*>::iterator tbdIter = taxBreakdowns.begin();
  for (; tbdIter != taxBreakdowns.end(); ++tbdIter)
  {
    TaxBreakdown* taxBreakdown = *tbdIter;

    if (taxBreakdown->publishedCurrencyCode().equalToConst("OV")) // For Tax Override
    {
      // Create TaxDetail for the TaxOveride
      TaxDetail* taxDetail = nullptr;
      _trx->dataHandle().get(taxDetail);
      if (taxDetail != nullptr)
      {
        taxDetail->amount() = taxBreakdown->amount();
        taxDetail->code() = taxBreakdown->code();
        taxDetail->currencyCode() = taxBreakdown->currencyCode();
        taxDetail->description() = taxBreakdown->description();
        taxDetail->amountPublished() = 0;
        taxDetail->publishedCurrencyCode() = taxBreakdown->publishedCurrencyCode();
        taxDetail->emuCurrencyCode() = "";
        taxDetail->gst() = "";
        taxDetail->countryCode() = taxBreakdown->countryCode();
        taxDetail->airlineCode() = taxBreakdown->airlineCode();
        taxDetail->stationCode() = "";

        taxDetails.push_back(taxDetail);
      }
    }
  }

  if (!taxDetails.empty())
  {
    paxDetail->taxDetails().insert(
        paxDetail->taxDetails().begin(), taxDetails.begin(), taxDetails.end());
  }

  if (fallback::fixed::fallbackWpdfEnhancedRuleDisplay())
    return;

  // SFD
  std::vector<TaxDetail*> aslTaxDetails;

  std::vector<TaxBreakdown*>& aslTaxBreakdowns = paxDetail->sellingFare().taxBreakdowns();
  std::vector<TaxBreakdown*>::iterator aslTbdIter = aslTaxBreakdowns.begin();
  for (; aslTbdIter != aslTaxBreakdowns.end(); ++aslTbdIter)
  {
    TaxBreakdown* aslTaxBreakdown = *aslTbdIter;

    if (aslTaxBreakdown->publishedCurrencyCode().equalToConst("OV")) // For Tax Override
    {
      // Create TaxDetail for the TaxOveride
      TaxDetail* aslTaxDetail = nullptr;
      _trx->dataHandle().get(aslTaxDetail);
      if (aslTaxDetail != nullptr)
      {
        aslTaxDetail->amount() = aslTaxBreakdown->amount();
        aslTaxDetail->code() = aslTaxBreakdown->code();
        aslTaxDetail->currencyCode() = aslTaxBreakdown->currencyCode();
        aslTaxDetail->description() = aslTaxBreakdown->description();
        aslTaxDetail->amountPublished() = 0;
        aslTaxDetail->publishedCurrencyCode() = aslTaxBreakdown->publishedCurrencyCode();
        aslTaxDetail->emuCurrencyCode() = "";
        aslTaxDetail->gst() = "";
        aslTaxDetail->countryCode() = aslTaxBreakdown->countryCode();
        aslTaxDetail->airlineCode() = aslTaxBreakdown->airlineCode();
        aslTaxDetail->stationCode() = "";

        aslTaxDetails.push_back(aslTaxDetail);
      }
    }
  }

  if (!aslTaxDetails.empty())
  {
    paxDetail->sellingFare().taxDetails().insert(
        paxDetail->sellingFare().taxDetails().begin(), aslTaxDetails.begin(), aslTaxDetails.end());
  }
}

void
PricingDetailModelMap::storeTaxInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeTaxInformation");

  // Create a Tax Detail record is necessary
  if (_taxDetail == nullptr)
  {
    _trx->dataHandle().get(_taxDetail);
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // C6B - Tax amount
      _taxDetail->amount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 2: // BC0 - Tax code
      _taxDetail->code() = xmlValue.c_str();
      break;
    case 3: // C40 - Tax currency code
      _taxDetail->currencyCode() = xmlValue.c_str();
      break;
    case 4: // S04 - Tax description
      _taxDetail->description() = xmlValue.c_str();
      break;
    case 5: // C6A - Tax amount published
      _taxDetail->amountPublished() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 6: // C41 - Published currency code
      _taxDetail->publishedCurrencyCode() = xmlValue.c_str();
      break;
    case 7: // P2P - EMU currency code
      _taxDetail->emuCurrencyCode() = xmlValue.c_str();
      break;
    case 8: // P2Q - GST
      _taxDetail->gst() = xmlValue.c_str();
      break;
    case 9: // A40 - Country Code
      _taxDetail->countryCode() = xmlValue.c_str();
      break;
    case 10: // A04 - Airline Code
      _taxDetail->airlineCode() = xmlValue.c_str();
      break;
    case 11: // S05 - Station Code
      _taxDetail->stationCode() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeTaxInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storeTaxInformation");
}

void
PricingDetailModelMap::saveTaxInformation()
{
  LOG4CXX_DEBUG(logger, "In saveTaxInformation");
  if (_paxDetail && _taxDetail)
  {
    if (_isInAsl)
      _paxDetail->sellingFare().taxDetails().push_back(_taxDetail);
    else
      _paxDetail->taxDetails().push_back(_taxDetail);
    _taxDetail = nullptr;
  }
}

void
PricingDetailModelMap::storeTaxBreakdown(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeTaxBreakdown");

  // Create a Tax Detail record is necessary
  if (_taxBreakdown == nullptr)
  {
    _trx->dataHandle().get(_taxBreakdown);
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // C6B - Tax amount
      _taxBreakdown->amount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 2: // BC0 - Tax code
      _taxBreakdown->code() = xmlValue.c_str();
      break;
    case 3: // C40 - Tax currency code
      _taxBreakdown->currencyCode() = xmlValue.c_str();
      break;
    case 4: // C6A - Tax amount published
      _taxBreakdown->amountPublished() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 5: // A04 - Airline Code
      _taxBreakdown->airlineCode() = xmlValue.c_str();
      break;
    case 6: // S05 - Station Code
      _taxBreakdown->airportCode() = xmlValue.c_str();
      break;
    case 7: // A40 - Tax Country Code
      _taxBreakdown->countryCode() = xmlValue.c_str();
      break;
    case 8: // S04 - Tax Description
      _taxBreakdown->description() = xmlValue.c_str();
      break;
    case 9: // C41 - Published Currency
      _taxBreakdown->publishedCurrencyCode() = xmlValue.c_str();
      break;
    case 10: // A06 - Tax Type
      _taxBreakdown->type() = xmlValue.c_str()[0];
      break;
    default:
      LOG4CXX_WARN(logger, "storeTaxBreakdown - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storeTaxBreakdown");
}

void
PricingDetailModelMap::saveTaxBreakdown()
{
  LOG4CXX_DEBUG(logger, "In saveTaxBreakdown");
  if (_paxDetail && _taxBreakdown)
  {
    if (_isInAsl)
      _paxDetail->sellingFare().taxBreakdowns().push_back(_taxBreakdown);
    else
      _paxDetail->taxBreakdowns().push_back(_taxBreakdown);
    _taxBreakdown = nullptr;
  }
}

void
PricingDetailModelMap::storeCurrencyConversion(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeCurrencyConversion");

  // Create a Currency Detail record if necessary
  if (_currencyDetail == nullptr)
  {
    _trx->dataHandle().get(_currencyDetail);
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // C41 - From
      _currencyDetail->from() = xmlValue.c_str();
      break;
    case 2: // C42 - To
      _currencyDetail->to() = xmlValue.c_str();
      break;
    case 3: // C46 - Intermediate Currency
      _currencyDetail->intermediateCurrency() = xmlValue.c_str();
      break;
    case 4: // C52 - Amount
      _currencyDetail->amount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 5: // Q08 - Number of Decimal Places
      _currencyDetail->decimalPlaces() = (uint16_t)atoi(xmlValue.c_str());
      break;
    case 6: // C53 - Converted Amount
      _currencyDetail->convertedAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 7: // Q07 - Number of Decimal Places Converted Amount
      _currencyDetail->decimalPlacesConvertedAmount() = (uint16_t)atoi(xmlValue.c_str());
      break;
    case 8: // C54 - Exchange Rate One
      _currencyDetail->exchangeRateOne() = (ExchRate)atof(xmlValue.c_str());
      break;
    case 9: // Q05 - Number of Decimal Places Exchange Rate One
      _currencyDetail->decimalPlacesExchangeRateOne() = (uint16_t)atoi(xmlValue.c_str());
      break;
    case 10: // C55 - Exchange Rate Two
      _currencyDetail->exchangeRateTwo() = (ExchRate)atof(xmlValue.c_str());
      break;
    case 11: // Q06 - Number of Decimal Places Exchange Rate Two
      _currencyDetail->decimalPlacesExchangeRateTwo() = (uint16_t)atoi(xmlValue.c_str());
      break;
    case 12: // B00 - Currency Conversion Carrier Code
      _currencyDetail->currencyConversionCarrierCode() = xmlValue.c_str();
      break;
    case 13: // A40 - Country Code
      _currencyDetail->countryCode() = xmlValue.c_str();
      break;
    case 14: // D00 - Travel Date
      _currencyDetail->travelDate() = xmlValue.c_str();
      break;
    case 15: // B50 - Currency Conversion Fare Basis Code
      _currencyDetail->currencyCoversionFareBasisCode() = xmlValue.c_str();
      break;
    case 16: // N02 - Conversion Type
      _currencyDetail->conversionType() = xmlValue.c_str();
      break;
    case 17: // N01 - Application Type
      _currencyDetail->applicationType() = xmlValue.c_str();
      break;
    case 18: // D06 - Effective Date
      _currencyDetail->effectiveDate() = xmlValue.c_str();
      break;
    case 19: // D05 - Discontinue Date
      _currencyDetail->discontinueDate() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeCurrencyConversion - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storeCurrencyConversion");
}

void
PricingDetailModelMap::saveCurrencyConversion()
{
  LOG4CXX_DEBUG(logger, "In saveCurrencyConversion");
}

void
PricingDetailModelMap::storeFareIATARate(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeFareIATARate");
  LOG4CXX_DEBUG(logger, "Finished in storeFareIATARate");
}

void
PricingDetailModelMap::saveFareIATARate()
{
  LOG4CXX_DEBUG(logger, "In saveFareIATARate");
  if (_paxDetail && _currencyDetail)
  {
    _paxDetail->fareIATARates().push_back(_currencyDetail);
    _currencyDetail = nullptr;
  }
}

void
PricingDetailModelMap::storeFareBankerSellRate(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeFareBankerSellRate");
  LOG4CXX_DEBUG(logger, "Finished in storeFareBankerSellRate");
}

void
PricingDetailModelMap::saveFareBankerSellRate()
{
  LOG4CXX_DEBUG(logger, "In saveFareBankerSellRate");
  if (_paxDetail && _currencyDetail)
  {
    _paxDetail->fareBankerSellRates().push_back(_currencyDetail);
    _currencyDetail = nullptr;
  }
}

void
PricingDetailModelMap::storeTaxBankerSellRate(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeTaxBankerSellRate");
  LOG4CXX_DEBUG(logger, "Finished in storeTaxBankerSellRate");
}

void
PricingDetailModelMap::saveTaxBankerSellRate()
{
  LOG4CXX_DEBUG(logger, "In saveTaxBankerSellRate");
  if (_paxDetail && _currencyDetail)
  {
    _paxDetail->taxBankerSellRates().push_back(_currencyDetail);
    _currencyDetail = nullptr;
  }
}

void
PricingDetailModelMap::storePlusUp(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePlusUp");

  // Create a Plus Up Detail record if necessary
  if (_plusUpDetail == nullptr)
  {
    _trx->dataHandle().get(_plusUpDetail);
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // C6L - Plus Up Amount
      _plusUpDetail->amount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 2: // A11 - Plus Up Orig City
      _plusUpDetail->origCity() = xmlValue.c_str();
      break;
    case 3: // A12 - Plus Up Dest City
      _plusUpDetail->destCity() = xmlValue.c_str();
      break;
    case 4: // S68 - Plus Up Message
      _plusUpDetail->message() = xmlValue.c_str();
      break;
    case 5: // A18 - Plus Up Via City
      _plusUpDetail->viaCity() = xmlValue.c_str();
      break;
    case 6: // A40 - Plus Up Country of Payment
      _plusUpDetail->countryOfPayment() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "storePlusUp - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storePlusUp");
}

void
PricingDetailModelMap::savePlusUp()
{
  LOG4CXX_DEBUG(logger, "In savePlusUp");
  if (_paxDetail && _plusUpDetail)
  {
    _paxDetail->plusUpDetails().push_back(_plusUpDetail);
    _plusUpDetail = nullptr;
  }
}

void
PricingDetailModelMap::storeFareCalcInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeFareCalcInformation");

  // Create a FareCalc Detail record is necessary
  if (_fareCalcDetail == nullptr)
  {
    _trx->dataHandle().get(_fareCalcDetail);
    if (_fareCalcDetail == nullptr)
      return;
    _fareCalcDetail->sideTripIncluded() = 'F';
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A11 - Departure city
      _fareCalcDetail->departureCity() = xmlValue.c_str();
      break;
    case 2: // A01 - Departure airport
      _fareCalcDetail->departureAirport() = xmlValue.c_str();
      break;
    case 3: // B02 - Governing carrier
      _fareCalcDetail->governingCarrier() = xmlValue.c_str();
      break;
    case 4: // B03 - Secondary governing carrier
      _fareCalcDetail->secondaryGoverningCarrier() = xmlValue.c_str();
      break;
    case 5: // C40 - Base currency code
      _fareCalcDetail->baseCurrencyCode() = xmlValue.c_str();
      break;
    case 6: // A12 - Arrival city
      _fareCalcDetail->arrivalCity() = xmlValue.c_str();
      break;
    case 7: // A02 - Arrival airport
      _fareCalcDetail->arrivalAirport() = xmlValue.c_str();
      break;
    case 8: // C50 - Fare amount
      _fareCalcDetail->fareAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 9: // B50 - Fare basis code
      _fareCalcDetail->fareBasisCode() = xmlValue.c_str();
      break;
    case 10: // Q04 - Fare basis code length
      _fareCalcDetail->fareBasisCodeLength() = atoi(xmlValue.c_str());
      break;
    case 11: // B71 - Requested passenger type
      _fareCalcDetail->requestedPassengerType() = xmlValue.c_str();
      break;
    case 12: // B51 - Differential fare basis code
      _fareCalcDetail->differentialFareBasisCode() = xmlValue.c_str();
      break;
    case 13: // BE0 - Ticket designator
      _fareCalcDetail->ticketDesignator() = xmlValue.c_str();
      break;
    case 14: // BD0 - Snap designator discount
      _fareCalcDetail->snapDesignatorDiscount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 15: // N00 - Cabin code
      _fareCalcDetail->cabinCode() = xmlValue.c_str();
      break;
    case 16: // A41 - Departure country
      _fareCalcDetail->departureCountry() = xmlValue.c_str();
      break;
    case 17: // A51 - Departure IATA
      _fareCalcDetail->departureIATA() = xmlValue.c_str();
      break;
    case 18: // A31 - Departure state or province
      _fareCalcDetail->departureStateOrProvince() = xmlValue.c_str();
      break;
    case 19: // A42 - Arrival country
      _fareCalcDetail->arrivalCountry() = xmlValue.c_str();
      break;
    case 20: // A52 - Arrival IATA
      _fareCalcDetail->arrivalIATA() = xmlValue.c_str();
      break;
    case 21: // A32 - Arrival state or province
      _fareCalcDetail->arrivalStateOrProvince() = xmlValue.c_str();
      break;
    case 22: // C51 - Published fare amount
      _fareCalcDetail->publishedFareAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 23: // C40 - Fare component currency code
      _fareCalcDetail->fareComponentCurrencyCode() = xmlValue.c_str();
      break;
    case 24: // P05 - Round trip fare
      _fareCalcDetail->roundTripFare() = xmlValue.c_str()[0];
      break;
    case 25: // P04 - One way fare
      _fareCalcDetail->oneWayFare() = xmlValue.c_str()[0];
      break;
    case 26: // P03 - One way directional fare
      _fareCalcDetail->oneWayDirectionalFare() = xmlValue.c_str()[0];
      break;
    case 27: // P02 - IATA authorized carrier
      _fareCalcDetail->iataAuthorizedCarrier() = xmlValue.c_str()[0];
      break;
    case 28: // P0E - Verify geographic restrictions
      _fareCalcDetail->verifyGeographicRestrictions() = xmlValue.c_str()[0];
      break;
    case 29: // P01 - Fail CAT15
      _fareCalcDetail->failCat15() = xmlValue.c_str()[0];
      break;
    case 30: // P06 - Ticket restricted
      _fareCalcDetail->ticketRestricted() = xmlValue.c_str()[0];
      break;
    case 31: // P0B - Fare by rule spouse head
      _fareCalcDetail->fareByRuleSpouseHead() = xmlValue.c_str()[0];
      break;
    case 32: // P0A - Fare by rule spouse accompany
      _fareCalcDetail->fareByRuleSpouseAccompany() = xmlValue.c_str()[0];
      break;
    case 33: // P0D - Fare by rule seaman adult
      _fareCalcDetail->fareByRuleSeamanAdult() = xmlValue.c_str()[0];
      break;
    case 34: // C63 - Fare by rule seaman child
      _fareCalcDetail->fareByRuleSeamanChild() = xmlValue.c_str()[0];
      break;
    case 35: // P0C - Fare by rule seaman infant
      _fareCalcDetail->fareByRuleSeamanInfant() = xmlValue.c_str()[0];
      break;
    case 36: // P00 - Fare by rule BTS
      _fareCalcDetail->fareByRuleBTS() = xmlValue.c_str()[0];
      break;
    case 37: // P09 - Fare by rule negotiated
      _fareCalcDetail->fareByRuleNegotiated() = xmlValue.c_str()[0];
      break;
    case 38: // P07 - Fare by rule negotiated child
      _fareCalcDetail->fareByRuleNegotiatedChild() = xmlValue.c_str()[0];
      break;
    case 39: // P08 - Fare by rule negotiated infant
      _fareCalcDetail->fareByRuleNegotiatedInfant() = xmlValue.c_str()[0];
      break;
    case 40: // D00 - Commencement date
      _fareCalcDetail->commencementDate() = convertDate(xmlValue.c_str());
      break;
    case 41: // PAX - CAT5 requires rebook ?????
      //    _fareCalcDetail->commencementDate() = xmlValue.c_str()[0];
      break;
    case 42: // N0K - Type of fare
      _fareCalcDetail->typeOfFare() = xmlValue.c_str();
      break;
    case 43: // S67 - Discount Code
      _fareCalcDetail->discountCode() = xmlValue.c_str();
      break;
    case 44: // Q17 - Discount Percentage
      _fareCalcDetail->discountPercentage() = atoi(xmlValue.c_str());
      break;
    case 45: // PAY - Is Routing
      _fareCalcDetail->isRouting() = (xmlValue.c_str()[0] == 'T');
      break;
    case 46: // Q48 - Mileage Surcharge Pctg
      _fareCalcDetail->mileageSurchargePctg() = atoi(xmlValue.c_str());
      break;
    case 47: // A13 - HIP Orig City
      _fareCalcDetail->hipOrigCity() = xmlValue.c_str();
      break;
    case 48: // A14 - HIP Dest City
      _fareCalcDetail->hipDestCity() = xmlValue.c_str();
      break;
    case 49: // A18 - Constructed HIP City
      _fareCalcDetail->constructedHipCity() = xmlValue.c_str();
      break;
    case 50: // PAZ - Special Fare
      _fareCalcDetail->normalOrSpecialFare() = xmlValue.c_str()[0];
      break;
    case 51: // N1K - Pricing Unit Type
      _fareCalcDetail->pricingUnitType() = xmlValue.c_str();
      break;
    case 52: // A60 - Global Direction Indicator
      _fareCalcDetail->globalIndicator() = xmlValue.c_str();
      break;
    case 53: // S70 - Fare Calc Directionality
      _fareCalcDetail->directionality() = xmlValue.c_str();
      break;
    case 54: // Q4J - Pricing Unit Count
      _fareCalcDetail->pricingUnitCount() = atoi(xmlValue.c_str());
      break;
    case 55: // B08 - true governing carrier
      _fareCalcDetail->trueGoverningCarrier() = xmlValue.c_str();
      break;
    case 56: // P2N - Side Trip included
      _fareCalcDetail->sideTripIncluded() = xmlValue.c_str()[0];
      break;
    case 57: // Q0U - segment count
      _fareCalcDetail->segmentsCount() = atoi(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeFareCalcInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storeFareCalcInformation");
}

void
PricingDetailModelMap::saveFareCalcInformation()
{
  LOG4CXX_DEBUG(logger, "In saveFareCalcInformation");
  if (_paxDetail && _fareCalcDetail)
  {
    _paxDetail->fareCalcDetails().push_back(_fareCalcDetail);
    _fareCalcDetail = nullptr;
  }
}

void
PricingDetailModelMap::storeSurchargeInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeSurchargeInformation");

  // Create a Surcharge Detail record is necessary
  if (_surchargeDetail == nullptr)
  {
    _trx->dataHandle().get(_surchargeDetail);
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // N0F - Surcharge type
      _surchargeDetail->type() = xmlValue.c_str();
      break;
    case 2: // C69 - Surcharge amount
      _surchargeDetail->amount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 3: // C46 - Currency code
      _surchargeDetail->currencyCode() = xmlValue.c_str();
      break;
    case 4: // S03 - Surcharge description
      _surchargeDetail->description() = xmlValue.c_str();
      break;
    case 5: // C41 - Published currency
      _surchargeDetail->publishedCurrency() = xmlValue.c_str();
      break;
    case 6: // A11 - surcharge origin
      _surchargeDetail->origin() = xmlValue.c_str();
      break;
    case 7: // A12 - surcharge destination
      _surchargeDetail->destination() = xmlValue.c_str();
      break;

    default:
      LOG4CXX_WARN(logger,
                   "storeSurchargeInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storeSurchargeInformation");
}

void
PricingDetailModelMap::saveSurchargeInformation()
{
  LOG4CXX_DEBUG(logger, "In saveSurchargeInformation");
  if (_segmentDetail && _surchargeDetail)
  {
    _segmentDetail->surchargeDetails().push_back(_surchargeDetail);
    _surchargeDetail = nullptr;
  }
}

void
PricingDetailModelMap::storeSegmentInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeSegmentInformation");

  // Create a Segment Detail record is necessary
  if (_segmentDetail == nullptr)
  {
    _trx->dataHandle().get(_segmentDetail);
    if (_segmentDetail == nullptr)
      return;
    _segmentDetail->cityStopoverCharge() = 0;
    _segmentDetail->transferCharge() = 0;
    _segmentDetail->sideTripIndicator() = 'F';
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A11 - Departure city
      _segmentDetail->departureCity() = xmlValue.c_str();
      break;
    case 2: // C6I - Departure airport
      _segmentDetail->departureAirport() = xmlValue.c_str();
      break;
    case 3: // A12 - Arrival city
      _segmentDetail->arrivalCity() = xmlValue.c_str();
      break;
    case 4: // A02 - Arrival airport
      _segmentDetail->arrivalAirport() = xmlValue.c_str();
      break;
    case 5: // P72 - Fare class
      _segmentDetail->fareClass() = xmlValue.c_str();
      break;
    case 6: // C67 - City stopover charge
      _segmentDetail->cityStopoverCharge() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 7: // C68 - Transfer charge
      _segmentDetail->transferCharge() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 8: // Q0Z - Itin segment number
      _segmentDetail->itinSegmentNumber() = atoi(xmlValue.c_str());
      break;
    case 9: // S12 - Route travel
      _segmentDetail->routeTravel() = xmlValue.c_str();
      break;
    case 10: // D06 - Not valid before date
      _segmentDetail->notValidBeforeDate() = convertDate(xmlValue.c_str());
      break;
    case 11: // D05 - Not valid after date
      _segmentDetail->notValidAfterDate() = convertDate(xmlValue.c_str());
      break;
    case 12: // Q0Y - Passenger number
      _segmentDetail->passengerNumber() = atoi(xmlValue.c_str());
      break;
    case 13: // N0D - Baggage indicator
      _segmentDetail->baggageIndicator() = xmlValue.c_str()[0];
      break;
    case 14: // B20 - Baggage value
      _segmentDetail->baggageValue() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 15: // PAW - Availability     break;
      _segmentDetail->availabilityBreak() = xmlValue.c_str()[0];
      break;
    case 16: // P2N - Side trip indicator
      _segmentDetail->sideTripIndicator() = xmlValue.c_str()[0];
      break;
    case 17: // P2I - Extra mileage allowance
      _segmentDetail->extraMileageAllowance() = atoi(xmlValue.c_str());
      break;
    case 18: // P2K - Mileage exclusion
      _segmentDetail->mileageExclusion() = atoi(xmlValue.c_str());
      break;
    case 19: // P2L - Mileage reduction
      _segmentDetail->mileageReduction() = atoi(xmlValue.c_str());
      break;
    case 20: // P2J - Mileage equalization
      _segmentDetail->mileageEqualization() = atoi(xmlValue.c_str());
      break;
    case 21: // P2M - Stopover
      _segmentDetail->stopover() = xmlValue.c_str()[0];
      break;
    case 22: // P2H - Connection
      _segmentDetail->connection() = xmlValue.c_str()[0];
      break;
    case 23: // P2F - Fare     break point
      _segmentDetail->fareBreakPoint() = xmlValue.c_str()[0];
      break;
    case 24: // P2O - Turn around
      _segmentDetail->turnAround() = xmlValue.c_str()[0];
      break;
    case 25: // P2G - Off point part main component
      _segmentDetail->offPointPartMainComponent() = xmlValue.c_str()[0];
      break;
    case 26: // S07 - Side trip end
      _segmentDetail->sideTripEnd() = xmlValue.c_str()[0];
      break;
    case 27: // S08 - Side Trip End Component
      break;
    case 28: // S09 - Unchargeable surface
      _segmentDetail->unchargeableSurface() = xmlValue.c_str()[0];
      break;
    case 29: // S10 - Pure surface
      _segmentDetail->pureSurface() = xmlValue.c_str()[0];
      break;
    case 30: // P3M - Transfer
      _segmentDetail->transfer() = xmlValue.c_str()[0];
      break;
    case 31: // C72 - Transfer Published Currency
      _segmentDetail->transferPubCurrency() = xmlValue.c_str();
      break;
    case 32: // C73 - Stopover Published Currency
      _segmentDetail->stopoverPubCurrency() = xmlValue.c_str();
      ;
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeSegmentInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storeSegmentInformation");
}

void
PricingDetailModelMap::saveSegmentInformation()
{
  LOG4CXX_DEBUG(logger, "In saveSegmentInformation");
  if (_fareCalcDetail && _segmentDetail)
  {
    _fareCalcDetail->segmentDetails().push_back(_segmentDetail);
    _segmentDetail = nullptr;
  }
}

void
PricingDetailModelMap::storeHIPInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeHIPInformation");

  // Create a Differential Detail record if necessary
  if (_differentialDetail == nullptr)
  {
    _trx->dataHandle().get(_differentialDetail);
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A13 - Orig City
      _differentialDetail->origCityHIP() = xmlValue.c_str();
      break;
    case 2: // A14 - Dest City
      _differentialDetail->destCityHIP() = xmlValue.c_str();
      break;
    case 3: // A01 - Low Orig HIP
      _differentialDetail->lowOrigHIP() = xmlValue.c_str();
      break;
    case 4: // A02 - Low Dest HIP
      _differentialDetail->lowDestHIP() = xmlValue.c_str();
      break;
    case 5: // A03 - High Orig HIP
      _differentialDetail->highOrigHIP() = xmlValue.c_str();
      break;
    case 6: // A04 - High Dest HIP
      _differentialDetail->highDestHIP() = xmlValue.c_str();
      break;
    case 7: // BJ0 - Fare Class High
      _differentialDetail->fareClassHigh() = xmlValue.c_str();
      break;
    case 8: // B30 - Fare Class Low
      _differentialDetail->fareClassLow() = (fallback::purgeBookingCodeOfNonAlpha(_trx)) ?
                                            (xmlValue.c_str()) :
                                            DataModelMap::purgeBookingCodeOfNonAlpha(xmlValue.c_str());
      break;
    case 9: // N00 - Cabin Low HIP
      _differentialDetail->cabinLow() = xmlValue.c_str()[0];
      break;
    case 10: // N04 - Cabin High HIP
      _differentialDetail->cabinHigh() = xmlValue.c_str()[0];
      break;
    case 11: // C50 - Amount HIP
      _differentialDetail->amount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 12: // Q48 - Mileage
      _differentialDetail->mileagePctg() = atoi(xmlValue.c_str());
      break;
    case 13: // A11 - Orig City for WPDF only
      _differentialDetail->origCityHIP() = xmlValue.c_str();
      break;
    case 14: // A12 - Dest City for WPDF only
      _differentialDetail->destCityHIP() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeHIPInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storeHIPInformation");
}

void
PricingDetailModelMap::saveHIPInformation()
{
  LOG4CXX_DEBUG(logger, "In saveHIPInformation");
  if (_fareCalcDetail && _differentialDetail)
  {
    _fareCalcDetail->differentialDetails().push_back(_differentialDetail);
    _differentialDetail = nullptr;
  }
}

void
PricingDetailModelMap::storeMessageInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeMessageInformation");

#if 0
    // Store in response stream of the current transaction

    int numAtts = attrs.getLength();
    for (int i = 0; i < numAtts; i++)
    {
        XMLChString xmlStr(attrs.getLocalName(i));
        XMLChString xmlValue(attrs.getValue(i));
        switch(((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
        {
        case 1:                 // N06 - Message type
            break;
        case 2:                 // Q3H - Message count
            break;
        case 3:                 // S18 - Message text
            _pricingDetailTrx->response() << xmlValue.c_str() << "\n";
            break;
        default:
            LOG4CXX_WARN(logger, "storeMessageInformation - Attribute: " <<
                    xmlValue.c_str()  << ", not mapped");
            break;
        }
    }
#endif

  LOG4CXX_DEBUG(logger, "Finished in storeMessageInformation");
}

void
PricingDetailModelMap::saveMessageInformation()
{
  LOG4CXX_DEBUG(logger, "In saveMessageInformation");
}

void
PricingDetailModelMap::storeAgentInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeAgentInformation");

  Agent& agent = _pricingDetailTrx->ticketingAgent();

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A10 - Agent city
      agent.agentCity() = xmlValue.c_str();
      break;
    case 2: // A20 - Travel agency PCC
      agent.tvlAgencyPCC() = xmlValue.c_str();
      break;
    case 3: // A21 - Home travel agency PCC
    {
      // This was patterned from MultipleModelMap.cpp. Apparentley the
      // input might have a space in the 4th position that must be
      // eliminated. Note: original used strcpy; used strncpy here to
      // prevent buffer overrun.
      const int bufSize = 10;
      char tmpBuf[bufSize];
      strncpy(tmpBuf, xmlValue.c_str(), bufSize - 1);
      if (tmpBuf[3] == ' ')
      {
        tmpBuf[3] = '\0';
      }
      else
      {
        tmpBuf[bufSize - 1] = '\0';
      }
      agent.mainTvlAgencyPCC() = tmpBuf;
    }
    break;
    case 4: // AB0 - Agency IATA number
      agent.tvlAgencyIATA() = xmlValue.c_str();
      break;
    case 5: // AB1 - Home agency IATA number
      agent.homeAgencyIATA() = xmlValue.c_str();
      break;
    case 6: // A90 - Agent function code
      agent.agentFunctions() = xmlValue.c_str();
      break;
    case 7: // N0G - Agent duty code
      agent.agentDuty() = xmlValue.c_str();
      break;
    case 8: // A80 - Airline Dept Code
      agent.airlineDept() = xmlValue.c_str();
      break;
    case 9: // B00 - Originating CRS (carrier)
      agent.cxrCode() = xmlValue.c_str();
      break;
    case 10: // C40 - Agent currency
      agent.currencyCodeAgent() = xmlValue.c_str();
      break;
    case 11: // Q01 - coHost ID
      agent.coHostID() = atoi(xmlValue.c_str());
      break;
    case 12: // N0L - Agent commission type
      agent.agentCommissionType() = xmlValue.c_str();
      break;
    case 13: // C6C - Agent commission amount
      agent.agentCommissionAmount() = atol(xmlValue.c_str());
      break;
    case 14: // AE0 - Abacus user
      agent.vendorCrsCode() = xmlValue.c_str();
      break;
    case 15: // AE1 - Office Designator
      agent.officeDesignator() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeAgentInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storeAgentInformation");
}

void
PricingDetailModelMap::saveAgentInformation()
{
  LOG4CXX_DEBUG(logger, "In saveAgentInformation");
}

void
PricingDetailModelMap::storeDynamicConfigOverride(const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}
void
PricingDetailModelMap::storeSellingFareInformation(const xercesc::Attributes& attrs)
{
  if (fallback::fixed::fallbackWpdfEnhancedRuleDisplay())
    return;
 
  LOG4CXX_DEBUG(logger, "In storeSellingFareInformation");

  // Create a Selling Fare Data record is necessary
  if (_sellingFareData == nullptr)
  {
    _trx->dataHandle().get(_sellingFareData);
    if (_sellingFareData == nullptr)
      return;
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // TYP – Layer Type Name
      _sellingFareData->layerTypeName() = xmlValue.c_str();
      break;
    case 2: // C5A - Base fare amount
      _sellingFareData->baseFareAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 3: // C5E - Construction total amount
      _sellingFareData->constructedTotalAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 4: // C5F - Equivalent amount
      _sellingFareData->equivalentAmount() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 5: // C65 – total taxes
      _sellingFareData->totalTaxes() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 6: // C66 - total per passenger
      _sellingFareData->totalPerPassenger() = (MoneyAmount)atof(xmlValue.c_str());
      break;
    case 7: // S66 – fare calculation
      _sellingFareData->fareCalculation() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeSellingFareInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in storeSellingFareInformation");
}

void
PricingDetailModelMap::saveSellingFareInformation()
{
  if (fallback::fixed::fallbackWpdfEnhancedRuleDisplay())
    return;

  LOG4CXX_DEBUG(logger, "In saveSellingFareInformation");
  if (_paxDetail && _sellingFareData)
  {
    _paxDetail->sellingFare().layerTypeName() = _sellingFareData->layerTypeName();
    _paxDetail->sellingFare().baseFareAmount() = _sellingFareData->baseFareAmount();
    _paxDetail->sellingFare().constructedTotalAmount() = _sellingFareData->constructedTotalAmount();
    _paxDetail->sellingFare().equivalentAmount() = _sellingFareData->equivalentAmount();
    _paxDetail->sellingFare().totalTaxes() = _sellingFareData->totalTaxes();
    _paxDetail->sellingFare().totalPerPassenger() = _sellingFareData->totalPerPassenger();
    _paxDetail->sellingFare().fareCalculation() = _sellingFareData->fareCalculation();
    _sellingFareData = nullptr;
  }
}

} // tse namespace
