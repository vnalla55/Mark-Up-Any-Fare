//------------------------------------------------------------------------------
// � 2004, Sabre Inc.  All rights reserved.  This software/documentation is
// the confidential and proprietary product of Sabre Inc. Any unauthorized
// use, reproduction, or transfer of this software/documentation, in any
// medium, or incorporation of this software/documentation into any system or
// publication, is strictly prohibited
//
//------------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class TaxRestrictionTktDesignator
{
public:
  std::string& tktDesignator() { return _tktDesignator; }
  const std::string& tktDesignator() const { return _tktDesignator; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  bool operator==(const TaxRestrictionTktDesignator& rhs) const
  {
    return ((_tktDesignator == rhs._tktDesignator) && (_carrier == rhs._carrier));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TaxRestrictionTktDesignator& obj)
  {
    obj._tktDesignator = "aaaaaaaa";
    obj._carrier = "ABC";
  }

private:
  std::string _tktDesignator;
  CarrierCode _carrier;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _tktDesignator);
    FLATTENIZE(archive, _carrier);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_tktDesignator & ptr->_carrier;
  }
};
}
