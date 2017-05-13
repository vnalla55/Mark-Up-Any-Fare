//----------------------------------------------------------------------------
//  File: FDAddOnFareController.h
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

#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "FareDisplay/FDAddOnGroupingStrategy.h"
#include "FareDisplay/RoutingSequenceGenerator.h"


namespace tse
{

class FDAddOnFareController
{
  friend class FDAddOnFareControllerTest;

  // -----------------
  // Protected
  // -----------------
protected:
  virtual DataHandle& getDataHandle(FareDisplayTrx& trx) { return trx.dataHandle(); }

  virtual const std::vector<AddonFareInfo*>& get(DataHandle& dataHandle,
                                                 const LocCode& origin,
                                                 const LocCode& destination,
                                                 const CarrierCode& carrier,
                                                 const RecordScope& crossRefType,
                                                 const DateTime& date);

  bool populate(FareDisplayTrx& trx,
                const std::vector<AddonFareInfo*>& tempAddOnFareVec,
                const GlobalDirection& globalDir);

  bool groupAndSort();

  bool generateRoutingSeqNo(const RecordScope& crossRefType);

  bool generateAltCurency(FareDisplayTrx& trx,
                          const std::vector<FDAddOnFareInfo*>& addOnFaresVec,
                          std::vector<CurrencyCode>& alternateCurrencyVec);

  // ----------------------------------------------
  // New class for Unique Currency Determination
  // ----------------------------------------------
  class UniqueCurrency
  {
  public:
    UniqueCurrency(std::set<CurrencyCode>& currSet, const CurrencyCode& currency)
      : _currSet(currSet), _currency(currency)
    {
    }

    ~UniqueCurrency() {}

    void operator()(const tse::FDAddOnFareInfo* info)
    {
      if (info == nullptr)
        return;

      if (info->cur().empty())
        return;

      if (info->cur() == _currency)
        return;

      _currSet.insert(info->cur());
    }

  private:
    std::set<CurrencyCode>& _currSet;
    const CurrencyCode& _currency;
  };

  // -----------------
  // Public
  // -----------------
public:
  FDAddOnFareController(std::vector<FDAddOnFareInfo*>& addOnFaresVec)
    : _addOnFaresVec(addOnFaresVec) {};
  virtual ~FDAddOnFareController() {};

  bool process(FareDisplayTrx& trx, const GlobalDirection& globalDir = ZZ);

  bool isEmpty() { return (_addOnFaresVec.empty()) ? true : false; }

  bool
  isAvaiableInReverseCityPair(FareDisplayTrx& trx, std::vector<CurrencyCode>& alternateCurrencyVec);

  virtual bool
  alternateCurrencyVec(FareDisplayTrx& trx, std::vector<CurrencyCode>& alternateCurrencyVec);

  // -----------------
  // Private
  // -----------------
private:
  std::vector<FDAddOnFareInfo*>& _addOnFaresVec;

}; // End of Class FDAddOnFareController

} // end of namespace

