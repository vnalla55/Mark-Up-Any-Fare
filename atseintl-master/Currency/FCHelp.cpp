#include "Currency/FCHelp.h"

namespace tse
{

void
FCHelp::process()
{
  _trx.response() << "FCHELP    IROE NUC/CURRENCY FORMAT ENTRIES          " << std::endl
                  << "----------------------------------------------------------------" << std::endl
                  << "THE FC ENTRIES TO DISPLAY NUC RATES AND CONVERSIONS BETWEEN" << std::endl
                  << "NUC AND CURRENCY -  FORMAT OF RESPONSE HAS BEEN MODIFIED." << std::endl << " "
                  << std::endl << "FC* COMMANDS ARE USED TO DISPLAY IROE NUC RATES" << std::endl
                  << "FC$ COMMANDS ARE USED TO CONVERT NUC TO CUR AND CUR TO NUC" << std::endl
                  << "----------------------------------------------------------------" << std::endl
                  << " " << std::endl << "FC DISPLAY FORMAT  " << std::endl << " " << std::endl
                  << "FC* - LIST OF ALL CURRENCY CODES - CURRENT NUC RATES" << std::endl << " "
                  << std::endl << "FC** - LIST OF ALL CURRENCY CODES - CURRENT/FUTURE NUC RATES"
                  << std::endl << " " << std::endl
                  << "FC*CUR - SPECIFIC CURRENCY - CURRENT NUC RATE" << std::endl << " "
                  << std::endl << "FC**CUR - SPECIFIC CURRENCY - CURRENT/FUTURE NUC RATES"
                  << std::endl << "----------------------------------------------------------------"
                  << std::endl << " " << std::endl
                  << "HISTORICAL NUC RATE DISPLAY - DATE FORMAT DDMMMYY" << std::endl << " "
                  << std::endl << "FC*DDMMMYY - ALL CURRENCIES - NUC RATES FROM DATE ENTERED"
                  << std::endl << " " << std::endl
                  << "FC*CURDDMMMYY - SPECIFIC CURRENCY - NUC RATE FROM DATE ENTERED" << std::endl
                  << "----------------------------------------------------------------"
                  << " " << std::endl << "FC$ CONVERSION FORMAT" << std::endl << " " << std::endl
                  << "FC$NUCNNNN/CUR - CONVERT NUC /NNNN VALUE/ TO /CUR/ CURRENCY" << std::endl
                  << " " << std::endl << "FC$NNNN/CUR - SAME AS ABOVE WITHOUT NEED TO ENTER NUC"
                  << std::endl << " " << std::endl
                  << "FC$CURNNNN/NUC - CONVERT /CUR/ CURRENCY /NNNN VALUE/ TO NUC  " << std::endl
                  << " " << std::endl << "FC$CURNNNN - SAME AS ABOVE WITHOUT NEED TO ENTER NUC    "
                  << std::endl << " " << std::endl << "NNNN FORMAT LIMITS" << std::endl
                  << "NUC VALUE MUST NOT EXCEED 2 DECIMAL INPUT " << std::endl
                  << "NNNN DIGIT LIMITED TO 12 DIGIT ENTRY FOR VALID CONVERSION  " << std::endl
                  << " " << std::endl << " " << std::endl
                  << "HISTORICAL RATE CONVERSION - DATE FORMAT DDMMMYY" << std::endl << " "
                  << std::endl << "FC$NUCNNNN/CUR/DDMMMYY" << std::endl << " " << std::endl
                  << "FC$NNNN/CUR/DDMMMYY " << std::endl << " " << std::endl
                  << "FC$CURNNNN/NUC/DDMMMYY   " << std::endl << " " << std::endl
                  << "FC$CURNNNN/DDMMMYY     " << std::endl << std::endl;
}

} // tse
