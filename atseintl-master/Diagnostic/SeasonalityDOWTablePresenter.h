//-------------------------------------------------------------------
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

#pragma once

#include "DBAccess/SeasonalityDOW.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Common/TseCodeTypes.h"

namespace tse
{

class SeasonalityDOWPresenter : std::unary_function<const SeasonalityDOW*, void>
{
public:
  SeasonalityDOWPresenter(DiagCollector& dc, RexPricingTrx& trx) : _dc(dc), _trx(trx) {}

  void
  printSeasonalityDOW(const VendorCode& vendor, uint32_t tblItemNo, const DateTime& applicationDate)
  {
    if (tblItemNo)
    {
      const SeasonalityDOW* dow = _trx.getSeasonalityDOW(vendor, tblItemNo, applicationDate);

      if (dow)
        print(*dow);
    }

    else
      printEmptySeasonalityDOW();
  }

  void print(const SeasonalityDOW& sdow)
  {
    _dc << "SEASONALITY DOW TBL: " << std::setw(5) << sdow.itemNo() << "\n"
        << "A-" << static_cast<char>(sdow.indA()) << " B-" << static_cast<char>(sdow.indB())
        << " C-" << static_cast<char>(sdow.indC()) << " D-" << static_cast<char>(sdow.indD())
        << " E-" << static_cast<char>(sdow.indE()) << " F-" << static_cast<char>(sdow.indF())
        << " G-" << static_cast<char>(sdow.indG()) << " H-" << static_cast<char>(sdow.indH())
        << " I-" << static_cast<char>(sdow.indI()) << " J-" << static_cast<char>(sdow.indJ())
        << " K-" << static_cast<char>(sdow.indK()) << " L-" << static_cast<char>(sdow.indL())
        << " M-" << static_cast<char>(sdow.indM()) << "\n"
        << "N-" << static_cast<char>(sdow.indN()) << " O-" << static_cast<char>(sdow.indO())
        << " P-" << static_cast<char>(sdow.indP()) << " Q-" << static_cast<char>(sdow.indQ())
        << " R-" << static_cast<char>(sdow.indR()) << " S-" << static_cast<char>(sdow.indS())
        << " T-" << static_cast<char>(sdow.indT()) << " U-" << static_cast<char>(sdow.indU())
        << " V-" << static_cast<char>(sdow.indV()) << " W-" << static_cast<char>(sdow.indW())
        << " X-" << static_cast<char>(sdow.indX()) << " Y-" << static_cast<char>(sdow.indY())
        << " Z-" << static_cast<char>(sdow.indZ()) << "\n";
  }

private:
  void printEmptySeasonalityDOW()
  {
    SeasonalityDOW empty;
    empty.itemNo() = 0;
    empty.indA() = ' ';
    empty.indB() = ' ';
    empty.indC() = ' ';
    empty.indD() = ' ';
    empty.indE() = ' ';
    empty.indF() = ' ';
    empty.indG() = ' ';
    empty.indH() = ' ';
    empty.indI() = ' ';
    empty.indJ() = ' ';
    empty.indK() = ' ';
    empty.indL() = ' ';
    empty.indM() = ' ';
    empty.indN() = ' ';
    empty.indO() = ' ';
    empty.indP() = ' ';
    empty.indQ() = ' ';
    empty.indR() = ' ';
    empty.indS() = ' ';
    empty.indT() = ' ';
    empty.indU() = ' ';
    empty.indV() = ' ';
    empty.indW() = ' ';
    empty.indX() = ' ';
    empty.indY() = ' ';
    empty.indZ() = ' ';
    print(empty);
  }

  DiagCollector& _dc;
  RexPricingTrx& _trx;
};
}

