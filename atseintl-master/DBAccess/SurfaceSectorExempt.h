//----------------------------------------------------------------------------
//	   © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#ifndef SURFACESECTOREXEMPT_H
#define SURFACESECTOREXEMPT_H

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class SurfaceSectorExempt
{
public:
  LocCode& origLoc() { return _origLoc; }
  const LocCode& origLoc() const { return _origLoc; }

  LocCode& destLoc() { return _destLoc; }
  const LocCode& destLoc() const { return _destLoc; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  bool operator==(const SurfaceSectorExempt& rhs) const
  {
    return ((_origLoc == rhs._origLoc) && (_destLoc == rhs._destLoc) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate));
  }

  static void dummyData(SurfaceSectorExempt& obj)
  {
    obj._origLoc = "aaaaaaaa";
    obj._destLoc = "bbbbbbbb";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
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
  LocCode _origLoc;
  LocCode _destLoc;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _origLoc);
    FLATTENIZE(archive, _destLoc);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_origLoc
           & ptr->_destLoc
           & ptr->_expireDate
           & ptr->_createDate
           & ptr->_effDate
           & ptr->_discDate;
  }
};
}

#endif
