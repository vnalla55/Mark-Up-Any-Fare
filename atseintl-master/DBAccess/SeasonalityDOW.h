//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

#include <cstdlib>

namespace tse
{

class SeasonalityDOW
{
public:
  SeasonalityDOW() = default;
  SeasonalityDOW(const SeasonalityDOW&) = delete;
  SeasonalityDOW& operator=(const SeasonalityDOW&) = delete;

  bool operator==(const SeasonalityDOW& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_inhibit == rhs._inhibit) && (_indA == rhs._indA) && (_indB == rhs._indB) &&
            (_indC == rhs._indC) && (_indD == rhs._indD) && (_indE == rhs._indE) &&
            (_indF == rhs._indF) && (_indG == rhs._indG) && (_indH == rhs._indH) &&
            (_indI == rhs._indI) && (_indJ == rhs._indJ) && (_indK == rhs._indK) &&
            (_indL == rhs._indL) && (_indM == rhs._indM) && (_indN == rhs._indN) &&
            (_indO == rhs._indO) && (_indP == rhs._indP) && (_indQ == rhs._indQ) &&
            (_indR == rhs._indR) && (_indS == rhs._indS) && (_indT == rhs._indT) &&
            (_indU == rhs._indU) && (_indV == rhs._indV) && (_indW == rhs._indW) &&
            (_indX == rhs._indX) && (_indY == rhs._indY) && (_indZ == rhs._indZ));
  }

  static void dummyData(SeasonalityDOW& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 123;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._inhibit = 'X';
    obj._indA = 'A';
    obj._indB = 'B';
    obj._indC = 'C';
    obj._indD = 'D';
    obj._indE = 'E';
    obj._indF = 'F';
    obj._indG = 'G';
    obj._indH = 'H';
    obj._indI = 'I';
    obj._indJ = 'J';
    obj._indK = 'K';
    obj._indL = 'L';
    obj._indM = 'M';
    obj._indN = 'N';
    obj._indO = 'O';
    obj._indP = 'P';
    obj._indQ = 'Q';
    obj._indR = 'R';
    obj._indS = 'S';
    obj._indT = 'T';
    obj._indU = 'U';
    obj._indV = 'V';
    obj._indW = 'W';
    obj._indX = 'X';
    obj._indY = 'Y';
    obj._indZ = 'Z';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _indA);
    FLATTENIZE(archive, _indB);
    FLATTENIZE(archive, _indC);
    FLATTENIZE(archive, _indD);
    FLATTENIZE(archive, _indE);
    FLATTENIZE(archive, _indF);
    FLATTENIZE(archive, _indG);
    FLATTENIZE(archive, _indH);
    FLATTENIZE(archive, _indI);
    FLATTENIZE(archive, _indJ);
    FLATTENIZE(archive, _indK);
    FLATTENIZE(archive, _indL);
    FLATTENIZE(archive, _indM);
    FLATTENIZE(archive, _indN);
    FLATTENIZE(archive, _indO);
    FLATTENIZE(archive, _indP);
    FLATTENIZE(archive, _indQ);
    FLATTENIZE(archive, _indR);
    FLATTENIZE(archive, _indS);
    FLATTENIZE(archive, _indT);
    FLATTENIZE(archive, _indU);
    FLATTENIZE(archive, _indV);
    FLATTENIZE(archive, _indW);
    FLATTENIZE(archive, _indX);
    FLATTENIZE(archive, _indY);
    FLATTENIZE(archive, _indZ);
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int itemNo() const { return _itemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator inhibit() const { return _inhibit; }

  Indicator& indA() { return _indA; }
  const Indicator indA() const { return _indA; }

  Indicator& indB() { return _indB; }
  const Indicator indB() const { return _indB; }

  Indicator& indC() { return _indC; }
  const Indicator indC() const { return _indC; }

  Indicator& indD() { return _indD; }
  const Indicator indD() const { return _indD; }

  Indicator& indE() { return _indE; }
  const Indicator indE() const { return _indE; }

  Indicator& indF() { return _indF; }
  const Indicator indF() const { return _indF; }

  Indicator& indG() { return _indG; }
  const Indicator indG() const { return _indG; }

  Indicator& indH() { return _indH; }
  const Indicator indH() const { return _indH; }

  Indicator& indI() { return _indI; }
  const Indicator indI() const { return _indI; }

  Indicator& indJ() { return _indJ; }
  const Indicator indJ() const { return _indJ; }

  Indicator& indK() { return _indK; }
  const Indicator indK() const { return _indK; }

  Indicator& indL() { return _indL; }
  const Indicator indL() const { return _indL; }

  Indicator& indM() { return _indM; }
  const Indicator indM() const { return _indM; }

  Indicator& indN() { return _indN; }
  const Indicator indN() const { return _indN; }

  Indicator& indO() { return _indO; }
  const Indicator indO() const { return _indO; }

  Indicator& indP() { return _indP; }
  const Indicator indP() const { return _indP; }

  Indicator& indQ() { return _indQ; }
  const Indicator indQ() const { return _indQ; }

  Indicator& indR() { return _indR; }
  const Indicator indR() const { return _indR; }

  Indicator& indS() { return _indS; }
  const Indicator indS() const { return _indS; }

  Indicator& indT() { return _indT; }
  const Indicator indT() const { return _indT; }

  Indicator& indU() { return _indU; }
  const Indicator indU() const { return _indU; }

  Indicator& indV() { return _indV; }
  const Indicator indV() const { return _indV; }

  Indicator& indW() { return _indW; }
  const Indicator indW() const { return _indW; }

  Indicator& indX() { return _indX; }
  const Indicator indX() const { return _indX; }

  Indicator& indY() { return _indY; }
  const Indicator indY() const { return _indY; }

  Indicator& indZ() { return _indZ; }
  const Indicator indZ() const { return _indZ; }

private:
  VendorCode _vendor;
  int _itemNo = 0;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _inhibit = ' ';
  Indicator _indA = ' ';
  Indicator _indB = ' ';
  Indicator _indC = ' ';
  Indicator _indD = ' ';
  Indicator _indE = ' ';
  Indicator _indF = ' ';
  Indicator _indG = ' ';
  Indicator _indH = ' ';
  Indicator _indI = ' ';
  Indicator _indJ = ' ';
  Indicator _indK = ' ';
  Indicator _indL = ' ';
  Indicator _indM = ' ';
  Indicator _indN = ' ';
  Indicator _indO = ' ';
  Indicator _indP = ' ';
  Indicator _indQ = ' ';
  Indicator _indR = ' ';
  Indicator _indS = ' ';
  Indicator _indT = ' ';
  Indicator _indU = ' ';
  Indicator _indV = ' ';
  Indicator _indW = ' ';
  Indicator _indX = ' ';
  Indicator _indY = ' ';
  Indicator _indZ = ' ';

};
}
