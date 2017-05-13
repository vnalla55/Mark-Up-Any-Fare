// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#ifndef UTIL_FLAT_MAP_H
#define UTIL_FLAT_MAP_H

#include "Util/FlatBase.h"
#include "Util/Vector.h"

namespace tse
{
template <typename KeyType, typename MappedType, typename Compare, typename Container>
class FlatMap : public detail::FlatBase<KeyType, MappedType, Compare, Container, true>
{
  typedef detail::FlatBase<KeyType, MappedType, Compare, Container, true> Base;

public:
  typedef typename Base::key_type key_type;
  typedef typename Base::mapped_type mapped_type;
  typedef typename Base::value_type value_type;
  typedef typename Base::key_compare key_compare;
  typedef typename Base::value_compare value_compare;
  typedef typename Base::container_type container_type;
  typedef typename Base::allocator_type allocator_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;
  typedef typename Base::pointer pointer;
  typedef typename Base::const_pointer const_pointer;
  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::reverse_iterator reverse_iterator;
  typedef typename Base::const_reverse_iterator const_reverse_iterator;
  typedef typename Base::difference_type difference_type;
  typedef typename Base::size_type size_type;

  using Base::Base;

  using Base::operator[];
  using Base::at;
};

template <typename KeyType, typename MappedType, typename Compare, typename Container>
class FlatMultiMap : public detail::FlatBase<KeyType, MappedType, Compare, Container, false>
{
  typedef detail::FlatBase<KeyType, MappedType, Compare, Container, false> Base;

public:
  typedef typename Base::key_type key_type;
  typedef typename Base::mapped_type mapped_type;
  typedef typename Base::value_type value_type;
  typedef typename Base::key_compare key_compare;
  typedef typename Base::value_compare value_compare;
  typedef typename Base::container_type container_type;
  typedef typename Base::allocator_type allocator_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;
  typedef typename Base::pointer pointer;
  typedef typename Base::const_pointer const_pointer;
  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::reverse_iterator reverse_iterator;
  typedef typename Base::const_reverse_iterator const_reverse_iterator;
  typedef typename Base::difference_type difference_type;
  typedef typename Base::size_type size_type;

  using Base::Base;
};
}

#endif
