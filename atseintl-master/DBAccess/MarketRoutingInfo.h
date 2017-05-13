//----------------------------------------------------------------------------
//
//  File:           MarketRoutingInfo.h
//  Created:        09 Jul 2008
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Global.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Cache.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/HashKey.h"

#include <vector>

#include <tr1/unordered_map>

namespace tse
{

class MarketRoutingDoubles
{

public:
  MarketRoutingDoubles(const LocCode& market1, const LocCode& market2)
    : _market1(market1), _market2(market2)
  {
  }
  MarketRoutingDoubles() {}
  MarketRoutingDoubles(const MarketRoutingDoubles& pair)
    : _market1(pair._market1), _market2(pair._market2)
  {
  }
  ~MarketRoutingDoubles() {}
  MarketRoutingDoubles& operator=(const MarketRoutingDoubles& pair)
  {
    if (this != &pair)
    {
      this->_market1 = pair._market1;
      this->_market2 = pair._market2;
    }
    return *this;
  }

  bool operator==(const MarketRoutingDoubles& pair) const
  {
    if (this->_market1 != pair._market1)
    {
      return false;
    }

    if (this->_market2 != pair._market2)
    {
      return false;
    }

    return true;
  }

  static void dummyData(MarketRoutingDoubles& obj)
  {
    obj._market1 = "aaaaaaaa";
    obj._market2 = "bbbbbbbb";
  }

  const LocCode& market1() const { return _market1; }
  LocCode& market1() { return _market1; }

  const LocCode& market2() const { return _market2; }
  LocCode& market2() { return _market2; }

private:
  LocCode _market1;
  LocCode _market2;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _market1);
    FLATTENIZE(archive, _market2);
  }

private:
};

typedef std::vector<LocCode> MarketRoutingSinglesVec;
typedef std::vector<MarketRoutingDoubles> MarketRoutingDoublesVec;

class MarketRoutingInfo
{

public:
  MarketRoutingInfo() : _singles(nullptr), _doubles(nullptr) {}

  virtual ~MarketRoutingInfo() {}

  const MarketRoutingSinglesVec* singles() const { return _singles; }
  const MarketRoutingSinglesVec*& singles() { return _singles; }

  const MarketRoutingDoublesVec* doubles() const { return _doubles; }
  const MarketRoutingDoublesVec*& doubles() { return _doubles; }

  virtual bool operator==(const MarketRoutingInfo& rhs) const
  {
    return ((*_singles == *rhs._singles) && (*_doubles == *rhs._doubles));
  }

private:
  const MarketRoutingSinglesVec* _singles;
  const MarketRoutingDoublesVec* _doubles;
};

class MarketRoutingIndexInfo
{
public:
  MarketRoutingIndexInfo() : _singlesIndex(0), _doublesIndex(0) {}
  virtual ~MarketRoutingIndexInfo() {}
  MarketRoutingIndexInfo(size_t singles, size_t doubles)
    : _singlesIndex(singles), _doublesIndex(doubles)
  {
  }

  size_t singlesIndex() const { return _singlesIndex; }
  size_t& singlesIndex() { return _singlesIndex; }

  size_t doublesIndex() const { return _doublesIndex; }
  size_t& doublesIndex() { return _doublesIndex; }

  virtual bool operator==(const MarketRoutingIndexInfo& rhs) const
  {
    return ((_singlesIndex == rhs._singlesIndex) && (_doublesIndex == rhs._doublesIndex));
  }

  static void dummyData(MarketRoutingIndexInfo& obj)
  {
    obj._singlesIndex = 1;
    obj._doublesIndex = 2;
  }

private:
  size_t _singlesIndex;
  size_t _doublesIndex;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _singlesIndex);
    FLATTENIZE(archive, _doublesIndex);
  }

private:
};

typedef HashKey<LocCode, LocCode> MarketRoutingIndexKey;

class MarketRouting
{
public:
  struct hash_func
  {
    size_t operator()(const MarketRoutingIndexKey& key) const
    {
      tse::Hasher hasher(tse::Global::hasherMethod());
      hasher << key;
      return hasher.hash();
    }
  };

  typedef std::tr1::unordered_map<MarketRoutingIndexKey, MarketRoutingIndexInfo, hash_func> Map;

  MarketRouting() : _generationNumber(-1) {}

  ~MarketRouting()
  {
    std::vector<MarketRoutingSinglesVec*>::iterator sitr;
    for (sitr = _singles.begin(); sitr < _singles.end(); sitr++)
    {
      delete *sitr;
    }
    std::vector<MarketRoutingDoublesVec*>::iterator ditr;
    for (ditr = _doubles.begin(); ditr < _doubles.end(); ditr++)
    {
      delete *ditr;
    }
  }

  void addIndexInfo(const MarketRoutingIndexKey& indexKey, size_t singles, size_t doubles)
  {
    MarketRoutingIndexInfo indexInfo(singles, doubles);
    _marketIndexMap[indexKey] = indexInfo;
  }

  void addSingles(const std::vector<MarketRoutingSinglesVec*>& singles)
  {
    _singles.resize(singles.size());
    std::copy(singles.begin(), singles.end(), _singles.begin());
  }

  void addDoubles(const std::vector<MarketRoutingDoublesVec*>& doubles)
  {
    _doubles.resize(doubles.size());
    std::copy(doubles.begin(), doubles.end(), _doubles.begin());
  }

  MarketRoutingIndexInfo getIndexInfo(const LocCode& market1, const LocCode& market2) const
  {

    MarketRoutingIndexKey key(market1, market2);
    Map::const_iterator i = _marketIndexMap.find(key);
    if (i != _marketIndexMap.end())
    {
      return i->second;
    }
    return MarketRoutingIndexInfo();
  }

  const MarketRoutingSinglesVec* getSingles(size_t index) const
  {
    return (index == 0 || index >= _singles.size() ? nullptr : _singles[index]);
  }
  const MarketRoutingDoublesVec* getDoubles(size_t index) const
  {
    return (index == 0 || index >= _doubles.size() ? nullptr : _doubles[index]);
  }

  int32_t& generationNumber() { return _generationNumber; }
  const int32_t& generationNumber() const { return _generationNumber; }

  bool operator==(const MarketRouting& rhs) const
  {
    bool eq =
        ((_marketIndexMap.size() == rhs._marketIndexMap.size()) &&
         (_singles.size() == rhs._singles.size()) && (_doubles.size() == rhs._doubles.size()) &&
         (_generationNumber == rhs._generationNumber));

    if (eq)
    {
      Map::const_iterator itMine = _marketIndexMap.begin();
      Map::const_iterator itOther = rhs._marketIndexMap.begin();

      while (eq && (itMine != _marketIndexMap.end()) && (itOther != rhs._marketIndexMap.end()))
      {
        eq = (((*itMine).first == (*itOther).first) && ((*itMine).second == (*itOther).second));
        ++itMine;
        ++itOther;
      }
    }

    for (size_t i = 0; (eq && (i < _singles.size())); ++i)
    {
      eq = (*(_singles[i]) == *(rhs._singles[i]));
    }

    for (size_t j = 0; (eq && (j < _doubles.size())); ++j)
    {
      eq = (*(_doubles[j]) == *(rhs._doubles[j]));
    }

    return eq;
  }

  static void dummyData(MarketRouting& obj)
  {
    MarketRoutingIndexKey mrik1("aaaaaaaa", "bbbbbbbb");
    MarketRoutingIndexInfo mrii1;
    MarketRoutingIndexInfo::dummyData(mrii1);

    MarketRoutingIndexKey mrik2("cccccccc", "dddddddd");
    MarketRoutingIndexInfo mrii2;
    MarketRoutingIndexInfo::dummyData(mrii2);

    obj._marketIndexMap[mrik1] = mrii1;
    obj._marketIndexMap[mrik2] = mrii2;

    MarketRoutingSinglesVec* mrsv1 = new MarketRoutingSinglesVec;
    MarketRoutingSinglesVec* mrsv2 = new MarketRoutingSinglesVec;

    mrsv1->push_back("eeeeeeee");
    mrsv1->push_back("ffffffff");
    mrsv2->push_back("gggggggg");
    mrsv2->push_back("hhhhhhhh");

    obj._singles.push_back(mrsv1);
    obj._singles.push_back(mrsv2);

    MarketRoutingDoublesVec* mrdv1 = new MarketRoutingDoublesVec;
    MarketRoutingDoublesVec* mrdv2 = new MarketRoutingDoublesVec;

    MarketRoutingDoubles mrd1;
    MarketRoutingDoubles::dummyData(mrd1);
    MarketRoutingDoubles mrd2;
    MarketRoutingDoubles::dummyData(mrd2);
    MarketRoutingDoubles mrd3;
    MarketRoutingDoubles::dummyData(mrd3);
    MarketRoutingDoubles mrd4;
    MarketRoutingDoubles::dummyData(mrd4);

    mrdv1->push_back(mrd1);
    mrdv1->push_back(mrd2);
    mrdv2->push_back(mrd3);
    mrdv2->push_back(mrd4);

    obj._doubles.push_back(mrdv1);
    obj._doubles.push_back(mrdv2);
  }

private:
  Map _marketIndexMap;
  std::vector<MarketRoutingSinglesVec*> _singles;
  std::vector<MarketRoutingDoublesVec*> _doubles;
  int32_t _generationNumber;

  typedef std::multimap<tse::MarketRoutingIndexKey,
                        tse::MarketRoutingIndexInfo,
                        std::less<tse::MarketRoutingIndexKey> > STDMULTI;

  // Not sure why, but boost had a real problem with XML archive deserialization of _marketIndexMap
  // until I converted it to a std::multimap first.

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _marketIndexMap);
    FLATTENIZE(archive, _singles);
    FLATTENIZE(archive, _doubles);
    FLATTENIZE(archive, _generationNumber);
  }

private:
};

typedef HashKey<VendorCode, TariffNumber, CarrierCode, RoutingNumber> MarketRoutingKey;
typedef sfc::Cache<MarketRoutingKey, MarketRouting> MarketRoutingCache;

} // end namespace

