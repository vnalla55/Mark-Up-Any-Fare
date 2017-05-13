//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/NoPNROptionsSeg.h"

namespace tse
{

class NoPNROptions
{
public:
  NoPNROptions()
    : _userApplType(' '),
      _wqNotPermitted(' '),
      _maxNoOptions(0),
      _wqSort(' '),
      _wqDuplicateAmounts(' '),
      _fareLineHeaderFormat(' '),
      _passengerDetailLineFormat(' '),
      _fareLinePTC(' '),
      _primePTCRefNo(' '),
      _secondaryPTCRefNo(' '),
      _fareLinePTCBreak(' '),
      _passengerDetailPTCBreak(' '),
      _negPassengerTypeMapping(' '),
      _noMatchOptionsDisplay(' '),
      _allMatchTrailerMessage(' '),
      _matchIntegratedTrailer(' '),
      _accompaniedTvlTrailerMsg(' '),
      _rbdMatchTrailerMsg(' '),
      _rbdNoMatchTrailerMsg(' '),
      _rbdNoMatchTrailerMsg2(' '),
      _displayFareRuleWarningMsg(' '),
      _displayFareRuleWarningMsg2(' '),
      _displayFinalWarningMsg(' '),
      _displayFinalWarningMsg2(' '),
      _displayTaxWarningMsg(' '),
      _displayTaxWarningMsg2(' '),
      _displayPrivateFareInd(' '),
      _displayNonCOCCurrencyInd(' '),
      _displayTruePTCInFareLine(' '),
      _applyROInFareDisplay(' '),
      _alwaysMapToADTPsgrType(' '),
      _noMatchRBDMessage(' '),
      _noMatchNoFaresErrorMsg(' '),
      _totalNoOptions(0)
  {
  }

  ~NoPNROptions()
  {
    std::vector<NoPNROptionsSeg*>::iterator SegIt;
    for (SegIt = _segs.begin(); SegIt != _segs.end(); SegIt++)
    { // Nuke 'em!
      delete *SegIt;
    }
  }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  Indicator& wqNotPermitted() { return _wqNotPermitted; }
  const Indicator& wqNotPermitted() const { return _wqNotPermitted; }

  int& maxNoOptions() { return _maxNoOptions; }
  const int& maxNoOptions() const { return _maxNoOptions; }

  Indicator& wqSort() { return _wqSort; }
  const Indicator& wqSort() const { return _wqSort; }

  Indicator& wqDuplicateAmounts() { return _wqDuplicateAmounts; }
  const Indicator& wqDuplicateAmounts() const { return _wqDuplicateAmounts; }

  Indicator& fareLineHeaderFormat() { return _fareLineHeaderFormat; }
  const Indicator& fareLineHeaderFormat() const { return _fareLineHeaderFormat; }

  Indicator& passengerDetailLineFormat() { return _passengerDetailLineFormat; }
  const Indicator& passengerDetailLineFormat() const { return _passengerDetailLineFormat; }

  Indicator& fareLinePTC() { return _fareLinePTC; }
  const Indicator& fareLinePTC() const { return _fareLinePTC; }

  Indicator& primePTCRefNo() { return _primePTCRefNo; }
  const Indicator& primePTCRefNo() const { return _primePTCRefNo; }

  Indicator& secondaryPTCRefNo() { return _secondaryPTCRefNo; }
  const Indicator& secondaryPTCRefNo() const { return _secondaryPTCRefNo; }

  Indicator& fareLinePTCBreak() { return _fareLinePTCBreak; }
  const Indicator& fareLinePTCBreak() const { return _fareLinePTCBreak; }

  Indicator& passengerDetailPTCBreak() { return _passengerDetailPTCBreak; }
  const Indicator& passengerDetailPTCBreak() const { return _passengerDetailPTCBreak; }

  Indicator& negPassengerTypeMapping() { return _negPassengerTypeMapping; }
  const Indicator& negPassengerTypeMapping() const { return _negPassengerTypeMapping; }

  Indicator& noMatchOptionsDisplay() { return _noMatchOptionsDisplay; }
  const Indicator& noMatchOptionsDisplay() const { return _noMatchOptionsDisplay; }

  Indicator& allMatchTrailerMessage() { return _allMatchTrailerMessage; }
  const Indicator& allMatchTrailerMessage() const { return _allMatchTrailerMessage; }

  Indicator& matchIntegratedTrailer() { return _matchIntegratedTrailer; }
  const Indicator& matchIntegratedTrailer() const { return _matchIntegratedTrailer; }

  Indicator& accompaniedTvlTrailerMsg() { return _accompaniedTvlTrailerMsg; }
  const Indicator& accompaniedTvlTrailerMsg() const { return _accompaniedTvlTrailerMsg; }

  Indicator& rbdMatchTrailerMsg() { return _rbdMatchTrailerMsg; }
  const Indicator& rbdMatchTrailerMsg() const { return _rbdMatchTrailerMsg; }

  Indicator& rbdNoMatchTrailerMsg() { return _rbdNoMatchTrailerMsg; }
  const Indicator& rbdNoMatchTrailerMsg() const { return _rbdNoMatchTrailerMsg; }

  Indicator& rbdNoMatchTrailerMsg2() { return _rbdNoMatchTrailerMsg2; }
  const Indicator& rbdNoMatchTrailerMsg2() const { return _rbdNoMatchTrailerMsg2; }

  Indicator& displayFareRuleWarningMsg() { return _displayFareRuleWarningMsg; }
  const Indicator& displayFareRuleWarningMsg() const { return _displayFareRuleWarningMsg; }

  Indicator& displayFareRuleWarningMsg2() { return _displayFareRuleWarningMsg2; }
  const Indicator& displayFareRuleWarningMsg2() const { return _displayFareRuleWarningMsg2; }

  Indicator& displayFinalWarningMsg() { return _displayFinalWarningMsg; }
  const Indicator& displayFinalWarningMsg() const { return _displayFinalWarningMsg; }

  Indicator& displayFinalWarningMsg2() { return _displayFinalWarningMsg2; }
  const Indicator& displayFinalWarningMsg2() const { return _displayFinalWarningMsg2; }

  Indicator& displayTaxWarningMsg() { return _displayTaxWarningMsg; }
  const Indicator& displayTaxWarningMsg() const { return _displayTaxWarningMsg; }

  Indicator& displayTaxWarningMsg2() { return _displayTaxWarningMsg2; }
  const Indicator& displayTaxWarningMsg2() const { return _displayTaxWarningMsg2; }

  Indicator& displayPrivateFareInd() { return _displayPrivateFareInd; }
  const Indicator& displayPrivateFareInd() const { return _displayPrivateFareInd; }

  Indicator& displayNonCOCCurrencyInd() { return _displayNonCOCCurrencyInd; }
  const Indicator& displayNonCOCCurrencyInd() const { return _displayNonCOCCurrencyInd; }

  Indicator& displayTruePTCInFareLine() { return _displayTruePTCInFareLine; }
  const Indicator& displayTruePTCInFareLine() const { return _displayTruePTCInFareLine; }

  Indicator& applyROInFareDisplay() { return _applyROInFareDisplay; }
  const Indicator& applyROInFareDisplay() const { return _applyROInFareDisplay; }

  Indicator& alwaysMapToADTPsgrType() { return _alwaysMapToADTPsgrType; }
  const Indicator& alwaysMapToADTPsgrType() const { return _alwaysMapToADTPsgrType; }

  Indicator& noMatchRBDMessage() { return _noMatchRBDMessage; }
  const Indicator& noMatchRBDMessage() const { return _noMatchRBDMessage; }

  Indicator& noMatchNoFaresErrorMsg() { return _noMatchNoFaresErrorMsg; }
  const Indicator& noMatchNoFaresErrorMsg() const { return _noMatchNoFaresErrorMsg; }

  int& totalNoOptions() { return _totalNoOptions; }
  const int& totalNoOptions() const { return _totalNoOptions; }

  std::vector<NoPNROptionsSeg*>& segs() { return _segs; }
  const std::vector<NoPNROptionsSeg*>& segs() const { return _segs; }

  bool operator==(const NoPNROptions& rhs) const
  {
    bool eq((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_loc1 == rhs._loc1) && (_wqNotPermitted == rhs._wqNotPermitted) &&
            (_maxNoOptions == rhs._maxNoOptions) && (_wqSort == rhs._wqSort) &&
            (_wqDuplicateAmounts == rhs._wqDuplicateAmounts) &&
            (_fareLineHeaderFormat == rhs._fareLineHeaderFormat) &&
            (_passengerDetailLineFormat == rhs._passengerDetailLineFormat) &&
            (_fareLinePTC == rhs._fareLinePTC) && (_primePTCRefNo == rhs._primePTCRefNo) &&
            (_secondaryPTCRefNo == rhs._secondaryPTCRefNo) &&
            (_fareLinePTCBreak == rhs._fareLinePTCBreak) &&
            (_passengerDetailPTCBreak == rhs._passengerDetailPTCBreak) &&
            (_negPassengerTypeMapping == rhs._negPassengerTypeMapping) &&
            (_noMatchOptionsDisplay == rhs._noMatchOptionsDisplay) &&
            (_allMatchTrailerMessage == rhs._allMatchTrailerMessage) &&
            (_matchIntegratedTrailer == rhs._matchIntegratedTrailer) &&
            (_accompaniedTvlTrailerMsg == rhs._accompaniedTvlTrailerMsg) &&
            (_rbdMatchTrailerMsg == rhs._rbdMatchTrailerMsg) &&
            (_rbdNoMatchTrailerMsg == rhs._rbdNoMatchTrailerMsg) &&
            (_rbdNoMatchTrailerMsg2 == rhs._rbdNoMatchTrailerMsg2) &&
            (_displayFareRuleWarningMsg == rhs._displayFareRuleWarningMsg) &&
            (_displayFareRuleWarningMsg2 == rhs._displayFareRuleWarningMsg2) &&
            (_displayFinalWarningMsg == rhs._displayFinalWarningMsg) &&
            (_displayFinalWarningMsg2 == rhs._displayFinalWarningMsg2) &&
            (_displayTaxWarningMsg == rhs._displayTaxWarningMsg) &&
            (_displayTaxWarningMsg2 == rhs._displayTaxWarningMsg2) &&
            (_displayPrivateFareInd == rhs._displayPrivateFareInd) &&
            (_displayNonCOCCurrencyInd == rhs._displayNonCOCCurrencyInd) &&
            (_displayTruePTCInFareLine == rhs._displayTruePTCInFareLine) &&
            (_applyROInFareDisplay == rhs._applyROInFareDisplay) &&
            (_alwaysMapToADTPsgrType == rhs._alwaysMapToADTPsgrType) &&
            (_noMatchRBDMessage == rhs._noMatchRBDMessage) &&
            (_noMatchNoFaresErrorMsg == rhs._noMatchNoFaresErrorMsg) &&
            (_totalNoOptions == rhs._totalNoOptions) && (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(NoPNROptions& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";

    LocKey::dummyData(obj._loc1);

    obj._wqNotPermitted = 'F';
    obj._maxNoOptions = 1;
    obj._wqSort = 'G';
    obj._wqDuplicateAmounts = 'H';
    obj._fareLineHeaderFormat = 'I';
    obj._passengerDetailLineFormat = 'J';
    obj._fareLinePTC = 'K';
    obj._primePTCRefNo = 'L';
    obj._secondaryPTCRefNo = 'M';
    obj._fareLinePTCBreak = 'N';
    obj._passengerDetailPTCBreak = 'O';
    obj._negPassengerTypeMapping = 'P';
    obj._noMatchOptionsDisplay = 'Q';
    obj._allMatchTrailerMessage = 'R';
    obj._matchIntegratedTrailer = 'S';
    obj._accompaniedTvlTrailerMsg = 'T';
    obj._rbdMatchTrailerMsg = 'U';
    obj._rbdNoMatchTrailerMsg = 'V';
    obj._rbdNoMatchTrailerMsg2 = 'W';
    obj._displayFareRuleWarningMsg = 'X';
    obj._displayFareRuleWarningMsg2 = 'Y';
    obj._displayFinalWarningMsg = 'Z';
    obj._displayFinalWarningMsg2 = 'a';
    obj._displayTaxWarningMsg = 'b';
    obj._displayTaxWarningMsg2 = 'c';
    obj._displayPrivateFareInd = 'd';
    obj._displayNonCOCCurrencyInd = 'e';
    obj._displayTruePTCInFareLine = 'f';
    obj._applyROInFareDisplay = 'g';
    obj._alwaysMapToADTPsgrType = 'h';
    obj._noMatchRBDMessage = 'i';
    obj._noMatchNoFaresErrorMsg = 'j';
    obj._totalNoOptions = 2;

    NoPNROptionsSeg* npos1 = new NoPNROptionsSeg;
    NoPNROptionsSeg* npos2 = new NoPNROptionsSeg;

    NoPNROptionsSeg::dummyData(*npos1);
    NoPNROptionsSeg::dummyData(*npos2);

    obj._segs.push_back(npos1);
    obj._segs.push_back(npos2);
  }

protected:
  // Join fields (w/Child: NOPNROPTIONSSEG)
  Indicator _userApplType;
  UserApplCode _userAppl;
  LocKey _loc1;

  Indicator _wqNotPermitted;
  int _maxNoOptions;
  Indicator _wqSort;
  Indicator _wqDuplicateAmounts;
  Indicator _fareLineHeaderFormat;
  Indicator _passengerDetailLineFormat;
  Indicator _fareLinePTC;
  Indicator _primePTCRefNo;
  Indicator _secondaryPTCRefNo;
  Indicator _fareLinePTCBreak;
  Indicator _passengerDetailPTCBreak;
  Indicator _negPassengerTypeMapping;
  Indicator _noMatchOptionsDisplay;
  Indicator _allMatchTrailerMessage;
  Indicator _matchIntegratedTrailer;
  Indicator _accompaniedTvlTrailerMsg;
  Indicator _rbdMatchTrailerMsg;
  Indicator _rbdNoMatchTrailerMsg;
  Indicator _rbdNoMatchTrailerMsg2;
  Indicator _displayFareRuleWarningMsg;
  Indicator _displayFareRuleWarningMsg2;
  Indicator _displayFinalWarningMsg;
  Indicator _displayFinalWarningMsg2;
  Indicator _displayTaxWarningMsg;
  Indicator _displayTaxWarningMsg2;
  Indicator _displayPrivateFareInd;
  Indicator _displayNonCOCCurrencyInd;
  Indicator _displayTruePTCInFareLine;
  Indicator _applyROInFareDisplay;
  Indicator _alwaysMapToADTPsgrType;
  Indicator _noMatchRBDMessage;
  Indicator _noMatchNoFaresErrorMsg;
  int _totalNoOptions;
  std::vector<NoPNROptionsSeg*> _segs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _wqNotPermitted);
    FLATTENIZE(archive, _maxNoOptions);
    FLATTENIZE(archive, _wqSort);
    FLATTENIZE(archive, _wqDuplicateAmounts);
    FLATTENIZE(archive, _fareLineHeaderFormat);
    FLATTENIZE(archive, _passengerDetailLineFormat);
    FLATTENIZE(archive, _fareLinePTC);
    FLATTENIZE(archive, _primePTCRefNo);
    FLATTENIZE(archive, _secondaryPTCRefNo);
    FLATTENIZE(archive, _fareLinePTCBreak);
    FLATTENIZE(archive, _passengerDetailPTCBreak);
    FLATTENIZE(archive, _negPassengerTypeMapping);
    FLATTENIZE(archive, _noMatchOptionsDisplay);
    FLATTENIZE(archive, _allMatchTrailerMessage);
    FLATTENIZE(archive, _matchIntegratedTrailer);
    FLATTENIZE(archive, _accompaniedTvlTrailerMsg);
    FLATTENIZE(archive, _rbdMatchTrailerMsg);
    FLATTENIZE(archive, _rbdNoMatchTrailerMsg);
    FLATTENIZE(archive, _rbdNoMatchTrailerMsg2);
    FLATTENIZE(archive, _displayFareRuleWarningMsg);
    FLATTENIZE(archive, _displayFareRuleWarningMsg2);
    FLATTENIZE(archive, _displayFinalWarningMsg);
    FLATTENIZE(archive, _displayFinalWarningMsg2);
    FLATTENIZE(archive, _displayTaxWarningMsg);
    FLATTENIZE(archive, _displayTaxWarningMsg2);
    FLATTENIZE(archive, _displayPrivateFareInd);
    FLATTENIZE(archive, _displayNonCOCCurrencyInd);
    FLATTENIZE(archive, _displayTruePTCInFareLine);
    FLATTENIZE(archive, _applyROInFareDisplay);
    FLATTENIZE(archive, _alwaysMapToADTPsgrType);
    FLATTENIZE(archive, _noMatchRBDMessage);
    FLATTENIZE(archive, _noMatchNoFaresErrorMsg);
    FLATTENIZE(archive, _totalNoOptions);
    FLATTENIZE(archive, _segs);
  }

protected:
private:
  NoPNROptions(const NoPNROptions&);
  NoPNROptions& operator=(const NoPNROptions&);
};
}

