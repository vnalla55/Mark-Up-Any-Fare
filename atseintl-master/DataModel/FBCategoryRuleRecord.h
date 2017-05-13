
//-------------------------------------------------------------------
//
//  File:	FBCategoryRuleRecord.h
//  Authors:	Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
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
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/DBAForwardDecl.h"

namespace tse
{

// FB Display

// ------------------------------------------
//	Class: FBCategoryRuleRecord
// ------------------------------------------
class FBCategoryRuleRecord
{
public:
  bool hasFareRuleRecord2Info() { return fareRuleRecord2Info() ? true : false; }
  bool hasFootNoteRecord2Info() { return footNoteRecord2Info() ? true : false; }
  bool hasGeneralRuleRecord2Info() { return generalRuleRecord2Info() ? true : false; }
  bool hasCombinabilityRecord2Info() { return combinabilityRecord2Info() ? true : false; } // Cat10
  bool hasFareByRuleRecord2Info() { return fareByRuleRecord2Info() ? true : false; } // Cat25

  bool isAllEmpty()
  {
    return (!hasFareRuleRecord2Info() && !hasFootNoteRecord2Info() &&
            !hasGeneralRuleRecord2Info() && !hasCombinabilityRecord2Info() && // Cat10
            !hasFareByRuleRecord2Info() // Cat25
            );
  }

  bool& isRecordScopeDomestic() { return _isRecordScopeDomestic; }
  const bool& isRecordScopeDomestic() const { return _isRecordScopeDomestic; }

  const FareRuleRecord2Info*& fareRuleRecord2Info() { return _fareRuleRecord2Info; }
  const FareRuleRecord2Info* fareRuleRecord2Info() const { return _fareRuleRecord2Info; }

  const FootNoteRecord2Info*& footNoteRecord2Info() { return _footNoteRecord2Info; }
  const FootNoteRecord2Info* footNoteRecord2Info() const { return _footNoteRecord2Info; }

  const GeneralRuleRecord2Info*& generalRuleRecord2Info() { return _generalRuleRecord2Info; }
  const GeneralRuleRecord2Info* generalRuleRecord2Info() const { return _generalRuleRecord2Info; }

  // For Cat10
  const CombinabilityRuleInfo*& combinabilityRecord2Info() { return _combinabilityRecord2Info; }
  const CombinabilityRuleInfo* combinabilityRecord2Info() const
  {
    return _combinabilityRecord2Info;
  }

  // For Cat25
  const FareByRuleCtrlInfo*& fareByRuleRecord2Info() { return _fareByRuleRecord2Info; }
  const FareByRuleCtrlInfo* fareByRuleRecord2Info() const { return _fareByRuleRecord2Info; }

  bool& isFareRuleRecord2InfoLocationSwapped() { return _isFareRuleRecord2InfoLocationSwapped; }
  const bool& isFareRuleRecord2InfoLocationSwapped() const
  {
    return _isFareRuleRecord2InfoLocationSwapped;
  }

  bool& isFootNoteRecord2InfoLocationSwapped() { return _isFootNoteRecord2InfoLocationSwapped; }
  const bool& isFootNoteRecord2InfoLocationSwapped() const
  {
    return _isFootNoteRecord2InfoLocationSwapped;
  }

  bool& isGeneralRuleRecord2InfoLocationSwapped()
  {
    return _isGeneralRuleRecord2InfoLocationSwapped;
  }
  const bool& isGeneralRuleRecord2InfoLocationSwapped() const
  {
    return _isGeneralRuleRecord2InfoLocationSwapped;
  }

  bool& isFareByRuleRecord2InfoCtrLocationSwapped()
  {
    return _isFareByRuleRecord2InfoCtrLocationSwapped;
  }
  const bool& isFareByRuleRecord2InfoCtrLocationSwapped() const
  {
    return _isFareByRuleRecord2InfoCtrLocationSwapped;
  }

  FareClassCode& fareBasis() { return _fareBasis; }
  const FareClassCode& fareBasis() const { return _fareBasis; }

  FBCategoryRuleRecord()
    : _isRecordScopeDomestic(true),
      _fareRuleRecord2Info(nullptr),
      _footNoteRecord2Info(nullptr),
      _generalRuleRecord2Info(nullptr),
      _combinabilityRecord2Info(nullptr),
      _fareByRuleRecord2Info(nullptr),
      _isFareRuleRecord2InfoLocationSwapped(false),
      _isFootNoteRecord2InfoLocationSwapped(false),
      _isGeneralRuleRecord2InfoLocationSwapped(false),
      _isFareByRuleRecord2InfoCtrLocationSwapped(false)
  {
  }

  ~FBCategoryRuleRecord() {}

private:
  bool _isRecordScopeDomestic;

  // We can have Either following 3
  const FareRuleRecord2Info* _fareRuleRecord2Info;
  const FootNoteRecord2Info* _footNoteRecord2Info;
  const GeneralRuleRecord2Info* _generalRuleRecord2Info;

  // Or
  const CombinabilityRuleInfo* _combinabilityRecord2Info; // For Cat10

  // Or
  const FareByRuleCtrlInfo* _fareByRuleRecord2Info; // For Cat25

  bool _isFareRuleRecord2InfoLocationSwapped;
  bool _isFootNoteRecord2InfoLocationSwapped;
  bool _isGeneralRuleRecord2InfoLocationSwapped;
  bool _isFareByRuleRecord2InfoCtrLocationSwapped;

  FareClassCode _fareBasis;
};

} // tse namespace
