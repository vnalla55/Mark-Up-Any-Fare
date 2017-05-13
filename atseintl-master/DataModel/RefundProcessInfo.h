//-------------------------------------------------------------------
//
//  File:        RefundProcessInfo.h
//  Created:     July 29, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "DataModel/FareCompInfo.h"

namespace tse
{

class VoluntaryRefundsInfo;
class PaxTypeFare;
class FareMarket;

class RefundProcessInfo
{
public:
  RefundProcessInfo() : _record3(nullptr), _paxTypeFare(nullptr), _fareCompInfo(nullptr) {}

  RefundProcessInfo(const VoluntaryRefundsInfo* record3,
                    const PaxTypeFare* paxTypeFare,
                    const FareCompInfo* fareCompInfo)
    : _record3(record3), _paxTypeFare(paxTypeFare), _fareCompInfo(fareCompInfo)
  {
  }

  void assign(const VoluntaryRefundsInfo* record3,
              const PaxTypeFare* paxTypeFare,
              const FareCompInfo* fareCompInfo)
  {
    _record3 = record3;
    _paxTypeFare = paxTypeFare;
    _fareCompInfo = fareCompInfo;
  }

  const VoluntaryRefundsInfo& record3() const { return *_record3; }

  const PaxTypeFare& paxTypeFare() const { return *_paxTypeFare; }

  const FareCompInfo& fareCompInfo() const { return *_fareCompInfo; }

  const FareMarket& fareMarket() const { return *_fareCompInfo->fareMarket(); }

  uint16_t fareCompNumber() const { return _fareCompInfo->fareCompNumber(); }

private:
  const VoluntaryRefundsInfo* _record3;
  const PaxTypeFare* _paxTypeFare;
  const FareCompInfo* _fareCompInfo;
};
}

