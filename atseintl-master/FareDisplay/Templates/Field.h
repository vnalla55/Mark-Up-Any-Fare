//-------------------------------------------------------------------
//
//  File:        Field.h
//  Created:     July 10, 2005
//  Authors:     Mike Carroll
//  Description: Base class for a display field.
//
//  Updates:
//
//  Copyright Sabre 2003
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

#include <stdarg.h>
#include <sstream>
#include "Common/TseConsts.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "FareDisplay/Templates/TemplateEnums.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{

class Field
{
public:
  typedef const std::string& (PaxTypeFare::*PFS)() const;
  typedef const int32_t& (PaxTypeFare::*PFI)() const;
  typedef const MoneyAmount& (PaxTypeFare::*PFMoneyAmount)() const;
  typedef const bool& (PaxTypeFare::*PFBool)() const;

  virtual ~Field() = default;

  //--------------------------------------------------------------------------
  // @function Field::getData
  //
  // Description: Get int32_t value from the instance PFI
  //
  // @param anInstance - class instance
  // @param func - pointer to a function returning an int32_t
  //--------------------------------------------------------------------------
  template <class T>
  void getData(T& anInstance, PFI func);

  //--------------------------------------------------------------------------
  // @function Field::getData
  //
  // Description: Get string value from the instance PFS
  //
  // @param anInstance - class instance
  // @param func - pointer to a function returning an string
  //--------------------------------------------------------------------------
  template <class T>
  void getData(T& anInstance, PFS func);

  //--------------------------------------------------------------------------
  // @function Field::getData
  //
  // Description: Get MoneyAmount value from the instance PFMoneyAmount
  //
  // @param anInstance - class instance
  // @param func - pointer to a function returning a MoneyAmount
  //--------------------------------------------------------------------------
  template <class T>
  void getData(T& anInstance, PFMoneyAmount func);

  //--------------------------------------------------------------------------
  // @function Field::getData
  //
  // Description: Get bool value from the instance PFBool
  //
  // @param anInstance - class instance
  // @param func - pointer to a function returning a bool
  //--------------------------------------------------------------------------
  template <class T>
  void getData(T& anInstance, PFBool func);

  //--------------------------------------------------------------------------
  // @function Field::formatData
  //
  // Description: Format field data
  //
  // @param none
  //--------------------------------------------------------------------------
  virtual void formatData() = 0;

  //--------------------------------------------------------------------------
  // @function Field::render
  //
  // Description: Render this field
  //
  // @param oss - Where to render field
  // @param valueType - type of value to render
  //--------------------------------------------------------------------------
  void render(std::ostringstream* oss, const FieldValueType& valueType);

  //--------------------------------------------------------------------------
  // @function Field::renderLabel
  //
  // Description: Render this field label
  //
  // @param oss - Where to render field
  //--------------------------------------------------------------------------
  void renderLabel(std::ostringstream* oss);

  // Accessors
  const int16_t& labelPosition() const { return _labelPosition; }
  int16_t& labelPosition() { return _labelPosition; }

  const std::string& label() const { return _label; }
  std::string& label() { return _label; }

  const int16_t& valuePosition() const { return _valuePosition; }
  int16_t& valuePosition() { return _valuePosition; }

  const bool& boolValue() const { return _boolValue; }
  bool& boolValue() { return _boolValue; }

  const std::string& strValue() const { return _strValue; }
  std::string& strValue() { return _strValue; }

  const int32_t& intValue() const { return _intValue; }
  int32_t& intValue() { return _intValue; }

  const MoneyAmount& moneyValue() const { return _moneyValue; }
  MoneyAmount& moneyValue() { return _moneyValue; }

  const int16_t& valueFieldSize() const { return _valueFieldSize; }
  int16_t& valueFieldSize() { return _valueFieldSize; }

  const JustificationType& justify() const { return _justify; }
  JustificationType& justify() { return _justify; }

  const std::string& strValue2() const { return _strValue2; }
  std::string& strValue2() { return _strValue2; }

protected:
  int16_t _labelPosition = 0; // Start position (0 based) in row of label
  std::string _label = EMPTY_STRING(); // Label value
  int16_t _valuePosition = 0; // Start position (0 based) in row of data value
  bool _boolValue = false; // Boolean values value
  std::string _strValue = EMPTY_STRING(); // String value's value
  int32_t _intValue = 0; // Int value's value
  MoneyAmount _moneyValue = 0.0; // Money amounts value
  int16_t _valueFieldSize = 0; // Values field size
  JustificationType _justify =
      JustificationType::LEFT; // Value's justification, all labels are LEFT
  // justified

  std::string _strValue2; // String value's value (two line displays)
};

} // namespace tse

