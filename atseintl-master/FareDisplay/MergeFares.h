//-------------------------------------------------------------------
//
//  File:        MergeFares.h
//  Authors:     Jeff Hoffman
//
//  Description: creates final list of PaxTypeFares to be used by FareDisplayTrx
//
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

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <vector>

namespace tse
{
class FareMarket;
class PaxType;
class PaxTypeFare;
class FareDisplayTrx;
class PaxTypeBucket;

//-----------------------------------------------------------------
// tse::EqualBy unary predicate to find out the duplicate PaxTypeFare
//-----------------------------------------------------------------

class EqualBy : public std::unary_function<PaxTypeFare*, bool>
{
public:
  // pFare = 0 when operator() is not needed (not used as functor)
  EqualBy(const PaxTypeFare* pFare = nullptr, FareDisplayTrx* trx = nullptr) : _pFare(pFare), _trx(trx) {};
  EqualBy(FareDisplayTrx* trx) : _pFare(nullptr), _trx(trx) {};

  virtual ~EqualBy() {}; // lint !e1509

  //    void init() { /* TODO if(!_trx) throw ??? */};
  bool areEqual(const PaxTypeFare* lhs, const PaxTypeFare* rhs) const;

  bool operator()(const PaxTypeFare* p) const;

  const RoutingNumber& validRtgNum(const PaxTypeFare* p) const;
  bool matchRoutingNumber(const PaxTypeFare* pFare, const PaxTypeFare* p) const;
  bool matchCorpID(const PaxTypeFare* pFare, const PaxTypeFare* p) const;
  bool matchCreateDate(const PaxTypeFare* p, const PaxTypeFare* q) const;
  bool matchFareClass(const PaxTypeFare* p, const PaxTypeFare* q) const;

  const PaxTypeFare* _pFare;
  // TODO: see if can remove fareBasis checks (so no longer need trx)
  FareDisplayTrx* _trx;
};

class ValidFD : public std::unary_function<PaxTypeFare*, bool>
{
public:
  bool operator()(const PaxTypeFare* p) const;
};
class NotValid : public std::unary_function<PaxTypeFare*, bool>
{
public:
  bool operator()(const PaxTypeFare* p) const;
};

class MergeFares
{
public:
  MergeFares(FareDisplayTrx& trx) : _trx(trx) {};

  virtual ~MergeFares() {};

  void markZZ(std::vector<FareMarket*>& fareMarkets, bool isDomestic);
  static void markZZinMkt(FareMarket* fm);
  static void markZZinPTF(PaxTypeFare* p);

  void markDupe();

  static void markDupeInPTF(PaxTypeFare* dummy, PaxTypeFare* p);

  void mergeAllBuckets();
  bool isChildOrInfant(PaxType* pt);
  bool canPrice(PaxTypeFare* ptf);
  bool canMerge(PaxTypeBucket* bucket1, PaxTypeBucket* bucket2);
  static void doMergeInPTF(PaxTypeFare* ptf1, PaxTypeFare* ptf2);

  void forAllMatches(std::vector<PaxTypeFare*>& fares1,
                     std::vector<PaxTypeFare*>& fares2,
                     void (*funct)(PaxTypeFare* ptf1, PaxTypeFare* ptf2));

  void copyResults(std::vector<PaxTypeFare*>& fares);

  void validFaresToWorkarea(FareMarket* fm, bool isDomestic);
  void run(std::vector<FareMarket*>& fareMarket, bool copyAllFares, bool isDomestic);

private:
  FareDisplayTrx& _trx;
  std::vector<PaxTypeBucket*> _buckets;
};
} // tse namespace

