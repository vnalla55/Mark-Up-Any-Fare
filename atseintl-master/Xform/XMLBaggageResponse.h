#pragma once

#include "Xform/XMLWriter.h"

namespace tse
{
class BaggageCharge;
class BaggageTravel;
class FarePath;
class OptionalServicesInfo;

class XMLBaggageResponse
{
  friend class XMLBaggageResponseTest;

public:
  XMLBaggageResponse(XMLWriter& writer);

  void generateBDI(const FarePath& farePath);

private:
  void generateAllowanceBDI(const BaggageTravel& bt);
  void generateChargeBDI(const BaggageTravel& bt);
  void generateAllowanceAttributes(XMLWriter::Node& node, const OptionalServicesInfo& s7) const;
  void generateChargeAttributes(XMLWriter::Node& node, const BaggageCharge& bc) const;
  void generateQ00(const BaggageTravel& bt);

  XMLWriter& _writer;
};

}
