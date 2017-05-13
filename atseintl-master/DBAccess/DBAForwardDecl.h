#pragma once

#include <vector>
#include <utility>

namespace tse
{
class FootNoteCtrlInfo;
class FareByRuleCtrlInfo;
class GeneralFareRuleInfo;

class CombinabilityRuleInfo;

class CategoryRuleItemInfo;

// typedef for possible further extension
typedef bool GeoMatchResult;

typedef std::pair<const GeneralFareRuleInfo*, GeoMatchResult> GeneralFareRuleInfoPair;
typedef std::vector<GeneralFareRuleInfoPair> GeneralFareRuleInfoVec;
// FB Display
typedef GeneralFareRuleInfo FareRuleRecord2Info;
typedef GeneralFareRuleInfo GeneralRuleRecord2Info;

typedef std::pair<const FootNoteCtrlInfo*, GeoMatchResult> FootNoteCtrlInfoPair;
typedef std::vector<FootNoteCtrlInfoPair> FootNoteCtrlInfoVec;
// FB Display
typedef FootNoteCtrlInfo FootNoteRecord2Info;

typedef std::pair<const FareByRuleCtrlInfo*, GeoMatchResult> FareByRuleCtrlInfoPair;
typedef std::vector<FareByRuleCtrlInfoPair> FareByRuleCtrlInfoVec;
}
