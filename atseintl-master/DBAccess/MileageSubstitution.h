//----------------------------------------------------------------------------
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef MILEAGESUBSTITUTION_H
#define MILEAGESUBSTITUTION_H

#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class MileageSubstitution
{
public:
  LocCode& substitutionLoc() { return _substitutionLoc; }
  const LocCode& substitutionLoc() const { return _substitutionLoc; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  LocCode& publishedLoc() { return _publishedLoc; }
  const LocCode& publishedLoc() const { return _publishedLoc; }

  bool operator==(const MileageSubstitution& rhs) const
  {
    return ((_substitutionLoc == rhs._substitutionLoc) && (_expireDate == rhs._expireDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_publishedLoc == rhs._publishedLoc));
  }

  static void dummyData(MileageSubstitution& obj)
  {
    obj._substitutionLoc = "aaaaaaaa";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._publishedLoc = "bbbbbbbb";
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  LocCode _substitutionLoc;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  LocCode _publishedLoc;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _substitutionLoc);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _publishedLoc);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_substitutionLoc
           & ptr->_expireDate
           & ptr->_createDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_publishedLoc;
  }
};
}

#endif
