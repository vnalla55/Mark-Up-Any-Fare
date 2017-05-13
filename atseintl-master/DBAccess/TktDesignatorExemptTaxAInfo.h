#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class TktDesignatorExemptTaxAInfo
{
public:
  TktDesignatorExemptTaxAInfo() {}

  const TaxCode& taxCode() const { return _taxCode; }
  TaxCode& taxCode() { return _taxCode; }
  const NationCode& taxNation() const { return _taxNation; }
  NationCode& taxNation() { return _taxNation; }

  bool operator==(const TktDesignatorExemptTaxAInfo& rhs) const
  {
    return _taxCode == rhs._taxCode && _taxNation == rhs._taxNation;
  }

  static void dummyData(TktDesignatorExemptTaxAInfo& obj)
  {
    obj._taxCode = "US1";
    obj._taxNation = "US";
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _taxCode);
    FLATTENIZE(archive, _taxNation);
  }

private:
  TaxCode _taxCode;
  NationCode _taxNation;
};

} // tse

