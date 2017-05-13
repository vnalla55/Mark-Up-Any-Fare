//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

//#include "Common/TseCallableTrxTask.h"
#include "DateTime.h"
//#include "DataModel/Trx.h"

namespace tse {

DateTime DateTime::localTime()
{
		time_t t;
		time(&t);
		tm *ltime;
		ltime = localtime(&t);

		return DateTime(ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday,
		                ltime->tm_hour,ltime->tm_min,ltime->tm_sec);

}

const DateTime& DateTime::openDate()
{
  static const DateTime openDateValue(boost::gregorian::date(1966, 1, 1),
		                                  boost::posix_time::time_duration(0,0,0));
	return openDateValue;
}

const DateTime& DateTime::emptyDate()
{
  static DateTime emptyDateValue(boost::gregorian::date(1980, 1, 1),
		                             boost::posix_time::time_duration(0,0,0));
	return emptyDateValue;
}
}
