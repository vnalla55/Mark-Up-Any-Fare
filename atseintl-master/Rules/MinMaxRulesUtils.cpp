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
//----------------------------------------------------------------------------

#include "Rules/MinMaxRulesUtils.h"
namespace tse
{
namespace MinMaxRulesUtils
{
DateTime
determineReturnDate(const DateTime& periodReturnDate,
                    const DateTime& stayDate,
                    const Indicator earlierLaterInd)
{
  DateTime earliestReturnDate;
  if ((stayDate.isValid()) && (periodReturnDate.isValid()))
  {
    DateTime minStayReturnDate(
        periodReturnDate.year(), periodReturnDate.month(), periodReturnDate.day());

    if (earlierLaterInd == EARLIER)
    {
      if (stayDate > minStayReturnDate)
        earliestReturnDate = periodReturnDate;
      else
        earliestReturnDate = stayDate;
    }
    else if (earlierLaterInd == LATER)
    {
      if (stayDate > minStayReturnDate)
        earliestReturnDate = stayDate;
      else
        earliestReturnDate = periodReturnDate;
    }
    else
      earliestReturnDate = periodReturnDate;
  }
  else if ((stayDate.isValid()))
    earliestReturnDate = stayDate;
  else if (LIKELY((periodReturnDate.isValid())))
    earliestReturnDate = periodReturnDate;
  return earliestReturnDate;
}
}
}
