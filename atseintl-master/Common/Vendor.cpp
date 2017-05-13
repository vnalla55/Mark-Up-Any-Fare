//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/Vendor.h"

namespace tse
{

const VendorCode Vendor::ATPCO = "ATP";
const VendorCode Vendor::SITA = "SITA";
const VendorCode Vendor::SABRE = "SABR";
const VendorCode Vendor::FMS = "FMS";
const VendorCode Vendor::WN = "WN";
const VendorCode Vendor::POFO = "POFO";
const VendorCode Vendor::SABD = "SABD";
const VendorCode Vendor::HIKE = "HIKE";
const VendorCode Vendor::SMFO = "SMFO";
const VendorCode Vendor::EMPTY = "";

const char
Vendor::displayChar(const VendorCode& vendorCode)
{
  if (vendorCode == ATPCO)
    return 'A';
  else if (vendorCode == SITA)
    return 'S';
  else if (vendorCode == SABRE)
    return 'S';
  else if (vendorCode == FMS)
    return 'F';
  else if (vendorCode == WN)
    return 'W';
  else if (vendorCode == POFO)
    return 'P';
  else if (vendorCode == SABD)
    return 'D';
  else if (vendorCode == HIKE)
    return 'H';
  else if (vendorCode == SMFO)
    return 'O';
  else
    return '*';
}

} // namespace tse
