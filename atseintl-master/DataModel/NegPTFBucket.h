// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include "Common/Assert.h"
#include "DataModel/NegPaxTypeFareData.h"

#include <set>

namespace tse
{

struct NegPaxTypeFareData;
class PaxTypeFare;

class NegPTFBucket
{
  friend class NegPTFBucketTest;

public:
  typedef std::vector<NegPaxTypeFareData> R3NegFares;
  typedef std::vector<R3NegFares>::iterator iterator;
  typedef std::vector<R3NegFares>::const_iterator const_iterator;

  NegPTFBucket();
  void insert(const NegPaxTypeFareData& fareData);
  size_t size() const;
  iterator begin();
  iterator end();
  const R3NegFares& operator[](const size_t& i) const;
  bool empty() const { return _data.empty(); }
  const NegPaxTypeFareData& getRepresentative() const;
  void addCxrToTypeFare(NegPaxTypeFareData& negPtfData, const NegPaxTypeFareData& fareData);

  template <typename Validator>
  void collectValidNegFares(Validator validator, std::set<PaxTypeFare*>& result) const;

private:
  NegPaxTypeFareData* _representative;
  std::vector<R3NegFares> _data;
};

template <typename Validator>
void
NegPTFBucket::collectValidNegFares(Validator validator, std::set<PaxTypeFare*>& result) const
{
  result.clear();
  for (const R3NegFares& fares : _data)
  {
    if (!fares.empty() && validator(fares[0].ptf))
    {
      for (const NegPaxTypeFareData& negPtfData : fares)
        result.insert(negPtfData.ptf);

      return;
    }
  }
}
}

