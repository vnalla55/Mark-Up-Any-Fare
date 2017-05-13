//----------------------------------------------------------------------------
//
//  File:  PricingModelMap.cpp
//  Description: See PricingModelMap.h file
//  Created:  January 29, 2005
//  Authors:  Mike Carroll
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been suppliedave
//
//----------------------------------------------------------------------------

#include "Xform/PricingModelMap.h"

#include "Common/Assert.h"
#include "Common/CabinType.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/DynamicConfigLoader.h"
#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/Config/DynamicConfigurableNumber.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareTypePricingUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/TypeConvert.h"
#include "Common/XMLChString.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AvailData.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/ExchangeNewItin.h"
#include "DataModel/ExchangeOverrides.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/FrequentFlyerAccount.h"
#include "DataModel/MileageTypeData.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/ReservationData.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexNewItin.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/Traveler.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MultiTransport.h"
#include "Pricing/PricingUtil.h"
#include "Util/Algorithm/Container.h"
#include "Xform/CommonParserUtils.h"
#include "Xray/XrayUtil.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <time.h>

namespace tse
{
FALLBACK_DECL(fallbackBrandedFaresPricing);
FALLBACK_DECL(purgeBookingCodeOfNonAlpha);
FALLBACK_DECL(fallbackSearchForBrandsPricing);
FALLBACK_DECL(neutralToActualCarrierMapping);
FALLBACK_DECL(fallbackDisableStructuredRule);
FALLBACK_DECL(azPlusUp);
FALLBACK_DECL(azPlusUpExc);
FALLBACK_DECL(fallbackPriceByCabinActivation);
FALLBACK_DECL(fallbackAAExcludedBookingCode);
FALLBACK_DECL(virtualFOPMaxOBCalculation);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(fallbackFixProcessingRetailerCodeXRS);
FALLBACK_DECL(SSDSP_1844_removeArunkSegForSFR);
FIXEDFALLBACK_DECL(checkArunkForSfr);
FALLBACK_DECL(excDiscountAmountFix);
FALLBACK_DECL(excDiscountsFixDivideByZero);
FALLBACK_DECL(fallbackMoveGetCabinToDSS);
FALLBACK_DECL(fallbackAgencyRetailerHandlingFees);

namespace
{
DynamicConfigurableNumber
maxOptionsInBrandedPricing("OUTPUT_LIMITS",
                           "MAX_OPTIONS_IN_BRANDED_PRICING",
                           static_cast<size_t>(TnShoppingBrandsLimit::UNLIMITED_BRANDS));
DynamicConfigurableFlagOn
dynamicSoftPassDisabledInTn("TN_PATH", "DISABLE_SOFT_PASS", false);
DynamicConfigurableFlagOn
forceEnableWPZZ("TSE_SERVER", "FORCE_ENABLE_WPZZ", false);
}

static Logger
logger("atseintl.Xform.PricingModelMap");

static const char* PSS_OPEN_DATE_BASE = "1966-01";
static const char* DOLLAR_P = "$P";
static const char* WP_P = "WPP";
static const char* PSGR_AnyNN = "NN";
static const char* EPR_KEYWORD_MUL375 = "MUL375";
static const char* EPR_KEYWORD_TMPCRS = "TMPCRS";
static const char* EPR_KEYWORD_CRSAGT = "CRSAGT";
static const char* WEST_JET_CODE = "WS";
static std::string EPR_FFOCUS = "FFOCUS";
static std::string EPR_ORDFQD = "ORGFQD";
static std::string EPR_AGYRET = "AGYRET";

PricingModelMap::~PricingModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

bool
PricingModelMap::createMap()
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
      const NV& entry = *iter;
      const std::string& tagName = entry.name;
      // std::string tagValue = entry.value;
      Mapping* mapping = new Mapping();

      // Recognized class tags
      LOG4CXX_INFO(logger, "Tag name: " << tagName);
      if (tagName == "PRICINGREQUEST")
      {
        mapping->func = &tse::PricingModelMap::storePricingInformation;
        mapping->trxFunc = &tse::PricingModelMap::savePricingInformation;
      }
      else if (tagName == "ALTPRICINGREQUEST")
      {
        mapping->func = &tse::PricingModelMap::storeAltPricingInformation;
        mapping->trxFunc = &tse::PricingModelMap::savePricingInformation;
      }
      else if (tagName == "NOPNRPRICINGREQUEST")
      {
        mapping->func = &tse::PricingModelMap::storeNoPNRPricingInformation;
        mapping->trxFunc = &tse::PricingModelMap::savePricingInformation;
      }
      else if (tagName == "STRUCTUREDRULEREQUEST")
      {
        mapping->func = &tse::PricingModelMap::storeStructuredRuleInformation;
        mapping->trxFunc = &tse::PricingModelMap::savePricingInformation;
      }
      else if (tagName == "REXPRICINGREQUEST")
      {
        mapping->func = &tse::PricingModelMap::storeRexPricingInformation;
        mapping->trxFunc = &tse::PricingModelMap::savePricingInformation;
      }
      else if (tagName == "XRA")
      {
        mapping->func = &tse::PricingModelMap::storeXrayInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveXrayInformation;
      }
      else if (tagName == "AGI")
      {
        mapping->func = &tse::PricingModelMap::storeAgentInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveAgentInformation;
      }
      else if (tagName == "BIL")
      {
        mapping->func = &tse::PricingModelMap::storeBillingInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveBillingInformation;
      }
      else if (tagName == "PRO")
      {
        mapping->func = &tse::PricingModelMap::storeProcOptsInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveProcOptsInformation;
      }
      else if (tagName == "DIG")
      {
        mapping->func = &tse::PricingModelMap::storeDiagInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveDiagInformation;
      }
      else if (tagName == "PXI")
      {
        mapping->func = &tse::PricingModelMap::storePassengerInformation;
        mapping->trxFunc = &tse::PricingModelMap::savePassengerInformation;
      }
      else if (tagName == "PXF")
      {
        mapping->func = &tse::PricingModelMap::storePassengerTypeFlightInformation;
        mapping->trxFunc = &tse::PricingModelMap::savePassengerTypeFlightInformation;
      }
      else if (tagName == "PEN")
      {
        mapping->func = &tse::PricingModelMap::storePenaltyInformation;
        mapping->trxFunc = &tse::PricingModelMap::savePenaltyInformation;
      }
      else if (tagName == "FLI")
      {
        mapping->func = &tse::PricingModelMap::storeFlightInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveFlightInformation;
      }
      else if (tagName == "RFB")
      {
        mapping->func = &tse::PricingModelMap::storeRequestedFareBasisInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveRequestedFareBasisInformation;
      }
      else if (tagName == "SGI")
      {
        mapping->func = &tse::PricingModelMap::storeSegmentInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveSegmentInformation;
      }
      else if (tagName == "RES")
      {
        mapping->func = &tse::PricingModelMap::storeReservationInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveReservationInformation;
      }
      else if (tagName == "IDS")
      {
        mapping->func = &tse::PricingModelMap::storeItinSegmentInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveItinSegmentInformation;
      }
      else if (tagName == "POC")
      {
        mapping->func = &tse::PricingModelMap::storePocInformation;
        mapping->trxFunc = &tse::PricingModelMap::savePocInformation;
      }
      else if (tagName == "RLS")
      {
        mapping->func = &tse::PricingModelMap::storeRecordLocatorInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveRecordLocatorInformation;
      }
      else if (tagName == "PDT")
      {
        mapping->func = &tse::PricingModelMap::storeReservPassengerInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveReservPassengerInformation;
      }
      else if (tagName == "FFP")
      {
        mapping->func = &tse::PricingModelMap::storeFreqFlyerPartnerInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveFreqFlyerPartnerInformation;
      }
      else if (tagName == "CFF")
      {
        mapping->func = &tse::PricingModelMap::storeCorpFreqFlyerInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveCorpFreqFlyerInformation;
      }
      else if (tagName == "JPS")
      {
        mapping->func = &tse::PricingModelMap::storeJPSInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveJPSInformation;
      }
      else if (tagName == "AVL")
      {
        mapping->func = &tse::PricingModelMap::storeAvailabilityInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveAvailabilityInformation;
      }
      else if (tagName == "FBK")
      {
        mapping->func = &tse::PricingModelMap::storeFlightAvailInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveFlightAvailInformation;
      }
      else if (tagName == "EXC")
      {
        mapping->func = &tse::PricingModelMap::storeExchangeItinInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveExchangeItinInformation;
      }
      else if (tagName == "APX")
      {
        mapping->func = &tse::PricingModelMap::storeAccompanyPassengerInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveAccompanyPassengerInformation;
      }
      else if (tagName == "PUP")
      {
        mapping->func = &tse::PricingModelMap::storePlusUpInformation;
        mapping->trxFunc = &tse::PricingModelMap::savePlusUpInformation;
      }
      else if (tagName == "HIP")
      {
        mapping->func = &tse::PricingModelMap::storeDifferentialInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveDifferentialInformation;
      }
      else if (tagName == "SUR")
      {
        mapping->func = &tse::PricingModelMap::storeSurchargeInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveSurchargeInformation;
      }
      else if (tagName == "MIL")
      {
        mapping->func = &tse::PricingModelMap::storeMileageInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveMileageInformation;
      }
      else if (tagName == "STO")
      {
        mapping->func = &tse::PricingModelMap::storeStopoverInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveStopoverInformation;
      }
      else if (tagName == "FFY")
      {
        mapping->func = &tse::PricingModelMap::storeFreqFlyerStatus;
        mapping->trxFunc = nullptr;
      }
      else if (tagName == "RFG")
      {
        mapping->func = &tse::PricingModelMap::storeRequestedGroupCodes;
        mapping->trxFunc = nullptr;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::PricingModelMap::storeDynamicConfigOverride;
        mapping->trxFunc = nullptr;
      }
      else if (tagName == "TAX")
      {
        mapping->func = &tse::PricingModelMap::storeTaxInformation;
        mapping->trxFunc = &tse::PricingModelMap::saveTaxInformation;
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
        const NV& memEntry = *memIter;
        const std::string& memberName = memEntry.name;

        mapping->members[SDBMHash(memberName)] = atoi(memEntry.value.c_str());
      }

      // Set the map
      _classMap[SDBMHash(tagName)] = mapping;
    }
  }
  return true;
}

bool
PricingModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
{
  unsigned int hashedTagName = SDBMHash(tagName);

  // Find the mapping
  Mapping* mapping = (Mapping*)_classMap[hashedTagName];
  if (mapping && mapping->func)
  {
    _currentMapEntry = (void*)mapping;
    _currentMapEntryStack.push(_currentMapEntry);

    (this->*mapping->func)(atts);
  }
  else
  {
    LOG4CXX_WARN(logger, "Tag name: " << tagName << ", not mapped!");
    return false;
  }
  return true;
}

void
PricingModelMap::saveMapEntry(std::string& tagName)
{
  unsigned int hashedTagName = SDBMHash(tagName);

  // Find the mapping
  Mapping* mapping = (Mapping*)_classMap[hashedTagName];

  // Some mappings don't interact directly with a Trx object
  LOG4CXX_DEBUG(logger, "tag=" << tagName << ", mapping=" << mapping);

  if (mapping && mapping->trxFunc)
  {
    (this->*mapping->trxFunc)();
    _currentMapEntryStack.pop();
    if (_currentMapEntryStack.size() > 0)
    {
      _currentMapEntry = _currentMapEntryStack.top();
    }
  }
}

void
PricingModelMap::storePricingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePricingInformation");

  storeAltPricingInformation(attrs);
  AltPricingTrx* altPricingTrx = dynamic_cast<AltPricingTrx*>(_pricingTrx);
  if (altPricingTrx != nullptr)
  {
    altPricingTrx->altTrxType() = AltPricingTrx::WP;
  }

  const size_t numAtts = attrs.getLength();
  for (size_t i = 0; i < numAtts; ++i)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S96 - Exchange type
    {
      if (FULL_EXCHANGE == xmlValue.c_str())
      {
        _pricingTrx->setSkipGsa(true);
        _pricingTrx->setSkipNeutral(true);
      }
      else if (PARTIAL_EXCHANGE == xmlValue.c_str())
      {
        _pricingTrx->setSkipGsa(true);
        _pricingTrx->setSkipNeutral(true);
        _pricingTrx->setOnlyCheckAgreementExistence(true);
      }
      else
      {
        LOG4CXX_WARN(logger,
                     "Unsupported value: " << xmlValue.c_str()
                                           << " for attribute: " << xmlStr.c_str());
      }
      break;
    }
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::storeAltPricingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeAltPricingInformation");

  AltPricingTrx* altPricingTrx = nullptr;

  _dataHandle.get(altPricingTrx);

  if (altPricingTrx)
  {
    _pricingTrx = altPricingTrx;
  }
  else
  {
    return;
  }

  PricingRequest* request;
  _pricingTrx->dataHandle().get(request); // lint !e530
  _pricingTrx->setRequest(request);
  _trx = _pricingTrx;

  const int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    tryExtractVersion(XMLChString(attrs.getLocalName(i)), XMLChString(attrs.getValue(i)));
  }
}

void
PricingModelMap::storeStructuredRuleInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeStructuredRuleInformation");

  StructuredRuleTrx* structuredRuleTrx = 0;

  _dataHandle.get(structuredRuleTrx);

  if (fallback::fallbackDisableStructuredRule(structuredRuleTrx))
  {
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "STRUCTURED RULE TRANSACTION DISABLED BY FALLBACK");
  }

  if (structuredRuleTrx)
  {
    _pricingTrx = structuredRuleTrx;
    structuredRuleTrx->createMultiPassangerFCMapping();
  }
  else
  {
    return;
  }

  PricingRequest* request;
  _pricingTrx->dataHandle().get(request);
  _pricingTrx->setRequest(request);
  request->markAsSFR();
  _pricingTrx->dataHandle().get(_paxType);
  _mpo = smp::INFO;
  _trx = _pricingTrx;
}

void
PricingModelMap::storeNoPNRPricingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeNoPnrPricingInformation");

  NoPNRPricingTrx* noPNRPricingTrx = nullptr;

  _dataHandle.get(noPNRPricingTrx);

  if (noPNRPricingTrx)
  {
    noPNRPricingTrx->altTrxType() = AltPricingTrx::WPA;
    _pricingTrx = noPNRPricingTrx;
    _pricingTrx->noPNRPricing() = true;

    if (nullptr == _options)
    {
      NoPNRPricingOptions* noPNRPricingOptions = nullptr;
      _dataHandle.get(noPNRPricingOptions);

      if (nullptr != noPNRPricingOptions)
      {
        _options = noPNRPricingOptions;
        noPNRPricingTrx->setOptions(noPNRPricingOptions);
      }
    }
  }
  if (!_pricingTrx)
  {
    return;
  }

  PricingRequest* request;
  _pricingTrx->dataHandle().get(request); // lint !e530
  _pricingTrx->setRequest(request);
  _trx = _pricingTrx;
}

void
PricingModelMap::storeRexPricingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeRexPricingInformation");

  if (!Global::allowHistorical())
  {
    LOG4CXX_ERROR(logger, "Can not process RexPricingRequest");
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "CAN NOT PROCESS REXPRICINGREQUEST");
  }

  RexBaseTrx::RequestTypes refundTypes = getRefundTypes(attrs);

  if (refundTypes.first == FULL_EXCHANGE || refundTypes.first == PARTIAL_EXCHANGE ||
      refundTypes.first == TAG_10_EXCHANGE || refundTypes.first == AGENT_PRICING_MASK ||
      refundTypes.first == NON_FLIGHT_EXCHANGE)
    prepareExchangePricingTrx(refundTypes.first);

  else if (refundTypes.first == MULTIPLE_EXCHANGE)
    prepareMultiExchangePricingTrx();

  else if (refundTypes.first == AUTOMATED_REFUND)
    prepareRefundPricingTrx(refundTypes);

  else
    prepareRexPricingTrx(refundTypes);

  BaseExchangeTrx* baseTrx = static_cast<BaseExchangeTrx*>(_pricingTrx);
  baseTrx->setRexPrimaryProcessType(_primaryProcessType);
  baseTrx->setRexSecondaryProcessType(_secondaryProcessType);
  _isRexTrx = true;
}

void
PricingModelMap::savePricingInformation()
{
  LOG4CXX_DEBUG(logger, "In savePricingInformation");

  // Create new Itinerary
  if (createItin())
  {
    saveItin();
    if (_options && _options->isTicketingDateOverrideEntry())
      checkFuturePricingCurrency(_pricingTrx, _itin, _options);

    LOG4CXX_INFO(logger, "The Travel Seg has a size " << _pricingTrx->travelSeg().size());
    LOG4CXX_INFO(logger, "The itin has a size " << _pricingTrx->itin().size());
    LOG4CXX_INFO(logger, "The PaxType has a size " << _pricingTrx->paxType().size());
  }

  if (_pricingTrx->getRequest()->isSFR() && !fallback::fixed::checkArunkForSfr())
  {
    checkArunkSegmentForSfrReq();
  }

  if (!_isRexTrx && _pricingTrx->paxType().empty())
  {
    _paxType = _reqXmlValidator.getDefaultPaxType(_pricingTrx);
    if (_paxType != nullptr)
    {
      savePassengerInformation();
      _paxType = nullptr;
    }
  }

  if (!_isRexTrx)
  {
    NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(_pricingTrx);
    if (noPNRTrx)
      _reqXmlValidator.setForcedStopoverForNoPnrPricing(_itin);
  }

  if (_multiExcTrx)
  {
    _trx = _multiExcTrx;
    // should never happen, check newPricingTrx and request not 0
    if (_multiExcTrx->newPricingTrx() == nullptr)
      throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION);
    _pricingTrx = _multiExcTrx->newPricingTrx();
    checkRegularPricingTrxReqInfo(*_pricingTrx);

    PricingRequest* request = _pricingTrx->getRequest();
    _multiExcTrx->diagPricingTrx() = _multiExcTrx->newPricingTrx();

    if (request->diagnosticNumber() != DiagnosticNone)
    {
      DiagnosticTypes diagType = static_cast<DiagnosticTypes>(request->diagnosticNumber());

      std::vector<std::string>::const_iterator diagArgI = request->diagArgData().begin();
      const std::vector<std::string>::const_iterator diagArgIEnd = request->diagArgData().end();

      for (; diagArgI != diagArgIEnd; diagArgI++)
      {
        if (*diagArgI == "IT1")
        {
          _multiExcTrx->skipNewPricingTrx() = true;
          _multiExcTrx->skipExcPricingTrx2() = true;
          if (_multiExcTrx->excPricingTrx1() == nullptr)
          {
            _multiExcTrx->skipExcPricingTrx1() = true;
            throw ErrorResponseException(
                ErrorResponseException::UNKNOWN_EXCEPTION); // request not valid
          }
          _multiExcTrx->diagPricingTrx() = _multiExcTrx->excPricingTrx1();
          break;
        }
        else if (*diagArgI == "IT2")
        {
          _multiExcTrx->skipNewPricingTrx() = true;
          _multiExcTrx->skipExcPricingTrx1() = true;
          if (_multiExcTrx->excPricingTrx2() == nullptr)
          {
            _multiExcTrx->skipExcPricingTrx2() = true;
            throw ErrorResponseException(
                ErrorResponseException::UNKNOWN_EXCEPTION); // request not valid
          }
          _multiExcTrx->diagPricingTrx() = _multiExcTrx->excPricingTrx2();

          break;
        }
      }
      if (diagArgI == diagArgIEnd)
      {
        _multiExcTrx->skipExcPricingTrx1() = true;
        _multiExcTrx->skipExcPricingTrx2() = true;
        _multiExcTrx->diagPricingTrx() = _multiExcTrx->newPricingTrx();
      }

      _multiExcTrx->diagPricingTrx()->setExcTrxType(PricingTrx::ME_DIAG_TRX);
      if (_multiExcTrx->diagPricingTrx() != _multiExcTrx->newPricingTrx())
      {
        // copy diagParam ...
        _multiExcTrx->diagPricingTrx()->getRequest()->diagnosticNumber() = diagType;
        std::vector<std::string>& diagArgDataCopiedTo =
            _multiExcTrx->diagPricingTrx()->getRequest()->diagArgData();
        diagArgDataCopiedTo.insert(diagArgDataCopiedTo.end(),
                                   request->diagArgData().begin(),
                                   request->diagArgData().end());
      }
    }
    else
    {
      if (_multiExcTrx->excPricingTrx2() == nullptr)
        _multiExcTrx->skipExcPricingTrx2() = true;
      else
        checkRegularPricingTrxReqInfo(*_multiExcTrx->excPricingTrx2());

      if (_multiExcTrx->excPricingTrx1() == nullptr)
        _multiExcTrx->skipExcPricingTrx1() = true;
      else
        checkRegularPricingTrxReqInfo(*_multiExcTrx->excPricingTrx1());
    }
  }
  else
    checkTrxRequiredInfo();

  setFreqFlyerStatus();

  if (!_isRexTrx)
  {
    _reqXmlValidator.checkRequestCrossDependencies(_pricingTrx);

    _reqXmlValidator.validateCxrOverride(_pricingTrx, _itin);

    if (TrxUtil::isMultiTicketPricingEnabled(*_pricingTrx) &&
        _pricingTrx->getRequest()->isMultiTicketRequest())
      _reqXmlValidator.validateMultiTicketRequest(_pricingTrx);
  }

  if (!fallback::azPlusUpExc(_pricingTrx) && _pricingTrx->isExchangeTrx() &&
      _pricingTrx->excTrxType() != PricingTrx::PORT_EXC_TRX)
  {
    const RexBaseRequest* rexRequest = dynamic_cast<RexBaseRequest*>(_pricingTrx->getRequest());
    if (rexRequest &&
        (rexRequest->excDiscounts().isPAEntry() || rexRequest->excDiscounts().isPPEntry()))
    {
      _pricingTrx->getRequest()->discountsNew().clearAmountDiscounts();
      _pricingTrx->getRequest()->discountsNew().clearPercentageDiscounts();
    }
  }

  if (!fallback::excDiscountAmountFix(_pricingTrx))
  {
    if (RexBaseTrx* rexBaseTrx = dynamic_cast<RexBaseTrx*>(_pricingTrx))
    {
      std::vector<DiscountAmount> newAmounts;

      for (const DiscountAmount& discount : rexBaseTrx->getRexRequest().excDiscounts().getAmounts())
      {
        std::vector<FareCompInfo*> fareComponents;

        std::copy_if(rexBaseTrx->exchangeItin()[0]->fareComponent().begin(),
                     rexBaseTrx->exchangeItin()[0]->fareComponent().end(),
                     std::back_inserter(fareComponents),
                     [&discount](FareCompInfo* fc)
                     {
          const std::vector<TravelSeg*>& travelSeg = fc->fareMarket()->travelSeg();
          return travelSeg.front()->segmentOrder() >= discount.startSegmentOrder &&
                 travelSeg.back()->segmentOrder() <= discount.endSegmentOrder;
        });

        const MoneyAmount totalFareAmount = std::accumulate(fareComponents.begin(),
                                                            fareComponents.end(),
                                                            0.0,
                                                            [](MoneyAmount v, FareCompInfo* fc)
                                                            { return fc->fareCalcFareAmt() + v; });

        if (totalFareAmount > EPSILON || fallback::excDiscountsFixDivideByZero(_pricingTrx))
        {
          std::transform(fareComponents.begin(),
                         fareComponents.end(),
                         std::back_inserter(newAmounts),
                         [&discount, totalFareAmount](FareCompInfo* fc)
                         {
            // remove the assert on fallback removal
            TSE_ASSERT(totalFareAmount != 0.0);
            return DiscountAmount(discount.amount * fc->fareCalcFareAmt() / totalFareAmount,
                                  discount.currencyCode,
                                  fc->fareMarket()->travelSeg().front()->segmentOrder(),
                                  fc->fareMarket()->travelSeg().back()->segmentOrder());
          });
        }
      }

      rexBaseTrx->getRexRequest().excDiscounts().setAmounts(newAmounts);
    }
  }
}

void
PricingModelMap::storeXrayInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeXrayInformation");

  const size_t numAtts = attrs.getLength();
  for (size_t i = 0; i < numAtts; ++i)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // MID
    {
      _xrayMessageId.assign(xmlValue.c_str());
      LOG4CXX_DEBUG(logger, "MID = " << xmlValue.c_str());
      break;
    }
    case 2: // CID
    {
      _xrayConversationId.assign(xmlValue.c_str());
      LOG4CXX_DEBUG(logger, "CID = " << xmlValue.c_str());
      break;
    }
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveXrayInformation()
{
  LOG4CXX_DEBUG(logger, "In saveXrayInformation");

  if (xray::xrayEnabledCfg.isValid(_pricingTrx))
  {
    _pricingTrx->assignXrayJsonMessage(xray::JsonMessagePtr(
        new xray::JsonMessage(_xrayMessageId, _xrayConversationId)));
  }
  else
    LOG4CXX_WARN(logger, "Xray tracking is disabled");
}

bool
PricingModelMap::createItin()
{
  if (_pricingTrx == nullptr)
    return false;

  if (_isExchangeItin)
  {
    if (_excItin != nullptr)
      return true;

    ExcItin* exchangeItin = nullptr;
    _pricingTrx->dataHandle().get(exchangeItin);
    if (exchangeItin == nullptr)
      return false;

    if (_processExchangePricingTrx)
    {
      ExchangePricingTrx* exchangePricingTrx = getExchangePricingTrx();
      if (exchangePricingTrx == nullptr)
        return false;
      exchangePricingTrx->exchangeItin().push_back(exchangeItin);
    }
    else
    {
      RexBaseTrx* rexBaseTrx = getRexBaseTrx();
      if (rexBaseTrx == nullptr)
        return false;
      rexBaseTrx->exchangeItin().push_back(exchangeItin);
      exchangeItin->rexTrx() = rexBaseTrx;
    }
    _excItin = exchangeItin;
  }
  else
  {
    if (_itin != nullptr) // There is existing one
      return true;
    RexPricingTrx* rexPricingTrx = getRexPricingTrx();
    if (rexPricingTrx)
    {
      RexNewItin* newItin = _pricingTrx->dataHandle().create<RexNewItin>();
      newItin->rexTrx() = rexPricingTrx;
      _itin = newItin;
    }
    else
    {
      ExchangePricingTrx* exchangePricingTrx = nullptr;
      if (_processExchangePricingTrx)
      {
        exchangePricingTrx = getExchangePricingTrx();
      }

      if (exchangePricingTrx)
      {
        ExchangeNewItin* newItin = _pricingTrx->dataHandle().create<ExchangeNewItin>();
        newItin->trx() = exchangePricingTrx;
        _itin = newItin;
      }
      else
        _pricingTrx->dataHandle().get(_itin);
    }
    _pricingTrx->itin().push_back(_itin);
  }

  return true;
}

void
PricingModelMap::checkNoMatchItin(NoPNRPricingTrx& trx)
{
  TravelSeg* seg = trx.travelSeg().front();
  trx.noRBDItin() = seg && seg->getBookingCode().empty();
}

void
PricingModelMap::saveItin()
{
  Itin* itin = _isExchangeItin ? _excItin : _itin;
  DateTime& ticketingDate = _pricingTrx->ticketingDate(); // is _pricingTrx->ticketingDate() valid.
  if (!_isRexTrx)
  {
    _reqXmlValidator.validateItin(_pricingTrx, *itin);
    _reqXmlValidator.validateTicketingDate(_pricingTrx, itin);
  }
  DataHandle& dataHandle = _trx->dataHandle();

  bool isDatedSegment = false;
  bool isUndatedSegment = false;

  // Truncate any trailing ARUNKs
  int tvlSegSize = itin->travelSeg().size();
  while (tvlSegSize > 0)
  {
    TravelSeg* tvlSeg = itin->travelSeg()[tvlSegSize - 1];
    if (tvlSeg->segmentType() == Arunk)
    {
      itin->travelSeg().pop_back();

      if (!_isExchangeItin)
        _pricingTrx->travelSeg().pop_back();

      tvlSegSize = itin->travelSeg().size();
    }
    else
      break;
  }

  bool isNoPNRTrx = (dynamic_cast<NoPNRPricingTrx*>(_pricingTrx) != nullptr);

  // Open segments come over with a date of 1966-01-*.  These need to be
  // adjusted
  for (int i = 0; i < tvlSegSize; i++)
  {
    TravelSeg* tvlSeg = itin->travelSeg()[i];

    if (tvlSeg->segmentType() == Arunk)
    {
      if (i > 0) // Arunk is never be the first segment in itin.
      {
        tvlSeg->departureDT() = itin->travelSeg()[i - 1]->arrivalDT();
      }
    }
    else if (tvlSeg->segmentType() == Open)
    {
      if (i == 0)
      {
        if (!_isRexTrx)
        {
          if (tvlSeg->pssDepartureDate().empty()) // for PO departure date of 1st seg was set
          // earlier
          {
            tvlSeg->hasEmptyDate() = true;
            isUndatedSegment = true;
          }
          else
          {
            isDatedSegment = true;
          }
        }
        else
        {
          if (tvlSeg->departureDT() < ticketingDate &&
              !(isNoPNRTrx && tvlSeg->departureDT().day() == ticketingDate.day() &&
                tvlSeg->departureDT().month() == ticketingDate.month() &&
                tvlSeg->departureDT().year() == ticketingDate.year())) // somewhere in 1966
          {
            tvlSeg->departureDT() = ticketingDate; // make it today
            tvlSeg->hasEmptyDate() = true;
            isUndatedSegment = true;
          }
          else
          {
            isDatedSegment = true;
          }
        }

        if (tvlSeg->arrivalDT() < ticketingDate) // somewhere in 1966
          tvlSeg->arrivalDT() = tvlSeg->departureDT();

        if (i + 1 < tvlSegSize)
        {
          TravelSeg* nextSeg = itin->travelSeg()[i + 1];
          if (!tvlSeg->origin())
          {
            tvlSeg->origin() = dataHandle.getLoc(tvlSeg->origAirport(), nextSeg->departureDT());

            // Catch invalid date (1966-01-*)
            if (!tvlSeg->origin())
            {
              tvlSeg->origin() = dataHandle.getLoc(tvlSeg->origAirport(), ticketingDate);
              tvlSeg->departureDT() = ticketingDate;
            }
          }
          if (!tvlSeg->destination())
          {
            tvlSeg->destination() = dataHandle.getLoc(tvlSeg->destAirport(), nextSeg->arrivalDT());

            // Catch invalid date (1966-01-*)
            if (!tvlSeg->destination())
            {
              tvlSeg->destination() = dataHandle.getLoc(tvlSeg->destAirport(), ticketingDate);
              tvlSeg->arrivalDT() = ticketingDate;
            }
          }
        }
        else
        {
          if (!tvlSeg->origin())
          {
            tvlSeg->origin() = dataHandle.getLoc(tvlSeg->origAirport(), tvlSeg->departureDT());
          }
          if (!tvlSeg->destination())
          {
            tvlSeg->destination() = dataHandle.getLoc(tvlSeg->destAirport(), tvlSeg->arrivalDT());
          }
        }
      }
      else
      {
        // Get a previous segment
        TravelSeg* prevSeg;
        prevSeg = itin->travelSeg()[i - 1];
        bool caculatedDate = false;
        if (tvlSeg->departureDT() < ticketingDate &&
            !(isNoPNRTrx && tvlSeg->departureDT().day() == ticketingDate.day() &&
              tvlSeg->departureDT().month() == ticketingDate.month() &&
              tvlSeg->departureDT().year() == ticketingDate.year()))
        {
          isUndatedSegment = true;
          tvlSeg->hasEmptyDate() = true;
          DateTime newDate(prevSeg->departureDT().year(),
                           prevSeg->departureDT().month(),
                           prevSeg->departureDT().day(),
                           prevSeg->departureDT().hours() + 24,
                           prevSeg->departureDT().minutes(),
                           prevSeg->departureDT().seconds());
          tvlSeg->departureDT() = newDate;
          caculatedDate = true;
        }
        else
        {
          isDatedSegment = true;
        }

        if (!tvlSeg->origin())
          tvlSeg->origin() = dataHandle.getLoc(tvlSeg->origAirport(), tvlSeg->departureDT());

        if (prevSeg->segmentType() == Arunk)
        {
          if (tvlSeg->arrivalDT() < ticketingDate) // No arrival date specified
            tvlSeg->arrivalDT() = tvlSeg->departureDT();
        }
        else
        {
          if (tvlSeg->arrivalDT() < ticketingDate) // No arrival date specified
          {
            if (caculatedDate) // Departure date is caculated from prev segment,
            { // caculate arrival date two.
              DateTime newDate(prevSeg->arrivalDT().year(),
                               prevSeg->arrivalDT().month(),
                               prevSeg->arrivalDT().day(),
                               prevSeg->arrivalDT().hours() + 24,
                               prevSeg->arrivalDT().minutes(),
                               prevSeg->arrivalDT().seconds());
              tvlSeg->arrivalDT() = newDate;
            }
            else
              tvlSeg->arrivalDT() = tvlSeg->departureDT();
          }
        }

        if (!tvlSeg->destination())
          tvlSeg->destination() = dataHandle.getLoc(tvlSeg->destAirport(), tvlSeg->arrivalDT());
      }
    }

    if (!tvlSeg->origin())
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                   "UNRECOGNIZED ORIGIN");
    if (!tvlSeg->destination())
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                   "UNRECOGNIZED DESTINATON");

    adjustFlownSegArrivalTime(itin, tvlSeg, i);
    if (!_isRexTrx)
      _reqXmlValidator.validateDepartureDate(_pricingTrx, itin, tvlSeg, i);
  }

  if (true == isDatedSegment && true == isUndatedSegment)
  {
    // Mixed date
    itin->dateType() = Itin::PartialDate;
  }
  else if (true == isDatedSegment)
  {
    // All dates exists
    itin->dateType() = Itin::FullDate;
  }
  else if (true == isUndatedSegment)
  {
    // No date
    itin->dateType() = Itin::NoDate;
  }

  if (_intlItin)
  {
    setIntlBoardOffCity(*itin);
  }

  // We're guaranteed not to have back-to-back ARUNKs.  ARUNKs will
  // assume the board, off and departure date characteristics of previous
  // and following travel segment.
  for (int i = 1; i < tvlSegSize; i++)
  {
    TravelSeg* tvlSeg = itin->travelSeg()[i];
    if (tvlSeg->segmentType() == Arunk)
    {
      tvlSeg->origAirport() = itin->travelSeg()[i - 1]->destAirport();
      tvlSeg->origin() = itin->travelSeg()[i - 1]->destination();
      tvlSeg->boardMultiCity() = itin->travelSeg()[i - 1]->offMultiCity();
      if (i + 1 < tvlSegSize)
      {
        tvlSeg->destAirport() = itin->travelSeg()[i + 1]->origAirport();

        tvlSeg->destination() = itin->travelSeg()[i + 1]->origin();
        tvlSeg->offMultiCity() = itin->travelSeg()[i + 1]->boardMultiCity();
        tvlSeg->departureDT() = itin->travelSeg()[i + 1]->departureDT();
        tvlSeg->bookingDT() = itin->travelSeg()[i + 1]->bookingDT();
      }
    }
  }

  // Remove Arunk with same board and Off Cities
  if (_isExchangeItin)
  {
    uint16_t numTvlSeg = _excItin->travelSeg().size();

    removeArunkInSameCity(_excItin->travelSeg());

    if (numTvlSeg > itin->travelSeg().size()) // Some Arunk removed
    {
      std::vector<FareCompInfo*>::iterator iter = _excItin->fareComponent().begin();
      for (; iter != _excItin->fareComponent().end(); ++iter)
      {
        FareCompInfo* fc = *iter;
        if (fc->fareMarket())
          removeArunkInSameCity(fc->fareMarket()->travelSeg());
      }
    }
  }
  else
  {
    removeArunkInSameCity(itin->travelSeg());
    removeArunkInSameCity(_pricingTrx->travelSeg());
  }

  if (Billing* billing = _pricingTrx->billing())
  {
    if (billing->requestPath() == SWS_PO_ATSE_PATH)
    {
      _reqXmlValidator.validateItinForFlownSegments(*itin);
    }
  }
  else
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "ILL FORMED REQUEST");
  }

  // Set up ConsolidatorPlusUp object
  if (_itin && _consolidatorPlusUpAmount > 0.0)
  {
    ConsolidatorPlusUp* cPlusUp;
    dataHandle.get(cPlusUp);

    if (cPlusUp)
    {
      cPlusUp->initialize(*_pricingTrx,
                          _consolidatorPlusUpAmount,
                          _consolidatorPlusUpCurrency,
                          _consolidatorPlusUpTktDesignator);

      _itin->consolidatorPlusUp() = cPlusUp;
    }

    // Reset work variables
    _consolidatorPlusUpAmount = 0.0;
    _consolidatorPlusUpCurrency = EMPTY_STRING();
    _consolidatorPlusUpTktDesignator = EMPTY_STRING();
  }

  if (!_isExchangeItin && _itin)
  {
    if (_multiExcTrx != nullptr && _pricingTrx != _multiExcTrx->newPricingTrx())
    {
      // FareCalcCurrency on EX1 or EX2
      if (!_excItinFareCalcCurrency.empty())
      {
        _itin->calcCurrencyOverride() = _excItinFareCalcCurrency;
        _excItinFareCalcCurrency = "";
      }
    }
    else if (!_newItinFareCalcCurrency.empty())
    {
      _itin->calcCurrencyOverride() = _newItinFareCalcCurrency;
      _newItinFareCalcCurrency = "";
    }
  }

  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(_pricingTrx);
  if (noPNRTrx)
  {
    if (!_isRexTrx)
      _reqXmlValidator.checkItinBookingCodes(*noPNRTrx);
    checkNoMatchItin(*noPNRTrx);
  }
  _isMissingArunkForPO = false; // prepare for Reissue and Exchange transaction

  if (!fallback::azPlusUp(_pricingTrx))
  {
    if (_itin)
    {
      CommonParserUtils::addZeroDiscountForSegsWithNoDiscountIfReqHasDiscount(
          _pricingTrx->getRequest()->discountsNew(), _itin->travelSeg(), _pricingTrx->isMip());
    }
  }
}

void
PricingModelMap::storeAgentInformation(const xercesc::Attributes& attrs)

{
  LOG4CXX_DEBUG(logger, "In storeAgentInformation");

  Agent* agent = createAgent();
  if (agent == nullptr)
    throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION);

  if (_options == nullptr)
  {
    _trx->dataHandle().get(_options);
    _pricingTrx->setOptions(_options);
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A10 - Agent city
      agent->agentCity() = xmlValue.c_str();
      break;

    case 2: // A20 - Travel agent PCC
      agent->tvlAgencyPCC() = xmlValue.c_str();
      break;
    case 3: // A21 - Main travel agency PCC
    {
      char tmpBuf[10];
      strcpy(tmpBuf, xmlValue.c_str());
      if (tmpBuf[3] == ' ')
        tmpBuf[3] = '\0';
      agent->mainTvlAgencyPCC() = tmpBuf;
    }
    break;
    case 4: // AB0 - Agency IATA number
      agent->tvlAgencyIATA() = xmlValue.c_str();
      break;
    case 5: // AB1 - Home agency IATA number
      agent->homeAgencyIATA() = xmlValue.c_str();
      break;
    case 6: // A90 - Agent function code
      agent->agentFunctions() = xmlValue.c_str();
      break;
    case 7: // N0G - Agent duty code
      agent->agentDuty() = xmlValue.c_str();
      break;
    case 8: // A80 - Airline department
      agent->airlineDept() = xmlValue.c_str();
      break;
    case 9: // B00 - Originating carrier
      agent->cxrCode() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;
    case 10: // C40 - Agent currency code
      agent->currencyCodeAgent() = xmlValue.c_str();
      break;
    case 11: // Q01 - coHost ID
      agent->coHostID() = atoi(xmlValue.c_str());
      break;
    case 12: // N0L - Agent commission type
      agent->agentCommissionType() = xmlValue.c_str();
      break;
    case 13: // C6C - Agent commission amount
      agent->agentCommissionAmount() = atol(xmlValue.c_str());
      break;
    //        case 14:                // PAV - JAL/AXESS User
    //            agent->jalAxess() = (xmlValue.c_str())[0];
    //            break;
    case 15: // AE0 - Vendor CRS Code
      if (MCPCarrierUtil::isPseudoCarrier(xmlValue.c_str()))
      {
        _trx->mcpCarrierSwap() = true;
        if (_pricingTrx->billing())
          _pricingTrx->billing()->partitionID() =
              MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      }
      // LATAM MCP-S
      if (!fallback::neutralToActualCarrierMapping(_pricingTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        if (_pricingTrx->billing())
          _pricingTrx->billing()->partitionID() =
              MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());

        agent->vendorCrsCode() = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
      }
      else
        agent->vendorCrsCode() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;
    case 16:
      agent->airlineIATA() = xmlValue.c_str();
      break;
    case 17: // AE1 - Office Designator
      agent->officeDesignator() = xmlValue.c_str();
      break;
    case 18: // AE2 - Office/Station code
      agent->officeStationCode() = xmlValue.c_str();
      break;
    case 19: // AE3 - Default ticketing carrier
      agent->defaultTicketingCarrier() = xmlValue.c_str();
      break;
    case 20: // OAL - Original tkt agent location
      if (_isExchangeItin && (_pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX ||
                              _pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX))
      {
        RexBaseRequest& rexBaseRequest = static_cast<RexBaseRequest&>(*_pricingTrx->getRequest());
        rexBaseRequest.setOriginalTicketAgentLocation(xmlValue.c_str());
      }
      break;
    case 21: // AE4 - Channel code
      agent->airlineChannelCode() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  if (!_isExchangeItin && (_pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX ||
                           _pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX))
    static_cast<RexBaseTrx&>(*_pricingTrx).setUpSkipSecurityForExcItin();
}

Agent*
PricingModelMap::createAgent()
{
  if (_pricingTrx->getRequest() == nullptr)
    return nullptr;

  Agent* agent = nullptr;
  _pricingTrx->dataHandle().get(agent); // lint !e530

  if (_isExchangeItin)
  {
    RexBaseRequest* rexBaseRequest = dynamic_cast<RexBaseRequest*>(_pricingTrx->getRequest());
    if (rexBaseRequest != nullptr)
      rexBaseRequest->prevTicketIssueAgent() = agent;
    else
      _pricingTrx->getRequest()->ticketingAgent() = agent;
  }
  else
    _pricingTrx->getRequest()->ticketingAgent() = agent;

  return agent;
}

Agent*
PricingModelMap::getAgent(bool forPrevTicketIssueAgent)
{
  if (_pricingTrx->getRequest() == nullptr)
    return nullptr;

  if (forPrevTicketIssueAgent)
  {
    RexBaseRequest* rexBaseRequest = dynamic_cast<RexBaseRequest*>(_pricingTrx->getRequest());
    if (rexBaseRequest != nullptr)
      return rexBaseRequest->prevTicketIssueAgent();
  }

  return _pricingTrx->getRequest()->ticketingAgent();
}

void
PricingModelMap::saveAgentInformation()
{
  LOG4CXX_DEBUG(logger, "In saveAgentInformation");

  Agent* agent = getAgent(_isExchangeItin);

  if (agent == nullptr)
    throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION);

  //
  // Per marketing requirement the Customer Record can ONLY use agent city and tvlAgencyPCC
  // Home Psuedo City(Z360) is not Valid for branch(PCC-2N72)
  //
  // Change to use multiTranport table after the conversation with Vladi, Darrin and Gary
  // on 9-27 due to PL 10328
  //
  // For PCC-TP4A located near EAP airport that serves two nations
  // the POS needs to be correct nation

  //

  LocCode agentCity;
  DataHandle& dataHandle = _trx->dataHandle();

  const Loc* agentLocation = dataHandle.getLoc(agent->agentCity(), time(nullptr));

  std::vector<Customer*> custList = dataHandle.getCustomer(agent->tvlAgencyPCC());

  if (custList.empty())
  {
    if (agent->tvlAgencyPCC().size() == 4)
    {
      std::stringstream msg;
      msg << "Customer record: '" << agent->tvlAgencyPCC() << "' AGENT_PCC_NON_EXISTENT";
      LOG4CXX_ERROR(logger, msg.str());
    }
    agent->agentLocation() = agentLocation;

    if (!agent->agentLocation())
      throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);

    LOG4CXX_DEBUG(logger, "End of saveAgentInformation");
    return;
  }
  agent->agentTJR() = custList.front();
  if (agent->agentCity().empty() && _isExchangeItin)
  {
    agent->agentCity() = custList.front()->aaCity();
    agentLocation = dataHandle.getLoc(agent->agentCity(), time(nullptr));
  }

  agentCity = dataHandle.getMultiTransportCity(agent->agentCity());
  agent->agentLocation() = dataHandle.getLoc(agentCity, time(nullptr));

  if (agent->agentLocation() && agentLocation)
  {
    if (agent->agentLocation()->nation() != agentLocation->nation())
      agent->agentLocation() = agentLocation;
  }

  if (!agent->agentLocation())
    agent->agentLocation() = agentLocation;

  LOG4CXX_DEBUG(logger, "End of saveAgentInformation");
}

void
PricingModelMap::updateAgentInformation()
{
  LOG4CXX_DEBUG(logger, "In updateAgentInformation");

  if (!_trx->dataHandle().isHistorical() || _isExchangeItin)
    return;

  Agent* agent = getAgent(_isExchangeItin);

  if (!agent || !agent->agentLocation())
    return;

  if (!LocUtil::isFormerNetherlandsAntilles(agent->agentLocation()->nation()))
    return;

  DataHandle& dataHandle = _trx->dataHandle();

  const Loc* agentLocation =
      dataHandle.getLoc(agent->agentCity(), _pricingTrx->getRequest()->ticketingDT());

  if (agentLocation)
    agent->agentLocation() = agentLocation;
}

void
PricingModelMap::updatePrevTicketIssueAgent()
{
  LOG4CXX_DEBUG(logger, "In updatePrevTicketIssueAgent");

  Agent* agent = getAgent(_isExchangeItin);

  if (!agent)
  {
    Agent* ticketingAgent = getAgent(false);

    if (!ticketingAgent || !ticketingAgent->agentLocation())
      return;

    Agent* prevTicketIssueAgent = createAgent();

    *prevTicketIssueAgent = *ticketingAgent;

    agent = prevTicketIssueAgent;
  }

  if (!agent->agentLocation())
    return;

  if (!LocUtil::isFormerNetherlandsAntilles(agent->agentLocation()->nation()))
    return;

  RexBaseTrx* rexBaseTrx = getRexBaseTrx();

  if (rexBaseTrx)
  {
    DataHandle& dataHandle = _trx->dataHandle();
    const Loc* agentLocation =
        dataHandle.getLoc(agent->agentCity(), rexBaseTrx->originalTktIssueDT());

    if (agentLocation)
      agent->agentLocation() = agentLocation;
  }
}

void
PricingModelMap::storeBillingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeBillingInformation");

  Billing* billing;
  _pricingTrx->dataHandle().get(billing); // lint !e530

  _pricingTrx->billing() = billing;

  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A20 - User pseudoCity code
      billing->userPseudoCityCode() = xmlValue.c_str();
      break;
    case 2: // Q03 - User station
      billing->userStation() = xmlValue.c_str();
      break;
    case 3: // Q02 - User branch
      billing->userBranch() = xmlValue.c_str();
      break;
    case 4: // AE0 - Partition ID
      if (MCPCarrierUtil::isPseudoCarrier(xmlValue.c_str()))
      {
        _trx->mcpCarrierSwap() = true;
        Agent* agent = getAgent(_isExchangeItin);
        if (agent)
        {
          agent->cxrCode() = MCPCarrierUtil::swapToActual(_trx, agent->cxrCode());
          agent->vendorCrsCode() = MCPCarrierUtil::swapToActual(_trx, agent->vendorCrsCode());
        }
      }
      // LATAM MCP-S
      if (!fallback::neutralToActualCarrierMapping(_pricingTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        Agent* agent = getAgent(_isExchangeItin);
        if (agent)
        {
          agent->cxrCode() = MCPCarrierUtil::swapFromNeutralToActual(agent->cxrCode());
          agent->vendorCrsCode() = MCPCarrierUtil::swapFromNeutralToActual(agent->vendorCrsCode());
        }
        billing->partitionID() = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
      }
      else
        billing->partitionID() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      if (MCPCarrierUtil::isIAPCarrierRestricted(billing->partitionID()))
        throw ErrorResponseException(ErrorResponseException::MCP_IAP_RESTRICTED,
                                     ("UNABLE TO PROCESS-ENTRY RESTRICTED IN " +
                                      billing->partitionID() + " PARTITION").c_str());
      break;
    case 5: // AD0 - User set address
      billing->userSetAddress() = xmlValue.c_str();
      break;
    case 6: // C20 - Parent service name
      billing->parentServiceName() = xmlValue.c_str();
      break;
    case 7: // A22 - aaaCity
      billing->aaaCity() = xmlValue.c_str();
      break;
    case 8: // AA0 - agentSine
      billing->aaaSine() = xmlValue.c_str();
      break;
    case 9: // A70 - Action code
    {
      std::string actionCode = xmlValue.c_str();
      billing->actionCode() = actionCode;

      // The following code will be changed when a new indicator is passed in for QREX/ARP.
      if ((actionCode.size() > 1) && (actionCode.substr(0, 2) == "WF"))
      {
        _pricingTrx->getRequest()->rexEntry() = true;
        _pricingTrx->setActionCode();
      }
    }
    break;
    case 10: // C01
      billing->clientTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    case 11: // C00
      billing->parentTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    case 12: // C21 - Not parse it
      break;
    case 13: // S0R - Pricing Orchestrator request path
      billing->requestPath() = xmlValue.c_str();
      if (_reqXmlValidator.requestFromPo(_pricingTrx))
        _requestFromPO = true;
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  billing->updateTransactionIds(_pricingTrx->transactionId());
  billing->updateServiceNames(Billing::SVC_PRICING);
}

void
PricingModelMap::saveBillingInformation()

{
  if (!_pricingTrx->billing()->partitionID().empty() &&
      _pricingTrx->billing()->aaaCity().size() < 4)
  {
    if (dynamic_cast<ExchangePricingTrx*>(_pricingTrx) != nullptr ||
        dynamic_cast<MultiExchangeTrx*>(_trx) != nullptr)
    {
      std::string allowAirlineExchange;
      if (!Global::config().getValue(
              "ALLOW_EXCHANGE_FROM_AIRLINE", allowAirlineExchange, "ITIN_SVC"))
      {
        CONFIG_MAN_LOG_KEY_ERROR(logger, "ALLOW_EXCHANGE_FROM_AIRLINE", "ITIN_SVC");
      }
      if (allowAirlineExchange == "N")
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                             "CAN NOT PROCESS EXCHANGE FROM AIRLINE");
    }
  }
  if (!_isRexTrx)
    _reqXmlValidator.getAgentLocationAndCurrency(_pricingTrx);

  if (!_pricingTrx->getRequest()->ticketingAgent()->agentLocation())
    throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
  LOG4CXX_DEBUG(logger, "In saveBillingInformation");
}

void
PricingModelMap::storeRequestedFareBasisInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeRequestedFareBasisInformation");

  enum RfbAttrBit : uint8_t
  {
    RFB_B50 = 0,
    RFB_B70,
    RFB_S37,
    RFB_B09,
    RFB_S89,
    RFB_S90,
    RFB_C51,
    RFB_ATTR_SIZE
  };
  std::bitset<RFB_ATTR_SIZE> attrFlag;
  TravelSeg::RequestedFareBasis reqFareBasis;

  reqFareBasis.amount = 0.0;
  _isSpecificFareBasis = true;

  const int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
      case 1: // B50
        reqFareBasis.fareBasisCode = xmlValue.c_str();
        attrFlag.set(RFB_B50);
        break;
      case 2: // B70
        reqFareBasis.passengerCode = xmlValue.c_str();
        attrFlag.set(RFB_B70);
        break;
      case 3: // S37
        reqFareBasis.vendor = xmlValue.c_str();
        attrFlag.set(RFB_S37);
        break;
      case 4: // B09
        reqFareBasis.carrier = xmlValue.c_str();
        attrFlag.set(RFB_B09);
        break;
      case 5: // S89
        reqFareBasis.tariff = xmlValue.c_str();
        attrFlag.set(RFB_S89);
        break;
      case 6: // S90
        reqFareBasis.rule = xmlValue.c_str();
        attrFlag.set(RFB_S90);
        break;
      case 7: // C51
        reqFareBasis.amount = std::stod(xmlValue.c_str());
        attrFlag.set(RFB_C51);
        break;
      default:
        LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
        break;
    }
  }

  const bool onlyFbc = (attrFlag.count() == 1) && attrFlag[RFB_B50];

  if (!onlyFbc && !attrFlag.all())
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED, "$FORMAT$");

  if (!_specificFbcList.empty())
  {
    if (onlyFbc || _specificFbcList.front().passengerCode.empty())
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED, "$FORMAT$");

    if (tse::alg::contains(_specificFbcList, reqFareBasis))
    {
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED, "$FORMAT$");
    }
  }
  _specificFbcList.push_back(reqFareBasis);
}

bool
PricingModelMap::checkRequestedFareBasisInformation()
{
  if (!_isSpecificFareBasis)
    return false;

  if (!_itin || _itin->travelSeg().empty())
    return false;

  for (TravelSeg* ts : _itin->travelSeg())
  {
    if (!ts->fareBasisCode().empty())
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED, "$FORMAT$");
  }

  if (!_pricingTrx)
    return false;

  PricingRequest* request = _pricingTrx->getRequest();

  if (!forceEnableWPZZ.isValid(_pricingTrx) && request && request->ticketingAgent() &&
      !request->ticketingAgent()->abacusUser())
    return false;

  if (_specificFbcList.empty())
    return false;

  if (_itin->travelSeg().size() > 1)
  {
    const bool onlyFbc = _specificFbcList.front().passengerCode.empty();
    const bool firstOnlyFbc =
        _itin->travelSeg().front()->requestedFareBasis().empty() ||
        _itin->travelSeg().front()->requestedFareBasis().front().passengerCode.empty();

    if (onlyFbc != firstOnlyFbc)
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED, "$FORMAT$");
  }

  if (_pricingTrx->altTrxType() == AltPricingTrx::WPA || _options->bookingCodeOverride() ||
      ((request->isLowFareRequested() || request->isLowFareNoAvailability()) &&
       request->isLowFareNoRebook()))
  {
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                 "$ZZ QUALIFIER MAY NOT BE COMBINED$");
  }

  return true;
}

void
PricingModelMap::saveRequestedFareBasisInSegmentInformation()
{
  if (_currentTvlSeg)
    _currentTvlSeg->requestedFareBasis() = std::move(_specificFbcList);

  _specificFbcList.clear();
}

void
PricingModelMap::storeFlightInformation(const xercesc::Attributes& attrs)
{
  int timeInMins;

  LOG4CXX_DEBUG(logger, "In storeFlightInformation");

  TravelSegType tvlSegType = UnknownTravelSegType;
  DataHandle& dataHandle = _trx->dataHandle();

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 2: // N03 - Segment type
      // This where we find out the type of segment we're dealing with
      tvlSegType = TravelSegType(xmlValue.c_str()[0]);
      if (tvlSegType == Air || tvlSegType == Open)
      {
        AirSeg* airSeg = nullptr;
        dataHandle.get(airSeg);
        _currentTvlSeg = airSeg;
        _airSeg = airSeg;
      }
      else if (tvlSegType == Arunk)
      {
        ArunkSeg* arunkSeg = nullptr;
        dataHandle.get(arunkSeg);
        _currentTvlSeg = arunkSeg;
        _arunkSeg = arunkSeg;
      }
      else if (tvlSegType == Bus)
      {
        dataHandle.get(_airSeg);
        _currentTvlSeg = _airSeg;
        tvlSegType = Air;
        _currentTvlSeg->equipmentType() = BUS;
      }
      else if (tvlSegType == Train)
      {
        dataHandle.get(_airSeg);

        _currentTvlSeg = _airSeg;
        tvlSegType = Air;
        _currentTvlSeg->equipmentType() = TRAIN;
      }
      else if (tvlSegType == Tgv)
      {
        dataHandle.get(_airSeg);
        _currentTvlSeg = _airSeg;
        tvlSegType = Air;
        _currentTvlSeg->equipmentType() = TGV;
      }
      else if (tvlSegType == Ice)
      {
        dataHandle.get(_airSeg);
        _currentTvlSeg = _airSeg;
        tvlSegType = Air;
        _currentTvlSeg->equipmentType() = ICE;
      }
      else if (tvlSegType == Boat)
      {
        dataHandle.get(_airSeg);
        _currentTvlSeg = _airSeg;
        tvlSegType = Air;
        _currentTvlSeg->equipmentType() = BOAT;
      }
      else if (tvlSegType == Lmo)
      {
        dataHandle.get(_airSeg);
        _currentTvlSeg = _airSeg;
        tvlSegType = Air;
        _currentTvlSeg->equipmentType() = LMO;
      }
      else
      {
        if (_pricingTrx->getRequest()->isSFR() && !fallback::fixed::checkArunkForSfr())
        {
          dataHandle.get(_arunkSeg);
          _currentTvlSeg = _arunkSeg;
          tvlSegType = Arunk;
          LOG4CXX_WARN(logger, "Unknown Travel Segment type, defaulting to ARUNK for SFR");
        }
        else
        {
          LOG4CXX_WARN(logger, "Unknown Travel Segment type, defaulting to AIR");
          dataHandle.get(_airSeg);
          _currentTvlSeg = _airSeg;
        }
      }
      _currentTvlSeg->segmentType() = tvlSegType;
      _currentTvlSeg->segmentOrder() = _segmentNumber;
      _currentTvlSeg->setBrandCode(_brandCode);
      _currentTvlSeg->legId() = _legId;
      _legId = -1;
      break;
    default:
      continue;
    }

    break;
  }

  if (_currentTvlSeg == nullptr)
    return;

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q0C - PNR segment
      _pnrSegmentNumber = atoi(xmlValue.c_str());
      if (tvlSegType == Arunk)
        _currentTvlSeg->pnrSegment() = ARUNK_PNR_SEGMENT_ORDER;
      else
        _currentTvlSeg->pnrSegment() = _pnrSegmentNumber;
      break;
    case 2: // N03 - Segment type
      // It has been parsed above
      break;
    case 3: // B00 - Marketing Carrier
      if (_currentTvlSeg->segmentType() == Air || _currentTvlSeg->segmentType() == Open)
      {
        (dynamic_cast<AirSeg*>(_currentTvlSeg))
            ->setMarketingCarrierCode(MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str()));
      }
      break;
    case 4: // Q0B - Marketing Flight number
      if (_currentTvlSeg->segmentType() == Air || _currentTvlSeg->segmentType() == Open)
      {
        (dynamic_cast<AirSeg*>(_currentTvlSeg))->marketingFlightNumber() = atoi(xmlValue.c_str());
      }
      break;
    case 5: // D01 - Departure date
      if (strncmp(xmlValue.c_str(), PSS_OPEN_DATE_BASE, strlen(PSS_OPEN_DATE_BASE)))
      {
        _currentTvlSeg->pssDepartureDate() = xmlValue.c_str();
      }
      _currentTvlSeg->departureDT() = convertDate(xmlValue.c_str());
      break;
    case 6: // D02 - Arrival date
      if (strncmp(xmlValue.c_str(), PSS_OPEN_DATE_BASE, strlen(PSS_OPEN_DATE_BASE)))
      {
        _currentTvlSeg->pssArrivalDate() = xmlValue.c_str();
      }
      _currentTvlSeg->arrivalDT() = convertDate(xmlValue.c_str());
      break;
    case 7: // D00 - Booking date
      _currentTvlSeg->pssBookingDate() = xmlValue.c_str();
      _currentTvlSeg->bookingDT() = convertDate(xmlValue.c_str());
      break;
    case 8: // D31 - Depart time
      _currentTvlSeg->pssDepartureTime() = xmlValue.c_str();
      timeInMins = atoi(xmlValue.c_str());

      _currentTvlSeg->departureDT() = _currentTvlSeg->departureDT() + tse::Hours(timeInMins / 60) +
                                      tse::Minutes(timeInMins % 60) + tse::Seconds(0);

      break;
    case 9: // D32 - Arrival time
      _currentTvlSeg->pssArrivalTime() = xmlValue.c_str();

      timeInMins = atoi(xmlValue.c_str());
      _currentTvlSeg->arrivalDT() = _currentTvlSeg->arrivalDT() + tse::Hours(timeInMins / 60) +
                                    tse::Minutes(timeInMins % 60) + tse::Seconds(0);

      break;
    case 10: // D30 - Booking time
      _currentTvlSeg->pssBookingTime() = xmlValue.c_str();
      timeInMins = atoi(xmlValue.c_str());
      _currentTvlSeg->bookingDT() = _currentTvlSeg->bookingDT() + tse::Hours(timeInMins / 60) +
                                    tse::Minutes(timeInMins % 60) + tse::Seconds(0);
      break;
    case 11: // B30 - Class of service
      _currentTvlSeg->setBookingCode(
          fallback::purgeBookingCodeOfNonAlpha(_trx)
              ? xmlValue.c_str()
              : DataModelMap::purgeBookingCodeOfNonAlpha(xmlValue.c_str()));
      break;
    case 12: // BB0 - Res status
      _currentTvlSeg->resStatus() = xmlValue.c_str();
      break;
    case 13: // A01 - Board city

      _currentTvlSeg->origAirport() = xmlValue.c_str();
      break;
    case 14: // A02 - Off city
      _currentTvlSeg->destAirport() = xmlValue.c_str();
      break;
    case 15: // B01 - Operating carrier code
      if (_currentTvlSeg->segmentType() == Air)
        (dynamic_cast<AirSeg*>(_currentTvlSeg))
            ->setOperatingCarrierCode(MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str()));
      break;
    case 16: // P2X - Eticket
    {
      AirSeg* airSeg = dynamic_cast<AirSeg*>(_currentTvlSeg);
      if (airSeg != nullptr)
      {
        airSeg->eticket() = (strcmp(xmlValue.c_str(), "ET") == 0);
      }
    }
    break;
    case 17: // BB2 - Real Res status e.g. QF, NN, SS, HK
      _currentTvlSeg->realResStatus() = xmlValue.c_str();
      break;
    case 18: // BB3 - marriage status e.g S,E,P
      if (_currentTvlSeg->segmentType() == Air)
      {
        AirSeg* airSeg = dynamic_cast<AirSeg*>(_currentTvlSeg);
        if (airSeg != nullptr)
        {
          airSeg->marriageStatus() = (xmlValue.c_str())[0];
        }
      }
      break;
    case 19:
      _globalDirectionOverride = xmlValue.c_str(); // A60 - Global direction override
      break;
    case 20:
      // B40 - Equipment type
      if(!fallback::fixed::checkArunkForSfr())
      {
        if (_currentTvlSeg->isAir())
        {
          if(!_pricingTrx->getRequest()->isSFR() || _currentTvlSeg->equipmentType().empty() )
            _currentTvlSeg->equipmentType() = xmlValue.c_str();
        }
        break;
      }
      else
      {
        if (_currentTvlSeg->isAir())
        {
         _currentTvlSeg->equipmentType() = xmlValue.c_str();
        }
        break;
      }
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  if (!_currentTvlSeg->pssDepartureTime().empty())
    checkFlightContinuity(_currentTvlSeg->origAirport(), _currentTvlSeg->departureDT());
  if (!_currentTvlSeg->pssArrivalTime().empty())
    checkFlightContinuity(_currentTvlSeg->destAirport(), _currentTvlSeg->arrivalDT());

  if (!_isRexTrx)
  {
    if (_currentTvlSeg->pssBookingDate().empty())
      _reqXmlValidator.setBookingDate(_pricingTrx, _currentTvlSeg);

    if (_pricingTrx->billing()->requestPath() == SWS_PO_ATSE_PATH)
      _reqXmlValidator.setOperatingCarrierCode(*_currentTvlSeg);

    if (_itin == nullptr)
    {
      if (_currentTvlSeg->segmentType() == Arunk)
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                             "INCORRECT SURFACE BREAK DESIGNATION");
      if (_currentTvlSeg->pssDepartureDate().empty()) // means processing first segment of itin
        _reqXmlValidator.setDepartureDate(_pricingTrx, _currentTvlSeg);
    }
  }

  _reqXmlValidator.setOperatingCarrierCode(*_currentTvlSeg);
}

bool
PricingModelMap::shouldSaveFareComponentFromFLI() const
{
  return !(_pricingTrx->getRequest() && _pricingTrx->getRequest()->isSFR() &&
           _fareBasisCode.empty());
}

void
PricingModelMap::saveFlightInformation()
{
  LOG4CXX_DEBUG(logger, "In saveFlightInformation");

  if (!_currentTvlSeg)
    return;

  DataHandle& dataHandle = _trx->dataHandle();
  const DateTime& ticketingDate = dataHandle.ticketDate();

  // Populate the Loc's
  const Loc* originLoc = nullptr;
  const Loc* destinationLoc = nullptr;
  if (!_isRexTrx)
  {
    _reqXmlValidator.getOrigDestLoc(_pricingTrx, _currentTvlSeg);
    originLoc = _currentTvlSeg->origin();
    destinationLoc = _currentTvlSeg->destination();
  }
  else
  {
    originLoc = dataHandle.getLoc(_currentTvlSeg->origAirport(), ticketingDate);
    if (!originLoc && !_currentTvlSeg->origAirport().empty())
    {
      LOG4CXX_ERROR(logger, "Failed looking up loc: " << _currentTvlSeg->origAirport());
      throw ErrorResponseException(ErrorResponseException::AIRPORT_CODE_NOT_IN_SYS);
    }
    _currentTvlSeg->origin() = originLoc;

    destinationLoc = dataHandle.getLoc(_currentTvlSeg->destAirport(), ticketingDate);
    if (!destinationLoc && !_currentTvlSeg->destAirport().empty())
    {
      LOG4CXX_ERROR(logger, "Failed looking up loc: " << _currentTvlSeg->destAirport());
      throw ErrorResponseException(ErrorResponseException::AIRPORT_CODE_NOT_IN_SYS);
    }
    _currentTvlSeg->destination() = destinationLoc;
  }

  // Set GEO travel type
  if (originLoc != nullptr && destinationLoc != nullptr) // Surface Locs are empty at this point.
  {
    if (LocUtil::isDomestic(*originLoc, *destinationLoc))
      _currentTvlSeg->geoTravelType() = GeoTravelType::Domestic;
    else if (LocUtil::isInternational(*originLoc, *destinationLoc))
    {
      _currentTvlSeg->geoTravelType() = GeoTravelType::International;
      _intlItin = true;
    }
    else if (LocUtil::isTransBorder(*originLoc, *destinationLoc))
      _currentTvlSeg->geoTravelType() = GeoTravelType::Transborder;
    else if (LocUtil::isForeignDomestic(*originLoc, *destinationLoc))
      _currentTvlSeg->geoTravelType() = GeoTravelType::ForeignDomestic;
  }

  // Transfer flags
  _currentTvlSeg->forcedConx() = _forcedConx;
  _currentTvlSeg->forcedStopOver() = _forcedStopOver;

  if (!_isRexTrx && _forcedConx == 'T' && _forcedStopOver == 'T')
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "SAME POINT INDICATED AS X AND O - MODIFY AND REENTER");

  if (!_isRexTrx && _forcedFareBreak == 'T' && _forcedNoFareBreak == 'T')
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "FORMAT -INVALID NO BREAK ");

  if (!_globalDirectionOverride.empty())
    _currentTvlSeg->globalDirectionOverride() = _globalDirectionOverride;

  if (!_considerOnlyCabin.empty())
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(_currentTvlSeg);
    if (airSeg)
      airSeg->considerOnlyCabin() = _considerOnlyCabin;
  }

  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(_trx);
  if (noPNRTrx != nullptr)
  {
    if (_currentTvlSeg != nullptr && !_globalDirectionOverride.empty())
      noPNRTrx->globalDirectionOverride()[_currentTvlSeg] = _globalDirectionOverride;
  }

  // BF and NB flags
  _currentTvlSeg->forcedSideTrip() = _forcedSideTrip;
  _currentTvlSeg->forcedFareBrk() = _forcedFareBreak;
  _currentTvlSeg->forcedNoFareBrk() = _forcedNoFareBreak;

  // Set FareBasisCode for Q(FareBasis)-(specified FBC) with segment select
  if (!_fareBasisCode.empty())
  {
    _currentTvlSeg->fareBasisCode() = _fareBasisCode;
    _currentTvlSeg->fbcUsage() = _fbcUsage;
  }

  _currentTvlSeg->fareCalcFareAmt() = _fareCompAmountStr;

  if (!_specifiedFbc.empty())
    _currentTvlSeg->specifiedFbc() = _specifiedFbc;

  if (_currentTvlSeg->segmentType() == Air)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(_currentTvlSeg);

    // Assign offMultiCity and boardMultiCity
    setBoardCity(*airSeg);
    setOffCity(*airSeg);

    // Store booked cabin
    DateTime tempAfDate;
    if (PricingTrx::AF_EXC_TRX == _pricingTrx->excTrxType())
    {
      tempAfDate = dataHandle.ticketDate();
      dataHandle.setTicketDate(airSeg->departureDT());
    }

    if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(*_pricingTrx))
    {
      if(fallback::fallbackMoveGetCabinToDSS(_pricingTrx) || _pricingTrx->noPNRPricing())
      getCabin(*airSeg);
    }
    else
    {
      const DateTime& travelDate = _pricingTrx->adjustedTravelDate(airSeg->departureDT());
      const Cabin* aCabin =
          dataHandle.getCabin(airSeg->carrier(), airSeg->getBookingCode(), travelDate);

      if (aCabin)
      {
        airSeg->bookedCabin() = aCabin->cabin();
      }
      else
      {
        airSeg->bookedCabin().setInvalidClass(); // UNKNOWN CABIN valid values are 1,2,3
      }
    }
    if (PricingTrx::AF_EXC_TRX == _pricingTrx->excTrxType())
    {
      dataHandle.setTicketDate(tempAfDate);
    }
  }
  else if (_currentTvlSeg->segmentType() == Open)
  {
    // Store booked cabin
    AirSeg* airSeg = dynamic_cast<AirSeg*>(_currentTvlSeg);
    if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(*_pricingTrx))
    {
      if(fallback::fallbackMoveGetCabinToDSS(_pricingTrx) || _pricingTrx->noPNRPricing())
      getCabin(*airSeg);
    }
    else
    {
      const DateTime& travelDate = _pricingTrx->adjustedTravelDate(airSeg->departureDT());
      const Cabin* aCabin =
          dataHandle.getCabin(airSeg->carrier(), airSeg->getBookingCode(), travelDate);
      if (aCabin)
      {
        airSeg->bookedCabin() = aCabin->cabin();
      }
      else
      {
        airSeg->bookedCabin().setInvalidClass(); // UNKNOWN CABIN valid values are 1,2,3
        // for OPEN segments try to get the cabin value using todays date
        aCabin = nullptr;
        aCabin = dataHandle.getCabin(airSeg->carrier(), airSeg->getBookingCode(), ticketingDate);
        if (aCabin)
          airSeg->bookedCabin() = aCabin->cabin();
      }
    }
    // Assign offMultiCity and boardMultiCity
    const std::vector<tse::MultiTransport*>& boardMACList = dataHandle.getMultiTransportCity(
        _currentTvlSeg->origAirport(),
        (dynamic_cast<AirSeg*>(_currentTvlSeg))->carrier(),
        _currentTvlSeg->geoTravelType(),
        _currentTvlSeg->departureDT() < ticketingDate ? ticketingDate
                                                      : _currentTvlSeg->departureDT());
    std::vector<tse::MultiTransport*>::const_iterator listIter = boardMACList.begin();

    if (listIter == boardMACList.end())
      _currentTvlSeg->boardMultiCity() = _currentTvlSeg->origAirport();
    else
    {
      // If the flight is Domestic use the and the domAppl is set
      // use this multitranscity.  If the flight is International
      // and the intlAppl is set use this multitranscity
      _currentTvlSeg->boardMultiCity() = (**listIter).multitranscity();
    }
    LOG4CXX_DEBUG(logger, "boardMultiCity=" << _currentTvlSeg->boardMultiCity());

    const std::vector<tse::MultiTransport*>& offMACList = dataHandle.getMultiTransportCity(
        _currentTvlSeg->destAirport(),
        (dynamic_cast<AirSeg*>(_currentTvlSeg))->carrier(),
        _currentTvlSeg->geoTravelType(),
        _currentTvlSeg->departureDT() < ticketingDate ? ticketingDate
                                                      : _currentTvlSeg->departureDT());
    listIter = offMACList.begin();
    if (listIter == offMACList.end())
    {
      _currentTvlSeg->offMultiCity() = _currentTvlSeg->destAirport();
    }
    else
    {
      // Take the first, they should all be the same
      _currentTvlSeg->offMultiCity() = (**listIter).multitranscity();
    }
    LOG4CXX_DEBUG(logger, "offMultiCity=" << _currentTvlSeg->offMultiCity());
  }
  //  missing arunk segment on WQ entry for PO
  if ((_isExchangeItin && _excItin == nullptr) || (!_isExchangeItin && _itin == nullptr))
  {
    createItin();
  }

  bool checkMissingArunk = false;
  checkMissingArunk = !_isRexTrx && _itin != nullptr && !_itin->travelSeg().empty();

  if (checkMissingArunk)
  {
    _reqXmlValidator.processMissingArunkSegForPO(
        _pricingTrx, _itin, _currentTvlSeg, _isMissingArunkForPO);
    if (_isMissingArunkForPO)
    {
      // ARUNK was added, need to resequence segmentOrder() for the all segments!
      std::vector<TravelSeg*>& ts = _itin->travelSeg();
      if (ts.size() > 2)
      {
        _currentTvlSeg->segmentOrder() = ts.back()->segmentOrder() + 1;
      }
    }
  }
  // Set TktDesignator and Specified TktDesignator
  if (_pricingTrx->excTrxType() != PricingTrx::PORT_EXC_TRX && !_tktDesignator.empty())
  {
    _pricingTrx->getRequest()->tktDesignator().insert(
        std::pair<int16_t, TktDesignator>(_currentTvlSeg->segmentOrder(), _tktDesignator));
  }

  if (_isExchangeItin)
  {
    if (RexBaseRequest* request = dynamic_cast<RexBaseRequest*>(_pricingTrx->getRequest()))
    {
      saveFlightInformationForDiscountAndPlusUp(*request);
    }
  }
  // Get the objects we'll need for new itin
  else // for new itin only now
  {
    // Set TktDesignator and Specified TktDesignator
    if (_pricingTrx->excTrxType() == PricingTrx::PORT_EXC_TRX && !_tktDesignator.empty())
    {
      _pricingTrx->getRequest()->tktDesignator().insert(
          std::pair<int16_t, TktDesignator>(_currentTvlSeg->segmentOrder(), _tktDesignator));
    }

    // Collapse leading ARUNK
    if (_currentTvlSeg->segmentType() == Arunk)
    {
      if (_pricingTrx->travelSeg().size() == 0)
        return;
    }

    int numTvlSegs = 0;
    std::vector<TravelSeg*> tvlSegs;

    if (dynamic_cast<PricingTrx*>(_trx))
    {
      tvlSegs = _pricingTrx->travelSeg();
      numTvlSegs = tvlSegs.size();
    }
    else
    {
      LOG4CXX_WARN(logger, "Unknown transaction type!");
      return;
    }

    // PSS does not use multi city logic.  If this is an ARUNK we may not
    if (numTvlSegs >= 2)
    {
      TravelSeg* prevTvlSeg = tvlSegs[numTvlSegs - 1];
      if (prevTvlSeg->segmentType() == Arunk)
      {
        TravelSeg* cmpTravelSeg = tvlSegs[numTvlSegs - 2];
        if (prevTvlSeg->isForcedStopOver())
        {
          cmpTravelSeg->forcedStopOver() = prevTvlSeg->forcedStopOver();
        }

        if (cmpTravelSeg->offMultiCity() == _currentTvlSeg->boardMultiCity())
        {
          // Remove the unwanted ARUNK in Trx and Itin
          tvlSegs.pop_back();
          _itin->travelSeg().pop_back();
          _pricingTrx->travelSeg().pop_back();
          numTvlSegs = tvlSegs.size();
        }
      }
    }

    _pricingTrx->travelSeg().push_back(_currentTvlSeg);

    // Set YY/GovCxr Override
    PricingRequest* request = _pricingTrx->getRequest();
    if (!_cxrOverride.empty() && request)
    {
      if (_cxrOverride == INDUSTRY_CARRIER)
      {
        request->industryFareOverrides().push_back(_currentTvlSeg->segmentOrder());
      }
      else
      {
        request->governingCarrierOverrides().insert(
            std::pair<int16_t, CarrierCode>(_currentTvlSeg->segmentOrder(), _cxrOverride));
      }
    }

    if (!_specifiedTktDesignator.empty())
    {
      request->specifiedTktDesignator().insert(std::pair<int16_t, TktDesignator>(
          _currentTvlSeg->segmentOrder(), _specifiedTktDesignator));
    }
    // Store Consolidator Plus Up tkt designator per segment
    //  so we can reuse logic to append to fare basis code
    else if (!_consolidatorPlusUpTktDesignator.empty())
    {
      request->specifiedTktDesignator().insert(std::pair<int16_t, TktDesignator>(
          _currentTvlSeg->segmentOrder(), _consolidatorPlusUpTktDesignator));
    }

    saveFlightInformationForDiscountAndPlusUp(request);
  }

  _currentTvlSeg->unflown() = !_segFlown;

  if (_isExchangeItin)
  {
    if ((_excItin != nullptr) || createItin())
    {
      _excItin->travelSeg().push_back(_currentTvlSeg);
    }

    if (shouldSaveFareComponentFromFLI())
      saveFareComponent();
  }
  else
  {
    if ((_itin != nullptr) || createItin())
    {
      _itin->travelSeg().push_back(_currentTvlSeg);

      if (_pricingTrx->getRequest() && _pricingTrx->getRequest()->isSFR())
      {
        if (shouldSaveFareComponentFromFLI())
          saveFareComponent();
      }
      checkRtwSegment(*_currentTvlSeg);
    }
  }
}

void
PricingModelMap::saveFlightInformationForDiscountAndPlusUp(PricingRequest* request)
{
  // Set DA and DP
  if (!fallback::azPlusUp(_trx))
  {
    if (_discountAmountNew > 0.0)
    {
      request->addDiscountAmountNew(_discountGroupNum,
                                    _currentTvlSeg->segmentOrder(),
                                    _discountAmountNew,
                                    _discountCurrencyCode);
    }

    if (_discountPercentageNew > 0.0)
    {
      request->addDiscountPercentage(_currentTvlSeg->segmentOrder(), _discountPercentageNew);
    }
  }
  else
  {
    if (_discountAmount >= 0.0)
    {
      request->addDiscAmount(_discountGroupNum,
                             _currentTvlSeg->segmentOrder(),
                             _discountAmount,
                             _discountCurrencyCode);
    }

    if (_discountPercentage >= 0.0)
    {
      request->discPercentages().insert(
          std::pair<int16_t, Percent>(_currentTvlSeg->segmentOrder(), _discountPercentage));
    }
  }

  if (!fallback::azPlusUp(_trx))
  {
    // Set PA
    if (_discountAmountNew < 0.0)
    {
      request->addDiscountAmountNew(_discountGroupNum,
                                    _currentTvlSeg->segmentOrder(),
                                    _discountAmountNew,
                                    _discountCurrencyCode);
    }

    if (_discountPercentageNew < 0.0)
    {
      request->addDiscountPercentage(_currentTvlSeg->segmentOrder(), _discountPercentageNew);
    }
  }
}

void
PricingModelMap::saveFlightInformationForDiscountAndPlusUp(RexBaseRequest& request)
{
  if (!fallback::azPlusUp(_trx) && !fallback::azPlusUpExc(_trx))
  {
    if (_discountAmountNew != 0.0)
    {
      request.excDiscounts().addAmount(_discountGroupNum,
                                       _currentTvlSeg->segmentOrder(),
                                       _discountAmountNew,
                                       _discountCurrencyCode);
    }

    if (_discountPercentageNew != 0.0)
    {
      request.excDiscounts().addPercentage(_currentTvlSeg->segmentOrder(), _discountPercentageNew);
    }
  }
}

void
PricingModelMap::getCabin(AirSeg& airSeg)
{
  RBDByCabinUtil rbdUtil(*_pricingTrx, PRICING_RQ);
  rbdUtil.getCabinByRBD(airSeg);
}

void
PricingModelMap::saveExchangeOverrides()
{
  if (_fareCompNum <= 0)
    return;

  if (_fareBasisCode.empty())
    return;

  if (!shouldStoreOverrides())
    return;
  BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_pricingTrx);

  if (_isExchangeItin)
  {
    if (excTrx->excTrxType() == PricingTrx::AF_EXC_TRX)
    {
      if (!_surchargeOverride.empty())
      {
        std::vector<SurchargeOverride*>::iterator surI = _surchargeOverride.begin();
        std::vector<SurchargeOverride*>::iterator surE = _surchargeOverride.end();
        for (; surI != surE; ++surI)
        {
          SurchargeOverride* surOverride = *surI;
          surOverride->travelSeg() = _currentTvlSeg;
          excTrx->exchangeOverrides().surchargeOverride().push_back(surOverride);
        }
      }

      saveStopoverOverride(*excTrx);
      savePlusUpOverride(*excTrx);
      saveMileDataOverride(*excTrx);
      return;
    }
    if (exchangeTypeNotCE(excTrx))
      return;
  }

  if (_currentTvlSeg == nullptr)
    return;

  if (!_isExchangeItin)
    excTrx->exchangeOverrides().dummyFCSegs()[_currentTvlSeg] = _fareCompNum;

  if (_fareCompAmountStr.empty()) // dummy FC, not last seg
    _currentTvlSeg->forcedNoFareBrk() = 'T';

  if (_forcedSideTrip == 'T')
    excTrx->exchangeOverrides().forcedSideTrip()[_currentTvlSeg] = _forcedSideTrip;

  excTrx->exchangeOverrides().dummyFareMiles()[_currentTvlSeg] = _mileageSurchargePctg;
  _mileageSurchargePctg = -1;

  if (!_mileageSurchargeCity.empty())
  {
    excTrx->exchangeOverrides().dummyFareMileCity()[_currentTvlSeg] = _mileageSurchargeCity;
    _mileageSurchargeCity = "";
  }

  if (!_mileageTktCity.empty())
  {
    excTrx->exchangeOverrides().dummyFareMileTktCity()[_currentTvlSeg] = _mileageTktCity;
    _mileageTktCity = "";
  }

  // Surcharge overrides
  if (!_surchargeOverride.empty())
  {
    std::vector<SurchargeOverride*>::iterator surI = _surchargeOverride.begin();
    std::vector<SurchargeOverride*>::iterator surE = _surchargeOverride.end();
    for (; surI != surE; ++surI)
    {
      SurchargeOverride* surOverride = *surI;
      surOverride->travelSeg() = _currentTvlSeg;
      excTrx->exchangeOverrides().surchargeOverride().push_back(surOverride);
    }
  }

  saveStopoverOverride(*excTrx);
  savePlusUpOverride(*excTrx);
  saveMileDataOverride(*excTrx);
}

void
PricingModelMap::saveStopoverOverride(BaseExchangeTrx& excTrx)
{
  // Stopover overrides
  if (!_stopoverOverride.empty())
  {
    std::vector<StopoverOverride*>::iterator soI = _stopoverOverride.begin();
    const std::vector<StopoverOverride*>::iterator soE = _stopoverOverride.end();
    for (; soI != soE; ++soI)
    {
      StopoverOverride* soOverride = *soI;
      soOverride->travelSeg() = _currentTvlSeg; // 0 means charge on journey
      excTrx.exchangeOverrides().stopoverOverride().push_back(soOverride);
    }
    _stopoverOverride.clear();
  }
}

void
PricingModelMap::savePlusUpOverride(BaseExchangeTrx& excTrx)
{
  if (!_plusUpOverride.empty())
  {
    std::vector<PlusUpOverride*>::iterator plusUpI = _plusUpOverride.begin();
    const std::vector<PlusUpOverride*>::iterator plusUpE = _plusUpOverride.end();
    for (; plusUpI != plusUpE; ++plusUpI)
    {
      PlusUpOverride* plusUpOverride = *plusUpI;
      plusUpOverride->travelSeg() = _currentTvlSeg; // 0 means charge on journey
      excTrx.exchangeOverrides().plusUpOverride().push_back(plusUpOverride);
    }
    _plusUpOverride.clear();
  }
}

void
PricingModelMap::saveMileDataOverride(BaseExchangeTrx& excTrx)
{
  if (!_mileDataOverride.empty())
  {
    std::vector<MileageTypeData*>::iterator mileTypeDataI = _mileDataOverride.begin();
    const std::vector<MileageTypeData*>::iterator mileTypeDataE = _mileDataOverride.end();
    for (; mileTypeDataI != mileTypeDataE; ++mileTypeDataI)
    {
      (*mileTypeDataI)->travelSeg() = _currentTvlSeg; // 0 means charge on journey
      excTrx.exchangeOverrides().mileageTypeData().push_back(*mileTypeDataI);
    }
    _mileDataOverride.clear();
  }
}

FareCompInfo*
PricingModelMap::getFareComponent(Itin* itin, uint16_t fareCompNum) const
{
  if (_pricingTrx->isMultiPassengerSFRRequestType())
    return getPassengerFareComponent(_paxType, fareCompNum);

  FareCompInfo* curFc = findFareCompInfo(itin->fareComponent(), fareCompNum);
  if (curFc == nullptr) // Create new FareComponent
  {
    _pricingTrx->dataHandle().get(curFc);

    if (curFc)
    {
      FareMarket* fareMarket = nullptr;
      _pricingTrx->dataHandle().get(fareMarket);
      curFc->fareMarket() = fareMarket;
      fareMarket->fareCompInfo() = curFc;
      curFc->fareCompNumber() = fareCompNum;

      if (_pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX ||
          _pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX || _pricingTrx->getRequest()->isSFR())
      {
        if (itin->fareComponent().empty() && (_fareCompNum != 1))
          throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE);

        if (!itin->fareComponent().empty() &&
            _fareCompNum != (itin->fareComponent().back()->fareCompNumber() + 1))
          throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE);
      }

      itin->fareComponent().push_back(curFc);
    }
  }

  return curFc;
}

FareCompInfo*
PricingModelMap::getPassengerFareComponent(PaxType* paxType, uint16_t fareCompNum) const
{
  if (!paxType)
    return nullptr;

  auto& paxFCMapping = *_pricingTrx->getMultiPassengerFCMapping();
  auto& fareComponentVector = paxFCMapping[paxType];

  auto fareComponentIt =
      std::find_if(fareComponentVector.cbegin(),
                   fareComponentVector.cend(),
                   [fareCompNum](FareCompInfo* fareComponentInfo)
                   { return fareComponentInfo->fareCompNumber() == fareCompNum; });

  if (fareComponentIt == fareComponentVector.cend())
  {
    if (fareComponentVector.size() + 1 != fareCompNum)
      throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE);

    auto& dataHandle = _pricingTrx->dataHandle();
    FareCompInfo* newFC = &dataHandle.safe_create<FareCompInfo>();
    FareMarket* fareMarket = &dataHandle.safe_create<FareMarket>();

    newFC->fareMarket() = fareMarket;
    fareMarket->fareCompInfo() = newFC;
    newFC->fareCompNumber() = fareCompNum;

    fareComponentVector.push_back(newFC);
    return newFC;
  }
  else
  {
    return *fareComponentIt;
  }
}

void
PricingModelMap::saveFareComponent()
{
  if (_excItin || _pricingTrx->getRequest()->isSFR())
  {
    if (_sideTripStart && _prevFareCompInfo)
    {
      _prevFareCompInfo->fareMarket()->sideTripTravelSeg().push_back(std::vector<TravelSeg*>());
      _fareCompInfoStack.push(_prevFareCompInfo);
    }

    if (_fareCompInfoStack.empty() && (_sideTripNumber > 0 || _sideTripEnd))
    {
      if (_pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX ||
          _pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX)
        throw ErrorResponseException(ErrorResponseException::UNABLE_TO_MATCH_FARE);
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                   "SIDE TRIP DATA ERROR");
    }

    if (_sideTripNumber > 0) // Side Trip Number (start from 1) specified
    {
      _currentTvlSeg->forcedSideTrip() = 'T';
      if (_fareCompNum > 0)
        _fareCompInfoStack.top()->fareMarket()->sideTripTravelSeg().back().push_back(
            _currentTvlSeg);
    }

    if (_sideTripEnd)
    {
      _prevFareCompInfo = _fareCompInfoStack.top();
      _fareCompInfoStack.pop();
    }

    if (_fareCompNum > 0) // Fare Component Number (start from 1) specified
    {
      FareCompInfo* curFc = getFareComponent(_excItin ? _excItin : _itin, _fareCompNum);

      if (curFc)
      {
        if (!fallback::azPlusUp(_trx))
          curFc->discounted() = (_discountPercentageNew > 0.0 || _discountAmountNew > 0.0);
        else
          curFc->discounted() = (_discountPercentage > 0.0 || _discountAmount > 0.0);
        curFc->fareBasisCode() = _fareBasisCode;

        if ((curFc->hasVCTR() = hasFullVCTR()))
          curFc->VCTR() = _VCTR;

        if (_fareCompAmount > 0)
          curFc->fareCalcFareAmt() = curFc->tktFareCalcFareAmt() = _fareCompAmount;

        if (shouldStoreOverrides())
        {
          BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_pricingTrx);
          if (_forcedSideTrip == 'T')
            excTrx->exchangeOverrides().forcedSideTrip()[_currentTvlSeg] = _forcedSideTrip;
        }

        if (fallback::SSDSP_1844_removeArunkSegForSFR(_trx) ||
            (!(_pricingTrx->getRequest() && _pricingTrx->getRequest()->isSFR()) ||
             _currentTvlSeg->segmentType() != Arunk))
          curFc->fareMarket()->travelSeg().push_back(_currentTvlSeg);

        if (!_sideTripEnd)
          _prevFareCompInfo = curFc;
      }
    }
  }
  else if (_pricingTrx->getRequest() && _pricingTrx->getRequest()->isSFR())
  {
    if (_fareCompNum > 0) // Fare Component Number (start from 1) specified
    {
      if (FareCompInfo* curFc = getFareComponent(_itin, _fareCompNum))
      {
        curFc->fareBasisCode() = _fareBasisCode;
        if (_fareCompAmount > 0)
          curFc->fareCalcFareAmt() = curFc->tktFareCalcFareAmt() = _fareCompAmount;

        if (fallback::SSDSP_1844_removeArunkSegForSFR(_trx) ||
            _currentTvlSeg->segmentType() != Arunk)
          curFc->fareMarket()->travelSeg().push_back(_currentTvlSeg);

        if (!_sideTripEnd)
          _prevFareCompInfo = curFc;
      }
    }
  }
}

void
PricingModelMap::storeProcOptsInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeProcOptsInformation");

  // Get a Option if not already available
  PricingRequest* request = _pricingTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request object is NULL in Trx object");
    return;
  }

  DataHandle& dataHandle = _pricingTrx->dataHandle();

  Agent* agent = request->ticketingAgent();
  if (agent == nullptr)
  {
    dataHandle.get(agent); // lint !e530
    request->ticketingAgent() = agent;
    if (agent == nullptr)
    {
      LOG4CXX_WARN(logger, "Agent object is NULL in Request object");
      return;
    }
  }

  if (_options == nullptr)
  {
    dataHandle.get(_options);
    _pricingTrx->setOptions(_options);
  }

  if (_isExchangeItin)
  {
    storeExcItinProcOptsInformation(attrs);
    return;
  }

  NationCode countryCode, stateRegionCode;
  bool d07Present = false;
  bool p0jPresent = false;

  bool isPRM = false;
  uint8_t rcqCount = 0;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 4: // Q0S - Number of solutions    SHOPPING   ESVOptions ??
      _options->setRequestedNumberOfSolutions(atoi(xmlValue.c_str()));
      break;

    case 9: // P1Z - Private fares
      _options->privateFares() = (xmlValue.c_str())[0];
      break;
    case 10: // P1Y - Published fares
      _options->publishedFares() = (xmlValue.c_str())[0];
      break;
    case 11: // P20 - XO fares
      _options->xoFares() = (xmlValue.c_str())[0];
      break;
    case 12: // P1W - Online fares
      _options->onlineFares() = (xmlValue.c_str())[0];
      break;
    case 13: // P21 - IATA fares
      _options->iataFares() = (xmlValue.c_str())[0];
      break;
    case 14: // P1V - Price no availability
      request->priceNoAvailability() = (xmlValue.c_str())[0];
      break;
    case 16: // P44 - Currency override
      _options->currencyOverride() = xmlValue.c_str();
      break;
    case 17: // Q0P - Return all data
      _options->returnAllData() = (xmlValue.c_str())[0];
      break;
    case 18: // AC0 - Corp id
    {
      RexBaseRequest* rexBaseRequest = dynamic_cast<RexBaseRequest*>(request);
      if (rexBaseRequest != nullptr)
        rexBaseRequest->newCorporateID() = xmlValue.c_str();
      else
        request->corporateID() = xmlValue.c_str();
    }
    break;
    case 19: // P22 - Refund penalty
      request->refundPenalty() = (xmlValue.c_str())[0];
      break;
    case 24: // C10 - Diagnostic number
      request->diagnosticNumber() = atoi(xmlValue.c_str());
      break;
    case 53: // B05 - Validating carrier
    {
      // LATAM MCP-S
      if (!fallback::neutralToActualCarrierMapping(_pricingTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        request->validatingCarrier() = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
      }
      else
        request->validatingCarrier() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;
    }
    case 54: // AF0 - Ticketing point override
      request->ticketPointOverride() = xmlValue.c_str();
      break;
    case 55: // AG0 - Sale point override
      request->salePointOverride() = xmlValue.c_str();
      break;
    case 56: // C45 - Equiv amount currency code
      // request->equivAmountCurrencyCode() = xmlValue.c_str();
      _options->currencyOverride() = xmlValue.c_str();
      break;
    case 57: // C6D - Equiv amount override
      request->equivAmountOverride() = (double)atof(xmlValue.c_str());
      break;
    case 58: // C6E - Rate amount override
      request->rateAmountOverride() = (double)atof(xmlValue.c_str());
      break;
    case 59: // D07 - Ticket date override
      if (!_multiExcTrx)
      {
        RexBaseTrx* rexBaseTrx = getRexBaseTrx();
        if (rexBaseTrx)
        {
          RexBaseRequest* rexBaseRequest = dynamic_cast<RexBaseRequest*>(_pricingTrx->getRequest());
          if (rexBaseRequest != nullptr)
          {
            rexBaseRequest->setTicketingDT(convertDate(xmlValue.c_str()));
            if (rexBaseRequest->getTicketingDT().isValid())
            {
              rexBaseTrx->dataHandle().setTicketDate(
                  rexBaseRequest->getTicketingDT()); // set dataHandle's ticketingDate
              rexBaseTrx->currentTicketingDT() = rexBaseRequest->getTicketingDT();
              rexBaseTrx->setFareApplicationDT(
                  rexBaseRequest->getTicketingDT()); // set fareApplicationDT
            }
          }
        }
        else
        {
          ExchangePricingTrx* excTrx = getExchangePricingTrx();
          if (excTrx)
          {
            excTrx->currentTicketingDT() = convertDate(xmlValue.c_str());
            if (excTrx->purchaseDT().isEmptyDate())
            {
              excTrx->purchaseDT() = excTrx->currentTicketingDT();
              request->ticketingDT() = excTrx->currentTicketingDT();
              dataHandle.setTicketDate(excTrx->currentTicketingDT());
            }
          }
          else
          {
            d07Present = true;
            request->ticketingDT() = convertDate(xmlValue.c_str());
            if (request->ticketingDT().isValid())
            {
              dataHandle.setTicketDate(request->ticketingDT());
            }
          }
        }
      }
      else
      {
        ExchangePricingTrx* excTrx = getExchangePricingTrx();
        if (excTrx)
        {
          excTrx->currentTicketingDT() = convertDate(xmlValue.c_str());
        }
      }
      break;
    case 60: // D54 - Ticket time override
    {
      if (_multiExcTrx)
      {
        if (_pricingTrx->excTrxType() != PricingTrx::NEW_WITHIN_ME)
          break; // ignore time for EX1 EX2

        ExchangePricingTrx* excTrx = getExchangePricingTrx();
        if (!excTrx || (excTrx->currentTicketingDT() != request->ticketingDT()))
          break; // ignore time unless D07 and D93 are same date
      }
      _options->ticketTimeOverride() = atoi(xmlValue.c_str());

      RexBaseTrx* rexBaseTrx = getRexBaseTrx();
      if (rexBaseTrx)
      {
        RexBaseRequest* rexBaseRequest = dynamic_cast<RexBaseRequest*>(_pricingTrx->getRequest());
        if (rexBaseRequest != nullptr)
        {
          rexBaseRequest->getTicketingDT() =
              rexBaseRequest->getTicketingDT() + tse::Hours(_options->ticketTimeOverride() / 60) +
              tse::Minutes(_options->ticketTimeOverride() % 60) + tse::Seconds(0);

          rexBaseRequest->getTicketingDT().setHistoricalIncludesTime();

          if (rexBaseRequest->getTicketingDT().isValid())
          {
            rexBaseTrx->dataHandle().setTicketDate(
                rexBaseRequest->getTicketingDT()); // set dataHandle's ticketingDate
            rexBaseTrx->currentTicketingDT() = rexBaseRequest->getTicketingDT();
            rexBaseTrx->setFareApplicationDT(
                rexBaseRequest->getTicketingDT()); // set fareApplicationDT  ticketingDate
          }
        }
      }
      else
      {
        _ticketingDTadjustedWithTime = true;
        _originalTicketingDT = request->ticketingDT();
        request->ticketingDT() =
            request->ticketingDT() + tse::Hours(_options->ticketTimeOverride() / 60) +
            tse::Minutes(_options->ticketTimeOverride() % 60) + tse::Seconds(0);
        request->ticketingDT().setHistoricalIncludesTime();

        if (request->ticketingDT().isValid())
        {
          dataHandle.setTicketDate(request->ticketingDT());

          ExchangePricingTrx* excTrx = getExchangePricingTrx();
          if (excTrx && excTrx->reqType() == TAG_10_EXCHANGE)
          {
            // CE will take D07/D54 as purchaseDT, ignore D93
            excTrx->purchaseDT() = request->ticketingDT();
          }
        }
      }
    }
    break;
    case 61: // Q15 - Length of ATB fare calc
      request->lengthATBFareCalc() = atoi(xmlValue.c_str());
      break;
    case 62: // P47 - No penalties
      _options->noPenalties() = (xmlValue.c_str())[0];
      break;
    case 63: // P48 - No adv purch restrictions
      _options->noAdvPurchRestr() = (xmlValue.c_str())[0];
      break;
    case 64: // P49 - No min max stay restriction
      _options->noMinMaxStayRestr() = (xmlValue.c_str())[0];
      break;
    case 65: // P50 - Normal fare
      _options->normalFare() = (xmlValue.c_str())[0];
      break;
    case 66: // P51 - Low fare no availability
      request->lowFareNoAvailability() = (xmlValue.c_str())[0];
      break;
    case 67: // P52 - Low fare requested
      request->lowFareRequested() = (xmlValue.c_str())[0];
      break;
    case 68: // P53 - Exempt specific taxes
      setRestrictCmdPricing(xmlValue.c_str());

      request->exemptSpecificTaxes() = (xmlValue.c_str())[0];
      break;
    case 69: // P54 - Exempt all taxes
      setRestrictCmdPricing(xmlValue.c_str());

      request->exemptAllTaxes() = (xmlValue.c_str())[0];
      break;
    case 70: // P55 - Price null date

      request->priceNullDate() = (xmlValue.c_str())[0];
      break;
    case 73: // P58 - Ticket entry

      request->ticketEntry() = (xmlValue.c_str())[0];
      break;
    case 74: // P59 - Cat35 not allowed
      _options->cat35NotAllowed() = (xmlValue.c_str())[0];
      break;
    case 75: // P60 - Cat35 fare should be ignored
      _options->cat35FareShouldBeIgnored() = (xmlValue.c_str())[0];
      break;
    case 76: // P61 - Form of payment cash
      request->formOfPaymentCash() = (xmlValue.c_str())[0];
      break;
    case 77: // P62 - Form of payment check
      request->formOfPaymentCheck() = (xmlValue.c_str())[0];
      break;
    case 78: // P63 - Form of payment card
      request->formOfPaymentCard() = (xmlValue.c_str())[0];
      break;
    case 82: // P66 - Exempt from PFC tax
      request->exemptPFC() = (xmlValue.c_str())[0];
      break;
    case 83: // BG0 - Ticket stock
      _options->ticketStock() = atoi(xmlValue.c_str());
      break;
    case 84: // Q16 - Tax number of boxes
      request->numberTaxBoxes() = atoi(xmlValue.c_str());
      break;
    case 85: // N0L - Agency commission type
      agent->agentCommissionType() = xmlValue.c_str();
      break;
    case 86: // C6C - Agency commission amount
      agent->agentCommissionAmount() = atoi(xmlValue.c_str());
      break;
    case 88: // P0F - /WEB
      _options->web() = (xmlValue.c_str())[0];
      break;
    case 89: // P0J - Electronic ticket
    {
      p0jPresent = true;
      request->electronicTicket() = (xmlValue.c_str())[0];
    }
    break;
    case 91: // S11 - Account code
    {
      RexBaseRequest* rexBaseRequest = dynamic_cast<RexBaseRequest*>(request);
      if (rexBaseRequest != nullptr)
        rexBaseRequest->newAccountCode() = xmlValue.c_str();
      else
        request->accountCode() = xmlValue.c_str();
    }
    break;
    case 92: // S12 - Fare by rule ship registry
      _options->fareByRuleShipRegistry() = xmlValue.c_str();
      break;
    case 93: // N0M - Country type 'E' 'N' 'R'
      _currentCRC = xmlValue.c_str();
      break;
    case 94: // A40 - Country code
      countryCode = xmlValue.c_str();
      break;
    case 95: // AH0 - State region code
      stateRegionCode = xmlValue.c_str();
      break;
    case 96: // C6H - Second rate amount override
      request->secondRateAmountOverride() = (double)atof(xmlValue.c_str());
      break;
    case 97: // P67 - Government Transportation Request
      request->formOfPaymentGTR() = (xmlValue.c_str())[0];
      break;
    case 98: // S13 - Passenger Name Record
      _options->pnr() = xmlValue.c_str();
      break;
    case 99: // S14 - Line Entry
      _options->lineEntry() = xmlValue.c_str();
      break;
    case 100: // S15 - Employee profile entries
    {
      std::set<std::string>& theSet = _options->eprKeywords();
      char* pHolder = nullptr;
      for (char* token = strtok_r((char*)xmlValue.c_str(), ",", &pHolder); token != nullptr;
           token = strtok_r(nullptr, ",", &pHolder))
      {
        theSet.insert(token);
        setEPRKeywordFlag(token);
      }
    }
    break;
    case 102: // BC0 - Tax code override
      setRestrictCmdPricing(xmlValue.c_str());

      _taxCodeOverrides.push_back(xmlValue.c_str());
      break;
    case 103: // C6B - Override tax amount
      setRestrictCmdPricing(xmlValue.c_str());

      _overrideTaxAmounts.push_back((double)atof(xmlValue.c_str()));
      break;
    case 104: // BH0 - Tax code exempted
    {
      setRestrictCmdPricing(xmlValue.c_str());

      std::string tmpString = xmlValue.c_str();

      std::string::size_type sz = tmpString.find(" ");

      if (sz != std::string::npos) // lint !e530
      {
        request->taxIdExempted().push_back(tmpString.substr(0, sz));
      }
      else
      {
        request->taxIdExempted().push_back(tmpString);
      }
    }
    break;
    case 105: // C47 - Alternate currency
      _options->alternateCurrency() = xmlValue.c_str();
      break;
    case 106: // P69 - M override
      _options->mOverride() = (xmlValue.c_str())[0];
      break;

    case 107: // P70 - Cat 35 sell indicator
      _options->cat35Sell() = (xmlValue.c_str())[0];
      break;
    case 108: // P71 - Cat 35 net indicator
      _options->cat35Net() = (xmlValue.c_str())[0];
      break;
    case 109: // P77 - WP entry with RQ option
      _options->recordQuote() = (xmlValue.c_str())[0];
      break;
    case 110: // PBI - Ignore Fuel Surcharge
      _options->ignoreFuelSurcharge() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 111: // PBJ - Booking Code Override
      _options->bookingCodeOverride() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 112: // PBK - Free Baggage Subscriber
      _options->freeBaggage() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 113: // AE0 - PCC Host Carrier
      // LATAM MCP-S
      if (!fallback::neutralToActualCarrierMapping(_pricingTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        agent->hostCarrier() = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
      }
      else
        agent->hostCarrier() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      if (strcmp(xmlValue.c_str(), WEST_JET_CODE) == 0)
      {
        _isWsUser = true;
      }
      break;
    case 114: // N1R - FareType request fare Family N/S/T
      _options->fareFamilyType() = (xmlValue.c_str())[0];
      //            if(_options->specialFareType() || _options->incTourFareType())
      //              throw ErrorResponseException(ErrorResponseException::T_NOT_AVAILABLE_IN_V2);
      // //T/EX, T/IT
      break;
    case 116: // P4A - FareX indicator
      _options->fareX() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 117: // PBY - No-match turned off
      request->turnOffNoMatch() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 118: // P4B - UpSell entry
      request->upSellEntry() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 119: // PBZ - ticketing date override indicator
      _options->isTicketingDateOverrideEntry() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      LOG4CXX_DEBUG(logger,
                    "ticketing date override entry : " << _options->isTicketingDateOverrideEntry());
      break;
    case 120: // PCB - XPTS entry
      request->ptsOverride() = (xmlValue.c_str())[0];
      break;
    case 121: // PCD - WPNETT entry  for JAL/AXESS
      request->wpNettRequested() = (xmlValue.c_str())[0];
      break;
    case 122: // PCE - WPSEL entry  for JAL/AXESS
      request->wpSelRequested() = (xmlValue.c_str())[0];
      break;
    case 123: // S94 - Waiver Code for REX
      // Only read it from EXC PRO
      break;
    case 124: // D92 - Original Ticket Issue Date for REX
    {
      RexBaseTrx* rexBaseTrx = getRexBaseTrx();
      if (rexBaseTrx)
      {
        rexBaseTrx->setOriginalTktIssueDT() = convertDate(xmlValue.c_str());
      }
      else if (_multiExcTrx && _pricingTrx->excTrxType() != PricingTrx::NEW_WITHIN_ME)
      {
        if (_pricingTrx->excTrxType() == PricingTrx::EXC1_WITHIN_ME)
          _multiExcTrx->cat5TktDT_Ex1() = convertDate(xmlValue.c_str());
        else if (_pricingTrx->excTrxType() == PricingTrx::EXC2_WITHIN_ME)
          _multiExcTrx->cat5TktDT_Ex2() = convertDate(xmlValue.c_str());

        ExchangePricingTrx* exchangePricingTrx = getExchangePricingTrx();
        if (exchangePricingTrx)
          exchangePricingTrx->setOriginalTktIssueDT() = convertDate(xmlValue.c_str());
      }
      else
      {
        ExchangePricingTrx* exchangePricingTrx = getExchangePricingTrx();
        if (exchangePricingTrx)
        {
          exchangePricingTrx->setOriginalTktIssueDT() = convertDate(xmlValue.c_str());
        }
      }
    }
    break;
    case 125: // C6Y - Fare Calc Currency for REX
      if (_multiExcTrx != nullptr && _pricingTrx != _multiExcTrx->newPricingTrx())
        _excItinFareCalcCurrency = xmlValue.c_str();
      else
        _newItinFareCalcCurrency = xmlValue.c_str();
      break;
    case 126: // PXC - Force Corporate Fares
      _options->forceCorpFares() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      //             throw
      // ErrorResponseException(ErrorResponseException::XC_QUALIFIER_NOT_AVAILABLE_IN_V2);
      break;
    case 127: // A50 - residency of city
      _options->residency() = xmlValue.c_str();
      break;
    case 128: // D94
    {
      RexBaseTrx* rexTrx = getRexBaseTrx();
      if (rexTrx)
      {
        rexTrx->lastTktReIssueDT() = convertDate(xmlValue.c_str());
      }
    }
    break;
    case 129: // C74 - Consolidator Plus Up Currency
      _consolidatorPlusUpCurrency = xmlValue.c_str();
      break;
    case 130: // C6Z - Consolidator Plus Up Amount ($)
      _consolidatorPlusUpAmount = strtod(xmlValue.c_str(), nullptr);
      break;
    case 131: // BE1 - Consolidator Plus Up Tkt Designator
      _consolidatorPlusUpTktDesignator = xmlValue.c_str();
      break;
    case 132: // PCG - Bypass Advance Purchase indicator
      if (!xmlValue.empty())
        _options->AdvancePurchaseOption() = xmlValue.c_str()[0];
      break;
    case 133: // Q6B - ROE override for exchange pricing Trx
    {
      request->roeOverride() = (ExchRate)atof(xmlValue.c_str());
      std::string tmpString = xmlValue.c_str();
      std::string::size_type decPos = tmpString.find(".");
      if (decPos != std::string::npos)
        request->roeOverrideNoDec() = tmpString.size() - decPos - 1;
      break;
    }
    case 135: // C6P - base fare currency override for exc trx
      _options->baseFareCurrencyOverride() = xmlValue.c_str();
      break;
    case 136: // D93 - Purchase Date
      if (_processExchangePricingTrx)
      {
        ExchangePricingTrx* excTrx = getExchangePricingTrx();
        if (excTrx && excTrx->reqType() != TAG_10_EXCHANGE)
        {
          excTrx->purchaseDT() = convertDate(xmlValue.c_str());
          request->ticketingDT() = excTrx->purchaseDT();
          dataHandle.setTicketDate(excTrx->purchaseDT());
        }
      }
      else if (RexBaseTrx* rexBaseTrx = getRexBaseTrx())
        rexBaseTrx->purchaseDT() = convertDate(xmlValue.c_str());

      break;
    case 137: // P88 - sort ascending (SA)
    {
      NoPNRPricingOptions* noPNRPricingOptions = dynamic_cast<NoPNRPricingOptions*>(_options);
      if (nullptr != noPNRPricingOptions)
        noPNRPricingOptions->sortAscending() = xmlValue.c_str()[0];
    }
    break;
    case 138: // P89 - sort descending (SD)
    {
      NoPNRPricingOptions* noPNRPricingOptions = dynamic_cast<NoPNRPricingOptions*>(_options);
      if (nullptr != noPNRPricingOptions)
        noPNRPricingOptions->sortDescending() = xmlValue.c_str()[0];
    }
    break;
    case 139: // AXM - forced no match (XM)
    {
      NoPNRPricingOptions* noPNRPricingOptions = dynamic_cast<NoPNRPricingOptions*>(_options);
      if (nullptr != noPNRPricingOptions)
        noPNRPricingOptions->noMatch() = xmlValue.c_str()[0];
    }
    break;
    case 140: // PXA - WPA 50 options
      request->wpa50() = (xmlValue.c_str())[0];
      break;
    case 141: // AP1 - reissue location for tax reissue for cat 31 and non-cat31
    {
      BaseExchangeTrx* bExcTrx = dynamic_cast<BaseExchangeTrx*>(_pricingTrx);
      if (bExcTrx)
        bExcTrx->reissueLocation() = xmlValue.c_str();
    }
    break;
    case 142: // P0L - E_tkt default =OFF, no override in input
      request->eTktOffAndNoOverride() = (xmlValue.c_str())[0];
      break;
    case 143: // PXB - WPAS entry -> WPA with no check availability
      request->wpas() = (xmlValue.c_str())[0];
      break;
    case 144: // AC1-AC4 - CorporateID (Multi AccCode/CorpID)
    case 145:
    case 146:
    case 147:
      if (_trx->dataHandle().corpIdExists(xmlValue.c_str(), _pricingTrx->ticketingDate()))
      {
        std::vector<std::string>& corpIdVec = request->corpIdVec();
        if (std::find(corpIdVec.begin(), corpIdVec.end(), xmlValue.c_str()) == corpIdVec.end())
          corpIdVec.push_back(xmlValue.c_str());
      }
      else
      {
        std::vector<std::string>& incorrectCorpIdVec = request->incorrectCorpIdVec();
        if (std::find(incorrectCorpIdVec.begin(), incorrectCorpIdVec.end(), xmlValue.c_str()) ==
            incorrectCorpIdVec.end())
          incorrectCorpIdVec.push_back(xmlValue.c_str());
      }
      break;
    case 148: // SM1-SM4 - Account code (Multi AccCode/CorpID)
    case 149:
    case 150:
    case 151:
    {
      std::vector<std::string>& accCodeVec = request->accCodeVec();
      if (std::find(accCodeVec.begin(), accCodeVec.end(), xmlValue.c_str()) == accCodeVec.end())
        accCodeVec.push_back(xmlValue.c_str());
    }
    break;
    case 152: // SEZ - Collect OB Fee
      request->collectOBFee() = (xmlValue.c_str())[0];
      break;
    case 153: // D95 - Transation Type Exchange Date
    {
      BaseExchangeTrx* baseTrx = dynamic_cast<BaseExchangeTrx*>(_pricingTrx);
      if (baseTrx)
      {
        baseTrx->previousExchangeDT() = convertDate(xmlValue.c_str());
      }
    }
    break;

    case 155: // SEY - Collect OC Fees
      request->collectOCFees() = (xmlValue.c_str())[0];
      break;
    case 157: // SET - Process All Ancillary services
      _options->isProcessAllGroups() = (xmlValue.c_str())[0];
      break;
    /*        case 157: // SET - Ancillary services qualifiers - OC Fees group codes
              _options->serviceGroups() = xmlValue.c_str();
              break;*/
    case 158: // TKI - Ancillary services Ticketing indicator
      _options->isTicketingInd() = (xmlValue.c_str())[0];
      break;
    case 159: // OCL - Low fare with No rebook
      request->lowFareNoRebook() = (xmlValue.c_str())[0];
      break;
    case 160: // Q6W - award request
      _pricingTrx->awardRequest() = (xmlValue.c_str())[0] == 'T';
      break;
    case 161: // FOP - form of payment
      request->formOfPayment() = xmlValue.c_str();
      break;
    case 162: // PPC - Temporary WP entry no Ancillary services is required
      _options->isWPwithOutAE() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 163: // PNS - TJR NETSELL for direct ticketing.
      _options->cat35NetSell() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 164: //  P4N - Ignore Net Remit Fares
      _options->setNetRemitFareRestricted(xmlValue.c_str()[0]);
      break;
    case 165: // T92
    {
      RexBaseTrx* rbt = getRexBaseTrx();
      if (rbt)
      {
        if (rbt->originalTktIssueDT().isEmptyDate())
          throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                               "D92 MISSING OR AFTER T92");

        rbt->setOriginalTktIssueDT().addMinutes(atoi(xmlValue.c_str()));
        rbt->setOriginalTktIssueDT().setHistoricalIncludesTime();
      }
      break;
    }
    case 166: // T94 - reissue time
    {
      RexBaseTrx* rexBaseTrx = getRexBaseTrx();
      if (rexBaseTrx)
      {
        if (rexBaseTrx->lastTktReIssueDT().isEmptyDate())
          throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                               "D94 MISSING OR AFTER T94");

        rexBaseTrx->lastTktReIssueDT().addMinutes(atoi(xmlValue.c_str()));
        rexBaseTrx->lastTktReIssueDT().setHistoricalIncludesTime();
      }
      break;
    }
    case 167: // T95 - reissue time
    {
      RexBaseTrx* rbt = getRexBaseTrx();
      if (rbt)
      {
        if (rbt->previousExchangeDT().isEmptyDate())
          throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                               "D95 MISSING OR AFTER T95");

        rbt->previousExchangeDT().addMinutes(atoi(xmlValue.c_str()));
        rbt->previousExchangeDT().setHistoricalIncludesTime();
      }
      break;
    }

    case 168: // DFN - Spanish Large Family discount
    {
      const int16_t value = atoi(xmlValue.c_str());
      _options->setSpanishLargeFamilyDiscountLevel(SLFUtil::getDiscountLevelFromInt(value));
      break;
    }
    case 169: // FP2 - second form of payment
      request->secondFormOfPayment() = xmlValue.c_str();
      break;

    case 170: // RTW - Round the world, sabre terminal qualifier 'RW'
      _options->setRtw(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;

    case 173: // PAT   payment amount for OB FOP
      request->paymentAmountFop() = (double)atof(xmlValue.c_str());
      break;

    case 174: // PRS    charge to residual/apecifiedAmount for OB FOP
      request->chargeResidualInd() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;

    case 175: // MTO - Multi Ticket request option
      if (_inPROCount == 0)
        request->isMultiTicketRequest() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 176: // SHC - Tour code
      request->setTourCode(xmlValue.c_str());
      break;
    case 177: // PBB - Price By Brand
      if (!fallback::fallbackBrandedFaresPricing(_pricingTrx))
      {
        _pricingTrx->setPbbRequest(
            TypeConvert::pssCharToBool(xmlValue.c_str()[0]) ? PBB_RQ_PROCESS_BRANDS : NOT_PBB_RQ);

        if (_pricingTrx->isPbbRequest() &&
            !(_pricingTrx->getTrxType() == PricingTrx::PRICING_TRX &&
              _pricingTrx->altTrxType() == PricingTrx::WP))
        {
          throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT);
        }
      }
      break;

    case 178: // SM0 - Settlement method
      request->setSettlementMethod(xmlValue.c_str());
      break;

    case 179: // SAT - Specific agency text
      request->setSpecificAgencyText(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;

    case 180: // XFF - exclude Fare Focus rule
      _options->setExcludeFareFocusRule(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;

    case 181: // SE2 - Collect R Type OB Fee
      request->setCollectRTypeOBFee(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;

    case 182: // SE3 - Collect T Type OB Fee
      request->setCollectTTypeOBFee(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;

    case 183: // SFT - Service Fees Template
      if (TypeConvert::pssCharToBool(xmlValue.c_str()[0]))
        _options->setServiceFeesTemplateRequested();
      break;

    case 184: // PA2 -- Split Taxes By Fare Component
      if (TrxUtil::isSplitTaxesByFareComponentEnabled(*_pricingTrx))
      {
        _options->setSplitTaxesByFareComponent((xmlValue.c_str())[0] == 'T');
      }
      break;
    case 186: // BRA -- Search For Brands Pricing
      if (!fallback::fallbackSearchForBrandsPricing(_pricingTrx))
      {
        if (_pricingTrx->getTrxType() == PricingTrx::PRICING_TRX &&
            _pricingTrx->altTrxType() == PricingTrx::WP)
        {
          _pricingTrx->modifiableActivationFlags().setSearchForBrandsPricing(
              TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
          _pricingTrx->setNumberOfBrands(maxOptionsInBrandedPricing.getValue(_pricingTrx));
          // in pricing in EXPEDIA disable soft passes (if option set to true)
          _pricingTrx->setSoftPassDisabled(dynamicSoftPassDisabledInTn.isValid(_pricingTrx));
        }
      }
      break;
    case 187: // AAL
      _options->setMatchAndNoMatchRequested(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;
    case 188: // PDO Fare Retailer Rule
      _options->setPDOForFRRule(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;
    case 189: // PDR Fare Retailer Rule
      _options->setPDRForFRRule(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;
    case 190: // XRS  Fare Retailer Rule
      _options->setXRSForFRRule(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;
    case 191: // NBP Number of Bag Pieces
      if (TrxUtil::isBaggageInPQEnabled(*_pricingTrx))
        _pricingTrx->mutableBaggagePolicy().setRequestedBagPieces(atoi(xmlValue.c_str()));
      break;
    case 192: // BI0 - Requested Cabin

      if (!fallback::fallbackPriceByCabinActivation(_pricingTrx))
      {
        if (strcmp(xmlValue.c_str(), "") == 0)
        {
          break;
        }
        else if(!(strlen(xmlValue.c_str()) == 2))
        {
          throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "INVALID CABIN CODE");
        }
        else if (strcmp(xmlValue.c_str(), "PB") == 0)
        {
          _options->cabin().setPremiumFirstClass();
          break;
        }
        else if (strcmp(xmlValue.c_str(), "FB") == 0)
        {
          _options->cabin().setFirstClass();
          break;
        }
        else if (strcmp(xmlValue.c_str(), "JB") == 0)
        {
          _options->cabin().setPremiumBusinessClass();
          break;
        }
        else if (strcmp(xmlValue.c_str(), "BB") == 0)
        {
          _options->cabin().setBusinessClass();
          break;
        }
        else if (strcmp(xmlValue.c_str(), "SB") == 0)
        {
          _options->cabin().setPremiumEconomyClass();
          break;
        }
        else if (strcmp(xmlValue.c_str(), "YB") == 0)
        {
          _options->cabin().setEconomyClass();
          break;
        }
        else if (strcmp(xmlValue.c_str(), "AB") == 0)
        {
          _options->cabin().setAllCabin();
          break;
        }
        else
          throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "INVALID CABIN CODE");
      }
      break;

    case 193: // EBC - Excluded Booking Code
      if (!fallback::fallbackAAExcludedBookingCode(_pricingTrx))
      {
        std::string strEBC = xmlValue.c_str();
        if(!strEBC.empty() && _pricingTrx && _pricingTrx->getOptions())
          _pricingTrx->getOptions()->aaBasicEBC() = strEBC;
      }
    case 194: // MOB - Maximum OB Fees
      if (!fallback::virtualFOPMaxOBCalculation(_pricingTrx))
      {
        if (xmlValue.c_str() != nullptr)
          request->setReturnMaxOBFeeOnly(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      }
    break;

    case 195: // PRM - related to retailer code setting
      if (!fallback::fallbackFRRProcessingRetailerCode(_trx))
        if (xmlValue.c_str()[0] == 'T' && request != nullptr)
        {
          request->setPRM(true);
          isPRM = true;
        }
      break;
    case 196: // Retailer code
      parseRetailerCode(xmlValue, request, rcqCount);
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  checkParsingRetailerCodeError(isPRM, rcqCount);

  restrictCmdPricing();

  restrictExcludeFareFocusRule();

  checkEprForPDOorXRS();
  checkEprForPDR();

  AltPricingTrx* altPricingTrx = dynamic_cast<AltPricingTrx*>(_pricingTrx);
  if (altPricingTrx != nullptr && (altPricingTrx->altTrxType() == AltPricingTrx::WPA) &&
      request->isLowFareRequested())
  {
    request->wpaXm() = true;
  }

  if (altPricingTrx != nullptr && altPricingTrx->billing() != nullptr &&
      altPricingTrx->billing()->requestPath() == SWS_PO_ATSE_PATH &&
      altPricingTrx->altTrxType() == AltPricingTrx::WP)
    request->turnOffNoMatch() = true;

  checkAndAdjustPurchaseDTwithTimeForExchange();

  if (!_isRexTrx && _inPROCount == 0)
  {
    _inPROCount++;
    _reqXmlValidator.validateSaleTicketOverride(_pricingTrx);
    _reqXmlValidator.setTicketingDate(_pricingTrx, d07Present);
    _reqXmlValidator.validatePassengerStatus(
        _pricingTrx, countryCode, stateRegionCode, _currentCRC, _options->residency());
    if (TrxUtil::swsPoAtsePath(*_pricingTrx))
    {
      _reqXmlValidator.setMOverride(_pricingTrx);

      if (!p0jPresent)
        _reqXmlValidator.validateETicketOptionStatus(_pricingTrx);
    }
  }

  storeCountryAndStateRegionCodes(countryCode,
                                  stateRegionCode,
                                  _options->nationality(),
                                  _options->employment(),
                                  _options->residency());

  checkCat35NetSellOption(_pricingTrx);
  _reqXmlValidator.setUpCorrectCurrencyConversionRules(*_pricingTrx);

  if (!fallback::fallbackAgencyRetailerHandlingFees(_pricingTrx))
  {
    _reqXmlValidator.checkCominationPDOorPDRorXRSOptionalParameters(_pricingTrx);
  }
}

void
PricingModelMap::restrictExcludeFareFocusRule()
{
  if (_options->isExcludeFareFocusRule() && !_options->isKeywordPresent(EPR_FFOCUS))
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "NEED KEYWORD FFOCUS");
}
void
PricingModelMap::checkEprForPDOorXRS()
{
  if ((_options->isPDOForFRRule() || _options->isXRSForFRRule()) &&
      !_options->isKeywordPresent(EPR_ORDFQD))
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "NEED KEYWORD ORGFQD");
}

void
PricingModelMap::checkEprForPDR()
{
  if (_options->isPDRForFRRule() && !_options->isKeywordPresent(EPR_AGYRET))
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "NEED KEYWORD AGYRET");
}

void
PricingModelMap::checkAndAdjustPurchaseDTwithTimeForExchange()
{
  if (_isRexTrx && _inPROCount == 0 && _processExchangePricingTrx &&
      _options->ticketTimeOverride() != 0)
  { // Rex && 1st PRO && D54 (time override) && FE/PE/CE/AM
    ExchangePricingTrx* excTrx = getExchangePricingTrx();
    PricingRequest* request = _pricingTrx->getRequest();

    if (excTrx && excTrx->reqType() != TAG_10_EXCHANGE && request &&
        ((!_ticketingDTadjustedWithTime &&
          excTrx->currentTicketingDT() == request->ticketingDT()) ||
         (_ticketingDTadjustedWithTime && excTrx->currentTicketingDT() == _originalTicketingDT)))
    { // CE was done
      DateTime pDT = excTrx->purchaseDT();
      pDT = pDT + tse::Hours(_options->ticketTimeOverride() / 60) +
            tse::Minutes(_options->ticketTimeOverride() % 60) + tse::Seconds(0);
      if (pDT.isValid())
      {
        excTrx->purchaseDT() = pDT;
        excTrx->purchaseDT().setHistoricalIncludesTime();
        request->ticketingDT().setHistoricalIncludesTime();
        DateTime modifiedTktDate(excTrx->dataHandle().ticketDate());
        modifiedTktDate.setHistoricalIncludesTime();
        excTrx->dataHandle().setTicketDate(modifiedTktDate);
      }
    }
  }
}

void
PricingModelMap::storeB50Data(const char* xmlValue)
{
  setRestrictCmdPricing(xmlValue);
  _fareBasisCode = xmlValue;
  if (_options != nullptr && !_isExchangeItin)
    _options->fbcSelected() = true;
}

void
PricingModelMap::setRestrictCmdPricing(const char* str)
{
  if ((!_isExchangeItin) && *str != '\0')
    _restrictCmdPricing = true;
}

void
PricingModelMap::setEPRKeywordFlag(const char* token)
{
  if (_isExchangeItin)
    return;

  if (strcmp(EPR_KEYWORD_MUL375, token) == 0 || strcmp(EPR_KEYWORD_TMPCRS, token) == 0 ||
      strcmp(EPR_KEYWORD_CRSAGT, token) == 0)
  {
    _isEPRKeyword = true;
  }
}

void
PricingModelMap::restrictCmdPricing()
{
  if ((!_isExchangeItin) && _restrictCmdPricing && _isWsUser && (!_isEPRKeyword))
    throw ErrorResponseException(ErrorResponseException::ENTRY_RESTRICTED_TO_EPR_KEYWORD_MUL375);
}

void
PricingModelMap::storeExcItinProcOptsInformation(const xercesc::Attributes& attrs)
{
  if (_pricingTrx == nullptr || _pricingTrx->getOptions() == nullptr ||
      _pricingTrx->getRequest() == nullptr)
    return;

  if (_pricingTrx->excTrxType() == PricingTrx::PORT_EXC_TRX)
  {
    storeExcItinProcOptsInformationNonCat31(attrs);
    return;
  }

  RexBaseTrx* rexTrx = dynamic_cast<RexBaseTrx*>(_pricingTrx);
  if (rexTrx == nullptr)
    return; // Non-Cat31 does not need any options under EXC.

  RexBaseRequest* rexRequest = dynamic_cast<RexBaseRequest*>(_pricingTrx->getRequest());
  RexPricingOptions* rexOptions = dynamic_cast<RexPricingOptions*>(_pricingTrx->getOptions());
  if (rexRequest == nullptr || rexOptions == nullptr)
    return;

  NationCode countryCode, stateRegionCode;
  MoneyAmount excNonRefundableAmt = -EPSILON;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 18: // AC0 - Corp id
      rexRequest->excCorporateID() = xmlValue.c_str();
      break;
    case 53: // B05 - Validating carrier
      // LATAM MCP-S
      if (!fallback::neutralToActualCarrierMapping(_pricingTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        rexRequest->excValidatingCarrier() =
            MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
      }
      else
        rexRequest->excValidatingCarrier() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;
    case 54: // AF0 - Ticketing point override
      rexRequest->ticketPointOverride() = xmlValue.c_str();
      break;
    case 55: // AG0 - Sale point override
      rexRequest->salePointOverride() = xmlValue.c_str();
      break;
    case 91: // S11 - Account code
      rexRequest->excAccountCode() = xmlValue.c_str();
      break;
    case 93: // N0M - Country type 'E' 'N' 'R'
      _currentCRC = xmlValue.c_str();
      break;
    case 94: // A40 - Country code
      countryCode = xmlValue.c_str();
      break;
    case 95: // AH0 - State region code
      stateRegionCode = xmlValue.c_str();
      break;
    case 123: // S94 - Waiver Code for REX
      rexTrx->waiverCode() = xmlValue.c_str();
      break;
    case 125: // C6Y - Fare Calc Currency for REX
      _fareCalcCurrency = xmlValue.c_str();
      break;
    case 127: // A50 - residency of city
      rexOptions->excResidency() = xmlValue.c_str();
      break;
    case 135: // C6P - base fare currency for exc trx
      rexOptions->excBaseFareCurrency() = xmlValue.c_str();
      break;
    case 154: // C5A - Base Fare Amount
      rexOptions->excTotalFareAmt() = xmlValue.c_str();
      break;
    case 156: // C5E - Total fare calc amount
      rexTrx->totalFareCalcAmount() = strtod(xmlValue.c_str(), nullptr);
      break;
    case 168: // DFN - Spanish Large Family discount of exc ticket
    {
      const int16_t value = atoi(xmlValue.c_str());
      SLFUtil::DiscountLevel level = SLFUtil::getDiscountLevelFromInt(value);
      _options->setSpanishLargeFamilyDiscountLevel(level);
      break;
    }
    case 170: // EXC/RTW - Exc tkt was Round the world type one
      rexOptions->setExcTaggedAsRtw(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;
    case 171: // PY6 - Net selling indicator with cat35 it/bt
      rexOptions->setNetSellingIndicator(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;
    case 172: // NRA - Non-Refundable Amount
      excNonRefundableAmt = strtod(xmlValue.c_str(), nullptr);
      break;
    default:
      break;
    }
  }

  // seems we have attribute duplication for cat31/33
  // C5A for c31 means same as C5E for c33 - total price of exc ticket
  // for backward compatibility I will override C5E by C5A for c31 if they differ
  if (rexTrx->excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    const RexPricingTrx& rtrx = static_cast<const RexPricingTrx&>(*rexTrx);
    if (rtrx.isPlusUpCalculationNeeded())
    {
      if (rexOptions->excTotalFareAmt().empty() && rexTrx->totalFareCalcAmount() < 0.0)
        throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                     "MISSING EXC TKT AMT - C5A OR C5E");

      rexTrx->totalFareCalcAmount() = strtod(rexOptions->excTotalFareAmt().c_str(), nullptr);
    }

    if (rexOptions->excBaseFareCurrency().empty() || rexOptions->excTotalFareAmt().empty())
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                   "MISSING EXC TKT AMOUNT/CURRENCY - C5A/C6P");

    rexTrx->setTotalBaseFareAmount(Money(strtod(rexOptions->excTotalFareAmt().c_str(), nullptr),
                                         rexOptions->excBaseFareCurrency()));
  }

  if (rexTrx->excTrxType() == PricingTrx::AR_EXC_TRX ||
      rexTrx->excTrxType() == PricingTrx::AF_EXC_TRX)
  {
    if (excNonRefundableAmt != -EPSILON)
    {
      if (rexOptions->excBaseFareCurrency().empty())
        throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                     "MISSING EXC TKT CURRENCY - C6P");

      rexTrx->setExcNonRefAmount(Money(excNonRefundableAmt, rexOptions->excBaseFareCurrency()));
    }
  }

  storeCountryAndStateRegionCodes(countryCode,
                                  stateRegionCode,
                                  rexOptions->excNationality(),
                                  rexOptions->excEmployment(),
                                  rexOptions->excResidency());
}

void
PricingModelMap::storeExcItinProcOptsInformationNonCat31(const xercesc::Attributes& attrs)
{
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 125: // C6Y - For CE type non-cat31 transactions take fare calc currency from exchange itin
      if (static_cast<ExchangePricingTrx&>(*_pricingTrx).reqType() == TAG_10_EXCHANGE)
        _fareCalcCurrency = xmlValue.c_str();
      break;

    default:
      break;
    }
  }
}

void
PricingModelMap::storeCountryAndStateRegionCodes(const NationCode& countryCode,
                                                 const NationCode& stateRegionCode,
                                                 NationCode& nationality,
                                                 NationCode& employment,
                                                 LocCode& residency)
{
  NationCode code(countryCode);
  for (int i = 0; i < 2; i++)
  {
    if (!code.empty())
    {
      if (_currentCRC == "E")
      {
        if (employment.size() > 0)
          employment += code;
        else
          employment = code;
      }
      else if (_currentCRC == "N")
      {
        nationality = code;
      }
      else if (_currentCRC == "R")
      {
        if (residency.size() > 0)
          residency += code;
        else
          residency = code;
      }
    }
    code = stateRegionCode;
  }
}

RexPricingTrx*
PricingModelMap::getRexPricingTrx()
{
  if (_pricingTrx && _pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX)
    return static_cast<RexPricingTrx*>(_pricingTrx);

  return nullptr;
}

RefundPricingTrx*
PricingModelMap::getRefundPricingTrx()
{
  if (_pricingTrx && _pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX)
    return static_cast<RefundPricingTrx*>(_pricingTrx);

  return nullptr;
}

RexBaseTrx*
PricingModelMap::getRexBaseTrx()
{
  RexBaseTrx* rexBaseTrx = getRexPricingTrx();

  if (!rexBaseTrx)
    rexBaseTrx = getRefundPricingTrx();

  return rexBaseTrx;
}

ExchangePricingTrx*
PricingModelMap::getExchangePricingTrx()
{
  if (_pricingTrx)
    return dynamic_cast<ExchangePricingTrx*>(_pricingTrx);

  return nullptr;
}

bool
PricingModelMap::corpIdExist() const
{
  return (_pricingTrx->dataHandle().corpIdExists(_pricingTrx->getRequest()->corporateID(),
                                                 _pricingTrx->ticketingDate()));
}

void
PricingModelMap::saveProcOptsInformation()
{
  PricingRequest* request;
  LOG4CXX_DEBUG(logger, "In saveProcOptsInformation");

  request = _pricingTrx->getRequest();

  if (request != nullptr)
  {
    updateAgentInformation();

    if (!_isExchangeItin && !request->corporateID().empty() && !corpIdExist())
    {
      std::string errMsg = "INVALID CORPORATE ID ";
      errMsg += request->corporateID();
      throw ErrorResponseException(ErrorResponseException::INVALID_CORP_ID, errMsg.c_str());
    }
    // There will always be a one to one correspondence between
    // _taxCodeOverrides and _overrideTaxAmounts
    uint16_t numOverrides = _taxCodeOverrides.size();
    for (uint16_t i = 0; i < numOverrides; i++)
    {
      if (i >= _overrideTaxAmounts.size())
        break;

      TaxOverride* taxOverride;
      _pricingTrx->dataHandle().get(taxOverride); // lint !e530
      taxOverride->taxCode() = _taxCodeOverrides[i];
      taxOverride->taxAmt() = _overrideTaxAmounts[i];
      request->taxOverride().push_back(taxOverride);
    }
    _taxCodeOverrides.clear();
    _overrideTaxAmounts.clear();

    if (!_isRexTrx)
      _reqXmlValidator.validateCurrencyCode(_pricingTrx);

    if (!_isExchangeItin && _pricingTrx->getOptions() &&
        !_pricingTrx->getOptions()->isMOverride() && !request->salePointOverride().empty())
    {
      CurrencyUtil::getSaleLocOverrideCurrency(request->salePointOverride(),
                                               _pricingTrx->getOptions()->currencyOverride(),
                                               request->ticketingDT());
    }

    // Mark multi account code / corporate ID scenario
    if (!request->accCodeVec().empty() || !request->corpIdVec().empty() ||
        !request->incorrectCorpIdVec().empty())
    {
      request->isMultiAccCorpId() = true;
    }

    const std::vector<std::string>& incorrectCorpIdVec = request->incorrectCorpIdVec();

    if (!incorrectCorpIdVec.empty() && request->corpIdVec().empty() &&
        request->accCodeVec().empty())
    {
      std::string errMsg = "INVALID CORPORATE ID";
      std::vector<std::string>::const_iterator corpIdIter = incorrectCorpIdVec.begin();
      std::vector<std::string>::const_iterator corpIdEnd = incorrectCorpIdVec.end();

      while (corpIdIter != corpIdEnd)
      {
        errMsg = errMsg + " " + (*corpIdIter);
        ++corpIdIter;
      }

      // if(request->ticketingAgent() && request->ticketingAgent()->axessUser())
      //{
      //    errMsg = std::string("VE\n") + errMsg;
      //}

      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_CORP_ID, errMsg.c_str());
    }
    if (!fallback::fallbackPriceByCabinActivation(_pricingTrx)  &&
        !_pricingTrx->getOptions()->cabin().isUndefinedClass() &&
        !_pricingTrx->getOptions()->cabin().isAllCabin())
    {
      _pricingTrx->getRequest()->setjumpUpCabinAllowed();
    }
  }
}

void
PricingModelMap::storePassengerInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePassengerInformation");
  if (_paxType == nullptr)
  {
    _trx->dataHandle().get(_paxType);
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B70 - Passenger type
      _paxType->paxType() = xmlValue.c_str();
      break;
    case 2: // Q0U - Passenger type count
      _paxType->number() = atoi(xmlValue.c_str());
      break;
    case 3: // Q0T - Passenger child age
      _paxType->age() = atoi(xmlValue.c_str());
      break;
    case 4: // A30 - Passenger state code
      _paxType->stateCode() = xmlValue.c_str();
      break;
    case 5: // Q79 - Total number of passengers
      _paxType->totalPaxNumber() = atoi(xmlValue.c_str());
      break;
    case 6: // MPO - Maximum penalty operation
      if (xmlValue == "I")
        _mpo = smp::INFO;
      else if (xmlValue == "O")
        _mpo = smp::OR;
      else if (xmlValue == "A")
        _mpo = smp::AND;
      else
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                             "INCORRECT MAXIMUM PENALTY OPERATION");
      break;
    case 7: // C51 - MSL adjustment value
      processManualAdjustmentAmount(xmlValue);
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  if (_options != nullptr && _options->isFareFamilyType())
  {
    if (_paxType != nullptr && !FareTypePricingUtil::matchNormalPaxType(_paxType->paxType()))
      throw ErrorResponseException(
          ErrorResponseException::PAXTYPE_NOT_ALLOWED_WITH_FARE_TYPE_PRICING);
  }
  if (!_isRexTrx)
  {
    if (_pricingTrx->billing()->requestPath() == SWS_PO_ATSE_PATH)
      _reqXmlValidator.validatePassengerTypeWithAges(*_paxType);

    _reqXmlValidator.validateDuplicatePassengerType(_pricingTrx, *_paxType);
    _reqXmlValidator.validatePassengerType(_pricingTrx, *_paxType);
  }
}

bool
PricingModelMap::isValidAmount(std::string& s, uint16_t currencyNoDec)
{
  boost::trim(s);
  if (s.empty())
    return false;

  bool isNegative = (s[0] == '-');
  if (isNegative)
  {
    s.erase(0, 1);
    boost::trim(s);
    if (s.empty())
      return false;
  }

  boost::trim_left_if(s, boost::is_any_of("0"));

  int decPos = -1;
  for (unsigned int i = 0; i < s.size(); ++i)
  {
    char c = s[i];
    if (c == '.')
    {
      if (decPos == -1)
        decPos = i;
      else
        return false; // no double decimal points
    }
    else if (!isdigit(c))
      return false;
  }

  if (decPos >= 0)
  {
    boost::trim_right_if(s, boost::is_any_of("0"));
    boost::trim_right_if(s, boost::is_any_of("."));
  }

  // If nothing is left, it is all 0s with or without decimal point
  if (s.empty())
  {
    if (isNegative)
      return false; // 0 is acceptable but -0 is not

    s = "0";
    return true;
  }

  size_t totalSize = s.size();
  if (totalSize > 11)
    return false;

  if (decPos == (int)totalSize)
    decPos = -1; // We removed . from the end above

  size_t noDec = (decPos >= 0) ? totalSize - decPos - 1 : 0;
  if (noDec > currencyNoDec)
    return false;

  if (isNegative)
    s = "-" + s;

  return true;
}

void
PricingModelMap::processManualAdjustmentAmount(const XMLChString& value)
{
  if (_options && !_options->isKeywordPresent(EPR_ORDFQD))
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "NEED KEYWORD ORGFQD");

  CurrencyCode paymentCurrency = PricingUtil::getPaymentCurrency(*_pricingTrx);
  Money money(paymentCurrency);

  std::string s = value.c_str();
  if (!isValidAmount(s, money.noDec(_pricingTrx->ticketingDate())))
    throw ErrorResponseException(ErrorResponseException::INVALID_MSL_AMOUNT_SPECIFIED);

  _paxType->mslAmount() = (double)atof(s.c_str());

  if (_options != nullptr)
    _options->setMslRequest();
}

void
PricingModelMap::savePassengerInformation()
{
  LOG4CXX_DEBUG(logger, "In savePassengerInformation");
  if (_paxType != nullptr)
  {
    _paxType->maxPenaltyInfo() = CommonParserUtils::validateMaxPenaltyInput(
        *_pricingTrx, std::move(_mpo), std::move(_mpChangeFilter), std::move(_mpRefundFilter));

    if (_isExchangeItin)
    {
      if (_pricingTrx != nullptr)
      {
        RexBaseTrx* rexBaseTrx = dynamic_cast<RexBaseTrx*>(_pricingTrx);
        if (rexBaseTrx)
        {
          PaxTypeUtil::initialize(*_pricingTrx,
                                  *_paxType,
                                  _paxType->paxType(),
                                  _paxType->number(),
                                  _paxType->age(),
                                  _paxType->stateCode(),
                                  0);

          rexBaseTrx->exchangePaxType() = _paxType;
          rexBaseTrx->excPaxType().push_back(_paxType);
        }
      }
    }
    else
    {
      // Calculate order number for PaxType in the input entry
      uint16_t order = 99;

      if (_options->lineEntry().find(DOLLAR_P) != std::string::npos ||
          _options->lineEntry().find(WP_P) != std::string::npos)
      {
        order = inputPaxTypeOrder(_options->lineEntry());
      }
      if (order != 99)
      {
        _paxInputOrder = order;
      }
      // If the passenger type is not ADT and exists increment number of
      // this type
      if (_paxType->paxType() == ADULT || _pricingTrx->paxType().size() == 0)
      {
        PaxTypeUtil::initialize(*_pricingTrx,
                                *_paxType,
                                _paxType->paxType(),
                                _paxType->number(),
                                _paxType->age(),
                                _paxType->stateCode(),
                                _paxInputOrder++);
        _pricingTrx->paxType().push_back(_paxType);
      }
      else
      {
        unsigned int i = 0;
        std::vector<PaxType*>::const_iterator paxIter;
        _pricingTrx->paxType().begin();
        for (paxIter = _pricingTrx->paxType().begin(); paxIter != _pricingTrx->paxType().end();
             paxIter++)
        {
          if (_pricingTrx->paxType()[i]->paxType() == _paxType->paxType() &&
              _pricingTrx->paxType()[i]->age() == _paxType->age())
          {
            (*paxIter)->number() += _paxType->number();
            break;
          }
        }

        // Not found
        if (paxIter == _pricingTrx->paxType().end())
        {
          PaxTypeUtil::initialize(*_pricingTrx,
                                  *_paxType,
                                  _paxType->paxType(),
                                  _paxType->number(),
                                  _paxType->age(),
                                  _paxType->stateCode(),
                                  _paxInputOrder++);
          _pricingTrx->paxType().push_back(_paxType);
        }
      }
      // cout  << "Order - " << order << " for PAX - " << _paxType->paxType()
      //      << "   Age = " << _paxType->age() << "  PAX input order - " << _paxInputOrder << endl;
    }

    _paxType = nullptr;
  }
}

void
PricingModelMap::storePenaltyInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePenaltyInformation");

  if (_pricingTrx->isExchangeTrx())
  {
    return;
  }

  enum MaxPenaltyType
  { MAX_PENALTY_CHANGE,
    MAX_PENALTY_REFUND };

  smp::RecordApplication abd = smp::BOTH;
  boost::optional<MaxPenaltyType> mpt;
  boost::optional<smp::ChangeQuery> mpi;
  boost::optional<MoneyAmount> mpa;
  boost::optional<CurrencyCode> mpc;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // ABD - After/before departure
      if (xmlValue == "A")
        abd = smp::AFTER;
      else if (xmlValue == "B")
        abd = smp::BEFORE;
      else
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                             "INCORRECT FORMAT BEFORE/AFTER DEPARTURE");
      break;
    case 2: // MPT - Change type
      if (xmlValue == "C")
        mpt = MAX_PENALTY_CHANGE;
      else if (xmlValue == "R")
        mpt = MAX_PENALTY_REFUND;
      else
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                             "INCORRECT MAXIMUM PENALTY CHANGE TYPE");
      break;
    case 3: // MPI - Maximum change condition
      if (xmlValue == "A")
        mpi = smp::CHANGEABLE;
      else if (xmlValue == "N")
        mpi = smp::NONCHANGEABLE;
      else
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                             "INCORRECT MAXIMUM PENALTY CHANGE CONDITION");
      break;
    case 4: // MPA - Maximum change amount
      MoneyAmount mcAmount;
      if (xmlValue.parse(mcAmount))
        mpa = mcAmount;
      else
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                             "INVALID MAXIMUM PENALTY AMOUNT - MODIFY AND REENTER");
      break;
    case 5: // MPC - Maximum change currency
      mpc = xmlValue.c_str();
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  const MaxPenaltyInfo::Filter filter =
      CommonParserUtils::validateMaxPenaltyFilter(*_pricingTrx, abd, mpi, mpa, mpc);

  if (!mpt || mpt.get() == MAX_PENALTY_CHANGE)
  {
    _mpChangeFilter = filter;
  }

  if (!mpt || mpt.get() == MAX_PENALTY_REFUND)
  {
    _mpRefundFilter = filter;
  }
}

void
PricingModelMap::savePenaltyInformation()
{
  LOG4CXX_DEBUG(logger, "In savePenaltyInformation");
}

void
PricingModelMap::storeSegmentInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeSegmentInformation");
  PricingRequest* request = nullptr;

  if (_pricingTrx != nullptr)
    request = _pricingTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q0C - Segment order
      // We don't know what type of segment we're dealing with yet.
      _segmentNumber = atoi(xmlValue.c_str());
      break;
    case 2: // A01 - Board point
      request->boardPoint() = xmlValue.c_str();
      break;
    case 3: // A02 - Off point
      request->offPoint() = xmlValue.c_str();
      break;
    case 10: // N0E - Consider only cabin
      //           (dynamic_cast<AirSeg*>(_currentTvlSeg))->considerOnlyCabin() = xmlValue.c_str();
      _considerOnlyCabin = xmlValue.c_str();
      break;
    case 11: // P72 - Forced connection (T/F)
      _forcedConx = (xmlValue.c_str())[0];
      break;
    case 12: // P73 - Forced stop over (T/F)
      _forcedStopOver = (xmlValue.c_str())[0];
      break;
    case 13: // P74 - Forced fare break (T/F)
      if (_pricingTrx->excTrxType() != PricingTrx::AF_EXC_TRX || !_isExchangeItin)
        _forcedFareBreak = (xmlValue.c_str())[0];
      break;
    case 14: // P75 - Forced no fare break (T/F)
      _forcedNoFareBreak = (xmlValue.c_str())[0];
      break;
    case 15: // P76 - Forced side trip (T/F)
      _forcedSideTrip = (xmlValue.c_str())[0];
      break;
    case 16: // C6I - Discount amount ($)
      setRestrictCmdPricing(xmlValue.c_str());
      if (!fallback::azPlusUp(_trx))
        _discountAmountNew = strtod(xmlValue.c_str(), nullptr);
      else
        _discountAmount = strtod(xmlValue.c_str(), nullptr);
      break;
    case 17: // Q17 - Discount percentage (short)
      setRestrictCmdPricing(xmlValue.c_str());

      if (!fallback::azPlusUp(_trx))
        _discountPercentageNew = strtod(xmlValue.c_str(), nullptr);
      else
        _discountPercentage = strtod(xmlValue.c_str(), nullptr);
      break;
    case 18: // B50 - Fare basis code (char[10])
      storeB50Data(xmlValue.c_str());

      break;
    case 19: // B02 - Governing carrier (char[4])
      //            don't use B02 as a GoverningCarrier, use B00 only.
      //            (dynamic_cast<AirSeg*>(_currentTvlSeg))->carrier() =
      // MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;
    case 22: // B00 - Governing Carrier Override
      _cxrOverride = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;
    case 25: // BE0 - Ticket Designator
      _tktDesignator = xmlValue.c_str();
      break;
    case 26: // B51 - specified Fare basis code (char[10])
      setRestrictCmdPricing(xmlValue.c_str());

      _specifiedFbc = xmlValue.c_str();
      break;
    case 27: // C48 - Discount/MarkUp Amount Currency Code
      _discountCurrencyCode = xmlValue.c_str();
      break;
    case 28: // Q12 - Discount/MarkUp Amount group number
      _discountGroupNum = atoi(xmlValue.c_str()) < 0 ? 0 : atoi(xmlValue.c_str());
      break;
    case 29: // BD0 - Specified Ticket Designator
      setRestrictCmdPricing(xmlValue.c_str());

      _specifiedTktDesignator = xmlValue.c_str();
      break;
    case 30: // Q6D - Fare Component Number
      _fareCompNum = atoi(xmlValue.c_str());
      break;
    case 31: // PCI - Segment Flown
      _segFlown = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 32: // C50 - Fare Amount
      _fareCompAmount = strtod(xmlValue.c_str(), nullptr);
      _fareCompAmountStr = xmlValue.c_str();
      break;
    case 33: // A60 - Global Direction Override
      _globalDirectionOverride = xmlValue.c_str();
      break;
    case 34: // Q6E - Side Trip Number
      _sideTripNumber = atoi(xmlValue.c_str());
      break;
    case 35: // S07 - Side Trip Start
      _sideTripStart = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 36: // S08 - Side Trip End
      _sideTripEnd = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 37: // S37 - Fare Vendor
      _VCTR.vendor() = xmlValue.c_str();
      _hasVCTR.set(VCTR_Vendor, !xmlValue.empty());
      break;
    case 38: // B09 - Fare Source Carrier; governing carrier even if YY fare is used
      _VCTR.carrier() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      _hasVCTR.set(VCTR_Carrier, !xmlValue.empty());
      break;
    case 39: // S89 - Fare Tariff
      _VCTR.tariff() = atoi(xmlValue.c_str());
      _hasVCTR.set(VCTR_Tariff, !xmlValue.empty());
      break;
    case 40: // S90 - Fare Rule
      _VCTR.rule() = xmlValue.c_str();
      _hasVCTR.set(VCTR_Rule, !xmlValue.empty());
      break;
    case 41: // A07 - Fare Basis code usage- command/filter/branded
      _fbcUsage = (xmlValue.c_str())[0];
      break;
    case 42: // SB2 - BrandCode
      _brandCode = xmlValue.c_str();
      break;
    case 43: // DMA - Mark Up amount
      if (!fallback::azPlusUp(_trx))
      {
        setRestrictCmdPricing(xmlValue.c_str());
        _discountAmount = -1; // reset to default value -- remove with fallback
        _discountAmountNew = -strtod(xmlValue.c_str(), nullptr);
        break;
      }
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    case 44: // DMP - Mark Up percentage
      if (!fallback::azPlusUp(_trx))
      {
        setRestrictCmdPricing(xmlValue.c_str());
        _discountPercentage = -1; // reset to default value -- remove with fallback
        _discountPercentageNew = -strtod(xmlValue.c_str(), nullptr);
        break;
      }
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    case 45: // Q0L - Leg id
      _legId = parseLegId(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");

      break;
    }
  }

  restrictCmdPricing();

  if (!_isExchangeItin || (_pricingTrx->excTrxType() != PricingTrx::AR_EXC_TRX &&
                           _pricingTrx->excTrxType() != PricingTrx::AF_EXC_TRX))
    return;

  if (!_fareBasisCode.empty() && !_fareCompNum)
    throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE);

  checkSideTrip();
}

void
PricingModelMap::storePassengerTypeFlightInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePassengerTypeFlightInformation");
  PricingRequest* request = nullptr;

  if (!_pricingTrx || !(request = _pricingTrx->getRequest()))
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  if (!request->isSFR())
  {
    LOG4CXX_WARN(logger, "Request is not Structure Fare Rule type");
    return;
  }

  _pricingTrx->setMultiPassengerSFRRequestType();
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    const XMLChString xmlNameStr(attrs.getLocalName(i));
    const XMLChString xmlValueStr(attrs.getValue(i));

    const char* xmlName = xmlNameStr.c_str();
    const char* xmlValue = xmlValueStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlName)])
    {
    case 1: // B50 - Fare basis code
      storeB50Data(xmlValue);
      break;

    case 2: // Q6D - Fare Component Number
      _fareCompNum = atoi(xmlValue);
      break;

    case 3: // B70 - Pax Type Code
      {
        auto paxTypeIt = std::find_if(_pricingTrx->paxType().cbegin(),
                                      _pricingTrx->paxType().cend(),
                                      [xmlValue](const PaxType* paxType)
                                      { return paxType->paxType() == xmlValue; });

        if (paxTypeIt != _pricingTrx->paxType().cend())
          _paxType = *paxTypeIt;
        else
          throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE);
      }
      break;

    case 4: // C51 - Fare Amount
      _fareCompAmount = strtod(xmlValue, nullptr);
      _fareCompAmountStr = xmlValue;
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlValue << ", not mapped");
      break;
    }
  }
  restrictCmdPricing();

  if (_fareBasisCode.empty() || !_fareCompNum)
    throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE);

  checkSideTrip();
}

void
PricingModelMap::savePassengerTypeFlightInformation()
{
  LOG4CXX_DEBUG(logger, "In savePassengerTypeFlightInformation");
  if (!_pricingTrx->getRequest()->isSFR())
  {
    LOG4CXX_WARN(logger, "Request is not Structure Fare Rule type");
    return;
  }
  if (!_fareBasisCode.empty())
  {
    _currentTvlSeg->fareBasisCode() = _fareBasisCode;
    _currentTvlSeg->fbcUsage() = _fbcUsage;
  }

  saveFareComponent();
}

// Preventive check for side trip.
// Ensure that side trip information is complete
void
PricingModelMap::checkSideTrip()
{
  if (_sideTripStart && _insideSideTrip)
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                 "SIDE TRIP DATA ERROR");

  if (_sideTripStart)
    _insideSideTrip = true;

  if ((_sideTripStart && _sideTripEnd) || (_sideTripNumber && !_insideSideTrip) ||
      (!_sideTripNumber && _insideSideTrip))
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                 "SIDE TRIP DATA ERROR");

  if (_sideTripEnd)
    _insideSideTrip = false;
}

void
PricingModelMap::saveSegmentInformation()
{
  LOG4CXX_DEBUG(logger, "In saveSegmentInformation");

  saveExchangeOverrides();

  if (checkRequestedFareBasisInformation())
    saveRequestedFareBasisInSegmentInformation();

  _surchargeOverride.clear();
  _stopoverOverride.clear();
  _currentTvlSeg = nullptr;
  _airSeg = nullptr;
  _arunkSeg = nullptr;
  _forcedConx = 'F';
  _forcedStopOver = 'F';
  _forcedSideTrip = 'F';
  _forcedFareBreak = 'F';
  _forcedNoFareBreak = 'F';
  _cxrOverride.clear();
  _globalDirectionOverride.clear();
  _fareBasisCode.clear();
  _fbcUsage = COMMAND_PRICE_FBC;
  _specifiedFbc.clear();
  _discountGroupNum = 0;
  _discountAmount = -1.0; // Indicate there is no DA since 0.0 will be valid.
  _discountAmountNew = 0.0;
  _discountCurrencyCode = "";
  _discountPercentage = -1.0; // Indicate there is no DP since 0.0 will be valid.
  _discountPercentageNew = 0.0;
  _tktDesignator = "";
  _specifiedTktDesignator = "";
  _fareCompNum = 0;
  _segFlown = false;
  _fareCompAmount = 0.0;
  _fareCompAmountStr = "";
  _sideTripNumber = 0;
  _sideTripStart = false;
  _sideTripEnd = false;
  _hasVCTR.clear(VCTR_AllSet);
  _VCTR.clear();
  _considerOnlyCabin.clear();
  _brandCode.clear();
}


void
PricingModelMap::storeTaxInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeTaxInformation");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // BC0 - Tax code
      _taxRequestedInfo.push_back(xmlValue.c_str());
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveTaxInformation()
{
  LOG4CXX_DEBUG(logger, "In saveTaxInformation");
}

void
PricingModelMap::storeDiagInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeDiagInformation");

  PricingRequest* request = nullptr;

  if (_pricingTrx)
    request = _pricingTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // QOA - Diag Argument Type
      request->diagArgType().push_back(xmlValue.c_str());
      break;
    case 2: // S01 - Diag Argument Data
      request->diagArgData().push_back(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveDiagInformation()
{
  LOG4CXX_DEBUG(logger, "In saveDiagInformation");
}

void
PricingModelMap::storeReservationInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeReservationInformation");

  PricingRequest* request = _pricingTrx->getRequest();

  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  ReservationData*& reservData = request->reservationData();
  if (reservData == nullptr)
  {
    _pricingTrx->dataHandle().get(reservData);
    if (reservData == nullptr)
    {
      LOG4CXX_WARN(logger, "Error allocate ReservationData from DataHandle");
      return;
    }
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S71 - Owner/Creator
      reservData->agent() = xmlValue.c_str();
      break;
    case 2: // E40 - Owner/Creator Ind.
      reservData->agentInd() = xmlValue.c_str()[0];
      break;
    case 3: // AB0 - IATA number
      reservData->agentIATA() = xmlValue.c_str();
      break;
    case 4: // A20 - Pseudo city code
      reservData->agentPCC() = xmlValue.c_str();
      break;
    case 5: // A10 - City
      reservData->agentCity() = xmlValue.c_str();
      break;
    case 6: // A40 - Country
      reservData->agentNation() = xmlValue.c_str();
      break;
    case 7: // S72 - CRS
      reservData->agentCRS() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveReservationInformation()
{
  LOG4CXX_DEBUG(logger, "In saveReservationInformation");
}

void
PricingModelMap::storeItinSegmentInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeItinSegmentInformation");

  PricingRequest* request = _pricingTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  ReservationData*& reservData = request->reservationData();
  if (reservData == nullptr)
  {
    LOG4CXX_WARN(logger, "ReservationData is NULL in PricingRequest object");
    return;
  }

  ReservationSeg* reservSeg;
  _pricingTrx->dataHandle().get(reservSeg); // lint !e530
  if (reservSeg == nullptr)
  {
    LOG4CXX_WARN(logger, "Error allocate ReservSeg from DataHandle");
    return;
  }

  reservData->reservationSegs().push_back(reservSeg);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B00 - Airline Code
      reservSeg->carrier() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;
    case 2: // Q0B - Flight Number
      reservSeg->flightNumber() = atoi(xmlValue.c_str());
      break;
    case 3: // B30 - Class of Service
      reservSeg->bookingCode() = fallback::purgeBookingCodeOfNonAlpha(_trx)
                                     ? xmlValue.c_str()
                                     : DataModelMap::purgeBookingCodeOfNonAlpha(xmlValue.c_str());
      break;

    case 4: // D01 - Departure date (YYYY-MM-DD)
      reservSeg->pssDepartureDate() = xmlValue.c_str();
      break;
    case 5: // D31 - Departure Time
      reservSeg->pssDepartureTime() = xmlValue.c_str();
      break;
    case 6: // A01 - Board Point
      reservSeg->origAirport() = xmlValue.c_str();
      break;
    case 7: // D02 - Arrival Date
      reservSeg->pssArrivalDate() = xmlValue.c_str();
      break;

    case 8: // D32 - Arrival Time
      reservSeg->pssArrivalTime() = xmlValue.c_str();
      break;
    case 9: // A02 - Off Point
      reservSeg->destAirport() = xmlValue.c_str();

      break;
    case 10: // A70 - Status Code
      reservSeg->actionCode() = xmlValue.c_str();
      break;
    case 11: // Q0U - Number in Party
      reservSeg->numInParty() = atoi(xmlValue.c_str());
      break;
    case 12: // N1N - Married segment ctl ID
      reservSeg->marriedSegCtrlId() = xmlValue.c_str()[0];
      break;
    case 13: // Q4L - Married Group Number
      reservSeg->marriedGrpNo() = atoi(xmlValue.c_str());
      break;
    case 14: // Q1K - Married Sequence Number
      reservSeg->marriedSeqNo() = atoi(xmlValue.c_str());
      break;
    case 15: // N0V - Polling Indicator
      reservSeg->pollingInd() = xmlValue.c_str()[0];
      break;
    case 16: // P2X - Electronic Tkt Indicator
      reservSeg->eticket() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveItinSegmentInformation()
{
  LOG4CXX_DEBUG(logger, "In saveItinSegmentInformation");
}

void
PricingModelMap::storeRecordLocatorInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeRecordLocatorInformation");

  PricingRequest* request = _pricingTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  ReservationData*& reservData = request->reservationData();
  if (reservData == nullptr)
  {
    LOG4CXX_WARN(logger, "ReservationData is NULL in PricingRequest object");
    return;
  }

  RecordLocatorInfo* recordLocator;
  _pricingTrx->dataHandle().get(recordLocator); // lint !e530
  if (recordLocator == nullptr)
  {
    LOG4CXX_WARN(logger, "Error allocate RecordLocatorInfo from DataHandle");
    return;
  }

  reservData->recordLocators().push_back(recordLocator);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B01 - Origianting Carrier Code
      recordLocator->originatingCxr() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;
    case 2: // S13 - Originating Record Locator
      recordLocator->recordLocator() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveRecordLocatorInformation()
{
  LOG4CXX_DEBUG(logger, "In saveRecordLocatorInformation");
}

void
PricingModelMap::storeReservPassengerInformation(const xercesc::Attributes& attrs)

{
  LOG4CXX_DEBUG(logger, "In storeReservPassengerInformation");

  PricingRequest* request = _pricingTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  ReservationData*& reservData = request->reservationData();
  if (reservData == nullptr)
  {
    LOG4CXX_WARN(logger, "ReservationData is NULL in PricingRequest object");
    return;
  }

  DataHandle& dataHandle = _pricingTrx->dataHandle();
  Traveler* traveler = nullptr;
  dataHandle.get(traveler);
  if (traveler == nullptr)
  {
    LOG4CXX_WARN(logger, "Error allocate Traveler from DataHandle");
    return;
  }

  reservData->passengers().push_back(traveler);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S73 - Last name Qualifier
      traveler->lastNameQualifier() = xmlValue.c_str();
      break;
    case 2: // S74 - Travelers Last Name
      traveler->lastName() = xmlValue.c_str();
      break;
    case 3: // Q4M - Travelers Reference number
      traveler->referenceNumber() = atoi(xmlValue.c_str());
      break;
    case 4: // S75 - First Name Qualifier
      traveler->firstNameQualifier() = xmlValue.c_str();
      break;
    case 5: // PBF - Traveler with Infant Ind
      traveler->travelWithInfant() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    case 6: // S76 - Traveler First Name
      traveler->firstName() = xmlValue.c_str();
      break;
    case 7: // S77 - Traveler Other Name
      traveler->otherName() = xmlValue.c_str();
      break;
    case 8: // N1O - FREQ Flyer VIP-type
    {
      FrequentFlyerAccount*& freqFlyerAcct = traveler->freqFlyerAccount();
      if (freqFlyerAcct == nullptr)
      {
        dataHandle.get(freqFlyerAcct);
        if (freqFlyerAcct == nullptr)
        {
          LOG4CXX_WARN(logger, "Error allocate FrequentFlyerAccount from DataHandle");
          return;
        }
      }
      freqFlyerAcct->vipType() = xmlValue.c_str()[0];
      freqFlyerAcct->tierLevel() = xmlValue.c_str();
    }
    break;
    case 9: // B00 - FREQ Flyer Carrier Code
    {
      FrequentFlyerAccount*& freqFlyerAcct = traveler->freqFlyerAccount();
      if (freqFlyerAcct == nullptr)
      {
        dataHandle.get(freqFlyerAcct);
        if (freqFlyerAcct == nullptr)
        {
          LOG4CXX_WARN(logger, "Error allocate FrequentFlyerAccount from DataHandle");
          return;
        }
      }
      freqFlyerAcct->carrier() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
    }
    break;
    case 10: // S78 - FREQ Flyer Number
    {
      FrequentFlyerAccount*& freqFlyerAcct = traveler->freqFlyerAccount();
      if (freqFlyerAcct == nullptr)
      {
        dataHandle.get(freqFlyerAcct);
        if (freqFlyerAcct == nullptr)
        {
          LOG4CXX_WARN(logger, "Error allocate FrequentFlyerAccount from DataHandle");
          return;
        }
      }
      freqFlyerAcct->accountNumber() = xmlValue.c_str();
    }
    break;
    case 11: // B01 - FREQ Flyer Partner
    {
      FrequentFlyerAccount*& freqFlyerAcct = traveler->freqFlyerAccount();
      if (freqFlyerAcct == nullptr)
      {
        dataHandle.get(freqFlyerAcct);
        if (freqFlyerAcct == nullptr)
        {
          LOG4CXX_WARN(logger, "Error allocate FrequentFlyerAccount from DataHandle");
          return;
        }
      }
      freqFlyerAcct->partner().push_back(xmlValue.c_str());
    }
    break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveReservPassengerInformation()
{
  LOG4CXX_DEBUG(logger, "In saveReservPassengerInformation");
}

void
PricingModelMap::storeCorpFreqFlyerInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeCorpFreqFlyerInformation");

  PricingRequest* request = _pricingTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  ReservationData*& reservData = request->reservationData();
  if (reservData == nullptr)
  {
    LOG4CXX_WARN(logger, "ReservationData is NULL in PricingRequest object");
    return;
  }

  FrequentFlyerAccount* corpFreqFlyerAcct = nullptr;
  _pricingTrx->dataHandle().get(corpFreqFlyerAcct);
  if (corpFreqFlyerAcct == nullptr)
  {
    LOG4CXX_WARN(logger, "Error allocate FrequentFlyerAccount from DataHandle");
    return;
  }

  reservData->corpFreqFlyerAccounts().push_back(corpFreqFlyerAcct);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B00 - FREQ Flyer Carrier Code
      corpFreqFlyerAcct->carrier() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;
    case 2: // S78 - FREQ Flyer Number
      corpFreqFlyerAcct->accountNumber() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveCorpFreqFlyerInformation()
{
  LOG4CXX_DEBUG(logger, "In saveCorpFreqFlyerInformation");
}

void
PricingModelMap::storeFreqFlyerPartnerInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeFreqFlyerPartnerInformation");

  PricingRequest* request = _pricingTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  ReservationData*& reservData = request->reservationData();
  if (reservData == nullptr)
  {
    LOG4CXX_WARN(logger, "ReservationData is NULL in PricingRequest object");
    return;
  }

  if (reservData->passengers().size() < 1)
  {
    LOG4CXX_WARN(logger, "No passenger in ReservationData object");
    return;
  }

  Traveler* traveler = reservData->passengers().back();

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B01 - FREQ Flyer Partner
    {
      FrequentFlyerAccount*& freqFlyerAcct = traveler->freqFlyerAccount();
      if (freqFlyerAcct == nullptr)
      {
        _pricingTrx->dataHandle().get(freqFlyerAcct);
        if (freqFlyerAcct == nullptr)
        {
          LOG4CXX_WARN(logger, "Error allocate FrequentFlyerAccount from DataHandle");
          return;
        }
      }
      freqFlyerAcct->partner().push_back(xmlValue.c_str());
    }
    break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveFreqFlyerPartnerInformation()
{
  LOG4CXX_DEBUG(logger, "In saveFreqFlyerPartnerInformation");
}

void
PricingModelMap::storeJPSInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeJPSInformation");

  PricingRequest* request = _pricingTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request object is NULL in Trx object");
  }

  if (_options == nullptr)
  {
    _pricingTrx->dataHandle().get(_options);
    _pricingTrx->setOptions(_options);
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));

    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // PBG - JPS-ON/JPS-OFF
      _options->jpsEntered() = xmlValue.c_str()[0];
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveJPSInformation()
{
  LOG4CXX_DEBUG(logger, "In saveJPSInformation");
}

void
PricingModelMap::storeExchangeItinInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeExchangeItinInformation");

  if (_multiExcTrx)
  {
    storeExcItinInfo(*_multiExcTrx, attrs);
    return;
  }

  _isExchangeItin = true;
  _intlItin = false;

  if (_pricingTrx != nullptr)
  {
    RexBaseTrx* rexBaseTrx = dynamic_cast<RexBaseTrx*>(_pricingTrx);
    if (rexBaseTrx)
      rexBaseTrx->setAnalyzingExcItin(true); // is originalTktDate, currentTkrDate valid
  }

  // Create exchange Itin
  createItin();
}

void
PricingModelMap::saveExchangeItinInformation()
{
  updatePrevTicketIssueAgent();

  LOG4CXX_DEBUG(logger, "In saveExchangeItinInformation");
  if (_insideSideTrip) // side trip end never set
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                 "SIDE TRIP DATA ERROR");

  if (_multiExcTrx)
  {
    saveItin();
    saveExcItinInfo(*_multiExcTrx);
    return;
  }

  if (_excItin)
  {
    saveItin();
    if (!_fareCalcCurrency.empty())
    {
      _excItin->calcCurrencyOverride() = _fareCalcCurrency;
      _excItin->calculationCurrency() = _fareCalcCurrency;
    }

    if (!fallback::azPlusUp(_pricingTrx) && !fallback::azPlusUpExc(_pricingTrx))
    {
      if (RexBaseRequest* request = dynamic_cast<RexBaseRequest*>(_pricingTrx->getRequest()))
      {
        ExcItin* itin = static_cast<RexBaseTrx*>(_pricingTrx)->exchangeItin().at(0);
        CommonParserUtils::initPricingDiscountForSegsWithNoDiscountIfReqHasDiscount(*request, itin);
      }
    }
  }

  // Resets
  _isExchangeItin = false;
  _intlItin = false;

  if (_pricingTrx != nullptr)
  {
    RexBaseTrx* rexBaseTrx = dynamic_cast<RexBaseTrx*>(_pricingTrx);
    if (rexBaseTrx)
    {
      std::vector<FareCompInfo*>::iterator i = _excItin->fareComponent().begin();
      for (; i != _excItin->fareComponent().end(); ++i)
      {
        std::vector<TravelSeg*>::iterator t = (**i).fareMarket()->travelSeg().begin();
        for (; t != (**i).fareMarket()->travelSeg().end(); ++t)
          (**t).fareBasisCode() = (**i).fareBasisCode().c_str();
      }

      rexBaseTrx->setAnalyzingExcItin(false); // is originalTktDate, currentTkrDate valid
    }

    if (_pricingTrx->excTrxType() == PricingTrx::PORT_EXC_TRX)
    {
      ExchangePricingTrx& excTrx = static_cast<ExchangePricingTrx&>(*_pricingTrx);
      excTrx.getRequest()->ticketingDT() = excTrx.purchaseDT();
      excTrx.dataHandle().setTicketDate(excTrx.purchaseDT());
    }


    PricingRequest* request = _pricingTrx->getRequest();
    if (request == nullptr)
      LOG4CXX_WARN(logger, "Request is NULL in Trx object");

    if (request)
    {
      std::vector<TaxCode> taxes;
      std::copy(_taxRequestedInfo.begin(), _taxRequestedInfo.end(),
          back_inserter(taxes));
      request->setTaxRequestedInfo(std::move(taxes));
      _taxRequestedInfo.clear();
    }
  }
}

void
PricingModelMap::checkRtwSegment(const TravelSeg& ts)
{
  if (_options && _options->isRtw())
  {
    if (TypeConvert::pssCharToBool(ts.forcedFareBrk()))
    {
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "SECONDARY ACTION CODE BF NOT VALID WITH RW ENTRY");
    }
    if (TypeConvert::pssCharToBool(ts.forcedSideTrip()))
    {
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "SECONDARY ACTION CODE ST NOT VALID WITH RW ENTRY");
    }
  }
}

void
PricingModelMap::storeExcItinInfo(MultiExchangeTrx& multiExcTrx, const xercesc::Attributes& attrs)
{
  reset();

  prepareExchangePricingTrx(AGENT_PRICING_MASK);
  ExchangePricingTrx* excTrx = getExchangePricingTrx();

  excTrx->transactionStartTime() = DateTime::localTime();

  excTrx->billing() = _multiExcTrx->newPricingTrx()->billing();
  excTrx->getRequest()->ticketingAgent() =
      _multiExcTrx->newPricingTrx()->getRequest()->ticketingAgent();
  excTrx->setParentTrx(&multiExcTrx);

  excTrx->setDynamicCfgOverriden(_multiExcTrx->newPricingTrx()->isDynamicCfgOverriden());
  excTrx->dynamicCfg() =
      DynamicConfigLoader::cloneOverridenConfig(_multiExcTrx->newPricingTrx()->dynamicCfg());
#ifdef CONFIG_HIERARCHY_REFACTOR
  excTrx->mutableConfigBundle() = _multiExcTrx->newPricingTrx()->configBundle().clone();
#endif

  if (multiExcTrx.excPricingTrx1() == nullptr)
  {
    excTrx->setExcTrxType(PricingTrx::EXC1_WITHIN_ME);
    multiExcTrx.excPricingTrx1() = excTrx;
    multiExcTrx.skipExcPricingTrx1() = false;
  }
  else
  {
    excTrx->setExcTrxType(PricingTrx::EXC2_WITHIN_ME);
    multiExcTrx.excPricingTrx2() = excTrx;
    multiExcTrx.skipExcPricingTrx2() = false;
  }
}

void
PricingModelMap::reset()
{
  _airSeg = nullptr;
  _arunkSeg = nullptr;
  _billing = nullptr;
  _itin = nullptr;
  _options = nullptr;
  _paxType = nullptr;
  _currentTvlSeg = nullptr;
  _intlItin = false;
  _excItin = nullptr;

  _pnrSegmentNumber = 0;
  _segmentNumber = 0;
  _discountPercentage = -1.0;
  _discountPercentageNew = 0.0;
  _discountGroupNum = 0;
  _discountAmount = -1.0;
  _discountAmountNew = 0.0;
  _discountCurrencyCode = "";
  _currentCRC = "";
  _forcedConx = 'F';
  _forcedStopOver = 'F';
  _forcedSideTrip = 'F';
  _forcedFareBreak = 'F';
  _forcedNoFareBreak = 'F';
  _cxrOverride = "";
  _globalDirectionOverride = "";

  _taxCodeOverrides.clear();
  _overrideTaxAmounts.clear();

  _taxRequestedInfo.clear();

  // Order passenger input
  _paxInputOrder = 0;

  _fareCalcCurrency = "";
  _isExchangeItin = false;
  _fareCompNum = 0;
  _surchargeOverride.clear();
  _stopoverOverride.clear();
  _plusUpOverride.clear();

  _airportTimes.clear();

  // for Consolidator Plus Up (Plus Up Pricing)
  _consolidatorPlusUpAmount = 0.0;
  _consolidatorPlusUpCurrency = "";
  _consolidatorPlusUpTktDesignator = "";
  _considerOnlyCabin = "";
  _legId = -1;
}

void
PricingModelMap::saveExcItinInfo(const MultiExchangeTrx& multiTrx)
{
  reset();

  _trx = multiTrx.newPricingTrx();
  _pricingTrx = multiTrx.newPricingTrx();

  _processExchangePricingTrx = true;
}

//--------------------------------------------------------------------------
// @function PricingModelMap::inputPaxTypeOrder
// Description: Convenience method for calculating order number for PaxType
//--------------------------------------------------------------------------
uint16_t
PricingModelMap::inputPaxTypeOrder(const std::string& inputEntry)
{
  std::basic_string<char>::size_type index, indexStartPSGR, indPaxT, indexEndPSGR, length;
  uint8_t mod = 4;
  uint16_t order = 99;
  bool psgrHasNN = false;
  length = inputEntry.length();
  div_t divresult;

  // Does Pax Type Code has NN ?
  indexEndPSGR = (_paxType->paxType()).find(PSGR_AnyNN);

  if (indexEndPSGR != std::string::npos) // Pax Type Code has NN
  {
    psgrHasNN = true;
  }

  // Find the start point where PaxType
  // is located in the input entry.
  // At first try the "$P", later "WPP"
  indexStartPSGR = inputEntry.find(DOLLAR_P);
  if (indexStartPSGR != std::string::npos)
  {
    // count size of everything before $P and plus $P
    index = indexStartPSGR + strlen(DOLLAR_P);
  }
  else
  {
    indexStartPSGR = inputEntry.find(WP_P);
    if (indexStartPSGR != std::string::npos)
    {
      // count size of everything before WPP and plus WPP
      index = indexStartPSGR + strlen(WP_P);
    }
  }

  // WPP or $P is found in the entry
  if (indexStartPSGR != std::string::npos)
  {
    if (psgrHasNN) //  PaxType has NN in the Code
    {
      // length of the input starts from the 1st PaxTypeCode
      length = length - index;

      // Find end of the PaxTypeCodes in the entry,
      // At first time looking for "$", later for the "+"

      indexEndPSGR = inputEntry.find("$", indexStartPSGR + 1);
      if (indexEndPSGR == std::string::npos)
      {
        // Try to find "+" if no "$" was found
        indexEndPSGR = inputEntry.find("+", indexStartPSGR + 1);

        if (indexEndPSGR == std::string::npos)
          return order;
      }
      // exact length of the all PaxTypeCodes in the entry
      length = indexEndPSGR - index;

      // The length of all PaxTypeCodes in the entry has been determined
      uint16_t allPaxLength = index + length;
      order = 0;
      for (int compResult; index < allPaxLength; index += 4, order++)
      {
        indexEndPSGR = inputEntry.find("/", index);
        if (indexEndPSGR != std::string::npos)
        {
          indPaxT = indexEndPSGR - index;
          if (indPaxT == 4)
          {
            index++;
          }
          else if (indPaxT == 5)
          {
            index += 2;
          }
        }

        compResult = inputEntry.compare(index, 1, _paxType->paxType(), 0, 1);

        if (!compResult)
        {
          std::string inputAge = inputEntry.substr(index + 1, 2);
          int age = atoi(inputAge.c_str());
          if (age && age == _paxType->age())
            return order;
        }
      }
      return order = 99;
    }
    else // Pax Type Code does not have NN, find just exact match
    {
      indPaxT = inputEntry.find(_paxType->paxType());
      if (indPaxT != std::string::npos)
      {
        order = indPaxT - index;
      }
      else
        return order = 99;
      if (order)

      {
        divresult = div(order, mod);
        //          if( divresult.rem)
        order = divresult.quot;
        //          else
        //            order = divresult.quot - 1;
      }
    }
  }
  return order;
}

void
PricingModelMap::storeAvailabilityInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeAvailabilityInformation");

  AvailData* availData = nullptr;
  _pricingTrx->dataHandle().get(availData);
  if (availData == nullptr)
  {
    LOG4CXX_WARN(logger, "Error allocate AvailData from DataHandle");
    return;
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q16 - Availability Set Order
      // No need to store it
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  if (_pricingTrx->getRequest() != nullptr)
  {
    _pricingTrx->getRequest()->availData().push_back(availData);
  }
}

void
PricingModelMap::saveAvailabilityInformation()
{
  LOG4CXX_DEBUG(logger, "In saveAvailabilityInformation");
}

void
PricingModelMap::storeFlightAvailInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeFlightAvailInformation");

  FlightAvail* flightAvail = nullptr;
  _pricingTrx->dataHandle().get(flightAvail);
  if (flightAvail == nullptr)
  {
    LOG4CXX_WARN(logger, "Error allocate FlightAvail from DataHandle");
    return;
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q1K - Flight Segment Order
      flightAvail->upSellPNRsegNum() = atoi(xmlValue.c_str());
      break;
    case 2: // Q17 - Flight Booking Codes and Num of Seats
      flightAvail->bookingCodeAndSeats() = xmlValue.c_str();
      break;
    case 3: // Q3F - Availability Method
      flightAvail->availMethod() = atoi(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  if (!_pricingTrx->getRequest()->availData().empty())
  {
    AvailData* availData = _pricingTrx->getRequest()->availData().back();
    if (availData != nullptr)
    {
      availData->flightAvails().push_back(flightAvail);
    }
  }
}

void
PricingModelMap::saveFlightAvailInformation()
{
  LOG4CXX_DEBUG(logger, "In saveFlightAvailInformation");
}

void
PricingModelMap::checkFlightContinuity(const LocCode& airport, const DateTime& time)
{
  if (_isExchangeItin) // Do not check for exchange itin
    return;

  std::map<LocCode, DateTime>::iterator iter = _airportTimes.find(airport);
  if (iter != _airportTimes.end())
  {
    if (time <= iter->second) // Going through the same airport at earlier or same time
    {
      throw ErrorResponseException(ErrorResponseException::FLIGHTS_CONTINUITY);
    }
    else
    {
      iter->second = time; // Set the latest time in the airport
    }
  }
  else
  {
    _airportTimes.insert(std::map<LocCode, DateTime>::value_type(airport, time));
  }
}

void
PricingModelMap::setBoardCity(AirSeg& airSeg)
{
  const std::vector<tse::MultiTransport*>& boardMACList = _trx->dataHandle().getMultiTransportCity(
      airSeg.origAirport(),
      airSeg.carrier(),
      (_intlItin ? GeoTravelType::International : airSeg.geoTravelType()),
      airSeg.departureDT());

  std::vector<tse::MultiTransport*>::const_iterator listIter = boardMACList.begin();

  if (listIter == boardMACList.end())
  {
    airSeg.boardMultiCity() = airSeg.origAirport();
    LOG4CXX_WARN(logger, "Board multi city vector is empty");
  }
  else
  {
    // If the flight is Domestic use the and the domAppl is set
    // use this multitranscity.  If the flight is International
    // and the intlAppl is set use this multitranscity
    airSeg.boardMultiCity() = (**listIter).multitranscity();
  }
  LOG4CXX_DEBUG(logger, "boardMultiCity=" << airSeg.boardMultiCity().c_str());
}

void
PricingModelMap::setOffCity(AirSeg& airSeg)
{
  const std::vector<tse::MultiTransport*>& offMACList = _trx->dataHandle().getMultiTransportCity(
      airSeg.destAirport(),
      airSeg.carrier(),
      (_intlItin ? GeoTravelType::International : airSeg.geoTravelType()),
      airSeg.departureDT());
  std::vector<tse::MultiTransport*>::const_iterator listIter = offMACList.begin();
  if (listIter == offMACList.end())
  {
    airSeg.offMultiCity() = airSeg.destAirport();
  }
  else
  {
    // Take the first, they should all be the same
    airSeg.offMultiCity() = (**listIter).multitranscity();
  }
  LOG4CXX_DEBUG(logger, "offMultiCity=" << airSeg.offMultiCity());
}

void
PricingModelMap::setIntlBoardOffCity(Itin& itin)
{
  bool foundIntlSeg = false;

  std::vector<TravelSeg*>::iterator iter = itin.travelSeg().begin();

  for (; iter != itin.travelSeg().end(); ++iter)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iter);
    if (airSeg == nullptr)
      continue;

    if (airSeg->geoTravelType() == GeoTravelType::International) // All board/off cities after first
                                                                 // Intl air seg has been set
                                                                 // already
      foundIntlSeg = true;

    if ((airSeg->segmentType() ==
         Open) || // Open segment without date may not have board/off cities set correctly
        (!foundIntlSeg &&
         (airSeg->geoTravelType() != GeoTravelType::International))) // Need reset board/off
    // cities use IntlAppl for
    // Domestic Seg
    {
      setBoardCity(*airSeg);
      setOffCity(*airSeg);
    }
  }
}

class IsArunkInSameCity
{
public:
  IsArunkInSameCity() {}
  bool operator()(TravelSeg* tvlSeg) const
  {
    if ((tvlSeg->segmentType() == Arunk) && (tvlSeg->boardMultiCity() == tvlSeg->offMultiCity()))
      return true;

    return false;
  }
};

void
PricingModelMap::removeArunkInSameCity(std::vector<TravelSeg*>& tvlSegs)
{
  tvlSegs.erase(remove_if(tvlSegs.begin(), tvlSegs.end(), IsArunkInSameCity()), tvlSegs.end());
}

void
PricingModelMap::storeAccompanyPassengerInformation(const xercesc::Attributes& attrs)
{
  // only process for new Itin
  if (_isExchangeItin)
    return;

  storePassengerInformation(attrs);
}

void
PricingModelMap::saveAccompanyPassengerInformation()
{
  // only process for new Itin
  if (_isExchangeItin)
    return;

  BaseExchangeTrx* bExcTrx = dynamic_cast<BaseExchangeTrx*>(_pricingTrx);
  if (!bExcTrx)
    return;

  PaxTypeUtil::initialize(*_pricingTrx,
                          *_paxType,
                          _paxType->paxType(),
                          _paxType->number(),
                          _paxType->age(),
                          _paxType->stateCode(),
                          0);

  bExcTrx->accompanyPaxType().push_back(_paxType);
  _paxType = nullptr;
}

void
PricingModelMap::parsePlusUpInformation(const xercesc::Attributes& attrs,
                                        MinimumFareOverride& minFareOverride,
                                        std::string& moduleStr)
{
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // C6L - Plus Up Amount
      minFareOverride.plusUpAmount = strtod(xmlValue.c_str(), nullptr);
      break;
    case 2: // A11 - Plus Up Origin City
      minFareOverride.boardPoint = xmlValue.c_str();
      break;
    case 3: // A12 - Plus Up Destination City
      minFareOverride.offPoint = xmlValue.c_str();
      break;
    case 4: // A13 - Plus Up Fare Origin City
      minFareOverride.fareBoardPoint = xmlValue.c_str();
      break;
    case 5: // A14 - Plus Up Fare Destination City
      minFareOverride.fareOffPoint = xmlValue.c_str();
      break;
    case 6: // A18 - Plus Up Via City
      minFareOverride.constructPoint = xmlValue.c_str();
      break;
    case 7: // S68 - Plus Up Message
      moduleStr = xmlValue.c_str();
      break;
    case 8: // A40 - Plus Up Country of Payment
      break;
    case 9: // A19 - Plus Up Second Via City
      minFareOverride.constructPoint2 = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

int16_t
PricingModelMap::parseLegId(const std::string& value) const
{
  int16_t legId;
  if (!TypeConvert::stringToValue(value, legId) || (legId <= 0))
  {
    std::ostringstream os;
    os << "INVALID LEG ID: " << value;
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, os.str().c_str());
  }
  return legId - 1;
}

void
PricingModelMap::storeNonCat31PlusUpInformation(MinimumFareOverride& minFareOverride,
                                                std::string& moduleStr)
{
  MinimumFareModule moduleName;

  if (moduleStr.empty())
    moduleName = HIP;
  else if (moduleStr == "P")
    moduleName = CTM;
  else if (moduleStr == "H")
    moduleName = OSC;
  else if (moduleStr == "U")
    moduleName = RSC;
  else if (moduleStr == "P R/")
    moduleName = CPM;
  else
    return;

  if (!setPlusUpType(moduleName, &minFareOverride))
    return;
  if (!storePlusUpOverride(moduleName, &minFareOverride))
    return;
}

void
PricingModelMap::storeRexPlusUpInformation(MinimumFareOverride* minFareOverride)
{
  if (_currentTvlSeg) // Plus up for Fare Component
  {
    if (_excItin && !_excItin->fareComponent().empty())
    {
      _excItin->fareComponent().back()->hip() = minFareOverride;
    }
  }
  else
  {
    // Need Implement later
  }
}

void
PricingModelMap::storePlusUpInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePlusUpInformationconst");

  std::string moduleStr;
  if (_pricingTrx->excTrxType() == PricingTrx::PORT_EXC_TRX)
  {
    if (_isExchangeItin)
      return;
    MinimumFareOverride portMinFareOverride;
    parsePlusUpInformation(attrs, portMinFareOverride, moduleStr);
    storeNonCat31PlusUpInformation(portMinFareOverride, moduleStr);
  }
  else if (_pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX ||
           _pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX)
  {
    MinimumFareOverride* minFareOverride = nullptr;
    _pricingTrx->dataHandle().get(minFareOverride);
    if (minFareOverride == nullptr)
      return;
    parsePlusUpInformation(attrs, *minFareOverride, moduleStr);
    storeRexPlusUpInformation(minFareOverride);
    if (shouldStoreOverrides() &&
        (!_isExchangeItin || _pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX))
      storeNonCat31PlusUpInformation(*minFareOverride, moduleStr);
  }
}

bool
PricingModelMap::setPlusUpType(MinimumFareModule& moduleName,
                               const MinimumFareOverride* minFareOverride)
{
  switch (moduleName)
  {
  case CTM:
  {
    if (!minFareOverride->boardPoint.empty() && !minFareOverride->offPoint.empty() &&
        !minFareOverride->fareBoardPoint.empty() && !minFareOverride->fareOffPoint.empty())
    {
      moduleName = BHC; // it is BHC
    }
    break;
  }
  case HIP:
  {
    if (minFareOverride->plusUpAmount != 0) // HIP override must be zero amout
    {
      return false;
    }
    break;
  }
  case COM:
  case DMC:
  case COP:
  case OSC:
  case RSC:
  case CPM:
  case OJM:
  case NCJ:
    break;
  default:
    break;
  }
  return true;
}

bool
PricingModelMap::storePlusUpOverride(MinimumFareModule& moduleName,
                                     MinimumFareOverride* minFareOverride)
{
  if (!shouldStoreOverrides())
    return false;
  BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_pricingTrx);

  PlusUpOverride* plusUpOverride = nullptr;
  excTrx->dataHandle().get(plusUpOverride);
  if (plusUpOverride == nullptr)
    return false;
  plusUpOverride->moduleName() = moduleName;
  if (moduleName == RSC)
  {
    FarePath::RscPlusUp* rscPlusUp = nullptr;
    excTrx->dataHandle().get(rscPlusUp);
    if (rscPlusUp == nullptr)
      return false;
    rscPlusUp->copyMinFarePlusUpItem(minFareOverride);
    rscPlusUp->inboundBoardPoint = minFareOverride->fareBoardPoint;
    rscPlusUp->inboundOffPoint = minFareOverride->fareOffPoint;
    rscPlusUp->constructPoint2 = minFareOverride->constructPoint2;
    plusUpOverride->plusUpItem() = rscPlusUp; // add RSC verride item.
  }
  else if (moduleName == OSC)
  {
    FarePath::OscPlusUp* oscPlusUp = nullptr;
    excTrx->dataHandle().get(oscPlusUp);
    if (oscPlusUp == nullptr)
      return false;
    oscPlusUp->copyMinFarePlusUpItem(minFareOverride);
    plusUpOverride->plusUpItem() = oscPlusUp; // add OSC override item.
  }
  else
  {
    BhcPlusUpItem* bhcPlusUp = nullptr;
    excTrx->dataHandle().get(bhcPlusUp);
    if (bhcPlusUp == nullptr)
      return false;
    bhcPlusUp->copyMinFarePlusUpItem(minFareOverride);
    bhcPlusUp->fareBoardPoint = minFareOverride->fareBoardPoint;
    bhcPlusUp->fareOffPoint = minFareOverride->fareOffPoint;
    plusUpOverride->plusUpItem() = bhcPlusUp; // add other minfare override item.
  }

  if (_isExchangeItin)
    plusUpOverride->fromExchange() = true;

  if (_fareCompNum == 0)
  {
    // plusup override on journey
    excTrx->exchangeOverrides().plusUpOverride().push_back(plusUpOverride);
  }
  else
  {
    _plusUpOverride.push_back(plusUpOverride);
    // need to set _currentTvlSeg point when it is created
  }
  return true;
}

void
PricingModelMap::savePlusUpInformation()
{
  LOG4CXX_DEBUG(logger, "In savePlusUpInformation");
}

void
PricingModelMap::storeDifferentialInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeDifferentialInformation");

  if (_isExchangeItin && _pricingTrx->excTrxType() != PricingTrx::AF_EXC_TRX)
  {
    return;
  }

  if (!shouldStoreOverrides())
    return;
  BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_pricingTrx);

  DifferentialOverride* diffOverride = nullptr;
  excTrx->dataHandle().get(diffOverride);
  if (diffOverride == nullptr)
    return;

  excTrx->exchangeOverrides().differentialOverride().push_back(diffOverride);

  if (_isExchangeItin)
    diffOverride->fromExchange() = true;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // C50 - Differential Amount
      diffOverride->amount() = strtod(xmlValue.c_str(), nullptr);
      break;
    case 2: // A13 - High differential origin city/airport
      diffOverride->highDiffFareOrigin() = xmlValue.c_str();
      break;
    case 3: // A14 - High differential Destination city/airport
      diffOverride->highDiffFareDestination() = xmlValue.c_str();
      break;
    case 4: // A01 - Low HIP on differential origin city/airport
      diffOverride->lowHipOrigin() = xmlValue.c_str();
      break;
    case 5: // A02 - Low HIP on differential destination city/airport
      diffOverride->lowHipDestination() = xmlValue.c_str();
      break;
    case 6: // A03 - High HIP on differential Origin City/airport
      diffOverride->highHipOrigin() = xmlValue.c_str();
      break;
    case 7: // A04 - High HIP on differential Destination City/sirport
      diffOverride->highHipDestination() = xmlValue.c_str();
      break;
    case 8: // B30 - Fare Class Low
      diffOverride->lowFareClass() =
          fallback::purgeBookingCodeOfNonAlpha(_trx)
              ? xmlValue.c_str()
              : DataModelMap::purgeBookingCodeOfNonAlpha(xmlValue.c_str());
      break;
    case 9: // BJ0 - Fare Class High
      diffOverride->highFareClass() = xmlValue.c_str();
      break;
    case 10: // N00 - Cabin Fare Low
      diffOverride->lowFareCabin() = (xmlValue.c_str())[0];
      break;
    case 11: // N04 - Cabin Fare High
      diffOverride->highFareCabin() = (xmlValue.c_str())[0];
      break;
    case 12: // A18 - HIP Constructed City
      diffOverride->hipConstructedCity() = xmlValue.c_str();
      break;
    case 13: // Q48 - Mileage on Differential
      diffOverride->mileageOnDiff() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingModelMap::saveDifferentialInformation()
{
  LOG4CXX_DEBUG(logger, "In saveDifferentialInformation");
}

void
PricingModelMap::storeSurchargeInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeSurchargeInformation");
  if (!shouldStoreOverrides())
    return;
  BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_pricingTrx);

  if (_isExchangeItin)
  {
    if (exchangeTypeNotCE(excTrx) && excTrx->excTrxType() != PricingTrx::AF_EXC_TRX)
      return;
  }

  Indicator surchargeOverrideType = 'Q';
  MoneyAmount surchargeOverrideAmt = 0.0;
  CurrencyCode surchargeOverrideCur;
  LocCode fcBrdCity;
  LocCode fcOffCity;
  Indicator surchargeSurchargeType = 'S';

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // N0F - Surcharge Override type
      surchargeOverrideType = (xmlValue.c_str())[0];
      break;
    case 2: // C69 - Surcharge Override amount
      surchargeOverrideAmt = strtod(xmlValue.c_str(), nullptr);
      break;
    case 3: // C46 - Surcharge Override currency
      surchargeOverrideCur = xmlValue.c_str();
      break;
    case 4: // A11 - SurchargeDepartureCity
      fcBrdCity = xmlValue.c_str();
      break;
    case 5: // A12 - SurchargeArrivalCity
      fcOffCity = xmlValue.c_str();
      break;
    case 6: // N28 - SurchargeSurchargeType
      surchargeSurchargeType = xmlValue.c_str()[0];
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  if (surchargeOverrideAmt == 0.0)
    return;

  SurchargeOverride* surOverride = nullptr;
  excTrx->dataHandle().get(surOverride);
  if (surOverride == nullptr)
    return;

  if (_isExchangeItin)
    surOverride->fromExchange() = true;

  surOverride->type() = surchargeOverrideType;
  surOverride->amount() = surchargeOverrideAmt;
  surOverride->currency() = surchargeOverrideCur;

  if (surchargeSurchargeType != 'S')
    surOverride->singleSector() = false;

  surOverride->fcBrdCity() = fcBrdCity;
  surOverride->fcOffCity() = fcOffCity;

  if (surchargeSurchargeType == 'J')
    surOverride->fcFpLevel() = true;

  _surchargeOverride.push_back(surOverride);
}

void
PricingModelMap::saveSurchargeInformation()
{
  LOG4CXX_DEBUG(logger, "saveSurchargeInformation");
}

void
PricingModelMap::storeMileageInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeMileageInformation");

  if (!_isExchangeItin && _fareCompNum > 0 && !shouldStoreOverrides())
  {
    LOG4CXX_ERROR(logger, "In storeMileageInformation - MIL tag is inside wrong tag");
    return;
  }

  int mileageSurchargePctg = 0;
  Indicator mileageSurchargeType = ' ';
  LocCode mileageSurchargeCity = "";

  uint16_t numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // AP2 - Mileage Display Type
      mileageSurchargeType = (xmlValue.c_str())[0];
      break;
    case 2: // AP3 - Mileage Display City
      mileageSurchargeCity = (xmlValue.c_str());
      break;
    case 3: // Q48 - Mileage Surcharge Percentage
      mileageSurchargePctg = atoi(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  if (mileageSurchargeType == 'T')
  {
    _mileageTktCity = mileageSurchargeCity;
  }
  else if (mileageSurchargeType == 'E')
  {
    _mileageSurchargeCity = mileageSurchargeCity;
  }
  else if (mileageSurchargeType == 'B')
  {
    saveMileDataOverideInformation(mileageSurchargeType, mileageSurchargeCity);
  }
  else if (mileageSurchargeType == 'M')
  {
    _mileageSurchargePctg = mileageSurchargePctg; // Exchange trx
  }

  if (_excItin && !_excItin->fareComponent().empty())
  {
    FareCompInfo* curFc = findFareCompInfo(_excItin->fareComponent(), _fareCompNum);
    if (curFc != nullptr)
    {
      if (mileageSurchargePctg > 0)
        curFc->mileageSurchargePctg() = mileageSurchargePctg;

      if (!mileageSurchargeCity.empty())
        curFc->mileageSurchargeCity() = mileageSurchargeCity;
    }
  }
}

void
PricingModelMap::saveMileageInformation()
{
  LOG4CXX_DEBUG(logger, "In saveMileageInformation");
}

void
PricingModelMap::saveMileDataOverideInformation(Indicator mileageSurchargeType,
                                                LocCode& mileageSurchargeCity)
{
  MileageTypeData* mtdOverride = nullptr;
  _pricingTrx->dataHandle().get(mtdOverride);
  if (mtdOverride == nullptr)
    return;
  mtdOverride->city() = mileageSurchargeCity;

  if (mileageSurchargeType == 'B')
  {
    mtdOverride->type() = MileageTypeData::EQUALIZATION;

    _mileDataOverride.push_back(mtdOverride);
  }
}

void
PricingModelMap::storeStopoverInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeStopoverInformation");

  if (!shouldStoreOverrides())
    return;
  BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_pricingTrx);

  int stopOverCnt = 0;
  MoneyAmount stopOverAmt = 0.0;

  uint16_t numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q0X - Stopover Count
      stopOverCnt = atoi(xmlValue.c_str());
      break;
    case 2: // C63 - Stopover Amount
      stopOverAmt = strtod(xmlValue.c_str(), nullptr);
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  if (_isExchangeItin)
  {
    if (!_excItin)
    {
      LOG4CXX_ERROR(logger, "In storeStopoverInformation - no ExcItin");
      return;
    }
    if (_pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX)
    {
      StopoverOverride* soOverride = nullptr;
      excTrx->dataHandle().get(soOverride);
      if (soOverride == nullptr)
        return;

      soOverride->count() = stopOverCnt;
      soOverride->amount() = stopOverAmt;
      soOverride->fromExchange() = true;

      if (_fareCompNum == 0)
        excTrx->exchangeOverrides().stopoverOverride().push_back(soOverride);
      else
        _stopoverOverride.push_back(soOverride);
    }

    // this is only used if transaction is CE, and we only copy stopover
    // from segment
    if (exchangeTypeNotCE(excTrx) || _segmentNumber == 0)
      return;

    FareCompInfo* curFc = findFareCompInfo(_excItin->fareComponent(), _fareCompNum);

    if (curFc == nullptr)
    {
      LOG4CXX_ERROR(logger, "In storeStopoverInformation - cannot find FareCompInfo");
      return;
    }

    curFc->stopoverSurcharges().insert(
        std::pair<uint16_t, MoneyAmount>(_segmentNumber, stopOverAmt));
  }
  else
  {
    StopoverOverride* soOverride = nullptr;
    excTrx->dataHandle().get(soOverride);
    if (soOverride == nullptr)
      return;

    soOverride->count() = stopOverCnt;
    soOverride->amount() = stopOverAmt;
    if (_fareCompNum == 0)
    {
      // stopover surcharge override on journey
      excTrx->exchangeOverrides().stopoverOverride().push_back(soOverride);
      excTrx->exchangeOverrides().journeySTOOverrideCnt() += stopOverCnt;
    }
    else
      _stopoverOverride.push_back(soOverride);
    // need to set _currentTvlSeg point when it is created
  }
}

void
PricingModelMap::saveStopoverInformation()
{
  LOG4CXX_DEBUG(logger, "In saveStopoverInformation");
}

RexBaseTrx::RequestTypes
PricingModelMap::getRefundTypes(const xercesc::Attributes& attrs)
{
  std::string exchangeRequestType;
  std::string secondaryExchangeRequestType;
  std::string primaryProcessType;
  std::string secondaryProcessType;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S96 - Exchange Request Type
      exchangeRequestType = xmlValue.c_str();
      break;

    case 2: // SA8 - Secondary Exchange Request Type
      secondaryExchangeRequestType = xmlValue.c_str();
      break;

    case 3: // N25 - Process Type Indicator for Primary Request Type

      primaryProcessType = (xmlValue.c_str())[0];
      break;

    case 4: // N26 - Process Type Indicator for Secondary Request Type

      secondaryProcessType = (xmlValue.c_str())[0];
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  if (exchangeRequestType == AUTOMATED_REISSUE && primaryProcessType.empty())
    throw ErrorResponseException(ErrorResponseException::BASE_CAT31_NO_LONGER_SUPPORTED);

  if (exchangeRequestType == AUTOMATED_REISSUE || exchangeRequestType == FULL_EXCHANGE ||
      exchangeRequestType == PARTIAL_EXCHANGE)
  {
    _primaryProcessType = (primaryProcessType.c_str())[0];
  }
  if (secondaryExchangeRequestType == FULL_EXCHANGE ||
      secondaryExchangeRequestType == PARTIAL_EXCHANGE)
  {
    _secondaryProcessType = (secondaryProcessType.c_str())[0];
  }
  return std::make_pair(exchangeRequestType, secondaryExchangeRequestType);
}

void
PricingModelMap::prepareExchangePricingTrx(
    const std::string& refundType) // is _pricingTrx->ticketingDate() valid.
{
  ExchangePricingTrx* exchangePricingTrx = nullptr;
  _dataHandle.get(exchangePricingTrx);

  if (exchangePricingTrx)
  {
    exchangePricingTrx->reqType() = refundType;
    _trx = exchangePricingTrx;
    _pricingTrx = exchangePricingTrx;

    PricingRequest* request = nullptr;

    _pricingTrx->dataHandle().get(request);
    if (request)
    {
      // request->setTrx(rexPricingTrx);
      _pricingTrx->setRequest(request);
    }
    _processExchangePricingTrx = true;
  }

  return;
}

FareCompInfo*
PricingModelMap::findFareCompInfo(const std::vector<FareCompInfo*>& itinFc, uint16_t fareCompNum)
    const
{
  std::vector<FareCompInfo*>::const_reverse_iterator rIter = itinFc.rbegin();

  for (; rIter != itinFc.rend(); ++rIter)
  {
    if ((**rIter).fareCompNumber() == fareCompNum)
      return (*rIter);
  }
  return nullptr;
}

void
PricingModelMap::prepareMultiExchangePricingTrx()
{
  MultiExchangeTrx* multiExcTrx = nullptr;
  _dataHandle.get(multiExcTrx);

  if (multiExcTrx)
  {
    _multiExcTrx = multiExcTrx;

    prepareExchangePricingTrx(AGENT_PRICING_MASK);

    if (_pricingTrx)
    {
      _pricingTrx->transactionStartTime() = DateTime::localTime();
      _pricingTrx->setExcTrxType(PricingTrx::NEW_WITHIN_ME);
      _pricingTrx->setParentTrx(multiExcTrx);
      multiExcTrx->newPricingTrx() = _pricingTrx;
      multiExcTrx->skipNewPricingTrx() = false;
    }
  }

  return;
}

void
PricingModelMap::prepareRexPricingTrx(
    const RexBaseTrx::RequestTypes& refundTypes) // is _pricingTrx->ticketingDate() valid.
{
  RexPricingTrx* rexPricingTrx = nullptr;
  _dataHandle.get(rexPricingTrx);

  if (rexPricingTrx)
    prepareRexBaseTrx(*rexPricingTrx, refundTypes);
}

void
PricingModelMap::prepareRefundPricingTrx(const RexBaseTrx::RequestTypes& refundTypes)
{
  RefundPricingTrx* refundPricingTrx = nullptr;
  _dataHandle.get(refundPricingTrx);

  if (refundPricingTrx)
    prepareRexBaseTrx(*refundPricingTrx, refundTypes);
}

void
PricingModelMap::prepareRexBaseTrx(RexBaseTrx& rexBaseTrx,
                                   const RexBaseTrx::RequestTypes& refundTypes)
{
  rexBaseTrx.set(refundTypes);
  rexBaseTrx.prepareRequest();
  _options = rexBaseTrx.prepareOptions();
  _trx = &rexBaseTrx;
  _pricingTrx = &rexBaseTrx;
}

void
PricingModelMap::adjustFlownSegArrivalTime(Itin* itin, TravelSeg* tvlSeg, int i)
{
  ExchangePricingTrx* excTrx = getExchangePricingTrx();
  RexBaseTrx* rexTrx = getRexBaseTrx();

  if (excTrx == nullptr && rexTrx == nullptr && _multiExcTrx == nullptr)
    return;

  int tvlSegSize = itin->travelSeg().size();
  if (tvlSegSize <= 1)
    return;
  if (i <= 0)
    return;
  if (tvlSeg->segmentType() == Arunk)
    return;

  TravelSeg* prevSeg = itin->travelSeg()[i - 1];
  if (prevSeg->segmentType() == Arunk)
  {
    prevSeg = nullptr;
    if (i > 1 && itin->travelSeg()[i - 2]->segmentType() != Arunk)
      prevSeg = itin->travelSeg()[i - 2];
  }

  if (prevSeg == nullptr)
    return;

  if (prevSeg->unflown())
    return;

  if (!(prevSeg->arrivalDT().hours() == 0 && prevSeg->arrivalDT().minutes() == 0))
    return;

  DateTime newDate(prevSeg->arrivalDT().year(),
                   prevSeg->arrivalDT().month(),
                   prevSeg->arrivalDT().day(),
                   prevSeg->arrivalDT().hours(),
                   prevSeg->arrivalDT().minutes(),
                   prevSeg->arrivalDT().seconds());
  if (tvlSeg->departureDT().hours() == 0 && tvlSeg->departureDT().minutes() == 0)
    newDate.subtractDays(1);

  DateTime newArrivalDate(newDate.year(),
                          newDate.month(),
                          newDate.day(),
                          tvlSeg->departureDT().hours(),
                          tvlSeg->departureDT().minutes() - 1,
                          newDate.seconds());

  prevSeg->arrivalDT() = newArrivalDate;
}

void
PricingModelMap::checkTrxRequiredInfo()
{
  if (_pricingTrx == nullptr)
    return;

  if (_pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    RexPricingTrx& rexTrx = static_cast<RexPricingTrx&>(*_pricingTrx);
    checkRexPricingTrxRequiredInfo(rexTrx);
  }
  else if (_pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX)
  {
    RexBaseTrx& rexBaseTrx = static_cast<RexBaseTrx&>(*_pricingTrx);
    checkRexBaseTrxRequiredInfo(rexBaseTrx);
  }
  else
  {
    checkRegularPricingTrxReqInfo(*_pricingTrx);
  }
}

void
PricingModelMap::checkRegularPricingTrxReqInfo(PricingTrx& pricingTrx)
{
  std::string errMsg;

  if (pricingTrx.ticketingDate().isEmptyDate())
    errMsg = "MISSING TICKET DATE";
  else if (pricingTrx.itin().empty() || pricingTrx.itin().front()->travelSeg().empty())
    errMsg = "MISSING ITINERARY";
  else
    errMsg = checkCommonTrxReqInfo(pricingTrx);

  if (!errMsg.empty())
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED, errMsg.c_str());

  if (!_isRexTrx)
    _reqXmlValidator.validateShipRegistry(_pricingTrx);

  if (pricingTrx.awardRequest())
  {
    if (pricingTrx.getRequest()->isLowFareNoAvailability() // WPNCS
        ||
        pricingTrx.getRequest()->isLowFareRequested() // WPNC/WPNCB
        )
    {
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                   "$FORMAT–WPNC WPNCS WPNCB WITH $AWARD");
    }
    if (!pricingTrx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
    {
      // not request from airline
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                   "$FORMAT-CHECK ENTRY COMMENCING WITH $AWARD");
    }
    if (PaxTypeUtil::hasNotAwardPaxType(pricingTrx.paxType()))
    {
      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                   "$FORMAT$ -PSGR TYPES");
    }
  }
}

std::string
PricingModelMap::checkCommonTrxReqInfo(PricingTrx& pricingTrx)
{
  if (pricingTrx.getRequest() == nullptr || pricingTrx.getRequest()->ticketingAgent() == nullptr)
    return std::string("MISSING AGENT INFORMATION");
  if (pricingTrx.billing() == nullptr)
    return std::string("MISSING BILLING INFORMATION");
  if (pricingTrx.getOptions() == nullptr)
    return std::string("MISSING OPTIONS");
  if (pricingTrx.paxType().empty())
    return std::string("MISSING PASSENGER INFORMATION");
  return std::string();
}

void
PricingModelMap::checkRexBaseTrxRequiredInfo(RexBaseTrx& rexTrx)
{
  std::string errMsg;

  if (rexTrx.originalTktIssueDT().isEmptyDate())
    errMsg = "MISSING ORIGINAL TICKET DATE";
  else if (rexTrx.currentTicketingDT().isEmptyDate())
    errMsg = "MISSING CURRENT TICKET DATE";
  else if (rexTrx.paxType().size() > 1)
    errMsg = "MULTIPLE PASSENGER TYPES NOT SUPPORTED";
  else if (rexTrx.exchangeItin().empty() || rexTrx.exchangeItin().front()->travelSeg().empty())
  {
    if (rexTrx.reqType() != TX_TAX_INFO_REQUEST)
      errMsg = "MISSING EXCHANGE ITINERARY";
  }
  else if (rexTrx.exchangeItin().front()->fareComponent().empty())
    errMsg = "MISSING EXCHANGE ITINERARY FARE COMPONENT INFORMATION";
  else
    errMsg = checkCommonTrxReqInfo(rexTrx);

  if (!errMsg.empty())
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED, errMsg.c_str());
}

void
PricingModelMap::checkRexPricingTrxRequiredInfo(RexPricingTrx& rexTrx)
{
  checkRexBaseTrxRequiredInfo(rexTrx);

  std::string errMsg;

  if (rexTrx.newItin().empty() || rexTrx.newItin().front()->travelSeg().empty())
    if (rexTrx.reqType() != TX_TAX_INFO_REQUEST)
      errMsg = "MISSING NEW ITINERARY";

  if (!errMsg.empty())
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED, errMsg.c_str());
}

bool
PricingModelMap::hasFullVCTR() const
{
  return _hasVCTR.isAllSet(VCTR_AllSet);
}

bool
PricingModelMap::shouldStoreOverrides()
{
  if (_pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX &&
      !((static_cast<RexPricingTrx&>(*_pricingTrx)).secondaryExcReqType().empty()))
    return true;

  if (_pricingTrx->excTrxType() == PricingTrx::PORT_EXC_TRX ||
      _pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX)
    return true;

  if (_multiExcTrx)
    return true;

  return false;
}

bool
PricingModelMap::exchangeTypeNotCE(BaseExchangeTrx* baseExcTrx)
{
  if (baseExcTrx->excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    if ((static_cast<RexPricingTrx&>(*_pricingTrx)).secondaryExcReqType() != TAG_10_EXCHANGE)
      return true;
  }

  if (baseExcTrx->excTrxType() == PricingTrx::PORT_EXC_TRX)
  {
    if (baseExcTrx->reqType() != TAG_10_EXCHANGE)
      return true;
  }
  return false;
}
bool
PricingModelMap::isFreqFlyerStatusCorrect(int ffStatus) const
{
  return (ffStatus > 0 && ffStatus < 10);
}

void
PricingModelMap::storeFreqFlyerStatus(const xercesc::Attributes& attrs)
{
  PaxTypeCode ptc;
  int ffStatus = 0;
  CarrierCode cxr = "";

  const int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q7D - Status
      ffStatus = atoi(xmlValue.c_str());
      if (!isFreqFlyerStatusCorrect(ffStatus))
        return;
      break;
    case 2: // B70 - Pax Type Code
      ptc = xmlValue.c_str();
      break;
    case 3: // B00 - Carrier code
      cxr = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  PaxType::FreqFlyerTierWithCarrier* ffd =
      _pricingTrx->dataHandle().create<PaxType::FreqFlyerTierWithCarrier>();
  if (ffd == nullptr)
  {
    LOG4CXX_ERROR(logger, "Reservation is NULL in FreqFlyerStatusData object");
    return;
  }

  ffd->setFreqFlyerTierLevel(ffStatus);
  ffd->setCxr(cxr);
  _ffData.insert(std::pair<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>(ptc, ffd));
}
void
PricingModelMap::setFreqFlyerStatus()
{
  std::vector<PaxType*>::iterator ipt = _pricingTrx->paxType().begin();
  std::vector<PaxType*>::iterator ipe = _pricingTrx->paxType().end();
  std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>::iterator it;
  std::pair<std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>::iterator,
            std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>::iterator> ret,
      retBlank;

  retBlank = _ffData.equal_range(PaxTypeCode(""));
  for (; ipt != ipe; ipt++)
  {
    ret = _ffData.equal_range((*ipt)->paxType());
    for (it = ret.first; it != ret.second; it++)
      (*ipt)->freqFlyerTierWithCarrier().push_back((*it).second);
    for (it = retBlank.first; it != retBlank.second; it++)
      (*ipt)->freqFlyerTierWithCarrier().push_back((*it).second);
  }
}

void
PricingModelMap::storeRequestedGroupCodes(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeRequestedGroupCodes");
  if (!_pricingTrx->getRequest()->collectOCFees())
  {
    LOG4CXX_ERROR(logger, "OC Fee is not requested");
    return;
  }
  PricingOptions* options = _pricingTrx->getOptions();
  RequestedOcFeeGroup requestedOcFeeGroup;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S01 - Group code
      requestedOcFeeGroup.groupCode() = xmlValue.c_str();
      break;
    case 2: // Q0A - Number of Groups
      requestedOcFeeGroup.numberOfItems() = atoi(xmlValue.c_str());
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
  options->serviceGroupsVec().push_back(requestedOcFeeGroup);
}

void
PricingModelMap::checkCat35NetSellOption(PricingTrx* trx)
{
  PricingOptions* opt = trx->getOptions();
  if (opt)
  {
    if (trx->getRequest() && trx->getRequest()->isTicketEntry())
    {
      if (opt->cat35NetSell())
      {
        opt->cat35Net() = 0;
        opt->cat35Sell() = 0;
      }
    }
    else if (opt->cat35NetSell())
      opt->cat35NetSell() = false;
  }
}

void
PricingModelMap::storePocInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePocInformation");

  PricingRequest* request = _pricingTrx->getRequest();

  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  ReservationData*& reservData = request->reservationData();

  if (reservData == nullptr)
  {
    LOG4CXX_WARN(logger, "ReservationData is NULL in PricingRequest object");
    return;
  }
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A03 - City
      reservData->pocAirport() = xmlValue.c_str();
      break;
    case 2: // D01 - Date
      reservData->pocDepartureDate() = convertDate(xmlValue.c_str());
      break;
    case 3: // D02 - Time
      reservData->pocDepartureTime() = xmlValue.c_str();

      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}
void
PricingModelMap::savePocInformation()
{
  LOG4CXX_DEBUG(logger, "In savePocInformation");
}

void
PricingModelMap::storeDynamicConfigOverride(const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}

void
PricingModelMap::tryExtractVersion(const XMLChString& attribName, const XMLChString& attribValue)
{
  std::string name = attribName.c_str();
  std::transform(name.begin(), name.end(), name.begin(), (int (*)(int))std::toupper);

  if (boost::iequals(name.c_str(), "VERSION"))
  {
    std::string fullVersion = attribValue.c_str();

    uint16_t majorSchemaVersion = 1;
    uint16_t minorSchemaVersion = 0;
    uint16_t revisionSchemeVersion = 0;
    // minimum size 3 i.e. "1.1"
    if (fullVersion.size() > 2)
    {
      try
      {
        majorSchemaVersion = boost::lexical_cast<uint16_t>(fullVersion[0]);
        minorSchemaVersion = boost::lexical_cast<uint16_t>(fullVersion[2]);
        _pricingTrx->getRequest()->majorSchemaVersion() = majorSchemaVersion;
        _pricingTrx->getRequest()->minorSchemaVersion() = minorSchemaVersion;

        if (attribValue.size() > 4)
        {
          revisionSchemeVersion = boost::lexical_cast<uint16_t>(fullVersion[4]);
          _pricingTrx->getRequest()->revisionSchemaVersion() = revisionSchemeVersion;
        }
      }
      catch (boost::bad_lexical_cast&)
      {
      }
    }
  }
}

void
PricingModelMap::parseRetailerCode(const XMLChString& xmlValue,
                                      PricingRequest* request, uint8_t& rcqCount)
{
  if (!fallback::fallbackFRRProcessingRetailerCode(_trx) && request != nullptr)
    rcqCount = request->setRCQValues(xmlValue.c_str());
}

void
PricingModelMap::checkParsingRetailerCodeError(bool isPRM, uint8_t rcqCount)
{
  if (!fallback::fallbackFRRProcessingRetailerCode(_trx))
  {
    if (isPRM && (rcqCount > 1))
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "MAXIMUM 1 RETAILER RULE QUALIFIER PERMITTED");

    if (!fallback::fallbackFixProcessingRetailerCodeXRS(_trx)
        && (rcqCount > 0)
        && _options->isXRSForFRRule())
    {
      std::string sMsg = "UNABLE TO COMBINE ";
      if (isPRM)
        sMsg+= "RRM*";
      else
        sMsg+= "RR*";

      sMsg += " WITH XRS";
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, sMsg.c_str());
    }
  }
}

void
PricingModelMap::checkArunkSegmentForSfrReq() const
{
  const auto& fareComponents = _pricingTrx->itin().front()->fareComponent();

  for (const auto* fareComponent : fareComponents)
  {
    if (std::all_of(fareComponent->fareMarket()->travelSeg().cbegin(),
                    fareComponent->fareMarket()->travelSeg().cend(),
                    [](const TravelSeg* ts)
                    { return ts->isNonAirTransportation(); }))
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_SEGMENT,
                                   "Fare Component with all non air segments not allowed");
    }
  }
}

} // tse namespace
