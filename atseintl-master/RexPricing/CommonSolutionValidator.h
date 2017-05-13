//-------------------------------------------------------------------
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareTypeTable.h"
#include "Rules/RuleUtil.h"

namespace tse
{

struct FareRestrictions
{
  static constexpr Indicator NOT_APPLICABLE = ' ';
  static constexpr Indicator RULE_INDICATOR_RULE_NUMBER = 'R';
  static constexpr Indicator RULE_INDICATOR_SAME_RULE = 'S';
  static constexpr Indicator RULE_INDICATOR_RULE_NOT_THE_SAME = 'N';
  static constexpr Indicator HIGHER_OR_EQUAL_AMOUNT = 'X';
  static constexpr Indicator HIGHER_AMOUNT = 'Y';
  static constexpr Indicator FARE_TYPE_IND_FARE_CLASS = 'F';
  static constexpr Indicator FARE_TYPE_IND_FAMILY_FARE = 'M';
  static constexpr Indicator FARE_TYPE_IND_SECOND_POS = 'S';
  static constexpr Indicator SAME_IND_FARE_TYPE = 'S';
  static constexpr Indicator SAME_IND_FARE_CLASS = 'X';
};

class CommonSolutionValidator
{
  friend class CommonSolutionValidatorTest;

  class FareTypeFinder
  {
    const FareTypeAbbrev& _fareType;

  public:
    FareTypeFinder(const FareTypeAbbrev& fareType) : _fareType(fareType) {}

    bool operator()(const FareTypeTable* fareTypeTable)
    {
      return fareTypeTable->fareType() == _fareType;
    }
  };

public:
  static constexpr Indicator BLANK = ' ';
  static constexpr Indicator NORMAL_FARE = 'N';
  static constexpr Indicator SPECIAL_FARE = 'S';
  static constexpr Indicator PERMITTED_FARE_TYPE = ' ';
  static constexpr Indicator FORBIDDEN_FARE_TYPE = 'N';
  static constexpr Indicator ASTERISK = '*';

  bool checkFareNormalSpecial(const Indicator& normalSpecialInd, const PaxTypeFare& ptf)
  {
    switch (normalSpecialInd)
    {
    case BLANK:
      return true;
    case NORMAL_FARE:
      return ptf.isNormal();
    case SPECIAL_FARE:
      return ptf.isSpecial();
    }
    return false;
  }

  bool checkOWRT(const Indicator& owrtInd, const PaxTypeFare& ptf)
  {
    switch (owrtInd)
    {
    case ALL_WAYS:
      return true;
    case ONE_WAY_MAY_BE_DOUBLED:
      return !ptf.isRoundTrip();
    case ROUND_TRIP_MAYNOT_BE_HALVED:
      return ptf.isRoundTrip();
    }
    return false;
  }

  bool checkFareClassCode(const Indicator& fareTypeInd,
                          const FareClassCode& fareClassRule,
                          const PaxTypeFare& ptf)
  {
    if (fareTypeInd == FareRestrictions::NOT_APPLICABLE)
      return true;

    FareClassCode fareClassFare = ptf.fare()->fareInfo()->fareClass();

    if (fareTypeInd == FareRestrictions::FARE_TYPE_IND_SECOND_POS)
    {
      if (fareClassFare.length() > 1)
        fareClassFare = fareClassFare.substr(1, fareClassFare.length());
      else
        return false;
    }

    return RuleUtil::matchFareClass(fareClassRule.c_str(), fareClassFare.c_str());
  }

  bool matchWildcardRuleNumber(const RuleNumber& seqRuleNo, const RuleNumber& ptfRuleNo)
  {

    if (seqRuleNo.size() == 4 && ptfRuleNo.size() == 4 && seqRuleNo[3] == ASTERISK)
      return seqRuleNo[0] == ptfRuleNo[0] && seqRuleNo[1] == ptfRuleNo[1];

    else
      return seqRuleNo == ptfRuleNo;
  }

  bool
  checkFareTypeTable(const std::vector<FareTypeTable*>& fareTypeTables, const FareType& fareType)
  {
    const std::vector<FareTypeTable*>::const_iterator found =
        std::find_if(fareTypeTables.begin(), fareTypeTables.end(), FareTypeFinder(fareType));

    if ((found != fareTypeTables.end() &&
         fareTypeTables.front()->fareTypeAppl() == PERMITTED_FARE_TYPE) ||
        (found == fareTypeTables.end() &&
         fareTypeTables.front()->fareTypeAppl() == FORBIDDEN_FARE_TYPE))
      return true;

    return false;
  }
};
}

