//----------------------------------------------------------------------------
//  File:        Diag856Collector.C
//
//  Description: Diagnostic 856 formatter
//
//  Copyright Sabre 2008
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
#include "Diagnostic/Diag856Collector.h"

#include "Common/LocUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DBAccess/NoPNROptions.h"

#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace tse
{
namespace
{
static const string unknownCatString = "UNKNOWN";

constexpr int minCatNo = 1;
constexpr int maxCatNo = 10;

constexpr int displayOptionFieldWidth = 57;
constexpr int fareTypeNoFieldWidth = 3;
constexpr int fareTypeNameFieldWidth = 54;
static const string ERROR_NO_NOPNROPTIONS_FOR_AGENT = "ERROR NO NOPNROPTIONS FOR AGENT";

struct CompareSegs
{
  bool operator()(NoPNROptionsSeg* seg, NoPNROptionsSeg* seg2)
  {
    return ((nullptr == seg) || ((nullptr != seg2) && (seg->seqNo() < seg2->seqNo())));
  }
};
}

const NoPNROptions*
Diag856Collector::getDBPnrOptions(const NoPNRPricingTrx& trx)
{
  const NoPNROptions* options = trx.noPNROptions();
  if (!options)
    throw runtime_error(ERROR_NO_NOPNROPTIONS_FOR_AGENT);
  return options;
}

string
Diag856Collector::getDiagHeader()
{
  return "*******************  NO PNR PRICING TABLE  *******************";
}

void
Diag856Collector::process(const NoPNRPricingTrx& trx)
{
  if (_active)
  {
    ostringstream response;
    try
    {
      response << getDiagHeader() << endl << getDiagBody(trx) << endl << getDiagFooter() << endl;
    }
    catch (const runtime_error& err)
    {
      return;
    }
    catch (...)
    {
      return;
    }
    (*this) << response.str();
  }
}

string
Diag856Collector::getDiagFooter()
{
  return "*********   END-OF-NO-PNR-PRICING TABLE-DISPLAY   ************";
}

string
Diag856Collector::getDiagBody(const NoPNRPricingTrx& trx)
{
  const NoPNROptions* options = getDBPnrOptions(trx);
  if (!options)
  {
    throw runtime_error(ERROR_NO_NOPNROPTIONS_FOR_AGENT);
  }

  ostringstream to_return;

  to_return << getCommonDiagBodyPart(*options);
  to_return << getSegmentsInfo(*options);

  return to_return.str();
}

string
Diag856Collector::getSegmentsInfo(const NoPNROptions& options)
{
  ostringstream to_return;
  to_return.setf(ios::left, ios::adjustfield);
  to_return << "******************* FARE TYPE DESIGNATION MATRIX  ************" << endl;
  const std::vector<NoPNROptionsSeg*>& segments = options.segs();

  vector<NoPNROptionsSeg*> sortedSegs;
  std::copy(segments.begin(), segments.end(), std::back_inserter(sortedSegs));
  std::sort(sortedSegs.begin(), sortedSegs.end(), CompareSegs());

  int totalOptionsFound = 0;

  for (unsigned int i = 0; i < sortedSegs.size(); ++i)
  {
    if (!sortedSegs[i])
      continue;
    to_return << setw(fareTypeNoFieldWidth) << (i + 1) << setw(fareTypeNameFieldWidth)
              << ((sortedSegs[i]->fareTypeGroup() >= minCatNo &&
                   sortedSegs[i]->fareTypeGroup() <= maxCatNo)
                      ? NoPNRPricingTrx::FareTypes::FTGC[sortedSegs[i]->fareTypeGroup()]
                      : unknownCatString) << sortedSegs[i]->noDisplayOptions() << endl;
    totalOptionsFound += sortedSegs[i]->noDisplayOptions();
  }
  to_return << setw(displayOptionFieldWidth) << " TOTAL NUMBER OF OPTIONS"
            << options.totalNoOptions() << endl;
  return to_return.str();
}

string
Diag856Collector::getCommonDiagBodyPart(const NoPNROptions& options)
{
  ostringstream to_return;
  to_return.setf(ios::left, ios::adjustfield);

  string crsUserAppl = "", mhUserAppl = "";
  if (options.userApplType() == 'C')
  {
    // this is CRS user
    crsUserAppl = options.userAppl();
  }
  else if (options.userApplType() == 'M')
  {
    // this is multihost user
    mhUserAppl = options.userAppl();
  }
  else
  {
    // what to do now ?
    // leaving both blank for now
  }

  to_return << setw(displayOptionFieldWidth) << "CRS USER APPLICATION:" << crsUserAppl << endl
            << setw(displayOptionFieldWidth) << "MULTI-HOST USER APPLICATION:" << mhUserAppl << endl
            << setw(displayOptionFieldWidth) << "LOC1TYPE:" << options.loc1().locType() << endl
            << setw(displayOptionFieldWidth) << "LOC1:" << options.loc1().loc() << endl << " "
            << endl << "*********************  WQ DISPLAY OPTIONS  *******************" << endl <<
      //(options.wqNotPermitted() == 'Y' ? "WQ NOT PERMITTED" : "WQ PERMITTED") << endl <<
      setw(displayOptionFieldWidth) << "WQ NOT PERMITTED" << options.wqNotPermitted() << endl
            << setw(displayOptionFieldWidth) << "MAX NUMBER OF OPTIONS" << options.maxNoOptions()
            << endl << setw(displayOptionFieldWidth) << "WQ SORT" << options.wqSort() << endl
            << setw(displayOptionFieldWidth) << "WQ DUPLICATE AMOUNTS"
            << options.wqDuplicateAmounts() << endl << setw(displayOptionFieldWidth)
            << "WQ FARE LINE HEADER FORMAT" << options.fareLineHeaderFormat() << endl
            << setw(displayOptionFieldWidth) << "PSGR DETAIL LINE FORMAT"
            << options.passengerDetailLineFormat() << endl << setw(displayOptionFieldWidth)
            << "FARE LINE PTC" << options.fareLinePTC() << endl << setw(displayOptionFieldWidth)
            << "PRIME PTC REFERENCE NUMBER" << options.primePTCRefNo() << endl
            << setw(displayOptionFieldWidth) << "SECONDARY PTC REFRENCE NUMBER"
            << options.secondaryPTCRefNo() << endl << setw(displayOptionFieldWidth)
            << "FARE LINE PTC BREAK" << options.fareLinePTCBreak() << endl
            << setw(displayOptionFieldWidth) << "PASSENGER DETAIL PTC BREAK"
            << options.passengerDetailPTCBreak() << endl << setw(displayOptionFieldWidth)
            << "NEG PASSENGER TYPE MAPPING" << options.negPassengerTypeMapping() << endl
            << setw(displayOptionFieldWidth) << "NO MATCH ROLLOVER OPTIONS DISPLAY PREFERENCE"
            << options.noMatchOptionsDisplay() << endl << setw(displayOptionFieldWidth)
            << "ALL MATCH TRAILER MESSAGE" << options.allMatchTrailerMessage() << endl
            << setw(displayOptionFieldWidth) << "MATCH/NO MATCH INTEGRATED TRAILER MESSAGE"
            << options.matchIntegratedTrailer() << endl << setw(displayOptionFieldWidth)
            << "ACCOMPANYING TRAVEL TRAILER MESSAGE" << options.accompaniedTvlTrailerMsg() << endl
            << setw(displayOptionFieldWidth)
            << "RBD MATCH/NO MATCH INTEGRATED TRAILER MESSAGE-PRIMARY"
            << options.rbdMatchTrailerMsg() << endl << setw(displayOptionFieldWidth)
            << "RBD NO MATCH TRAILER MESSAGE-PRIMARY" << options.rbdNoMatchTrailerMsg() << endl
            << setw(displayOptionFieldWidth) << "RBD NO MATCH TRAILER MESSAGE-SECONDARY"
            << options.rbdNoMatchTrailerMsg2() << endl << setw(displayOptionFieldWidth)
            << "DISPLAY FARE RULE WARNING-PRIMARY" << options.displayFareRuleWarningMsg() << endl
            << setw(displayOptionFieldWidth) << "DISPLAY FARE RULE WARNING-SECONDARY"
            << options.displayFareRuleWarningMsg2() << endl << setw(displayOptionFieldWidth)
            << "DISPLAY TAX WARNING-PRIMARY" << options.displayTaxWarningMsg() << endl
            << setw(displayOptionFieldWidth) << "DISPLAY TAX WARNING-SECONDARY"
            << options.displayTaxWarningMsg2() << endl << setw(displayOptionFieldWidth)
            << "DISPLAY FINAL WARNING MESSAGE-PRIMARY" << options.displayFinalWarningMsg() << endl
            << setw(displayOptionFieldWidth) << "DISPLAY FINAL WARNING MESSAGE-SECONDARY"
            << options.displayFinalWarningMsg2() << endl << setw(displayOptionFieldWidth)
            << "DISPLAY PRIVATE FARE INDICATORS" << options.displayPrivateFareInd() << endl
            << setw(displayOptionFieldWidth) << "ALWAYS MAP TO ADT PSGR TYPE"
            << options.alwaysMapToADTPsgrType() << endl << setw(displayOptionFieldWidth)
            << "APPLY RO IN FARE LINE DISPLAY" << options.applyROInFareDisplay() << endl
            << setw(displayOptionFieldWidth) << "DISPLAY NON COC CURRENCY INDICATOR"
            << options.displayNonCOCCurrencyInd() << endl << setw(displayOptionFieldWidth)
            << "DISPLAY TRUE PTC IN FARE LINE DISPLAY" << options.displayTruePTCInFareLine() << endl
            << " " << endl << "******************  WQ NO MATCH / ROLLOVER OPTIONS  **********"
            << endl << setw(displayOptionFieldWidth) << "NO MATCH RBD HEADER MESSAGE"
            << options.noMatchRBDMessage() << endl << setw(displayOptionFieldWidth)
            << "NO MATCH/NO FARES ERROR MESSAGE" << options.noMatchNoFaresErrorMsg() << endl << " "
            << endl;

  return to_return.str();
}
}
