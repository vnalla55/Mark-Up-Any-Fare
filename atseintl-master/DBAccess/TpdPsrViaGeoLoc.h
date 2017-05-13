//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class TpdPsrViaGeoLoc
{
  // friend class TicketedPointDeduction;      ////////////////TEST ONLY ////////////////////
public:
  TpdPsrViaGeoLoc()
    : _setNo(0),
      _orderNo(0),
      _relationalInd(' '),
      _stopoverNotAllowed(' '),
      _noStopBtwViaAndLoc1(' '),
      _reqDirectSvcBtwViaAndLoc1(' '),
      _noStopBtwViaAndLoc2(' '),
      _reqDirectSvcBtwViaAndLoc2(' ')
  {
  }

  int& setNo() { return _setNo; }
  const int& setNo() const { return _setNo; }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  LocKey& loc() { return _loc; }
  const LocKey& loc() const { return _loc; }

  Indicator& relationalInd() { return _relationalInd; }
  const Indicator& relationalInd() const { return _relationalInd; }

  Indicator& stopoverNotAllowed() { return _stopoverNotAllowed; }
  const Indicator& stopoverNotAllowed() const { return _stopoverNotAllowed; }

  Indicator& noStopBtwViaAndLoc1() { return _noStopBtwViaAndLoc1; }
  const Indicator& noStopBtwViaAndLoc1() const { return _noStopBtwViaAndLoc1; }

  Indicator& reqDirectSvcBtwViaAndLoc1() { return _reqDirectSvcBtwViaAndLoc1; }
  const Indicator& reqDirectSvcBtwViaAndLoc1() const { return _reqDirectSvcBtwViaAndLoc1; }

  Indicator& noStopBtwViaAndLoc2() { return _noStopBtwViaAndLoc2; }
  const Indicator& noStopBtwViaAndLoc2() const { return _noStopBtwViaAndLoc2; }

  Indicator& reqDirectSvcBtwViaAndLoc2() { return _reqDirectSvcBtwViaAndLoc2; }
  const Indicator& reqDirectSvcBtwViaAndLoc2() const { return _reqDirectSvcBtwViaAndLoc2; }

  bool operator==(const TpdPsrViaGeoLoc& rhs) const
  {
    return ((_setNo == rhs._setNo) && (_orderNo == rhs._orderNo) && (_loc == rhs._loc) &&
            (_relationalInd == rhs._relationalInd) &&
            (_stopoverNotAllowed == rhs._stopoverNotAllowed) &&
            (_noStopBtwViaAndLoc1 == rhs._noStopBtwViaAndLoc1) &&
            (_reqDirectSvcBtwViaAndLoc1 == rhs._reqDirectSvcBtwViaAndLoc1) &&
            (_noStopBtwViaAndLoc2 == rhs._noStopBtwViaAndLoc2) &&
            (_reqDirectSvcBtwViaAndLoc2 == rhs._reqDirectSvcBtwViaAndLoc2));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TpdPsrViaGeoLoc& obj)
  {
    obj._setNo = 1;
    obj._orderNo = 2;
    LocKey::dummyData(obj._loc);
    obj._relationalInd = 'A';
    obj._stopoverNotAllowed = 'B';
    obj._noStopBtwViaAndLoc1 = 'B';
    obj._reqDirectSvcBtwViaAndLoc1 = 'B';
    obj._noStopBtwViaAndLoc2 = 'B';
    obj._reqDirectSvcBtwViaAndLoc2 = 'B';
  }

private:
  int _setNo;
  int _orderNo;
  LocKey _loc;
  Indicator _relationalInd;
  Indicator _stopoverNotAllowed;
  Indicator _noStopBtwViaAndLoc1;
  Indicator _reqDirectSvcBtwViaAndLoc1;
  Indicator _noStopBtwViaAndLoc2;
  Indicator _reqDirectSvcBtwViaAndLoc2;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_setNo & ptr->_orderNo & ptr->_loc & ptr->_relationalInd &
           ptr->_stopoverNotAllowed & ptr->_noStopBtwViaAndLoc1 & ptr->_reqDirectSvcBtwViaAndLoc1 &
           ptr->_noStopBtwViaAndLoc2 & ptr->_reqDirectSvcBtwViaAndLoc2;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _setNo);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _loc);
    FLATTENIZE(archive, _relationalInd);
    FLATTENIZE(archive, _stopoverNotAllowed);
    FLATTENIZE(archive, _noStopBtwViaAndLoc1);
    FLATTENIZE(archive, _reqDirectSvcBtwViaAndLoc1);
    FLATTENIZE(archive, _noStopBtwViaAndLoc2);
    FLATTENIZE(archive, _reqDirectSvcBtwViaAndLoc2);
  }
};
}

