//----------------------------------------------------------------------------
//  File: FDSuppressFareController.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FDSuppressFare.h"
#include "DBAccess/LocKey.h"
#include "Diagnostic/Diag212CollectorFD.h"


namespace tse
{

class Diag212Collector;

class FDSuppressFareController
{
  friend class FDSuppressFareControllerTest;
  friend class FDSuppressFareControllerForFareDisplayTest;

protected:
  static const Indicator TYPE_NONE;
  static const PseudoCityCode PCC_NONE;
  static const TJRGroup TJR_NONE;
  static const CarrierCode CARRIER_NONE;

  enum SuppressFareMatchLevel
  {
    SFS_AGENTPCC,
    SFS_MAINPCC,
    SFS_TJRGROUPNO
  };

  virtual std::vector<const tse::FDSuppressFare*>& getSuppressFareList(const FareDisplayTrx& trx,
                                                                       const PseudoCityCode& pCC,
                                                                       const Indicator pCCType,
                                                                       const TJRGroup& ssgGroupNo,
                                                                       const CarrierCode& carrier,
                                                                       const DateTime& date)
  {
    return trx.dataHandle().getSuppressFareList(pCC, pCCType, ssgGroupNo, carrier, date);
  }
  virtual std::vector<const tse::FDSuppressFare*>&
  getSuppressFareList(FareDisplayTrx& trx, SuppressFareMatchLevel level);

  virtual bool useTjrGroupNo(TJRGroup tjrGroup);

  virtual bool find(const std::vector<const tse::FDSuppressFare*>& fdSuppressFareList,
                    const Loc& origin,
                    const Loc& destination,
                    std::set<CarrierCode>& matchedCarriers);

  virtual bool eliminateCarriers(std::set<CarrierCode>& from, const std::set<CarrierCode>& which);

  virtual bool processSupressRecords(FareDisplayTrx& trx,
                                     SuppressFareMatchLevel level,
                                     std::set<CarrierCode>& matchedCarriers);

  Indicator& fareDisplayType() { return _fareDisplayType; }
  const Indicator& fareDisplayType() const { return _fareDisplayType; }

private:
  Indicator _fareDisplayType = ' ';
  Diag212CollectorFD* _diag = nullptr;
  bool _diagActive = false;

public:
  virtual void suppressFare(FareDisplayTrx& trx);
  virtual ~FDSuppressFareController() = default;

}; // End of Class FDSuppressFareController
} // end of namespace
