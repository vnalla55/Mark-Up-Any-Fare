// ----------------------------------------------------------------------------
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
// ----------------------------------------------------------------------------

#include "Common/TrxUtil.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Diagnostic/Diag807Collector.h"
#include "Diagnostic/DiagManager.h"
#include "PfcTaxesExemption/AutomaticPfcTaxExemption.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/TktDesignatorExemptInfo.h"
#include "Taxes/Pfc/PfcItem.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

namespace tse
{
namespace PFCTaxExemption
{
struct FarePreValidation
{
  FarePreValidation(const PaxTypeFare* ptf, const TktDesignator& tktDesignator)
    : _ptf(ptf), _tktDesignator(tktDesignator)
  {
  }
  const PaxTypeFare* _ptf;
  TktDesignator _tktDesignator;
};

typedef std::vector<FarePreValidation> FarePreValidationVector;

bool
isCandidate(PricingTrx& trx,
            const FarePath& farePath,
            FarePreValidationVector& farePreValidationVector,
            Diag807Collector* diag)
{
  if (!TrxUtil::isAutomaticPfcTaxExemptionEnabled(trx))
  {
    return false;
  }

  if (diag)
  {
    *diag << "FARE COMPONENT VALIDATION SCOPE" << std::endl;
  }
  bool isNotFBRFare = false;
  bool tktDesPresent = false;
  std::vector<PricingUnit*>::const_iterator puIt(farePath.pricingUnit().begin());
  std::vector<PricingUnit*>::const_iterator puItEnd(farePath.pricingUnit().end());
  std::vector<const PaxTypeFare*> paxTypeFares;

  for (; puIt != puItEnd; ++puIt)
  {
    const PricingUnit& pricingUnit(**puIt);
    std::vector<FareUsage*>::const_iterator fuIt(pricingUnit.fareUsage().begin());
    std::vector<FareUsage*>::const_iterator fuItEnd(pricingUnit.fareUsage().end());
    for (; fuIt != fuItEnd; ++fuIt)
    {
      tktDesPresent = false;
      const PaxTypeFare* ptf((*fuIt)->paxTypeFare());
      if (diag)
      {
        DiagCollector& dc = *diag;
        dc.setf(std::ios::left, std::ios::adjustfield);

        dc << " " << std::endl;
        dc << std::setw(3) << ptf->fareMarket()->origin()->loc() << '-';
        dc << std::setw(3) << ptf->fareMarket()->destination()->loc() << "  ";
        dc << std::setw(4) << ptf->carrier() << "   ";
        dc << std::setw(10) << ptf->fareClass() << "  ";
      }
      if (!ptf->isFareByRule())
      {
        isNotFBRFare = true;
        if (diag)
          *diag << "          FARE TYPE: *NON CAT25*" << std::endl;
      }
      else if (diag)
      {
        *diag << "          FARE TYPE: CAT25" << std::endl;
      }
      if (ptf->isFareByRule())
      {
        const TktDesignator& tktDesignator8(ptf->fbrApp().tktDesignator());
        if (diag)
        {
          *diag << "CAT 25 REC 8 TKT DES    : " << tktDesignator8 << std::endl;
        }
        if (!tktDesignator8.empty())
        {
          farePreValidationVector.push_back(FarePreValidation(ptf, tktDesignator8));
          tktDesPresent = true;
        }
      }
      else if (diag)
      {
        *diag << "CAT 25 REC 8 TKT DES    : " << std::endl;
      }
      if (ptf->isNegotiated())
      {
        const TktDesignator& tktDesignator35(ptf->negotiatedInfo().tktDesignator1());

        if (diag)
        {
          *diag << "CAT 35 REC 3 TKT DES    : " << tktDesignator35 << std::endl;
        }
        if (!tktDesPresent && !tktDesignator35.empty())
        {
          farePreValidationVector.push_back(FarePreValidation(ptf, tktDesignator35));
          tktDesPresent = true;
        }
      }
      else if (diag)
      {
        *diag << "CAT 35 REC 3 TKT DES    : " << std::endl;
      }
      if (ptf->isDiscounted())
      {
        const TktDesignator& tktDesignator19(ptf->discountInfo().tktDesignator());

        if (diag)
        {
          *diag << "CAT 19-22 REC 3 TKT DES : " << tktDesignator19 << std::endl;
        }
        if (!tktDesPresent && !tktDesignator19.empty())
        {
          farePreValidationVector.push_back(FarePreValidation(ptf, tktDesignator19));
          tktDesPresent = true;
        }
      }
      else if (diag)
      {
        *diag << "CAT 19-22 REC 3 TKT DES : " << std::endl;
      }
      if (ptf->isFareByRule())
      {
        const TktDesignator& tktDesignatorFBR(ptf->fareByRuleInfo().tktDesignator());

        if (diag)
        {
          *diag << "CAT 25 REC 3 TKT DES    : " << tktDesignatorFBR << std::endl;
        }

        if (!tktDesPresent && !tktDesignatorFBR.empty())
        {
          farePreValidationVector.push_back(FarePreValidation(ptf, tktDesignatorFBR));
          tktDesPresent = true;
        }
      }
      else if (diag)
      {
        *diag << "CAT 25 REC 3 TKT DES    : " << std::endl;
      }
    }
  }
  if (isNotFBRFare)
  {
    if (diag)
    {
      *diag << " " << std::endl
            << "************************************************************" << std::endl
            << "VALIDATION RESULT: FAIL - ITIN INCLUDES NON CAT25 FARES     " << std::endl
            << "************************************************************" << std::endl;
    }
    return false;
  }
  if (!tktDesPresent)
  {
    if (diag)
    {
      *diag << " " << std::endl
            << "************************************************************" << std::endl
            << "VALIDATION RESULT: FAIL - ITIN INCLUDES NON TKT DES FARES   " << std::endl
            << "************************************************************" << std::endl;
    }
    return false;
  }
  if (diag)
  {
    *diag << " " << std::endl
          << "************************************************************" << std::endl
          << "VALIDATION RESULT: SOFT PASS - CONTINUE CONTENT VALIDATION  " << std::endl
          << "************************************************************" << std::endl;
  }
  return true;
}
bool
isEligible(PricingTrx& trx,
           const FarePath& farePath,
           const FarePreValidationVector& farePreValidationVector,
           Diag807Collector* diag,
           AutomaticPfcTaxExemptionData& exemptionData) // validation
{
  if (diag)
  {
    *diag << "CONTENT PARENT TABLE VALIDATION SCOPE " << std::endl;
  }
  bool isFirstFare(true);
  bool isPfcExemptConflict(false);
  bool isTaxExemptConflict(false);
  bool isChildTaxExemptConflict(false);

  Indicator pfcExempt = ' ';
  Indicator taxExempt = ' ';

  std::vector<TaxCode> taxCodeVec;
  std::vector<TaxCode> taxCodeVecA;

  for (FarePreValidationVector::const_iterator pit(farePreValidationVector.begin()),
       pitend(farePreValidationVector.end());
       pit != pitend;
       ++pit)
  {
    const PaxTypeFare* ptf(pit->_ptf);
    const PaxTypeCode& ptfPaxTypeCode(ptf->fcasPaxType());
    const TktDesignator& tktDesignator(pit->_tktDesignator);
    if (diag)
    {
      DiagCollector& dc = *diag;
      dc.setf(std::ios::left, std::ios::adjustfield);

      dc << " " << std::endl;
      dc << std::setw(3) << ptf->fareMarket()->origin()->loc() << '-';
      dc << std::setw(3) << ptf->fareMarket()->destination()->loc() << "  ";
      dc << std::setw(7) << ptf->carrier();
      dc << std::setw(10) << ptf->fareClass() << "";
      dc << "PTC: ";
      dc << std::setw(3) << ptf->paxType()->paxType() << "       TKT DES: ";
      dc << std::setw(10) << tktDesignator << std::endl;
    }
    typedef std::vector<TktDesignatorExemptInfo*> ExemptInfoVector;
    const ExemptInfoVector& exemptInfoVector(
        trx.dataHandle().getTktDesignatorExempt(ptf->carrier()));
    taxCodeVec.erase(taxCodeVec.begin(), taxCodeVec.end());

    bool foundMatch(false);
    for (ExemptInfoVector::const_iterator eit(exemptInfoVector.begin()),
         eitend(exemptInfoVector.end());
         eit != eitend && !foundMatch;
         ++eit)
    {
      foundMatch = ((*eit)->ticketDesignator().empty() ||
                    AutomaticPfcTaxExemption::matchTktDes((*eit)->ticketDesignator().c_str(),
                                                          tktDesignator.c_str())) &&
                   ((*eit)->paxType() == ptfPaxTypeCode || (*eit)->paxType().empty() ||
                    ptfPaxTypeCode.empty());
      if (foundMatch)
      {
        if (diag)
        {
          std::string disStr;
          *diag << "TBL SEQ NBR: " << (*eit)->sequenceNumber() << "   "
                << "  EFF: " << (*eit)->effDate().dateToString(YYYYMMDD, "-") << " ";
          if ((*eit)->discDate().isPosInfinity())
          {
            *diag << "    DIS: INFINITY" << std::endl;
          }
          else
            *diag << "    DIS: " << (*eit)->discDate().dateToString(YYYYMMDD, "-") << "  "
                  << std::endl;

          *diag << "PSGR TYPE CODE: " << (*eit)->paxType() << std::endl;
          *diag << "TICKET DES    : " << (*eit)->ticketDesignator() << std::endl;

          if ((*eit)->pfcExempt() == 'X')
          {
            *diag << "PFC EXEMPTION : EXEMPT ALL PFCS" << std::endl;
          }
          else
          {
            *diag << "PFC EXEMPTION : " << std::endl;
          }
          if ((*eit)->taxExempt() == 'U')
          {
            *diag << "TAX EXEMPTION : EXEMPT US1/US2/ZP " << std::endl;
          }
          else if ((*eit)->taxExempt() == ' ')
          {
            *diag << "TAX EXEMPTION : " << std::endl;
          }
          else if ((*eit)->taxExempt() == 'N')
          {
            std::string taxCodeString;
            for (TktDesignatorExemptTaxAInfo* taxi : (*eit)->tktDesignatorExemptTaxA())
            {
              taxCodeString = taxCodeString + taxi->taxCode();
              taxCodeString = taxCodeString + '/';
            }
            taxCodeString.erase(taxCodeString.end() - 1);
            *diag << "TAX EXEMPTION : EXEMPT ONLY " << taxCodeString << std::endl;
          }
        }
        if ((*eit)->taxExempt() == 'N')
        {
          for (TktDesignatorExemptTaxAInfo* taxi : (*eit)->tktDesignatorExemptTaxA())
          {
            taxCodeVec.push_back(taxi->taxCode());
          }
        }
        if (isFirstFare)
        {
          if ((*eit)->taxExempt() == 'N')
          {
            taxCodeVecA = taxCodeVec;
            std::sort(taxCodeVecA.begin(), taxCodeVecA.end());
          }
          pfcExempt = (*eit)->pfcExempt();
          taxExempt = (*eit)->taxExempt();
          isFirstFare = false;
        }
        else
        {
          if ((*eit)->taxExempt() == 'N')
          {
            std::sort(taxCodeVec.begin(), taxCodeVec.end());
          }
          if (!isPfcExemptConflict && pfcExempt != (*eit)->pfcExempt())
          {
            isPfcExemptConflict = true;
          }
          if (!isTaxExemptConflict && taxExempt != (*eit)->taxExempt())
          {
            isTaxExemptConflict = true;
          }

          else if (taxExempt == 'N')
          {
            if (!isChildTaxExemptConflict && taxCodeVecA != taxCodeVec)
            {
              isChildTaxExemptConflict = true;
            }
          }
          if (isPfcExemptConflict && (isTaxExemptConflict || isChildTaxExemptConflict))
          {
            break;
          }
        }
      }
    }

    if (!foundMatch)
    {
      if (diag)
      {
        *diag << " " << std::endl
              << "************************************************************" << std::endl
              << "VALIDATION RESULT: FAIL - NO PARENT TABLE MATCH FOUND"        << std::endl
              << "************************************************************" << std::endl;
      }
      return false;
    }
  }
  if (diag)
  {
    *diag << " " << std::endl
          << "************************************************************" << std::endl;
    *diag << "VALIDATION RESULT: " << std::endl;
    if (isPfcExemptConflict)
    {
      *diag << "PFC:  FAIL - MULTIPLE PARENT TABLE VALUES - BYPASS EXEMPTION" << std::endl;
    }
    else if (pfcExempt == ' ')
    {
      *diag << "PFC:  BYPASS PFC EXEMPTION PROCESS - NO EXEMPTIONS APPLY" << std::endl;
    }
    else if (pfcExempt == 'X')
    {
      *diag << "PFC:  PASS - CONTINUE TO FINAL PFC / TAX EXEMPTION RESULTS" << std::endl;
    }
    if (isTaxExemptConflict)
    {
      *diag << "TAX:  FAIL - MULTIPLE PARENT TABLE VALUES - BYPASS EXEMPTION" << std::endl;
    }
    else if (isChildTaxExemptConflict)
    {
      *diag << "TAX:  FAIL - MULTIPLE CHILD TABLE VALUES - BYPASS EXEMPTION" << std::endl;
    }
    else if (taxExempt == ' ')
    {
      *diag << "TAX:  BYPASS TAX EXEMPTION PROCESS - NO EXEMPTIONS APPLY" << std::endl;
    }
    else if (taxExempt == 'U' || taxExempt == 'N')
    {
      *diag << "TAX:  PASS - CONTINUE TO FINAL PFC / TAX EXEMPTION RESULTS" << std::endl;
    }

    *diag << "************************************************************" << std::endl;
  }

  if (isPfcExemptConflict && (isTaxExemptConflict || isChildTaxExemptConflict))
    return false;

  if (!isTaxExemptConflict)
  {
    AutomaticPfcTaxExemptionData::ExemptionTaxCodes& taxCodes = exemptionData.exemptionTaxCodes();

    if (taxExempt == 'U')
    {
      if (taxCodes.empty())
      {
        taxCodes.push_back(TaxCode("US1"));
        taxCodes.push_back(TaxCode("US2"));
        taxCodes.push_back(TaxCode("ZP"));
        exemptionData.taxExemptionOption() = AutomaticPfcTaxExemptionData::EXEMPT_INCLUDED;
      }
    }
    else if (!isChildTaxExemptConflict && taxExempt == 'N')
    {
      for (TaxCode& taxCode : taxCodeVecA)
      {
        taxCodes.push_back(taxCode);
      }
      exemptionData.taxExemptionOption() = AutomaticPfcTaxExemptionData::EXEMPT_INCLUDED;
    }
  }

  if (!isPfcExemptConflict && pfcExempt == 'X')
  {
    exemptionData.pfcExemptionOption() = AutomaticPfcTaxExemptionData::EXEMPT_ALL;
  }

  if (diag)
    *diag << "\nFINAL PFC/TAX EXEMPTION RESULTS \n" << std::endl;
  return true;
}
} // PFCTaxExemption

bool
AutomaticPfcTaxExemption::isAutomaticPfcExemtpionEnabled(PricingTrx& trx, TaxResponse& rsp)
{
  analyzePfcExemtpion(trx, rsp);
  return rsp.automaticPFCTaxExemptionData()->automaticPfcTaxExemptionEnabled();
}

void
AutomaticPfcTaxExemption::analyzePfcExemtpion(PricingTrx& trx, TaxResponse& rsp)
{
  if (!rsp.automaticPFCTaxExemptionData())
  {
    rsp.automaticPFCTaxExemptionData().reset(createPfcExemptionData(trx, rsp));
  }
}

AutomaticPfcTaxExemptionData
AutomaticPfcTaxExemption::createPfcExemptionData(PricingTrx& trx, TaxResponse& rsp)
{
  AutomaticPfcTaxExemptionData exemptionData;

  if (TrxUtil::isAutomaticPfcTaxExemptionEnabled(trx) &&
      AutomaticPfcTaxExemption::bypassTktDes(*rsp.farePath()))
  {
    Diag807Collector* diag(nullptr);
    DiagManager diagMgr(trx, Diagnostic807);
    if (diagMgr.isActive())
    {
      diag = &static_cast<Diag807Collector&>(diagMgr.collector());
    }

    if (diag)
    {
      diag->printHeader();
      DateTime tktDate = TrxUtil::getTicketingDT(trx);
      const TravelSeg* primarySeg = rsp.farePath()->itin()->travelSeg().front();

      *diag << "                  ** REQUESTED PTC: " << rsp.paxTypeCode() << " ** " << std::endl
            << "SALES DATE: " << tktDate.dateToString(YYYYMMDD, "-")
            << "       ITIN FIRST TVL DATE: " << primarySeg->pssDepartureDate() << std::endl
            << "************************************************************" << std::endl;
    }

    if (isFreeTicketByPass(trx, rsp))
    {
      if (diag)
      {
        *diag << "PFC/TAX EXEMPT NOT APPL WITH YQ/YR/Q SURCHARGES ON FREE" << std::endl
              << "TKTS" << std::endl;
      }
      exemptionData.automaticPfcTaxExemptionEnabled() = false;
      return exemptionData;
    }

    if (trx.getRequest())
    {
      const PricingRequest& req = *trx.getRequest();
      if (req.isExemptAllTaxes() || req.isExemptPFC() || req.isExemptSpecificTaxes() ||
          !req.taxOverride().empty())
      {
        if (diag)
          diag->printInfo(Diag807Collector::EXEMPTION_MODIFIERS_USED);
        exemptionData.automaticPfcTaxExemptionEnabled() = false;
        return exemptionData;
      }
    }

    PFCTaxExemption::FarePreValidationVector farePreValidationVector;
    if (PFCTaxExemption::isCandidate(trx, *rsp.farePath(), farePreValidationVector, diag) &&
        PFCTaxExemption::isEligible(
            trx, *rsp.farePath(), farePreValidationVector, diag, exemptionData))
    {
      exemptionData.automaticPfcTaxExemptionEnabled() = true;
      return exemptionData;
    }
  }
  exemptionData.automaticPfcTaxExemptionEnabled() = false;
  return exemptionData;
}

bool
AutomaticPfcTaxExemption::matchTktDes(const std::string& rule, const std::string& fare)
{
  std::size_t pos = rule.find("-");

  if (std::string::npos == pos)
  {
    if (rule == fare)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  bool hyphenBegin = (pos == 0);

  if (fare.length() == rule.length() - 1)
  {
    return false;
  }
  if (hyphenBegin)
  {
    if (fare.compare(
            (fare.size() - (rule.size() - 1)), rule.size() - 1, rule, 1, (rule.size() - 1)) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    if (fare.compare(0, (rule.size() - 1), rule, 0, (rule.size() - 1)) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  return false;
}

bool
AutomaticPfcTaxExemption::bypassTktDes(const FarePath& farePath)
{
  if (isAAItinOnly(farePath))
    return true;

  FareUsage* fareUsage = *(*farePath.pricingUnit().begin())->fareUsage().begin();
  if (fareUsage->paxTypeFare()->actualPaxType()->paxType().equalToConst("FFY") ||
      fareUsage->paxTypeFare()->actualPaxType()->paxType().equalToConst("FFP"))
    return true;
  return false;
}
bool
AutomaticPfcTaxExemption::isAAItinOnly(const FarePath& farePath)
{
  for (const TravelSeg* ts : farePath.itin()->travelSeg())
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(ts);
    if (airSeg && airSeg->carrier() != "AA")
    {
      return false;
    }
  }
  return true;
}

bool
AutomaticPfcTaxExemption::isFreeTicketByPass(PricingTrx& trx, const TaxResponse& rsp)
{
  if (taxUtil::doUsTaxesApplyOnYQYR(trx, *rsp.farePath()))
  {
    if (rsp.farePath()->getTotalNUCAmount() > EPSILON)
      return true;

    for (TaxItem* taxItem : rsp.taxItemVector())
    {
      TaxCode& taxCode = taxItem->taxCode();
      if ((taxCode.equalToConst("YQF") || taxCode.equalToConst("YQI") || taxCode.equalToConst("YRF") || taxCode.equalToConst("YRI")) &&
          taxItem->taxAmount() > EPSILON)
        return true;
    }
  }
  return false;
}
}
