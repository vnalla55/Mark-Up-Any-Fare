//----------------------------------------------------------------------------
//
//      File:           EndOnEnd.h
//      Description:    Combinability cat 104 data
//      Created:        3/30/2004
//      Authors:        Roger Kelly
//
//     (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/EndOnEndSegment.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class EndOnEnd : public RuleItemInfo
{
public:
  EndOnEnd() = default;
  EndOnEnd(const EndOnEnd&) = delete;
  EndOnEnd& operator=(const EndOnEnd&) = delete;

  ~EndOnEnd()
  { // Nuke the Kids!
    for (const auto endOnEndSegment : _segs)
      delete endOnEndSegment;
  }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  Indicator& eoeRestInd() { return _eoeRestInd; }
  const Indicator& eoeRestInd() const { return _eoeRestInd; }

  Indicator& eoeNormalInd() { return _eoeNormalInd; }
  const Indicator& eoeNormalInd() const { return _eoeNormalInd; }

  Indicator& eoespecialInd() { return _eoespecialInd; }
  const Indicator& eoespecialInd() const { return _eoespecialInd; }

  Indicator& eoespecialApplInd() { return _eoespecialApplInd; }
  const Indicator& eoespecialApplInd() const { return _eoespecialApplInd; }

  Indicator& uscatransborderInd() { return _uscatransborderInd; }
  const Indicator& uscatransborderInd() const { return _uscatransborderInd; }

  Indicator& domInd() { return _domInd; }
  const Indicator& domInd() const { return _domInd; }

  Indicator& intlInd() { return _intlInd; }
  const Indicator& intlInd() const { return _intlInd; }

  Indicator& sameCarrierInd() { return _sameCarrierInd; }
  const Indicator& sameCarrierInd() const { return _sameCarrierInd; }

  Indicator& fareTypeLocAppl() { return _fareTypeLocAppl; }
  const Indicator& fareTypeLocAppl() const { return _fareTypeLocAppl; }

  LocType& fareTypeLoc1Type() { return _fareTypeLoc1Type; }
  const LocType& fareTypeLoc1Type() const { return _fareTypeLoc1Type; }

  LocCode& fareTypeLoc1() { return _fareTypeLoc1; }
  const LocCode& fareTypeLoc1() const { return _fareTypeLoc1; }

  LocType& fareTypeLoc2Type() { return _fareTypeLoc2Type; }
  const LocType& fareTypeLoc2Type() const { return _fareTypeLoc2Type; }

  LocCode& fareTypeLoc2() { return _fareTypeLoc2; }
  const LocCode& fareTypeLoc2() const { return _fareTypeLoc2; }

  Indicator& constLocAppl() { return _constLocAppl; }
  const Indicator& constLocAppl() const { return _constLocAppl; }

  LocType& constLoc1Type() { return _constLoc1Type; }
  const LocType& constLoc1Type() const { return _constLoc1Type; }

  LocCode& constLoc1() { return _constLoc1; }
  const LocCode& constLoc1() const { return _constLoc1; }

  LocType& constLoc2Type() { return _constLoc2Type; }
  const LocType& constLoc2Type() const { return _constLoc2Type; }

  LocCode& constLoc2() { return _constLoc2; }
  const LocCode& constLoc2() const { return _constLoc2; }

  Indicator& tktInd() { return _tktInd; }
  const Indicator& tktInd() const { return _tktInd; }

  Indicator& abacombInd() { return _abacombInd; }
  const Indicator& abacombInd() const { return _abacombInd; }

  Indicator& allsegsInd() { return _allsegsInd; }
  const Indicator& allsegsInd() const { return _allsegsInd; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  const Indicator inhibit() const { return _inhibit; }
  Indicator& inhibit() { return _inhibit; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<EndOnEndSegment*>& segs() { return _segs; }
  const std::vector<EndOnEndSegment*>& segs() const { return _segs; }

  bool operator==(const EndOnEnd& rhs) const
  {
    bool eq((RuleItemInfo::operator==(rhs)) && (_segCnt == rhs._segCnt) &&
            (_eoeRestInd == rhs._eoeRestInd) && (_eoeNormalInd == rhs._eoeNormalInd) &&
            (_eoespecialInd == rhs._eoespecialInd) &&
            (_eoespecialApplInd == rhs._eoespecialApplInd) &&
            (_uscatransborderInd == rhs._uscatransborderInd) && (_domInd == rhs._domInd) &&
            (_intlInd == rhs._intlInd) && (_sameCarrierInd == rhs._sameCarrierInd) &&
            (_fareTypeLocAppl == rhs._fareTypeLocAppl) &&
            (_fareTypeLoc1Type == rhs._fareTypeLoc1Type) && (_fareTypeLoc1 == rhs._fareTypeLoc1) &&
            (_fareTypeLoc2Type == rhs._fareTypeLoc2Type) && (_fareTypeLoc2 == rhs._fareTypeLoc2) &&
            (_constLocAppl == rhs._constLocAppl) && (_constLoc1Type == rhs._constLoc1Type) &&
            (_constLoc1 == rhs._constLoc1) && (_constLoc2Type == rhs._constLoc2Type) &&
            (_constLoc2 == rhs._constLoc2) && (_tktInd == rhs._tktInd) &&
            (_abacombInd == rhs._abacombInd) && (_allsegsInd == rhs._allsegsInd) &&
            (_unavailTag == rhs._unavailTag) && (_inhibit == rhs._inhibit) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(EndOnEnd& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._segCnt = 1;
    obj._eoeRestInd = 'A';
    obj._eoeNormalInd = 'B';
    obj._eoespecialInd = 'C';
    obj._eoespecialApplInd = 'D';
    obj._uscatransborderInd = 'E';
    obj._domInd = 'F';
    obj._intlInd = 'G';
    obj._sameCarrierInd = 'H';
    obj._fareTypeLocAppl = 'I';
    obj._fareTypeLoc1Type = MARKET;
    obj._fareTypeLoc1 = "JKLMNOPQ";
    obj._fareTypeLoc2Type = NATION;
    obj._fareTypeLoc2 = "RSTUVWXY";
    obj._constLocAppl = 'Z';
    obj._constLoc1Type = SUBAREA;
    obj._constLoc1 = "abcdefgh";
    obj._constLoc2Type = IATA_AREA;
    obj._constLoc2 = "ijklmnop";
    obj._tktInd = 'q';
    obj._abacombInd = 'r';
    obj._allsegsInd = 's';
    obj._unavailTag = 't';
    obj._inhibit = 'u';
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);

    EndOnEndSegment* eoes1 = new EndOnEndSegment;
    EndOnEndSegment* eoes2 = new EndOnEndSegment;

    EndOnEndSegment::dummyData(*eoes1);
    EndOnEndSegment::dummyData(*eoes2);

    obj._segs.push_back(eoes1);
    obj._segs.push_back(eoes2);
  }

private:
  int _segCnt = 0;
  Indicator _eoeRestInd = ' ';
  Indicator _eoeNormalInd = ' ';
  Indicator _eoespecialInd = ' ';
  Indicator _eoespecialApplInd = ' ';
  Indicator _uscatransborderInd = ' ';
  Indicator _domInd = ' ';
  Indicator _intlInd = ' ';
  Indicator _sameCarrierInd = ' ';
  Indicator _fareTypeLocAppl = ' ';
  LocType _fareTypeLoc1Type = LocType::UNKNOWN_LOC;
  LocCode _fareTypeLoc1;
  LocType _fareTypeLoc2Type = LocType::UNKNOWN_LOC;
  LocCode _fareTypeLoc2;
  Indicator _constLocAppl = ' ';
  LocType _constLoc1Type = LocType::UNKNOWN_LOC;
  LocCode _constLoc1;
  LocType _constLoc2Type = LocType::UNKNOWN_LOC;
  LocCode _constLoc2;
  Indicator _tktInd = ' ';
  Indicator _abacombInd = ' ';
  Indicator _allsegsInd = ' ';
  Indicator _unavailTag = ' ';
  Indicator _inhibit = ' '; // Inhibit now checked at App Level
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<EndOnEndSegment*> _segs;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _eoeRestInd);
    FLATTENIZE(archive, _eoeNormalInd);
    FLATTENIZE(archive, _eoespecialInd);
    FLATTENIZE(archive, _eoespecialApplInd);
    FLATTENIZE(archive, _uscatransborderInd);
    FLATTENIZE(archive, _domInd);
    FLATTENIZE(archive, _intlInd);
    FLATTENIZE(archive, _sameCarrierInd);
    FLATTENIZE(archive, _fareTypeLocAppl);
    FLATTENIZE(archive, _fareTypeLoc1Type);
    FLATTENIZE(archive, _fareTypeLoc1);
    FLATTENIZE(archive, _fareTypeLoc2Type);
    FLATTENIZE(archive, _fareTypeLoc2);
    FLATTENIZE(archive, _constLocAppl);
    FLATTENIZE(archive, _constLoc1Type);
    FLATTENIZE(archive, _constLoc1);
    FLATTENIZE(archive, _constLoc2Type);
    FLATTENIZE(archive, _constLoc2);
    FLATTENIZE(archive, _tktInd);
    FLATTENIZE(archive, _abacombInd);
    FLATTENIZE(archive, _allsegsInd);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _segs);
  }

};
}
