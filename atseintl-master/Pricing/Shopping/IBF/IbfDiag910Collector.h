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

#include "Diagnostic/DiagManager.h"
#include "Pricing/Shopping/FiltersAndPipes/ICollector.h"
#include "Pricing/Shopping/FiltersAndPipes/IFilterObserver.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <map>
#include <sstream>
#include <string>

namespace tse
{

class DiagManager;
class ShoppingTrx;
class TravelSeg;

class SopCombinationListFormatter
{
public:
  SopCombinationListFormatter(ShoppingTrx& trx);

  void setCommentMetatext(const std::string& text) { _commentMetatext = text; }

  void addSopCombination(const utils::SopCombination& combination, const std::string& comment = "");

  size_t getNbrOfCombinations() const { return _list.size(); }

  size_t getWidth() const
  {
    size_t w = _solutionNbrWidth + (3 * _numericFieldWidth) + (2 * _locationWidth);
    if (_commentMetatext.size() > 0)
    {
      w += (_commentMetatext.size() + 1);
    }
    return w;
  }

  void flush(std::ostream& out);

private:
  typedef std::pair<utils::SopCombination, std::string> CommentedCombination;

  void printHeader(std::ostream& out) const;
  void dumpSopCombination(std::ostream& out, const CommentedCombination& c, int index) const;
  std::string formatTravelSegment(const TravelSeg& segment) const;

  ShoppingTrx& _trx;
  std::vector<CommentedCombination> _list;

  int _solutionNbrWidth;
  int _numericFieldWidth;
  int _locationWidth;

  std::string _commentMetatext;
};

class IbfDiag910Collector : public utils::ICollector<utils::SopCombination>,
                            public utils::IFilterObserver<utils::SopCandidate>,
                            public utils::IFilterObserver<utils::SopCombination>
{
public:
  IbfDiag910Collector(ShoppingTrx& trx, const std::string& customHeaderMessage = "");

  bool areDetailsFor910Enabled() const { return _areDetailsEnabled; }

  // Set the initial number of SOPs
  // on given leg (just before FOS combinations
  // are genarated)
  void setNumberOfSopsForLeg(unsigned int legId, unsigned int sopsNumber)
  {
    _sopsPerLeg[legId] = sopsNumber;
  }

  // Here accumulate failed SOPs that did not
  // pass CabinClass validation
  void
  elementInvalid(const utils::SopCandidate& candid,
                 const utils::INamedPredicate<utils::SopCandidate>& failedPredicate) override;

  // Here accumulate failed SOP combinations
  // that did not pass Interline Ticketing Agreement
  // or Minimum Connect Time
  void elementInvalid(const utils::SopCombination& combination,
                      const utils::INamedPredicate<utils::SopCombination>& failedPredicate) override;

  // Here accumulate correct FOS combinations
  void collect(const utils::SopCombination& comb) override;

  // Output diagnostic data to a diag manager
  void flush();

  void setShouldPrintFailedSops(bool value) { _shouldPrintFailedSops = value; }

private:
  IbfDiag910Collector(const IbfDiag910Collector& right);
  IbfDiag910Collector& operator=(const IbfDiag910Collector& right);

  void printSopPerLegInfo(std::ostream& out) const;

  ShoppingTrx& _trx;
  DiagManager _diag;

  // Passed FOS collecting
  SopCombinationListFormatter _fos;

  bool _areDetailsEnabled;

  // Failed SOPs collecting
  std::ostringstream _failedSopStream;
  bool _failedSopsPresent;

  // Failed SOP combinations
  SopCombinationListFormatter _failedCombinations;

  std::map<unsigned int, unsigned int> _sopsPerLeg;
  std::string _customHeaderMessage;
  bool _shouldPrintFailedSops;
};

} // namespace tse

