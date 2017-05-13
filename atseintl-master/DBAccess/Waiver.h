// ---------------------------------------------------------------------------
// @ 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

#include <stdlib.h>

namespace tse
{

class Waiver
{

public:
  enum WaiverCode
  {
    DEATH = 1,
    ILLNES = 2,
    ACTS_OF_GOD = 3,
    COURT_APPEARANCE = 4,
    MILITARY = 5,
    TOUR_PACKAGE = 6,
    LACK_OF_TVL_DOC = 7
  };

  Waiver() : _itemNo(0), _inhibit(' '), _validityInd(' '), _waiver(0) {}
  ~Waiver() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }
  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }
  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }
  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }
  std::string& creatorId() { return _creatorId; }
  const std::string& creatorId() const { return _creatorId; }
  std::string& creatorBusinessUnit() { return _creatorBusinessUnit; }
  const std::string& creatorBusinessUnit() const { return _creatorBusinessUnit; }
  int& waiver() { return _waiver; }
  const int& waiver() const { return _waiver; }

  bool operator==(const Waiver& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_inhibit == rhs._inhibit) && (_validityInd == rhs._validityInd) &&
            (_creatorId == rhs._creatorId) && (_creatorBusinessUnit == rhs._creatorBusinessUnit) &&
            (_waiver == rhs._waiver));
  }

  static void dummyData(Waiver& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._inhibit = 'E';
    obj._validityInd = 'F';
    obj._creatorId = "aaaaaaaa";
    obj._creatorBusinessUnit = "bbbbbbbb";
    obj._waiver = 2;
  }

protected:
  VendorCode _vendor;
  int _itemNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _inhibit;
  Indicator _validityInd;
  std::string _creatorId;
  std::string _creatorBusinessUnit;
  int _waiver;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _creatorId);
    FLATTENIZE(archive, _creatorBusinessUnit);
    FLATTENIZE(archive, _waiver);
  }

protected:
private:
  Waiver(const Waiver&);
  Waiver& operator=(const Waiver&);
};

} // end tse namespace

