//-------------------------------------------------------------------
//
//  File:
//  Created:     April 4, 2005
//  Authors:     LeAnn Perez
//
//  Updates:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/FareSelector.h"

#include "BookingCode/FareDisplayBookingCode.h"
#include "Common/FareCalcUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/ItinUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/Thread/ThreadPoolFactory.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TSEException.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareDisplay/BookingCodeQualifier.h"
#include "FareDisplay/DirectionalityQualifier.h"
#include "FareDisplay/FareClassAppSegInfoQualifier.h"
#include "FareDisplay/FareClassAppUnavailTagQualifier.h"
#include "FareDisplay/FareClassQualifier.h"
#include "FareDisplay/FareSelectorQualifierChain.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/InclusionCodeQualifier.h"
#include "FareDisplay/NETInclusionCodeQualifier.h"
#include "FareDisplay/OWRTQualifier.h"
#include "FareDisplay/PublicOrPrivateQualifier.h"
#include "FareDisplay/TicketDesignatorQualifier.h"
#include "FareDisplay/TravelDateQualifier.h"
#include "Rules/RuleConst.h"

#include <vector>

namespace tse
{
const Indicator FareSelector::UNAVAILABLE = 'X'; // an anavailable tag in REC1

Logger
FareSelector::_logger("atseintl.FareDisplay.FareSelector");

// initialize set for SITA unique pax types
std::set<PaxTypeCode> FareSelector::_sitaUniquePaxTypes = FareSelector::initSitaUniquePaxTypesSet();

std::set<PaxTypeCode>
FareSelector::initSitaUniquePaxTypesSet()
{
  std::set<PaxTypeCode> temp;

  temp.insert("PSD");
  temp.insert("PSZ");
  temp.insert("STC");
  temp.insert("STN");
  temp.insert("XES");
  temp.insert("XMA");
  temp.insert("XPA");
  temp.insert("ZBE");
  temp.insert("ZCD");
  temp.insert("ZCH");
  temp.insert("ZDD");
  temp.insert("ZDE");
  temp.insert("ZDM");
  temp.insert("ZDP");
  temp.insert("ZFA");
  temp.insert("ZFB");
  temp.insert("ZFH");
  temp.insert("ZFI");
  temp.insert("ZFR");
  temp.insert("ZGB");
  temp.insert("ZGR");
  temp.insert("ZGS");
  temp.insert("ZIE");
  temp.insert("ZIL");
  temp.insert("ZIT");
  temp.insert("ZJT");
  temp.insert("ZKR");
  temp.insert("ZLR");
  temp.insert("ZLU");
  temp.insert("ZMP");
  temp.insert("ZMY");
  temp.insert("ZNL");
  temp.insert("ZOR");
  temp.insert("ZPF");
  temp.insert("ZPG");
  temp.insert("ZPR");
  temp.insert("ZRC");
  temp.insert("ZRD");
  temp.insert("ZRG");
  temp.insert("ZRS");
  temp.insert("ZSF");
  temp.insert("ZSG");
  temp.insert("ZSI");
  temp.insert("ZSK");
  temp.insert("ZSM");
  temp.insert("ZSY");
  temp.insert("ZTN");
  temp.insert("ZTP");
  temp.insert("ZTR");
  temp.insert("ZTW");
  temp.insert("ZUI");
  temp.insert("ZUS");
  temp.insert("ZVN");
  temp.insert("ZWV");
  temp.insert("ZYA");
  temp.insert("ZYC");
  temp.insert("ZZA");
  temp.insert("ZZB");
  temp.insert("ZZC");
  temp.insert("ZZD");
  temp.insert("ZZE");
  temp.insert("ZZF");
  temp.insert("ZZG");
  temp.insert("ZZH");
  temp.insert("ZZI");
  temp.insert("ZZJ");
  temp.insert("ZZK");
  temp.insert("ZZL");
  temp.insert("ZZM");
  temp.insert("ZZN");
  temp.insert("ZZO");
  temp.insert("ZZP");
  temp.insert("ZZQ");
  temp.insert("ZZR");
  temp.insert("ZZS");
  temp.insert("ZZT");
  temp.insert("ZZU");
  temp.insert("ZZV");
  temp.insert("ZZW");
  temp.insert("ZZX");
  temp.insert("ZZY");

  return temp;
}

bool
FareSelector::setup(FareDisplayTrx& trx)
{
  FareSelectorQualifierChain* p;
  _dh.get(p);
  _qualifierChain = p;
  setupChain(trx);
  return true;
}
bool
FareSelector::setupChain(FareDisplayTrx& trx)
{
  _qualifierChain->buildChain(trx, true);
  return true;
}

//------------------------------------------------------
// selectFares()
//------------------------------------------------------
bool
FareSelector::selectFares(FareDisplayTrx& trx)
{
  LOG4CXX_ERROR(_logger, " Entered FareSelector::selectFares()  THIS IS AN ERROR");
  return false;
}

/**
*  QualifyOutboundCurrency()
**/
void
FareSelector::qualifyOutboundCurrency(FareDisplayTrx& trx,
                                      FareMarket& fareMarket,
                                      CurrencyCode& alternateCurrency)
{
  TSELatencyData metrics(trx, "QUAL OUT BOUND CURR");
  CurrencyCode currency;

  Itin& itin = *(trx.itin().front());

  // Skip currency check if multiple currencies allowed.
  if (alternateCurrency.empty())
  {
    if (ItinUtil::applyMultiCurrencyPricing(&trx, itin))
    {
      return;
    }
  }

  std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();
  std::vector<PaxTypeBucket>::iterator paxTypeCortI = paxTypeCortegeVec.begin();
  std::vector<PaxTypeBucket>::iterator paxTypeCortEnd = paxTypeCortegeVec.end();

  for (; paxTypeCortI < paxTypeCortEnd; paxTypeCortI++)
  {
    // Select fares based on outbound currency or Alternate Currency
    currency = (*paxTypeCortI).outboundCurrency();
    if (!alternateCurrency.empty())
    {
      currency = alternateCurrency;
    }

    PaxTypeBucket& paxTypeCortege = (*paxTypeCortI);
    PaxTypeFarePtrVec& paxTypeFareVec = paxTypeCortege.paxTypeFare();

    std::vector<tse::PaxTypeFare*>::const_iterator paxTypeFareI = paxTypeFareVec.begin();
    std::vector<tse::PaxTypeFare*>::const_iterator paxTypeFareEnd = paxTypeFareVec.end();

    for (; paxTypeFareI < paxTypeFareEnd; paxTypeFareI++)
    {
      PaxTypeFare& paxTypeFare = **paxTypeFareI;

      if (!paxTypeFare.isValid())
      {
        continue;
      }
      if (paxTypeFare.carrier() == INDUSTRY_CARRIER && alternateCurrency.empty())
      {
        continue;
      }

      // Allow CAT 25 fare with Fare Type K, E, or F.
      if (paxTypeFare.isSpecifiedCurFare())
      {
        continue;
      }

      if (paxTypeFare.currency() != currency)
      {
        LOG4CXX_DEBUG(_logger, "Outbound Currency invalidating: " << paxTypeFare.fareClass());
        paxTypeFare.invalidateFare(PaxTypeFare::FD_Outbound_Currency);
      }
    }
  }
}

/**
*  checkInhibitIndicator()
**/
void
FareSelector::checkInhibitIndicator(FareDisplayTrx& trx, PaxTypeFare& ptFare)
{
  // Check if inhibit indicator is 'D' - for Display Only
  if ((!ptFare.isFareInfoMissing() && ptFare.inhibit() == RuleConst::FARE_FOR_DISPLAY_ONLY) ||
      ptFare.isFareValidForDisplayOnly())
  {

    // Get Fare Display Info object
    FareDisplayInfo* fdInfo = ptFare.fareDisplayInfo();

    if (fdInfo)
    {
      // Update FareDisplayInfo object: fare for Display Only
      fdInfo->setDisplayOnly();
    }
  }
}

/**
*  checkUnavailTag()
**/
void
FareSelector::checkUnavailTag(PaxTypeFare& ptFare)
{
  if (ptFare.fcaUnavailTag() == UNAVAILABLE)
  {

    // Get Fare Display Info object
    FareDisplayInfo* fdInfo = ptFare.fareDisplayInfo();

    if (fdInfo)
    {
      // Update FareDisplayInfo object: fare for Display Only
      fdInfo->setUnavailableR1Rule();
    }
  }
}

/**
*  checkUnsupportedSITAPaxTypes()
**/
void
FareSelector::checkUnsupportedSITAPaxTypes(PaxTypeFare& ptFare)
{
  // Due to a PSS pricing limitation, some SITA fares with unique pax types
  //  will be returned by Fare Display yet are not priceable

  if (ptFare.vendor() != SITA_VENDOR_CODE)
  {
    return;
  }

  PaxTypeCode paxType = "ADT";

  if (ptFare.actualPaxType() != nullptr)
  {
    paxType = ptFare.actualPaxType()->paxType();
  }

  // If the fare is filed in one of SITA unique pax types,
  //  it will not auto price
  if (_sitaUniquePaxTypes.find(paxType) == _sitaUniquePaxTypes.end())
  {
    return;
  }

  // Get Fare Display Info object
  FareDisplayInfo* fdInfo = ptFare.fareDisplayInfo();

  if (fdInfo)
  {
    // Update FareDisplayInfo object: fare for Display Only
    fdInfo->setUnsupportedPaxType();
  }
}

void
FareSelector::getBookingCode(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare)
{
  TSELatencyData metrics(trx, "GET BKG CODE");
  FareDisplayBookingCode fdbc;
  fdbc.getBookingCode(trx, paxTypeFare, paxTypeFare.bookingCode());
}

// -------------------------------------------------------------------
//
// @MethodName  FareSelector::qualifyThisFare()
//
// Determine if this fare is valid
//
// @param trx - a reference to a FareDisplayTrx.
//
// @return bool
//
// -------------------------------------------------------------------------

bool
FareSelector::qualifyThisFare(FareDisplayTrx& trx, PaxTypeFare& ptFare)
{
  TSELatencyData metrics(trx, "QUAL FARE");
  LOG4CXX_INFO(_logger, " Entered FareSelector::qualifyThisFare()");

  if (_qualifierChain)
  {
    PaxTypeFare::FareDisplayState res = _qualifierChain->qualifyPaxTypeFare(trx, ptFare);

    if (!(res == PaxTypeFare::FD_Valid ||
          (res == PaxTypeFare::FD_FareClassApp_Unavailable && trx.isERD() && trx.getOptions() &&
           trx.getOptions()->isErdAfterCmdPricing())))
    {
      ptFare.invalidateFare(res);
      return false;
    }
  }

  checkInhibitIndicator(trx, ptFare);
  checkUnavailTag(ptFare);
  checkUnsupportedSITAPaxTypes(ptFare);

  LOG4CXX_DEBUG(
      _logger, "!!!! VALID FARE: " << ptFare.fareClass() << ", pax type: " << ptFare.fcasPaxType());

  return true;
}

uint32_t
FareSelector::getActiveThreads()
{
  if (!ThreadPoolFactory::isMetricsEnabled())
    return 0;

  return ThreadPoolFactory::getNumberActiveThreads(TseThreadingConst::FARE_SELECTOR_TASK);
}
} //tse
