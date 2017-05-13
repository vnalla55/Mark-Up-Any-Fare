//-------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class DBEGlobalClass
{
public:
  DBEClass& dbeGlobalClass() { return _dbeGlobalClass; }
  const DBEClass& dbeGlobalClass() const { return _dbeGlobalClass; }

  std::vector<DBEClass>& dbeClasses() { return _dbeClasses; }
  const std::vector<DBEClass>& dbeClasses() const { return _dbeClasses; }

  bool operator==(const DBEGlobalClass& rhs) const
  {
    return ((_dbeGlobalClass == rhs._dbeGlobalClass) && (_dbeClasses == rhs._dbeClasses));
  }

  static void dummyData(DBEGlobalClass& obj)
  {
    obj._dbeGlobalClass = "ABC";
    obj._dbeClasses.push_back("DEF");
    obj._dbeClasses.push_back("GHI");
  }

private:
  DBEClass _dbeGlobalClass;
  std::vector<DBEClass> _dbeClasses;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _dbeGlobalClass);
    FLATTENIZE(archive, _dbeClasses);
  }
};

} // End namespace tse
