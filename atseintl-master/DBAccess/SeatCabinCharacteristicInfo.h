//----------------------------------------------------------------------------
//       (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class SeatCabinCharacteristicInfo
{
public:
  SeatCabinCharacteristicInfo() = default;

  virtual ~SeatCabinCharacteristicInfo() = default;

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Indicator& codeType() { return _codeType; }
  const Indicator& codeType() const { return _codeType; }

  SeatCabinCode& seatCabinCode() { return _seatCabinCode; }
  const SeatCabinCode& seatCabinCode() const { return _seatCabinCode; }

  std::string& codeDescription() { return _codeDescription; }
  const std::string& codeDescription() const { return _codeDescription; }

  std::string& displayDescription() { return _displayDescription; }
  const std::string& displayDescription() const { return _displayDescription; }

  std::string& abbreviatedDescription() { return _abbreviatedDescription; }
  const std::string& abbreviatedDescription() const { return _abbreviatedDescription; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime expireDate() const { return _expireDate; }

  virtual bool operator==(const SeatCabinCharacteristicInfo& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_codeType == rhs._codeType) &&
            (_seatCabinCode == rhs._seatCabinCode) && (_codeDescription == rhs._codeDescription) &&
            (_displayDescription == rhs._displayDescription) &&
            (_abbreviatedDescription == rhs._abbreviatedDescription) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate));
  }

  static void dummyData(SeatCabinCharacteristicInfo& obj)
  {
    obj._carrier = "ABC";
    obj._codeType = 'A';
    obj._seatCabinCode = "AB";
    obj._codeDescription = "CodeDescription";
    obj._displayDescription = "DisplayDescription";
    obj._abbreviatedDescription = "AbbreviatedDescription";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
  }

protected:
  CarrierCode _carrier;
  Indicator _codeType = 0;
  SeatCabinCode _seatCabinCode;
  std::string _codeDescription;
  std::string _displayDescription;
  std::string _abbreviatedDescription;
  DateTime _createDate;
  DateTime _expireDate;

public:
  virtual void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _codeType);
    FLATTENIZE(archive, _seatCabinCode);
    FLATTENIZE(archive, _codeDescription);
    FLATTENIZE(archive, _displayDescription);
    FLATTENIZE(archive, _abbreviatedDescription);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_carrier & ptr->_codeType & ptr->_seatCabinCode & ptr->_codeDescription &
           ptr->_displayDescription & ptr->_abbreviatedDescription & ptr->_createDate &
           ptr->_expireDate;
  }

  SeatCabinCharacteristicInfo(const SeatCabinCharacteristicInfo&);
  SeatCabinCharacteristicInfo& operator=(const SeatCabinCharacteristicInfo&);
};

} // namespace tse

