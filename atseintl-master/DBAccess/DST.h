
// ---------------------------------------------------------------------------
// @ 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

#include <stdlib.h>

namespace tse
{
class DSTAdjMinutes
{
public:
  DSTAdjMinutes() : _dstStart(0), _dstFinish(0), _dstAdjmin(0) {}
  virtual ~DSTAdjMinutes() {}

  short getMinutesToAdjust(const DateTime& dt)
  {
    return (dt.isBetween(_dstStart, _dstFinish) ? _dstAdjmin : short(0));
  }

  virtual bool operator==(const DSTAdjMinutes& rhs) const
  {
    return ((_dstStart == rhs._dstStart) && (_dstFinish == rhs._dstFinish) &&
            (_dstAdjmin == rhs._dstAdjmin));
  }

  static void dummyData(DSTAdjMinutes& obj)
  {
    obj._dstStart = time(nullptr);
    obj._dstFinish = time(nullptr);
    obj._dstAdjmin = 1;
  }

  DateTime _dstStart;
  DateTime _dstFinish;
  short _dstAdjmin;

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _dstStart);
    FLATTENIZE(archive, _dstFinish);
    FLATTENIZE(archive, _dstAdjmin);
  }
};

class DST
{
public:
  DST() : _utcoffset(0) {}

  virtual ~DST()
  {
    std::vector<DSTAdjMinutes*>::iterator it = _dstAdjMinutes.begin();
    for (; it != _dstAdjMinutes.end(); ++it)
    {
      delete *it;
    }
  }

  short getMinutesToAdjust(const DateTime& dt) const
  {
    std::vector<DSTAdjMinutes*>::const_iterator cit = _dstAdjMinutes.begin();
    for (; cit != _dstAdjMinutes.end(); ++cit)
    {
      if ((*cit)->getMinutesToAdjust(dt))
        return (*cit)->getMinutesToAdjust(dt);
    }
    return 0;
  }

  int16_t utcoffset(const DateTime& dt) const
  {
    return static_cast<int16_t>(
        getMinutesToAdjust(DateTime(const_cast<DateTime&>(dt), TimeDuration(0, _utcoffset, 0))) +
        _utcoffset);
  }

  DSTGrpCode& dstgroup() { return _dstgroup; }
  const DSTGrpCode& dstgroup() const { return _dstgroup; }

  short& utcoffset() { return _utcoffset; }
  const short& utcoffset() const { return _utcoffset; }

  std::vector<DSTAdjMinutes*>& dstAdjMinutes() { return _dstAdjMinutes; }
  const std::vector<DSTAdjMinutes*>& dstAdjMinutes() const { return _dstAdjMinutes; }

  virtual bool operator==(const DST& rhs) const
  {
    bool eq = ((_dstgroup == rhs._dstgroup) && (_utcoffset == rhs._utcoffset) &&
               (_dstAdjMinutes.size() == rhs._dstAdjMinutes.size()));

    for (size_t i = 0; (eq && (i < _dstAdjMinutes.size())); ++i)
    {
      eq = (*(_dstAdjMinutes[i]) == *(rhs._dstAdjMinutes[i]));
    }

    return eq;
  }

  static void dummyData(DST& obj)
  {
    obj._dstgroup = "ABCD";
    obj._utcoffset = 1;

    DSTAdjMinutes* dam1 = new DSTAdjMinutes;
    DSTAdjMinutes* dam2 = new DSTAdjMinutes;

    DSTAdjMinutes::dummyData(*dam1);
    DSTAdjMinutes::dummyData(*dam2);

    obj._dstAdjMinutes.push_back(dam1);
    obj._dstAdjMinutes.push_back(dam2);
  }

protected:
  DSTGrpCode _dstgroup;
  short _utcoffset;
  std::vector<DSTAdjMinutes*> _dstAdjMinutes;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _dstgroup);
    FLATTENIZE(archive, _utcoffset);
    FLATTENIZE(archive, _dstAdjMinutes);
  }

protected:
private:
  DST(const DST&);
  DST& operator=(const DST&);
};

} // end tse namespace

