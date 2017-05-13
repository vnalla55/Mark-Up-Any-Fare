//----------------------------------------------------------------------------
//  File: FDAddOnFareInfo.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/AddonFareInfoFactory.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/SITAAddonFareInfo.h"

namespace tse
{


class FDAddOnFareInfo : public SITAAddonFareInfo
{
public:
  // construction / destruction
  // ============ = ===========

  FDAddOnFareInfo() : SITAAddonFareInfo() {};
  virtual ~FDAddOnFareInfo() {};
  const eAddonFareInfoType objectType() const override { return eFDAddonFareInfo; }

  std::string& addOnRoutingSeq() { return _addOnRoutingSeq; }
  const std::string& addOnRoutingSeq() const { return _addOnRoutingSeq; }

  virtual bool operator==(const FDAddOnFareInfo& rhs) const
  {
    return ((SITAAddonFareInfo::operator==(rhs)) && (_addOnRoutingSeq == rhs._addOnRoutingSeq));
  }

  static void dummyData(FDAddOnFareInfo& obj)
  {
    SITAAddonFareInfo::dummyData(obj);
    obj._addOnRoutingSeq = "aaaaaaaa";
  }

  WBuffer& write(WBuffer& os, size_t* memSize) const override
  {
    os.write(static_cast<boost::uint8_t>(eFDAddonFareInfo));
    if (memSize)
    {
      *memSize += sizeof(FDAddOnFareInfo);
    }
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is) override { return convert(is, this); }

private:
  std::string _addOnRoutingSeq;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, SITAAddonFareInfo);
    FLATTENIZE(archive, _addOnRoutingSeq);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return SITAAddonFareInfo::convert(buffer, ptr) & ptr->_addOnRoutingSeq;
  }

}; // class FDAddOnFareInfo

} // namespace tse
