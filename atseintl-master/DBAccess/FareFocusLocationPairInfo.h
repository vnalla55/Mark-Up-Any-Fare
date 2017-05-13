#pragma once

#include "DBAccess/FareFocusLocationPairDetailInfo.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{


class FareFocusLocationPairInfo
{
 public:
  FareFocusLocationPairInfo()
    : _locationPairItemNo(0)
  {
  }

  ~FareFocusLocationPairInfo()
  {
    for (std::vector<FareFocusLocationPairDetailInfo*>::const_iterator it(_details.begin()),
                                                                   itend(_details.end());
         it != itend;
         ++it)
    {
      delete *it;
    }
  }

  uint64_t& locationPairItemNo() { return _locationPairItemNo; }
  uint64_t locationPairItemNo() const { return _locationPairItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<FareFocusLocationPairDetailInfo*>& details() { return _details; }
  const std::vector<FareFocusLocationPairDetailInfo*>& details() const { return _details; }

  bool operator==(const FareFocusLocationPairInfo& rhs) const
  {
    bool result(_locationPairItemNo == rhs._locationPairItemNo
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

  static void dummyData(FareFocusLocationPairInfo& obj)
  {
    static DateTime current(::time(nullptr));
    obj._locationPairItemNo = 111222;
    obj._createDate = current;
    obj._expireDate = current;
    FareFocusLocationPairDetailInfo* detail(new FareFocusLocationPairDetailInfo);
    FareFocusLocationPairDetailInfo::dummyData(*detail);
    obj._details.push_back(detail);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _locationPairItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _details);
  }

 private:
  uint64_t _locationPairItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<FareFocusLocationPairDetailInfo*> _details;
};

}// tse

