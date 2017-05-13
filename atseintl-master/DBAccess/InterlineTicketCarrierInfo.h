//----------------------------------------------------------------------------
//          File:           InterlineTicketCarrierInfo.h
//          Description:    InterlineTicketCarrierInfo
//          Created:        10/1/2010
//          Authors:        Anna Kulig
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class InterlineTicketCarrierInfo
{
public:
  InterlineTicketCarrierInfo()
    : _hostInterline(' '), _pseudoInterline(' '), _superPseudoInterline(' '), _emdInterline(' ')
  {
  }

  virtual ~InterlineTicketCarrierInfo() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& lastModDate() { return _lastModDate; }
  const DateTime& lastModDate() const { return _lastModDate; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  CarrierCode& interlineCarrier() { return _interlineCarrier; }
  const CarrierCode& interlineCarrier() const { return _interlineCarrier; }

  Indicator& hostInterline() { return _hostInterline; }
  const Indicator& hostInterline() const { return _hostInterline; }

  Indicator& pseudoInterline() { return _pseudoInterline; }
  const Indicator& pseudoInterline() const { return _pseudoInterline; }

  Indicator& superPseudoInterline() { return _superPseudoInterline; }
  const Indicator& superPseudoInterline() const { return _superPseudoInterline; }

  Indicator& emdInterline() { return _emdInterline; }
  const Indicator& emdInterline() const { return _emdInterline; }

  virtual bool operator==(const InterlineTicketCarrierInfo& rhs) const
  {
    return ((_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_lastModDate == rhs._lastModDate) && (_carrier == rhs._carrier) &&
            (_interlineCarrier == rhs._interlineCarrier) &&
            (_hostInterline == rhs._hostInterline) && (_pseudoInterline == rhs._pseudoInterline) &&
            (_superPseudoInterline == rhs._superPseudoInterline) &&
            (_emdInterline == rhs._emdInterline));
  }

  static void dummyData(InterlineTicketCarrierInfo& obj)
  {
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._lastModDate = time(nullptr);
    obj._carrier = "ABC";
    obj._interlineCarrier = "DEF";
    obj._hostInterline = 'N';
    obj._pseudoInterline = 'N';
    obj._superPseudoInterline = 'N';
    obj._emdInterline = 'N';
  }

private:
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _lastModDate;
  CarrierCode _carrier;
  CarrierCode _interlineCarrier;
  Indicator _hostInterline;
  Indicator _pseudoInterline;
  Indicator _superPseudoInterline;
  Indicator _emdInterline;

public:
  virtual void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _lastModDate);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _interlineCarrier);
    FLATTENIZE(archive, _hostInterline);
    FLATTENIZE(archive, _pseudoInterline);
    FLATTENIZE(archive, _superPseudoInterline);
    FLATTENIZE(archive, _emdInterline);
  }

};
}
