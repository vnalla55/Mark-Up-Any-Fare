// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         25-10-2011
//! \file         DiagSoloPQFilter.cpp
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

#include "Pricing/Shopping/PQ/DiagSoloPQFilter.h"

#include "Common/ErrorResponseException.h"

#include <boost/tokenizer.hpp>

namespace tse
{
namespace shpq
{

const std::string
DiagSoloPQFilter::PARAM_LEVEL("LV");
const std::string
DiagSoloPQFilter::PARAM_SOLUTION_PATTERNS("SP");
const std::string
DiagSoloPQFilter::PARAM_ALLDIAG_SOLUTION_PATTERNS("PS");
const std::string
DiagSoloPQFilter::PARAM_SOLUTION_IDS("ID");
const std::string
DiagSoloPQFilter::PARAM_VIEW("VW");
const std::string
DiagSoloPQFilter::PARAM_NUMBER_OF_EXPANSIONS("NUMBER_OF_EXPANSIONS"); // temporary

namespace
{
template <typename T>
struct ElemParser
{
  T operator()(const std::string& arg) const;
};

template <>
struct ElemParser<SoloPQItem::SoloPQItemLevel>
{
  SoloPQItem::SoloPQItemLevel operator()(const std::string& arg)
  {
    if (arg == "SP")
      return SoloPQItem::SP_LEVEL;
    if (arg == "CR")
      return SoloPQItem::CR_LEVEL;
    if (arg == "CRC")
      return SoloPQItem::CRC_LEVEL;
    if (arg == "FMP")
      return SoloPQItem::FMP_LEVEL;
    if (arg == "FPF")
      return SoloPQItem::FPF_LEVEL;

    VALIDATE_OR_THROW(false, INVALID_INPUT, "Invalid value of level parameter");

    return SoloPQItem::SP_LEVEL; // not reached - just to make the compilr happy;
  }
};

template <>
struct ElemParser<SolutionPattern::SPNumberType>
{
  SolutionPattern::SPNumberType operator()(const std::string& arg)
  {
    uint16_t spNo = 0u;
    try
    {
      spNo = boost::lexical_cast<uint16_t>(arg);
      const SolutionPattern& sp = SolutionPatternStorage::instance().getSPByNumber(spNo);
      spNo = sp.getSPNumber();
    }
    catch (const std::exception& ex)
    {
      // ignored
    }
    VALIDATE_OR_THROW(spNo != 0u, INVALID_INPUT, "Invalid value of spNumber");
    return spNo;
  }
};

template <>
struct ElemParser<size_t>
{
  size_t operator()(const std::string& arg)
  {
    size_t val = 0u;
    try { val = boost::lexical_cast<size_t>(arg); }
    catch (const std::exception& ex)
    {
      // ignored
    }
    VALIDATE_OR_THROW(val != 0u, INVALID_INPUT, "Invalid value of spId");
    return val;
  }
};
} // anon namespace

template <typename T>
DiagSoloPQFilter::ParamSet<T>::ParamSet()
  : _inactive(true)
{
}

template <typename T>
void
DiagSoloPQFilter::ParamSet<T>::initialize(const std::string& arg)
{
  if (arg.empty())
    return;

  // arg can be
  // ARG1*-ARG2*+ARG3*ARG5*ARG6*-ARG7  (or ARG1XMARG2XARG3XARG5XARG6XMARG7)
  // `*' or 'X' is a separator
  // `-' or 'M' means exclude argument
  // `+' means include argument, `+' is default, may be skipped
  // The 'X' and 'M' are meant for green screen users.

  typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
  const boost::char_separator<char> optionsSeparator("*X");
  Tokenizer tokenizer(arg, optionsSeparator);

  for (const std::string& token : tokenizer)
  {
    if (token.empty()) // shouldn't happen, boost::tokenizer skips empty tokens
      continue;

    SetT& set(token[0] == '-' || token[0] == 'M' ? _excluded : _included);
    const std::string& tokArg(
        (token[0] == '-' || token[0] == '+' || token[0] == 'M') ? token.substr(1) : token);

    VALIDATE_OR_THROW(!tokArg.empty(), INVALID_INPUT, "Missing argument for -, M or + operand");

    set.insert(ElemParser<T>()(tokArg));
  }

  _inactive = _included.empty() && _excluded.empty();
}

template <typename T>
bool
DiagSoloPQFilter::ParamSet<T>::isFilteredOut(const T& param) const
{
  if (LIKELY(_inactive))
    return false;

  if (_excluded.find(param) != _excluded.end())
    return true;

  if (_included.empty())
    return false;

  return _included.find(param) == _included.end();
}

template <typename T>
void
DiagSoloPQFilter::ParamSet<T>::addToFilterByParent(const T& param, const T& parent)
{
  if (_inactive)
    return;

  if (_excluded.find(parent) != _excluded.end())
    _excluded.insert(param);

  if (_included.find(parent) != _included.end())
    _included.insert(param);
}

bool
DiagSoloPQFilter::isItemFilteredOut(const SoloPQItem* const item,
                                    const SoloPQItem::SoloPQItemLevel level,
                                    const size_t itemId) const
{

  if (_levels.isFilteredOut(level))
    return true;

  if (item && item->getSolPattern() &&
      _spNumbers.isFilteredOut(item->getSolPattern()->getSPNumber()))
    return true;

  if (itemId && _itemIds.isFilteredOut(itemId))
    return true;

  return false;
}

bool
DiagSoloPQFilter::isItemFilteredOut(const SoloPQItemPtr& item, const size_t itemId) const
{
  return isItemFilteredOut(item.get(), item->getLevel(), itemId);
}

bool
DiagSoloPQFilter::isItemFilteredOut(const SoloPQItem* const item, const size_t itemId) const
{
  return item && isItemFilteredOut(item, item->getLevel(), itemId);
}

bool
DiagSoloPQFilter::isFMPFilteredOut(const SoloPQItem* const parentItem, const size_t parentItemId)
    const
{

  return isItemFilteredOut(parentItem, SoloPQItem::FMP_LEVEL, parentItemId);
}

void
DiagSoloPQFilter::addIdRelationShip(const size_t itemId, const size_t parentItemId)
{
  if (parentItemId)
    _itemIds.addToFilterByParent(itemId, parentItemId);
}

void
DiagSoloPQFilter::initialize(const Diagnostic* const root,
                             const bool allDiagParamsOnly /* = false */)
{
  if (!root)
    return;

  if (allDiagParamsOnly)
  {
    _spNumbers.initialize(root->diagParamMapItem(PARAM_ALLDIAG_SOLUTION_PATTERNS));
    return;
  }

  _levels.initialize(root->diagParamMapItem(PARAM_LEVEL));
  _spNumbers.initialize(root->diagParamMapItem(PARAM_SOLUTION_PATTERNS));
  _itemIds.initialize(root->diagParamMapItem(PARAM_SOLUTION_IDS));
  initializeVerbosityLevel(root->diagParamMapItem(PARAM_VIEW));

  // TODO: temporary parameter
  if (!root->diagParamMapItem(PARAM_NUMBER_OF_EXPANSIONS).empty())
  {
    try
    {
      _noOfExpansions = boost::lexical_cast<size_t>(
          root->diagParamMapItem(PARAM_NUMBER_OF_EXPANSIONS)); // TODO: temporary for QAs
    }
    catch (...)
    {
      VALIDATE_OR_THROW(false, INVALID_INPUT, "Invalid value of " << PARAM_NUMBER_OF_EXPANSIONS);
    }
  }
}

void
DiagSoloPQFilter::initializeVerbosityLevel(const std::string& arg)
{
  _verbosityLevel = SoloPQItem::SVL_NORMAL;
  if (arg == "BARE")
    _verbosityLevel = SoloPQItem::SVL_BARE;
  else if (arg == "NORMAL")
    _verbosityLevel = SoloPQItem::SVL_NORMAL;
  else if (arg == "DETAILS")
    _verbosityLevel = SoloPQItem::SVL_DETAILS;
  else
  {
    VALIDATE_OR_THROW(arg.empty(), INVALID_INPUT, "Invalid view details level: " << arg);
  }
}

DiagSoloPQFilter::DiagSoloPQFilter()
  : _levels(), _spNumbers(), _verbosityLevel(SoloPQItem::SVL_NORMAL), _noOfExpansions(0)
{
}

} /* namespace shpq */
} /* namespace tse */
