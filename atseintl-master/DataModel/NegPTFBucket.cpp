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

#include "DataModel/NegPTFBucket.h"

#include "DataModel/NegPaxTypeFareData.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{

NegPTFBucket::NegPTFBucket() : _representative(nullptr) {}

void
NegPTFBucket::insert(const NegPaxTypeFareData& fareData)
{
  if (!_data.empty())
  {
    TSE_ASSERT(_representative);
    TSE_ASSERT(!_data.back().empty());

    if (_data.back().back().catRuleItemInfo == fareData.catRuleItemInfo)
    {
      _data.back().push_back(fareData);
      _representative = &_data.back().back();
      return;
    }
  }
  _data.push_back(R3NegFares());
  _data.back().push_back(fareData);
  _representative = &_data.back().back();
}

size_t
NegPTFBucket::size() const
{
  return _data.size();
}

NegPTFBucket::iterator
NegPTFBucket::begin()
{
  return _data.begin();
}

NegPTFBucket::iterator
NegPTFBucket::end()
{
  return _data.end();
}

const NegPTFBucket::R3NegFares&
NegPTFBucket::
operator[](const size_t& i) const
{
  return _data[i];
}

const NegPaxTypeFareData&
NegPTFBucket::getRepresentative() const
{
  TSE_ASSERT(_representative);
  return *_representative;
}
void
NegPTFBucket::addCxrToTypeFare(NegPaxTypeFareData& negPtfData, const NegPaxTypeFareData& fareData)
{
    negPtfData.ptf->validatingCarriers().push_back(fareData.validatingCxr);
}

}
