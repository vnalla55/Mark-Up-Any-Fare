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

#include "Common/Logger.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TrackCollector.h"
#include "Diagnostic/DiagCollector.h"

#include <log4cxx/patternlayout.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/helpers/transcoder.h>

#include <functional>
#include <limits>

namespace tse
{

namespace
{
Logger logger("atseintl.Common.TrackCollector");
}

const TrackCollector::Counter TrackCollector::MAX_COUNTER = std::numeric_limits<Counter>::max();

const size_t TrackCollector::LABEL_LENGTH = 10;

TrackCollector::XmlHashes TrackCollector::_xmlHashes;

void
TrackCollector::collect(const Label& label, const std::string& additionalInfo)
{
  TSE_ASSERT(!label.empty());
  if (label.at(0) == '@')
  {
    const Trx* currentTrx = TseCallableTrxTask::currentTrx();
    const std::string& reqXml = currentTrx->rawRequest();
    std::hash<std::string> hash_fn;
    std::size_t str_hash = hash_fn(reqXml);
    Label dandy = label.substr(1, LABEL_LENGTH);

    boost::lock_guard<boost::mutex> g(_mutex);
    std::pair<std::size_t, Label> hash_pair = std::make_pair(str_hash, dandy);
    XmlHashes::const_iterator it = _xmlHashes.find(hash_pair);
    if (it ==_xmlHashes.end())
    {
      _xmlHashes.emplace(hash_pair);
      LOG4CXX_DECODE_CHAR(file_name, "collector/" + dandy + ".log");
      log4cxx::PatternLayoutPtr layout = new log4cxx::PatternLayout("%d: %t %-5p %c{2} - %m%n");
      log4cxx::FileAppenderPtr appender
        (new log4cxx::FileAppender(layout, file_name, true));
      logger->addAppender(appender);
      LOG4CXX_INFO(logger,
                   hash_pair.first << " " << hash_pair.second << " " << reqXml << "(currently in "
                   << currentTrx->getCurrentService()->name() << ") - " << additionalInfo);
      logger->removeAppender(appender);
    }
    return;
  }

  boost::lock_guard<boost::mutex> g(_mutex);

  Counter& counter = (*this)[cutLabel(label)];
  if (counter < MAX_COUNTER)
    counter++;
}

void
TrackCollector::print(DiagCollector& diag) const
{
  if (!diag.isActive())
    return;

  static const std::string SEPARATOR(" : "), COLUMN_SPACE(2, ' '), NEW_LINE("\n");

  static const size_t COLUMN = 3,
                      COUNTER_LENGTH =
                          ((DiagCollector::DEFAULT_LINEWRAP_LENGTH - COLUMN_SPACE.size()) / COLUMN -
                           SEPARATOR.size() - LABEL_LENGTH - COLUMN);

  boost::lock_guard<boost::mutex> g(_mutex);

  size_t j = 0;
  for (const auto& elem : *this)
  {
    diag << ' ' << std::setw(int(LABEL_LENGTH)) << elem.first << SEPARATOR
         << std::setw(int(COUNTER_LENGTH)) << elem.second;
    diag << ((++j % COLUMN == 0) ? NEW_LINE : COLUMN_SPACE);
  }
  if (j % COLUMN)
    diag << NEW_LINE;
}

void
TrackCollector::getLabels(std::vector<Label>& labels) const
{
  boost::lock_guard<boost::mutex> g(_mutex);

  for (const auto& elem : *this)
    labels.push_back(elem.first);
}

void
TrackCollector::assign(const TrackCollector& src)
{
  boost::lock_guard<boost::mutex> g(_mutex), gs(src._mutex);

  std::map<Label, Counter>::operator=(src);

  for (auto& elem : *this)
    elem.second = Counter(elem.second + 10000);
}

} // tse

