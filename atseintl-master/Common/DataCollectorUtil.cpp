#include "Common/DataCollectorUtil.h"

namespace tse
{

std::string
DataCollectorUtil::format(const DateTime& dateTime, bool includeMillis)
{
  // This returns a date in the form YYYY-MM-DDTHH:MM:SS,fffffffff
  std::string formattedDate = boost::posix_time::to_iso_extended_string(dateTime);

  if (formattedDate.empty())
    return formattedDate;

  size_t dateLen = formattedDate.length();
  // The XML wants the date in the form YYYY-MM-DD:HH:MM:SS.NNN

  if (dateLen > 10)
    formattedDate[10] = ':'; // Fix the T between DD and HH

  if (!includeMillis)
  {
    if (dateLen > 19)
      formattedDate.resize(19, ' ');

    return formattedDate;
  }

  if (dateLen > 19)
    formattedDate[19] = '.'; // Fix the , between SS and fffffffff

  if (dateLen > 23)
    formattedDate.resize(23, ' ');

  return formattedDate;
}
}
