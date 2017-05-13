//----------------------------------------------------------------------------
//  File:           FareTypeMatcher.h
//  Description:    Fare type pricing - fare matcher logic
//  Created:        03/07/2007
//  Authors:        Quan Ta
//
//  Updates:
//
// Copyright 2009, Sabre Inc.  All rights reserved.  This software/documentation is the
// confidential and proprietary product of Sabre Inc. Any unauthorized use,
// reproduction, or transfer of this software/documentation, in any medium, or
// incorporation of this software/documentation into any system or publication,
// is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/FareTypeQualifier.h"

#include <memory>
#include <vector>

namespace tse
{
class PricingTrx;
class PricingOptions;
class FarePath;
class PricingUnit;
class PaxTypeFare;
class Diag203Collector;

class FareTypeMatcher
{
  friend class FareTypeMatcherTest;

public:
  enum TripType
  {
    Unknown = 0,
    Domestic = 'D',
    International = 'I',
    EndOnEnd = 'E'
  };

  FareTypeMatcher(PricingTrx& trx, const FarePath* farePath = nullptr)
    : _trx(trx),
      _farePath(farePath),
      _journeyType(FareTypeMatcher::Unknown),
      _diag(nullptr),
      _groupTrailerMsg(false),
      _itTrailerMsg(false)
  {
    _journeyType = journeyType();
    getFareTypeQualifier();
  }

  bool operator()(const FareType& fareType) const;
  bool operator()(const PaxTypeFare* ptf) const;
  bool operator()(const FareType& fareType, const PaxTypeFare* ptf) const;

  bool operator()(const FarePath* fp) const;
  bool operator()(const PricingUnit* pu) const;

  const std::vector<FareTypeQualifier*>& fareTypeQualifier() const { return _ftqList; }
  const std::set<PaxTypeCode>& psgTypes() const { return _psgTypes; }

  std::string getMessage() const
  {
    if (_msg)
    {
      return _msg->str();
    }
    else
    {
      return "";
    }
  }

  static std::string getFtPricingQualifier(const PricingOptions& options);

  bool groupTrailerMsg() const { return _groupTrailerMsg; }
  bool itTrailerMsg() const { return _itTrailerMsg; }

private:
  void getFareTypeQualifier();
  const FareTypeQualifier* getFareTypeQualifier(TripType puType) const;

  TripType journeyType() const;
  TripType puTripType(const PricingUnit& pu) const;

  bool matchFareTypeQualifier(const FareTypeQualifier* ftq, const FareType& fareType) const;

  // Match fare against the fare type qualifier item.
  bool matchFareTypeQualifier(const FareTypeQualifier* ftq,
                              const FareType& fareType,
                              const PaxTypeFare* ptf) const;

  bool matchFareTypeQualifier(const FareTypeQualifier* ftq,
                              const FareType& fareType,
                              const PaxTypeFare* ptf,
                              bool& ftrMatched) const;

  // Check if FareTypeRequired field processing is required.
  bool hasFareTypeRequired() const;
  bool hasFareTypeRequired(std::set<PaxTypeCode>& ftReqs, TripType puType) const;

  // Match FareTypeRequired field
  bool matchFareTypeRequired(const FarePath* fp, TripType puType) const;
  bool matchFareTypePermittedFP(const FarePath* fp) const;

  bool matchFareTypePermitted(const FarePath* fp, TripType puType) const;
  bool matchFareTypePermittedPU(const PricingUnit* pu, TripType puType) const;

private:
  PricingTrx& _trx;
  const FarePath* _farePath;
  TripType _journeyType;
  std::vector<FareTypeQualifier*> _ftqList;
  std::set<PaxTypeCode> _psgTypes;

  std::ostringstream& msg() const
  {
    if (!_msg)
      _msg.reset(new std::ostringstream);

    return *_msg;
  }
  mutable std::unique_ptr<std::ostringstream> _msg;
  mutable Diag203Collector* _diag;

  mutable bool _groupTrailerMsg;
  mutable bool _itTrailerMsg;
};

struct MatchJourneyType
{
  MatchJourneyType(FareTypeMatcher::TripType t, bool m = true) : tripType(t), match(m) {}
  const FareTypeMatcher::TripType tripType;
  const bool match;

  bool operator()(const FareTypeQualifier* ftq) const;
};

struct MatchPuType
{
  MatchPuType(FareTypeMatcher::TripType t, bool m = true) : tripType(t), match(m) {}
  const FareTypeMatcher::TripType tripType;
  const bool match;

  bool operator()(const FareTypeQualifier* ftq) const;
};
}

