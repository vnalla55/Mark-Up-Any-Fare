//-------------------------------------------------------------------
// File:    FarePathCollector.h
// Created: January 2006
// Authors: Andrew Ahmad
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

#include <cstdint>
#include <vector>

namespace tse
{
class PaxType;
class FarePath;
class AltPricingTrx;
class DiagCollector;
class PaxFarePathFactory;
class PaxFarePathFactoryBase;
class FarePathFactory;
class PUPathMatrix;
class DiagCollector;
class NoPNRPricingTrx;

struct Diag611Info
{
  int dupResCnt = 0;
  int noDupResCnt = 0;
  bool validFarePathFoundForPaxType = false;

  PaxFarePathFactoryBase* pfpf = nullptr;
  std::vector<FarePath*> farePaths;
};

class FarePathCollector final
{
public:
  FarePathCollector() = default;
  FarePathCollector(const FarePathCollector&) = delete;
  FarePathCollector& operator=(const FarePathCollector&) = delete;

  bool process(const std::vector<PaxFarePathFactory*>& paxFarePathFactoryBucket,
               AltPricingTrx& trx,
               PUPathMatrix& puMatrix,
               DiagCollector& diag);

  bool enoughOptions(AltPricingTrx& trx,
                     std::vector<FarePath*>& path,
                     std::vector<bool>& paxOptions,
                     uint16_t i);

private:
  void build611Diagnostic(AltPricingTrx& trx,
                          std::vector<Diag611Info>& diagInfoVec,
                          DiagCollector& diag);

  void printNoPNRDiag(NoPNRPricingTrx& trx, DiagCollector& diag, Diag611Info& diagInfo);

  bool _alreadyReadFCC = false;
  bool _allowDuplicateTotals = false;
  uint32_t _farePathMaxCount = 0;
};
} // tse namespace
