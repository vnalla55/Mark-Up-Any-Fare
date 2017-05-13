//----------------------------------------------------------------------------
////
////  Copyright Sabre 2013
////
////      The copyright to the computer program(s) herein
////      is the property of Sabre.
////      The program(s) may be used and/or copied only with
////      the written permission of Sabre or in accordance
////      with the terms and conditions stipulated in the
////      agreement/contract under which the program(s)
////      have been supplied.
////
////----------------------------------------------------------------------------
#include "DataModel/GfrRecord2Key.h"

namespace tse
{

bool
GfrRecord2Key::lessKey::
operator()(GfrRecord2Key* const& key1, GfrRecord2Key* const& key2) const
{
  if (key1->_categoryNumber < key2->_categoryNumber)
    return true;
  if (key1->_categoryNumber > key2->_categoryNumber)
    return false;
  if (key1->_ruleNumber < key2->_ruleNumber)
    return true;
  if (key1->_ruleNumber > key2->_ruleNumber)
    return false;
  if (key1->_ruleTariff < key2->_ruleTariff)
    return true;
  if (key1->_ruleTariff > key2->_ruleTariff)
    return false;
  if (key1->_carrier < key2->_carrier)
    return true;
  if (key1->_carrier > key2->_carrier)
    return false;
  if (key1->_vendor < key2->_vendor)
    return true;
  if (key1->_vendor > key2->_vendor)
    return false;
  if (key1->_travelDate.get64BitRepDateOnly() < key2->_travelDate.get64BitRepDateOnly())
    return true;
  if (key1->_travelDate.get64BitRepDateOnly() > key2->_travelDate.get64BitRepDateOnly())
    return false;
  if (key1->_returnDate.get64BitRepDateOnly() < key2->_returnDate.get64BitRepDateOnly())
    return true;
  if (key1->_returnDate.get64BitRepDateOnly() > key2->_returnDate.get64BitRepDateOnly())
    return false;

  if (key1->_ticketDate.get64BitRepDateOnly() < key2->_ticketDate.get64BitRepDateOnly())
    return true;
  if (key1->_ticketDate.get64BitRepDateOnly() > key2->_ticketDate.get64BitRepDateOnly())
    return false;

  return false;
}

} // tse
