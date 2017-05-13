//-------------------------------------------------------------------
//  Copyright Sabre 2015
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#include "Rules/AgencyCommissions.h"

#include "Rules/CommissionRuleCollector.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "Diagnostic/Diag867Collector.h"

namespace tse
{
using namespace amc;

FALLBACK_DECL(fallbackAMCPhase2);
FALLBACK_DECL(fallbackFixDisplayValidatingCarrierDiag867);
FIXEDFALLBACK_DECL(fallbackAMC2SrcTgtPCCChange);
FALLBACK_DECL(fallbackAMC2Diag867NewLogic);

namespace
{

FcCommissionData*
findFcCommissionData(
    const FcCommissionInfoPairVec& fcAgcyCommDataCol,
    const FareUsage& fu)
{
  FcCommissionInfoPairVec::const_iterator it;
  for (it = fcAgcyCommDataCol.begin(); it < fcAgcyCommDataCol.end(); ++it)
    if (it->first == &fu)
      return it->second;

  return nullptr;
}

void
printValidatingFcCommission(
    Diag867Collector& diag867,
    const FarePath& fpath,
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    const FcCommissionInfoPairVec& fcAgcyCommDataCol)
{
  if (diag867) // Not for DDAMC/DDINFO
  {
    if(!diag867.isDetailedDiagnostic())
    {
      diag867.printValidatingCarrierFCHeader(vcfmptfCommRules.begin()->first.valCxr);
    }
    for (auto pu : fpath.pricingUnit())
    {
      if (!pu) continue;
      for (auto fu : pu->fareUsage())
      {
        if (!fu) continue;

        // if Commission exists for this FC
        auto it = findFcCommissionData(fcAgcyCommDataCol, *fu);
        if (it)
          diag867.printSingleFcCommission(fpath,
                                          fu,
                                          it->fcCommAmt(),
                                          it->commRuleData());
      }
    }
  }
}

} //namespace

AgencyCommissions::AgencyCommissions(PricingTrx& trx, Diag867Collector* diag867)
  : _trx(trx),
    _diag867(diag867)
{
}

AgencyCommissions::~AgencyCommissions()
{
}

bool
AgencyCommissions::getAgencyCommissions(
    FarePath& fpath,
    const std::vector<CustomerSecurityHandshakeInfo*>& csHsInfoCol,
    VCFMCommissionPrograms& vcfmCommProgs,
    VCFMPTFCommissionRules& vcfmptfCommRules)
{
  if(_diag867 && !_diag867->isDiagPaxTypeMatch(fpath))
    return true;

  if (_diag867)
  {
    _diag867->printFarePathHeader(fpath);
  }

  PseudoCityCode tvlAgencyPCC;
  if (_trx.getRequest() && _trx.getRequest()->ticketingAgent())
    tvlAgencyPCC = _trx.getRequest()->ticketingAgent()->tvlAgencyPCC();

  if (tvlAgencyPCC.empty())
    return false;

  CommissionRuleCollector crCollector(
      _trx,
      _diag867,
      fpath,
      vcfmCommProgs,
      vcfmptfCommRules);

  const CarrierCode defValCxr = _trx.isValidatingCxrGsaApplicable() ?
    fpath.defaultValidatingCarrier() :
    (fpath.itin() ? fpath.itin()->validatingCarrier() : "");

  ContractIdCol anyCxrContIdCol;
  collectContractIdForAgency(ANY_CARRIER, tvlAgencyPCC, csHsInfoCol, anyCxrContIdCol);

  // Collect CommissionRule data for def val-cxr
  if (!defValCxr.empty() && (!_diag867 || _diag867->isDiagValCxrMatch(defValCxr)))
    collectCommissionRuleData(defValCxr, tvlAgencyPCC, csHsInfoCol, anyCxrContIdCol, crCollector);

  // Collect CommissionRule data for alt val-cxr
  for (const CarrierCode& valCxr : fpath.validatingCarriers())
  {
    if (!_diag867 || _diag867->isDiagValCxrMatch(valCxr))
    {
      collectCommissionRuleData(valCxr, tvlAgencyPCC, csHsInfoCol, anyCxrContIdCol, crCollector);
    }
  }

  if (!crCollector.commissionRulesPerKey().empty() &&
      calculateCommission(fpath, crCollector.commissionRulesPerKey()))
    return true;

  if(_diag867)
    _diag867->printCommissionNotProcessed();

  return false;
}

/// Calculate commission based on Commission Rule Table
bool
AgencyCommissions::calculateCommission(
    FarePath& fpath,
    const VCFMPTFCommRulesPerValCxr& commRulesPerKey)
{
  for (const auto& p : commRulesPerKey)
  {
    if (p.second.empty()) // we found no rules for valCxr
      continue;

    FcCommissionInfoPairVec fcAgcyCommDataCol;
    const VCFMPTFCommissionRules& vcfmptfCommRules = p.second;
    MoneyAmount maxAmt = 0;
    bool res = (vcfmptfCommRules.size() ==1) ?
      calculateMaxCommForSingleFc(fpath, vcfmptfCommRules, fcAgcyCommDataCol, maxAmt):
      calculateMaxCommForMultipleFc(fpath, vcfmptfCommRules, fcAgcyCommDataCol, maxAmt);

    if (res)
    {
      if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
      {
        if(_diag867)
          _diag867->printFinalAgencyCommission(maxAmt);
      }

      fpath.storeCommissionForValidatingCarrier(p.first, maxAmt);
      if (!fallback::fallbackAMCPhase2(&_trx))
      {
        storeFcCommissionDataInFC(p.first, fcAgcyCommDataCol);
        if(!fallback::fallbackAMC2Diag867NewLogic(&_trx))
        {
          if(_diag867)
          {
            printValidatingFcCommission(*_diag867,
                                         fpath,
                                         vcfmptfCommRules,
                                         fcAgcyCommDataCol);
            _diag867->printFinalAgencyCommission(maxAmt);
          }
        }
      }
    }
  }
  return !fpath.valCxrCommissionAmount().empty();
}

/// Return maximum commission for single FC across all contracts
bool
AgencyCommissions::calculateMaxCommForSingleFc(
    FarePath& fpath,
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    FcCommissionInfoPairVec& fcAgcyCommDataCol,
    MoneyAmount& maxAmt) const
{
  auto it = vcfmptfCommRules.begin();
  if (it == vcfmptfCommRules.end())
    return false;

  FareUsage* fu = findFareUsage(fpath, it->first);
  if (!fu)
    return false;

  const ContractFcCommRuleDataMap& contRuleData = it->second;
  if (contRuleData.empty())
    return false;

  if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
  {
    if(!fallback::fallbackFixDisplayValidatingCarrierDiag867(&_trx))
    {
      if(_diag867)
        _diag867->printValidatingCarrierFCHeader(vcfmptfCommRules.begin()->first.valCxr);
    }
  }

  CommissionRuleData selectedCrd;
  for (const auto& p : contRuleData) // iterates through contractIds
  {
    const FcCommissionRuleData* fcCommRuleData = p.second;
    if (fcCommRuleData)
    {
      MoneyAmount tmpAmt = 0;
      for (const auto& q : fcCommRuleData->commRuleDataColPerCommType())
      {
        CommissionRuleData crd;
        tmpAmt = getMaxCommAmount(fpath, *fu, q.second, crd);
        if (!fallback::fallbackAMCPhase2(&_trx))
        {
          // Zero comm amount is possible unless there is no valid comm-rule.
          if (tmpAmt > maxAmt ||
              (crd.commRuleInfo() && tmpAmt == 0 && maxAmt <= 0))
          {
            maxAmt = tmpAmt;
            selectedCrd = crd;
          }
        }
        else
        {
          if (tmpAmt > maxAmt)
            maxAmt = tmpAmt;
        }
      }
    }
  }

  if (!fallback::fallbackAMCPhase2(&_trx))
    storeFcCommissionData(fcAgcyCommDataCol, *fu, selectedCrd, maxAmt);

  if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
  {
    if(_diag867)
      _diag867->printSingleFcCommission(fpath, fu, maxAmt, selectedCrd);
  }

  return true;
}

/// Return maximum commission for multiple FC
bool
AgencyCommissions::calculateMaxCommForMultipleFc(
    FarePath& fpath,
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    FcCommissionInfoPairVec& fcAgcyCommDataCol,
    MoneyAmount& maxAmt) const
{
  bool res = false;
  res |= calculateCommissionForCT9(fpath, vcfmptfCommRules, fcAgcyCommDataCol, maxAmt);
  res |= calculateCommissionForCT10(fpath, vcfmptfCommRules, fcAgcyCommDataCol, maxAmt);
  res |= calculateCommissionForCT11(fpath, vcfmptfCommRules, fcAgcyCommDataCol, maxAmt);
  return res;
}

bool
AgencyCommissions::calculateCommissionForCT9(
    FarePath& fpath,
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    FcCommissionInfoPairVec& fcAgcyCommDataCol,
    MoneyAmount& maxAmt) const
{
  ContractId contId = -1;
  if (!checkCommType(vcfmptfCommRules, contId, CT9))
    return false; // failed to find a contract

  if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
  {
    if(!fallback::fallbackFixDisplayValidatingCarrierDiag867(&_trx))
    {
      if(_diag867)
        _diag867->printValidatingCarrierFCHeader(vcfmptfCommRules.begin()->first.valCxr);
    }
  }

  if (!fallback::fallbackAMCPhase2(&_trx))
  {
    FcCommissionRulePairVec ct9FcCrdCol;
    if (!findMaxCommRulesForCT9(fpath, vcfmptfCommRules, contId, ct9FcCrdCol) ||
        ct9FcCrdCol.empty())
      return false;

    // Find lowest CommissionRuleData to apply across FCs
    FcCommissionInfoPairVec tmpFcAgcyCommDataCol;
    auto it = std::min_element(ct9FcCrdCol.begin(), ct9FcCrdCol.end(),
        [] (const auto& p1, const auto& p2) {
        const CommissionRuleInfo* cri1 = p1.second.commRuleInfo();
        const CommissionRuleInfo* cri2 = p2.second.commRuleInfo();
        return cri1 && cri2 && cri1->commissionValue() < cri2->commissionValue();
        });

    const CommissionRuleData& lowestCrd = it->second;
    MoneyAmount tmpAmt =
      collectCT9FromSelectedCommRuleData(fpath, ct9FcCrdCol, lowestCrd,
                                         tmpFcAgcyCommDataCol);
    if (tmpAmt > maxAmt ||
        (!tmpFcAgcyCommDataCol.empty() && tmpAmt == 0 && maxAmt <= 0))
    {
      maxAmt = tmpAmt;
      fcAgcyCommDataCol.swap(tmpFcAgcyCommDataCol);
    }
  }
  else
  {
    FcCommRuleDataCol ct9FcCrdCol;
    if (!findMaxCommRulesForCT9(fpath, vcfmptfCommRules, ct9FcCrdCol, contId))
      return false;

    const SelectedFcCommRuleData& lowestCrd = *(--ct9FcCrdCol.end()); // Pick the lowest comm-rule
    if (!lowestCrd.commissionRuleData().commRuleInfo())
      return false; // should not happen

    MoneyAmount percentage = lowestCrd.commissionRuleData().commRuleInfo()->commissionValue();
    MoneyAmount tmpAmt = 0;
    for (const auto& p : vcfmptfCommRules)
    {
      const FareUsage* fu = findFareUsage(fpath, p.first);
      if (!fu)
        return false;

      auto it = p.second.find(contId);
      if (it == p.second.end() || !it->second) // restricting to contract selected
        return false; // should not happen

      const FcCommissionRuleData* fcCrd = it->second;
      bool applyQs = checkApplyQSurcharge(ct9FcCrdCol, fcCrd);

      MoneyAmount amt = (getNetTotalAmount(fpath, *fu, applyQs) * percentage)/100;

      if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
      {
        if(_diag867)
          _diag867->printSingleFcCommission(fpath, fu, amt, lowestCrd.commissionRuleData());
      }
      tmpAmt += amt;
    }

    if (tmpAmt > maxAmt)
      maxAmt = tmpAmt;
  }

  return true;
}

MoneyAmount
AgencyCommissions::collectCT9FromSelectedCommRuleData(
  FarePath& fpath,
  FcCommissionRulePairVec& ct9FcCrdCol,
  const CommissionRuleData& selectedCrd,
  FcCommissionInfoPairVec& tmpFcAgcyCommDataCol) const
{
  MoneyAmount totalCommAmt = 0;
  if (!selectedCrd.commRuleInfo())
    return totalCommAmt;

  const MoneyAmount percentage = selectedCrd.commRuleInfo()->commissionValue();
  for (auto& p : ct9FcCrdCol)
  {
    if (p.first) // fareUsage
    {
      FareUsage* fu = p.first;
      const CommissionRuleData& crd = p.second;
      MoneyAmount amt = (getNetTotalAmount(
            fpath,
            *fu,
            crd.isQSurchargeApplicable()) * percentage
          ) / 100;

      totalCommAmt += amt;
      storeFcCommissionData(tmpFcAgcyCommDataCol, *fu, crd, amt);

      if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
      {
        if(_diag867)
          _diag867->printSingleFcCommission(fpath, fu, amt, selectedCrd);
      }
    }
  }

  return totalCommAmt;
}

/// Delete it with fallbackAMC2
/// Apply Q surcharge based on matching rule and not based on lowest comm-rule
bool
AgencyCommissions::checkApplyQSurcharge(
    const FcCommRuleDataCol& selectedFcCrdCol,
    const FcCommissionRuleData* fcCommRuleData) const
{
  // match process FcCommissionRuleData with selected and see whether it has apply Q Surcharge
  return std::find_if(selectedFcCrdCol.begin(), selectedFcCrdCol.end(),
      [fcCommRuleData] (const SelectedFcCommRuleData& obj) -> bool {
      return (obj.fcCommRuleData() == fcCommRuleData &&
              obj.commissionRuleData().isQSurchargeApplicable());
      }) != selectedFcCrdCol.end();
}

bool
AgencyCommissions::calculateCommissionForCT10(
    FarePath& fpath,
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    FcCommissionInfoPairVec& fcAgcyCommDataCol,
    MoneyAmount& maxAmt) const
{
  ContractId contId = -1;
  if (!checkCommType(vcfmptfCommRules, contId, CT10))
    return false; // failed to find contract

  if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
  {
    if(!fallback::fallbackFixDisplayValidatingCarrierDiag867(&_trx))
    {
      if(_diag867)
        _diag867->printValidatingCarrierFCHeader(vcfmptfCommRules.begin()->first.valCxr);
    }
  }

  MoneyAmount tmpAmt = 0;
  FcCommissionInfoPairVec tmpFcAgcyCommDataCol;
  FareUsage* fu = nullptr;
  for (const auto& p : vcfmptfCommRules)
  {
    auto it = p.second.find(contId);
    if (it == p.second.end() || !it->second) // restricting to contract selected
    {
      if(_diag867 && (fu=findFareUsage(fpath, p.first)))
        _diag867->printCommissionNotFound(*fu);
      return false;
    }

    const FcCommissionRuleData& fcCrd = *it->second;
    auto itCt10 = fcCrd.commRuleDataColPerCommType().find(CT10);
    if (itCt10 == fcCrd.commRuleDataColPerCommType().end() || itCt10->second.empty())
    {
      if (_diag867 && (fu=findFareUsage(fpath, p.first)))
        _diag867->printCommissionNotFound(*fu);
      return false;
    }

    fu = findFareUsage(fpath, p.first);
    if (!fu) // should not happen
      return false;

    const CommissionRuleData& crd = *itCt10->second.begin();
    if (crd.commRuleInfo())
    {
      MoneyAmount percentage = crd.commRuleInfo()->commissionValue();
      MoneyAmount amt =
        (getNetTotalAmount(fpath, *fu, crd.isQSurchargeApplicable()) * percentage)/100;

      if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
      {
        if(_diag867)
          _diag867->printSingleFcCommission(fpath, fu, amt, crd);
      }

      tmpAmt += amt;
      if (!fallback::fallbackAMCPhase2(&_trx))
        storeFcCommissionData(tmpFcAgcyCommDataCol, *fu, crd, amt);
    }
  }

  if (!fallback::fallbackAMCPhase2(&_trx))
  {
    if (tmpAmt > maxAmt ||
        (!tmpFcAgcyCommDataCol.empty() && tmpAmt == 0 && maxAmt <= 0))
    {
      maxAmt = tmpAmt;
      fcAgcyCommDataCol.swap(tmpFcAgcyCommDataCol);
    }
  }
  else
  {
    if (tmpAmt > maxAmt)
      maxAmt = tmpAmt;
  }

  return true;
}

bool
AgencyCommissions::calculateCommissionForCT11(
    FarePath& fpath,
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    FcCommissionInfoPairVec& fcAgcyCommDataCol,
    MoneyAmount& maxAmt) const
{
  uint16_t nonCommFcCnt = 0;
  ContractId contId = -1;
  if (!checkCommType11(vcfmptfCommRules, nonCommFcCnt, contId) ||
      nonCommFcCnt == vcfmptfCommRules.size())
    return false;

  if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
  {
    if(!fallback::fallbackFixDisplayValidatingCarrierDiag867(&_trx))
    {
      if(_diag867)
        _diag867->printValidatingCarrierFCHeader(vcfmptfCommRules.begin()->first.valCxr);
    }
  }

  MoneyAmount totalCommAmt = 0;
  FcCommissionInfoPairVec tmpFcAgcyCommDataCol;
  FareUsage* fu = nullptr;
  for (const auto& p : vcfmptfCommRules)
  {
    if (!fallback::fallbackAMCPhase2(&_trx))
    {
      if (p.second.empty()) // non-comm FC
        continue;

      MoneyAmount commAmt = 0.0;
      auto it = p.second.find(contId);
      if (it == p.second.end() || !it->second) // CT11 combines with non-comm FC
      {
        if(_diag867 && (fu=findFareUsage(fpath, p.first)))
          _diag867->printCommissionNotFound(*fu);
        continue; // next FC
      }

      fu = findFareUsage(fpath, p.first);
      if (!fu) // should not happen
        continue;

      const FcCommissionRuleData& fcCrd = *it->second;
      CommissionRuleData selectedCrd;
      if (collectCommForCT11(fpath, *fu, fcCrd, nonCommFcCnt, selectedCrd, commAmt))
        totalCommAmt += commAmt;
      storeFcCommissionData(tmpFcAgcyCommDataCol, *fu, selectedCrd, commAmt);
    }
    else
    {
      fu = findFareUsage(fpath, p.first);
      if (!fu || p.second.empty()) // non-comm FC
        continue;

      MoneyAmount commAmt = 0.0;
      auto it = p.second.find(contId);
      const FcCommissionRuleData& fcCrd = *it->second;
      if (it == p.second.end() || !it->second) // restricting to contract selected
      {
        if(_diag867)
          _diag867->printCommissionNotFound(*fu);
        return false; // should not happen
      }

      // CT10s or CT11s or both in FC
      auto it11 = fcCrd.commRuleDataColPerCommType().find(CT11);
      if (it11 == fcCrd.commRuleDataColPerCommType().end())
      {
        auto it10 = fcCrd.commRuleDataColPerCommType().find(CT10);
        if (it10 == fcCrd.commRuleDataColPerCommType().end() || nonCommFcCnt > 0)
        {
          if(_diag867)
            _diag867->printCommissionNotFound(*fu);

          continue;
        }
        else
        {
          const CommissionRuleData& crd = *it10->second.begin();
          if (crd.commRuleInfo())
          {
            MoneyAmount percentage = crd.commRuleInfo()->commissionValue();
            commAmt = (getNetTotalAmount(fpath, *fu, crd.isQSurchargeApplicable()) * percentage)/100;

            if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
            {
              if(_diag867)
                _diag867->printSingleFcCommission(fpath, fu, commAmt, crd);
            }
          }
        }
      }
      else
      {
        const CommissionRuleData& crd = *it11->second.begin();
        if (crd.commRuleInfo())
        {
          MoneyAmount percentage = crd.commRuleInfo()->commissionValue();
          commAmt = (getNetTotalAmount(fpath, *fu, crd.isQSurchargeApplicable()) * percentage)/100;
        }

        // Checking CT10 in FC for max comm
        if (nonCommFcCnt == 0)
        {
          auto it10 = fcCrd.commRuleDataColPerCommType().find(CT10);
          if (it10 != fcCrd.commRuleDataColPerCommType().end() && nonCommFcCnt == 0)
          {
            const CommissionRuleData& crdForCt10 = *it10->second.begin();
            if (crdForCt10.commRuleInfo())
            {
              MoneyAmount tmpAmt = (getNetTotalAmount(fpath, *fu, crdForCt10.isQSurchargeApplicable()) * crdForCt10.commRuleInfo()->commissionValue())/100;
              if (tmpAmt > commAmt)
                commAmt = tmpAmt;
            }
          }
        }

        if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
        {
          if(_diag867)
            _diag867->printSingleFcCommission(fpath, fu, commAmt, crd);
        }
      }

      totalCommAmt += commAmt;
    }
  }

  if (!fallback::fallbackAMCPhase2(&_trx))
  {
    if (totalCommAmt > maxAmt ||
        (!tmpFcAgcyCommDataCol.empty() && totalCommAmt == 0 && maxAmt <= 0))
    {
      maxAmt = totalCommAmt;
      fcAgcyCommDataCol.swap(tmpFcAgcyCommDataCol);
    }
  }
  else
  {
    maxAmt = totalCommAmt;
  }

  return true;
}

bool
AgencyCommissions::collectCommForCT11(
    const FarePath& fpath,
    const FareUsage& fu,
    const FcCommissionRuleData& fcCrd,
    const uint16_t nonCommFcCnt,
    CommissionRuleData& selectedCrd,
    MoneyAmount& commAmt) const
{
  auto it11 = fcCrd.commRuleDataColPerCommType().find(CT11);
  bool commFound = (it11 != fcCrd.commRuleDataColPerCommType().end());
  if (!commFound)
  {
    if (nonCommFcCnt == 0) // CT10 does not combine if there is non-comm FC present
      commFound = collectCommForCT10(fpath, fu, fcCrd, selectedCrd, commAmt);
  }
  else
  {
    const CommissionRuleData& crd = *it11->second.begin();
    if (crd.commRuleInfo())
    {
      MoneyAmount percentage = crd.commRuleInfo()->commissionValue();
      commAmt = (getNetTotalAmount(fpath, fu, crd.isQSurchargeApplicable()) * percentage)/100;
      selectedCrd = crd;
    }

    // Checking CT10 in FC for max comm
    if (nonCommFcCnt == 0)
    {
      MoneyAmount tmpAmt = 0;
      if (collectCommForCT10(fpath, fu, fcCrd, selectedCrd, tmpAmt) && tmpAmt > commAmt)
        commAmt = tmpAmt;
    }
  }

  if(_diag867)
  {
    if(fallback::fallbackAMC2Diag867NewLogic(&_trx))
    {
      if (commFound)
        _diag867->printSingleFcCommission(fpath, &fu, commAmt, selectedCrd);
      else
        _diag867->printCommissionNotFound(fu);
    }
    else
    {
      if (!commFound)
        _diag867->printCommissionNotFound(fu);
    }
  }
  return commFound;
}

bool
AgencyCommissions::collectCommForCT10(
    const FarePath& fpath,
    const FareUsage& fu,
    const FcCommissionRuleData& fcCrd,
    CommissionRuleData& selectedCrd,
    MoneyAmount& commAmt) const
{
  auto it10 = fcCrd.commRuleDataColPerCommType().find(CT10);
  if (it10 == fcCrd.commRuleDataColPerCommType().end())
    return false;

  const CommissionRuleData& crd = *it10->second.begin();
  if (!crd.commRuleInfo())
    return false;

  MoneyAmount percentage = crd.commRuleInfo()->commissionValue();
  commAmt = (getNetTotalAmount(fpath, fu, crd.isQSurchargeApplicable()) * percentage)/100;
  selectedCrd = crd;
  return true;
}

// Delete it with fallbackAMC2
bool
AgencyCommissions::findMaxCommRulesForCT9(
    const FarePath& fpath,
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    FcCommRuleDataCol& fcCrdCol,
    ContractId contId) const
{
  for (const auto& p : vcfmptfCommRules)
  {
    const ContractFcCommRuleDataMap& contRulesCol = p.second;

    // Process only single contract that is been selected
    auto contIt = contRulesCol.find(contId);
    if (contIt != contRulesCol.end() && contIt->second)
    {
      const FcCommissionRuleData* fcCommRuleData = contIt->second;
      if (fcCommRuleData)
      {
        auto it = fcCommRuleData->commRuleDataColPerCommType().find(CT9);
        if (it != fcCommRuleData->commRuleDataColPerCommType().end() &&
            !it->second.empty())
          fcCrdCol.insert(SelectedFcCommRuleData(fcCommRuleData, *it->second.begin()));
      }
    }
  }
  return !fcCrdCol.empty();
}

bool
AgencyCommissions::findMaxCommRulesForCT9(
    const FarePath& fpath,
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    const ContractId contId,
    FcCommissionRulePairVec& fcRuleCol) const
{
  for (const auto& p : vcfmptfCommRules)
  {
    const ContractFcCommRuleDataMap& contRulesCol = p.second;
    auto contIt = contRulesCol.find(contId); // Restricting to selected contract-id
    if (contIt != contRulesCol.end() && contIt->second)
    {
      const FcCommissionRuleData& fcCommRuleData = *contIt->second;
      auto it = fcCommRuleData.commRuleDataColPerCommType().find(CT9);
      if (it != fcCommRuleData.commRuleDataColPerCommType().end() &&
          !it->second.empty())
      {
        fcRuleCol.push_back(
            std::make_pair(findFareUsage(fpath, p.first), *it->second.begin()));
      }
    }
  }
  return !fcRuleCol.empty();
}

bool
AgencyCommissions::checkCommType11(
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    uint16_t& nonCommFcCnt,
    ContractId& contId) const
{
  if (vcfmptfCommRules.empty())
    return false;

  // crKey => {contId => FcCommData*}
  auto it = vcfmptfCommRules.begin();
  auto itEnd = vcfmptfCommRules.end();
  for (; it != itEnd; ++it)
  {
    if (!it->second.empty())
      break;
    else if (!fallback::fallbackAMCPhase2(&_trx))
      ++nonCommFcCnt;
  }

  if (it == itEnd)
    return false;

  for (const auto& p : it->second) // iterating over first FC's contracts
  {
    if (checkForContractAndCommType11(vcfmptfCommRules, p.first, nonCommFcCnt))
    {
      contId = p.first;
      return true;
    }
  }
  return false;
}

/* Use first FC to find a common contract accross FCs
 * and and check for existence of CT for that contrat */
bool
AgencyCommissions::checkCommType(
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    ContractId& contId,
    CommissionTypeId ct) const
{
  auto it = vcfmptfCommRules.begin();
  auto itEnd = vcfmptfCommRules.end();
  if (it != itEnd && !it->second.empty())
  {
    for (const auto& p : it->second) // iterating over first FC's contracts
    {
      if (checkForContractAndCommType(vcfmptfCommRules, p.first, ct))
      {
        contId = p.first;
        return true;
      }
    }
  }
  return false;
}

/* *
 * Check whether a contact from first FC exists in all FCs. It also check
 * whether we can use it for CT
 * */
bool
AgencyCommissions::checkForContractAndCommType(
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    ContractId contId,
    CommissionTypeId ct) const
{
  for (const auto& p : vcfmptfCommRules) // iterating over each FC
  {
    if (p.second.empty()) // non commissional FC
      return false; // CT9 or CT10 does not combines

    const ContractFcCommRuleDataMap& contRulesCol = p.second;
    auto it = std::find_if(contRulesCol.begin(), contRulesCol.end(),
        [contId] (const ContractFcCommRuleDataPair& obj) -> bool {
        return obj.first == contId; });

    if (it != contRulesCol.end())
    {
      const FcCommissionRuleData* fcCrd = it->second;
      if (fcCrd && fcCrd->doesFcHaveCommissionType(ct))
        continue; // check next FC
      else
        return false;
    }
  }
  return true;
}

bool
AgencyCommissions::checkForContractAndCommType11(
    const VCFMPTFCommissionRules& vcfmptfCommRules,
    const ContractId& contId,
    uint16_t& nonCommFcCnt) const
{
  uint16_t ct11Cnt = 0;
  for (const auto& p : vcfmptfCommRules) // iterating over each FC
  {
    if (p.second.empty()) // non commissional FC
      continue;

    const ContractFcCommRuleDataMap& contRulesCol = p.second;
    auto it = std::find_if(contRulesCol.begin(), contRulesCol.end(),
        [contId] (const ContractFcCommRuleDataPair& obj) -> bool {
        return obj.first == contId; });

    if (it != contRulesCol.end()) // Found contract
    {
      bool ct10Found = false;
      bool ct11Found = false;
      if (!it->second) // Is non-commissionable FC ?
        ++nonCommFcCnt;
      else
      {
        const FcCommissionRuleData* fcCrd = it->second;
        ct10Found = fcCrd->doesFcHaveCommissionType(CT10);
        ct11Found = fcCrd->doesFcHaveCommissionType(CT11);
        if (ct11Found)
          ++ct11Cnt;
      }

      if (!it->second || ct10Found || ct11Found)
        continue;
      else // C11 does not combine with CT9
        return false;
    }
  }

  // At least we should find one CT11 across FCs
  return ct11Cnt > 0 ? true : false;
}

void
AgencyCommissions::collectCommissionRuleData(
    const CarrierCode& valCxr,
    const PseudoCityCode& pcc,
    const std::vector<CustomerSecurityHandshakeInfo*>& csHsInfoCol,
    const ContractIdCol& anyCxrContractIdCol,
    CommissionRuleCollector& crCollector) const
{
  ContractIdCol contIdCol(anyCxrContractIdCol);
  collectContractIdForAgency(valCxr, pcc, csHsInfoCol, contIdCol);
  if (!contIdCol.empty())
  {
    if(!fallback::fallbackAMC2Diag867NewLogic(&_trx))
    {
      if (_diag867 && _diag867->isDetailedDiagnostic())
      {
        _diag867->printValidatingCarrierFCHeader(valCxr);
      }
    }
    crCollector.collectCommissionRules(valCxr, contIdCol);
  }
}

void
AgencyCommissions::collectContractIdForAgency(
    const CarrierCode& valCxr,
    const PseudoCityCode& pcc,
    const std::vector<CustomerSecurityHandshakeInfo*>& col,
    ContractIdCol& contractIdCol) const
{
  for (const CommissionContractInfo* ccInfo : getCommissionContractInfo(valCxr, pcc))
  {
    if (ccInfo)
    {
      // We need fixed fallback here due to delay in data on Daily.
      if (!fallback::fallbackAMCPhase2(&_trx) &&
          !fallback::fixed::fallbackAMC2SrcTgtPCCChange())
      {
        // TargetPCC of CustomerSecurityHandshake creates the commissions. They should match
        // to sourcePCC of CommissionContract
        if (std::find_if(col.begin(), col.end(),
              [ccInfo] (const CustomerSecurityHandshakeInfo* obj) -> bool
              {return obj && obj->securityTargetPCC() == ccInfo->sourcePCC(); }) != col.end())
        contractIdCol.insert(ccInfo);
      }
      else
        contractIdCol.insert(ccInfo);
    }
  }

  if (_diag867 && contractIdCol.empty() && _diag867->isDetailedDiagnostic())
    _diag867->printContractNotFound(valCxr, pcc);
}

FareUsage*
AgencyCommissions::findFareUsage(const FarePath& fp, const CommissionRuleKey& crKey) const
{
  for (const PricingUnit* pu : fp.pricingUnit())
  {
    if (pu)
    {
      for (FareUsage* fu : pu->fareUsage())
      {
        if (fu && fu->paxTypeFare() == crKey.ptFare)
          return fu;
      }
    }
  }
  return nullptr;
}

MoneyAmount
AgencyCommissions::getMaxCommAmount(
    const FarePath& fp,
    const FareUsage& fu,
    const SortedCommRuleDataVec& crdCol,
    CommissionRuleData& selectedCrd) const
{
  return isCommRuleDataColHasSurcharge(crdCol) ?
    getMaxCommAmountWithSurcharge(fp, fu, crdCol, selectedCrd):
    getMaxCommAmountWithoutSurcharge(fp, fu, crdCol, selectedCrd);
}

boost::logic::tribool
AgencyCommissions::isCommRuleDataColHasSurcharge(const SortedCommRuleDataVec& crdCol) const
{
  using boost::logic::indeterminate;
  if (crdCol.empty())
    return indeterminate;

  return std::find_if(crdCol.begin(), crdCol.end(),
      [] (const CommissionRuleData& obj)->bool {
      return obj.isQSurchargeApplicable() == true; }) != crdCol.end();
}

/// Look for maximum across all comm-rules
MoneyAmount
AgencyCommissions::getMaxCommAmountWithSurcharge(
    const FarePath& fp,
    const FareUsage& fu,
    const SortedCommRuleDataVec& crdCol,
    CommissionRuleData& selectedCrd) const
{
  MoneyAmount maxAmt = 0;
  for (const CommissionRuleData& crd : crdCol)
  {
    if (crd.commRuleInfo())
    {
      MoneyAmount tmp = (getNetTotalAmount(fp, fu, crd.isQSurchargeApplicable()) * crd.commRuleInfo()->commissionValue())/100;
      if (tmp - maxAmt > EPSILON)
      {
        maxAmt = tmp;
        if(_diag867)
          selectedCrd = crd;
      }
    }
  }
  return maxAmt;
}

/// Take the first front since vector is sorted in desceding order of commission percentage
MoneyAmount
AgencyCommissions::getMaxCommAmountWithoutSurcharge(
    const FarePath& fp,
    const FareUsage& fu,
    const SortedCommRuleDataVec& crdCol,
    CommissionRuleData& selectedCrd) const
{
  if (!crdCol.empty() && crdCol.begin()->commRuleInfo())
  {
    const CommissionRuleData& crd = *crdCol.begin();
    if(_diag867)
      selectedCrd = crd;

    const CommissionRuleInfo& cri = *(crd.commRuleInfo());
    return (getNetTotalAmount(fp, fu, crd.isQSurchargeApplicable()) * cri.commissionValue())/100;
  }
  return 0;
}

MoneyAmount
AgencyCommissions::getNetTotalAmount(
    const FarePath& fp,
    const FareUsage& fu,
    bool applyQSurcharge) const
{
  MoneyAmount netTotalInPaymCurr = applyQSurcharge ? fu.totalFareAmount() :
    fu.totalFareAmount() - fu.surchargeAmt();
  CurrencyCode calcCurrency = fp.calculationCurrency();
  CurrencyCode baseFareCurrency = fp.baseFareCurrency();
  CurrencyCode paymentCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!_trx.getOptions()->currencyOverride().empty())
    paymentCurrency = _trx.getOptions()->currencyOverride();

  if (calcCurrency != baseFareCurrency)
  {
    netTotalInPaymCurr = convertCurrency(fp, netTotalInPaymCurr, calcCurrency,
        baseFareCurrency, fp.applyNonIATARounding(_trx));
  }

  if (baseFareCurrency != paymentCurrency)
  {
    netTotalInPaymCurr =
      convertCurrency(fp, netTotalInPaymCurr, baseFareCurrency, paymentCurrency);
  }

  return netTotalInPaymCurr;
}

void
AgencyCommissions::storeFcCommissionData(
    FcCommissionInfoPairVec& fcAgcyCommDataCol,
    FareUsage& fu,
    const CommissionRuleData& crd,
    const MoneyAmount amt) const
{
  FcCommissionData* fcCommInfo = _trx.dataHandle().create<FcCommissionData>();
  if (fcCommInfo)
  {
    fcCommInfo->fcCommAmt() = amt;
    fcCommInfo->commRuleData() = crd;
    fcAgcyCommDataCol.push_back(std::make_pair(&fu, fcCommInfo));
  }
}

void
AgencyCommissions::storeFcCommissionDataInFC(
    const CarrierCode& valCxr,
    const FcCommissionInfoPairVec& fuAgcyCommRuleDataCol) const
{
  for (auto& p : fuAgcyCommRuleDataCol)
    if (p.first)
      p.first->fcCommInfoCol().insert(std::make_pair(valCxr, p.second));
}


} // tse

