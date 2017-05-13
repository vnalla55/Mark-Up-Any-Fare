//-------------------------------------------------------------------
//
//  File:        SurchargeData.h
//  Created:     August 9, 2004
//  Authors:     Vladimir Koliasnikov
//
//  Description:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class SurchargeData
{
public:
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  int& itinItemCount() { return _itinItemCount; }
  const int& itinItemCount() const { return _itinItemCount; }

  Indicator& surchargeAppl() { return _surchargeAppl; }
  const Indicator& surchargeAppl() const { return _surchargeAppl; }

  Indicator& surchargeType() { return _surchargeType; }
  const Indicator& surchargeType() const { return _surchargeType; }

  Indicator& travelPortion() { return _travelPortion; }
  const Indicator& travelPortion() const { return _travelPortion; }

  MoneyAmount& amountSelected() { return _amountSelected; }
  const MoneyAmount& amountSelected() const { return _amountSelected; }

  CurrencyCode& currSelected() { return _currSelected; }
  const CurrencyCode& currSelected() const { return _currSelected; }

  CurrencyNoDec& currNoDecSelected() { return _currNoDecSelected; }
  const CurrencyNoDec& currNoDecSelected() const { return _currNoDecSelected; }

  MoneyAmount& amountNuc() { return _amountNuc; }
  const MoneyAmount& amountNuc() const { return _amountNuc; }

  LocCode& brdAirport() { return _brdAirport; }
  const LocCode& brdAirport() const { return _brdAirport; }

  LocCode& offAirport() { return _offAirport; }
  const LocCode& offAirport() const { return _offAirport; }

  int& geoTblNo() { return _geoTblNo; }
  const int& geoTblNo() const { return _geoTblNo; }

  TSICode& tsi() { return _tsi; }
  const TSICode& tsi() const { return _tsi; }

  LocKey& locKey1() { return _locKey1; }
  const LocKey& locKey1() const { return _locKey1; }

  LocKey& locKey2() { return _locKey2; }
  const LocKey& locKey2() const { return _locKey2; }

  int& geoTblNoBtw() { return _geoTblNoBtw; }
  const int& geoTblNoBtw() const { return _geoTblNoBtw; }

  TSICode& tsiBtw() { return _tsiBtw; }
  const TSICode& tsiBtw() const { return _tsiBtw; }

  LocKey& locKeyBtw1() { return _locKeyBtw1; }
  const LocKey& locKeyBtw1() const { return _locKeyBtw1; }

  LocKey& locKeyBtw2() { return _locKeyBtw2; }
  const LocKey& locKeyBtw2() const { return _locKeyBtw2; }

  int& geoTblNoAnd() { return _geoTblNoAnd; }
  const int& geoTblNoAnd() const { return _geoTblNoAnd; }

  TSICode& tsiAnd() { return _tsiAnd; }
  const TSICode& tsiAnd() const { return _tsiAnd; }

  LocKey& locKeyAnd1() { return _locKeyAnd1; }
  const LocKey& locKeyAnd1() const { return _locKeyAnd1; }

  LocKey& locKeyAnd2() { return _locKeyAnd2; }
  const LocKey& locKeyAnd2() const { return _locKeyAnd2; }

  bool& processedTkt() { return _processedTkt; }
  const bool& processedTkt() const { return _processedTkt; }

  bool& selectedTkt() { return _selectedTkt; }
  const bool& selectedTkt() const { return _selectedTkt; }

  std::string& surchargeDesc() { return _surchargeDesc; }
  const std::string& surchargeDesc() const { return _surchargeDesc; }

  bool& isFromOverride() { return _isFromOverride; }
  const bool& isFromOverride() const { return _isFromOverride; }

  bool& singleSector() { return _singleSector; }
  const bool& singleSector() const { return _singleSector; }

  LocCode& fcBrdCity() { return _fcBrdCity; }
  const LocCode& fcBrdCity() const { return _fcBrdCity; }

  LocCode& fcOffCity() { return _fcOffCity; }
  const LocCode& fcOffCity() const { return _fcOffCity; }

  bool& fcFpLevel() { return _fcFpLevel; }
  const bool& fcFpLevel() const { return _fcFpLevel; }

private:
  MoneyAmount _amountSelected = 0; // amount in Selected currency
  MoneyAmount _amountNuc = 0; // Amt in NUC
  LocCode _brdAirport;
  LocCode _offAirport;
  LocCode _fcBrdCity;
  LocCode _fcOffCity;
  LocKey _locKeyBtw1;
  LocKey _locKeyBtw2;
  LocKey _locKey1;
  LocKey _locKey2;
  LocKey _locKeyAnd1;
  LocKey _locKeyAnd2;
  std::string _surchargeDesc;
  CarrierCode _carrier;
  int _itinItemCount = 0;
  bool _processedTkt = false;
  bool _selectedTkt = false;
  bool _isFromOverride = false;
  bool _singleSector = true;
  bool _fcFpLevel = false;
  Indicator _surchargeAppl = ' ';
  Indicator _surchargeType = ' ';
  Indicator _travelPortion = ' ';
  CurrencyCode _currSelected;
  CurrencyNoDec _currNoDecSelected = 0;
  int _geoTblNo = 0; // 995
  int _geoTblNoBtw = 0; // 995 Between
  int _geoTblNoAnd = 0; // 995 And
  TSICode _tsi = 0;
  TSICode _tsiBtw = 0;
  TSICode _tsiAnd = 0;
};

using SurchargeDataPtrVecIC = std::vector<SurchargeData*>::const_iterator;
} // tse
