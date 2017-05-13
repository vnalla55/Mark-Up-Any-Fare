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

#include "Diagnostic/Diag858Collector.h"

#include "Common/TseConsts.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"


#include <string>

namespace tse
{
void
Diag858Collector::printHeader()
{
  *this << "******************** TICKETING ENDORSEMENTS *******************\n";
  *this << "PRICING UNIT ENDORSEMENTS\n";
}

void
Diag858Collector::print(const PricingUnit& prU)
{
  lineSkip(0);
  *this << " PRICING UNIT:";
  unsigned short wrap = 0;

  for (const FareUsage* fu : prU.fareUsage())
  {
    *this << "  " << fu->paxTypeFare()->fareMarket()->boardMultiCity() << "-"
          << fu->paxTypeFare()->fareMarket()->governingCarrier() << "-"
          << fu->paxTypeFare()->fareMarket()->offMultiCity();

    if (++wrap % 3 == 0) // && prU.fareUsage().size() % 3 != 0)
      *this << "\n" << std::string(13, ' ');
  }

  *this << "\n";
}

void
Diag858Collector::printEndorsement(const PaxTypeFare* ptf,
                                   const std::vector<TicketEndorseItem>& endos,
                                   size_t validCounter)
{
  *this << " " << *ptf << "\n";

  unsigned short counter = 0;
  for (const TicketEndorseItem& tei : endos)
  {
    printSingleEndorsement(++counter, tei);

    if (isFop(tei))
    {
      *this << "\n      FOP - NOT INCLUDED IN FINAL MESSAGE";
    }

    *this << "\n";
  }
}

void
Diag858Collector::printEndorsement(const FarePath& fp)
{
  printLine();
  *this << "FARE PATH ENDORSEMENTS\n";

  unsigned short counter = 0;
  for (const TicketEndorseItem& tei : fp.tktEndorsement())
  {
    printSingleEndorsement(++counter, tei);
    *this << "\n";
  }
}

void
Diag858Collector::printSingleEndorsement(unsigned short counter, const TicketEndorseItem& tei)
{
  *this << "  " << counter << " - " << tei.endorsementTxt << "\n"
        << "      TKTLOCIND - " << tei.tktLocInd << "  PRIORITY - " << tei.priorityCode
        << "  ITEM NO - " << tei.itemNo;
}

void
Diag858Collector::printAllEndoLines(const TicketingEndorsement::TicketEndoLines& msgs)
{
  unsigned short counter = 0;
  for (const TicketEndorseLine* tel : msgs)
  {
    *this << "\n " << ++counter << " - ";

    const size_t maxLineLength = 57;
    const size_t maxSize = tel->endorseMessage.size() + maxLineLength;
    for (size_t i = 1; i * maxLineLength < maxSize; ++i)
      *this << tel->endorseMessage.substr((i - 1) * maxLineLength, maxLineLength) << "\n     ";

    *this << "CARRIER - " << tel->carrier;

    if (!tel->segmentOrders.empty())
    {
      *this << "  SEG - ";

      for (int16_t segNum : tel->segmentOrders)
        if (segNum != ARUNK_PNR_SEGMENT_ORDER)
          *this << segNum << "/";
    }

    *this << "  PRIORITY - " << tel->priorityCode;
  }
}

void
Diag858Collector::printGluedMsgs(const TicketingEndorsement::TicketEndoLines& msgs)
{
  printLine();
  *this << "FINAL ENDORSEMENT MESSAGE\nSORTING FOR ";

  *this << "PRICING USING PNR SEGMENT ORDER";

  printAllEndoLines(msgs);
}

void
Diag858Collector::printGluedMsgs(const TicketingEndorsement::TicketEndoLines& msgs,
                                 const CarrierCode& cxr)
{
  printLine();
  *this << "FINAL ENDORSEMENT MESSAGE\nSORTING FOR ";

  *this << "TICKETING ENTRY USING PRIORITY AND CARRIER: " << cxr;

  printAllEndoLines(msgs);
}
}
