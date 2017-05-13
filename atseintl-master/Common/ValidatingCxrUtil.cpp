//-------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Common/ValidatingCxrUtil.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/DefaultValidatingCarrierFinder.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/Agent.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DBAccess/AirlineCountrySettlementPlanInfo.h"
#include "DBAccess/AirlineInterlineAgreementInfo.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/GenSalesAgentInfo.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NeutralValidatingAirlineInfo.h"
#include "Diagnostic/Diag191Collector.h"
#include "Pricing/FarePathFactoryStorage.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/PricingUtil.h"

#include <boost/algorithm/string/trim.hpp>

namespace tse
{

FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(fallbackNonPreferredVC);
//FALLBACK_DECL(fallback_TSDTS454_YYCxrFareSelection)
FALLBACK_DECL(fallbackNonBSPVcxrPhase1);

// Only following Settlement Plans have GSA or NVC swaps
const std::vector<SettlementPlanType> ValidatingCxrUtil::swapAllowedSp{"BSP", "ARC"};

static Logger
logger("atseintl.Common.ValidatingCxrUtil");

/* Return required ticket type given ticket type preference and requirement by settlement method
   and carrier in country of POS. Refer to the FRD for details.
*/
vcx::TicketType
ValidatingCxrUtil::getTicketingMethod(const CountrySettlementPlanInfo& cspi,
                                      const AirlineCountrySettlementPlanInfo* acspi)
{
  if ((cspi.getRequiredTicketingMethod() == vcx::TM_ELECTRONIC) ||
      (acspi->getRequiredTicketingMethod() == vcx::TM_ELECTRONIC))
    return vcx::ETKT_REQ;

  if (acspi->getPreferredTicketingMethod() == vcx::TM_PAPER)
    return vcx::PAPER_TKT_PREF;

  return vcx::ETKT_PREF;
}

vcx::TicketType
ValidatingCxrUtil::getTicketingMethod(const CountrySettlementPlanInfo& cspi)
{
  if (cspi.getRequiredTicketingMethod() == vcx::TM_ELECTRONIC)
    return vcx::ETKT_REQ;
  else
    return vcx::ETKT_PREF;
}

/* Determines if airline participates in the country settlement plan. If not, returns zero.
   Otherwise, returns the plan info
*/
AirlineCountrySettlementPlanInfo*
ValidatingCxrUtil::getAirlineCountrySettlementPlanInfo(const CountrySettlementPlanInfo& cspi,
                                                       Diag191Collector* diag,
                                                       const CarrierCode& carrier,
                                                       const CrsCode& crs,
                                                       DataHandle& dataHandle)
{
  const std::vector<AirlineCountrySettlementPlanInfo*>& acspiList =
      dataHandle.getAirlineCountrySettlementPlans(
          cspi.getCountryCode(), crs, carrier, cspi.getSettlementPlanTypeCode());

  if (UNLIKELY(diag))
  {
    std::string str = (acspiList.empty() ? " DOES NOT PARTICIPATE IN SETTLEMENT METHOD"
                                         : " PARTICIPATES IN SETTLEMENT METHOD");
    *diag << "  " << carrier << str << " " << cspi.getSettlementPlanTypeCode() << "\n";
  }

  return (acspiList.empty() ? nullptr : acspiList.front());
}

/* Determines country settlement plan. If requestedPlan is not 0 (i.e. request has
   settlement plan override), then requested plan is returned (if available) or 0 is returned.
   If requestedPlan is 0, then the first available plan in SETTLEMENT_PLAN_TYPE_HIERARCHY is
   returned.
*/
void
ValidatingCxrUtil::determineCountrySettlementPlan(PricingTrx& trx,
                                                  Diag191Collector* diag,
                                                  const SettlementPlanType* requestedPlan)
{
  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    if (!trx.countrySettlementPlanInfos().empty())
      return;
  }
  else if (trx.countrySettlementPlanInfo())
    return;

  const Loc* posLoc = getPOSLoc(trx, diag);
  NationCode nation = getNation(trx, posLoc);
  DataHandle dataHandle(determineTicketDate(trx));

  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    if (diag && trx.getRequest()->ticketingAgent())
      *diag << "** COUNTRY SETTLEMENT PLAN INFORMATION FOR NATION " << nation << " **\n";

    std::string msg;
    const std::vector<CountrySettlementPlanInfo*>& cspiList =
      getCountrySettlementPlanInfo(dataHandle, diag, nation, requestedPlan);

    CountrySettlementPlanInfo* cspInfo = nullptr;
    // Settlement Plan Override (VM)
    if (requestedPlan && !requestedPlan->empty())
    {
      if ( diag && requestedPlan && !requestedPlan->empty())
        *diag << "SPECIFIED SETTLEMENT METHOD: " << *requestedPlan << std::endl;

      cspInfo = determineCountrySettlementPlanBase(cspiList, diag, nation, requestedPlan, msg);
      if (cspInfo)
        trx.addCountrySettlementPlanInfo(cspInfo);
    }
    else
    {
      // Checking TJR multiple settlement plans
      std::vector<SettlementPlanType> settlementPlans;
      if (trx.getRequest() && trx.getRequest()->ticketingAgent())
        trx.getRequest()->ticketingAgent()->getMultiSettlementPlanTypes(settlementPlans);

      if (settlementPlans.empty())
      {
        // Follow Hierarchy
        cspInfo = determineCountrySettlementPlanBase(cspiList, diag, nation, requestedPlan, msg);
        if (cspInfo)
          trx.addCountrySettlementPlanInfo(cspInfo);
        else if (diag)
          *diag << msg << "\n";
      }
      else
      {
        if (diag)
        {
          *diag << "TJR SETTLEMENT METHOD/S:";
          for (const SettlementPlanType& sp : settlementPlans)
            if (diag) *diag << " " << sp;
          if (diag) *diag << "\n";
        }

        size_t ind =0;
        for (const SettlementPlanType& sp : vcx::SP_HIERARCHY)
        {
          if (std::find(settlementPlans.begin(), settlementPlans.end(), sp) == settlementPlans.end())
            continue;

          cspInfo = determineCountrySettlementPlanBase(cspiList, diag, nation, &sp, msg);
          if (cspInfo)
            trx.addCountrySettlementPlanInfo(cspInfo);

          if (++ind >= settlementPlans.size())
            break;
        }
      }

      // GTC Settlement Plan
      SettlementPlanType gtcSp = "GTC";
      cspInfo = determineCountrySettlementPlanBase(cspiList, diag, nation, &gtcSp, msg);
      if (cspInfo)
        trx.addCountrySettlementPlanInfo(cspInfo);
      else if (diag)
      {
        *diag << " \n" << std::endl;
        *diag << "PLAN TYPE: " << gtcSp << " NATION: " << posLoc->nation() << std::endl;
        *diag << gtcSp << " " << msg << std::endl;
      }
    }

    if (trx.countrySettlementPlanInfos().empty())
    {
      if (msg.empty())
        msg = "INVALID SETTLEMENT METHOD FOR POINT OF SALE";
      throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR, msg.c_str());
    }

    const std::vector<CountrySettlementPlanInfo*>& countrySettlementPlanInfos =
        trx.isLockingNeededInShoppingPQ() ?
          trx.getCopyOfCountrySettlementPlanInfos() :
          trx.countrySettlementPlanInfos();

    // We do not need it once we move to mulitple settlement plans completely
    trx.countrySettlementPlanInfo() = countrySettlementPlanInfos.size() == 1 ?
      countrySettlementPlanInfos.front():
      determinePlanFromHierarchy(countrySettlementPlanInfos);
  }
  else
  {
    trx.countrySettlementPlanInfo() =
      determineCountrySettlementPlanBase(dataHandle, nation, diag, requestedPlan);
  }

  if ( diag )
  {
    if ( nation != posLoc->nation() )
    {
      const PseudoCityCode pcc = trx.getRequest()->ticketingAgent()->agentTJR()->pseudoCity();
      *diag << "NATION " << nation << " USED FOR ARC USER " << pcc << std::endl;
    }
    const CrsCode crs = trx.getRequest()->ticketingAgent()->cxrCode();
    *diag << "POS: NATION: " << posLoc->nation() << "  PRIME HOST: " << crs << std::endl << std::endl;
  }
}

/* Determines country settlement plan. If requestedPlan is not 0 (i.e. request has
   settlement plan override), then requested plan is returned (if available) or 0 is returned.
   If requestedPlan is 0, then the first available plan in SETTLEMENT_PLAN_TYPE_HIERARCHY is
   returned.
*/
CountrySettlementPlanInfo*
ValidatingCxrUtil::determineCountrySettlementPlan(TicketingCxrTrx& trx,
                                                  Diag191Collector* diag,
                                                  const SettlementPlanType* requestedPlan)
{
  return determineCountrySettlementPlanBase(
      trx.dataHandle(), trx.getCountry(), diag, requestedPlan);
}

/* Gets the list of settlement plans from the database for the given nation. And then
   either searches for the requested plan if it is given, or calls determinePlanFromHierarchy()
   to find the foremost available plan in the hierarchy.
*/
CountrySettlementPlanInfo*
ValidatingCxrUtil::determineCountrySettlementPlanBase(DataHandle& dh,
                                                      const NationCode& nation,
                                                      Diag191Collector* diag,
                                                      const SettlementPlanType* requestedPlan)
{
  const std::vector<CountrySettlementPlanInfo*>& cspiList = dh.getCountrySettlementPlans(nation);

  if (cspiList.empty())
  {
    if (diag)
    {
      *diag <<  "NO VALID SETTLEMENT METHOD FOUND FOR NATION " + nation << std::endl;
      diag->flushMsg();
    }
    if (requestedPlan && !requestedPlan->empty())
    {
      std::string msg = "INVALID SETTLEMENT METHOD FOR POINT OF SALE";
      throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR, msg.c_str());
    }
    else
    {
      std::string msg = "NO VALID SETTLEMENT METHOD FOUND FOR NATION " + nation;
      throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR, msg.c_str());
    }
    return nullptr;
  }

  CountrySettlementPlanInfo* cspi = nullptr;

    if (requestedPlan && !requestedPlan->empty())
    {
      if ( diag )
      {
        *diag << "SPECIFIED SETTLEMENT METHOD: " << *requestedPlan << std::endl;
      }

      cspi = getRequestedPlan(*requestedPlan, cspiList);
      if (!cspi)
      {
        if (diag)
        {
          *diag << "INVALID SETTLEMENT METHOD FOR POINT OF SALE" << std::endl;
          diag->flushMsg();
        }
        std::string msg = "INVALID SETTLEMENT METHOD FOR POINT OF SALE";
        throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR, msg.c_str());
      return nullptr;
      }
    }
    else
    {
      cspi = determinePlanFromHierarchy(cspiList);
      if (!cspi)
      {
        if (diag)
        {
          *diag << "NO VALID SETTLEMENT METHOD FOUND FOR NATION " + nation  << std::endl;
          diag->flushMsg();
        }
        std::string msg = "NO VALID SETTLEMENT METHOD FOUND FOR NATION " + nation;
        throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR, msg.c_str());
        return nullptr;
      }
    }

    if (diag && cspi)
      diag->displayCountrySettlementPlanInfo(*cspi);

  return cspi;
}

const std::vector<CountrySettlementPlanInfo*>&
ValidatingCxrUtil::getCountrySettlementPlanInfo(DataHandle& dataHandle,
    Diag191Collector* diag,
    const NationCode& nation,
    const SettlementPlanType* requestedPlan)
{
  const std::vector<CountrySettlementPlanInfo*>& cspiList =
    dataHandle.getCountrySettlementPlans(nation);
  if (cspiList.empty())
  {
    if (diag)
    {
      *diag << "NO VALID SETTLEMENT METHOD FOUND FOR NATION " + nation
            << std::endl;
      diag->flushMsg();
    }

    std::string msg;
    if (requestedPlan && !requestedPlan->empty())
    {
      msg = "INVALID SETTLEMENT METHOD FOR POINT OF SALE";
      throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR,
          msg.c_str());
    }
    else
    {
      msg = "NO VALID SETTLEMENT METHOD FOUND FOR NATION " + nation;
      throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR,
          msg.c_str());
    }
  }
  return cspiList;
}

CountrySettlementPlanInfo*
ValidatingCxrUtil::determineCountrySettlementPlanBase(const std::vector<CountrySettlementPlanInfo*>& cspiList,
                                                      Diag191Collector* diag,
                                                      const NationCode& nation,
                                                      const SettlementPlanType* requestedPlan,
                                                      std::string& msg)
{
  CountrySettlementPlanInfo* cspi = nullptr;
  if (requestedPlan && !requestedPlan->empty())
  {
    cspi = getRequestedPlan(*requestedPlan, cspiList);
    if (!cspi)
    {
      msg = "INVALID SETTLEMENT METHOD FOR POINT OF SALE";
      if (diag && *requestedPlan != "GTC")
        *diag << *requestedPlan << " " << msg << std::endl;
    }
  }
  else
  {
    cspi = determinePlanFromHierarchy(cspiList);
    if (!cspi)
    {
      msg = "NO VALID SETTLEMENT METHOD FOUND FOR NATION " + nation;
      if (diag)
        *diag << msg << std::endl;
    }
  }

  return cspi;
}

SettlementPlanType
ValidatingCxrUtil::getSettlementPlanFromHierarchy(const SpValidatingCxrGSADataMap& spGsaDataMap)
{
  if (spGsaDataMap.size()==1)
    return spGsaDataMap.begin()->first;

  for (const SettlementPlanType& spType : vcx::SP_HIERARCHY)
  {
    if (spType == "IPC")
      continue;

    auto it = spGsaDataMap.find(spType);
    if (it != spGsaDataMap.end() && it->second != nullptr)
      return spType;
  }

  return "";
}

CountrySettlementPlanInfo*
ValidatingCxrUtil::determinePlanFromHierarchy(
    const std::vector<CountrySettlementPlanInfo*>& cspiList)
{
  CountrySettlementPlanInfo* cspi = nullptr;
  for (const SettlementPlanType& spType : vcx::SP_HIERARCHY)
  {
    if (!spType.equalToConst("GTC") && !spType.equalToConst("IPC")) //Skip GTC/IPC
    {
      auto it = std::find_if(cspiList.begin(), cspiList.end(),
          [&spType] (CountrySettlementPlanInfo* ptr)->bool {
          return ptr->getSettlementPlanTypeCode() == spType;
          });

      if (it != cspiList.end())
      {
        cspi = *it;
        break;
      }
    }
  }
  return cspi;
}

SettlementPlanType
ValidatingCxrUtil::determinePlanFromHierarchy(const std::vector<SettlementPlanType>& spList)
{
  for (const SettlementPlanType& sp : vcx::SP_HIERARCHY)
    if (std::find(spList.begin(), spList.end(), sp) != spList.end())
      return sp;
  return spList.front();
}

CountrySettlementPlanInfo*
ValidatingCxrUtil::getRequestedPlan(const SettlementPlanType& sPlan,
                                    const std::vector<CountrySettlementPlanInfo*>& cspiList)
{
  for (CountrySettlementPlanInfo* cspi : cspiList)
  {
    if (cspi->getSettlementPlanTypeCode() == sPlan)
      return cspi;
  }
  return nullptr;
}

/* Given a marketing carrier, returns the corresponding validating carriers (VCs)
   If the marketing carrier is already VC, it returns it. Otherwise, it returns a
   list of swap carriers
*/
//@todo remove it with fallbackValidatingCxrMultiSp
void
ValidatingCxrUtil::getValidatingCxrsFromMarketingCxr(const Itin& itin,
                                                     const CarrierCode& marketingCarrier,
                                                     std::set<CarrierCode>& validatingCxrs)
{
  if (itin.validatingCxrGsaData()->hasCarrier(marketingCarrier))
    validatingCxrs.insert(marketingCarrier);
  else
    itin.getSwapCarriers(marketingCarrier, validatingCxrs);
}

/* Given a list of validating carriers, gets the corresponding marketing carriers */
void
ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(PricingTrx& trx,
                                                      const Itin& itin,
                                                      const std::vector<CarrierCode>& validatingCxrs,
                                                      std::vector<CarrierCode>& marketingCxrs)
{
  if (UNLIKELY(itin.hasNeutralValidatingCarrier()))
    return;

  std::vector<CarrierCode> allMarketingCarriers, participatingCarriers;

  if (UNLIKELY(!trx.getRequest()->validatingCarrier().empty()))
  {
    allMarketingCarriers.push_back(trx.getRequest()->validatingCarrier());
    getParticipatingCxrs(trx, itin, participatingCarriers);
  }
  else
  {
    getAllItinCarriers(trx, itin, allMarketingCarriers, participatingCarriers);
  }

  std::set<CarrierCode> tempMarketingCxrs;
  for (const CarrierCode& marketingCarrier : allMarketingCarriers)
  {
    if (std::find(validatingCxrs.begin(), validatingCxrs.end(), marketingCarrier) !=
        validatingCxrs.end())
      tempMarketingCxrs.insert(marketingCarrier); // marketing carrier already a VC
    else
    {
      std::set<CarrierCode> swapCarriers;
      if (itin.getSwapCarriers(marketingCarrier, swapCarriers))
      {
        for (const auto validatingCxr : validatingCxrs)
        {
          if (swapCarriers.find(validatingCxr) != swapCarriers.end())
          {
            tempMarketingCxrs.insert(marketingCarrier);
            break;
          }
        }
      }
    }
  }

  marketingCxrs.insert(marketingCxrs.end(), tempMarketingCxrs.begin(), tempMarketingCxrs.end());
}

/* Given a validating carrier, gets the corresponding marketing carrier(s) if it was swapped */
std::set<CarrierCode>
ValidatingCxrUtil::getMarketingCxrFromSwapCxr(const Itin& itin, const CarrierCode& carrier)
{
  std::set<CarrierCode> ret;

  if (itin.hasNeutralValidatingCarrier())
    return ret;

  std::map<CarrierCode, std::set<CarrierCode> >::const_iterator i, iEnd = itin.gsaSwapMap().end();

  for (i = itin.gsaSwapMap().begin(); i != iEnd; ++i)
  {
    const std::set<CarrierCode>& swapCarriers = i->second;
    if (swapCarriers.count(carrier))
      ret.insert(i->first);
  }

  return ret;
}

DateTime
ValidatingCxrUtil::determineTicketDate(PricingTrx& trx)
{
  DateTime localTime = DateTime::localTime();
  BaseExchangeTrx* baseExchangeTrx = dynamic_cast<BaseExchangeTrx*>(&trx);
  TicketingCxrTrx* tTrx = dynamic_cast<TicketingCxrTrx*>(&trx);
  if (LIKELY(!(baseExchangeTrx || tTrx)))
  {
    short utcOffset = adjustTicketDate(trx);
    localTime = trx.dataHandle().ticketDate().addSeconds(utcOffset * 60);
  }
  return localTime;
}

bool
ValidatingCxrUtil::checkNSPCarrierSMParticipation(PricingTrx& trx,
                                               Diag191Collector* diag,
                                               const CountrySettlementPlanInfo& cspInfo,
                                               vcx::ValidatingCxrData& vcxrData,
                                               const CarrierCode& validatingCarrier)
{
  if(cspInfo.getSettlementPlanTypeCode() == NO_SETTLEMENTPLAN && trx.getRequest()->isNSPCxr(validatingCarrier))
  {
    vcxrData.ticketType = getTicketingMethod(cspInfo);
    if (UNLIKELY(diag))
      *diag << "  CARRIER " << validatingCarrier << " SKIPPED FOR SETTLEMENT PLAN VALIDATIONS\n";
    return true;
  }
  else
    return checkCarrierSMParticipation(trx, diag, cspInfo, vcxrData, validatingCarrier);
}

bool
ValidatingCxrUtil::checkCarrierSMParticipation(PricingTrx& trx,
                                               Diag191Collector* diag,
                                               const CountrySettlementPlanInfo& cspInfo,
                                               vcx::ValidatingCxrData& vcxrData,
                                               const CarrierCode& validatingCarrier)
{
  CrsCode crs = trx.getRequest()->ticketingAgent()->cxrCode();
  DataHandle dataHandle(determineTicketDate(trx));
  AirlineCountrySettlementPlanInfo* acspi =
    getAirlineCountrySettlementPlanInfo (cspInfo,
        diag,
        validatingCarrier,
        crs,
        dataHandle);

  if (acspi == nullptr)
  {
    if (UNLIKELY(diag))
      *diag << "  CARRIER " << validatingCarrier << " IS NOT ADDED AS VALIDATING CARRIER\n";
    return false;
  }

  vcxrData.ticketType = getTicketingMethod(cspInfo, acspi);
  return true;
}

bool
ValidatingCxrUtil::isPaperTicketConflict(const PricingTrx& trx,
    Diag191Collector* diag,
    vcx::ValidatingCxrData& vcxrData,
    const CarrierCode& validatingCarrier)
{
  const vcx::TicketType reqTktType =
    (trx.getRequest()->isElectronicTicket() ? vcx::ETKT_REQ : vcx::PAPER_TKT_REQ);

  if (UNLIKELY(isPaperTicketOverrideConflict(vcxrData.ticketType, reqTktType)))
  {
    if (diag)
      *diag << "CARRIER " << validatingCarrier
        << " IS NOT ADDED AS VALIDATING CARRIER\n"
        << "BECAUSE OF PAPER TICKET OVERRIDE CONFLICT\n";
    return true;
  }
  return false;
}

void
ValidatingCxrUtil::pushIfNotExists(std::vector<CarrierCode>& v, const CarrierCode& carrier)
{
  if (!carrier.empty() && (std::find(v.begin(), v.end(), carrier) == v.end()))
    v.push_back(carrier);
}

/* Returns all marketing and participating (both marketing and operating) carriers
   in an itin
*/
void
ValidatingCxrUtil::getAllItinCarriers(const PricingTrx& trx,
                                      const Itin& itin,
                                      std::vector<CarrierCode>& marketingCarriers,
                                      std::vector<CarrierCode>& participatingCarriers)
{
  for (TravelSeg* tSeg : itin.travelSeg())
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tSeg);
    if (UNLIKELY( trx.excTrxType() != PricingTrx::NOT_EXC_TRX && !tSeg->unflown()))
    {
      continue;
    }
    if (airSeg)
    {
      pushIfNotExists(marketingCarriers, airSeg->marketingCarrierCode());
      pushIfNotExists(participatingCarriers, airSeg->marketingCarrierCode());
      pushIfNotExists(participatingCarriers, airSeg->operatingCarrierCode());
    }
  }
}

void
ValidatingCxrUtil::getParticipatingCxrs(const PricingTrx& trx,
                                        const Itin& itin,
                                        std::vector<CarrierCode>& participatingCarriers)
{
  for (TravelSeg* tSeg : itin.travelSeg())
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tSeg);
    if ( trx.excTrxType() != PricingTrx::NOT_EXC_TRX && !tSeg->unflown())
    {
      if (airSeg && trx.getRequest()->validatingCarrier() == airSeg->marketingCarrierCode())
      {
         pushIfNotExists(participatingCarriers, airSeg->marketingCarrierCode());
      }
      continue;
    }
    if (airSeg)
    {
      pushIfNotExists(participatingCarriers, airSeg->marketingCarrierCode());
      pushIfNotExists(participatingCarriers, airSeg->operatingCarrierCode());
    }
  }
}

void
ValidatingCxrUtil::getMarketingItinCxrs(const Itin& itin,
                                        std::vector<CarrierCode>& marketingCxrs)
{
  for (TravelSeg* tSeg : itin.travelSeg())
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tSeg);
    if ( airSeg )
    {
      pushIfNotExists(marketingCxrs, airSeg->marketingCarrierCode());
    }
  }
}

bool
ValidatingCxrUtil::isPassNSPInterlineAgreement(PricingTrx& trx,
                                                 Diag191Collector* diag,
                                                 std::string& errMsg,
                                                 const NationCode& countryCode,
                                                 vcx::ValidatingCxrData& vcxrData,
                                                 const CarrierCode& vcxr,
                                                 const std::vector<CarrierCode>& participatingCarriers,
                                                 const std::vector<CarrierCode>& marketingItinCxrs,
                                                 const CarrierCode& swappedFor,
                                                 const bool isNeutral)
{

  bool hasInterlineAgmtInNSPCntry = false;
  for (const NationCode nspCountryCode : trx.getRequest()->spvCntyCode())
  {
    if(!isPassInterlineAgreement(trx,
        diag,
        nspCountryCode,
        vcxrData,
        vcxr,
        participatingCarriers,
        marketingItinCxrs))
    {
        errMsg = buildValidationCxrMsg(vcxrData.interlineStatusCode,
                                      vcxr,
                                      vcxrData.interlineFailedCxr);
    }
    else
    {
      if(!hasInterlineAgmtInNSPCntry)
        hasInterlineAgmtInNSPCntry = true;
      vcxrData.interlineValidCountries.push_back(nspCountryCode);
    }
  }
  return hasInterlineAgmtInNSPCntry;

}

void
ValidatingCxrUtil::addParticipatingCxr(vcx::ValidatingCxrData& vcxrData,
                                       const CarrierCode& participatingCarrier)
{
  bool found = false;
  for(auto it: vcxrData.participatingCxrs)
  {
    if(it.cxrName == participatingCarrier)
    {
      found = true;
      break;
    }
  }
  if(!found)
    vcxrData.participatingCxrs.push_back(participatingCarrier);
}
// Returns true if a carrier passes all interline agreements of participating carriers.
// If true, updates itin.validatingCarriersData map with the validating carrier data.
bool ValidatingCxrUtil::isPassInterlineAgreement(PricingTrx& trx,
                                                 Diag191Collector* diag,
                                                 const NationCode& countryCode,
                                                 vcx::ValidatingCxrData& vcxrData,
                                                 const CarrierCode& vcxr,
                                                 const std::vector<CarrierCode>& participatingCarriers,
                                                 const std::vector<CarrierCode>& marketingItinCxrs,
                                                 const CarrierCode& swappedFor,
                                                 const bool isNeutral)
{
  vcx::Pos pos(countryCode, trx.getRequest()->ticketingAgent()->cxrCode());

  bool isVcxrPartOfItin = false;

  isVcxrPartOfItin = std::find(marketingItinCxrs.begin(), marketingItinCxrs.end(), vcxr) != marketingItinCxrs.end();
  if ( isNeutral && isVcxrPartOfItin )
    return false;

  for (const CarrierCode& participatingCarrier : participatingCarriers)
  {
    if (vcxr == participatingCarrier)
      continue; // no need to check IA with itself

    if (participatingCarrier == swappedFor)
      continue; // no need to check IA with carrier and its swap

    vcx::ParticipatingCxr pCxr(participatingCarrier);
    if(!fallback::fallbackNonBSPVcxrPhase1(&trx) || trx.overrideFallbackValidationCXRMultiSP())
      addParticipatingCxr(vcxrData, participatingCarrier);
    else
      vcxrData.participatingCxrs.push_back(pCxr);
  }

  bool onlyCheckAgreementExistence = false;
  if (UNLIKELY( trx.onlyCheckAgreementExistence() ))
    onlyCheckAgreementExistence = true;

  const vcx::TicketType requestedTicketType = (trx.getRequest()->isElectronicTicket() ?
                                           vcx::ETKT_REQ : vcx::PAPER_TKT_REQ);
  DataHandle dataHandle( determineTicketDate(trx));

  if ( !vcxrData.participatingCxrs.empty()                 &&
       !validateInterlineAgreements(dataHandle,
                                    diag,
                                    pos,
                                    vcxr,
                                    vcxrData,
                                    isVcxrPartOfItin,
                                    requestedTicketType,
                                    onlyCheckAgreementExistence) )
  {
    return false;
  }

  return true;
}

/* Hash key keeps marketing and operating carriers so the next itin with the same
   marketing and operating carriers does not need to process validating carriers again.
   Hash key string is "marketing-cxrs|operating-cxr" with duplicates eliminated.
*/
std::string
ValidatingCxrUtil::createHashString(const std::vector<CarrierCode>& marketingCarriers,
                                    const std::vector<CarrierCode>& participatingCarriers)
{
  std::ostringstream oss;

  std::set<CarrierCode> sortedMCs(marketingCarriers.begin(), marketingCarriers.end());
  for (const CarrierCode& carrier : sortedMCs)
    oss << carrier;

  oss << "|";

  std::set<CarrierCode> sortedPCs(participatingCarriers.begin(), participatingCarriers.end());
  std::set<CarrierCode> sortedOPs;

  std::set_difference(sortedPCs.begin(), sortedPCs.end(), sortedMCs.begin(), sortedMCs.end(),
                      std::inserter(sortedOPs, sortedOPs.begin()));

  for (const CarrierCode& carrier : sortedOPs)
    oss << carrier;

  return oss.str();
}


bool
ValidatingCxrUtil::isAddNSPValidatingCxr(PricingTrx& trx,
                                    Diag191Collector* diag,
                                    const CountrySettlementPlanInfo& cspInfo,
                                    vcx::ValidatingCxrData& vcxrData,
                                    ValidatingCxrGSAData*& valCxrGsaData,
                                    const CarrierCode& carrier,
                                    const bool isNeutral)
{
  if(cspInfo.getSettlementPlanTypeCode() == NO_SETTLEMENTPLAN &&
      trx.getRequest()->spvInd() == tse::spValidator::noSMV_noIEV)
  {
    if (UNLIKELY(diag))
      *diag << "  CARRIER " << carrier << " SKIPPED FOR IET VALIDATIONS\n";
    addValidatingCxr(trx, diag, vcxrData, valCxrGsaData, carrier);
    return true;
  }
  return false;
}

void
ValidatingCxrUtil::addValidatingCxr(PricingTrx& pricingTrx,
                                    Diag191Collector* diag,
                                    vcx::ValidatingCxrData& vcxrData,
                                    ValidatingCxrGSAData*& valCxrGsaData,
                                    const CarrierCode& carrier,
                                    const bool isNeutral)
{
  if (!valCxrGsaData)
    pricingTrx.dataHandle().get(valCxrGsaData);

  valCxrGsaData->isNeutralValCxr() = isNeutral;

  valCxrGsaData->validatingCarriersData()[carrier] = vcxrData;

  if (UNLIKELY(diag))
  {
    *diag << (isNeutral ? "\nNEUTRAL " : "\n");
    *diag << "  CARRIER " << carrier << " IS ADDED AS A VALIDATING CARRIER" <<std::endl <<std::endl;
  }
}

/* If no valid validating carrier is found, searches and validates
   neutral carriers for the country
*/
void
ValidatingCxrUtil::processNeutralCarriers(PricingTrx& trx,
                                          Diag191Collector* diag,
                                          const CountrySettlementPlanInfo& cspInfo,
                                          ValidatingCxrGSAData*& valCxrGsaData,
                                          const std::vector<CarrierCode>& participatingCarriers,
                                          const std::vector<CarrierCode>& marketingCarriers)
{
  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    //@todo Move this outside and include check for override inside canProcessNVCSwap
    if (!canProcessNVCSwap(trx, cspInfo.getSettlementPlanTypeCode()))
      return;
  }
  else if ( trx.isSkipNeutral() )
      return;

  if (diag)
    *diag << "ERROR: NO VALIDATING CARRIER FOUND. TRYING NEUTRAL CARRIERS.\n";

  const std::vector<NeutralValidatingAirlineInfo*> neutralCarriers =
    trx.dataHandle().getNeutralValidatingAirlines(
        cspInfo.getCountryCode(),
        trx.getRequest()->ticketingAgent()->cxrCode(),
        cspInfo.getSettlementPlanTypeCode());

  if (neutralCarriers.empty())
  {
    if (diag)
      *diag << "NO NEUTRAL VALIDATING CARRIERS IN COUNTRY "
        << cspInfo.getCountryCode() << "\n";
    return;
  }

  for (const NeutralValidatingAirlineInfo* nvai : neutralCarriers)
  {
    const CarrierCode& neutralCxr = nvai->getAirline();

    vcx::ValidatingCxrData vcxrData;

    bool isValidVC = fallback::fallbackNonPreferredVC(&trx)?true:!isNonPreferredVC(trx, neutralCxr);
    if(isValidVC)
    {
      isValidVC = checkCarrierSMParticipation(trx, diag, cspInfo, vcxrData, neutralCxr);
    }

    if (isValidVC &&
        !isPaperTicketConflict(trx, diag, vcxrData, neutralCxr) &&
        isPassInterlineAgreement(trx,
          diag,
          cspInfo.getCountryCode(),
          vcxrData,
          neutralCxr,
          participatingCarriers,
          marketingCarriers,
          "",
          true))
    {
      addValidatingCxr(trx, diag, vcxrData, valCxrGsaData, neutralCxr, true);
    }
  }
}


/* Returns true if the vector vecFirst1, contains all the elements in the vecFirst2).*/
bool
ValidatingCxrUtil::stdIncludesWrapper(const std::vector<CarrierCode>& vecFirst1, const std::vector<CarrierCode>& vecFirst2)
{
  std::set<CarrierCode> setFirst1(vecFirst1.begin(), vecFirst1.end());
  std::set<CarrierCode> setFirst2(vecFirst2.begin(), vecFirst2.end());

  return(std::includes(setFirst1.begin(), setFirst1.end(), setFirst2.begin(), setFirst2.end()));
}

/* For each itin, first calculates the default validating carrier, and then processes all
   marketing carriers to see if they can be a validating carrier. If not, it searches for
   GSA swaps. If still not found, calls for neutral VC processing
*/
ValidatingCxrGSAData*
ValidatingCxrUtil::getValidatingCxrList(PricingTrx& trx,
                                        Diag191Collector* diag,
                                        const CountrySettlementPlanInfo& cspInfo,
                                        const std::vector<CarrierCode>& potentialVcxrs,
                                        const std::vector<CarrierCode>& participatingCxrs,
                                        std::string& errMsg,
                                        const std::vector<CarrierCode>& marketingItinCxrs)
{
  if (UNLIKELY(checkMarketingCarrierMissing(marketingItinCxrs)))
  {
    errMsg = "OPEN SEG ** NOT ALLOWED - CANCEL/REBOOK";
    return nullptr;
  }

  if (UNLIKELY( isRexPricing(trx) ))
  {
    return getValidatingCxrListForRexTrx(
        trx, diag, cspInfo, potentialVcxrs, participatingCxrs, errMsg, marketingItinCxrs);
  }

  ValidatingCxrGSAData* valCxrGsaData = nullptr;
  std::set<CarrierCode> carriersToBeSwapped, failedCarriers;

  for (const CarrierCode& carrier : potentialVcxrs)
  {
    vcx::ValidatingCxrData vcxrData;
    bool isValidVC = fallback::fallbackNonPreferredVC(&trx)?true:!isNonPreferredVC(trx, carrier);
    if(isValidVC)
    {
      if(!fallback::fallbackNonBSPVcxrPhase1(&trx) || trx.overrideFallbackValidationCXRMultiSP())
      {
        if((cspInfo.getSettlementPlanTypeCode() != NO_SETTLEMENTPLAN &&
           trx.getRequest()->isNSPCxr(carrier)) ||
           (cspInfo.getSettlementPlanTypeCode() == NO_SETTLEMENTPLAN &&
           !trx.getRequest()->isNSPCxr(carrier))
           )
          continue;
        isValidVC = checkNSPCarrierSMParticipation(trx, diag, cspInfo, vcxrData, carrier);
      }
      else
        isValidVC = checkCarrierSMParticipation(trx, diag, cspInfo, vcxrData, carrier);
    }
    if (!isValidVC)
    {
      if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
      {
        if (canProcessGSASwap(trx, cspInfo.getSettlementPlanTypeCode()))
          carriersToBeSwapped.insert(carrier);
      }
      else
        carriersToBeSwapped.insert(carrier);
    }
    else
    {
      if(!fallback::fallbackNonBSPVcxrPhase1(&trx) || trx.overrideFallbackValidationCXRMultiSP())
      {
        if(isAddNSPValidatingCxr(trx, diag, cspInfo, vcxrData, valCxrGsaData, carrier))
          continue;
      }
      if (checkPaperAndInterlineAgmt(trx,
          diag,
          cspInfo,
          vcxrData,
          valCxrGsaData,
          carrier,
          participatingCxrs,
          marketingItinCxrs,
          errMsg))
        addValidatingCxr(trx, diag, vcxrData, valCxrGsaData, carrier);
      else
      {
        failedCarriers.insert(carrier);
        if (UNLIKELY(diag))
          *diag << "  CARRIER " << carrier << " IS NOT ADDED AS A VALIDATING CARRIER\n";
      }
    }
  }
  if(!fallback::fallbackNonBSPVcxrPhase1(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    switch(trx.getRequest()->spvInd())
    {
      case tse::spValidator::noSMV_noIEV:
      case tse::spValidator::noSMV_IEV:
        if(trx.getRequest()->spvCxrsCode().empty() ||
            stdIncludesWrapper(trx.getRequest()->spvCxrsCode(), potentialVcxrs) ||
            cspInfo.getSettlementPlanTypeCode() == NO_SETTLEMENTPLAN)
          return valCxrGsaData;
      case tse::spValidator::SMV_IEV:
        break;
    }
  }

  // To find paper conflict among GSAs
  bool anyCxrHasGsa = false;
  std::vector<vcx::ValidationStatus> gsaStatusList;
  gsaStatusList.reserve(carriersToBeSwapped.size());
  for (const CarrierCode& marketingCarrier : carriersToBeSwapped)
  {
    if (fallback::fallbackValidatingCxrMultiSp(&trx) && !trx.overrideFallbackValidationCXRMultiSP()
        && trx.isSkipGsa())
      break;

    std::set<CarrierCode> swapCarriers;
    getGenSalesAgents(trx.dataHandle(),
        diag,
        marketingCarrier,
        cspInfo.getCountryCode(),
        trx.getRequest()->ticketingAgent()->cxrCode(),
        cspInfo.getSettlementPlanTypeCode(),
        swapCarriers);

    for (const CarrierCode& swapCxr : swapCarriers)
    {
      if (failedCarriers.count(swapCxr))
        continue; // already failed

      // Check if it is already added as marketing carrier. If not, attempt to add it
      if (valCxrGsaData && valCxrGsaData->hasCarrier(swapCxr))
      {
        valCxrGsaData->gsaSwapMap()[marketingCarrier].insert(swapCxr);
        continue;
      }

      vcx::ValidatingCxrData vcxrData;

      bool isValidVC = fallback::fallbackNonPreferredVC(&trx)?true:!isNonPreferredVC(trx, swapCxr);
      if(isValidVC)
      {
        isValidVC = checkCarrierSMParticipation(trx, diag, cspInfo, vcxrData, swapCxr);
      }
      if (!isValidVC)
      {
        failedCarriers.insert(swapCxr);
      }
      else
      {
        if (checkPaperAndInterlineAgmt(trx,
              diag,
              cspInfo,
              vcxrData,
              valCxrGsaData,
              swapCxr,
              participatingCxrs,
              marketingItinCxrs,
              errMsg,
              marketingCarrier))
        {
          addValidatingCxr(trx, diag, vcxrData, valCxrGsaData, swapCxr);
          valCxrGsaData->gsaSwapMap()[marketingCarrier].insert(swapCxr);
        }
        else
        {
          failedCarriers.insert(swapCxr);
          gsaStatusList.push_back(vcxrData.interlineStatusCode);
        }
      }
    }

    if (!anyCxrHasGsa && !swapCarriers.empty())
      anyCxrHasGsa = true;
  }

  // For Validating carrier overwrite do not process Neutral
  if (!valCxrGsaData)
  {
    if (trx.getRequest()->validatingCarrier().empty())
    {
      processNeutralCarriers(trx,
          diag,
          cspInfo,
          valCxrGsaData,
          participatingCxrs,
          marketingItinCxrs);

      if (!valCxrGsaData)
        errMsg = "NO VALID TICKETING AGREEMENTS FOUND";
    }
    else
    {
      setErrorMsg(trx, diag, anyCxrHasGsa, gsaStatusList, errMsg);
    }
  }

  return valCxrGsaData;
}

/*
 * Check for paper and interline agmt. It also set errMsg text.
 */
bool
ValidatingCxrUtil::checkPaperAndInterlineAgmt(PricingTrx& trx,
                                              Diag191Collector* diag,
                                              const CountrySettlementPlanInfo& cspInfo,
                                              vcx::ValidatingCxrData& vcxrData,
                                              ValidatingCxrGSAData*& valCxrGsaData,
                                              const CarrierCode& cxr,
                                              const std::vector<CarrierCode>& participatingCxrs,
                                              const std::vector<CarrierCode>& marketingItinCxrs,
                                              std::string& errMsg,
                                              const CarrierCode& mktCxr)
{
  bool hasInterlineAgmt = false;
  const bool isPaperConflict = isPaperTicketConflict(trx, diag, vcxrData, cxr);

  if (LIKELY(!isPaperConflict))
  {
    //if SP is NSP and spvInd=noSMV_IEV do below
    if((!fallback::fallbackNonBSPVcxrPhase1(&trx) || trx.overrideFallbackValidationCXRMultiSP()) &&
        cspInfo.getSettlementPlanTypeCode() == NO_SETTLEMENTPLAN &&
        trx.getRequest()->spvInd() == tse::spValidator::noSMV_IEV &&
        trx.getRequest()->isNSPCxr(cxr))
    {
      hasInterlineAgmt = isPassNSPInterlineAgreement(trx,
                    diag,
                    errMsg,
                    cspInfo.getCountryCode(),
                    vcxrData,
                    cxr,
                    participatingCxrs,
                    marketingItinCxrs,
                    mktCxr);
    }
    else
    {
      hasInterlineAgmt = isPassInterlineAgreement(trx,
              diag,
              cspInfo.getCountryCode(),
              vcxrData,
              cxr,
              participatingCxrs,
              marketingItinCxrs,
              mktCxr);

      if (!hasInterlineAgmt &&
          !trx.getRequest()->validatingCarrier().empty()) //carrier override
      {
        errMsg = buildValidationCxrMsg(vcxrData.interlineStatusCode,
            cxr,
            vcxrData.interlineFailedCxr);
      }
    }
  }
  else
  {
    errMsg = "PAPER TICKET NOT PERMITTED";
    vcxrData.interlineStatusCode = vcx::PAPER_TKT_OVERRIDE_ERR;
  }

  return (!isPaperConflict && hasInterlineAgmt);
}


// For ExchangeTrx type AM we only check for settlement participation
// For Other types (S96=!AM), we check for settlement participation,
//    paper conflict and interline agreement
ValidatingCxrGSAData*
ValidatingCxrUtil::getValidatingCxrListForRexTrx(PricingTrx& trx,
                                                  Diag191Collector* diag,
                                                  const CountrySettlementPlanInfo& cspInfo,
                                                  const std::vector<CarrierCode>& mCxrs /*marketing carriers */,
                                                  const std::vector<CarrierCode>& pCxrs /*participating carriers */,
                                                  std::string& errMsg,
                                                  const std::vector<CarrierCode>& marketingItinCxrs)
{
  ValidatingCxrGSAData* valCxrGsaData = nullptr;
  if (!mCxrs.empty())
  {
    const CarrierCode& vcxr = mCxrs.front();
    const bool isExchangeTrxAM = trx.excTrxType() == PricingTrx::PORT_EXC_TRX &&
      static_cast<ExchangePricingTrx&>(trx).reqType() == AGENT_PRICING_MASK;

    vcx::ValidatingCxrData vcxrData;
    bool isCxrParticipates;
    if(!fallback::fallbackNonBSPVcxrPhase1(&trx) || trx.overrideFallbackValidationCXRMultiSP())
    {
      if(cspInfo.getSettlementPlanTypeCode() == NO_SETTLEMENTPLAN && trx.getRequest()->isNSPCxr(vcxr))
      {
        vcxrData.ticketType = getTicketingMethod(cspInfo);
        isCxrParticipates = true;
      }
      else
        isCxrParticipates = checkCarrierSMParticipation(trx,
            diag,
            cspInfo,
            vcxrData,
            vcxr);
    }
    else
      isCxrParticipates = checkCarrierSMParticipation(trx,
          diag,
          cspInfo,
          vcxrData,
          vcxr);

    if (isCxrParticipates && !isExchangeTrxAM )
    {
      if((!fallback::fallbackNonBSPVcxrPhase1(&trx) || trx.overrideFallbackValidationCXRMultiSP()) &&
                cspInfo.getSettlementPlanTypeCode() == NO_SETTLEMENTPLAN &&
                trx.getRequest()->spvInd() == tse::spValidator::noSMV_noIEV)
      {
        addValidatingCxr(trx, diag, vcxrData, valCxrGsaData, vcxr);
        return valCxrGsaData;
      }
      if (checkPaperAndInterlineAgmt(trx,
            diag,
            cspInfo,
            vcxrData,
            valCxrGsaData,
            vcxr,
            pCxrs,
            marketingItinCxrs,
            errMsg))
        addValidatingCxr(trx, diag, vcxrData, valCxrGsaData, vcxr);
    }
    else if (isCxrParticipates)/*S96=AM*/
    {
      addValidatingCxr(trx, diag, vcxrData, valCxrGsaData, vcxr);
    }
    else
    {
      errMsg = vcxr;
      errMsg += " NOT VALID FOR SETTLEMENT METHOD";
    }
  }

  return valCxrGsaData;
}

bool
ValidatingCxrUtil::validateSwapCxrInterlineAgreement(DataHandle& dh,
                                               const NationCode& country,
                                               const CrsCode& primeHost,
                                               const CarrierCode& vcxr,
                                               const CarrierCode& swapCxr)
{
  std::vector<AirlineInterlineAgreementInfo*> aiaList;
  getAirlineInterlineAgreements(dh, swapCxr, country, primeHost, aiaList);
  for (AirlineInterlineAgreementInfo* aia : aiaList)
  {
    if (aia->getParticipatingCarrier() == vcxr &&
        aia->isThirdPartyAgreement())
      return true;
  }
  return false;
}

bool
ValidatingCxrUtil::validateInterlineAgreements(DataHandle& dh,
                                               Diag191Collector* diag,
                                               const vcx::Pos& pos,
                                               const CarrierCode& vcxr,
                                               vcx::ValidatingCxrData& vcxrData,
                                               bool isVcxrPartOfItin,
                                               vcx::TicketType requestedTicketType,
                                               bool onlyCheckAgreementExistence)
{
  const vcx::ValidationStatus status =
      checkInterlineAgreements(dh, pos, vcxr, vcxrData, isVcxrPartOfItin, requestedTicketType, onlyCheckAgreementExistence);
  if (UNLIKELY(diag))
  {
    std::string failureText;
    if (vcx::VALID_MSG != status)
    {
      failureText = buildValidationCxrMsg(status, vcxr, vcxrData.interlineFailedCxr);
    }

    diag->displayInterlineAgreements(
        vcxr, vcxrData, isVcxrPartOfItin, requestedTicketType, failureText, &pos);
  }

  vcxrData.interlineStatusCode = status;
  return vcx::VALID_MSG == status;
}

std::string
ValidatingCxrUtil::buildValidationCxrMsg(vcx::ValidationStatus status,
                                         const CarrierCode& vcxr,
                                         const CarrierCode& cxr)
{
  std::stringstream msg;
  switch (status)
  {
  case vcx::VALID_MSG:
  case vcx::VALID_OVERRIDE:
    msg << getValidationCxrMsg(status) << vcxr;
    break;
  case vcx::NO_VALID_TKT_AGMT_FOUND:
    msg << getValidationCxrMsg(status);
    break;
  case vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH:
    msg << vcxr << getValidationCxrMsg(status) << cxr;
    break;
  case vcx::VALID_SINGLE_GSA:
  {
    std::string temp( getValidationCxrMsg( status ) );
    boost::algorithm::replace_first( temp, "|XX|", vcxr  );
    boost::algorithm::replace_first( temp, "|ZZ|", cxr );
    msg.str( temp );
    break;
  }
  default:
    msg << "UNKNOWN STATUS";
    break;
  }
  return msg.str();
}

std::string
ValidatingCxrUtil::buildValidationCxrMsg(const Itin& itin,
                                         vcx::ValidationStatus status,
                                         const std::vector<CarrierCode>& valCxr)
{
  std::stringstream msg;
  std::vector<CarrierCode>& vcCxr = const_cast<std::vector<CarrierCode>&>(valCxr);

  switch (status)
  {
  case vcx::ALTERNATE_CXR:
  {
    msg << getValidationCxrMsg(status);
    if (vcCxr.size() > 1)
    {
       getAlternateCxrInOrder(itin, vcCxr);
    }
    break;
  }
  case vcx::OPTIONAL_CXR:
  {
    msg << getValidationCxrMsg(status);
    if(!vcCxr.empty())
       std::sort(vcCxr.begin(), vcCxr.end());
    break;
  }
  default:
    msg << "UNKNOWN STATUS";
    break;
  }
  for (CarrierCode cxr : vcCxr)
  {
    msg << cxr << " ";
  }
  return msg.str();
}

void
ValidatingCxrUtil::getAlternateCxrInOrder(const Itin& itin,
                                          std::vector<CarrierCode>& validatingCxrs)
{
  std::vector<CarrierCode> marketingCxrs;
  getMarketingCarriersInItinOrder(itin, marketingCxrs);

  // look for marketing carrier in the validating carrier vector
  std::vector<CarrierCode> altValCxrs;
  for (CarrierCode mktCxr : marketingCxrs)
  {
    if (isAlternateValCxr(mktCxr, validatingCxrs, altValCxrs))
    {
      // marketing carrier added to the list in itin order
      altValCxrs.push_back(mktCxr);
    }
    else
    {
      std::set<CarrierCode> swapCxrs;
      if (itin.getSwapCarriers(mktCxr, swapCxrs))
      {
        std::set<CarrierCode> tmpAltCxrs;
        for (const CarrierCode swapCxr : swapCxrs)
        {
          if (isAlternateValCxr(swapCxr, validatingCxrs, altValCxrs))
            tmpAltCxrs.insert(swapCxr);
        }

        if (!tmpAltCxrs.empty())
          altValCxrs.insert(altValCxrs.end(), tmpAltCxrs.begin(), tmpAltCxrs.end());
      }
    }
  }

  if(!altValCxrs.empty())
    validatingCxrs.swap(altValCxrs);
}

void
ValidatingCxrUtil::getAlternateCxrInOrder(
    const Itin& itin,
    const std::vector<CarrierCode>& mktCxrs,
    std::vector<CarrierCode>& cxrs,
    const SettlementPlanType* sp)
{
  std::vector<CarrierCode> altCxrs;
  std::set<CarrierCode> swapCxrs, tmpAltCxrs;
  for (const CarrierCode& mktCxr : mktCxrs)
  {
    if (isAlternateValCxr(mktCxr, cxrs, altCxrs))
      altCxrs.push_back(mktCxr);
    else if (sp ? itin.getSwapCarriers(mktCxr, swapCxrs, *sp) : itin.getSwapCarriers(mktCxr, swapCxrs))
    {
      tmpAltCxrs.clear();
      for (const CarrierCode swapCxr : swapCxrs)
      {
        if (isAlternateValCxr(swapCxr, cxrs, altCxrs))
          tmpAltCxrs.insert(swapCxr);
      }

      if (!tmpAltCxrs.empty())
        altCxrs.insert(altCxrs.end(), tmpAltCxrs.begin(), tmpAltCxrs.end());
    }
  }

  if(!altCxrs.empty())
    cxrs.swap(altCxrs);
}

void
ValidatingCxrUtil::getMarketingCarriersInItinOrder(
    const Itin& itin,
    std::vector<CarrierCode>& marketingCxr)
{
  for (TravelSeg* tSeg : itin.travelSeg())
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tSeg);
    if (airSeg)
    {
      if( std::find(marketingCxr.begin(), marketingCxr.end(), airSeg->marketingCarrierCode()) ==
          marketingCxr.end() )
        marketingCxr.push_back(airSeg->marketingCarrierCode());
    }
  }
}

vcx::ValidationStatus
ValidatingCxrUtil::checkInterlineAgreements(DataHandle& dh,
                                            const vcx::Pos& pos,
                                            const CarrierCode& vcxr,
                                            vcx::ValidatingCxrData& vcxrData,
                                            bool isVcxrPartOfItin,
                                            vcx::TicketType requestedTicketType,
                                            bool onlyCheckAgreementExistence)
{
  std::vector<vcx::ParticipatingCxr>& pCxrs = vcxrData.participatingCxrs;
  if (UNLIKELY(pCxrs.empty() || (pCxrs.size() == 1 && vcxr==pCxrs.front().cxrName)))
    return vcx::VALID_MSG;

  std::vector<AirlineInterlineAgreementInfo*> aiaList;
  getAirlineInterlineAgreements(dh, vcxr, pos.country, pos.primeHost, aiaList);
  if (aiaList.empty())
    return vcx::NO_VALID_TKT_AGMT_FOUND;

  vcx::ValidationStatus status = vcx::VALID_MSG;
  for (vcx::ParticipatingCxr& pcx : pCxrs)
  {
    if (UNLIKELY( pcx.cxrName == vcxr ))
      continue;

    AirlineInterlineAgreementInfo* aia = findAgreement(aiaList, pcx.cxrName);
    if (nullptr == aia)
    {
      status = vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH;
      vcxrData.interlineFailedCxr = pcx.cxrName;
      break;
    }

    pcx.agmtType = vcx::getITAgreementType(aia->getAgreementTypeCode());

    if (UNLIKELY( onlyCheckAgreementExistence ))
    {
      status = vcx::VALID_MSG;
      continue;
    }

    status = checkAgreement(*aia, vcxrData.ticketType, requestedTicketType, isVcxrPartOfItin);
    if (status != vcx::VALID_MSG)
    {
      vcxrData.interlineFailedCxr = pcx.cxrName;
      break;
    }
  }
  return status;
}

vcx::ValidationStatus
ValidatingCxrUtil::checkAgreement(const AirlineInterlineAgreementInfo& aia,
                                  vcx::TicketType vcxrTicketType,
                                  vcx::TicketType requestedTicketType,
                                  bool isVcxrPartOfItin)
{
  if (isTicketTypeElectronic(requestedTicketType) && aia.isStandardAgreement() && !isVcxrPartOfItin)
  {
    return vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH;
  }

  if (isTicketTypeElectronic(requestedTicketType) && aia.isPaperOnlyAgreement())
  {
    return vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH;
  }

  return vcx::VALID_MSG;
}

AirlineInterlineAgreementInfo*
ValidatingCxrUtil::findAgreement(const std::vector<AirlineInterlineAgreementInfo*>& aiaList,
                                 const CarrierCode& airline)
{
  for (AirlineInterlineAgreementInfo* aia : aiaList)
  {
    if (aia->getParticipatingCarrier() == airline)
      return aia;
  }

  return nullptr;
}

bool
ValidatingCxrUtil::isPaperTicketOverrideConflict(vcx::TicketType vcxrTicketingType,
                                                 vcx::TicketType requestedTicketingType)
{
  return isElectronicTicketRequired(vcxrTicketingType) && isTicketTypePaper(requestedTicketingType);
}

void
ValidatingCxrUtil::getAllSopMarketingCarriers(FlightFinderTrx& trx,
                                              std::vector<CarrierCode>& marketingCarriers)
{
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  for (ShoppingTrx::Leg& leg : legs)
  {
    for (ShoppingTrx::SchedulingOption& sop : leg.sop())
    {
      for (TravelSeg* tSeg : sop.itin()->travelSeg())
      {
        AirSeg* airSeg = tSeg->toAirSeg();
        if (airSeg)
        {
          pushIfNotExists(marketingCarriers, airSeg->marketingCarrierCode());
        }
      }
    }
  }
}

ValidatingCxrGSAData*
ValidatingCxrUtil::validateSettlementMethodAndGsa(PricingTrx& trx,
                                                  Diag191Collector* diag,
                                                  const CountrySettlementPlanInfo& cspInfo,
                                                  const std::vector<CarrierCode>& marketingCarriers)
{
  ValidatingCxrGSAData* valCxrGsaData = nullptr;
  std::set<CarrierCode> carriersToBeSwapped, failedCarriers;
  for (const CarrierCode& carrier : marketingCarriers)
  {
    vcx::ValidatingCxrData vcxrData;
    const bool isCxrParticipates =
      checkCarrierSMParticipation(trx, diag, cspInfo, vcxrData, carrier);
    if (isCxrParticipates)
    {
      if (!isPaperTicketConflict(trx, diag, vcxrData, carrier))
        addValidatingCxr(trx, diag, vcxrData, valCxrGsaData, carrier);
    }
    else
    {
      carriersToBeSwapped.insert(carrier);
    }
  }

  for (const CarrierCode& marketingCarrier : carriersToBeSwapped)
  {
    std::set<CarrierCode> swapCarriers;
    getGenSalesAgents(trx.dataHandle(),
        diag,
        marketingCarrier,
        cspInfo.getCountryCode(),
        trx.getRequest()->ticketingAgent()->cxrCode(),
        cspInfo.getSettlementPlanTypeCode(),
        swapCarriers);

    for (const CarrierCode& swapCxr : swapCarriers)
    {
      if (failedCarriers.count(swapCxr))
        continue; // already failed

      // Check if it is already added as marketing carrier. If not, attempt to add it
      if (valCxrGsaData && valCxrGsaData->hasCarrier(swapCxr))
      {
        valCxrGsaData->gsaSwapMap()[marketingCarrier].insert(swapCxr);
        continue;
      }

      vcx::ValidatingCxrData vcxrData;
      const bool isCxrParticipates =
        checkCarrierSMParticipation(trx, diag, cspInfo, vcxrData, swapCxr);
      if (isCxrParticipates)
      {
        if (!isPaperTicketConflict(trx, diag, vcxrData, swapCxr))
        {
          addValidatingCxr(trx, diag, vcxrData, valCxrGsaData, swapCxr);
          valCxrGsaData->gsaSwapMap()[marketingCarrier].insert(swapCxr);
        }
      }
      else
      {
        failedCarriers.insert(swapCxr);
      }
    }
  }
  return valCxrGsaData;
}

bool
ValidatingCxrUtil::isNationValid( Trx& trx,
                                  const NationCode& country,
                                  Diag191Collector* diag191)
{
  const std::vector<Nation*>& nations=  trx.dataHandle().getAllNation(trx.ticketingDate());
  if (nations.empty())
  {
    if (diag191)
      *diag191 << "\nNO NATION FOUND IN DATABASE\n";
  }
  else
  {
    for (Nation* nation : nations)
    {
      if (country == vcx::ALL_COUNTRIES)
        break;
      if (nation->nation()== country)
      {
        return true;
      }
    }
    if (diag191)
    {
      *diag191 << "\nNATION IS NOT VALID: ";
      *diag191 << country.c_str();
      *diag191 << "\n";
    }
  }
  if (diag191)
  {
    diag191->print191Footer();
    diag191->flushMsg();
  }
  return false;
}

// Get General Sales Agent for carrier that does not participates in the settlement
// plan. Only GSAs that has 3rd party agmt are returned
void
ValidatingCxrUtil::getGenSalesAgents(DataHandle& dh,
                                      Diag191Collector* diag191,
                                      const CarrierCode& cxr,
                                      const NationCode& country,
                                      const CrsCode& primeHost,
                                      const SettlementPlanType& stmlPlan,
                                      std::set<CarrierCode>& gsaList)
{
  const std::vector<GenSalesAgentInfo*>& possibleGSAs =
    dh.getGenSalesAgents(primeHost, country, stmlPlan, cxr);

  if (UNLIKELY(diag191))
    *diag191 << "\nGENERAL SALES AGENT INFO FOR " << cxr << std::endl;

  for (GenSalesAgentInfo* info : possibleGSAs)
  {
    const CarrierCode& gsa = info->getCxrCode();
    if (diag191)
      *diag191 << "  " << gsa << " IS GSA of " << cxr << std::endl;
    if (ValidatingCxrUtil::validateSwapCxrInterlineAgreement(dh,
          country,
          primeHost,
          cxr,
          gsa))
    {
      gsaList.insert(gsa);
    }
    else if (diag191)
    {
      *diag191 << "  " << gsa << " DOES NOT HAVE 3RD PARTY AGMT WITH " << cxr << std::endl;
    }
  }

  if (UNLIKELY(diag191))
    *diag191 << "  " << (gsaList.empty() ? "NO GSA FOUND"
        : DiagnosticUtil::containerToString(gsaList))
      << "\n";
}

void
ValidatingCxrUtil::getAirlineInterlineAgreements(DataHandle& dh,
                                                  const CarrierCode& vcxr,
                                                  const NationCode& country,
                                                  const CrsCode& gds,
                                                  std::vector<AirlineInterlineAgreementInfo*>& aiaList)
{
  aiaList.reserve(500);
  const std::vector<AirlineInterlineAgreementInfo*>& zzList =
    dh.getAirlineInterlineAgreements(vcx::ALL_COUNTRIES, gds, vcxr);
  if (!zzList.empty())
  {
    aiaList.insert( aiaList.end(), zzList.begin(), zzList.end() );
  }
  const std::vector<AirlineInterlineAgreementInfo*>& specificCountryList =
      dh.getAirlineInterlineAgreements(country, gds, vcxr);
  if (!specificCountryList.empty())
  {
    aiaList.insert(aiaList.end(), specificCountryList.begin(), specificCountryList.end());
  }

  if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
  {
    if (zzList.empty())
    {
      LOG4CXX_DEBUG( logger, "NO ZZ ITEMS FOUND" );
    }
    else
    {
      LOG4CXX_DEBUG( logger, "ZZ ITEMS FOUND" );
    }

    if (specificCountryList.empty())
    {
      LOG4CXX_DEBUG( logger, "NO COUNTRY ITEMS FOUND" );
    }
    else
    {
      LOG4CXX_DEBUG( logger, "COUNTRY ITEMS FOUND" );
    }
  }
}

void
ValidatingCxrUtil::getAllValidatingCarriersForFlightFinderTrx(const PricingTrx& trx,
                                                              std::vector<CarrierCode>& result)
{
  result.clear();

  if (fallback::fallbackValidatingCxrMultiSp(&trx) && !trx.overrideFallbackValidationCXRMultiSP())
  {
    getAllValidatingCarriersForFlightFinderTrx(trx.validatingCxrHashMap(), result);
    return;
  }

  std::set<CarrierCode> uniqueSet;
  for (auto& kv1 : trx.hashSpValidatingCxrGsaDataMap())
    if (kv1.second)
      for (auto& kv2 : *kv1.second)
        if (kv2.second)
          for (auto& kv3 : kv2.second->validatingCarriersData())
            uniqueSet.insert(kv3.first);

  result.insert(result.end(), uniqueSet.begin(), uniqueSet.end());
}

// Get all potential validating carriers for a given Trx
void
ValidatingCxrUtil::getAllValidatingCarriersForFlightFinderTrx(const ValidatingCxrGSADataHashMap& valCxrGsaDataHashMap,
    std::vector<CarrierCode>& result)
{
  std::set<CarrierCode> uniqueSet;

  for (const auto& elem : valCxrGsaDataHashMap)
  {
    if (!elem.second)
      continue;

    ValidatingCxrDataMap::const_iterator j, jEnd = (elem.second)->validatingCarriersData().end();
    for (j = (elem.second)->validatingCarriersData().begin(); j != jEnd; ++j)
      uniqueSet.insert(j->first);
  }

  result.insert(result.end(), uniqueSet.begin(), uniqueSet.end());
}

short
ValidatingCxrUtil::adjustTicketDate(PricingTrx& trx)
{
  short utcOffset = 0;
  DateTime time = DateTime::localTime();
  if (LIKELY(trx.getRequest()->ticketingAgent()))
  {
	const Loc* hdqLoc = trx.dataHandle().getLoc(RuleConst::HDQ_CITY, trx.ticketingDate());

    const Loc* salesLoc = trx.getRequest()->salePointOverride().empty()
                              ? trx.getRequest()->ticketingAgent()->agentLocation()
                              : trx.dataHandle().getLoc(trx.getRequest()->salePointOverride(), trx.ticketingDate());

    if (LIKELY(hdqLoc && salesLoc))
    {
      if (LIKELY(LocUtil::getUtcOffsetDifference(*hdqLoc, *salesLoc, utcOffset, trx.dataHandle(), time, time)))
        return utcOffset;
    }
  }
  return utcOffset;
}

bool
ValidatingCxrUtil::isGTCCarriers(const PricingTrx& trx,
                                  const std::vector<CarrierCode>& marketingCarriers)
{
  std::string gtcCarrierList = TrxUtil::gtcCarrierListData(trx);
  if (UNLIKELY(marketingCarriers.empty() || gtcCarrierList.empty()))
    return false;

  for (const CarrierCode& cxr : marketingCarriers)
  {
    if (cxr.empty() || gtcCarrierList.find(cxr) == std::string::npos)
      return false;
  }

  return true;
}

bool
ValidatingCxrUtil::isThrowException(const PricingTrx& trx)
{
  bool isShopping = (trx.getTrxType() == PricingTrx::IS_TRX) ||
    (trx.getTrxType() == PricingTrx::MIP_TRX) ||
    (trx.getTrxType() == PricingTrx::FF_TRX) ||
    (trx.getRequest()->isMultiTicketRequest() && !trx.multiTicketMap().empty());

  return (!isShopping ||
      (trx.getTrxType() == PricingTrx::MIP_TRX &&
       (trx.billing()->actionCode() == "WPNI.C" || trx.billing()->actionCode() == "WFR.C")));
}

// For open segment Itin, Ticketing is sending ** or BLANK as marketing carrier
// Return true if open segment found (**, BLANKBLANK, BLANK)
bool
ValidatingCxrUtil::checkMarketingCarrierMissing(const std::vector<CarrierCode>& mktItinCxrs)
{
  for (const CarrierCode& cc : mktItinCxrs)
  {
    if (UNLIKELY(cc.equalToConst("**") || cc.equalToConst("  ") || cc == " "))
      return true;
  }
  return false;
}

bool
ValidatingCxrUtil::canProcessGSASwap(const PricingTrx& trx, const SettlementPlanType& sp)
{
  return (!trx.isSkipGsa() &&
      find(swapAllowedSp.begin(), swapAllowedSp.end(), sp) != swapAllowedSp.end());
}

bool
ValidatingCxrUtil::canProcessNVCSwap(const PricingTrx& trx, const SettlementPlanType& sp)
{
  return (!trx.isSkipNeutral() &&
      find(swapAllowedSp.begin(), swapAllowedSp.end(), sp) != swapAllowedSp.end());
}

void
ValidatingCxrUtil::setErrorMsg(
    PricingTrx& trx,
    Diag191Collector* diag,
    bool anyCxrHasGsa,
    const std::vector<vcx::ValidationStatus>& gsaStatusList,
    std::string& errMsg)
{
  if (anyCxrHasGsa)
  {
    if (std::find(gsaStatusList.begin(),
          gsaStatusList.end(),
          vcx::PAPER_TKT_OVERRIDE_ERR) != gsaStatusList.end())
      errMsg = "PAPER TICKET NOT PERMITTED";
    else
      errMsg = "NO VALID TICKETING AGREEMENTS FOUND";
  }
  else
  {
    if (errMsg.empty())
    {
      const CarrierCode& defValCxr = trx.getRequest()->validatingCarrier();
      errMsg = defValCxr;
      errMsg += " NOT VALID FOR SETTLEMENT METHOD";
      if ((!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP()) &&
          trx.getRequest()->isTicketEntry())
      {
        DataHandle dataHandle(determineTicketDate(trx));
        const NationCode nation(getNation(trx, getPOSLoc(trx, diag)));
        const CrsCode crs(trx.getRequest()->ticketingAgent()->cxrCode());
        if (isGTCCarrier(dataHandle, diag, nation, crs, defValCxr))
          errMsg += " - GTC CARRIER";
      }
    }
  }
}

// Check whether carrier participates (based on previous vaidation results) and
CountrySettlementPlanInfo* ValidatingCxrUtil::getCountrySettlementPlanInfoForSp(
    const std::vector<CountrySettlementPlanInfo*>& cspiCol,
    const SettlementPlanType& sp)
{
  auto it = find_if(cspiCol.begin(), cspiCol.end(),
      [&sp](const CountrySettlementPlanInfo* obj)-> bool {
      return obj->getSettlementPlanTypeCode() == sp;
      });
  return it != cspiCol.end() ? *it : nullptr;
}

bool
ValidatingCxrUtil::isValidFPathForValidatingCxr(FarePath& farePath)
{
  std::vector<CarrierCode> fpCxrVec;
  bool firstTime = true;

  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    if (firstTime)
    {
      fpCxrVec = pu->validatingCarriers();
      firstTime = false;
      if (UNLIKELY(fpCxrVec.empty()))
      {
        // If a valid PU doesn't have a validating Cxr List,
        // We don't need to process GSA validation and just let this FarePath pass
        farePath.validatingCarriers().clear();
        return true;
      }
    }
    else
    {
      // Intersect with already being processed list of Validating Cxr
      PricingUtil::intersectCarrierList(fpCxrVec, pu->validatingCarriers());

      if (fpCxrVec.empty())
      {
        // Empty intersection list means, this FarePath is not good for further processing
        farePath.validatingCarriers().clear();
        return false;
      }
    }
  }

  farePath.validatingCarriers() = fpCxrVec;
  return true;
}

bool
ValidatingCxrUtil::isNonPreferredVC(const PricingTrx& trx, const CarrierCode& carrier)
{
  return std::find(trx.getRequest()->nonPreferredVCs().begin(),
      trx.getRequest()->nonPreferredVCs().end(),
      carrier) != trx.getRequest()->nonPreferredVCs().end();
}

const Loc*
ValidatingCxrUtil::getPOSLoc(PricingTrx& trx, Diag191Collector* diag)
{
  const Loc* posLoc = TrxUtil::ticketingLoc(trx);
  if (!posLoc)
  {
    if (diag)
      diag->flushMsg();

    throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR,
        "UNABLE TO DETERMINE POINT OF SALE LOCATION");
  }
  return posLoc;
}

NationCode
ValidatingCxrUtil::getNation(PricingTrx& trx, const Loc* posLoc)
{
  NationCode nation = posLoc ? posLoc->nation() : "";
  if (trx.getRequest() &&
      trx.getRequest()->ticketingAgent() &&
      trx.getRequest()->ticketingAgent()->isArcUser() &&
      trx.getRequest()->ticketPointOverride().empty())
  {
    nation = "US";
  }
  return nation;
}
namespace
{
template <typename T>
const T&
as_const(T& r)
{
  return r;
}

template <typename T>
const T*
as_const(T* p)
{
  return p;
}

void
setValidatingCarrier(FPPQItem& newItem, CarrierCode cxr)
{
  newItem.farePath()->validatingCarriers().clear();
  newItem.farePath()->validatingCarriers().push_back(cxr);

  for (PricingUnit* pu : newItem.farePath()->pricingUnit())
  {
    pu->validatingCarriers().clear();
    pu->validatingCarriers().push_back(cxr);
  }
}
}

void
ValidatingCxrUtil::cloneFpPQItemForValCxrs(PricingTrx& trx,
                                           FPPQItem& fppqItem,
                                           std::vector<FPPQItem*>& clonedFpPQ,
                                           FarePathFactoryStorage& storage)
{
  if (as_const(fppqItem.farePath())->validatingCarriers().size() <= 1)
  {
    clonedFpPQ.push_back(&fppqItem);
    return;
  }

  // Reusing original FPPQItem
  std::vector<CarrierCode> valCxrs = as_const(fppqItem.farePath())->validatingCarriers();

  const bool ignoreAltVCs =
      (trx.isValidatingCxrGsaApplicable() && (trx.getTrxType() == PricingTrx::MIP_TRX) &&
       (!trx.getRequest()->isAlternateValidatingCarrierRequest()));

  if (_UNLIKELY(ignoreAltVCs))
  {
    // Move Default validating carrier to the beginning for MIP trx
    CarrierCode defVcxr, marketVcxr;
    bool retVal = false;

    if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
    {
      DefaultValidatingCarrierFinder defValCxrFinder(trx, *fppqItem.farePath()->itin());
      retVal = defValCxrFinder.determineDefaultValidatingCarrier(valCxrs, defVcxr, marketVcxr);
    }
    else
    {
      ValidatingCarrierUpdater valCxrUpdater(trx);
      retVal = valCxrUpdater.determineDefaultValidatingCarrier(
          *(fppqItem.farePath()->itin()), valCxrs, defVcxr, marketVcxr);
    }

    if (retVal)
    {
      std::vector<CarrierCode>::iterator itemItr =
          std::find(valCxrs.begin(), valCxrs.end(), defVcxr);
      if (itemItr != valCxrs.end())
      {
        std::iter_swap(valCxrs.begin(), itemItr);
      }
    }
  }
  std::vector<CarrierCode>::const_iterator it = valCxrs.begin();

  fppqItem.farePath()->validatingCarriers().clear();
  fppqItem.farePath()->validatingCarriers().push_back(*it++);
  clonedFpPQ.push_back(&fppqItem);

  for (; it != valCxrs.end(); ++it)
  {
    LOG4CXX_DEBUG(logger, "CLONE FOR VAL-CXR " << *it);
    FPPQItem* newItem = fppqItem.clone(storage);
    setValidatingCarrier(*newItem, *it);
    clonedFpPQ.push_back(newItem);
  }
}

// Check if valCxr is GTC carrier. A carrier is GTC if it participates
// in GTC settlement plan
bool
ValidatingCxrUtil::isGTCCarrier(
    DataHandle& dataHandle,
    Diag191Collector* diag,
    const NationCode& nation,
    const CrsCode& crs,
    const CarrierCode& valCxr)
{
  bool isGTC = false;
  const SettlementPlanType gtc("GTC");
  const std::vector<CountrySettlementPlanInfo*>&
    cspiList = getCountrySettlementPlanInfo(dataHandle, diag, nation, &gtc);
  if (!cspiList.empty())
  {
    auto it = std::find_if(cspiList.begin(), cspiList.end(),
        [&gtc] (CountrySettlementPlanInfo* ptr)->bool {
        return ptr->getSettlementPlanTypeCode()==gtc;
        });

    isGTC =
      (
       it != cspiList.end() &&
       *it &&
       getAirlineCountrySettlementPlanInfo(**it, diag, valCxr, crs, dataHandle)
      );
  }
  return isGTC;
}

// Set Difference
// Return true if lhs set has any element left after set diff
bool
ValidatingCxrUtil::getValCxrSetDifference(std::vector<CarrierCode>& lhs,
                                          const std::vector<CarrierCode>& rhs,
                                          const PricingTrx* trx)
{
  std::set<CarrierCode> lset(lhs.begin(), lhs.end());
  std::set<CarrierCode> rset(rhs.begin(), rhs.end());

  std::set<CarrierCode> result;
  std::set_difference(
      lset.begin(), lset.end(), rset.begin(), rset.end(), std::inserter(result, result.end()));

  if (result.size())
  {
    // replace val-cxrs with new from set diff result
    lhs.assign(result.begin(), result.end());
    return true;
  }
  else
  {
    lhs.clear();
  }
  return false;
}

void
ValidatingCxrUtil::removeTaggedValCxr(FarePath& fp,
                                      const std::vector<FarePath*>& taggedFps,
                                      PricingTrx& trx)
{
  std::vector<CarrierCode> taggedValCxrs;
  for (FarePath* taggedFp : taggedFps)
  {
    taggedValCxrs.insert(taggedValCxrs.end(),
                         taggedFp->validatingCarriers().begin(),
                         taggedFp->validatingCarriers().end());
  }
  getValCxrSetDifference(fp.validatingCarriers(), taggedValCxrs, &trx);
}

void
ValidatingCxrUtil::mergeValidatingCxrs(FarePath& primary, const FarePath& fp)
{
  std::vector<CarrierCode>& valCxrs = primary.validatingCarriers();
  valCxrs.insert(valCxrs.end(), fp.validatingCarriers().begin(), fp.validatingCarriers().end());
}

FarePath*
ValidatingCxrUtil::getTaggedFarePathWithValCxr(CarrierCode vcxr, const FarePath& fp)
{
  for (FarePath* clonedFp : fp.gsaClonedFarePaths())
  {
    const std::vector<CarrierCode>& taggedValCxrs = clonedFp->validatingCarriers();
    if (std::find(taggedValCxrs.begin(), taggedValCxrs.end(), vcxr) != taggedValCxrs.end())
      return clonedFp;
  }
  return nullptr;
}

void
ValidatingCxrUtil::mergeFarePathWithEqualComponents(std::list<FPPQItem*>& clonedItems,
                                                    FarePathFactoryStorage& storage)
{
  for (auto fppqIt = clonedItems.begin(); fppqIt != clonedItems.end(); ++fppqIt)
  {
    FPPQItem& fppqItem = **fppqIt;

    for (auto otherIt = std::next(fppqIt); otherIt != clonedItems.end();)
    {
      FPPQItem& otherItem = **otherIt;

      const MoneyAmount diff = fabs(fppqItem.farePath()->getNUCAmountScore() -
                                    otherItem.farePath()->getNUCAmountScore());

      if (diff < EPSILON && otherItem.isEqualAmountComponents(fppqItem))
      {
        ValidatingCxrUtil::mergeValidatingCxrs(*fppqItem.farePath(), *otherItem.farePath());

        storage.releaseFPPQItemDeep(otherItem);
        otherIt = clonedItems.erase(otherIt);
      }
      else
        ++otherIt;
    }
  }
}

void
ValidatingCxrUtil::tagFarePathForSameAmountDiffComps(std::list<FPPQItem*>& items,
                                                     std::vector<FPPQItem*>& equalAmtDiffComps)
{
  std::list<FPPQItem*>::iterator i = items.begin();
  while (i != items.end())
  {
    if (std::find(equalAmtDiffComps.begin(), equalAmtDiffComps.end(), *i) !=
        equalAmtDiffComps.end())
    {
      ++i;
      continue;
    }

    FPPQItem* fppqItem = *i;
    for (std::list<FPPQItem*>::iterator j = ++i; j != items.end(); ++j)
    {
      FPPQItem* item = *j;

      if (std::find(equalAmtDiffComps.begin(), equalAmtDiffComps.end(), item) !=
          equalAmtDiffComps.end())
        continue;

      MoneyAmount diff;
      diff = fppqItem->farePath()->getNUCAmountScore() - item->farePath()->getNUCAmountScore();
      if (fabs(diff) < EPSILON)
      {
        ValidatingCxrUtil::mergeValidatingCxrs(*fppqItem->farePath(), *item->farePath());
        fppqItem->farePath()->gsaClonedFarePaths().push_back(item->farePath());
        equalAmtDiffComps.push_back(item);
      }
    }
  }
}

FPPQItem*
ValidatingCxrUtil::findLowestFarePath(const std::list<FPPQItem*>& items)
{
  const auto lowestIt = std::min_element(items.begin(),
                                         items.end(),
                                         [](FPPQItem* l, FPPQItem* r)
                                         {
    return l->farePath()->getNUCAmountScore() + EPSILON < r->farePath()->getNUCAmountScore();
  });

  return (lowestIt != items.end()) ? *lowestIt : nullptr;
}

//-------------------------------------------------------------------
// Select item based on default validating carrier. Make sure the priamary is always default.
void
ValidatingCxrUtil::setDefaultFarePath(std::list<FPPQItem*>& items, PricingTrx* trx)
{
  ValidatingCarrierUpdater valCxrUpdater(*trx);
  CarrierCode defVcxr, marketVcxr;
  for (FPPQItem* item : items)
  {
    if (item == nullptr || item->farePath() == nullptr ||
        item->farePath()->gsaClonedFarePaths().empty())
      continue;

    defVcxr = "";
    marketVcxr = "";
    bool retVal = false;

    if (!fallback::fallbackValidatingCxrMultiSp(trx) || trx->overrideFallbackValidationCXRMultiSP())
    {
      DefaultValidatingCarrierFinder defValCxrFinder(*trx, *item->farePath()->itin());
      retVal = defValCxrFinder.determineDefaultValidatingCarrier(
          item->farePath()->validatingCarriers(), defVcxr, marketVcxr);
    }
    else
    {
      retVal = valCxrUpdater.determineDefaultValidatingCarrier(
          *item->farePath()->itin(), item->farePath()->validatingCarriers(), defVcxr, marketVcxr);
    }

    if (!retVal)
      continue;

    FarePath* primaryFp =
        ValidatingCxrUtil::getTaggedFarePathWithValCxr(defVcxr, *item->farePath());
    if (!primaryFp)
      continue;

    std::vector<FarePath*> taggedFps = item->farePath()->gsaClonedFarePaths();
    item->farePath()->clearGsaClonedFarePaths();

    ValidatingCxrUtil::removeTaggedValCxr(*item->farePath(), taggedFps, *trx);

    taggedFps.push_back(item->farePath());
    for (FarePath* fp : taggedFps)
    {
      if (fp == primaryFp)
        continue;

      ValidatingCxrUtil::mergeValidatingCxrs(*primaryFp, *fp);
      primaryFp->gsaClonedFarePaths().push_back(fp);
    }
    item->farePath() = primaryFp;
  }
}

void
ValidatingCxrUtil::processFarePathClones(FPPQItem*& fppqItem,
                                         std::list<FPPQItem*>& clonedFPPQItems,
                                         FarePathFactoryStorage& storage,
                                         PricingTrx* trx)
{
  if (fppqItem->isValid())
    clonedFPPQItems.push_front(fppqItem);

  ValidatingCxrUtil::mergeFarePathWithEqualComponents(clonedFPPQItems, storage);
  FPPQItem* selectedItem = ValidatingCxrUtil::findLowestFarePath(clonedFPPQItems);

  if (selectedItem && fppqItem != selectedItem)
  {
    // Selected Item should be first since tagged logic depends
    clonedFPPQItems.remove(selectedItem);
    clonedFPPQItems.push_front(selectedItem);
    fppqItem = selectedItem;
  }

  std::vector<FPPQItem*> equalAmtDiffComps;
  ValidatingCxrUtil::tagFarePathForSameAmountDiffComps(clonedFPPQItems, equalAmtDiffComps);

  for (FPPQItem* item : equalAmtDiffComps)
    clonedFPPQItems.remove(item);

  ValidatingCxrUtil::setDefaultFarePath(clonedFPPQItems, trx);
}
} // namespace tse
