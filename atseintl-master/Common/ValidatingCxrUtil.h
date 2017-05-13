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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCxrConst.h"
#include "DataModel/FlightFinderTrx.h"

#include <algorithm>

namespace tse
{

class CountrySettlementPlanInfo;
class FarePathFactoryStorage;
class FPPQItem;
class TicketingCxrTrx;
class Diag191Collector;
class ValidatingCxrGSAData;
class ValidatingCxrUtil
{
public:
  static void
  determineCountrySettlementPlan(PricingTrx& trx,
                                 Diag191Collector* diag,
                                 const SettlementPlanType* requestedPlan = nullptr);

  static CountrySettlementPlanInfo*
  determineCountrySettlementPlan(TicketingCxrTrx& trx,
                                 Diag191Collector* diag,
                                 const SettlementPlanType* requestedPlan = nullptr);

  static bool stdIncludesWrapper(const std::vector<CarrierCode>& vecFirst1,
                                 const std::vector<CarrierCode>& vecFirst2);
  static ValidatingCxrGSAData*
  getValidatingCxrList(PricingTrx& trx,
                       Diag191Collector* diag,
                       const CountrySettlementPlanInfo& cspInfo,
                       const std::vector<CarrierCode>& potentialVcxrs,
                       const std::vector<CarrierCode>& participatingCarriers,
                       std::string& errMsg,
                       const std::vector<CarrierCode>& marketingItinCxrs);

  static std::set<CarrierCode>
  getMarketingCxrFromSwapCxr(const Itin& itin, const CarrierCode& carrier);

  static void getMarketingCxrsFromValidatingCxrs(PricingTrx& trx,
                                                 const Itin& itin,
                                                 const std::vector<CarrierCode>& validatingCxrs,
                                                 std::vector<CarrierCode>& marketingCxrs /*out*/);

  static void getValidatingCxrsFromMarketingCxr(const Itin& itin,
                                                const CarrierCode& marketingCarrier,
                                                std::set<CarrierCode>& validatingCxrs);

  static void getAllValidatingCarriersForFlightFinderTrx(const PricingTrx& trx,
                                                         std::vector<CarrierCode>& result);

  static vcx::TicketType getTicketingMethod(const CountrySettlementPlanInfo& cspi,
                                            const AirlineCountrySettlementPlanInfo* acspi);
  static vcx::TicketType getTicketingMethod(const CountrySettlementPlanInfo& cspi);

  static AirlineCountrySettlementPlanInfo*
  getAirlineCountrySettlementPlanInfo(const CountrySettlementPlanInfo& cspi,
                                      Diag191Collector* diag,
                                      const CarrierCode& carrier,
                                      const CrsCode& crs,
                                      DataHandle& dataHandle);

  static bool validateInterlineAgreements(DataHandle& dh,
                                          Diag191Collector* diag,
                                          const vcx::Pos& pos,
                                          const CarrierCode& vcxr,
                                          vcx::ValidatingCxrData& vcxrData,
                                          bool isVcxrPartOfItin = true,
                                          vcx::TicketType requestedTicketType = vcx::ETKT_PREF,
                                          bool onlyCheckAgreementExistence=false);
  static bool validateSwapCxrInterlineAgreement(DataHandle& dh,
      const NationCode& country,
      const CrsCode& primeHost,
      const CarrierCode& vcxr,
      const CarrierCode& swapCxr);

  static void getGenSalesAgents(DataHandle& dh,
      Diag191Collector* diag191,
      const CarrierCode& cxr,
      const NationCode& country,
      const CrsCode& primeHost,
      const SettlementPlanType& stmlPlan,
      std::set<CarrierCode>& gsaList);

  static void getAirlineInterlineAgreements(
      DataHandle& dh,
      const CarrierCode& vcxr,
      const NationCode& country,
      const CrsCode& gds,
      std::vector<AirlineInterlineAgreementInfo*>& aiaList);

  static bool
  isPaperTicketOverrideConflict(vcx::TicketType vcxrTktType, vcx::TicketType requestedTktType);



  static void getAllItinCarriers(const PricingTrx& trx,
                                 const Itin& itin,
                                 std::vector<CarrierCode>& marketingCarriers,
                                 std::vector<CarrierCode>& participatingCarriers);

  static void getParticipatingCxrs(const PricingTrx& trx,
                                   const Itin& itin,
                                   std::vector<CarrierCode>& participatingCarriers);

  static void getMarketingItinCxrs(const Itin& itin,
                                   std::vector<CarrierCode>& marketingCxrs);

  static std::string createHashString(const std::vector<CarrierCode>& marketingCarriers,
                                      const std::vector<CarrierCode>& participatingCarriers);

  static std::string buildValidationCxrMsg(vcx::ValidationStatus status,
                                           const CarrierCode& vcxr,
                                           const CarrierCode& cxr);

  static std::string buildValidationCxrMsg(const Itin& itin,
                                           vcx::ValidationStatus status,
                                           const std::vector<CarrierCode>& valCxr);
  static void getAllSopMarketingCarriers(FlightFinderTrx& trx,
                                         std::vector<CarrierCode>& marketingCarriers);

  static ValidatingCxrGSAData*
  validateSettlementMethodAndGsa(PricingTrx& trx,
                                 Diag191Collector* diag,
                                 const CountrySettlementPlanInfo& cspInfo,
                                 const std::vector<CarrierCode>& marketingCarriers);

  static void addParticipatingCxr(vcx::ValidatingCxrData& vcxrData,
                                  const CarrierCode& participatingCarrier);

  static bool isPassNSPInterlineAgreement(PricingTrx& trx,
                                       Diag191Collector* diag,
                                       std::string& errMsg,
                                       const NationCode& countryCode,
                                       vcx::ValidatingCxrData& vcxrData,
                                       const CarrierCode& carrier,
                                       const std::vector<CarrierCode>& participatingCarriers,
                                       const std::vector<CarrierCode>& marketingCarriers,
                                       const CarrierCode& swappedFor = "",
                                       const bool isNeutral = false);
  static bool isPassInterlineAgreement(PricingTrx& trx,
                                       Diag191Collector* diag,
                                       const NationCode& countryCode,
                                       vcx::ValidatingCxrData& vcxrData,
                                       const CarrierCode& carrier,
                                       const std::vector<CarrierCode>& participatingCarriers,
                                       const std::vector<CarrierCode>& marketingCarriers,
                                       const CarrierCode& swappedFor = "",
                                       const bool isNeutral = false);

  static bool  isNationValid( Trx& trx,
                              const NationCode& country,
                              Diag191Collector* diag191);

  static short adjustTicketDate(PricingTrx& trx);

  static  DateTime determineTicketDate(PricingTrx& trx);
  static bool isGTCCarriers(const PricingTrx& trx,
      const std::vector<CarrierCode>& marketingCxrs);
  static bool isGTCCarrier(
      DataHandle& dataHandle,
      Diag191Collector* diag,
      const NationCode& nation,
      const CrsCode& crs,
      const CarrierCode& valCxr);

  static bool isThrowException(const PricingTrx& trx);
  static CountrySettlementPlanInfo*
  determinePlanFromHierarchy(const std::vector<CountrySettlementPlanInfo*>& cspiList);
  static SettlementPlanType determinePlanFromHierarchy(const std::vector<SettlementPlanType>& spList);
  static SettlementPlanType getSettlementPlanFromHierarchy(const SpValidatingCxrGSADataMap& spGsaDataMap);
  static CountrySettlementPlanInfo* getCountrySettlementPlanInfoForSp(const std::vector<CountrySettlementPlanInfo*>& cspiCol,
                                                                      const SettlementPlanType& sp);

  static bool isValidFPathForValidatingCxr(FarePath& farePath);
  static bool isNonPreferredVC(const PricingTrx& trx, const CarrierCode&);
  static void getMarketingCarriersInItinOrder(
      const Itin& itin,
      std::vector<CarrierCode>& marketingCxr);

  static void getAlternateCxrInOrder(
    const Itin& itin,
    const std::vector<CarrierCode>& mktCxrs,
    std::vector<CarrierCode>& cxrs,
    const SettlementPlanType* sp = nullptr);

  static bool isRexPricing(const PricingTrx& pTrx)
    {
      return (pTrx.excTrxType() == PricingTrx::PORT_EXC_TRX) ||
             (pTrx.excTrxType() == PricingTrx::AR_EXC_TRX)   ||
             (pTrx.excTrxType() == PricingTrx::AF_EXC_TRX);
    }

  static void cloneFpPQItemForValCxrs(PricingTrx& trx,
                                      FPPQItem& fppqItem,
                                      std::vector<FPPQItem*>& clonedFpPQ,
                                      FarePathFactoryStorage& storage);
  static void mergeValidatingCxrs(FarePath& primary, const FarePath& fp);
  static bool getValCxrSetDifference(std::vector<CarrierCode>& lhs,
                                     const std::vector<CarrierCode>& rhs,
                                     const PricingTrx* trx);
  static void
  removeTaggedValCxr(FarePath& fp, const std::vector<FarePath*>& taggedFps, PricingTrx& trx);
  static FarePath* getTaggedFarePathWithValCxr(CarrierCode vcxr, const FarePath& fp);
  static void mergeFarePathWithEqualComponents(std::list<FPPQItem*>& clonedItems,
                                               FarePathFactoryStorage& storage);

  static void tagFarePathForSameAmountDiffComps(std::list<FPPQItem*>& items,
                                                std::vector<FPPQItem*>& equalAmtDiffComps);

  static FPPQItem* findLowestFarePath(const std::list<FPPQItem*>& items);

  static void setDefaultFarePath(std::list<FPPQItem*>& items, PricingTrx* trx);

  static void processFarePathClones(FPPQItem*& fppqItem,
                                    std::list<FPPQItem*>& clonedFPPQItems,
                                    FarePathFactoryStorage& storage,
                                    PricingTrx* trx);
  static const Loc* getPOSLoc(PricingTrx& trx, Diag191Collector* diag);
  static NationCode getNation(PricingTrx& trx, const Loc* posLoc);
protected:
  static CountrySettlementPlanInfo*
  determineCountrySettlementPlanBase(DataHandle& dh,
                                     const NationCode& nation,
                                     Diag191Collector* diag,
                                     const SettlementPlanType* requestedPlan);

  static const std::vector<CountrySettlementPlanInfo*>&
  getCountrySettlementPlanInfo(DataHandle& dataHandle,
      Diag191Collector* diag,
      const NationCode& nation,
      const SettlementPlanType* requestedPlan);

  static CountrySettlementPlanInfo*
  determineCountrySettlementPlanBase(const std::vector<CountrySettlementPlanInfo*>& cspiList,
                                     Diag191Collector* diag,
                                     const NationCode& nation,
                                     const SettlementPlanType* requestedPlan,
                                     std::string& msg);

  static CountrySettlementPlanInfo*
  getRequestedPlan(const SettlementPlanType& sPlan,
                   const std::vector<CountrySettlementPlanInfo*>& cspiList);

  static void processNeutralCarriers(PricingTrx& trx,
      Diag191Collector* diag,
      const CountrySettlementPlanInfo& cspInfo,
      ValidatingCxrGSAData*& valCxrGsaData,
      const std::vector<CarrierCode>& participatingCarriers,
      const std::vector<CarrierCode>& marketingCarriers);

  static bool checkNSPCarrierSMParticipation(PricingTrx& trx,
                                          Diag191Collector* diag,
                                          const CountrySettlementPlanInfo& cspInfo,
                                          vcx::ValidatingCxrData& vcxrData,
                                          const CarrierCode& validatingCarrier);
  static bool checkCarrierSMParticipation(PricingTrx& trx,
                                          Diag191Collector* diag,
                                          const CountrySettlementPlanInfo& cspInfo,
                                          vcx::ValidatingCxrData& vcxrData,
                                          const CarrierCode& validatingCarrier);
  static bool isAddNSPValidatingCxr(PricingTrx& pricingTrx,
                                      Diag191Collector* diag,
                                      const CountrySettlementPlanInfo& cspInfo,
                                      vcx::ValidatingCxrData& vcxrData,
                                      ValidatingCxrGSAData*& valCxrGsaData,
                                      const CarrierCode& carrier,
                                      const bool isNeutral=false);

  static void addValidatingCxr(PricingTrx& pricingTrx,
                               Diag191Collector* diag,
                               vcx::ValidatingCxrData& vcxrData,
                               ValidatingCxrGSAData*& valCxrGsaData,
                               const CarrierCode& carrier,
                               const bool isNeutral = false);

  static void pushIfNotExists(std::vector<CarrierCode>& v, const CarrierCode& carrier);

  static AirlineInterlineAgreementInfo*
  findAgreement(const std::vector<AirlineInterlineAgreementInfo*>& aiaList,
                const CarrierCode& airline);

  static bool isElectronicTicketRequired(vcx::TicketType ticketType)
  {
    return vcx::ETKT_REQ == ticketType;
  }

  static bool isTicketTypeElectronic(vcx::TicketType ticketType)
  {
    return (vcx::ETKT_PREF == ticketType) || (vcx::ETKT_REQ == ticketType);
  }

  static bool isTicketTypePaper(vcx::TicketType ticketType)
  {
    return (vcx::PAPER_TKT_REQ == ticketType) || (vcx::PAPER_TKT_PREF == ticketType);
  }



private:

  static void getAllValidatingCarriersForFlightFinderTrx(const ValidatingCxrGSADataHashMap& validatingCxrMap,
                                                         std::vector<CarrierCode>& result);

  static vcx::ValidationStatus checkInterlineAgreements(DataHandle& dh,
                                                        const vcx::Pos& pos,
                                                        const CarrierCode& vcxr,
                                                        vcx::ValidatingCxrData& vcxrData,
                                                        bool isVcxrPartOfItin,
                                                        vcx::TicketType requestedTicketType,
                                                        bool onlyCheckAgreementExistence=false);

  static vcx::ValidationStatus checkAgreement(const AirlineInterlineAgreementInfo& aia,
                                              vcx::TicketType vcxrTicketType,
                                              vcx::TicketType requestedTicketType,
                                              bool vcxrPartOfItin);

  static void getAlternateCxrInOrder(const Itin& itin,
                                     std::vector<CarrierCode>& validatingCxr);

  static ValidatingCxrGSAData*
    getValidatingCxrListForRexTrx(PricingTrx& trx,
    Diag191Collector* diag,
    const CountrySettlementPlanInfo& cspInfo,
    const std::vector<CarrierCode>& marketingCxrs,
    const std::vector<CarrierCode>& participatingCxrs,
    std::string& errMsg,
    const std::vector<CarrierCode>& marketingItinCxrs);

  static bool isPaperTicketConflict(const PricingTrx& trx,
    Diag191Collector* diag,
    vcx::ValidatingCxrData& vcxrData,
    const CarrierCode& validatingCarrier);

  static bool checkPaperAndInterlineAgmt(PricingTrx& trx,
    Diag191Collector* diag,
    const CountrySettlementPlanInfo& cspInfo,
    vcx::ValidatingCxrData& vcxrData,
    ValidatingCxrGSAData*& valCxrGsaData,
    const CarrierCode& cxr,
    const std::vector<CarrierCode>& participatingCxrs,
    const std::vector<CarrierCode>& marketingCarriers,
    std::string& errMsg,
    const CarrierCode& swappedFor = "");

  static bool isAlternateValCxr(const CarrierCode& cxr,
      const std::vector<CarrierCode>& valCxrs,
      const std::vector<CarrierCode>& altValCxrs)
  {
    return (std::find(valCxrs.begin(), valCxrs.end(), cxr) != valCxrs.end() &&
        std::find(altValCxrs.begin(), altValCxrs.end(), cxr) == altValCxrs.end());
  }

  static bool checkMarketingCarrierMissing(const std::vector<CarrierCode>& mktItinCxrs);
  static bool canProcessGSASwap(const PricingTrx& trx, const SettlementPlanType& sp);
  static bool canProcessNVCSwap(const PricingTrx& trx, const SettlementPlanType& sp);
  static void setErrorMsg(
      PricingTrx& trx,
      Diag191Collector* diag,
      bool anyCxrHasGsa,
      const std::vector<vcx::ValidationStatus>& gsaStatusList,
      std::string& errMsg);


  static const std::vector<SettlementPlanType> swapAllowedSp;

  friend class ValidatingCxrUtilTest;
};

} // namespace tse

