//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include <map>
#include <ostream>
#include <set>
#include <vector>

#include <tr1/unordered_map>
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
#include <unordered_map>
#endif

namespace tse
{
//
// Forward declarations
//

template <typename I>
inline bool
objectsIdentical(const std::vector<I*>& vec1, const std::vector<I*>& vec2);

template <typename K, typename I>
inline bool
objectsIdentical(const std::map<K, I*>& map1, const std::map<K, I*>& map2);

template <typename A, typename B, typename C, typename D>
inline bool
objectsIdentical(const std::tr1::unordered_multimap<const A, B*, C, D>& mm1,
                 const std::tr1::unordered_multimap<const A, B*, C, D>& mm2);

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

template <typename A, typename B, typename C, typename D>
inline bool
objectsIdentical(const std::unordered_map<A, std::vector<B*>, C, D>& mm1,
                 const std::unordered_map<A, std::vector<B*>, C, D>& mm2);

#endif

template <typename A, typename B>
inline bool
objectsIdentical(const std::multimap<A, B*>& mm1, const std::multimap<A, B*>& mm2);

template <typename DEFAULT>
inline bool
objectsIdentical(const DEFAULT& a1, const DEFAULT& a2);

template <typename I>
inline std::ostream&
dumpObject(std::ostream& os, const std::vector<I*>& vec);

template <typename K, typename I>
inline std::ostream&
dumpObject(std::ostream& os, const std::map<K, I*>& map);

template <typename A, typename B, typename C, typename D>
inline std::ostream&
dumpObject(std::ostream& os, const std::tr1::unordered_multimap<const A, B*, C, D>& map);

template <typename A, typename B>
inline std::ostream&
dumpObject(std::ostream& os, const std::multimap<A, B*>& map);

template <typename DEFAULT>
inline std::ostream&
dumpObject(std::ostream& os, const DEFAULT& a);

//
// Object Comparators
//

template <typename I>
inline bool
objectsIdentical(const std::vector<I*>& vec1, const std::vector<I*>& vec2)
{
  bool eq(vec1.size() == vec2.size());

  typename std::vector<I*>::const_iterator it1 = vec1.begin();
  typename std::vector<I*>::const_iterator it2 = vec2.begin();

  while (eq && (it1 != vec1.end()) && (it2 != vec2.end()))
  {
    eq = (*(*it1) == *(*it2));
    ++it1;
    ++it2;
  }

  return eq;
}

template <typename K, typename I>
inline bool
objectsIdentical(const std::map<K, I*>& map1, const std::map<K, I*>& map2)
{
  bool eq(map1.size() == map2.size());

  typename std::map<K, I*>::const_iterator it1 = map1.begin();
  typename std::map<K, I*>::const_iterator it2 = map2.begin();

  while (eq && (it1 != map1.end()) && (it2 != map2.end()))
  {
    eq = (((*it1).first == (*it2).first) && (*(*it1).second == *(*it2).second));
    ++it1;
    ++it2;
  }

  return eq;
}

template <typename B>
struct DereferencingComparator
{
  bool operator()(const B* lhs, const B* rhs) const { return (*lhs < *rhs); }
};

template <typename A, typename B, typename MM>
inline bool
reorg_and_compare_multimaps(const MM& mm1, const MM& mm2)
{
  bool eq(mm1.size() == mm2.size());

  if (eq)
  {
    typedef std::set<B*, DereferencingComparator<B> > BSET;
    typedef std::map<A, BSET, std::less<A> > REORG;

    REORG r1;
    REORG r2;

    typename MM::const_iterator it1 = mm1.begin();
    typename MM::const_iterator it2 = mm2.begin();

    while ((it1 != mm1.end()) && (it2 != mm2.end()))
    {
      BSET& sref1 = r1[(*it1).first];
      BSET& sref2 = r2[(*it2).first];

      sref1.insert((*it1).second);
      sref2.insert((*it2).second);

      ++it1;
      ++it2;
    }

    typename REORG::const_iterator ir1 = r1.begin();
    typename REORG::const_iterator ir2 = r2.begin();

    while (eq && (ir1 != r1.end()) && (ir2 != r2.end()))
    {
      eq = (eq && ((*ir1).first == (*ir2).first));

      if (eq)
      {
        const BSET& bs1 = (*ir1).second;
        const BSET& bs2 = (*ir2).second;

        eq = (eq && (bs1.size() == bs2.size()));

        if (eq)
        {
          typename BSET::const_iterator ib1 = bs1.begin();
          typename BSET::const_iterator ib2 = bs2.begin();

          while (eq && (ib1 != bs1.end()) && (ib2 != bs2.end()))
          {
            eq = (eq && (*(*ib1) == *(*ib2)));
            if (eq)
            {
              ++ib1;
              ++ib2;
            }
          }
        }
      }

      ++ir1;
      ++ir2;
    }
  }

  return eq;
}

template <typename A, typename B>
inline bool
objectsIdentical(const std::multimap<A, B*>& mm1, const std::multimap<A, B*>& mm2)
{
  return reorg_and_compare_multimaps<A, B, std::multimap<A, B*> >(mm1, mm2);
}

template <typename A, typename B, typename C, typename D>
inline bool
objectsIdentical(const std::tr1::unordered_multimap<const A, B*, C, D>& mm1,
                 const std::tr1::unordered_multimap<const A, B*, C, D>& mm2)
{
  return reorg_and_compare_multimaps<A, B, std::tr1::unordered_multimap<const A, B*, C, D> >(mm1,
                                                                                             mm2);
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

template <typename A, typename B, typename C, typename D>
inline bool
objectsIdentical(const std::unordered_map<A, std::vector<B*>, C, D>& mm1,
                 const std::unordered_map<A, std::vector<B*>, C, D>& mm2)
{
  if (mm1.size() != mm2.size())
    return false;

  typedef std::map<A, std::vector<B*> > ORDER;
  ORDER o1(mm1.begin(), mm1.end()), o2(mm2.begin(), mm2.end());

  typename ORDER::const_iterator itr1(o1.begin()), itr2(o2.begin());
  for ( ; itr1 != o1.end() && itr2 != o2.end(); ++itr1, ++itr2)
  {
    if (!(itr1->first == itr2->first) || !objectsIdentical(itr1->second, itr2->second))
      return false;
  }
  return true;
}

#endif

template <typename DEFAULT>
inline bool
objectsIdentical(const DEFAULT& a1, const DEFAULT& a2)
{
  return (a1 == a2);
}

//
// Object Dumpers
//

template <typename I>
inline std::ostream&
dumpObject(std::ostream& os, const std::vector<I*>& vec)
{
  for (typename std::vector<I*>::const_iterator it = vec.begin(); it != vec.end(); ++it)
  {
    dumpObject(os, *(*it));
  }
  return os;
}

template <typename K, typename I>
inline std::ostream&
dumpObject(std::ostream& os, const std::map<K, I*>& map)
{
  for (typename std::map<K, I*>::const_iterator it = map.begin(); it != map.end(); ++it)
  {
    dumpObject(os, (*it).first);
    os << "=";
    dumpObject(os, *((*it).second));
  }
  return os;
}

template <typename A, typename B, typename C, typename D>
inline std::ostream&
dumpObject(std::ostream& os, const std::tr1::unordered_multimap<const A, B*, C, D>& map)
{
  for (const auto& elem : map)
  {
    dumpObject(os, elem.first);
    os << "=";
    dumpObject(os, *elem.second);
  }
  return os;
}

template <typename A, typename B>
inline std::ostream&
dumpObject(std::ostream& os, const std::multimap<A, B*>& map)
{
  for (typename std::multimap<A, B*>::const_iterator it = map.begin(); it != map.end(); ++it)
  {
    dumpObject(os, (*it).first);
    os << "=";
    dumpObject(os, *((*it).second));
  }
  return os;
}

template <typename DEFAULT>
inline std::ostream&
dumpObject(std::ostream& os, const DEFAULT& a)
{
  //
  // if you get here, then you haven't specialized your info object
  // with a 'dumpObject' function.  For an example, see NegFareSecurityInfo.h.
  //
  os << "[OBJECT NOT DUMPABLE]";
  return os;
}

} // namespace tse

