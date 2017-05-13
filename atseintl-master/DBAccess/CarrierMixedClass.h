//----------------------------------------------------------------------------
//     CarrierMixedClass.h
//     2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//------------------------------------------------------------------------------

#pragma once

#include "DBAccess/CarrierMixedClassSeg.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class CarrierMixedClass
{
public:
  CarrierMixedClass() {}

  ~CarrierMixedClass()
  { // Nuke the Kids!
    std::vector<CarrierMixedClassSeg*>::iterator SegIt;
    for (SegIt = _segs.begin(); SegIt != _segs.end(); SegIt++)
    {
      delete *SegIt;
    }
  }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<tse::CarrierMixedClassSeg*>& segs() { return _segs; }
  const std::vector<tse::CarrierMixedClassSeg*>& segs() const { return _segs; }

  bool operator==(const CarrierMixedClass& rhs) const
  {
    bool eq = ((_carrier == rhs._carrier) && (_createDate == rhs._createDate) &&
               (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
               (_expireDate == rhs._expireDate) && (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(CarrierMixedClass& obj)
  {
    obj._carrier = "ABC";
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._expireDate = time(nullptr);

    CarrierMixedClassSeg* cmcs1 = new CarrierMixedClassSeg;
    CarrierMixedClassSeg* cmcs2 = new CarrierMixedClassSeg;

    CarrierMixedClassSeg::dummyData(*cmcs1);
    CarrierMixedClassSeg::dummyData(*cmcs2);

    obj._segs.push_back(cmcs1);
    obj._segs.push_back(cmcs2);
  }

protected:
  CarrierCode _carrier;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _expireDate;
  std::vector<tse::CarrierMixedClassSeg*> _segs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _segs);
  }

protected:
private:
  CarrierMixedClass(const CarrierMixedClass&);
  CarrierMixedClass& operator=(const CarrierMixedClass&);
};
}
