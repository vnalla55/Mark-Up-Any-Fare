//----------------------------------------------------------------------------
//
//  File:           FareDisplayUtil.cpp
//  Created:        04/25/2004
//  Authors:
//
//  Description:    Common functions required for ATSE fare display
//
//  Copyright Sabre 2007
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

#include "Common/FareDisplayUtil.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplaySurcharge.h"
#include "Common/FareDisplayTax.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MultiTransportMarkets.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurchargeData.h"
#include "DBAccess/AddonZoneInfo.h"
#include "DBAccess/BrandedCarrier.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Nation.h"
#include "DSS/FlightCount.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"

#include <numeric>

using std::string;
using std::vector;
using std::ostringstream;
using std::map;
using std::make_pair;

namespace tse
{
FALLBACK_DECL(fallbackBrandedFaresFareDisplay);
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

namespace
{
ConfigurableValue<uint32_t>
maxResponseSize("FAREDISPLAY_SVC", "MAX_RESPONSE_SIZE", 335000);
ConfigurableValue<bool>
fdGetBkgcodeDuringFareval("FARESV_SVC", "FD_GET_BKGCODE_DURING_FAREVAL", true);
ConfigurableValue<bool>
brandedCarrier("ABCC_SVC", "BRANDED_CARRIER", false);
ConfigurableValue<bool>
fdCat15FullPrevalidation("RULECATEGORY", "FD_CAT15_FULL_PREVALIDATION", false);
ConfigurableValue<bool>
brandGrouping("FAREDISPLAY_SVC", "BRAND_GROUPING", false);
ConfigurableValue<bool>
brandService("ABCC_SVC", "BRAND_SERVICE", false);
ConfigurableValue<bool>
getMultiAirportFares("FARESC_SVC", "GET_MULTIAIRPORT_FARES", false);
ConfigurableValue<bool>
requestedMarketOnlyForSingleCarrier("FAREDISPLAY_SVC",
                                    "REQUESTED_MARKET_ONLY_FOR_SINGLE_CARRIER",
                                    false);
ConfigurableValue<bool>
chkMarketCarrier("FAREDISPLAY_SVC", "CHK_MARKET_CARRIER", false);
ConfigurableValue<uint32_t>
maxFaresCfg("FAREDISPLAY_SVC", "MAX_FARES", 650);
ConfigurableValue<uint32_t>
maxFaresSDSCfg("FAREDISPLAY_SVC", "MAX_FARES_SDS", 650);

Logger
logger("atseintl.Common.FareDisplayUtil");
}

const std::string FareDisplayUtil::DISPLAY_DETAIL = "DD";
const std::string FareDisplayUtil::HOST = "HOST";

const std::string FareDisplayUtil::ALT_CXR_YY_MSG = "ADDITIONAL CARRIERS PARTICIPATE IN YY FARES";
const std::string FareDisplayUtil::INTERLINE_INDICATOR = "I";

bool
FareDisplayUtil::getFareDisplayTrx(PricingTrx* pTrx, FareDisplayTrx*& fdTrx)
{
  fdTrx = pTrx && PricingTrx::FAREDISPLAY_TRX == pTrx->getTrxType()
              ? static_cast<FareDisplayTrx*>(pTrx)
              : nullptr;
  return fdTrx != nullptr;
}

//---------------------------------------------------------------------------
// To get a new temporary FareDisplayInfo and initialize it
//---------------------------------------------------------------------------
bool
FareDisplayUtil::initFareDisplayInfo(FareDisplayTrx* fdTrx, PaxTypeFare& ptFare)
{
  if (fdTrx == nullptr)
    return false;
  FareDisplayInfo* tempFareDisplayInfo = nullptr;

  // Get a new temporary FareDisplayInfo
  fdTrx->dataHandle().get(tempFareDisplayInfo);
  if (tempFareDisplayInfo == nullptr)
    return false;

  tempFareDisplayInfo->initialize(*fdTrx, ptFare);
  ptFare.fareDisplayInfo() = tempFareDisplayInfo;
  return true;
}

//---------------------------------------------------------------------------
// Get a FareDisplayTrx and the FareDisplayInfo object, and set the Display
// Only indicator to True
//---------------------------------------------------------------------------
void
FareDisplayUtil::setDisplayOnly(PricingTrx& pTrx,
                                PaxTypeFare& ptFare,
                                bool isDisplayOnly,
                                bool isMinMaxFare)
{
  // Get a Fare Display Transaction from the Pricing Transaction
  FareDisplayTrx* fdTrx;

  if (FareDisplayUtil::getFareDisplayTrx(&pTrx, fdTrx))
  {
    // Get Fare Display Info object
    FareDisplayInfo* fdInfo = ptFare.fareDisplayInfo();

    if (fdInfo)
    {
      if (isDisplayOnly)
        fdInfo->setDisplayOnly();

      fdInfo->setMinMaxFare(isMinMaxFare);
    }
  }
}

//-----------------------------------------------------------------------------
// getTotalFares including taxes and surcharges.
//-----------------------------------------------------------------------------
void
FareDisplayUtil::getTotalFares(FareDisplayTrx& trx)
{
  std::vector<PaxTypeFare*>::const_iterator ptfItr = trx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::const_iterator ptfEnd = trx.allPaxTypeFare().end();

  for (; ptfItr != ptfEnd; ptfItr++)
  {
    PaxTypeFare* paxTypeFare = (*ptfItr);

    if (paxTypeFare == nullptr)
    {
      continue;
    }

    getTotalFare(trx, *paxTypeFare);
  }
}

//-----------------------------------------------------------------------------
// getTotalFare including taxes and surcharges.
//-----------------------------------------------------------------------------
void
FareDisplayUtil::getTotalFare(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare)
{
  MoneyAmount totalFare = paxTypeFare.nucFareAmount();
  FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (!fareDisplayInfo)
  {
    LOG4CXX_ERROR(logger,
                  "Unable to get FareDisplayInfo object for FareBasisCode: "
                      << paxTypeFare.createFareBasisCodeFD(trx) << "\n");
    return;
  }

  LOG4CXX_DEBUG(logger,
                "getTotalFare, FareClass: " << paxTypeFare.fareClass() << ", owrt: "
                                            << paxTypeFare.owrt() << ", FareAmount: " << totalFare);

  // Add Taxes and Surcharges
  Itin* itin = trx.itin().front();
  MoneyAmount owTaxAmount(0), rtTaxAmount(0);

  MoneyAmount owSurcharge(0), rtSurcharge(0);
  if (trx.getOptions()->applySurcharges() == YES)
  {
    FareDisplaySurcharge::getTotalOWSurcharge(trx, paxTypeFare, owSurcharge);
    FareDisplaySurcharge::getTotalRTSurcharge(trx, paxTypeFare, rtSurcharge);
  }

  if (itin->calculationCurrency() != NUC &&
      // FL display need fare without tax
      trx.getOptions()->displayBaseTaxTotalAmounts() != TRUE_INDICATOR)
  {
    FareDisplayTax::getTotalTax(trx, paxTypeFare, owTaxAmount, rtTaxAmount);
  }

  const Indicator owrt = paxTypeFare.owrt();

  if (owrt == ONE_WAY_MAY_BE_DOUBLED)
  {
    paxTypeFare.convertedFareAmount() = totalFare + owTaxAmount + owSurcharge;
    paxTypeFare.convertedFareAmountRT() = totalFare * 2 + rtTaxAmount + rtSurcharge;
  }

  if (owrt == ROUND_TRIP_MAYNOT_BE_HALVED)
  {
    paxTypeFare.convertedFareAmount() = totalFare + rtTaxAmount + rtSurcharge;
    paxTypeFare.convertedFareAmountRT() = totalFare + rtTaxAmount + rtSurcharge;
  }

  if (owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
    paxTypeFare.convertedFareAmount() = totalFare + owTaxAmount + owSurcharge;

  return;
}
const bool
FareDisplayUtil::getCRSParams(FareDisplayTrx& trx, Indicator& userApplType, UserApplCode& userAppl)
{
  Agent* agent = trx.getRequest()->ticketingAgent();

  // Determine CRS type
  if (agent->cxrCode() == INFINI_MULTIHOST_ID)
  {
    userAppl = INFINI_USER;
    userApplType = CRS_USER_APPL;
  }
  else if (agent->cxrCode() == AXESS_MULTIHOST_ID)
  {
    userAppl = AXESS_USER;
    userApplType = CRS_USER_APPL;
  }
  else if (agent->cxrCode() == ABACUS_MULTIHOST_ID)
  {
    userAppl = ABACUS_USER;
    userApplType = CRS_USER_APPL;
  }
  else if (agent->cxrCode() == SABRE_MULTIHOST_ID)
  {
    userAppl = SABRE_USER;
    userApplType = CRS_USER_APPL;
  }
  else
    return false;

  return true;
}

const std::set<CarrierCode>&
FareDisplayUtil::getUniqueSetOfCarriers(
    FareDisplayTrx& trx,
    bool includeAddon,
    std::map<MultiTransportMarkets::Market, std::set<CarrierCode>>* marketCarrierMap)
{
  TSELatencyData metrics(trx, "QUERY GETMARKETCARRIERS");
  std::set<CarrierCode>* uniqueCxrSetPtr;
  trx.dataHandle().get(uniqueCxrSetPtr);
  std::set<CarrierCode>& uniqueCxrSet = *uniqueCxrSetPtr;
  const DateTime& date = trx.getRequest()->requestedDepartureDT();

  if (getMultiAirportFares.getValue() && trx.getRequest()->inclusionCode() != FD_ADDON &&
      (trx.getOptions()->isAllCarriers() || !requestedMarketOnlyForSingleCarrier.getValue()))
  {
    const std::vector<FareMarket*>& fareMarketVector = trx.fareMarket();

    if (!fareMarketVector.empty())
    {
      const FareMarket* fareMarket = *fareMarketVector.begin();

      MultiTransportMarkets multiTransportMkts(fareMarket->origin()->loc(),
                                               fareMarket->destination()->loc(),
                                               fareMarket->governingCarrier(),
                                               fareMarket->geoTravelType(),
                                               trx.ticketingDate(),
                                               fareMarket->travelDate(),
                                               fareMarket);

      std::vector<MultiTransportMarkets::Market> markets;
      multiTransportMkts.getMarkets(markets);

      std::vector<MultiTransportMarkets::Market>::const_iterator ptrItr = markets.begin();
      std::vector<MultiTransportMarkets::Market>::const_iterator ptrEnd = markets.end();

      while (ptrItr != ptrEnd)
      {
        const std::vector<CarrierCode>& carriers = trx.dataHandle().getCarriersForMarket(
            ptrItr->first, ptrItr->second, includeAddon, date);

        if (marketCarrierMap && chkMarketCarrier.getValue())
        {
          // Save this market and all carriers that have fares for it into the map.
          // This data will be used later in the process to avoid looking for fares in the db
          //  for markets/carriers that we know will not return any fare.
          std::set<CarrierCode>* cxrSetPtr;
          trx.dataHandle().get(cxrSetPtr);
          std::set<CarrierCode>& cxrSet = *cxrSetPtr;

          if (!carriers.empty())
            cxrSet.insert(carriers.begin(), carriers.end());

          MultiTransportMarkets::Market mkt = std::make_pair(ptrItr->first, ptrItr->second);
          marketCarrierMap->insert(std::make_pair(mkt, cxrSet));
        }

        if (!carriers.empty())
          uniqueCxrSet.insert(carriers.begin(), carriers.end());

        ++ptrItr;
      }

      return uniqueCxrSet;
    }
  }

  // Multi Airport fares disabled or Single carrier request or AD inclusion code

  const LocCode& orig = trx.origin()->loc();
  const LocCode& dest = trx.destination()->loc();
  const LocCode& boardMultiCity = trx.travelSeg().front()->boardMultiCity();
  const LocCode& offMultiCity = trx.travelSeg().front()->offMultiCity();

  bool diffBoardPoint = (!boardMultiCity.empty()) && (boardMultiCity != orig);

  bool diffOffPoint = (!offMultiCity.empty()) && (offMultiCity != dest);

  const std::vector<CarrierCode>& carriers =
      (trx.getRequest()->inclusionCode() == FD_ADDON)
          ? trx.dataHandle().getAddOnCarriersForMarket(orig, dest, trx.travelDate())
          : trx.dataHandle().getCarriersForMarket(orig, dest, includeAddon, date);

  if (!carriers.empty())
    uniqueCxrSet.insert(carriers.begin(), carriers.end());

  if (diffBoardPoint)
  {
    const std::vector<CarrierCode>& carriers =
        (trx.getRequest()->inclusionCode() == FD_ADDON)
            ? trx.dataHandle().getAddOnCarriersForMarket(boardMultiCity, dest, trx.travelDate())
            : trx.dataHandle().getCarriersForMarket(boardMultiCity, dest, includeAddon, date);

    if (!carriers.empty())
      uniqueCxrSet.insert(carriers.begin(), carriers.end());
  }

  if (diffOffPoint)
  {
    const std::vector<CarrierCode>& carriers =
        (trx.getRequest()->inclusionCode() == FD_ADDON)
            ? trx.dataHandle().getAddOnCarriersForMarket(orig, offMultiCity, trx.travelDate())
            : trx.dataHandle().getCarriersForMarket(orig, offMultiCity, includeAddon, date);

    if (!carriers.empty())
      uniqueCxrSet.insert(carriers.begin(), carriers.end());
  }

  if (diffBoardPoint && diffOffPoint)
  {
    const std::vector<CarrierCode>& carriers =
        (trx.getRequest()->inclusionCode() == FD_ADDON)
            ? trx.dataHandle().getAddOnCarriersForMarket(
                  boardMultiCity, offMultiCity, trx.travelDate())
            : trx.dataHandle().getCarriersForMarket(
                  boardMultiCity, offMultiCity, includeAddon, date);
    if (!carriers.empty())
      uniqueCxrSet.insert(carriers.begin(), carriers.end());
  }

  return uniqueCxrSet;
}

void
FareDisplayUtil::formatMonth(int32_t& monthValue,
                             std::ostringstream& oss,
                             const Indicator& dateFormat)
{
  if (monthValue <= 0 || monthValue > 12)
  {
    oss << ' ';
    return;
  }

  switch (dateFormat)
  {
  case '1': // DDMM
  case '3': // DDMMY
    oss.setf(std::ios::right, std::ios::adjustfield);
    oss << std::setfill('0') << std::setw(2) << SHORT_MONTHS_UPPER_CASE_FARE_DISPLAY[monthValue];
    break;
  case '2': // DDMMM
  case '4': // DDMMMY
  case '5': // DDMMMYY
    oss.setf(std::ios::right, std::ios::adjustfield);
    oss << std::setfill('0') << std::setw(3) << MONTHS_UPPER_CASE[monthValue];
    break;
  default:
    break;
  }
}

void
FareDisplayUtil::formatYear(int32_t& yearValue,
                            std::ostringstream& oss,
                            const Indicator& dateFormat)
{
  switch (dateFormat)
  {
  default:
  case '1': // DDMM
  case '2': // DDMMM
    break;
  case '3': // DDMMY
  case '4': // DDMMMY
    if (yearValue == -1)
      oss << ' ';
    else
      oss << std::setw(1) << yearValue;
    break;
  case '5': // DDMMMYY
    if (yearValue == -1)
      oss << ' ';
    else
      oss << std::setw(2) << yearValue;
    break;
  }
}

namespace
{
const Indicator ZONE_EXCLUDE = 'E';
const Indicator ZONE_INCLUDE = 'I';
const uint16_t ZONE_NO_LENGTH = 7;
const char ZONE_NO_PADDING = '0';

typedef vector<vector<ZoneInfo::ZoneSeg>> ZoneSets;

class ZoneDescrBuilder
{
public:
  ZoneDescrBuilder(Indicator inclExclInd, DataHandle& dh, const DateTime& travelDate)
    : inclExclInd_(inclExclInd), dh_(dh), travelDate_(travelDate)
  {
  }
  string& operator()(string& descr, const AddonZoneInfo* zone) const
  {
    if (zone != nullptr && zone->inclExclInd() == inclExclInd_)
    {
      if (!descr.empty())
        descr.append("/");
      const LocKey& market(zone->market());
      descr.append(decode(market.locType(), market.loc()));
    }
    return descr;
  }
  string& operator()(string& descr, const ZoneInfo::ZoneSeg& zoneSeg) const
  {
    if (zoneSeg.inclExclInd() == inclExclInd_)
    {
      if (!descr.empty())
        descr.append("/");
      descr.append(decode(zoneSeg.locType(), zoneSeg.loc()));
    }
    return descr;
  }
  string& operator()(string& descr, const vector<ZoneInfo::ZoneSeg>& zoneSegSet) const
  {
    if (!zoneSegSet.empty())
      descr = accumulate(zoneSegSet.begin(), zoneSegSet.end(), descr, *this);
    return descr;
  }
  static bool addSubAreaDescr(const IATASubAreaCode& code, const string& descr)
  {
    return subAreaDescriptions_.insert(make_pair(code, descr)).second == 1;
  }

private:
  string decode(Indicator locType, const LocCode& loc) const
  {
    string descr;
    if (locType == LOCTYPE_AREA)
    {
      descr = "AREA ";
      descr.append(loc);
    }
    else if (locType == LOCTYPE_SUBAREA)
    {
      descr = "SUB AREA ";
      descr.append(decodeSubArea(loc));
    }
    else if (locType == LOCTYPE_STATE)
    {
      descr = "STATE/PROVINCE ";
      descr.append(loc);
    }
    else if (locType == LOCTYPE_NATION)
    {
      const Nation* nation(dh_.getNation(loc, travelDate_));
      if (nation != nullptr)
        descr = nation->description();
    }
    else
      descr.append(loc);
    return descr;
  }
  string decodeSubArea(const LocCode& loc) const
  {
    string descr;
    map<IATASubAreaCode, string>::const_iterator i(subAreaDescriptions_.find(loc));
    if (i != subAreaDescriptions_.end())
      descr = i->second;
    else
      descr = loc;
    return descr;
  }
  Indicator inclExclInd_;
  DataHandle& dh_;
  const DateTime& travelDate_;
  static map<IATASubAreaCode, string> subAreaDescriptions_;
  static const bool b11, b12, b13, b14, b21, b22, b23, b31, b32, b33, b34;
};

map<IATASubAreaCode, string> ZoneDescrBuilder::subAreaDescriptions_;

const bool ZoneDescrBuilder::b11 =
    ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_AREA_11(), "NORTH AMERICA");
const bool ZoneDescrBuilder::b12 =
    ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_AREA_12(), "CARIBBEAN");
const bool ZoneDescrBuilder::b13 =
    ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_AREA_13(), "CENTRAL AMERICA");
const bool ZoneDescrBuilder::b14 =
    ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_AREA_14(), "SOUTH AMERICA");
const bool ZoneDescrBuilder::b21 = ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_AREA_21(), "EUROPE");
const bool ZoneDescrBuilder::b22 =
    ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_AREA_22(), "MIDDLE EAST");
const bool ZoneDescrBuilder::b23 = ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_AREA_23(), "AFRICA");
const bool ZoneDescrBuilder::b31 =
    ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_ARE_31(), "JAPAN/KOREA");
const bool ZoneDescrBuilder::b32 =
    ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_ARE_32(), "SOUTH EAST ASIA");
const bool ZoneDescrBuilder::b33 =
    ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_ARE_33(), "SOUTH ASIA SUB CONTINENT");
const bool ZoneDescrBuilder::b34 =
    ZoneDescrBuilder::addSubAreaDescr(IATA_SUB_ARE_34(), "SOUTH WEST PACIFIC");
}

string
FareDisplayUtil::getAddonZoneDescr(const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   TariffNumber tariff,
                                   AddonZone zone,
                                   const FareDisplayTrx& fdTrx)
{
  string descr;
  const DateTime& travelDate = fdTrx.travelDate();
  DataHandle dh(fdTrx.ticketingDate());
  bool result = getZones(dh, vendor, carrier, tariff, zone, travelDate, descr);

  if (!result)
  {
    const vector<TariffCrossRefInfo*>& addonTariff =
        dh.getTariffXRefByAddonTariff(vendor, carrier, INTERNATIONAL, tariff, travelDate);
    if (!addonTariff.empty())
    {
      std::vector<TariffCrossRefInfo*>::const_iterator ptItr = addonTariff.begin();
      std::vector<TariffCrossRefInfo*>::const_iterator ptEnd = addonTariff.end();

      for (; ptItr != ptEnd; ptItr++)
      {
        TariffCrossRefInfo* tariffXRef = (*ptItr);

        if (tariff == tariffXRef->addonTariff1())
        {
          const TariffNumber& addOnTar = tariffXRef->addonTariff2();
          result = getZones(dh, vendor, carrier, addOnTar, zone, travelDate, descr);
        }
        else
        {
          const TariffNumber& addOnTar = tariffXRef->addonTariff1();
          result = getZones(dh, vendor, carrier, addOnTar, zone, travelDate, descr);
        }

        if (result)
          return descr;

      } // end for
    } // end if

    // LOG4CXX_ERROR(logger, "No AddonZoneInfos for " << vendor << " " << carrier << " " << tariff
    // << " " << zone);
  }
  return descr;
}

string
FareDisplayUtil::getZoneDescr(const Zone& zone,
                              const FareDisplayTrx& fdTrx,
                              const VendorCode& vendor,
                              Indicator zoneType)
{
  string descr, zonePad(ZONE_NO_LENGTH, ZONE_NO_PADDING);
  copy_backward(zone.begin(), zone.end(), zonePad.end());
  const DateTime& travelDate = fdTrx.travelDate();
  DataHandle dh(fdTrx.ticketingDate());
  const ZoneInfo* zoneInfo(dh.getZone(vendor, zonePad, zoneType, travelDate));
  if (zoneInfo != nullptr)
  {
    const ZoneSets& sets(zoneInfo->sets());
    descr =
        accumulate(sets.begin(), sets.end(), descr, ZoneDescrBuilder(ZONE_INCLUDE, dh, travelDate));
    string except;
    except = accumulate(
        sets.begin(), sets.end(), except, ZoneDescrBuilder(ZONE_EXCLUDE, dh, travelDate));
    if (!except.empty())
      descr.append(" EXCEPT " + except);
  }
  else
  {
    LOG4CXX_ERROR(logger, "No ZoneInfos for " << vendor << " " << zone << " " << zoneType);
  }
  return descr;
}

string
FareDisplayUtil::splitLines(const string& text,
                            size_t lineLength,
                            const string& terminators,
                            size_t firstLineLength,
                            const string& prefix)
{
  string msg;
  size_t pos = 0;
  size_t length = firstLineLength;

  while (text.size() - pos > length)
  {
    size_t effectiveLength = length;
    if (!terminators.empty())
    {
      const size_t terminatedPos = text.find_last_of(terminators, pos + effectiveLength - 1);
      if (terminatedPos != string::npos && terminatedPos >= pos)
        effectiveLength = terminatedPos - pos + 1;
    }

    msg.append(text, pos, effectiveLength);
    msg += '\n';
    msg += prefix;

    pos += effectiveLength;

    length = lineLength;
  }

  msg.append(text, pos, string::npos);
  return msg;
}

bool
FareDisplayUtil::getZones(DataHandle& dh,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          TariffNumber tariff,
                          AddonZone zone,
                          const DateTime& travelDate,
                          std::string& descr)
{
  //   DataHandle dh;

  const vector<AddonZoneInfo*>& zones(dh.getAddOnZone(vendor, carrier, tariff, zone, travelDate));
  if (!zones.empty())
  {
    descr = accumulate(
        zones.begin(), zones.end(), descr, ZoneDescrBuilder(ZONE_INCLUDE, dh, travelDate));
    string except;
    except = accumulate(
        zones.begin(), zones.end(), except, ZoneDescrBuilder(ZONE_EXCLUDE, dh, travelDate));
    if (!except.empty())
      descr.append(" EXCEPT " + except);

    return true;
  }
  else
    return false;
}

MPType
FareDisplayUtil::determineMPType(const FareDisplayTrx& trx)
{
  if (trx.boardMultiCity().empty())
    return NO_MARKET_MP;
  else if (trx.getOptions()->lineNumber() > 0)
    return SHORT_MP;
  else
    return LONG_MP;
}

bool
FareDisplayUtil::isCat15TuningEnabled()
{
  return fdCat15FullPrevalidation.getValue();
}

bool
FareDisplayUtil::isBrandGroupingEnabled()
{
  return brandGrouping.getValue();
}

bool
FareDisplayUtil::isBrandServiceEnabled()
{
  return brandService.getValue();
}

uint32_t
FareDisplayUtil::getMaxFares()
{
  return maxFaresCfg.getValue();
}

std::string
FareDisplayUtil::getAltCxrMsg(const LocCode& orig, const LocCode& dest)
{
  std::ostringstream s;
  s << "THE FOLLOWING CARRIERS ALSO PUBLISH FARES " << orig << "-" << dest << ":";
  return s.str();
}

std::string
FareDisplayUtil::getConnectingFlightCount(const FareDisplayTrx& trx, const FlightCount& fc)
{
  std::ostringstream s;
  if (isWebUser(trx) && fc._interLineServiceExist == true)
    s << INTERLINE_INDICATOR;
  else
    s << (fc._onlineConnection < MAX_ALLOWED_ONLINE_CONNECTION ? fc._onlineConnection
                                                               : MAX_ALLOWED_ONLINE_CONNECTION);
  return s.str();
}

bool
FareDisplayUtil::isWebUser(const FareDisplayTrx& trx)
{
  if (trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
  {
    return false;
    ;
  }
  if (trx.getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() == YES)
  {
    return true;
  }
  return false;
}

bool
FareDisplayUtil::isAxessUser(const FareDisplayTrx& trx)
{
  const Agent* agent = trx.getRequest()->ticketingAgent();

  if (agent && agent->cxrCode() == AXESS_MULTIHOST_ID)
  {
    return true;
  }

  return false;
}

bool
FareDisplayUtil::isQualifiedRequest(const FareDisplayTrx& trx)
{
  const FareDisplayRequest* request = trx.getRequest();
  const FareDisplayOptions* options = trx.getOptions();

  if (!request || !options)
  {
    return false;
  }

  if (!request->displayPassengerTypes().empty() || !request->fareBasisCode().empty() ||
      !request->bookingCode().empty() || !request->accountCode().empty() ||
      !request->corporateID().empty() || !request->ticketDesignator().empty() ||
      !options->alternateCurrency().empty() || options->isExcludePenaltyFares() ||
      options->isExcludeAdvPurchaseFares() || options->isExcludeMinMaxStayFares() ||
      options->isExcludeRestrictedFares() || options->isPrivateFares() || options->isOneWayFare() ||
      options->isAdultFares() || options->isChildFares() || options->isInfantFares() ||
      options->isUniqueAccountCode() || options->isUniqueCorporateID())
  {
    return true;
  }

  return false;
}

bool
FareDisplayUtil::isHostDBInfoDiagRequest(const FareDisplayTrx& trx)
{
  int16_t diagNum = trx.getRequest()->diagnosticNumber();

  if (diagNum == 854)
    return true;

  // Check if the diagnostic request included the /DDHOST option
  std::map<std::string, std::string>::const_iterator diagIter =
      trx.diagnostic().diagParamMap().find(DISPLAY_DETAIL);

  if (diagIter != trx.diagnostic().diagParamMap().end())
  {
    if (diagIter->second == HOST)
      return true;
  }

  return false;
}

bool
FareDisplayUtil::validCarrierFaresExist(const FareDisplayTrx& trx)
{
  Itin* itin = trx.itin().front();
  std::vector<FareMarket*>::const_iterator fmItr = itin->fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmEnd = itin->fareMarket().end();

  CarrierCode carrierCode;
  int fareCount = 0;

  for (; fmItr != fmEnd; fmItr++)
  {
    FareMarket* fareMarket = (*fmItr);
    std::vector<PaxTypeFare*>::const_iterator ptfItr = fareMarket->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::const_iterator ptfEnd = fareMarket->allPaxTypeFare().end();

    for (; ptfItr != ptfEnd; ptfItr++)
    {
      PaxTypeFare* paxTypeFare = (*ptfItr);
      carrierCode = paxTypeFare->carrier();

      if (carrierCode != INDUSTRY_CARRIER && paxTypeFare->isValidForPricing())
        fareCount++;
    }
  }
  if (fareCount > 0)
    return true;
  else
    return false;
}

bool
FareDisplayUtil::validYYFaresExist(const FareDisplayTrx& trx)
{
  Itin* itin = trx.itin().front();
  std::vector<FareMarket*>::const_iterator fmItr = itin->fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmEnd = itin->fareMarket().end();

  CarrierCode carrierCode;
  int fareCount = 0;

  for (; fmItr != fmEnd; fmItr++)
  {
    FareMarket* fareMarket = (*fmItr);
    std::vector<PaxTypeFare*>::const_iterator ptfItr = fareMarket->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::const_iterator ptfEnd = fareMarket->allPaxTypeFare().end();

    for (; ptfItr != ptfEnd; ptfItr++)
    {
      PaxTypeFare* paxTypeFare = (*ptfItr);
      carrierCode = paxTypeFare->carrier();

      if (carrierCode == INDUSTRY_CARRIER && paxTypeFare->isValidForPricing())
        fareCount++;
    }
  }
  if (fareCount > 0)
    return true;
  else
    return false;
}

void
FareDisplayUtil::displayNotPublish(const FareDisplayTrx& trx,
                                   std::ostringstream& p0,
                                   InclusionCode& incCode,
                                   std::string& privateString,
                                   bool padding,
                                   bool firstLine)
{
  const CarrierCode& carrier = trx.requestedCarrier();
  if (padding && firstLine)
    p0 << " " << std::endl;
  p0 << "*** " << carrier << " DOES NOT PUBLISH ";

  if ((incCode != "NLX") || ((trx.boardMultiCity()) != (trx.offMultiCity())))
  {
    p0 << incCode;
  }
  p0 << privateString << " "
     << "FARES " << trx.boardMultiCity() << "-" << trx.offMultiCity() << " ***";
  if (padding)
    p0 << " " << std::endl;
}

bool
FareDisplayUtil::getCarrierFareHeaderMsg(const FareDisplayTrx& trx,
                                         std::ostringstream& s,
                                         bool padding)
{
  const CarrierCode& carrier = trx.requestedCarrier();
  bool ret = false;
  std::ostringstream& p0 = s;

  InclusionCode incCode;

  if (trx.getRequest()->inclusionCode() == ALL_INCLUSION_CODE || isQualifiedRequest(trx) ||
      isAxessUser(trx))
  {
    incCode = "REQUESTED";
  }
  else
  {
    incCode = trx.getRequest()->inclusionCode();
  }

  std::string privateString = "";
  if ((trx.getOptions()->isPrivateFares()) && (incCode == "REQUESTED"))
    privateString = " PRVT";
  else if (trx.getOptions()->isPrivateFares())
    privateString = " PRIVATE";

  if (trx.isShopperRequest())
    return ret;
  else if (!validCarrierFaresExist(trx) && carrier != INDUSTRY_CARRIER)
  {
    // Carrier does NOT Publish fares - 3.12.3
    if (!fallback::fallbackFareDisplayByCabinActivation(&trx) &&
        trx.getRequest()->multiInclusionCodes() && incCode != "REQUESTED")
    {
      if (incCode.size() <= 10)
      {
        bool firstLine = true;
        uint8_t sizeIncl = trx.getRequest()->requestedInclusionCode().size() / 2;
        for (uint8_t number = 0; number < sizeIncl; ++number)
        {
          incCode = trx.getRequest()->requestedInclusionCode().substr(number * 2, 2);
          displayNotPublish(trx, p0, incCode, privateString, padding, firstLine);
          firstLine = false;
        }
        return true;
      }
      if (incCode.size() > 10)
      {
        incCode = "AB";
      }
    }
    if (!fallback::fallbackFareDisplayByCabinActivation(&trx))
      displayNotPublish(trx, p0, incCode, privateString, padding);
    else
    {
      if (padding)
        p0 << " " << std::endl;
      p0 << "*** " << carrier << " DOES NOT PUBLISH ";

      if ((incCode != "NLX") || ((trx.boardMultiCity()) != (trx.offMultiCity())))
      {
        p0 << incCode;
      }
      p0 << privateString << " "
         << "FARES " << trx.boardMultiCity() << "-" << trx.offMultiCity() << " ***";
      if (padding)
        p0 << " " << std::endl;
    }
    ret = true;
  }

  return ret;
}

//---------------------------------------------------------------------------
// shortRequestForPublished(PricingTrx& trx)
//
//---------------------------------------------------------------------------
bool
FareDisplayUtil::shortRequestForPublished(PricingTrx& trx)
{
  FareDisplayTrx* fdTrx;

  if (UNLIKELY(getFareDisplayTrx(&trx, fdTrx) && fdTrx->isShortRequestForPublished()))
  {
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
// initializeFarePath()
//----------------------------------------------------------------------------
bool
FareDisplayUtil::initializeFarePath(const FareDisplayTrx& trx, FarePath* farePath)
{
  // Create an itin
  Itin* itin(nullptr);
  trx.dataHandle().get(itin);
  if (!itin)
  {
    LOG4CXX_ERROR(logger,
                  "FareDisplayUtil::initializeFarePath - ERROR-UNABLE TO ALLOCATE MEMORY FOR ITIN");
    return false;
  }
  itin->duplicate(*(trx.itin().front()), trx.dataHandle());
  // Add a return travel seg, if one doesn't exist already
  if (itin->travelSeg().size() == 1)
  {
    TravelSeg* tvlSeg = itin->travelSeg().front();
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlSeg);
    if (airSeg == nullptr)
    {
      LOG4CXX_DEBUG(
          logger,
          "FareDisplayUtil::initializeFarePath - ERROR DYNAMIC CASTING TRAVELSEG TO AIRSEG");
      return false;
    }
    AirSeg* returnSeg(nullptr);
    trx.dataHandle().get(returnSeg);
    returnSeg->segmentOrder() = 1;
    returnSeg->departureDT() = airSeg->departureDT().addDays(2);
    returnSeg->arrivalDT() = returnSeg->departureDT().addDays(2);
    returnSeg->origAirport() = airSeg->destAirport();
    returnSeg->origin() = airSeg->destination();
    returnSeg->destAirport() = airSeg->origAirport();
    returnSeg->destination() = airSeg->origin();

    itin->travelSeg().push_back(returnSeg);
  }

  farePath->itin() = itin;
  itin->farePath().push_back(farePath);

  // Build a Fare Usage and populate it with travelSegs from Itin
  if (farePath->pricingUnit().empty())
  {
    //-------------------------------
    // Build an Outbound Fare Usage
    //-------------------------------
    FareUsage* fareUsage(nullptr);
    trx.dataHandle().get(fareUsage);
    if (!fareUsage)
    {
      LOG4CXX_DEBUG(
          logger,
          "FareDisplayUtil::initializeFarePath - ERROR-UNABLE TO ALLOCATE MEMORY FOR FAREUSAGE");
      return false;
    }
    fareUsage->travelSeg().push_back(itin->travelSeg()[0]);
    fareUsage->inbound() = false;
    fareUsage->isRoundTrip() = false;

    // -------------------
    // Build Pricing Unit
    // -------------------
    PricingUnit* pricingUnit(nullptr);
    trx.dataHandle().get(pricingUnit);
    if (!pricingUnit)
    {
      LOG4CXX_DEBUG(logger,
                    "FareDisplayUtil::initializeFarePath - ERROR-UNABLE TO ALLOCATE "
                    "MEMORY FOR PRICING UNIT;");
      return false;
    }
    pricingUnit->fareUsage().push_back(fareUsage);
    pricingUnit->travelSeg().push_back(itin->travelSeg()[0]);
    pricingUnit->travelSeg().push_back(itin->travelSeg()[1]);

    // -------------------------------------------
    // Build an Inbound Fare Usage
    // --------------------------------------------
    FareUsage* returnFareUsage(nullptr);
    trx.dataHandle().get(returnFareUsage);
    if (!returnFareUsage)
    {
      LOG4CXX_DEBUG(logger,
                    "FareDisplayTax::initializeFarePath - ERROR-UNABLE TO ALLOCATE MEMORY "
                    "FOR RETURN FARE USAGE");
      return false;
    }
    returnFareUsage->inbound() = true;
    returnFareUsage->isRoundTrip() = false;
    returnFareUsage->travelSeg().push_back(itin->travelSeg()[1]);
    pricingUnit->fareUsage().push_back(returnFareUsage);
    farePath->pricingUnit().push_back(pricingUnit);

    /*All paxTypeFare.nucFareAmount has already been convered to calculation
    * currency, hence set the origination currency to calculation currency
    * for correct tax calculations*/
    farePath->itin()->originationCurrency() = trx.itin().front()->calculationCurrency();
    farePath->baseFareCurrency() = trx.itin().front()->calculationCurrency();
    farePath->calculationCurrency() = trx.itin().front()->calculationCurrency();
  }
  return true;
}

const LocCode&
FareDisplayUtil::getFareOrigin(PaxTypeFare* paxTypeFare)
{
  const PaxTypeFare* ptFare = paxTypeFare->fareWithoutBase();
  if (ptFare->isReversed() && ptFare->directionality() == BOTH)
    return ptFare->destination();
  else
    return ptFare->origin();
}
const LocCode&
FareDisplayUtil::getFareDestination(PaxTypeFare* paxTypeFare)
{
  const PaxTypeFare* ptFare = paxTypeFare->fareWithoutBase();
  if (ptFare->isReversed() && ptFare->directionality() == BOTH)
    return ptFare->origin();
  else
    return ptFare->destination();
}

bool
FareDisplayUtil::updateCreateTimeFormat(std::string& crTime, bool dash)
{
  if (dash)
  {
    // Change create time from HH:MM:SS.mmmmmm to HH-MM-SS.mmmmmm format
    if (crTime[2] != ':' && crTime[5] != ':')
      return false;
    crTime[2] = '-';
    crTime[5] = '-';
  }
  else
  {
    // Change create time from HH-MM-SS.mmmmmm to HH:MM:SS.mmmmmm format
    if (crTime[2] != '-' && crTime[5] != '-')
      return false;
    crTime[2] = ':';
    crTime[5] = ':';
  }
  return true;
}

//------------------------------------------------------
// FareDisplayUtil::isBrandGrouping()
//------------------------------------------------------
bool
FareDisplayUtil::isBrandGrouping(FareDisplayTrx& trx)
{
  // New Branded service active, data collected by S8BrandService, return true
  if (TrxUtil::isFqS8BrandedServiceActivated(trx) &&
      !fallback::fallbackBrandedFaresFareDisplay(&trx) && trx.isS8ServiceCalled())
  {
    return true;
  }

  if (!FareDisplayUtil::isBrandGroupingEnabled() || trx.isShopperRequest() ||
      trx.dataHandle().isHistEnabled(trx.getRequest()->ticketingDT()))
  {
    return false;
  }

  if (FareDisplayUtil::isBrandServiceEnabled())
  {
    // Check if this carrier is branded
    if (!brandedCarrier.getValue())
      return true; // No Branded Carrier table - call the service for any carrier

    const std::vector<BrandedCarrier*>& brandedCarrierVec = trx.dataHandle().getBrandedCarriers();

    if (brandedCarrierVec.empty())
    {
      return false;
    }

    std::vector<BrandedCarrier*>::const_iterator i = brandedCarrierVec.begin();
    std::vector<BrandedCarrier*>::const_iterator e = brandedCarrierVec.end();

    CarrierCode carrier = trx.requestedCarrier();

    for (; i != e; i++)
    {
      if ((*i)->carrier() == carrier)
      {
        return true;
      }
    }
  }
  else if (!trx.getOptions()->isBrandGroupingOptOut())
  {
    return true;
  }

  return false;
}

bool
FareDisplayUtil::isXMLDiagRequested(FareDisplayTrx& trx, const std::string& diagParam)
{
  if (diagParam.empty())
    return false;

  if (trx.getRequest()->diagnosticNumber() != DIAG_195_ID)
    return false;

  if (!trx.diagnostic().isActive())
    return false;

  std::map<std::string, std::string>::iterator endI = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::iterator beginI =
      trx.diagnostic().diagParamMap().find(DISPLAY_DETAIL);

  if (beginI == endI)
    return false;

  return (diagParam == (*beginI).second);
}

void
FareDisplayUtil::displayXMLDiag(FareDisplayTrx& trx,
                                const std::string& xmlData,
                                const std::string& diagHeader,
                                StatusBrandingService status)
{
  static const char startChar = '<'; // change to '-'
  static const char endChar = '>'; // change to ' '
  static const char excChar = '?'; // change to ' '
  static const char eomChar = '|'; // change to ','
  static const char equChar = '='; // change to '-'
  static const char endTag = '/'; // change to ' '
  static const char quoteChar = '"'; // change to ':'
  static const char commaChar = ',';
  static const char tildeChar = '~';
  static const char apostropheChar = '\'';
  static const char endlChar = '\n';

  static const char ch = ':';
  static const char bl = ' ';
  static const char dc = '-';

  bool elementStart = false;
  bool elementEnd = false;
  bool quoteEnd = false;

  int charCount = 0;
  int identCount = 1;

  trx.response() << "***************************************************************" << std::endl;
  trx.response() << " " << diagHeader << " - START" << std::endl;
  trx.response() << "***************************************************************" << std::endl
                 << " " << std::endl;

  std::string result;
  result.resize(xmlData.size()); // allocate space

  std::transform(xmlData.begin(), xmlData.end(), result.begin(), (int (*)(int))toupper);

  // Remove blank spaces and new line chars between ">" and "<"
  std::string::size_type position = 0;

  while (position < result.size())
  {
    if (elementEnd && (result[position] == bl || result[position] == endlChar))
    {
      result.erase(position, 1);
      continue;
    }

    if (result[position] == endChar)
      elementEnd = true;

    else if (result[position] == startChar)
      elementEnd = false;

    ++position;
  } // end while

  // Format XML request or response for display
  position = 0;

  while (position < result.size())
  {
    if (charCount > 63)
    {
      result.insert(position, 1, endlChar);
      charCount = 0;
    }

    if (result[position] == startChar)
    {
      result.replace(position, 1, 1, dc);
      elementStart = true;
    }

    else if (result[position] == endTag)
    {
      if (!quoteEnd)
      {
        elementStart = false;

        if (result[position + 1] == endChar)
          result.replace(position, 1, 1, bl);

        if (identCount > 0)
          --identCount;
      }
    }

    else if (result[position] == endChar)
    {
      if (result[position + 1] != startChar && elementStart)
      {
        result.replace(position, 1, 1, dc);
        ++position;
        ++charCount;
        continue;
      }

      result.replace(position, 1, 1, endlChar);
      ++position;

      if (result[position] == startChar && result[position + 1] == endTag && identCount > 0)
        --identCount;

      if (identCount)
        result.insert(position, identCount, bl);

      position = position + identCount;
      charCount = identCount + 1;
      elementStart = false;
      ++identCount;

      continue;
    }

    else if (elementStart && result[position] == bl)
    {
      result.replace(position, 1, 1, endlChar);
      ++position;

      if (identCount)
        result.insert(position, identCount, bl);

      position = position + identCount;
      charCount = identCount + 1;
      elementStart = false;

      continue;
    }

    else if (result[position] == excChar)
    {
      result.replace(position, 1, 1, bl);

      if (result[position + 1] == endChar && identCount > 0)
        --identCount;
    }

    else if (result[position] == eomChar)
    {
      result.replace(position, 1, 1, commaChar);
    }

    else if (result[position] == equChar)
    {
      result.replace(position, 1, 1, dc);
    }

    else if (result[position] == apostropheChar)
    {
      result.replace(position, 1, 1, bl);
    }

    else if (result[position] == quoteChar)
    {
      result.replace(position, 1, 1, ch);

      if (quoteEnd)
      {
        quoteEnd = false;
        ++position;

        if (result[position] == bl && result[position + 1] != endTag &&
            result[position + 1] != endChar)
        {
          result.replace(position, 1, 1, endlChar);
          ++position;

          if (identCount)
            result.insert(position, identCount, bl);

          position = position + identCount;
          charCount = identCount + 1;

          continue;
        }

        --position;
      }
      else
        quoteEnd = true;
    }

    else if (result[position] == tildeChar || result[position] == endlChar)
    {
      charCount = 0;
    }

    ++position;
    ++charCount;
  } // end while

  trx.response() << result.c_str() << std::endl << " " << std::endl;

  trx.response() << "***************************************************************" << std::endl;
  trx.response() << " " << diagHeader << " - END" << std::endl;
  trx.response() << "***************************************************************" << std::endl
                 << " " << std::endl;

  if (status == StatusBrandingService::NO_BS_ERROR)
  {
    return;
  }

  std::string errorMessage;
  getBrandingServiceErrorMessage(status, errorMessage);
  trx.response() << errorMessage << std::endl << " " << std::endl;
}

bool
FareDisplayUtil::isFrrCustomer(PricingTrx& trx)
{
  if (!trx.getRequest() || !trx.getRequest()->ticketingAgent() ||
      trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
    return false;

  const DateTime localTimeDT = DateTime::localTime();
  const std::vector<CustomerSecurityHandshakeInfo*>& customerSH1 =
      trx.dataHandle().getCustomerSecurityHandshake(
          trx.getRequest()->ticketingAgent()->tvlAgencyPCC(), "RN", localTimeDT);
  if (!customerSH1.empty())
    return true;

  const std::vector<CustomerSecurityHandshakeInfo*>& customerSH3 =
      trx.dataHandle().getCustomerSecurityHandshake(
          trx.getRequest()->ticketingAgent()->tvlAgencyPCC(), "RS", localTimeDT);
  if (!customerSH3.empty())
    return true;

  if (trx.getRequest()->ticketingAgent()->mainTvlAgencyPCC() !=
      trx.getRequest()->ticketingAgent()->tvlAgencyPCC())
  {
    const std::vector<CustomerSecurityHandshakeInfo*>& customerSH2 =
        trx.dataHandle().getCustomerSecurityHandshake(
            trx.getRequest()->ticketingAgent()->mainTvlAgencyPCC(), "RN", localTimeDT);
    if (!customerSH2.empty())
      return true;

    const std::vector<CustomerSecurityHandshakeInfo*>& customerSH4 =
        trx.dataHandle().getCustomerSecurityHandshake(
            trx.getRequest()->ticketingAgent()->mainTvlAgencyPCC(), "RS", localTimeDT);
    if (!customerSH4.empty())
      return true;
  }

  return false;
}
}
