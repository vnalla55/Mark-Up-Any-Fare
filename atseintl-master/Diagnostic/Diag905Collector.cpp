//----------------------------------------------------------------------------
//  File:        Diag905Collector.C
//  Author:      Adrienne A. Stipe
//  Created:     2004-09-08
//
//  Description: Diagnostic 905 formatter
//
//  Updates:
//
//  Copyright Sabre 2004
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

#include "Diagnostic/Diag905Collector.h"

#include "Common/ClassOfService.h"
#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/Diversity.h"
#include "DataModel/ESVOptions.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/TravelSeg.h"

#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

namespace tse
{
FALLBACK_DECL(fallbackNonPreferredVC);
FALLBACK_DECL(fallbackPreferredVC);
FALLBACK_DECL(fallbackNonBSPVcxrPhase1);


//----------------------------------------------------------------------------
//  Convert null character to single space
//----------------------------------------------------------------------------
namespace
{
inline char
charIndicator(char value)
{
  return (value == '\0') ? ' ' : value;
}

//----------------------------------------------------------------------------
//  Convert boolean value to human friendly form
//----------------------------------------------------------------------------
inline std::string
boolIndicator(bool value)
{
  return value ? "T" : "F";
}
}

Diag905Collector& Diag905Collector::operator<<(const PricingTrx& trx)
{
  const ShoppingTrx* const shoppingTrx = dynamic_cast<const ShoppingTrx*>(&trx);
  const RexShoppingTrx* const rexShoppingTrx = dynamic_cast<const RexShoppingTrx*>(&trx);

  int count;

  if (_active)
  {
    DiagCollector& dc(*this);

    std::string trxType;
    if (rexShoppingTrx != nullptr)
    {
      trxType = "REX SHOPPING";
    }
    else
    {
      trxType = shoppingTrx != nullptr ? "SHOPPING" : "PRICING";
    }

    dc << endl;
    dc << "******************************************************\n";
    dc << "905: " << trxType << " TRX CONTENTS AFTER SHOPPING XML PARSE:\n";
    dc << "******************************************************\n\n";

    // Options
    //

    const PricingOptions& options = *trx.getOptions();

    // assuming we always have options
    //
    dc << endl;
    dc << "Options:" << endl;

    if (shoppingTrx)
    {
      dc << " P1W ONLINESOLUTIONSONLY : " << boolIndicator(shoppingTrx->onlineSolutionsOnly())
         << endl << "  P41 INTERLINESOLUTIONSONLY : "
         << boolIndicator(shoppingTrx->interlineSolutionsOnly()) << endl
         << "  PAE NODIVERSITY : " << boolIndicator(shoppingTrx->noDiversity()) << endl
         << "  P1F ALTERNATEDATEREQUEST : " << boolIndicator(shoppingTrx->isAltDates()) << endl;

      if (shoppingTrx->interlineWeightFactor() == 0)
      {
        dc << "  Q19 INTERLINEWEIGHTFACTOR : " << endl;
      }
      else
      {
        dc << "  Q19 INTERLINEWEIGHTFACTOR : " << shoppingTrx->interlineWeightFactor() << endl;
      }

      dc << " MIN MINDURATION : " << shoppingTrx->minDuration() << endl;
      dc << " MAX MAXDURATION : " << shoppingTrx->maxDuration() << endl;
      dc << " N0W EXCHANGE IS TRX : "
         << boolIndicator(shoppingTrx->excTrxType() == PricingTrx::EXC_IS_TRX) << endl;
      dc << " Q6W AWARD REQUEST : " << boolIndicator(shoppingTrx->awardRequest()) << endl;

      int64_t minConnectionTimeDomestic = -1;

      if ((shoppingTrx->getOptions()->getMinConnectionTimeDomestic() > 0))
      {
        minConnectionTimeDomestic =
            shoppingTrx->getOptions()->getMinConnectionTimeDomestic() / SECONDS_IN_MINUTE;
      }

      dc << " MINIMUM CONNECT TIME DOMESTIC: " << minConnectionTimeDomestic << std::endl;

      int64_t minConnectionTimeInternational = -1;

      if ((shoppingTrx->getOptions()->getMinConnectionTimeInternational() > 0))
      {
        minConnectionTimeInternational =
            shoppingTrx->getOptions()->getMinConnectionTimeInternational() / SECONDS_IN_MINUTE;
      }

      dc << " MINIMUM CONNECT TIME INTERNATIONAL: " << minConnectionTimeInternational << std::endl;
    }

    dc << " C48 CURRENCYOVERRIDE : " << options.currencyOverride() << endl;
    dc << " C45 ALTERNATECURRENCY : " << options.alternateCurrency() << endl;
    dc << " POF WEB : " << charIndicator(options.web()) << endl;
    dc << " P1W ONLINEFARES : " << charIndicator(options.onlineFares()) << endl;
    dc << " P1Y PUBLISHEDFARES : " << charIndicator(options.publishedFares()) << endl;
    dc << " P1Z PRIVATEFARES : " << charIndicator(options.privateFares()) << endl;
    dc << " P20 XOFARES : " << charIndicator(options.xoFares()) << endl;

    // not in schema
    dc << " P47 NOPENALTIES : " << charIndicator(options.noPenalties()) << endl;
    dc << " P48 NOADVPURCHRESTR : " << charIndicator(options.noAdvPurchRestr()) << endl;
    dc << " P49 NOMINMAXSTAYRESTR : " << charIndicator(options.noMinMaxStayRestr()) << endl;
    dc << " Q0S NUMBEROFSOLUTIONS : " << options.getRequestedNumberOfSolutions() << endl;
    dc << " P21 IATA FARES : " << charIndicator(options.iataFares()) << endl;
    dc << " PBG JPSENTERED : " << charIndicator(options.jpsEntered()) << endl;
    dc << " PBM CALLTOAVAILABILITY : " << charIndicator(options.callToAvailability()) << endl;
    dc << " C47 ALTERNATECURRENCY : " << options.alternateCurrency() << endl;
    dc << " D54 TICKETTIMEOVERRIDE : " << options.ticketTimeOverride() << endl;
    dc << " P43 THRUFARES : " << charIndicator(options.thruFares()) << endl;
    dc << " NOM COUNTRYCODE : " << options.employment() << endl;
    dc << " A40 NATIONALITY : " << options.nationality() << endl;
    dc << " AHO RESIDENCY : " << options.residency() << endl;
    dc << " VTI VALIDATETICKETINGAGREEMENTIND : "
       << boolIndicator(options.validateTicketingAgreement()) << endl;

    if (trx.getTrxType() == PricingTrx::ESV_TRX)
    {
      dc << endl;
      dc << "ESV Options: " << endl;
      dc << " Q0E NOOFESVLOWFARESOLUTIONSREQ : "
         << shoppingTrx->esvOptions().noOfESVLowFareSolutionsReq() << endl
         << " Q60 NOOFMUSTPRICEONLINESOLUTIONS : "
         << shoppingTrx->esvOptions().noOfMustPriceOnlineSolutions() << endl
         << " Q61 NOOFMUSTPRICEINTERLINESOLUTIONS : "
         << shoppingTrx->esvOptions().noOfMustPriceInterlineSolutions() << endl
         << " Q62 NOOFMUSTPRICENONSTOPONLINESOLUTIONS : "
         << shoppingTrx->esvOptions().noOfMustPriceNonstopOnlineSolutions() << endl
         << " Q63 NOOFMUSTPRICENONSTOPINTERLINESOLUTIONS : "
         << shoppingTrx->esvOptions().noOfMustPriceNonstopInterlineSolutions() << endl
         << " Q64 NOOFMUSTPRICESINGLESTOPONLINESOLUTIONS : "
         << shoppingTrx->esvOptions().noOfMustPriceSingleStopOnlineSolutions() << endl
         << " Q65 STOPPENALTYINDOLLARS : " << shoppingTrx->esvOptions().perStopPenalty() << endl
         << " Q66 DURATIONPENALTYINDOLLARS : " << shoppingTrx->esvOptions().travelDurationPenalty()
         << endl << " Q67 DEPARTUREPENALTYINDOLLARS : "
         << shoppingTrx->esvOptions().departureTimeDeviationPenalty() << endl
         << " Q68 MAXALLOWEDOVERAGEPERCARRIER : " << shoppingTrx->esvOptions().percentFactor()
         << endl
         << " Q69 FLIGHTOPTIONREUSELIMIT : " << shoppingTrx->esvOptions().flightOptionReuseLimit()
         << endl << " Q6A UPPERBOUNDFACTORFORNOTNONSTOP : "
         << shoppingTrx->esvOptions().upperBoundFactorForNotNonstop() << endl
         << " Q6Q NOOFMUSTPRIVENON/1STOPONLINESOLUTIONS : "
         << shoppingTrx->esvOptions().noOfMustPriceNonAndOneStopOnlineSolutions() << endl
         << " Q6R NOOFMUSTPRIVENON/1STOPINTERLINESOLUTIONS : "
         << shoppingTrx->esvOptions().noOfMustPriceNonAndOneStopInterlineSolutions() << endl
         << " Q6S UPPERBOUNDFACTORFORNONSTOP : "
         << shoppingTrx->esvOptions().upperBoundFactorForNonstop() << endl
         << " Q6U UPPERBOUNDFACTORFORLFS : " << shoppingTrx->esvOptions().upperBoundFactorForLFS()
         << endl << " Q5T ESVPERCENT : " << shoppingTrx->esvOptions().esvPercent() << endl
         << " Q5U NOOFMINONLINESOLUTIONSPERCARRIER : "
         << shoppingTrx->esvOptions().noOfMinOnlinePerCarrier() << endl
         << " Q5V ONLINEPERCENT : " << shoppingTrx->esvOptions().onlinePercent() << endl
         << " Q5W LOWMAXIMUMPEROPTION (AVS) : " << shoppingTrx->esvOptions().loMaximumPerOption()
         << endl
         << " Q5X HIMAXIMUMPEROPTION (NO AVS) : " << shoppingTrx->esvOptions().hiMaximumPerOption()
         << endl << " S5A AVSPENALTYCARRIERS : " << shoppingTrx->esvOptions().avsCarriersString()
         << endl;
    }

    // Request
    //

    const PricingRequest& request = *trx.getRequest();

    // assuming we always have request
    //
    dc << endl;
    dc << "REQUEST:" << endl;
    dc << " TICKETINGDT : " << request.ticketingDT() << endl;
    dc << " AC0 CORPORATEID : " << request.corporateID() << endl;
    dc << " AF0 SALEPOINTOVERRIDE : " << request.salePointOverride() << endl;
    dc << " AG0 TICKETPOINTOVERRIDE : " << request.ticketPointOverride() << endl;
    dc << " D01 TICKETDATEOVERRIDE : " << request.ticketDateOverride() << endl;
    dc << " P77 EXEMPTALLTAXES : " << charIndicator(request.exemptAllTaxes()) << endl;
    dc << " P0J ELECTRONICTICKET : " << charIndicator(request.electronicTicket()) << endl;
    dc << " P0L ELECTRONICTICKET OFF AND NO OVERRIDE : "
       << charIndicator(request.eTktOffAndNoOverride()) << endl;
    dc << " P1V LOWFARENOAVAILABILITY : " << charIndicator(request.lowFareNoAvailability()) << endl;
    dc << " PA2 CONSIDERMULTIAIRPORT : " << charIndicator(request.considerMultiAirport()) << endl;
    dc << " S11 ACCOUNTCODE : " << request.accountCode() << endl;
    dc << " P53 EXEMPTSPECIFICTAXES : " << charIndicator(request.exemptSpecificTaxes()) << endl;
    dc << " P54 EXEMPTALLTAXES : " << charIndicator(request.exemptAllTaxes()) << endl;
    dc << " D07 TICKETINGDT : " << request.ticketingDT() << endl;
    dc << " B05 VALIDATINGCARRIER : " << request.validatingCarrier() << endl;
    if(!fallback::fallbackNonPreferredVC(&trx) && !request.nonPreferredVCs().empty())
    {
      dc << "NON-PREFERRED VALIDATING CARRIERS: ";
      dc << DiagnosticUtil::containerToString(request.nonPreferredVCs()) << std::endl;
    }
    if(!fallback::fallbackPreferredVC(&trx) && !request.preferredVCs().empty())
    {
      dc << "PREFERRED VALIDATING CARRIERS: ";
      dc << DiagnosticUtil::containerToString(request.preferredVCs()) << std::endl;
    }
    if(!fallback::fallbackNonBSPVcxrPhase1(&trx) || trx.overrideFallbackValidationCXRMultiSP())
    {
      switch(trx.getRequest()->spvInd())
      {
        case tse::spValidator::noSMV_noIEV:
          dc<<"SETTLEMENT PLAN VALIDATION:    F, IET VALIDATION:    F"<< std::endl;
          break;
        case tse::spValidator::noSMV_IEV:
          dc<<"SETTLEMENT PLAN VALIDATION:    F, IET VALIDATION:    T"<< std::endl;
          break;
        case tse::spValidator::SMV_IEV:
          dc<<"SETTLEMENT PLAN VALIDATION:    T, IET VALIDATION:    T"<< std::endl;
          break;
      }
      if(!trx.getRequest()->spvCxrsCode().empty())
        dc<<"NSP CARRIERS:    "<<DiagnosticUtil::containerToString(trx.getRequest()->spvCxrsCode()) << std::endl;
      if(!trx.getRequest()->spvCntyCode().empty())
        dc<<"NSP COUNTRIES:    "<<DiagnosticUtil::containerToString(trx.getRequest()->spvCntyCode()) << std::endl;
    }
    dc << " SEV PROCESSVITADATAIND : " << boolIndicator(request.processVITAData()) << endl;
    dc << " PXS EXPANDJUMPCABINLOGIC : " << boolIndicator(request.getJumpCabinLogic() != JumpCabinLogic::ENABLED) << endl;
    if (TrxUtil::isJumpCabinLogicDisableCompletely(trx))
      dc << "     JUMPCABINLOGICSTATUS : " << PricingRequest::jumpCabinLogicStatusToString(request.getJumpCabinLogic())  << endl;
    dc << " Q6F PERCENTOFLNGCNXSOLUTIONS : " << request.percentOfLngCnxSolutions() << endl;
    dc << "     MAXNUMOFLNGCNXSOLUTIONS : " << shoppingTrx->maxNumOfLngCnxSolutions() << endl;

    if (trx.getTrxType() == PricingTrx::IS_TRX)
    {
      dc << " PTF SUPPRESS SUM OF LOCALS ON SOL PATH: "
         << boolIndicator(request.isSuppressedSumOfLocals()) << endl;
      if (shoppingTrx)
        dc << "       SOL PROCESSING ENABLED : "
           << boolIndicator(shoppingTrx->isSumOfLocalsProcessingEnabled()) << endl;
      dc << " SLC CARNIVAL SUM OF LOCALS : " << boolIndicator(options.isCarnivalSumOfLocal())
         << endl;
      dc << " QD1 ADDITIONAL ITINS REQUESTED FOR OW FARES : "
         << options.getAdditionalItinsRequestedForOWFares() << endl;
      dc << " QD2 MAX ALLOWED USES OF FARE COMBINATION    : "
         << options.getMaxAllowedUsesOfFareCombination() << endl;
      dc << " B11 GOVERNING CARRIER OVERRIDE: " << request.governingCarrierOverride(0) << endl;
      dc << " QCU CUSTOM ROUTING AND CARRIER: " << shoppingTrx->getNumOfCustomSolutions() << endl;
      dc << " BFR S8 BRANDED FARE REQUEST: " << boolIndicator(request.isBrandedFaresRequest())
         << endl;
      dc << " SRL SCHEDULE REPEAT LIMIT: " << request.getScheduleRepeatLimit() << endl;
      dc << " IBF ALL FLIGHTS REPRESENTED DIVERSITY OPTION: "
         << boolIndicator(request.isAllFlightsRepresented()) << endl;
      dc << " ALL FLIGHTS DATA: " << boolIndicator(request.isAllFlightsData()) << endl;
      dc << " VCX VALIDATING CARRIER REQUEST: " << boolIndicator(trx.isValidatingCxrGsaApplicable()) << endl;
      dc << " SM0 SETTLEMENT METHOD OVERRIDE: " << request.getSettlementMethod() << endl;
      dc << " DVL ALTERNATE VALIDATING CARRIER REQUEST: " << boolIndicator(request.isAlternateValidatingCarrierRequest()) << std::endl;

    }

    if (trx.getTrxType() == PricingTrx::ESV_TRX)
    {
      dc << " ORIGINATION/DESTINATION DATA :" << endl;
      dc << "   D07 REQUESTEDDEPARTUREDT : " << request.requestedDepartureDT() << endl;
    }

    if (shoppingTrx)
      outputDiversityParameters(shoppingTrx->diversity());

    // tax override vector
    //
    std::vector<TaxOverride*>::const_iterator TOit = request.taxOverride().begin();
    std::vector<TaxOverride*>::const_iterator TOendIt = request.taxOverride().end();

    dc << " TAX OVERRIDE :" << endl;

    for (count = 1; TOit != TOendIt; ++TOit)
    {
      const TaxOverride* curTO = *TOit;
      if (curTO != nullptr)
      {
        dc << "   TAX OVERRIDE" << count << ":" << endl;
        dc << "   C6B TAXAMT : " << curTO->taxAmt() << endl;
        dc << "   BH0 TAXCODE : " << curTO->taxCode() << endl;
        ++count;
      }
    }

    // Ticketing Agent
    //

    const Agent* agent = request.ticketingAgent();

    // assuming we always have request
    //
    dc << " TICKETING AGENT:" << endl;

    outputAgentData(agent, dc);

    // Multi CorpID/Account code

    const std::vector<std::string>& corpIdVec = request.corpIdVec();
    std::vector<std::string>::const_iterator corpIdVecIt = corpIdVec.begin();

    dc << " MULTI CORP ID:" << endl;
    for (; corpIdVecIt != corpIdVec.end(); ++corpIdVecIt)
    {
      dc << "   " << *corpIdVecIt << endl;
    }

    const std::vector<std::string>& incorrectCorpIdVec = request.incorrectCorpIdVec();
    corpIdVecIt = incorrectCorpIdVec.begin();

    for (; corpIdVecIt != incorrectCorpIdVec.end(); ++corpIdVecIt)
    {
      dc << "   " << *corpIdVecIt << endl;
    }

    const std::vector<std::string>& accCodeVec = request.accCodeVec();
    corpIdVecIt = accCodeVec.begin();

    dc << " MULTI ACCOUNT CODE:" << endl;
    for (; corpIdVecIt != accCodeVec.end(); ++corpIdVecIt)
    {
      dc << "   " << *corpIdVecIt << endl;
    }

    // Pax Types
    //

    std::vector<PaxType*>::const_iterator PTit = trx.paxType().begin();
    std::vector<PaxType*>::const_iterator PTendIt = trx.paxType().end();

    dc << endl;
    dc << "Passenger Types:" << endl;

    for (count = 1; PTit != PTendIt; ++PTit)
    {
      const PaxType* curPT = *PTit;
      if (curPT != nullptr)
      {
        dc << " PAXTYPE " << count << ":" << endl;
        dc << "   B70 PAXTYPE : " << curPT->paxType() << endl;
        dc << "   Q0T AGE : " << curPT->age() << endl;
        dc << "   Q0U NUMBER : " << curPT->number() << endl;
        ++count;
      }
    }

    dc << endl;

    if (shoppingTrx != nullptr)
    {
      dc << "POS PAX TYPES:" << endl;
      for (std::vector<PosPaxTypePerPax>::const_iterator i = shoppingTrx->posPaxType().begin();
           i != shoppingTrx->posPaxType().end();
           ++i)
      {
        const size_t index = i - shoppingTrx->posPaxType().begin() + 1;
        dc << " POS PAX TYPE PER PAX " << index << ": " << endl;
        for (PosPaxTypePerPax::const_iterator j = i->begin(); j != i->end(); ++j)
        {
          const size_t index = j - i->begin() + 1;
          const PosPaxType& pos = **j;
          dc << "    POS PAX TYPE " << index << endl;
          dc << "    B70 PAXTYPE : " << pos.paxType() << endl;
          dc << "    A01 AGENTCITY : " << pos.pcc() << endl;
          dc << "    P33 POSITIVE : " << boolIndicator(pos.positive()) << endl;
          dc << "    AC0 CORPID : " << pos.corpID() << endl;
          dc << "    Q18 FGNUMBER : " << pos.fgNumber() << endl;
          dc << "    Q0C PRIORITY : " << int(pos.priority()) << endl << endl;
        }
      }
      // Requested CabinType

      std::vector<ShoppingTrx::Leg>::const_iterator legIt;
      int legCount = 0;

      dc << endl;
      for (legIt = shoppingTrx->legs().begin(); legIt != shoppingTrx->legs().end();
           legIt++, legCount++)
      {
        dc << "  PREFERRED CABIN FOR LEG NUMBER " << legCount << " IS "
           << (*legIt).preferredCabinClass().printName() << endl;
      }
    }

    if (rexShoppingTrx != nullptr)
    {
      dc << "REXSHOPPING INFORMATION DATA: " << endl;

      const RexPricingOptions& rexOptions =
          dynamic_cast<const RexPricingOptions&>(*rexShoppingTrx->getOptions());
      dc << endl;
      dc << "  EXCHANGE OPTIONS (EXC/PRO):" << endl;
      dc << "    NOM COUNTRYCODE : " << rexOptions.excEmployment() << endl;
      dc << "    A40 NATIONALITY : " << rexOptions.excNationality() << endl;
      dc << "    AHO RESIDENCY : " << rexOptions.excResidency() << endl;

      const RexPricingRequest& rexRequest =
          dynamic_cast<const RexPricingRequest&>(*rexShoppingTrx->getRequest());

      dc << "    B05 EXCVALIDATIONCARRIER : " << rexRequest.excValidatingCarrier() << endl
         << "    AF0 SALEPOINTOVERRIDE : " << rexRequest.salePointOverride() << endl
         << "    AG0 TICKETPOINTOVERRIDE : " << rexRequest.ticketPointOverride() << endl
         << "    S11 EXCACCOUNTCODE : " << rexRequest.excAccountCode() << endl
         << "    S94 WAIVERCODE : " << rexShoppingTrx->waiverCode() << endl;

      ExcItin* exchangeItin = rexShoppingTrx->exchangeItin().back();
      if (exchangeItin != nullptr)
      {
        dc << "    C6Y EXCITINCALCCURRENCY : " << exchangeItin->calculationCurrency() << endl
           << "    EXCITINCALCCUROVERR : " << exchangeItin->calcCurrencyOverride() << endl;
      }

      dc << endl;
      dc << "  REX SHOPPING NEW OPTIONS (PRO):" << endl;
      dc << "    D07 CURRENTTICKETINGDT : " << rexShoppingTrx->currentTicketingDT() << endl;
      dc << "    REX FAREAPPLDT :  " << rexShoppingTrx->fareApplicationDT() << endl;
      dc << "    C6Y NEWITINCURRENCYOVERR : "
         << rexShoppingTrx->newItin().back()->calcCurrencyOverride() << endl;
      dc << "    C6P BASEFARECURRENCY : " << rexOptions.baseFareCurrencyOverride() << endl;
      dc << "    D92 ORIGINALTKTISSUEDT : " << rexShoppingTrx->originalTktIssueDT() << endl;
      dc << "    D94 LASTTKTREISSUEDT : " << rexShoppingTrx->lastTktReIssueDT() << endl;
      dc << "    N25 PROCESS TYPE INDICATOR FOR PRIMARY REQUEST TYPE : "
         << boolIndicator(rexShoppingTrx->applyReissueExcPrimary()) << endl;
      dc << "    D95 PREVIOUS EXCHANGE DATE : " << rexShoppingTrx->previousExchangeDT() << endl;
      dc << "    C5A BASE FARE AMOUNT : " << rexOptions.excTotalFareAmt() << endl;

      dc << endl;
      dc << "  EXCHANGE PASSENGER TYPES:" << endl;

      for (PTit = rexShoppingTrx->excPaxType().begin(), count = 1;
           PTit != rexShoppingTrx->excPaxType().end();
           ++PTit)
      {
        outputRexPassengerData(*PTit, count, dc);
      }

      dc << endl;
      dc << "  EXCHANGE ACCOMPANY PASSENGER TYPES:" << endl;

      for (PTit = rexShoppingTrx->accompanyPaxType().begin(), count = 1;
           PTit != rexShoppingTrx->accompanyPaxType().end();
           ++PTit)
      {

        outputRexPassengerData(*PTit, count, dc);
      }

      const Agent* rexAgent = rexRequest.prevTicketIssueAgent();
      dc << endl;
      dc << "  EXCHANGE TICKETING AGENT:" << endl;

      outputAgentData(rexAgent, dc, true);

      dc << endl;
      outputExchangeItinData(*rexShoppingTrx, dc);

      outputExchangeFareComponentData(*rexShoppingTrx, dc);

      outputExchangePUPData(*rexShoppingTrx, dc);
    }

    // Billing
    //

    const Billing& billing = *trx.billing();

    // assuming we always have billing
    //
    dc << endl;
    dc << "BILLING:" << endl;
    dc << " A20 USERPSEUDOCITYCODE : " << billing.userPseudoCityCode() << endl;
    dc << " Q03 USERSTATION : " << billing.userStation() << endl;
    dc << " Q02 USERBRANCH : " << billing.userBranch() << endl;
    dc << " A0E PARTITIONID : " << billing.partitionID() << endl;
    dc << " AD0 USERSETADDRESS : " << billing.userSetAddress() << endl;
    dc << " A22 AAACITY : " << billing.aaaCity() << endl;
    dc << " AA0 AAASINE : " << billing.aaaSine() << endl;
    dc << " A70 ACTIONCODE : " << billing.actionCode() << endl;
    dc << " C01 CLIENTTRANID : " << billing.clientTransactionID() << endl;
    dc << " C02 PARENTTRANID : " << billing.parentTransactionID() << endl;
    dc << " C20 SERVICENAME : " << billing.serviceName() << endl;
    dc << " C21 CLIENTSERVICENAME : " << billing.clientServiceName() << endl;
    dc << " C22 PARENTSERVICENAME : " << billing.parentServiceName() << endl;

    if (trx.getTrxType() == PricingTrx::FF_TRX)
    {
      const FlightFinderTrx* ffTrx = dynamic_cast<const FlightFinderTrx*>(&trx);

      AirSeg* const firstJourneySeg =
          dynamic_cast<AirSeg* const>(ffTrx->journeyItin()->travelSeg().front());

      string projName = (ffTrx->isBffReq()) ? "BEST FARE FINDER" : "FLIGHT FINDER";
      dc << "-----------------------------" << endl;
      dc << projName << endl << " A11 BOARDCITY : " << firstJourneySeg->origin()->loc() << endl
         << " A12 OFFCITY : " << firstJourneySeg->destination()->loc() << endl
         << " B00 CARRIER : " << firstJourneySeg->carrier() << endl
         << " D01 OUTBOUNDDATE : " << firstJourneySeg->departureDT().dateToString(DDMMYYYY, "-")
         << endl << " D02 INBOUNDDATE : ";

      if (ffTrx->journeyItin()->travelSeg().size() > 1)
      {

        AirSeg* const secondJourneySeg =
            dynamic_cast<AirSeg*>(ffTrx->journeyItin()->travelSeg().back());
        dc << secondJourneySeg->departureDT().dateToString(DDMMYYYY, "-") << endl;
      }
      else
        dc << "NO DATE" << endl;

      if (ffTrx->isBffReq())
      {
        dc << "  N23 OW/RT : " << ffTrx->owrt() << endl
           << "  Q6T STEP : " << (short)ffTrx->bffStep() << endl
           << "  NAA NEEDAVL : " << boolIndicator(ffTrx->avlInS1S3Request()) << endl;

        if (ffTrx->avlInS1S3Request() || (ffTrx->bffStep() == FlightFinderTrx::STEP_1) ||
            (ffTrx->bffStep() == FlightFinderTrx::STEP_3))
        {
          dc << "-----------------------------" << endl << "BFF DATE RANGE: " << endl
             << " D01 DEPARTUREDATE : " << ffTrx->departureDT().dateToString(DDMMYYYY, "-") << endl
             << " Q4V NUMDAYSFORWAD : " << ffTrx->numDaysFwd() << endl;
        }
      }

      const std::vector<FlightFinderTrx::FareBasisCodeInfo>& faresInfo = ffTrx->fareBasisCodesFF();
      std::vector<FlightFinderTrx::FareBasisCodeInfo>::const_iterator faresInfoBeg =
          faresInfo.begin();

      for (; faresInfoBeg != faresInfo.end(); ++faresInfoBeg)
      {
        const FlightFinderTrx::FareBasisCodeInfo& fareBInfo = *faresInfoBeg;

        dc << "-----------------------------" << endl << "REQUESTED FARE: " << endl
           << " B50 FAREBASISCODE : " << fareBInfo.fareBasiscode << endl
           << " C75 FAREAMOUNT : " << fareBInfo.fareAmount << endl
           << " C76 FARECURRENCY : " << fareBInfo.currencyCode << endl;
      } // END_OF_FARES_INFO_BEG

      std::vector<PaxTypeCode>::const_iterator psgTypeI = ffTrx->reqPaxType().begin();
      for (; psgTypeI != ffTrx->reqPaxType().end(); ++psgTypeI)
      {
        dc << "REQUESTED PAX TYPE: " << endl << " B70 PAX TYPE : " << *psgTypeI << endl;
      }

      outputAltDatesList(trx, dc);

      if (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "SEGMENTS")
      {
        outputJourneyItinSegs(*ffTrx, dc);
      } // END_IF_FF_TRX
    }

    if (trx.getTrxType() == PricingTrx::RESHOP_TRX)
    {
      if (rexShoppingTrx)
      {
        dc << endl << "CXR LIST FROM PSS:" << endl;
        std::set<CarrierCode>::const_iterator cxrIter =
            rexShoppingTrx->cxrListFromPSS().cxrList.begin();
        for (; cxrIter != rexShoppingTrx->cxrListFromPSS().cxrList.end(); ++cxrIter)
        {
          dc << " B00 CARRIERCODE : " << *cxrIter << endl;
        }
        dc << " EXL EXCLUDE : " << boolIndicator(rexShoppingTrx->cxrListFromPSS().excluded) << endl
           << endl;
      }
    }

    if (trx.getTrxType() == PricingTrx::IS_TRX && trx.excTrxType() == PricingTrx::EXC_IS_TRX)
    {
      if (shoppingTrx)
      {
        dc << endl << "FORCED CONNECTION:" << endl;
        ShoppingTrx::ForcedConnectionSet::const_iterator locIter =
            shoppingTrx->forcedConnection().begin();
        for (; locIter != shoppingTrx->forcedConnection().end(); ++locIter)
        {
          dc << " AJ0 LOCATIONCODE : " << *locIter << endl;
        }
      }
    }

    // Other
    //

    dc << endl;
    dc << "OTHER:" << endl;
    dc << " Q0A DIAGNOSTIC : " << trx.diagnostic().diagnosticType() << endl;

    dc << endl;

    dc << "CALLENDAR SHOPPING FOR INTERLINES:" << endl;
    dc << "  PA0 ENABLE_CALENDAR_FOR_INTERLINES             : "
       << (options.isEnableCalendarForInterlines() ? 'T' : 'F') << endl;
    dc << "EXCLUDE FARE FOCUS RULES: "
       << (options.isExcludeFareFocusRule() ? 'T' : 'F') << std::endl;
    dc << endl;
    dc << "******************************************************" << endl;
    dc << endl;
  }

  return (*this);
}

void
Diag905Collector::outputRexPassengerData(const PaxType* curPT, int& count, DiagCollector& dc)
{
  if (curPT != nullptr)
  {
    dc << "   PAXTYPE " << count << ":" << endl;
    dc << "     B70 PAXTYPE : " << curPT->paxType() << endl;
    dc << "     Q0T AGE : " << curPT->age() << endl;
    dc << "     Q0U NUMBER : " << curPT->number() << endl;
    dc << "     A30 STATE CODE : " << curPT->stateCode() << endl;
    ++count;
  }
}

void
Diag905Collector::outputAgentData(const Agent* agent,
                                  DiagCollector& dc,
                                  bool displayRexShoppingData)
{
  if (agent == nullptr)
    return;

  dc << "    A10 AGENTCITY : " << agent->agentCity() << endl;
  dc << "    A20 TVLAGENCYPCC : " << agent->tvlAgencyPCC() << endl;
  dc << "    A21 MAINTVLAGENCYPCC : " << agent->mainTvlAgencyPCC() << endl;
  dc << "    AB0 TVLAGENCYIATA : " << agent->tvlAgencyIATA() << endl;
  dc << "    AB1 HOMEAGENCYIATA : " << agent->homeAgencyIATA() << endl;

  if (!agent->officeDesignator().empty())
    dc << "    AE1 OFFICE DESIGNATOR : " << agent->officeDesignator() << endl;
  if (!agent->officeStationCode().empty())
    dc << "    AE2 OFFICE/STATION CODE : " << agent->officeStationCode() << endl;
  if (!agent->defaultTicketingCarrier().empty())
    dc << "    AE3 DEFAULT TICKETING CARRIER : " << agent->defaultTicketingCarrier() << endl;

  dc << "    A90 AGENTFUNCTIONS : " << agent->agentFunctions() << endl;
  dc << "    A80 AIRLINEDEPT : " << agent->airlineDept() << endl;
  dc << "    N0G AGENTDUTY : " << agent->agentDuty() << endl;
  dc << "    B00 CXRCODE : " << agent->cxrCode() << endl;
  dc << "    C40 CURRENCYCODEAGENT : " << agent->currencyCodeAgent() << endl;
  dc << "    Q01 COHOSTID : " << agent->coHostID() << endl;

  if (displayRexShoppingData) // RexShopping transaction
  {
    dc << "   N0L AGENTCOMMISSIONTYPE : " << agent->agentCommissionType() << endl;
    dc << "   C6C AGENTCOMMISIONAMT : " << agent->agentCommissionAmount() << endl;
  }
}

void
Diag905Collector::outputExchangeItinData(const RexShoppingTrx& rexShoppingTrx, DiagCollector& dc)
{
  dc << " EXCHANGE ITIN: " << endl;

  ExcItin* excItin = rexShoppingTrx.exchangeItin().front();
  if (excItin == nullptr)
  {
    return;
  }
  int index = 1;
  for (std::vector<TravelSeg*>::const_iterator j = excItin->travelSeg().begin();
       j != excItin->travelSeg().end();
       ++j, ++index)
  {
    const AirSeg* seg = dynamic_cast<const AirSeg*>(*j);
    if (seg == nullptr)
    {
      continue;
    }

    dc << "   TRAVEL SEG " << index << ":\n";
    outputAirSeg(nullptr, index, *seg, "     ", true);
  }
}

void
Diag905Collector::outputExchangeFareComponentData(const RexShoppingTrx& rexShoppingTrx,
                                                  DiagCollector& dc)
{
  dc << " EXCHANGE FARE COMPONENTS: " << endl;

  ExcItin* excItin = rexShoppingTrx.exchangeItin().front();

  if (excItin == nullptr)
  {
    return;
  }
  for (const auto fareComponent : excItin->fareComponent())
  {
    dc << "   Q6D FARECOMPNUMBER : " << fareComponent->fareCompNumber() << endl;
    dc << "   B50 FAREBASISCODE  : " << fareComponent->fareBasisCode() << endl;
    dc << "   S37 VTCRVENDOR : " << fareComponent->VCTR().vendor() << endl;
    dc << "   B09 VTCRCARRIER : " << fareComponent->VCTR().carrier() << endl;
    dc << "   S89 VTCRTARIFF : " << fareComponent->VCTR().tariff() << endl;
    dc << "   S90 VTCRRULE : " << fareComponent->VCTR().rule() << endl;
    dc << "   HASVTCR : " << boolIndicator(fareComponent->hasVCTR()) << endl;
  }
}

void
Diag905Collector::outputExchangePUPData(const RexShoppingTrx& rexShoppingTrx, DiagCollector& dc)
{
  dc << " EXCHANGE PLUS UP:" << endl;

  ExcItin* excItin = rexShoppingTrx.exchangeItin().front();

  if ((excItin == nullptr) || (excItin->fareComponent().empty()) ||
      (excItin->fareComponent().back() == nullptr))
  {
    return;
  }

  FareCompInfo* fareComponent = excItin->fareComponent().back();

  const MinimumFareOverride* minFareOverride =
      dynamic_cast<const MinimumFareOverride*>(fareComponent->hip());

  if (minFareOverride == nullptr)
  {
    return;
  }

  dc << "   C6L PLUSUPAMOUNT : " << minFareOverride->plusUpAmount << endl;
  dc << "   A11 BOARDPOINT  : " << minFareOverride->boardPoint << endl;
  dc << "   A12 OFFPOINT  : " << minFareOverride->offPoint << endl;
  dc << "   A13 FAREBOARDPOINT  : " << minFareOverride->fareBoardPoint << endl;
  dc << "   A14 FAREOFFPOINT  : " << minFareOverride->fareOffPoint << endl;
  dc << "   A18 CONSTRUCTPOINT : " << minFareOverride->constructPoint << endl;
  dc << "   A19 CONSTRUCTPOINT2 : " << minFareOverride->constructPoint2 << endl;
}

void
Diag905Collector::outputJourneyItinSegs(const FlightFinderTrx& ffTrx, DiagCollector& dc)
{
  std::vector<TravelSeg*>::const_iterator segIterBeg = ffTrx.journeyItin()->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator segIterEnd = ffTrx.journeyItin()->travelSeg().end();

  for (; segIterBeg != segIterEnd; ++segIterBeg)
  {
    AirSeg* const journeySeg = dynamic_cast<AirSeg*>(*segIterBeg);
    printSegment(journeySeg, dc);
  }

  if (ffTrx.isAltDates())
  {
    dc << "---------------------------------" << endl << "ALTERNATE_DATE_JOURNEY_ITINS:" << endl;

    std::map<DatePair, PricingTrx::AltDateInfo*>::const_iterator altDateIter =
        ffTrx.altDatePairs().begin();

    for (; altDateIter != ffTrx.altDatePairs().end(); altDateIter++)
    {
      DatePair myPair = (*altDateIter).first;
      Itin* journeyItin = (*altDateIter).second->journeyItin;

      dc << "--------------------------------------" << endl
         << "  DATE_PAIR : " << myPair.first.dateToString(DDMMYYYY, "-") << " , "
         << myPair.second.dateToString(DDMMYYYY, "-") << endl;

      std::vector<TravelSeg*>::const_iterator segIterBeg = journeyItin->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator segIterEnd = journeyItin->travelSeg().end();

      for (; segIterBeg != segIterEnd; ++segIterBeg)
      {
        const AirSeg* const journeySeg = dynamic_cast<AirSeg*>(*segIterBeg);
        printSegment(journeySeg, dc);
      }
    } // END for
  }
}

void
Diag905Collector::printSegment(const AirSeg* journeySeg, DiagCollector& dc)
{
  const std::string geoTravelType = DiagnosticUtil::geoTravelTypeToString(journeySeg->geoTravelType());

  dc << "--------------------------------------" << endl << "JOURNEY_ITIN SEGMENT:" << endl
     << "  BOARDCITY : " << journeySeg->origin()->loc() << endl
     << "  OFFCITY : " << journeySeg->destination()->loc() << endl
     << "  CARRIER : " << journeySeg->carrier() << endl
     << "  DEPARTURE_DATE :" << journeySeg->departureDT().dateToString(DDMMYYYY, "-") << endl
     << "  BOOKING_DATE : " << journeySeg->bookingDT().dateToString(DDMMYYYY, "-") << endl
     << "  ORIGAIRPORT : " << journeySeg->origAirport() << endl
     << "  DESTAIRPORT : " << journeySeg->destAirport() << endl
     << "  BOARDMULTICITY :" << journeySeg->boardMultiCity() << endl
     << "  OFFMULTICITY : " << journeySeg->offMultiCity() << endl
     << "  GEOTRAVELTYPE : " << geoTravelType << endl
     << "  PNRSEGMENT : " << journeySeg->pnrSegment() << endl;
}

void
Diag905Collector::outputAltDatesList(const PricingTrx& trx, DiagCollector& dc)
{
  if (trx.isAltDates())
  {
    dc << "-----------------------------" << endl << "ALTERNATE_DATE_LIST:" << endl;

    std::map<DatePair, PricingTrx::AltDateInfo*>::const_iterator altDateIter;
    DateTime prevOutbound(DateTime::emptyDate());

    for (const auto& elem : trx.altDatePairs())
    {
      DatePair myPair = elem.first;

      if (prevOutbound != myPair.first)
      {
        dc << "  D17 OUTBOUND_DATE : " << myPair.first.dateToString(DDMMYYYY, "-") << endl
           << "  INBOUND_DATE_LIST:" << endl;
      }

      dc << "   D18 INBOUND_DATE : " << myPair.second.dateToString(DDMMYYYY, "-") << endl;
      prevOutbound = myPair.first;
    }
  }
  else
    dc << "NO ALTERNATE DATES LIST" << endl;
}

void
Diag905Collector::outputAirSeg(const ShoppingTrx::SchedulingOption* sop,
                               int segNum,
                               const AirSeg& seg,
                               const std::string& indent,
                               bool displayRexShoppingData)
{
  DiagCollector& dc(*this);
  dc << indent << "B00 CARRIER : " << seg.carrier() << endl;
  dc << indent << "B01 OPERATINGCARRIERCODE : " << seg.operatingCarrierCode() << endl;
  dc << indent << "Q0B FLIGHTNUMBER : " << seg.flightNumber() << endl;
  dc << indent << "S05 EQUIPMENTTYPE : " << seg.equipmentType() << endl;
  dc << indent << "A01 ORIGAIRPORT : " << seg.origAirport() << endl;
  dc << indent << "D01/D31 DEPARTUREDT : " << seg.departureDT() << endl;
  dc << indent << "A02 DESTAIRPORT : " << seg.destAirport() << endl;
  dc << indent << "D02/D32 ARRIVALDT : " << seg.arrivalDT() << endl;
  dc << indent << "PNRSEGMENT : " << seg.pnrSegment() << endl;
  dc << indent << "SEGMENTORDER : " << seg.segmentOrder() << endl;
  dc << indent << "STOPOVER : " << boolIndicator(seg.stopOver()) << endl;

  if (displayRexShoppingData)
  {
    dc << indent << "P72 FORCEDCONX : " << boolIndicator(seg.forcedConx()) << endl;
    dc << indent << "P73 FORCEDSTOPOVER : " << boolIndicator(seg.forcedStopOver()) << endl;
    dc << indent << "PCI SEGMENTFLOWN : " << boolIndicator(seg.unflown()) << endl;
    dc << indent << "SSH SEGMENTSHOPPED : " << boolIndicator(seg.isShopped()) << endl;
    dc << indent << "B50 FAREBASISCODE : " << seg.fareBasisCode() << endl;
    dc << indent << "C50 FARECALCFAREAMT : " << seg.fareCalcFareAmt().c_str() << endl;
  }
}

void
Diag905Collector::outputDiversityParameters(const Diversity& dvr)
{
  DiagCollector& dc(*this);
  dc << "DIVERSITY PARAMETERS:" << endl;
  dc << "    IS ENABLED: " << boolIndicator(dvr.isEnabled()) << endl;
  dc << "    MODEL: " << DiversityModelType::getName(dvr.getModel()) << endl;
  dc << "    TOD TIME OF DATE RANGE: ";
  for (size_t i = 0; i < dvr.getTODRanges().size(); ++i)
    dc << " " << i << ": " << std::setfill('0') << std::setw(2) << dvr.getTODRanges()[i].first / 60
       << std::setw(2) << dvr.getTODRanges()[i].first % 60 << "-" << std::setw(2)
       << dvr.getTODRanges()[i].second / 60 << std::setw(2) << dvr.getTODRanges()[i].second % 60
       << std::setfill(' ') << std::setw(0);
  dc << endl;
  dc << "    FAC FARE CUT-OFF COEFFICIENT: ";
  if (dvr.getFareCutoffCoef() < 1.0)
    dc << "NOT SET";
  else
    dc << dvr.getFareCutoffCoef();
  dc << "\n";
  if (dvr.isOptionsPerCarrierInPercents())
    dc << "    PCP OPTIONS PER CARRIER PERCENTAGE: " << -dvr.getOptionsPerCarrierDefault();
  else
    dc << "    OPC MINIMAL NUMBER OF OPTIONS PER CARRIER: " << dvr.getOptionsPerCarrierDefault();
  dc << '\n';
  dc << "    NSO NON STOP OPTIONS PERCENTAGE: " << dvr.getNonStopOptionsPercentage() << endl;
  dc << "    NSV NON STOP OPTIONS COUNT: " << dvr.getNonStopOptionsCount() << endl;
  dc << "    IOP INBOUND/OUTBOUND PAIRING: " << dvr.getInboundOutboundPairing() << endl;
  dc << "    FLN FARE LEVEL NUMBER: " << dvr.getFareLevelNumber() << endl;
  dc << "    FAS FARE AMOUNT SEPARATOR COEFFICIENT: ";
  if (dvr.getFareAmountSeparatorCoef() < 1.0)
    dc << "NOT SET";
  else
    dc << dvr.getFareAmountSeparatorCoef();
  dc << "\n";
  dc << "    TTS TIME TRAVEL SEPARATOR COEFFICIENT: ";
  if (!dvr.getTravelTimeSeparatorCoefDefined())
    dc << "NOT SET";
  else
    dc << dvr.getTravelTimeSeparatorCoef();
  dc << "\n";
  dc << "    TTD TOD DISTRIBUTION: ";
  for (const auto elem : dvr.getTODDistribution())
    dc << " " << elem;
  dc << endl;
  dc << "    BKD BUCKET DISTRIBUTION: ";
  for (size_t i = 0; i < Diversity::BUCKET_COUNT; ++i)
    dc << " " << dvr.getBucketDistribution((Diversity::BucketType)i);
  dc << endl;
  dc << "    PFC PREFERRED CARRIERS: ";
  for (const auto elem : dvr.getPreferredCarriers())
    dc << " " << elem;
  dc << endl;
  dc << "    FRL FLIGHT REPEAT LIMIT: " << dvr.getFlightRepeatLimit() << endl;
  dc << "    CXP CARRIER SPECIFIC NUMBER OF OPTIONS:";
  for (const auto& elem : dvr.getOptionsPerCarrierMap())
  {
    dc << " " << elem.first << ":" << elem.second;
  }
  dc << endl;
  dc << "    HIGH DENSITY NON-STOP MARKET: " << (boost::indeterminate(dvr.isHighDensityMarket())
                                                     ? "U"
                                                     : (dvr.isHighDensityMarket() ? "T" : "F"))
     << endl;
}
}
