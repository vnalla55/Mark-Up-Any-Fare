// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class BankIdentificationInfo
{
public:
  BankIdentificationInfo() : _cardType(' '), _inhibit(' '), _validityInd(' ') {}

  ~BankIdentificationInfo() {}

  FopBinNumber& binNumber() { return _binNumber; }
  const FopBinNumber& binNumber() const { return _binNumber; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& cardType() { return _cardType; }
  const Indicator& cardType() const { return _cardType; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& lastModDate() { return _lastModDate; }
  const DateTime& lastModDate() const { return _lastModDate; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  bool operator==(const BankIdentificationInfo& rhs) const
  {
    return ((_binNumber == rhs._binNumber) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_cardType == rhs._cardType) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_lastModDate == rhs._lastModDate) && (_inhibit == rhs._inhibit) &&
            (_validityInd == rhs._validityInd));
  }

  static void dummyData(BankIdentificationInfo& obj)
  {
    obj._binNumber = "123456";
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._cardType = 'C';
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._lastModDate = time(nullptr);
    obj._inhibit = 'E';
    obj._validityInd = 'F';
  }

protected:
  FopBinNumber _binNumber;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _cardType;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _lastModDate;
  Indicator _inhibit;
  Indicator _validityInd;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _binNumber);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _cardType);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _lastModDate);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _validityInd);
  }

private:
  BankIdentificationInfo(const BankIdentificationInfo& rhs);
  BankIdentificationInfo& operator=(const BankIdentificationInfo& rhs);
};
}

