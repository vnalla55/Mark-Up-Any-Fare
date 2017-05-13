//----------------------------------------------------------------------------
//   2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class BasicBookingRequest
{
public:
  BasicBookingRequest() {}

  ~BasicBookingRequest() {}

  std::string& ownerId() { return _ownerId; }
  const std::string& ownerId() const { return _ownerId; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  BasicBookingRequest* newDuplicate()
  {
    // Warning!  Caller is responsible for deleting this when finished!
    BasicBookingRequest* retval = new BasicBookingRequest;

    retval->_ownerId = _ownerId;
    retval->_carrier = _carrier;
    retval->_effDate = _effDate;
    retval->_discDate = _discDate;

    return retval;
  }

  bool operator==(const BasicBookingRequest& rhs) const
  {
    return ((_ownerId == rhs._ownerId) && (_carrier == rhs._carrier) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate));
  }

  static void dummyData(BasicBookingRequest& obj)
  {
    obj._ownerId = "OWNERID";
    obj._carrier = "ABC";
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
  }

protected:
  std::string _ownerId;
  CarrierCode _carrier;
  DateTime _effDate;
  DateTime _discDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _ownerId);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
  }

protected:
private:
  BasicBookingRequest(const BasicBookingRequest&);
  BasicBookingRequest& operator=(const BasicBookingRequest&);
};
}

