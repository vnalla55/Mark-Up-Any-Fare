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

#ifndef MockLoc_H
#define MockLoc_H

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include "DBAccess/Loc.h"

namespace tse
{
class MockLoc : public Loc
{
public:
  MockLoc() : Loc() {}
  ~MockLoc() {}

  // Provide public setters for testing purposes.
  void setNation(NationCode nation) { _nation = nation; }
  void setState(StateCode state) { _state = state; }
  void setSubIATAArea(IATASubAreaCode subarea) { _subarea = subarea; }
  void setIATAArea(IATAAreaCode area) { _area = area; }
  void setLoc(LocCode loc) { _loc = loc; }
};
} // tse namespace

#endif // ifndef MockLoc_H
