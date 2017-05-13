#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/GlobalDirSeg.h"

#include <vector>

namespace tse
{

class GlobalDir
{
public:
  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& displayOnlyInd() { return _displayOnlyInd; }
  const Indicator& displayOnlyInd() const { return _displayOnlyInd; }

  std::string& description() { return _description; }
  const std::string& description() const { return _description; }

  std::vector<GlobalDirSeg>& segs()
  {
    return _segs;
  };
  const std::vector<GlobalDirSeg>& segs() const
  {
    return _segs;
  };

  bool operator==(const GlobalDir& rhs) const
  {
    return ((_globalDir == rhs._globalDir) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_lockDate == rhs._lockDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_memoNo == rhs._memoNo) && (_displayOnlyInd == rhs._displayOnlyInd) &&
            (_description == rhs._description) &&
            (_creatorBusinessUnit == rhs._creatorBusinessUnit) && (_creatorId == rhs._creatorId) &&
            (_segs == rhs._segs));
  }

  static void dummyData(GlobalDir& obj)
  {
    obj._globalDir = GlobalDirection::NO_DIR;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._lockDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._memoNo = 1;
    obj._displayOnlyInd = 'A';
    obj._description = "aaaaaaaa";
    obj._creatorBusinessUnit = "bbbbbbbb";
    obj._creatorId = "cccccccc";

    GlobalDirSeg gds1;
    GlobalDirSeg gds2;

    GlobalDirSeg::dummyData(gds1);
    GlobalDirSeg::dummyData(gds2);

    obj._segs.push_back(gds1);
    obj._segs.push_back(gds2);
  }

private:
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _lockDate;
  DateTime _effDate;
  DateTime _discDate;
  int _memoNo = 0;
  Indicator _displayOnlyInd = ' ';
  std::string _description;
  std::string _creatorBusinessUnit;
  std::string _creatorId;
  std::vector<GlobalDirSeg> _segs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _memoNo);
    FLATTENIZE(archive, _displayOnlyInd);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _creatorBusinessUnit);
    FLATTENIZE(archive, _creatorId);
    FLATTENIZE(archive, _segs);
  }

};
}

