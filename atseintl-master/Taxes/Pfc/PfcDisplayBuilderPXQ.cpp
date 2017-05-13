// --------------------------------------------------------------------------
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

#include "Taxes/Pfc/PfcDisplayBuilderPXQ.h"

using namespace tse;

const std::string PfcDisplayBuilderPXQ::TABLE_HEADER =
    "EQUIP      NBR OF     EFFECTIVE   DISCONTINUE   STATE";
const std::string PfcDisplayBuilderPXQ::TABLE_HEADER2 =
    "TYPE       SEATS      DATE        DATE          LOCATION";
const std::string PfcDisplayBuilderPXQ::TABLE_FIELDPOS =
    "XXXX         XXX      XXXX        XXXX          XXXXXXXX";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXQ::PfcDisplayBuilderPXQ
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXQ::PfcDisplayBuilderPXQ(TaxTrx* trx, PfcDisplayData* data)
  : PfcDisplayBuilder(trx, data), _formatter(PfcDisplayFormatterPXQ(TABLE_FIELDPOS))
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::~PfcDisplayBuilderPXC
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderPXQ::~PfcDisplayBuilderPXQ() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXC::buildHeader
//
// Description:  PXC entry header.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXQ::buildHeader()
{
  std::string header = "                  PASSENGER FACILITY CHARGES\n \n                ESSENTIAL "
                       "EQUIPMENT EXEMPTIONS\n \n";
  header += TABLE_HEADER + "\n" + TABLE_HEADER2 + "\n \n";
  return header;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXCQ:buildBody
//
// Description:  PXC entry body.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderPXQ::buildBody()
{
  /// reference to vector of equip exemts from DB
  const std::vector<PfcEquipTypeExempt*>& vec = data()->getAllEquipmentExempt();
  if (vec.empty())
    return std::string();

  std::vector<PfcEquipTypeExempt*>::const_iterator itBegin = vec.begin();
  std::vector<PfcEquipTypeExempt*>::const_iterator it = itBegin;
  std::vector<PfcEquipTypeExempt*>::const_iterator itEnd = vec.end();

  /// contains whole body information
  std::string body;

  std::string equipType;
  std::string nbrOfSeats;
  std::string effDate;
  std::string discDate;
  std::string stateLocation;
  std::string state;
  std::string country = "US";

  /// loop through all items
  for (; it < itEnd; it++)
  {
    state = (*it)->state();

    equipType = (*it)->equip();
    nbrOfSeats = fmt().toString((*it)->noSeats());
    effDate = fmt().toString((*it)->effDate());
    discDate = fmt().toString((*it)->discDate());
    stateLocation = state.empty() ? state : country + state;

    body += fmt().line(equipType, nbrOfSeats, effDate, discDate, stateLocation);
  }

  return body;
}
