// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class TaxOrderTktIssue
{

public:
  TaxOrderTktIssue() {}
  TaxOrderTktIssue(const TaxCode& taxCode, const TaxType& taxType)
      : _taxCode(taxCode),
        _taxType(taxType) {}

  bool operator==(const TaxOrderTktIssue& rhs) const
  {
    return _taxCode == rhs._taxCode && _taxType == rhs._taxType;
  }

  const TaxCode& taxCode() const { return _taxCode; }
  TaxCode& taxCode() { return _taxCode; }

  const TaxType& taxType() const { return _taxType; }
  TaxType& taxType() { return _taxType; }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _taxCode);
    FLATTENIZE(archive, _taxType);
  }

  static void dummyData(TaxOrderTktIssue& obj)
  {
    obj._taxCode = "ASD";
    obj._taxType = "001";
  }

private:
  template<typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_taxCode & ptr->_taxType;
  }

  TaxCode _taxCode;
  TaxType _taxType;
};

template<> struct cdu_pod_traits<TaxOrderTktIssue>: std::true_type{};

} /* namespace tse */

