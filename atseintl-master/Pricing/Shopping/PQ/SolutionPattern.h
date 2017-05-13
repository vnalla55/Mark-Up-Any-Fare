// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         14-09-2011
//! \file         SolutionPattern.h
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include "Common/fvector.h"
#include "Common/ShpqTypes.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Trx.h"
#include "Pricing/Shopping/PQ/PUFullType.h"

#include <boost/noncopyable.hpp>

#include <algorithm>

#include <cstdint>

namespace tse
{
class FareMarketPath;
class PUPath;
}

namespace tse
{
namespace shpq
{
class SoloPUPathCollector;
class SolutionPatternStorage;

class DiagSoloPQCollector;

class SolutionPattern
{
public:
  using SPNumberType = uint8_t;
  using PUTypeVector = fvector<PUFullTypePtr, 4>;
  enum SPEnumType
  {
    SP10,
    SP11,
    SP12,
    SP20,
    SP21,
    SP30,
    SP31,
    SP32,
    SP33,
    SP34,
    SP35,
    SP36,
    SP37,
    SP40,
    SP41,
    SP42,
    SP43,
    SP44,
    SP45,
    SP46,
    SP47,
    SP48,
    SP49,
    NUMBER_OF_SPS,

    /*alias*/
    SP_SNOWMAN = SP41
  };

  SPNumberType getSPNumber() const { return _number; }
  SPEnumType getSPId() const { return _id; }
  SolutionType getOutboundSolution() const { return _outboundST; }
  SolutionType getInboundSolution() const { return _inboundST; }
  PUTypeVector getPUTypeVector() const { return _puTypeVector; }
  bool hasOutboundSol() const { return _outboundST != NONE; }
  bool hasInboundSol() const { return _inboundST != NONE; }
  const char* getSPIdStr() const { return _idCStr; }
  const char* getOutboundSolCStr() const { return getSPCStr(_outboundST); }
  const char* getInboundSolCStr() const { return getSPCStr(_inboundST); }
  const std::string& getPUPathStr() const { return _puPathStr; }
  bool hasHRT() const { return _hasHRT; }
  bool isEoePattern() const { return _puTypeVector.size() > 1; }

  bool isThruPattern() const { return (_id == SP10 || _id == SP20 || _id == SP21); }

  bool isItemValid(const ConxRoutePQItem* const, DiagSoloPQCollector&) const;
  bool createPUPath(SoloPUPathCollector&, FareMarketPath*, PUPath*) const;

private:
  friend class SolutionPatternStorage;
  SolutionPattern() = default;
  SolutionPattern(const SolutionPattern& other);
  SolutionPattern& operator=(const SolutionPattern& other);

  const char* getSPCStr(const SolutionType& type) const;
  void setPUPathStr();
  std::string validationFailMsg(const PUFullTypePtr) const;

  template <size_t N_PuPaths>
  SolutionPattern(const SPEnumType id,
                  const char* const idCStr,
                  const SPNumberType number,
                  const SolutionType outboundST,
                  const SolutionType inboundST,
                  const PUFullTypePtr (&puPaths)[N_PuPaths]);

private:
  SPEnumType _id = SolutionPattern::SPEnumType::NUMBER_OF_SPS;
  const char* _idCStr = "";
  SPNumberType _number = 0;
  SolutionType _outboundST = SolutionType::NONE;
  SolutionType _inboundST = SolutionType::NONE;
  PUTypeVector _puTypeVector;
  std::string _puPathStr;
  bool _hasHRT = false;
};

class SolutionPatternStorage : private boost::noncopyable
{
public:
  typedef const SolutionPattern* const_iterator;
  typedef SolutionPattern::SPNumberType SPNumberType;
  typedef SolutionPattern::SPEnumType SPEnumType;

  static const SolutionPatternStorage& instance() { return instance_; }

  const SolutionPattern& getSPById(const SPEnumType& sp) const { return solPatterns_[sp]; }
  const_iterator begin() const { return solPatterns_; }
  const_iterator end() const { return solPatterns_ + SolutionPattern::NUMBER_OF_SPS; }

  const SolutionPattern& getSPByNumber(const SPNumberType& number) const;

private:
  SolutionPatternStorage();
  static const SolutionPatternStorage instance_;
  mutable SolutionPattern solPatterns_[SolutionPattern::NUMBER_OF_SPS + 1];
};

} /* namespace shpq */
} /* namespace tse */
