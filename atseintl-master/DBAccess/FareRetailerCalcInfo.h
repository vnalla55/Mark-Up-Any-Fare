#pragma once

#include "DBAccess/FareRetailerCalcDetailInfo.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareRetailerCalcInfo
{
 public:

  FareRetailerCalcInfo()
    : _fareRetailerCalcItemNo(0)
  {
  }

  ~FareRetailerCalcInfo()
  {
    for (std::vector<FareRetailerCalcDetailInfo*>::const_iterator it(_details.begin()),
                                                                   itend(_details.end());
         it != itend;
         ++it)
    {
      delete *it;
    }
  }

  uint64_t& fareRetailerCalcItemNo() { return _fareRetailerCalcItemNo; }
  uint64_t fareRetailerCalcItemNo() const { return _fareRetailerCalcItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<FareRetailerCalcDetailInfo*>& details() { return _details; }
  const std::vector<FareRetailerCalcDetailInfo*>& details() const { return _details; }

  bool operator==(const FareRetailerCalcInfo& rhs) const
  {
    bool result(_fareRetailerCalcItemNo == rhs._fareRetailerCalcItemNo
                && _createDate == rhs._createDate
                && _expireDate == rhs._expireDate
                && _details.size() == rhs._details.size());
    if (result)
    {
      for (size_t i = 0; i <  _details.size(); ++i)
      {
        result = result && *_details[i] == *rhs._details[i];
        if (!result)
        {
          break;
        }
      }
    }
    return result;
  }

  static void dummyData(FareRetailerCalcInfo& obj)
  {
    obj._fareRetailerCalcItemNo = 111111;
    obj._createDate = ::time(nullptr);
    obj._expireDate = ::time(nullptr);
    FareRetailerCalcDetailInfo* detail(new FareRetailerCalcDetailInfo);
    FareRetailerCalcDetailInfo::dummyData(*detail);
    obj._details.push_back(detail);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _fareRetailerCalcItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _details);
  }

 private:

  uint64_t _fareRetailerCalcItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<FareRetailerCalcDetailInfo*> _details;

};

}// tse




