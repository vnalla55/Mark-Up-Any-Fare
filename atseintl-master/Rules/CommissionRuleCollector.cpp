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
#include "Rules/CommissionRuleCollector.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "Diagnostic/Diag867Collector.h"
#include "Rules/CommissionsProgramValidator.h"
#include "Rules/CommissionsRuleValidator.h"

namespace tse
{
namespace amc
{

CommissionRuleCollector::CommissionRuleCollector(
      PricingTrx& trx,
      Diag867Collector* diag867,
      const FarePath& fp,
      VCFMCommissionPrograms& vcxrProgs,
      VCFMPTFCommissionRules& vcxrRules) :
    _trx(trx),
    _diag867(diag867),
    _fp(fp),
    _vcfmCommProgs(vcxrProgs),
    _vcfmptfCommRules(vcxrRules)
{
}

/// Find valid commission-rules for all fare-market in the solution
void
CommissionRuleCollector::collectCommissionRules(
    const CarrierCode& valCxr,
    const ContractIdCol& contIdCol)
{
  if (_diag867 && _diag867->isDetailedDiagnostic())
    _diag867->printCommissionFCHeader();

  for (const PricingUnit* pu : _fp.pricingUnit())
  {
    if (!pu)
      continue;

    for (const FareUsage* fu : pu->fareUsage())
    {
      if (!fu || !fu->paxTypeFare() || !fu->paxTypeFare()->fareMarket())
        continue;

      if (_diag867 && _diag867->isDiagFCMatchFail(*fu))
        continue;

      if (_diag867 && _diag867->isDetailedDiagnostic())
        _diag867->printCommissionFC(*fu);

      CommissionProgramKey cpKey(valCxr, fu->paxTypeFare()->fareMarket());
      CommissionRuleKey crKey(valCxr, fu->paxTypeFare()->fareMarket(), fu->paxTypeFare());
      auto cpIt = _vcfmCommProgs.find(cpKey);
      if (cpIt != _vcfmCommProgs.end())
      {
        auto crIt = _vcfmptfCommRules.find(crKey);
        if (crIt != _vcfmptfCommRules.end())
        {
          storeLocalFcCommRules(valCxr, crKey, crIt->second);
          if(_diag867)
            _diag867->printReuseMessage();
          continue; // already collected commission-rules, next FC
        }
      }

      bool crFound = (cpIt != _vcfmCommProgs.end()) ?
        collectCommissionRuleForFcFromPrevalidatedProgram(*fu, cpIt->second, crKey) :
        collectCommissionRuleForFc(*fu, valCxr, cpKey, crKey, contIdCol);

      if (!crFound) // No valid commission-rule found for FC
      {
        ContractFcCommRuleDataMap contFcCommRuleCol;
        storeValCxrCommRulesPerKey(valCxr, crKey, contFcCommRuleCol);
      }

      if (_diag867 && _diag867->isDetailedDiagnostic())
      {
        if (crFound)
          _diag867->printFuClosingLine();
        else
          _diag867->printFuNoCommissionFound();
      }
    }
  }
}

/// Iterates through all contract-id and its corresponding comm-prog to collect comm-rules
bool CommissionRuleCollector::collectCommissionRuleForFcFromPrevalidatedProgram(
    const FareUsage& fu,
    const std::vector<ContractCommProgramData>& ccpdCol,
    const CommissionRuleKey& crKey)
{
  bool crFound = false;
  CommissionsRuleValidator crv(_trx, _fp, fu, _diag867);
  for (const ContractCommProgramData& ccpd : ccpdCol)
    for (const CommissionProgramInfo* cpInfo : ccpd.cpInfoCol)
      if (cpInfo && (!_diag867 || _diag867->isDiagProgramMatch(cpInfo->programId())))
        crFound |= collectCommissionRulesForContract(fu, crv, ccpd.ccInfo, *cpInfo, crKey);
  return crFound;
}

/// Iterates through all contract-id, find its comm-programs and comm-rules
bool CommissionRuleCollector::collectCommissionRuleForFc(
    const FareUsage& fu,
    const CarrierCode& valCxr,
    const CommissionProgramKey& cpKey,
    const CommissionRuleKey& crKey,
    const ContractIdCol& contIdCol)
{
  bool crFound = false;
  std::vector<ContractCommProgramData> ccpdCol;
  CommissionsRuleValidator crv(_trx, _fp, fu, _diag867);
  CommissionsProgramValidator cpv(_trx, _fp, fu, _diag867);
  for (const CommissionContractInfo* ccInfo : contIdCol)
  {
    if (!ccInfo ||
        (_diag867 && !_diag867->isDiagContractMatch(ccInfo->contractId())))
      continue;

    if (_diag867)
      _diag867->printDetailedCommissionContractInfo(*ccInfo);

    const std::vector<CommissionProgramInfo*>&
      cpiCol = _trx.dataHandle().getCommissionProgram(COS_VENDOR_CODE, ccInfo->contractId());
    if (!cpiCol.empty())
    {
      ContractCommProgramData ccpd(*ccInfo);
      for (auto cpInfo : cpiCol)
      {
        if (!cpInfo ||
            (_diag867 && !_diag867->isDiagProgramMatch(cpInfo->programId())))
          continue;

        if (cpv.isCommissionProgramMatched(*cpInfo, valCxr))
        {
          ccpd.cpInfoCol.push_back(cpInfo);
          crFound |= collectCommissionRulesForContract(fu, crv, *ccInfo, *cpInfo, crKey);
        }
      }

      if (!ccpd.cpInfoCol.empty())
        ccpdCol.push_back(ccpd);
    }
  }

  if (!ccpdCol.empty())
    _vcfmCommProgs.insert(VCFMCommProgPair(cpKey, ccpdCol));

  return crFound;
}

/// Collection commission-rules for give comm-id and its comm-programs
bool CommissionRuleCollector::collectCommissionRulesForContract(
    const FareUsage& fu,
    CommissionsRuleValidator& crv, /* non const but not modifying, bad code */
    const CommissionContractInfo& ccInfo,
    const CommissionProgramInfo& cpInfo,
    const CommissionRuleKey& crKey)
{
  if(_diag867 && (_diag867->isDDAMC() || _diag867->isDDPASSED()))
    _diag867->printCommissionRuleShortHeader();

  bool res = false;
  const std::vector<CommissionRuleInfo*>&
    criCol = _trx.dataHandle().getCommissionRule(COS_VENDOR_CODE, cpInfo.programId());
  if (!criCol.empty())
  {
    FcCommissionRuleData* fcRuleData = _trx.dataHandle().create<FcCommissionRuleData>();
    TSE_ASSERT(fcRuleData);

    CommRuleDataColPerCT& crdColPerCt = fcRuleData->commRuleDataColPerCommType();
    for (CommissionRuleInfo* crInfo : criCol)
    {
      if (!crInfo ||
          (_diag867 && !_diag867->isDiagRuleMatch(crInfo->commissionId())))
        continue;

      if (crv.isCommissionRuleMatched(*crInfo, crKey.valCxr))
        crdColPerCt[crInfo->commissionTypeId()].insert(CommissionRuleData(crInfo, &cpInfo, &ccInfo));
    }

    if (!crdColPerCt.empty())
    {
      ContractFcCommRuleDataMap contFcCommRuleCol;
      contFcCommRuleCol[ccInfo.contractId()] = fcRuleData;
      storeValCxrCommRulesPerKey(crKey.valCxr, crKey, contFcCommRuleCol);
      res = true;
    }
  }
  return res;
}

void
CommissionRuleCollector::storeValCxrCommRulesPerKey(
    const CarrierCode& valCxr,
    const CommissionRuleKey& crKey,
    const ContractFcCommRuleDataMap& contFcCommRuleCol)
{
  // master copy
  _vcfmptfCommRules.insert(VCFMPTFCommRulePair(crKey, contFcCommRuleCol));
  storeLocalFcCommRules(valCxr, crKey, contFcCommRuleCol);
}

void
CommissionRuleCollector::storeLocalFcCommRules(
    const CarrierCode& valCxr,
    const CommissionRuleKey& crKey,
    const ContractFcCommRuleDataMap& contFcCommRuleCol)
{
  VCFMPTFCommissionRules vcfmptfCommRules;
  vcfmptfCommRules[crKey] = contFcCommRuleCol;
  auto it = _commRulesPerKey.emplace(valCxr, vcfmptfCommRules);
  if (!it.second) // valCxr exists
  {
    VCFMPTFCommissionRules& refRulesCol = it.first->second;
    auto subIt = refRulesCol.emplace(crKey, contFcCommRuleCol);
    if (!subIt.second) // crKey exists
    {
      ContractFcCommRuleDataMap& refContRulesCol = subIt.first->second;
      if (!contFcCommRuleCol.empty())
      {
        refContRulesCol.emplace(
            contFcCommRuleCol.begin()->first, // cont-Id
            contFcCommRuleCol.begin()->second); // comm-data for given FC
      }
      else
        refRulesCol[crKey] = contFcCommRuleCol;
    }
  }
}

} // amc
} // tse
