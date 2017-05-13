//----------------------------------------------------------------------------
//
//     File:           PaxTypeMatrix.h
//     Description:    PsgTypeMatrix data
//     Created:        3/27/2004
//     Authors:        Roger Kelly
//
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------

#ifndef PAXTYPEMATRIX_H
#define PAXTYPEMATRIX_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class PaxTypeMatrix
{
private:
  PaxTypeCode _sabrePaxType;
  PaxTypeCode _atpPaxType;
  CarrierCode _carrier;
  DateTime _createDate;
  PaxTypeCode _alternatePaxType;
  Indicator _bulkInd;

public:
  PaxTypeMatrix() : _bulkInd(' ') {}

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _sabrePaxType);
    FLATTENIZE(archive, _atpPaxType);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _alternatePaxType);
    FLATTENIZE(archive, _bulkInd);
  }

  bool operator==(const PaxTypeMatrix& rhs) const
  {
    return ((_sabrePaxType == rhs._sabrePaxType) && (_atpPaxType == rhs._atpPaxType) &&
            (_carrier == rhs._carrier) && (_createDate == rhs._createDate) &&
            (_alternatePaxType == rhs._alternatePaxType) && (_bulkInd == rhs._bulkInd));
  }

  static void dummyData(PaxTypeMatrix& obj)
  {
    obj._sabrePaxType = "ABC";
    obj._atpPaxType = "DEF";
    obj._carrier = "HIJ";
    obj._createDate = time(nullptr);
    obj._alternatePaxType = "JKL";
    obj._bulkInd = 'M';
  }

  PaxTypeCode& sabrePaxType() { return _sabrePaxType; }
  const PaxTypeCode& sabrePaxType() const { return _sabrePaxType; }

  PaxTypeCode& atpPaxType() { return _atpPaxType; }
  const PaxTypeCode& atpPaxType() const { return _atpPaxType; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  PaxTypeCode& alternatePaxType() { return _alternatePaxType; }
  const PaxTypeCode& alternatePaxType() const { return _alternatePaxType; }

  Indicator& bulkInd() { return _bulkInd; }
  const Indicator& bulkInd() const { return _bulkInd; }
};
}

#endif
