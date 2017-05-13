#pragma once

#include "DBAccess/FareFocusSecurityDetailInfo.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

// extract SECURITYITEMNO for matching FareFocusRule
// SECURITYITEMNO match FAREFOCUSRULE.SECURITYITEMNO
// PSEUDOCITY - AGENT's PCC
// PSEUDOCITYTYPE = ['T' (branch) 'H' (home)]
// Is PCC allowed for the rule

class FareFocusSecurityInfo
{
 public:
  FareFocusSecurityInfo()
    : _securityItemNo(0)
  {
  }

  ~FareFocusSecurityInfo()
  {
    for (std::vector<FareFocusSecurityDetailInfo*>::const_iterator it(_details.begin()),
                                                                   itend(_details.end());
         it != itend;
         ++it)
    {
      delete *it;
    }
  }

  uint64_t& securityItemNo() { return _securityItemNo; }
  uint64_t securityItemNo() const { return _securityItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<FareFocusSecurityDetailInfo*>& details() { return _details; }
  const std::vector<FareFocusSecurityDetailInfo*>& details() const { return _details; }

  bool operator==(const FareFocusSecurityInfo& rhs) const
  {
    bool result(_securityItemNo == rhs._securityItemNo
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

  static void dummyData(FareFocusSecurityInfo& obj)
  {
    static DateTime current(::time(nullptr));
    obj._securityItemNo = 11111111;
    obj._createDate = current;
    obj._expireDate = current;
    FareFocusSecurityDetailInfo* detail(new FareFocusSecurityDetailInfo);
    FareFocusSecurityDetailInfo::dummyData(*detail);
    obj._details.push_back(detail);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _securityItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _details);
  }

 private:
  uint64_t _securityItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<FareFocusSecurityDetailInfo*> _details;
};

}// tse

