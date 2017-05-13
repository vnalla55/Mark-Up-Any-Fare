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

#pragma once

#include "Common/CommissionKeys.h"
#include "Common/TseCodeTypes.h"

#include <vector>
#include <map>

namespace tse
{
class CommissionContractInfo;
class CommissionProgramInfo;
class CommissionsRuleValidator;
class Diag867Collector;
class FarePath;
class FareUsage;
class PricingTrx;

namespace amc
{

/**
 * \class CommissionRuleCollectorTest
 *
 * Calls CommissionProgram and CommissionRule validators and collect valid
 * CommissionProgramInfo and CommissionRulesInfo in VCFMCommissionPrograms and
 * VCFMPTFCommissionRules. It also create a local VCFMPTFCommissionRule map for
 * calculating commissions
 */

class CommissionRuleCollector
{
  friend class CommissionRuleCollectorTest;
  public:
  CommissionRuleCollector(
      PricingTrx& pTrx,
      Diag867Collector* diag867,
      const FarePath& fp,
      VCFMCommissionPrograms& vcfmCommProgs,
      VCFMPTFCommissionRules& vcfmptfCommRules);
  virtual ~CommissionRuleCollector(){};

  void collectCommissionRules(const CarrierCode& valCxr, const ContractIdCol& contIdCol);
  const VCFMPTFCommRulesPerValCxr& commissionRulesPerKey() const { return _commRulesPerKey; }
  VCFMPTFCommRulesPerValCxr& commissionRulesPerKey() { return _commRulesPerKey; }

  private:
  bool collectCommissionRuleForFcFromPrevalidatedProgram(
      const FareUsage& fu,
      const std::vector<ContractCommProgramData>& ccpdCol,
      const CommissionRuleKey& crKey);

  bool collectCommissionRuleForFc(
      const FareUsage& fu,
      const CarrierCode& valCxr,
      const CommissionProgramKey& cpKey,
      const CommissionRuleKey& crKey,
      const ContractIdCol& contIdCol);

  bool collectCommissionRulesForContract(
      const FareUsage& fu,
      CommissionsRuleValidator& crv,
      const CommissionContractInfo& ccInfo,
      const CommissionProgramInfo& cpInfo,
      const CommissionRuleKey& crKey);

  void storeValCxrCommRulesPerKey(
      const CarrierCode& valCxr,
      const CommissionRuleKey& crKey,
      const ContractFcCommRuleDataMap& contFcCommRuleCol);

  void storeLocalFcCommRules(
      const CarrierCode& valCxr,
      const CommissionRuleKey& crKey,
      const ContractFcCommRuleDataMap& contFcCommRuleCol);

  private:
  PricingTrx& _trx;
  Diag867Collector* _diag867;
  const FarePath& _fp;
  VCFMCommissionPrograms _vcfmCommProgs;
  VCFMPTFCommissionRules _vcfmptfCommRules;

  VCFMPTFCommRulesPerValCxr _commRulesPerKey;
};

}
}


