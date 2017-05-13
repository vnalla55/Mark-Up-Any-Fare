//-------------------------------------------------------------------
//
//  File:        FBDisplay.h
//  Created:     Apr 04, 2005
//  Design:      Partha Kumar Chakraborti
//  Authors:
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
//-------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DBAForwardDecl.h"

#include <iostream>
#include <map>
#include <string>

namespace tse
{
class FareDisplayTrx;
class DataHandle;
class FBCategoryRuleRecord;
class PaxTypeFare;
class PricingTrx;

class FBDisplay
{
public:
  virtual const char getCategoryApplicability(const bool isDutyCode7Or8, const int16_t cat) const;
  virtual void setRuleRecordData(CatNumber cat, FBCategoryRuleRecord* rd);

  virtual void setRuleRecordData(CatNumber cat, FBCategoryRuleRecord* rd, bool useBaseFareRule);

  static void setRuleRecordData(CatNumber cat,
                                PricingTrx& trx,
                                const PaxTypeFare& srcPaxTypeFare,
                                const PaxTypeFare& destPaxTypeFare,
                                bool useBaseFareRule = false);

  virtual FBCategoryRuleRecord* getBaseFareRuleRecordData(CatNumber cat) const;

  virtual FBCategoryRuleRecord* getRuleRecordData(CatNumber cat) const;

  virtual const std::map<CatNumber, FBCategoryRuleRecord*>& fbCategoryRuleRecordMap() const
  {
    return _fbCategoryRuleRecordMap;
  }

  virtual void setData(CatNumber cat,
                       const FareRuleRecord2Info* fRR2,
                       const FootNoteRecord2Info* fNR2,
                       const GeneralRuleRecord2Info* gRR2,
                       const FareClassCode& fareBasis,
                       DataHandle& dataHandle);

  virtual void setLocationSwapped(CatNumber cat, bool isLocationSwapped, DataHandle& dataHandle);

  virtual void setData(const CombinabilityRuleInfo* cRR2,
                       const FareClassCode& fareBasis,
                       DataHandle& dataHandle,
                       bool isDomestic);

  virtual void
  setData(const FareByRuleCtrlInfo* cFBRR2, const FareClassCode& fareBasis, DataHandle& dataHandle);

  std::map<CatNumber, std::string>& ruleTextMap() { return _ruleTextMap; }
  const std::map<CatNumber, std::string>& ruleTextMap() const { return _ruleTextMap; }

  std::string& mxCombinabilityInfo() { return _mxCombinabilityInfo; }
  const std::string& mxCombinabilityInfo() const { return _mxCombinabilityInfo; }

  virtual void initialize(FareDisplayTrx& trx);

  virtual ~FBDisplay() {}

private:
  std::map<CatNumber, FBCategoryRuleRecord*> _fbCategoryRuleRecordMap;
  std::map<CatNumber, FBCategoryRuleRecord*> _baseFarefbCategoryRuleRecordMap;
  std::map<CatNumber, std::string> _ruleTextMap;
  std::string _mxCombinabilityInfo;
};
} // end of namespace tse

