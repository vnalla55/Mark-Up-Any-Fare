//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class MerchActivationInfo
{
public:
  MerchActivationInfo()
    : _activationId(0),
      _productId(0),
      _userApplType(' '),
      _displayOnly(' '),
      _includeInd(' '),
      _memoNo(0)
  {
  }

  uint32_t& activationId() { return _activationId; }
  const uint32_t& activationId() const { return _activationId; }

  uint32_t& productId() { return _productId; }
  const uint32_t& productId() const { return _productId; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  ServiceGroup& groupCode() { return _groupCode; }
  const ServiceGroup& groupCode() const { return _groupCode; }

  ServiceGroup& subgroupCode() { return _subgroupCode; }
  const ServiceGroup& subgroupCode() const { return _subgroupCode; }

  ServiceSubTypeCode& subCode() { return _subCode; }
  const ServiceSubTypeCode& subCode() const { return _subCode; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& displayOnly() { return _displayOnly; }
  const Indicator& displayOnly() const { return _displayOnly; }

  Indicator& includeInd() { return _includeInd; }
  const Indicator& includeInd() const { return _includeInd; }

  uint32_t& memoNo() { return _memoNo; }
  const uint32_t& memoNo() const { return _memoNo; }

  bool operator==(const MerchActivationInfo& second) const
  {
    return (_activationId == second._activationId) && (_productId == second._productId) &&
           (_carrier == second._carrier) && (_userApplType == second._userApplType) &&
           (_userAppl == second._userAppl) && (_pseudoCity == second._pseudoCity) &&
           (_groupCode == second._groupCode) && (_subgroupCode == second._subgroupCode) &&
           (_subCode == second._subCode) && (_createDate == second._createDate) &&
           (_expireDate == second._expireDate) && (_effDate == second._effDate) &&
           (_discDate == second._discDate) && (_displayOnly == second._displayOnly) &&
           (_includeInd == second._includeInd) && (_memoNo == second._memoNo);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _activationId);
    FLATTENIZE(archive, _productId);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _groupCode);
    FLATTENIZE(archive, _subgroupCode);
    FLATTENIZE(archive, _subCode);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _displayOnly);
    FLATTENIZE(archive, _includeInd);
    FLATTENIZE(archive, _memoNo);
  }

  static void dummyData(MerchActivationInfo& obj)
  {
    obj._activationId = 1;
    obj._productId = 2;
    obj._carrier = "AAA";
    obj._userApplType = 'B';
    obj._userAppl = "CCCC";
    obj._pseudoCity = "DDDDD";
    obj._groupCode = "EEE";
    obj._subgroupCode = "FFF";
    obj._subCode = "GGG";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._displayOnly = 'H';
    obj._includeInd = 'I';
    obj._memoNo = 3;
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
  uint32_t _activationId;
  uint32_t _productId;
  CarrierCode _carrier;
  Indicator _userApplType;
  UserApplCode _userAppl;
  PseudoCityCode _pseudoCity;
  ServiceGroup _groupCode;
  ServiceGroup _subgroupCode;
  ServiceSubTypeCode _subCode;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _displayOnly;
  Indicator _includeInd;
  uint32_t _memoNo;

  friend class SerializationTestBase;
private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_activationId
           & ptr->_productId
           & ptr->_carrier
           & ptr->_userApplType
           & ptr->_userAppl
           & ptr->_pseudoCity
           & ptr->_groupCode
           & ptr->_subgroupCode
           & ptr->_subCode
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_displayOnly
           & ptr->_includeInd
           & ptr->_memoNo;
  }
};
}

