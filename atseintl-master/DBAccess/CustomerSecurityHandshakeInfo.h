#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/TSEDateInterval.h"

namespace tse
{

// client code comes with three parameters : ticketing date, SECURITYSOURCEPCC, and PRODUCTCD
// result - vector of securityTargetPCCs

class CustomerSecurityHandshakeInfo
{
public:

  CustomerSecurityHandshakeInfo()
  {
  }

  PseudoCityCode& securitySourcePCC() { return  _securitySourcePCC; }
  const PseudoCityCode& securitySourcePCC() const { return  _securitySourcePCC; }

  TSEDateInterval& effInterval() { return _effInterval; }
  const TSEDateInterval& effInterval() const { return _effInterval; }

  DateTime& createDate() { return _effInterval.createDate(); }
  const DateTime& createDate() const { return _effInterval.createDate(); }

  DateTime& effDate() { return _effInterval.effDate(); }
  const DateTime& effDate() const { return _effInterval.effDate(); }

  DateTime& expireDate() { return _effInterval.expireDate(); }
  const DateTime& expireDate() const { return _effInterval.expireDate(); }

  DateTime& discDate() { return _effInterval.discDate(); }
  const DateTime& discDate() const { return _effInterval.discDate(); }

  Code<8>& productCode() { return _productCode; }
  const Code<8>& productCode() const { return _productCode; }

  PseudoCityCode& securityTargetPCC() { return _securityTargetPCC; }
  const PseudoCityCode& securityTargetPCC() const { return _securityTargetPCC; }

  PseudoCityCode& homePCC() { return  _homePCC; }
  const PseudoCityCode& homePCC() const { return  _homePCC; }

  bool operator ==(const CustomerSecurityHandshakeInfo& rhs) const
  {
    return _securitySourcePCC == rhs._securitySourcePCC
           && _effInterval == rhs._effInterval
           && _productCode == rhs._productCode
           && _securityTargetPCC == rhs._securityTargetPCC
           && _homePCC == rhs._homePCC;
  }

  static void dummyData(CustomerSecurityHandshakeInfo& obj)
  {
    obj._securitySourcePCC = "ABCDE";
    TSEDateInterval::dummyData(obj._effInterval);
    obj._productCode = "FF";
    obj._securityTargetPCC = "AMEX";
    obj._homePCC = "FGHIJ";
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _securitySourcePCC);
    FLATTENIZE(archive, _effInterval);
    FLATTENIZE(archive, _productCode);
    FLATTENIZE(archive, _securityTargetPCC);
    FLATTENIZE(archive, _homePCC);
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_securitySourcePCC
           & ptr->_effInterval
           & ptr->_productCode
           & ptr->_securityTargetPCC
           & ptr->_homePCC;
  }
  PseudoCityCode _securitySourcePCC;// agent - branch, under HOMEPCC (customer)
  TSEDateInterval _effInterval;
  Code<8> _productCode;// 'FF'
  PseudoCityCode _securityTargetPCC;// rule creator, above homePCC (AMEX)
  PseudoCityCode _homePCC;// above securitySourcePCC, not used
};

}// tse

