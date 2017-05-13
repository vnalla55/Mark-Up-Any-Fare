//----------------------------------------------------------------------------
//   � 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class ContractPreference
{
public:
  ContractPreference() : _applyRoundingException(' ') {}

  ~ContractPreference() {}

  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  const RuleNumber& rule() const
  {
    return _rule;
  };
  RuleNumber& rule()
  {
    return _rule;
  };

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& applyRoundingException() { return _applyRoundingException; }
  const Indicator& applyRoundingException() const { return _applyRoundingException; }

  std::string& algorithmName() { return _algorithmName; }
  const std::string& algorithmName() const { return _algorithmName; }

  bool operator==(const ContractPreference& rhs) const
  {
    return ((_pseudoCity == rhs._pseudoCity) && (_carrier == rhs._carrier) &&
            (_rule == rhs._rule) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) &&
            (_applyRoundingException == rhs._applyRoundingException) &&
            (_algorithmName == rhs._algorithmName));
  }

  static void dummyData(ContractPreference& obj)
  {
    obj._pseudoCity = "ABCDE";
    obj._carrier = "FGH";
    obj._rule = "IJKL";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._applyRoundingException = 'M';
    obj._algorithmName = "aaaaaaaa";
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
  PseudoCityCode _pseudoCity;
  CarrierCode _carrier;
  RuleNumber _rule;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _applyRoundingException;
  std::string _algorithmName;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _applyRoundingException);
    FLATTENIZE(archive, _algorithmName);
  }

protected:
private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_pseudoCity
           & ptr->_carrier
           & ptr->_rule
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_applyRoundingException
           & ptr->_algorithmName;
  }

  ContractPreference(const ContractPreference&);
  ContractPreference& operator=(const ContractPreference&);
};

}// tse

