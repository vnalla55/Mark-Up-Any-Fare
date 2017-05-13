// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         14-09-2011
//! \file         SolutionPattern.cpp
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
#include "Pricing/Shopping/PQ/SolutionPattern.h"

#include "Common/Assert.h"
#include "Pricing/Shopping/PQ/ConxRoutePQItem.h"
#include "Pricing/Shopping/PQ/DiagSoloPQCollector.h"

#include <boost/format.hpp>

#include <algorithm>

namespace tse
{
namespace shpq
{

namespace
{

struct PUTypeCreator
{
  PUTypeCreator()
    : pOOJ("OOJ", PricingUnit::Type::OPENJAW, PricingUnit::ORIG_OPENJAW),
      pTOJ("TOJ", PricingUnit::Type::OPENJAW, PricingUnit::DEST_OPENJAW),
      pDOW("DOW", PricingUnit::Type::OPENJAW, PricingUnit::DOUBLE_OPENJAW)
  {
  }

  PUFullTypePtr getOW(uint16_t fmIndex) { return OWPUType::create(fmIndex); }
  PUFullTypePtr getRT(uint16_t fmIndex1, uint16_t fmIndex2, bool checkConnectingCities = true)
  {
    return RTPUType::create(fmIndex1, fmIndex2, checkConnectingCities);
  }

  PUFullTypePtr getOOJ(uint16_t outboundInd, uint16_t inboundInd)
  {
    return OJPUType::create(pOOJ, outboundInd, inboundInd);
  }

  PUFullTypePtr getTOJ(uint16_t outboundInd, uint16_t inboundInd)
  {
    return OJPUType::create(pTOJ, outboundInd, inboundInd);
  }

  PUFullTypePtr getDOW(uint16_t outboundInd, uint16_t inboundInd)
  {
    return OJPUType::create(pDOW, outboundInd, inboundInd);
  }

  PUFullTypePtr getCT() { return CTPUType::create(); }

private:
  const PUTypeDetails pOOJ;
  const PUTypeDetails pTOJ;
  const PUTypeDetails pDOW;
};
}

const SolutionPatternStorage SolutionPatternStorage::instance_;

template <size_t N_PuPaths>
SolutionPattern::SolutionPattern(const SolutionPattern::SPEnumType id,
                                 const char* const idCStr,
                                 const SolutionPattern::SPNumberType number,
                                 const SolutionType outboundST,
                                 const SolutionType inboundST,
                                 const PUFullTypePtr (&puPaths)[N_PuPaths])
  : _id(id), _idCStr(idCStr), _number(number), _outboundST(outboundST), _inboundST(inboundST)
{
  std::copy(puPaths, puPaths + N_PuPaths, std::back_inserter(_puTypeVector));
  setPUPathStr();

  switch (_outboundST)
  {
  case HRT:
  case OW_HRT:
  case HRT_OW:
  case HRT_HRT:
    _hasHRT = true;
    break;
  default:
    break;
  }

  switch (_inboundST)
  {
  case HRT:
  case OW_HRT:
  case HRT_OW:
  case HRT_HRT:
    _hasHRT = true;
    break;
  default:
    break;
  }
}

SolutionPattern::SolutionPattern(const SolutionPattern& other)
  : _id(other._id),
    _idCStr(other._idCStr),
    _number(other._number),
    _outboundST(other._outboundST),
    _inboundST(other._inboundST),
    _puTypeVector(other._puTypeVector),
    _puPathStr(other._puPathStr),
    _hasHRT(other._hasHRT)
{
}

SolutionPattern&
SolutionPattern::
operator=(const SolutionPattern& other)
{
  if (this != &other)
  {
    _id = other._id;
    _idCStr = other._idCStr;
    _number = other._number;
    _outboundST = other._outboundST;
    _inboundST = other._inboundST;
    _puTypeVector = other._puTypeVector;
    _puPathStr = other._puPathStr;
    _hasHRT = other._hasHRT;
  }
  return *this;
}

SolutionPatternStorage::SolutionPatternStorage()
{
  PUTypeCreator pu;

  const PUFullTypePtr puOW[] = { pu.getOW(0) };
  const PUFullTypePtr puCT[] = { pu.getCT() };
  const PUFullTypePtr puRT[] = { pu.getRT(0, 1, false) };

  const PUFullTypePtr puOOJ[] = { pu.getOOJ(0, 1) };

  const PUFullTypePtr puTOJ_OOJ[] = { pu.getTOJ(0, 3), pu.getOOJ(1, 2) };
  const PUFullTypePtr puOOJ_OW[] = { pu.getOOJ(0, 1), pu.getOW(2) };
  const PUFullTypePtr puOW_OOJ[] = { pu.getOW(0), pu.getOOJ(1, 2) };
  const PUFullTypePtr puOW_OW[] = { pu.getOW(0), pu.getOW(1) };
  const PUFullTypePtr puRT_RT[] = { pu.getRT(0, 3), pu.getRT(1, 2) };
  const PUFullTypePtr puTOJ_OW[] = { pu.getTOJ(0, 2), pu.getOW(1) };

  const PUFullTypePtr puDOW_OW_OW[] = { pu.getDOW(0, 2), pu.getOW(1), pu.getOW(3) };
  const PUFullTypePtr puOW_DOW_OW[] = { pu.getOW(0), pu.getDOW(1, 3), pu.getOW(2) };
  const PUFullTypePtr puOW_OOJ_OW[] = { pu.getOW(0), pu.getOOJ(1, 2), pu.getOW(3) };
  const PUFullTypePtr puOW_OW_OW[] = { pu.getOW(0), pu.getOW(1), pu.getOW(2) };
  const PUFullTypePtr puOW_RT_OW[] = { pu.getOW(0), pu.getRT(1, 2), pu.getOW(3) };
  const PUFullTypePtr puRT_OW_OW[] = { pu.getRT(0, 3), pu.getOW(1), pu.getOW(2) };
  const PUFullTypePtr puTOJ_OW_OW[] = { pu.getTOJ(0, 3), pu.getOW(1), pu.getOW(2) };

  const PUFullTypePtr puOW_OW_OW_OW[] = { pu.getOW(0), pu.getOW(1), pu.getOW(2), pu.getOW(3) };

#define JOIN(a, b) a##b
#define SP(a) JOIN(SolutionPattern::SP, a)
#define SPCSTR(a) "SP" #a
#define CREATE_SP(sp, ob, ib, pu)                                                                  \
  {                                                                                                \
    ++i;                                                                                           \
  }                                                                                                \
  solPatterns_[SP(sp)] = SolutionPattern(SP(sp), SPCSTR(sp), sp, ob, ib, pu)
  size_t i(0);

  CREATE_SP(10, OW, NONE, puOW);
  CREATE_SP(11, OW_OW, NONE, puOW_OW);
  CREATE_SP(12, HRT_HRT, NONE, puOOJ);

  CREATE_SP(20, OW, OW, puOW_OW);
  CREATE_SP(21, HRT, HRT, puRT);

  CREATE_SP(30, OW_OW, OW, puOW_OW_OW);
  CREATE_SP(31, HRT_OW, HRT, puTOJ_OW);
  CREATE_SP(32, OW_HRT, HRT, puOW_OOJ);
  CREATE_SP(33, OW, OW_OW, puOW_OW_OW);
  CREATE_SP(34, HRT, OW_HRT, puTOJ_OW);
  CREATE_SP(35, HRT, HRT_OW, puOOJ_OW);
  CREATE_SP(36, HRT, HRT_HRT, puCT);
  CREATE_SP(37, HRT_HRT, HRT, puCT);

  CREATE_SP(40, OW_OW, OW_OW, puOW_OW_OW_OW);
  CREATE_SP(41, HRT_HRT, HRT_HRT, puRT_RT);
  CREATE_SP(42, HRT_OW, OW_HRT, puRT_OW_OW);
  CREATE_SP(43, OW_HRT, HRT_OW, puOW_RT_OW);
  CREATE_SP(44, HRT_HRT, HRT_HRT, puTOJ_OOJ);
  CREATE_SP(45, HRT_OW, OW_HRT, puTOJ_OW_OW);
  CREATE_SP(46, OW_HRT, HRT_OW, puOW_OOJ_OW);
  CREATE_SP(47, OW_HRT, OW_HRT, puOW_DOW_OW);
  CREATE_SP(48, HRT_OW, HRT_OW, puDOW_OW_OW);
  CREATE_SP(49, HRT_HRT, HRT_HRT, puCT);

  TSE_ASSERT(i == SolutionPattern::NUMBER_OF_SPS);
}

struct BySPNumberLess
{
  template <typename T1, typename T2>
  bool operator()(T1& sp1, T2& sp2) const
  {
    return sp1.getSPNumber() < sp2;
  }
};

const SolutionPattern&
SolutionPatternStorage::getSPByNumber(const SPNumberType& number) const
{

  const const_iterator it = std::lower_bound(begin(), end(), number, BySPNumberLess());
  if (it == end() || it->getSPNumber() != number)
  {
    throw std::out_of_range(boost::str(boost::format("Cannot find solution pattern number %d") %
                                       static_cast<int>(number)));
  }
  return *it;
}

const char*
SolutionPattern::getSPCStr(const SolutionType& type) const
{
  switch (type)
  {
  case NONE:
    return "NONE";
  case OW:
    return "OW";
  case HRT:
    return "HRT";
  case OW_OW:
    return "OW_OW";
  case OW_HRT:
    return "OW_HRT";
  case HRT_OW:
    return "HRT_OW";
  case HRT_HRT:
    return "HRT_HRT";
  }
  return "UNKNOWN";
}

void
SolutionPattern::setPUPathStr()
{
  std::string plusSep(" + ");
  for (const PUFullTypePtr puFullType : _puTypeVector)
    _puPathStr +=
        _puPathStr.empty() ? puFullType->getPUIdStr() : plusSep + puFullType->getPUIdStr();
}

std::string
SolutionPattern::validationFailMsg(const PUFullTypePtr puType) const
{
  std::string msg = (boost::format("FAIL: %s %s [%s %s]") % puType->getPUIdStr() %
                     puType->getFMOrder() % getSPIdStr() % getPUPathStr()).str();
  return msg;
}

bool
SolutionPattern::isItemValid(const ConxRoutePQItem* const pqItem, DiagSoloPQCollector& dc) const
{
  if (pqItem)
  {
    for (const PUFullTypePtr puFullType : _puTypeVector)
    {
      if (LIKELY(!puFullType->isItemValid(pqItem)))
      {
        dc.displayValidationMsg(validationFailMsg(puFullType), pqItem);
        return false;
      }
    }
    return true;
  }
  return false;
}

bool
SolutionPattern::createPUPath(SoloPUPathCollector& puPathCollector,
                              FareMarketPath* fmPath,
                              PUPath* puPath) const
{
  if (puPath && fmPath)
  {
    for (const PUFullTypePtr puFullType : _puTypeVector)
    {
      if (LIKELY(!puFullType->addPU(puPathCollector, fmPath, puPath)))
        return false;
    }
    return true;
  }
  return false;
}
} /* namespace shpq */
} /* namespace tse */
