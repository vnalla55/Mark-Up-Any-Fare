//-------------------------------------------------------------------
//
//  File:        NetRemitFarePath.h
//  Created:     Feb 15, 2006
//  Authors:
//
//  Description: Fare Path contains Net Remit Fares.
//
//  Copyright Sabre 2006
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

#include "DataModel/FarePath.h"
#include "DBAccess/Record2Types.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class PricingUnit;
class FareUsage;
class PricingTrx;
class DataHandle;
class PaxTypeFare;
class NegFareRest;

class NetRemitFarePath : public FarePath
{
public:
  NetRemitFarePath() : _trx(nullptr), _dataHandle(nullptr), _origFp(nullptr), _usedFareBox(false) {}
  virtual ~NetRemitFarePath() {}
  void initialize(
      PricingTrx* _trx,
      FarePath* origFp,
      std::map<const PaxTypeFare*, const CombinabilityRuleInfo*>* cat10ForNetFaresMap = nullptr);

  const FarePath* originalFarePath() const { return _origFp; }

  const std::multimap<FareUsage*, FareUsage*>& fareUsageMap() const { return _fareUsageMap; }
  std::multimap<FareUsage*, FareUsage*>& fareUsageMap() { return _fareUsageMap; }

  void getNetRemitFareUsage(FareUsage* origFu, FareUsage*& netRemitFu1, FareUsage*& netRemitFu2);
  const FareUsage* getOriginalFareUsage(const FareUsage* netRemitFu) const;
  static bool isTFDPSCFareAmtFare(const FareUsage* fu);
  const bool usedFareBox() const { return _usedFareBox; }
  bool& usedFareBox() { return _usedFareBox; }

  virtual NetRemitFarePath* clone(DataHandle& dataHandle) const override;

private:
  void copyItin();

  PricingTrx* _trx;
  DataHandle* _dataHandle;
  FarePath* _origFp;
  bool _usedFareBox;

  std::multimap<FareUsage*, FareUsage*> _fareUsageMap;
};

class CalcNUCAmount
{
public:
  CalcNUCAmount(NetRemitFarePath& fp) : _fp(fp) {}

  void operator()(PricingUnit* pu);

private:
  NetRemitFarePath& _fp;
};

class CloneAndAddPricingUnit
{
public:
  CloneAndAddPricingUnit(
      PricingTrx* trx,
      DataHandle* dataHandle,
      NetRemitFarePath& fp,
      std::vector<PricingUnit*>& pricingUnits,
      const std::map<const PaxTypeFare*, const CombinabilityRuleInfo*>* cat10ForNetFaresMap = nullptr)
    : _trx(trx),
      _dataHandle(dataHandle),
      _fp(fp),
      _pricingUnits(pricingUnits),
      _itin(*(fp.itin())),
      _cat10ForNetFaresMap(cat10ForNetFaresMap)
  {
  }

  void operator()(PricingUnit* pu);

private:
  PricingTrx* _trx;
  DataHandle* _dataHandle;
  NetRemitFarePath& _fp;
  std::vector<PricingUnit*>& _pricingUnits;
  Itin& _itin;
  const std::map<const PaxTypeFare*, const CombinabilityRuleInfo*>* _cat10ForNetFaresMap;
};

class CloneAndAddFareUsage
{
public:
  CloneAndAddFareUsage(
      PricingTrx* trx,
      DataHandle* dataHandle,
      NetRemitFarePath& fp,
      PricingUnit& pu,
      const std::map<const PaxTypeFare*, const CombinabilityRuleInfo*>* cat10ForNetFaresMap = nullptr)
    : _trx(trx),
      _dataHandle(dataHandle),
      _fp(fp),
      _pu(pu),
      _itin(*(fp.itin())),
      _cat10ForNetFaresMap(cat10ForNetFaresMap)
  {
  }

  void operator()(FareUsage* pu);

private:
  bool setHrtIndForNetRemitFare(const PaxTypeFare* tktNetRemitFare, const PaxTypeFare* ptf) const;
  void setupFareUsage(FareUsage*& newFu,
                      FareUsage* fu,
                      const std::vector<TravelSeg*>& travelSegs,
                      bool isAsteriskInd);
  void cloneFareUsageForTFDPSC(FareUsage* fu);
  void updateNucFareAmount(FareUsage* newFu, FareUsage* fu, bool& isAsteriskInd) const;
  void overlayFareAmount(FareUsage* newFu, FareUsage* fu);
  void setNucAmounts(PaxTypeFare& ptf);
  bool useFareBox(const NegFareRest& negFareRest) const;

  PricingTrx* _trx;
  DataHandle* _dataHandle;
  NetRemitFarePath& _fp;
  PricingUnit& _pu;
  Itin& _itin;
  const std::map<const PaxTypeFare*, const CombinabilityRuleInfo*>* _cat10ForNetFaresMap;
};

} // tse namespace

