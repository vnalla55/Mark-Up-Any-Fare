//----------------------------------------------------------------------------
//  File:           TaxResponse.h
//  Description:    TaxResponse header file for ATSE International Project
//  Created:        2/11/2004
//  Authors:        Dean Van Decker
//
//  Description: This Object will be used for all Tax Response functionality.
//          Tax Package will build TaxResponse Objects for each Passenger Type and
//          Fare Calculation. The Fare Calculation Configuration and
//          Tax Diagnostics will utilize these objects.
//
//
//  Updates:
//          2/11/04 - DVD - updated for model changes.
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AutomaticPfcTaxExemptionData.h"

#include <boost/optional.hpp>

#include <string>

namespace tse
{
class DiagCollector;
class FarePath;
class Itin;
class Money;
class PfcItem;
class Tax;
class TaxDisplayItem;
class TaxItem;
class TaxRecord;

// ----------------------------------------------------------------------------
// <PRE>
//
// @class TaxResponse
// Description:  Handles creation and initialization of TaxResponseVector elements.
//
// </PRE>
// ----------------------------------------------------------------------------

class TaxResponse
{
public:
  /// Find response for given farepath
  /// Itinerary is taken from the farepath
  /// @return Pointer to response or NULL if there is none
  static const TaxResponse* findFor(const FarePath*);

  /// Find response for given itinerary and farepath.
  /// @return Pointer to response or NULL if there is none
  static const TaxResponse* findFor(const Itin*, const FarePath*);
  // static       TaxResponse* findFor(      Itin*, const FarePath*);

  //-----------------------------------------------------------------------------
  // Convenience typedefs
  //-----------------------------------------------------------------------------

  typedef std::vector<PfcItem*> PfcItemVector;
  typedef std::vector<TaxRecord*> TaxRecordVector;
  typedef std::vector<TaxItem*> TaxItemVector;
  typedef std::vector<TaxItem*> ChangeFeeTaxItemVector;
  typedef std::vector<TaxDisplayItem*> TaxDisplayItemVector;

  TaxResponse() : _farePath(nullptr), _diagCollector(nullptr), _displayed(false), _createdInAdvance(false)
  {
  }

  //-----------------------------------------------------------------------------
  // Copy Constructor is Public for this Pooled Objects
  //-----------------------------------------------------------------------------

  const PaxTypeCode& paxTypeCode() const { return _paxTypeCode; }
  PaxTypeCode& paxTypeCode() { return _paxTypeCode; }

  const FarePath* farePath() const { return _farePath; }
  FarePath*& farePath() { return _farePath; }

  const DiagCollector* diagCollector() const { return _diagCollector; }
  DiagCollector*& diagCollector() { return _diagCollector; }

  const CarrierCode& validatingCarrier() const { return _valCxr; }
  CarrierCode& validatingCarrier() { return _valCxr; }

  const PfcItemVector& pfcItemVector() const { return _pfcItemVector; }
  PfcItemVector& pfcItemVector() { return _pfcItemVector; }

  const TaxItemVector& taxItemVector() const { return _taxItemVector; }
  TaxItemVector& taxItemVector() { return _taxItemVector; }

  const ChangeFeeTaxItemVector& changeFeeTaxItemVector() const { return _changeFeeTaxItemVector; }
  ChangeFeeTaxItemVector& changeFeeTaxItemVector() { return _changeFeeTaxItemVector; }

  const TaxDisplayItemVector& taxDisplayItemVector() const { return _taxDisplayItemVector; }
  TaxDisplayItemVector& taxDisplayItemVector() { return _taxDisplayItemVector; }

  const TaxRecordVector& taxRecordVector() const { return _taxRecordVector; }
  TaxRecordVector& taxRecordVector() { return _taxRecordVector; }

  const bool& createdInAdvance() const { return _createdInAdvance; }
  bool& createdInAdvance() { return _createdInAdvance; }

  const boost::optional<AutomaticPfcTaxExemptionData>& automaticPFCTaxExemptionData() const
  {
    return _automaticPFCTaxExemptionData;
  }
  boost::optional<AutomaticPfcTaxExemptionData>& automaticPFCTaxExemptionData()
  {
    return _automaticPFCTaxExemptionData;
  }

  void getTaxRecordTotal(Money& taxTotal) const;
  bool& displayed() {return _displayed;}
  const bool& isDisplayed() const {return _displayed;}

  std::vector<SettlementPlanType>& settlementPlans() { return _settlementPlans; }
  const std::vector<SettlementPlanType>& settlementPlans() const { return _settlementPlans; }

private:
  FarePath* _farePath;
  DiagCollector* _diagCollector;
  PaxTypeCode _paxTypeCode;
  CarrierCode _valCxr;

  PfcItemVector _pfcItemVector;
  TaxRecordVector _taxRecordVector;
  TaxItemVector _taxItemVector;
  ChangeFeeTaxItemVector _changeFeeTaxItemVector;
  TaxDisplayItemVector _taxDisplayItemVector;
  std::vector<SettlementPlanType> _settlementPlans;

  boost::optional<AutomaticPfcTaxExemptionData> _automaticPFCTaxExemptionData;
  bool _displayed;
  bool _createdInAdvance;
};
} // namespace tse
