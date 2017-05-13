// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "TaxDisplay/Common/DetailLevelsSequencer.h"

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <string>

namespace tax
{
namespace display
{

DetailLevelsSequencer::DetailLevelsSequencer(const DetailLevels& detailLevels)
{
  const boost::char_separator<char> detailLevelsSeparator("|");
  boost::tokenizer<boost::char_separator<char>> tokens(detailLevels, detailLevelsSeparator);
  for (const std::string& detailEntryNo : tokens)
  {
    _detailEntries.push_back(boost::lexical_cast<DetailEntryNo>(detailEntryNo));
  }
}

bool DetailLevelsSequencer::run() const
{
  if (_detailEntries.size() > _callbacks.size())
    return false;

  for (size_t i = 0; i < _detailEntries.size(); ++i)
  {
    bool isThisLastCallback = i == _detailEntries.size() - 1;
    if ( !_callbacks[i](_detailEntries[i], isThisLastCallback) )
      return false;
  }

  return true;
}

} // namespace display
} // namespace tax
