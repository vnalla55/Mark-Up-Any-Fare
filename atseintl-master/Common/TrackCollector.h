//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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

#include <boost/thread/mutex.hpp>

#include <set>
#include <map>
#include <string>

namespace tse
{

class DiagCollector;

class TrackCollector : protected std::map<std::string, uint16_t>
{
  friend class TrackCollectorTest;

public:
  typedef mapped_type Counter;
  typedef key_type Label;
  typedef std::multiset<std::pair<std::size_t, Label>> XmlHashes;

  using std::map<Label, Counter>::empty;

  void collect(const Label& key, const std::string& additionalInfo = "");
  void print(DiagCollector& diag) const;
  void getLabels(std::vector<Label>& labels) const;
  void assign(const TrackCollector& src);

private:
  static const Counter MAX_COUNTER;
  static const Label::size_type LABEL_LENGTH;
  static XmlHashes _xmlHashes;

  std::string cutLabel(const Label& label) const { return label.substr(0, LABEL_LENGTH); }

  mutable boost::mutex _mutex;
};

} // tse
