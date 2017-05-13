//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/CabinType.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/FlexFares/Types.h"

#include <set>
#include <tuple>

namespace tse
{
namespace flexFares
{

class ValidationStatus
{
private:
  typedef std::tuple<std::set<std::string>, // valid corpIds
                     std::set<std::string>, // valid accCodes
                     bool, // is fare public?
                     bool, // is fare private?
                     PaxTypeCode, // passenger type
                     Record3ReturnTypes, // is valid for no adv purchase?
                     Record3ReturnTypes, // is valid for no penalties
                     Record3ReturnTypes> // is valid for no min/max stay
      VStatus;

  VStatus _status;

public:
  ValidationStatus()
    : _status(std::set<std::string>(),
              std::set<std::string>(),
              false,
              false,
              "",
              tse::SOFTPASS,
              tse::SOFTPASS,
              tse::SOFTPASS)
  {
  }

  template <Attribute attribute>
  struct ConstVStatus
  {
    using Ref = const typename std::tuple_element<attribute, VStatus>::type&;
  };

  template <Attribute attribute>
  typename ConstVStatus<attribute>::Ref getStatusForAttribute() const
  {
    return std::get<attribute>(_status);
  }

  // WARNING: this method should not be used for CORP_IDS and ACC_CODES
  // see specializations in cpp file
  template <Attribute attribute>
  void setStatus(typename ConstVStatus<attribute>::Ref valid)
  {
    std::get<attribute>(_status) = valid;
  }

  void setValidCorpId(const std::string& corpId) { std::get<CORP_IDS>(_status).insert(corpId); }
  void setValidAccCode(const std::string& accCode) { std::get<ACC_CODES>(_status).insert(accCode); }

  template <Attribute attribute>
  void updateAttribute(const ValidationStatusPtr& source)
  {
    std::get<attribute>(_status) = std::get<attribute>(source->_status);
  }

private:
  template <Attribute attribute>
  void updateValidationResult(const Record3ReturnTypes& result)
  {
    if (getStatusForAttribute<attribute>() == tse::FAIL)
      return;

    setStatus<attribute>(result);
  }

  void updateCorpId(const std::set<std::string>& source)
  {
    for (const std::string& key : source)
      setValidCorpId(key);
  }

  void updateAccCode(const std::set<std::string>& source)
  {
    for (const std::string& key : source)
      setValidAccCode(key);
  }
};

} // flexFares
} // tse

