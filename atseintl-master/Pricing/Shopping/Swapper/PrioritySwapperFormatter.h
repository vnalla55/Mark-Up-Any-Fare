//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Common/Assert.h"
#include "Pricing/Shopping/Swapper/PriorityScoreBuilder.h"
#include "Pricing/Shopping/Swapper/Swapper.h"
#include "Pricing/Shopping/Utils/StreamFormat.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace tse
{

namespace swp
{

template <typename ItemType>
class PriorityAppraiserSorter
{
public:
  typedef Swapper<ItemType, PriorityScoreBuilder> Swp;
  typedef std::vector<const typename Swp::Appraiser*> Appraisers;

  PriorityAppraiserSorter(const Swp& swapper) : _swapper(swapper) {}

  class PriorityComp
  {
  public:
    PriorityComp(const Swp& swapper) : _swapper(swapper) {}
    bool operator()(const typename Swp::Appraiser* left, const typename Swp::Appraiser* right)
    {
      return _swapper.getDerivedInfoForAppraiser(left) < _swapper.getDerivedInfoForAppraiser(right);
    }

  private:
    const Swp& _swapper;
  };

  Appraisers operator()(const typename Swp::AppraiserScoresMap& m)
  {
    Appraisers v;
    for (typename Swp::AppraiserScoresMap::const_iterator it = m.begin(); it != m.end(); ++it)
    {
      v.push_back(it->first);
    }
    std::sort(v.begin(), v.end(), PriorityComp(_swapper));
    return v;
  }

private:
  const Swp& _swapper;
};

template <typename ItemType>
class PriorityScoreFormatter
{
public:
  typedef Swapper<ItemType, PriorityScoreBuilder> Swp;
  typedef std::vector<const typename Swp::Appraiser*> Appraisers;

  PriorityScoreFormatter(const Swp& swapper) : _swapper(swapper) {}

  std::string formatScores(const typename Swp::AppraiserScoresMap& m)
  {
    PriorityAppraiserSorter<ItemType> pas(_swapper);
    Appraisers sorted = pas(m);
    std::ostringstream out;
    for (size_t i = 0; i < sorted.size(); ++i)
    {
      out << m.find(sorted[i])->second;
      if (i != (sorted.size() - 1))
      {
        out << " ";
      }
    }
    return out.str();
  }

private:
  const Swp& _swapper;
};

// Displays contents of a Swapper using PriorityScoreBuilder
template <typename ItemType>
class PrioritySwapperFormatter
{
public:
  typedef Swapper<ItemType, PriorityScoreBuilder> Swp;

  PrioritySwapperFormatter(unsigned int appraiserIndent = 2)
    : _appraiserIndent(appraiserIndent), _maxItemWidth(0)
  {
  }

  static void format(std::ostream& out, const Swp& swp)
  {
    PrioritySwapperFormatter<ItemType> psf;
    psf.dumpAll(out, swp);
  }

private:
  typedef std::vector<const typename Swp::Appraiser*> Appraisers;
  typedef std::map<unsigned int, Appraisers> RankMap;
  typedef std::vector<std::string> ScoredItem;
  typedef std::vector<ScoredItem> ScoredItems;

  void dumpAll(std::ostream& out, const Swp& swp)
  {
    const RankMap rankMap = buildRankMap(swp);
    const std::vector<std::string> builderLines = fillBuilderLines(swp, rankMap);
    const ScoredItems scoredItems = fillScoredItems(swp);
    dumpHeader(out, swp);
    dumpBuilder(out, builderLines);
    dumpTableHeader(out, swp);
    dumpItems(out, scoredItems);
  }

  // rank -> [appraisers]
  RankMap buildRankMap(const Swp& swp)
  {
    RankMap rankMap;

    const std::vector<const typename Swp::Appraiser*>& appraisers = swp.getAppraisers();
    for (unsigned int i = 0; i < appraisers.size(); ++i)
    {
      const typename Swp::DerivedInfo derivedInfo = swp.getDerivedInfoForAppraiser(appraisers[i]);
      rankMap[derivedInfo].push_back(appraisers[i]);
    }

    return rankMap;
  }

  std::vector<std::string> fillBuilderLines(const Swp& swp, const RankMap& rankMap)
  {
    std::vector<std::string> builderLines;

    for (typename RankMap::const_iterator it = rankMap.begin(); it != rankMap.end(); ++it)
    {
      const unsigned int& rank = it->first;
      const Appraisers& appraisersWithRank = it->second;
      const std::string indent(_appraiserIndent * rank, ' ');

      for (unsigned int i = 0; i < appraisersWithRank.size(); ++i)
      {
        const typename Swp::Appraiser* app = appraisersWithRank[i];
        std::ostringstream tmp;
        tmp << indent;
        tmp << ((app->isSatisfied()) ? "S" : "U");
        tmp << " --- ";
        tmp << app->toString();
        tmp << " (priority " << swp.getInfoForAppraiser(app) << ")";
        builderLines.push_back(tmp.str());
      }
    }
    return builderLines;
  }

  ScoredItems fillScoredItems(const Swp& swp)
  {
    ScoredItems scoredItems;

    for (typename Swp::Iterator it = swp.begin(); it != swp.end(); ++it)
    {
      std::ostringstream tmp;
      tmp << it->key;
      ScoredItem newItem;
      std::string keystring = tmp.str();
      newItem.push_back(keystring);
      if (keystring.size() > _maxItemWidth)
      {
        _maxItemWidth = keystring.size();
      }

      tmp.str("");
      tmp << it->score;
      newItem.push_back(tmp.str());

      const typename Swp::AppraiserScoresMap& m = swp.getAppraiserScoresForItem(it->key);
      const std::string appScores = PriorityScoreFormatter<ItemType>(swp).formatScores(m);
      tmp.str("");
      tmp << "[" << appScores << "]";
      newItem.push_back(tmp.str());

      scoredItems.push_back(newItem);
    }

    return scoredItems;
  }

  void dumpHeader(std::ostream& out, const Swp& swp)
  {
    out << std::left;
    out << "SWAPPER: " << swp.getSize() << "/" << swp.getCapacity() << " items" << std::endl;
    out << "number of last iterations with no progress: " << swp.getNoProgressIterationsCount()
        << std::endl;
    out << "total number of iterations:                 " << swp.getTotalIterationsCount()
        << std::endl;
    out << std::endl;
    out << std::setw(_maxItemWidth + ITEM_SPACING) << "";
    out << "REQUIREMENTS" << std::endl;
    out << std::setw(_maxItemWidth + ITEM_SPACING) << "";
    out << "S: requirement satisfied" << std::endl;
    out << std::setw(_maxItemWidth + ITEM_SPACING) << "";
    out << "U: requirement unsatisfied" << std::endl;
    out << std::endl;
  }

  void dumpTableHeader(std::ostream& out, const Swp& swp)
  {
    out << std::endl;
    out << "[Item] // [Must have] // [Nice to have] // [Minor sort key] // [Appraiser scores]";
    out << std::endl;
  }

  void dumpBuilder(std::ostream& out, const std::vector<std::string>& builderLines)
  {
    out << std::left;
    for (auto& builderLine : builderLines)
    {
      out << std::setw(_maxItemWidth + ITEM_SPACING + OPENING_BRACKET_SZ) << "";
      out << builderLine << std::endl;
    }
  }

  void dumpItems(std::ostream& out, const ScoredItems& scoredItems)
  {
    out << std::left;
    for (int i = static_cast<int>(scoredItems.size() - 1); i >= 0; --i)
    {
      out << std::setw(_maxItemWidth + ITEM_SPACING) << scoredItems[i][0];
      out << scoredItems[i][1];
      out << " ";
      out << scoredItems[i][2];
      out << std::endl;
    }
  }

  // Formatting
  unsigned int _appraiserIndent;
  unsigned int _maxItemWidth;
  static const unsigned int ITEM_SPACING = 2;
  static const unsigned int OPENING_BRACKET_SZ = 1;
};

template <typename ItemType>
std::ostream& operator<<(std::ostream& out, const Swapper<ItemType, PriorityScoreBuilder>& ps)
{
  out << utils::format(ps, PrioritySwapperFormatter<ItemType>::format);
  return out;
}

} // namespace swp

} // namespace tse

