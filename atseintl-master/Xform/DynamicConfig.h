// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include <string>

namespace tse
{

class Trx;

class DynamicConfigInput
{
public:
  DynamicConfigInput();

  // The following are descriptions of <DynamicConfig /> tag attributes.

  // String: The localization of config value to override.
  //
  // The [SEP] is '\', '/', '::' if Substitute is true, '\' otherwise (see below).
  // There are 3 formats of this attribute:
  //   NAME: A value in the default group (FALLBACK_SECTION).
  //   GROUP [SEP] NAME: A value in a specific group.
  //   GROUP [SEP] NAME [SEP] VALUE: Same as above, but the value is already given in the Name
  //       attribute. This should match the FCCONFIG diagnostic output. With this format,
  //       the following Value attribute MUST BE empty.
  const std::string& name() const { return _name; }
  void setName(const std::string& value) { _name = value; }

  // String: The overriding value.
  //
  // Should be empty when using 2 separators in the Name attribute (see above).
  const std::string& value() const { return _value; }
  void setValue(const std::string& value) { _value = value; }

  // Bool (default: true): Whether to substitute slashes, spaces and double colons.
  //
  // When true, '/' and '::' characters are replaced with '\', and ' ' with '_'.
  // This way, one can easily copy the configuration line from FCCONFIG diagnostic.
  // In general, it shouldn't be necessary to override this attribute.
  const std::string& substitute() const { return _substitute; }
  void setSubstitute(const std::string& value) { _substitute = value; }

  // Bool (default: false): Whether to suppress errors or not.
  //
  // When true, any errors (like non-existent fallback) cause the override to be ignored instead
  // of aborting the whole transaction.
  const std::string& optional() const { return _optional; }
  void setOptional(const std::string& value) { _optional = value; }

private:
  std::string _name, _value, _substitute, _optional;
};

class DynamicConfigHandler
{
  static const std::string DEFAULT_GROUP;

public:
  DynamicConfigHandler(Trx& trx) : _trx(trx), _substitute(true), _optional(false) {}

  bool check();
  void process(const DynamicConfigInput& input);

private:
  Trx& _trx;

  bool _substitute, _optional;
  std::string _group, _name, _value;

  void parse(const DynamicConfigInput& input);
  void apply();

  void parseNameValue(const DynamicConfigInput& input);
  void parseSubstitute(const DynamicConfigInput& input);
  void parseOptional(const DynamicConfigInput& input);
};

}

