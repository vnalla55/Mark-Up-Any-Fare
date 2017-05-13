//-------------------------------------------------------------------
//
//  File:        AddOnFaresSection.h
//  Created:     July 24, 2005
//  Authors:     Mike Carroll
//  Description: Used for display of addon fares and related fare information
//
//  Updates:
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "DBAccess/FareDispTemplateSeg.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "FareDisplay/FDAddOnFareController.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/Section.h"

#include <vector>

namespace tse
{
class FareDisplayTrx;

class AddOnFaresSection : public Section
{
public:
  enum class DisplayType : uint8_t
  { ALTERNATE_CURRENCY = 1,
    ALTERNATE_CARRIER };

  AddOnFaresSection(FareDisplayTrx& trx, std::vector<FareDispTemplateSeg*>* templateSegRecs)
    : Section(trx), _templateSegRecs(templateSegRecs)
  {
  }

  void buildDisplay() override;

  std::vector<ElementField*>& columnFields() { return _columnFields; }
  const std::vector<ElementField*>& columnFields() const { return _columnFields; }

  const std::vector<FareDispTemplateSeg*>& templateSegRecs() const { return *_templateSegRecs; }

private:
  std::vector<FareDispTemplateSeg*>* _templateSegRecs = nullptr;

  std::vector<ElementField*> _columnFields;

  class Display
  {
  public:
    Display(FareDisplayTrx& trx, AddOnFaresSection::DisplayType dType)
      : _displayType(dType), _trx(trx)
    {
    }

    void operator()(std::string item)
    {
      if (_curLineLen + item.size() + 1 > (uint16_t)MAX_PSS_LINE_SIZE)
      {
        _trx.response() << std::endl;
        _curLineLen = 0;
      }
      switch (_displayType)
      {
      case DisplayType::ALTERNATE_CURRENCY:
        _trx.response() << (_curLineLen > 0 ? SLASH : "") << item;
        break;
      case DisplayType::ALTERNATE_CARRIER:
        _trx.response() << item << SPACE;
        break;
      }
      _curLineLen += (item.size() + 1);
    }

  private:
    AddOnFaresSection::DisplayType _displayType;
    FareDisplayTrx& _trx;
    int16_t _curLineLen = 0;
  };

  void displayMoreFaresExistMessage(FareDisplayTrx& trx) const;
};
} // namespace tse
