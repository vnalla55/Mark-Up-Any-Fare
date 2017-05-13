
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/Pfc/PfcDisplayBuilderPXA.h"
#include "Taxes/Pfc/PfcDisplayErrorMsg.h"

using namespace tse;

const std::string PfcDisplayBuilderPXA::MAIN_HEADER_PXA = "ABSORPTION AIRPORT TABLE";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::PfcDisplayBuilderPXA
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXA::PfcDisplayBuilderPXA(TaxTrx* trx, PfcDisplayData* data)
  : PfcDisplayBuilder(trx, data)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::~PfcDisplayBuilderPXA
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXA::~PfcDisplayBuilderPXA() {}

const std::string PfcDisplayBuilderPXA_generalInfo::INFO_HEADER =
    "            DISPLAY BELOW APPLIES AS GUIDELINE\n"
    "     TO DETERMINE FARE AND TAX AMOUNTS TO BE DEDUCTED\n"
    "      OR ANY RULES AND RESTRICTIONS WHICH MAY APPLY\n"
    "     TO SPECIFIC AIRPORT - ENTER ONE OF 3 ENTRIES BELOW\n";

const std::string PfcDisplayBuilderPXA_generalInfo::HELP_HEADER = "PXA*LINE NUMBER\n"
                                                                  "PXA*AIRPORT CODE\n"
                                                                  "PXA*AIRPORT CODE/CARRIER";

const std::string PfcDisplayBuilderPXA_generalInfo::TABLE_HEADER =
    "LINE    ABSORPTION  EFFECTIVE  DISCONTINUE  ABSORPTION\n"
    "NBR     AIRPORT     DATE       DATE         CARRIER";

const std::string PfcDisplayBuilderPXA_generalInfo::TABLE_FORMATTING =
    "LINE    ABSORPTION  EFFECTIVE  DISCONTINUE     ORPTION\n"
    "NBR     AIRPORT     DATE       DATE            XXXX";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::PfcDisplayBuilderPXA
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXA_generalInfo::PfcDisplayBuilderPXA_generalInfo(TaxTrx* trx,
                                                                   PfcDisplayData* data)
  : PfcDisplayBuilderPXA(trx, data), _formatter(PfcDisplayFormatterPXA(TABLE_FORMATTING))
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::~PfcDisplayBuilderPXA
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXA_generalInfo::~PfcDisplayBuilderPXA_generalInfo() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::buildHeader
//
// Description:  PXC entry header.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_generalInfo::buildHeader()
{
  return fmt().center(MAIN_HEADER + " \n" + MAIN_HEADER_PXA) + " \n" + (INFO_HEADER) + " \n" +
         fmt().margin(HELP_HEADER, 12) + " \n" + TABLE_HEADER + "\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::buildBody
//
// Description:  PXA entry body.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_generalInfo::buildBody()
{
  std::string body;

  const std::vector<PfcAbsorb*>& pfcAbsorbV = data()->getPfcAbsorb();

  if (pfcAbsorbV.empty())
  {
    return body;
  }

  body = " \n";

  std::string nbr;
  std::string arpt;
  std::string carrier;
  std::string effDate;
  std::string discDate;
  std::string discDateCurrent;

  size_t lineCounter = 1;

  std::vector<PfcAbsorb*>::const_iterator it = pfcAbsorbV.begin();
  std::vector<PfcAbsorb*>::const_iterator itEnd = pfcAbsorbV.end();

  for (; it < itEnd; it++)
  {
    discDateCurrent = fmt().toString((*it)->discDate());

    if (data()->ifNotEqualitySet(std::string((*it)->pfcAirport()),
                                 std::string((*it)->localCarrier()),
                                 fmt().toString((*it)->effDate()),
                                 discDateCurrent,
                                 arpt,
                                 carrier,
                                 effDate,
                                 discDate))
    {
      nbr = fmt().toString(lineCounter++);

      body += fmt().line(nbr, arpt, effDate, discDate, carrier);
    }
  }

  return body;
}

const std::string PfcDisplayBuilderPXA_detailInfo::MULTICARRIER_INFO_HEADER =
    "ENTER PXA*AIRPORT/CARRIER CODE FOR CARRIER DETAILS\n";

const std::string PfcDisplayBuilderPXA_detailInfo::INFO_HEADER =
    "          DISPLAY BELOW APPLIES AS GUIDELINE\n"
    "   DISPLAY REFLECTS PFC FARE AMOUNT DEDUCTED FROM CARRIER \n"
    " FARE BASED ON CURRENT U.S DOMESTIC TRANSPORTATION TAX RATE\n"
    "          USE ENTRY PXA*AIRPORT CODE/CARRIER CODE-.TAX PERCENT\n"
    "           FOR SPECIFIC TAX RATES PXA*DEN/CC-.075\n";

const std::string PfcDisplayBuilderPXA_detailInfo::TABLE_HEADER1 =
    std::string(2, PfcDisplayFormatter::SPACE) + "ABSORPTION" +
    std::string(22, PfcDisplayFormatter::SPACE) + "ABSORPTION\n";

const std::string PfcDisplayBuilderPXA_detailInfo::TABLE_HEADER2 =
    "CXR         AIRPORT         EFF. DATE    DISC. DATE\n";

const std::string PfcDisplayBuilderPXA_detailInfo::HELP_HEADER =
    "GEO - GEOGRAPHIC DESCRIPTION\n"
    "USA - WHOLLY WITHIN THE U.S. 50 STATES\n"
    "USP - WHOLLY WITHIN THE U.S. 50 STATES AND POSSESSION\n";

const std::string PfcDisplayBuilderPXA_detailInfo::SEQ_HEADER =
    "SEQ    GEO  ABSORPTION ACTION                    FARE   TAX\n";

const std::string PfcDisplayBuilderPXA_detailInfo::SEQ_LINE1 =
    "AIRPORT-" + std::string(14, PfcDisplayFormatter::SPACE) + "RULE-" +
    std::string(16, PfcDisplayFormatter::SPACE) + "FARETARIFF-\n";

const std::string PfcDisplayBuilderPXA_detailInfo::SEQ_LINE2 =
    "FARECLASS-" + std::string(12, PfcDisplayFormatter::SPACE) + "ROUTING RANGE-" +
    std::string(11, PfcDisplayFormatter::SPACE) + "OW/RT-\n";

const std::string PfcDisplayBuilderPXA_detailInfo::SEQ_LINE3 =
    "FLT RANGE1-" + std::string(11, PfcDisplayFormatter::SPACE) + "FLT RANGE2-" +
    std::string(13, PfcDisplayFormatter::SPACE) + "JOINTCXR-\n";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXAItinerary::PfcDisplayBuilderPXCItinerary
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXA_detailInfo::PfcDisplayBuilderPXA_detailInfo(TaxTrx* trx, PfcDisplayData* data)
  : PfcDisplayBuilderPXA(trx, data),
    _formatter(PfcDisplayFormatterPXA(TABLE_HEADER2)),
    _formatterSeqHeader(PfcDisplayFormatterPXA(SEQ_HEADER)),
    _formatterSeqLine1(PfcDisplayFormatterPXA(SEQ_LINE1)),
    _formatterSeqLine2(PfcDisplayFormatterPXA(SEQ_LINE2)),
    _formatterSeqLine3(PfcDisplayFormatterPXA(SEQ_LINE3)),
    _isMoreThanOneLine(false)
{
  std::string carrier;
  std::string arpt;
  std::string effDate;
  std::string discDate;

  std::vector<PfcAbsorb*>::const_iterator it = this->data()->getPfcAbsorb().begin();
  std::vector<PfcAbsorb*>::const_iterator itEnd = this->data()->getPfcAbsorb().end();

  uint32_t count = 0;

  for (; it < itEnd; it++)
  {
    if (this->data()->ifNotEqualitySet(std::string((*it)->pfcAirport()),
                                       std::string((*it)->localCarrier()),
                                       fmt().toString((*it)->effDate()),
                                       fmt().toString((*it)->expireDate()),
                                       arpt,
                                       carrier,
                                       effDate,
                                       discDate))
    {
      if (count)
      {
        isMoreThanOneLine() = true;
        break;
      }
      count++;
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXCItinerary::~PfcDisplayBuilderPXCItinerary
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXA_detailInfo::~PfcDisplayBuilderPXA_detailInfo() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::buildHeader
//
// Description:  PXA entry header.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_detailInfo::buildHeader()
{

  if (data()->getPfcAbsorb().empty())
  {
    return PfcDisplayErrorMsg::DATA_NOT_FOUND;
  }

  if (isMoreThanOneLine())
  {
    return fmt().center(MAIN_HEADER + " \n" + MAIN_HEADER_PXA) + " \n" +
           fmt().center(MULTICARRIER_INFO_HEADER) + " \n" + TABLE_HEADER1 + TABLE_HEADER2 + "\n";
  }
  else
  {
    return fmt().center(MAIN_HEADER + " \n" + MAIN_HEADER_PXA) + " \n" + (INFO_HEADER) + " \n" +
           fmt().margin(HELP_HEADER, 6) + " \n" + TABLE_HEADER1 + TABLE_HEADER2 + "\n";
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::buildHeader
//
// Description:  PXA entry body.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_detailInfo::buildBody()
{
  std::string body;

  if (data()->getPfcAbsorb().empty())
  {
    return body;
  }

  body = " \n";

  std::string carrier;
  std::string arpt;
  std::string effDate;
  std::string discDate;

  std::vector<PfcAbsorb*>::const_iterator it = data()->getPfcAbsorb().begin();
  std::vector<PfcAbsorb*>::const_iterator itEnd = data()->getPfcAbsorb().end();

  for (; it < itEnd; it++)
  {
    if (data()->ifNotEqualitySet(std::string((*it)->pfcAirport()),
                                 std::string((*it)->localCarrier()),
                                 fmt().toString((*it)->effDate()),
                                 fmt().toString((*it)->expireDate()),
                                 arpt,
                                 carrier,
                                 effDate,
                                 discDate))
    {
      body += fmt().line(carrier, arpt, effDate, discDate);
    }
  }

  return body;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::buildFootnote
//
// Description:  PXA entry footnote.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_detailInfo::buildFootnote()
{
  std::string footnote;

  if (data()->getPfcAbsorb().empty() || isMoreThanOneLine())
  {
    return footnote;
  }

  footnote = footnote + " \n";

  std::string seq;
  std::string geo;
  std::string action;
  std::string fare;
  std::string tax;

  std::string arpts;
  std::string rule;
  std::string fareTariff;

  std::string fareClass;
  std::string routingRange;
  std::string owRt;

  std::string fltRange1;
  std::string fltRange2;
  std::string jointCxr;

  std::vector<PfcAbsorb*>::const_iterator it = data()->getPfcAbsorb().begin();
  std::vector<PfcAbsorb*>::const_iterator itEnd = data()->getPfcAbsorb().end();

  for (; it < itEnd; it++)
  {
    // HEADER
    seq = fmt().toString((*it)->seqNo());
    geo = geoApplDesc((*it)->geoAppl());
    action = absorbTypeDesc((*it)->absorbType());
    fare = fmt().fare(fmt().round(data()->getFareAmt((*it)->pfcAirport())));

    if (fare == PfcDisplayFormatter::NOT_APPLICABLE)
    {
      tax = PfcDisplayFormatter::NOT_APPLICABLE;
    }
    else
    {
      tax = fmt().tax(fmt().round(data()->getTaxAmt((*it)->pfcAirport())));
    }

    footnote += SEQ_HEADER;
    footnote += fmtSeqHeader().line(seq, geo, action, fare, tax);

    // LINE 1

    if (!(*it)->absorbCity1().empty() && !(*it)->absorbCity2().empty())
    {
      arpts = (*it)->absorbCity1() + "-" + (*it)->absorbCity2();
    }
    else if (!(*it)->absorbCity1().empty())
    {
      arpts = (*it)->absorbCity1();
    }
    else if (!(*it)->absorbCity2().empty())
    {
      arpts = (*it)->absorbCity2();
    }

    rule = (*it)->ruleNo();
    fareTariff = fmt().toString((*it)->fareTariff());

    footnote += " \n";
    footnote += fmtSeqLine1().line(arpts, rule, fareTariff);

    // LINE 2

    fareClass = (*it)->fareClass().c_str();

    if (!(*it)->routing1().empty() && !(*it)->routing2().empty())
    {
      routingRange = (*it)->routing1() + "-" + (*it)->routing2();
    }

    owRt = owrtDesc((*it)->OwRt());

    footnote += fmtSeqLine2().line(fareClass, routingRange, owRt);

    // LINE 3

    if ((*it)->flt1() != 0 && (*it)->flt2() != 0)
    {
      fltRange1 = fmt().toString((*it)->flt1()) + "-" + fmt().toString((*it)->flt2());
    }

    if ((*it)->flt3() != 0 && (*it)->flt4() != 0)
    {
      fltRange2 = fmt().toString((*it)->flt3()) + "-" + fmt().toString((*it)->flt4());
    }

    jointCxr = (*it)->jointCarrier();

    footnote += fmtSeqLine3().line(fltRange1, fltRange2, jointCxr);

    footnote += " \n";
  }

  return footnote;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::getGeoApplDesc
//
// Description:  Get GEOAPPL Description.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_detailInfo::geoApplDesc(Indicator i) const
{
  if (i == '1')
    return std::string("USA");
  else if (i == '2')
    return std::string("USP");
  else if (i == '3')
    return std::string("USI");
  else if (i == '4')
    return std::string("USPI");
  else
    return "";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::absorbTypeDesc
//
// Description:  Get Absorbtion Type Description.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_detailInfo::absorbTypeDesc(Indicator i) const
{
  if (i == '1')
    return std::string("DO NOT ABSORB");
  else if (i == '2')
    return std::string("ONLY PFC CONNECTION/STOPOVER");
  else if (i == '3')
    return std::string("ONLY PFC CONNECTION POINT");
  else if (i == '4')
    return std::string("ONLY PFC ORIGIN OR FARE BREAK");
  else
    return "";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::owrtDesc
//
// Description:  OW/RT Description.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_detailInfo::owrtDesc(Indicator i) const
{
  if (i == '1')
    return std::string("OW");
  else if (i == '2')
    return std::string("RT");
  else if (i == ' ')
    return std::string("BOTH");
  else
    return "";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA_diffInfo::PfcDisplayBuilderPXA_diffInfo
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXA_diffInfo::PfcDisplayBuilderPXA_diffInfo(TaxTrx* trx, PfcDisplayData* data)
  : PfcDisplayBuilderPXA_detailInfo(trx, data)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA_diffInfo::~PfcDisplayBuilderPXA_diffInfo
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXA_diffInfo::~PfcDisplayBuilderPXA_diffInfo() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilder::build
//
// Description:  PXA diff entry build.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_diffInfo::build()
{
  std::string firstCarrierInfo = buildHeader() + buildBody() + buildFootnote();

  PfcDisplayBuilderPXA_detailInfo::data()->releasePfcAbsorbData();
  PfcDisplayBuilderPXA_detailInfo::data()->setCarrier(PfcDisplayDataPXA::SECOND_CARRIER);

  std::string secondCarrierInfo = buildBody() + buildFootnote();

  if (firstCarrierInfo.empty() || secondCarrierInfo.empty() ||
      firstCarrierInfo == PfcDisplayErrorMsg::DATA_NOT_FOUND ||
      secondCarrierInfo == PfcDisplayErrorMsg::DATA_NOT_FOUND)
  {
    return PfcDisplayErrorMsg::DATA_NOT_FOUND;
  }
  else
  {

    return PfcDisplayBuilderPXA_detailInfo::data()->getAxessPrefix() + firstCarrierInfo +
           std::string(59, 'X') + "\n \n" + TABLE_HEADER1 + TABLE_HEADER2 + secondCarrierInfo;
  }
}

const std::string PfcDisplayBuilderPXA_itineraryInfo::INFO_HEADER =
    "            DISPLAY BELOW APPLIES AS GUIDELINE\n"
    "     TO DETERMINE FARE AND TAX AMOUNTS TO BE DEDUCTED\n"
    "      OR ANY RULES AND RESTRICTIONS WHICH MAY APPLY\n"
    "           TO SPECIFIC AIRPORT - ENTER ENTRY BELOW\n";

const std::string PfcDisplayBuilderPXA_itineraryInfo::HELP_HEADER = "PXA*AIRPORT CODE/CARRIER";

const std::string PfcDisplayBuilderPXA_itineraryInfo::TABLE_HEADER =
    "ABSORPTION  EFFECTIVE  DISCONTINUE  ABSORPTION\n"
    "AIRPORT     DATE       DATE         CARRIER";

const std::string PfcDisplayBuilderPXA_itineraryInfo::TABLE_FORMATTING =
    "ABSORPTION  EFFECTIVE  DISCONTINUE     ORPTION\n"
    "AIRPORT     DATE       DATE            XXXX";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::PfcDisplayBuilderPXA
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXA_itineraryInfo::PfcDisplayBuilderPXA_itineraryInfo(TaxTrx* trx,
                                                                       PfcDisplayData* data)
  : PfcDisplayBuilderPXA(trx, data), _formatter(PfcDisplayFormatterPXA(TABLE_FORMATTING))
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::~PfcDisplayBuilderPXA
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXA_itineraryInfo::~PfcDisplayBuilderPXA_itineraryInfo() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::buildHeader
//
// Description:  PXA*I entry header.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_itineraryInfo::buildHeader()
{
  return fmt().center(MAIN_HEADER + " \n" + MAIN_HEADER_PXA) + " \n" + (INFO_HEADER) + " \n" +
         fmt().margin(HELP_HEADER, 12) + " \n" + TABLE_HEADER + "\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::buildBody
//
// Description:  PXA*I entry body.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXA_itineraryInfo::buildBody()
{
  std::string body;

  const std::vector<PfcAbsorb*>& pfcAbsorbV = data()->getPfcAbsorb();

  if (pfcAbsorbV.empty())
  {
    return body;
  }

  body = " \n";

  std::string arpt;
  std::string carrier;
  std::string effDate;
  std::string discDate;
  std::string discDateCurrent;

  std::vector<PfcAbsorb*>::const_iterator it = pfcAbsorbV.begin();
  std::vector<PfcAbsorb*>::const_iterator itEnd = pfcAbsorbV.end();

  for (; it < itEnd; it++)
  {
    if (!(*it)->expireDate().isValid() && !(*it)->effDate().isValid())
    {
      discDateCurrent = EMPTY_STRING();
    }
    else
    {
      discDateCurrent = fmt().toString((*it)->discDate());
    }

    if (data()->ifNotEqualitySet(std::string((*it)->pfcAirport()),
                                 std::string((*it)->localCarrier()),
                                 fmt().toString((*it)->effDate()),
                                 discDateCurrent,
                                 arpt,
                                 carrier,
                                 effDate,
                                 discDate))
    {
      body += fmt().line(arpt, effDate, discDate, carrier);
    }
  }

  return body;
}
