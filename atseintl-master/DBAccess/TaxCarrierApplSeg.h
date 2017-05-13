// ----------------------------------------------------------------------------
// © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the
// confidential and proprietary product of Sabre Inc. Any unauthorized use,
// reproduction, or transfer of this software/documentation, in any medium, or
// incorporation of this
// software/documentation into any system or publication, is strictly prohibited
// ----------------------------------------------------------------------------

#ifndef TAXCARRIERAPPLSEG_H
#define TAXCARRIERAPPLSEG_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class TaxCarrierApplSeg
{
public:
  TaxCarrierApplSeg() : _applInd(' ') {}
  Indicator& applInd() { return _applInd; }
  const Indicator& applInd() const { return _applInd; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  bool operator==(const TaxCarrierApplSeg& rhs) const
  {
    return ((_applInd == rhs._applInd) && (_carrier == rhs._carrier));
  }

  static void dummyData(TaxCarrierApplSeg& obj)
  {
    obj._applInd = 'A';
    obj._carrier = "BCD";
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
  Indicator _applInd;
  CarrierCode _carrier;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _applInd);
    FLATTENIZE(archive, _carrier);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_applInd
           & ptr->_carrier;
  }
};
}
#endif
