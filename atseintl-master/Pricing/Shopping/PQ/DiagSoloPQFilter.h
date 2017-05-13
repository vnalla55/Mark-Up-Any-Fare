// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         25-10-2011
//! \file         DiagSoloPQFilter.h
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

#pragma once

#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"

namespace tse
{
class Diagnostic;
}

namespace tse
{
namespace shpq
{

class DiagSoloPQFilter
{

public:
  void initialize(const Diagnostic* const root, const bool allDiagParamsOnly = false);

  bool isItemFilteredOut(const SoloPQItemPtr& item, const size_t itemId) const;

  bool isItemFilteredOut(const SoloPQItem* const item, const size_t itemId) const;

  bool isFMPFilteredOut(const SoloPQItem* const parentItem, const size_t parentItemId) const;

  void addIdRelationShip(const size_t itemId, const size_t parentItemId);

  SoloPQItem::StrVerbosityLevel getVerbosityLevel() const { return _verbosityLevel; }

  size_t getNoOfExpansions() const { return _noOfExpansions; }

  DiagSoloPQFilter();

  static const std::string PARAM_LEVEL;
  static const std::string PARAM_SOLUTION_PATTERNS;
  static const std::string PARAM_ALLDIAG_SOLUTION_PATTERNS; // used for SoloPQ initialization
  static const std::string PARAM_SOLUTION_IDS;
  static const std::string PARAM_VIEW;
  static const std::string PARAM_NUMBER_OF_EXPANSIONS; // temporary

private:
  template <typename T>
  class ParamSet
  {
  public:
    ParamSet();
    void initialize(const std::string& arg);
    bool isFilteredOut(const T& param) const;
    void addToFilterByParent(const T& param, const T& parent);

  private:
    typedef typename std::set<T> SetT;
    SetT _included;
    SetT _excluded;
    bool _inactive;
  };

  void initializeVerbosityLevel(const std::string& arg);

  bool isItemFilteredOut(const SoloPQItem* const item,
                         const SoloPQItem::SoloPQItemLevel level,
                         const size_t itemId) const;

  ParamSet<SoloPQItem::SoloPQItemLevel> _levels;
  ParamSet<SolutionPattern::SPNumberType> _spNumbers;
  ParamSet<size_t> _itemIds;
  SoloPQItem::StrVerbosityLevel _verbosityLevel;
  size_t _noOfExpansions; // TODO: Temporary code, remove it
};

} /* namespace shpq */
} /* namespace tse */
