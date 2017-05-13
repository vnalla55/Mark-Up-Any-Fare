//----------------------------------------------------------------------------
//	   © 2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#ifndef ETICKET_CARRIER_INFO_H
#define ETICKET_CARRIER_INFO_H

#include "DBAccess/Flattenizable.h"

namespace tse
{

class ETicketCarrierInfo
{
public:
  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  std::vector<CarrierCode>& interlineCarriers() { return _interlineCarriers; }
  const std::vector<CarrierCode>& interlineCarriers() const { return _interlineCarriers; }

  bool operator==(const ETicketCarrierInfo& rhs) const
  {
    return ((_pseudoCity == rhs._pseudoCity) && (_carrier == rhs._carrier) &&
            (_interlineCarriers == rhs._interlineCarriers));
  }

  static void dummyData(ETicketCarrierInfo& obj)
  {
    obj._pseudoCity = "ABCDE";
    obj._carrier = "FGH";
    obj._interlineCarriers.push_back("IJK");
    obj._interlineCarriers.push_back("LMN");
  }

private:
  PseudoCityCode _pseudoCity;
  CarrierCode _carrier;
  std::vector<CarrierCode> _interlineCarriers;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _interlineCarriers);
  }

};
}
#endif
