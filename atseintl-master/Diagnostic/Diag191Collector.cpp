//----------------------------------------------------------------------------
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
#include "Diagnostic/Diag191Collector.h"

#include "Common/FallbackUtil.h"
#include "Common/ValidatingCxrConst.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/TicketingCxrDisplayRequest.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DBAccess/CountrySettlementPlanInfo.h"


#include <string>

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(fallbackNonBSPVcxrPhase1);

namespace
{
const std::string header = "*** START OF VALIDATING CXR DIAG ***\n";
const std::string footer = "*** END OF VALIDATING CXR DIAG ***\n";
}

void
Diag191Collector::print191Header(const PricingTrx* trx,
                                 const Itin* itin,
                                 const std::string& hashString,
                                 bool isSopProcessing,
                                 const SopIdVec* sops)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  if (!trx || hashString.empty()) // Pricing has hashString empty
    dc << "\n" << header;
  else
  {
    if (trx->getTrxType() == PricingTrx::MIP_TRX)
      dc << "\n\n** PROCESSING ITIN " << itin->itinNum() << " -";
    else if (trx->getTrxType() == PricingTrx::IS_TRX)
    {
      if (isSopProcessing)
        dc << "\n  SOP COMBINATION";
      else
        dc << "\n** PROCESSING SHOPPING FARE PATH -";
    }

    if (!hashString.empty())
    {
      if(!trx->getRequest()->isMultiTicketRequest())
      {
        dc << " HASH STRING: " << hashString;
      }
      else
      {
        dc << " \n" << std::endl;
        dc << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " << std::endl;
        dc << " \n" << std::endl;
        dc << "TKT" << itin->getMultiTktItinOrderNum() << ": " << hashString ;
      }
    }
    if (!isSopProcessing)
      dc << " **";

    if (sops)
      dc << "\n  SOPS IN THE FAREPATH:" << printSops(trx, sops);
  }

  dc << " \n\n";
}

std::string
Diag191Collector::printSops(const PricingTrx* trx, const SopIdVec* sops)
{
  std::ostringstream ss;
  const ShoppingTrx* shoppingTrx = dynamic_cast<const ShoppingTrx*>(trx);
  if (shoppingTrx && sops)
  {
    const SopIdVec& sopVector = *sops;
    for (uint32_t legNo = 0; legNo < sopVector.size(); ++legNo)
    {
      const ShoppingTrx::SchedulingOption& sop = shoppingTrx->legs()[legNo].sop()[sopVector[legNo]];
      ss << ((legNo == 0) ? " " : ",") << sop.originalSopId();
    }
  }

  return ss.str();
}
void
Diag191Collector::print191Footer()
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << " \n\n";
    dc << footer;
  }
}


Diag191Collector&
Diag191Collector::operator << (const TicketingCxrTrx& trx)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << header;
    dc << footer;
  }
  return *this;
}

void
Diag191Collector::displaySettlementPlanInfo(const CountrySettlementPlanInfo& cspi)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " \n" ;
  dc << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " << " \n";
  dc << "PLAN TYPE: " << cspi.getSettlementPlanTypeCode();
  dc << "  NATION: " << cspi.getCountryCode() << "\n";
  dc << " \n";
}

void
Diag191Collector::displayCountrySettlementPlanInfo(const CountrySettlementPlanInfo& cspi)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc << "** COUNTRY SETTLEMENT PLAN INFORMATION FOR NATION " << cspi.getCountryCode() << " **\n";
  dc << " \n";
  dc << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " << " \n";
  dc << "PLAN TYPE: " << cspi.getSettlementPlanTypeCode();
  dc << "  NATION: " << cspi.getCountryCode() << "\n";
  dc << " \n";
}

void
Diag191Collector::displayInterlineAgreements(const CarrierCode& vcxr,
                                             const vcx::ValidatingCxrData& vcxrData,
                                             bool isVcxrPartOfItin,
                                             vcx::TicketType requestedTicketType,
                                             const std::string& failureText,
                                             const vcx::Pos* pos)
{
  if (!isActive())
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  const char isPartOfItin = isVcxrPartOfItin ? 'Y' : 'N';
  if (pos)
    dc << "\n  INTERLINE AGREEMENT INFO FOR " << vcxr <<" IN:"<<pos->country << std::endl;
  else
    dc << "\n  INTERLINE AGREEMENT INFO FOR " << vcxr <<":" << std::endl;
  dc << "  PARTICIPATES IN ITIN: " << isPartOfItin << std::endl;
  dc << "  TICKET TYPE: " << vcx::getTicketTypeText(vcxrData.ticketType) << std::endl;
  dc << "  REQUESTED TICKET TYPE: " << vcx::getTicketTypeText(requestedTicketType) << std::endl;

  for (const vcx::ParticipatingCxr& pcx : vcxrData.participatingCxrs)
  {
    dc << "  PARTICIPATING CARRIER: " << pcx.cxrName
       << "  AGREEMENT: " << vcx::getITAgreementTypeCodeText(pcx.agmtType) << std::endl;
  }

  if (!failureText.empty())
    dc << "  " << failureText << std::endl;

  dc << std::endl;
}

void
Diag191Collector::printValidatingCxrList(const PricingTrx& trx, const Itin& itin, bool isSopProcessing)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc << "  \n* ";
  dc << (isSopProcessing ? "SOP COMBINATION" : "ITIN");

  if (trx.useTraditionalValidatingCxr())
    dc << " CARRIER/S PASSING IET CHECK  *\n";
  else
    dc << " POTENTIAL VALIDATING CARRIER LIST *\n";

  dc << "  \n";

    if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
    {
      if (itin.spValidatingCxrGsaDataMap())
      {
        for (const auto& spGsaItem : *itin.spValidatingCxrGsaDataMap())
        {
          if (!spGsaItem.second)
            continue;
          for (const auto& valCxrItem : spGsaItem.second->validatingCarriersData())
          {
            dc << "  " << spGsaItem.first;
            dc << "  VALIDATING CARRIER: " << valCxrItem.first;
            dc << "   TICKETING METHOD: " << vcx::getTicketTypeText(valCxrItem.second.ticketType) << "\n";
          }
        }
      }
    }
    else
    {
      std::map<CarrierCode, vcx::ValidatingCxrData>::const_iterator i,
        iEnd = itin.validatingCxrGsaData()->validatingCarriersData().end();

      for (i = itin.validatingCxrGsaData()->validatingCarriersData().begin(); i != iEnd; ++i)
      {
        dc << "  VALIDATING CARRIER: " << i->first;
        dc << "   TICKETING METHOD: " << vcx::getTicketTypeText(i->second.ticketType) << "\n";
      }
    }
}
void
Diag191Collector::printDiagMsg(const PricingTrx& trx,
                               const Itin& itin,
                               const CarrierCode& specifiedVC)
{
  DiagCollector& dc = (DiagCollector&)*this;

    if (specifiedVC.empty())
    {
      if (trx.useTraditionalValidatingCxr())
        dc << "** PROCESSING TRADITIONAL VALIDATING CARRIER " << itin.validatingCarrier();
      else
        dc << "** PROCESSING POTENTIAL VALIDATING CARRIERS";
    }
    else
     dc << "** PROCESSING SPECIFIED VALIDATING CARRIER " << specifiedVC;


  if (trx.getTrxType() == PricingTrx::MIP_TRX)
       dc << " FOR ITIN " << itin.itinNum();

  dc << " **\n";
  dc << " \n";
}

void
Diag191Collector::printHeaderForPlausibilityReq(const TicketingCxrTrx& tcsTrx)
{
  if (!isActive())
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "COUNTRY: " << tcsTrx.getCountry() << "\n";

  if ( tcsTrx.getRequest()->isArcUser() )
  {
    dc << "COUNTRY US USED FOR ARC USER " << tcsTrx.getRequest()->getPcc() << "\n";
  }

  dc << "PRIME HOST: " << tcsTrx.getRequest()->getMultiHost() << "\n"
     << "SPECIFIED VALIDATING CXR: " << tcsTrx.getRequest()->getValidatingCxr() << "\n\n";
}

void
Diag191Collector::printPlausibilityResult(const TicketingCxrTrx& tcsTrx)
{
  if (!isActive())
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  std::string msg;
  if (tcsTrx.isResultValid())
  {
    msg = "VALIDATING CARRIER - ";
    if (tcsTrx.isMultipleGSASwap())
      dc << msg << "\n";
  }
  tcsTrx.buildMessageText(msg);
  dc << "\n\n" << msg << std::endl;
}

void
Diag191Collector::printPaperConflict(const TicketingCxrTrx& tcsTrx)
{
  if (!isActive()) return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "  TICKET TYPE: " <<
    vcx::getTicketTypeText(tcsTrx.vcxrData().ticketType) << std::endl;
  dc << "  REQUESTED TICKET TYPE: " <<
    vcx::getTicketTypeText(tcsTrx.getRequest()->getTicketType()) << std::endl;
  dc << "  PAPER_TKT_OVERRIDE_ERR" << std::endl;
  dc << std::endl;
}

void
Diag191Collector::printHeaderForDisplayReq(const TicketingCxrDisplayTrx& trx)
{
  if (!isActive())
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  if (trx.getRequest())
  {
    dc << "\n*** POINT OF SALE ***\n";
    dc << "  PRIME HOST: " << trx.getRequest()->pointOfSale().primeHost() << std::endl;
    dc << "  COUNTRY: " << trx.getRequest()->pointOfSale().country() << std::endl;
    dc << "  PCC: " << trx.getRequest()->pointOfSale().pcc() << std::endl;

    if (vcx::DISPLAY_INTERLINE_AGMT==trx.getRequest()->getRequestType())
    {
      dc << "\n*** DISPLAY INTERLINE AGMT ***\n";
      dc << "  VALIDATING CARRIER: " << trx.getRequest()->validatingCxr()<<std::endl;
    }
    else if (vcx::DISPLAY_VCXR==trx.getRequest()->getRequestType())
    {
      dc << "\n*** DISPLAY VALIDATING CXR ***\n";
    }
    else
    {
      dc << "\n*** ERROR: UNKNOWN DISPLAY COMMAND\n";
      return;
    }
    dc << "  SPECIFIED COUNTRY: " << trx.getRequest()->specifiedCountry()<<std::endl;
    dc << "  SPECIFIED PRIME HOST: " << trx.getRequest()->specifiedPrimeHost()<<std::endl;
    dc << "  SPECIFIED SETTLEMENT PLAN: " << trx.getRequest()->settlementPlan()<<std::endl;
  }
  dc << std::endl;
}

void
Diag191Collector::print191DefaultValidatingCarrier(const PricingTrx& trx , Itin & itin, bool isShopping)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    if(trx.useTraditionalValidatingCxr())
    {
       std::set<CarrierCode> ret;
       std::set<CarrierCode>::iterator it;
       ValidatingCxrUtil::getValidatingCxrsFromMarketingCxr(itin, itin.traditionalValidatingCxr(), ret);
       dc << " \n\n  POTENTIAL TRADITIONAL VALIDATING CARRIER/S: ";

      if (ret.empty())
       {
         dc << itin.traditionalValidatingCxr() << "\n";
       }
       else
       {
         for (const auto& it : ret)
         {
           dc << it << " ";
         }
         dc << " \n";
       }
     }
     else
       dc << " \n\n  ITIN DEFAULT VALIDATING CARRIER: " << itin.validatingCarrier() << "\n";

     if (!isShopping)
     {
       dc << " \n\n";
       dc << footer;
     }
  }
}

} // end namespace tse
