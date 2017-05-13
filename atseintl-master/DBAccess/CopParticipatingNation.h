//----------------------------------------------------------------------------
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef COPPARTICIPATINGNATION_H
#define COPPARTICIPATINGNATION_H

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class CopParticipatingNation
{
public:
  virtual ~CopParticipatingNation() {}

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  NationCode& copNation() { return _copNation; }
  const NationCode& copNation() const { return _copNation; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  virtual bool operator==(const CopParticipatingNation& rhs) const
  {
    return ((_nation == rhs._nation) && (_copNation == rhs._copNation) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate));
  }

  static void dummyData(CopParticipatingNation& obj)
  {
    obj._nation = "ABC";
    obj._copNation = "DEF";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
  }

private:
  NationCode _nation;
  NationCode _copNation;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _copNation);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
  }
};
}
#endif
