#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{
class BrandedFareSeg
{
public:
  BrandedFareSeg() : _tier(9999), _svcFeesFareIdTblItemNo(-1), _taxTextTblItemNo(-1), _segNo(-1) {}
  int& tier() { return _tier; }
  int tier() const { return _tier; }
  std::string& brandName() { return _brandName; }
  const std::string& brandName() const { return _brandName; }
  long& svcFeesFareIdTblItemNo() { return _svcFeesFareIdTblItemNo; }
  long svcFeesFareIdTblItemNo() const { return _svcFeesFareIdTblItemNo; }
  long& taxTextTblItemNo() { return _taxTextTblItemNo; }
  long taxTextTblItemNo() const { return _taxTextTblItemNo; }
  int& segNo() { return _segNo; }
  int segNo() const { return _segNo; }

  bool operator==(const BrandedFareSeg& rhs) const
  {
    return _tier == rhs._tier && _brandName == rhs._brandName &&
           _svcFeesFareIdTblItemNo == rhs._svcFeesFareIdTblItemNo &&
           _taxTextTblItemNo == rhs._taxTextTblItemNo && _segNo == rhs._segNo;
  }

  static void dummyData(BrandedFareSeg& obj)
  {
    obj._tier = 5;
    obj._brandName = "brandName";
    obj._svcFeesFareIdTblItemNo = 22222;
    obj._taxTextTblItemNo = 33333;
    obj._segNo = 1;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _tier);
    FLATTENIZE(archive, _brandName);
    FLATTENIZE(archive, _svcFeesFareIdTblItemNo);
    FLATTENIZE(archive, _taxTextTblItemNo);
    FLATTENIZE(archive, _segNo);
  }

private:
  int _tier; // rank, 1 to 9999
  std::string _brandName;
  long _svcFeesFareIdTblItemNo;
  long _taxTextTblItemNo;
  int _segNo;
};
} // tse

