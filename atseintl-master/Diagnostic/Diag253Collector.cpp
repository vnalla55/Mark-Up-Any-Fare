//----------------------------------------------------------------------------
//  Copyright Sabre 2005
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

#include "Diagnostic/Diag253Collector.h"

#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/DiagRequest.h"
#include "DBAccess/AddonCombFareClassInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"

#include <iomanip>

namespace tse
{
void
Diag253Collector::writeTariffXRefHeader(const VendorCode& vendor)
{
  _skipVendorCarrier = !_active || !_diagRequest->isValidForDiag(vendor);

  if (_skipVendorCarrier)
    return;

  writeCommonHeader(vendor);

  (*this) << " \nTARIFF XREF FOR " << _cJob->carrier() << '/' << vendor << '\n' << SEPARATOR
          << "SPEC TAR/CODE   ADDON1 TAR/CODE       ADDON2 TAR/CODE  FD\n" << SEPARATOR;
}

void
Diag253Collector::writeTariffXRefRef(const TariffCrossRefInfo& tariffCrossRef, const bool isFD)
{
  if (_skipVendorCarrier)
    return;

  if (!_diagRequest->isValidForDiag(tariffCrossRef))
    return;

  Diag253Collector& dc = *this;

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc << std::setw(4) << tariffCrossRef.fareTariff() << ' ';

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(8) << tariffCrossRef.fareTariffCode() << "     ";

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc << std::setw(4) << tariffCrossRef.addonTariff1() << ' ';

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(8) << tariffCrossRef.addonTariff1Code() << "         ";

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc << std::setw(4) << tariffCrossRef.addonTariff2() << ' ';

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(8) << tariffCrossRef.addonTariff2Code();
  dc << (isFD ? "  FD" : "    ") << '\n';
}

void
Diag253Collector::writeTariffXRefFooter()
{
  if (_skipVendorCarrier)
    return;

  (*this) << SEPARATOR;
}

void
Diag253Collector::writeCombFareClassHeader(const VendorCode& vendor)
{
  _skipVendorCarrier = !_active || !_diagRequest->isValidForDiag(vendor);

  if (_skipVendorCarrier)
    return;

  writeCommonHeader(vendor);

  (*this) << " \nFARE CLASS COMBINATIONS FOR " << _cJob->carrier() << '/' << vendor << '\n'
          << SEPARATOR << "FARETARIFF   FARECLASS   ADDON-FARECLASS    GEOAPPL  OWRT\n"
          << SEPARATOR;
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

void
Diag253Collector::writeCombFareClass(const AddonCombFareClassInfo& combFareClass,
                                     TariffNumber fareTariff)
{
  if (_skipVendorCarrier)
    return;

  if (!_diagRequest->isValidForDiag(combFareClass))
    return;

  Diag253Collector& dc = *this;

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc << std::setw(5) << fareTariff << "          ";

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(16) << combFareClass.fareClass()
     << /* 1 char */ combFareClass.addonFareClass() << std::setw(15) << "*****" << std::setw(8)
     << combFareClass.geoAppl() << std::setw(8) << combFareClass.owrt() << '\n';
}

#else

void
Diag253Collector::writeCombFareClass(const AddonCombFareClassInfo& combFareClass)
{
  if (_skipVendorCarrier)
    return;

  if (!_diagRequest->isValidForDiag(combFareClass))
    return;

  Diag253Collector& dc = *this;

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc << std::setw(5) << combFareClass.fareTariff() << "          ";

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(16) << combFareClass.fareClass() << std::setw(16)
     << combFareClass.addonFareClass() << std::setw(8) << combFareClass.geoAppl() << std::setw(8)
     << combFareClass.owrt() << '\n';
}

#endif

void
Diag253Collector::writeCombFareClassFooter()
{
  if (_skipVendorCarrier)
    return;

  (*this) << SEPARATOR;
}
}
