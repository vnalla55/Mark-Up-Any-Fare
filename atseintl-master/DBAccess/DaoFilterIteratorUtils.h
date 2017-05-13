//----------------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DaoPredicates.h"
#include "Util/IteratorRange.h"

#include <boost/iterator/filter_iterator.hpp>
#include <tuple>
#include <type_traits>

namespace tse
{

template <typename T, typename ContextType>
class DaoPredicateWrapper
{
public:
  using PredicateImpl = bool (*)(const T* rec, const ContextType& context);

  DaoPredicateWrapper(PredicateImpl predicateImpl, ContextType&& context)
    : _predicateImpl(predicateImpl), _context(std::move(context))
  {
  }

  DaoPredicateWrapper() = default;

  bool operator()(const T* rec) const
  {
    return (_predicateImpl) ? _predicateImpl(rec, _context) : true;
  }

private:
  PredicateImpl _predicateImpl = nullptr;
  ContextType _context;
};

template <typename PointersContainer, typename PredicateContext, typename Predicate>
auto makeFilterIteratorRange(PointersContainer* chachePtr, PredicateContext context, Predicate pred)
{
  DaoPredicateWrapper<std::remove_pointer_t<typename PointersContainer::value_type>,
                      PredicateContext> wrapperPredicate(pred, std::move(context));
  static PointersContainer emptyVecPtr;

  if (chachePtr == nullptr)
    chachePtr = &emptyVecPtr;

  using FilterItType = boost::filter_iterator<decltype(wrapperPredicate),
                                              typename PointersContainer::const_iterator>;
  FilterItType begin(wrapperPredicate, chachePtr->cbegin(), chachePtr->cend());
  FilterItType end(wrapperPredicate, chachePtr->cend(), chachePtr->cend());
  return makeIteratorRange(begin, end);
}

template <typename ElemType, typename ContextType>
using FilterItRange =
    IteratorRange<boost::filter_iterator<DaoPredicateWrapper<ElemType, ContextType>,
                                         typename std::vector<ElemType*>::const_iterator>>;
}

