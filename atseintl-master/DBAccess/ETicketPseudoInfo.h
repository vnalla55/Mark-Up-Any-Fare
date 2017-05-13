//----------------------------------------------------------------------------
//	   © 2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#ifndef ETICKET_PSEUDO_INFO_H
#define ETICKET_PSEUDO_INFO_H

#include "DBAccess/Flattenizable.h"

namespace tse
{

class ETicketPseudoInfo
{
public:
  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  CarrierCode& interlineCarrier() { return _interlineCarrier; }
  const CarrierCode& interlineCarrier() const { return _interlineCarrier; }

  CarrierCode& eTicketCarrier() { return _eTicketCarrier; }
  const CarrierCode& eTicketCarrier() const { return _eTicketCarrier; }

  bool operator==(const ETicketPseudoInfo& rhs) const
  {
    return ((_pseudoCity == rhs._pseudoCity) && (_interlineCarrier == rhs._interlineCarrier) &&
            (_eTicketCarrier == rhs._eTicketCarrier));
  }

  static void dummyData(ETicketPseudoInfo& obj)
  {
    obj._pseudoCity = "ABCDE";
    obj._interlineCarrier = "FGH";
    obj._eTicketCarrier = "IJK";
  }

private:
  PseudoCityCode _pseudoCity;
  CarrierCode _interlineCarrier;
  CarrierCode _eTicketCarrier;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _interlineCarrier);
    FLATTENIZE(archive, _eTicketCarrier);
  }

};
}
#endif
