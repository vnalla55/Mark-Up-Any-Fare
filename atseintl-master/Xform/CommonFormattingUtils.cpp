//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Xform/CommonFormattingUtils.h"

namespace tse
{
namespace xform
{

namespace
{

std::string U(const std::string& s)
{
  return boost::to_upper_copy(s);
}

}

template <>
void addAttribute<XMLConstruct>(XMLConstruct& construct,
                                const std::string& key,
                                const std::string& value)
{
  construct.addAttribute(key, value);
}

template <>
void addAttribute<XMLWriter::Node>(XMLWriter::Node& construct,
                                   const std::string& key,
                                   const std::string& value)
{
  construct.convertAttr(key, value);
}

template <>
void addAttribute<DiagCollector>(DiagCollector& construct,
                                 const std::string& key,
                                 const std::string& value)
{
  construct << key << "=" << value << std::endl;
}

template <>
void addAttribute<std::vector<std::string>>(std::vector<std::string>& construct,
                                            const std::string& key,
                                            const std::string& value)
{
  std::string tmp(key);
  tmp.append("=");
  tmp.append(value);
  construct.push_back(tmp);
}

template <typename T>
bool formatBrandProgramData(T& construct,
                            const QualifiedBrand& qualifiedBrand,
                            const BrandCode& fareBrandCode)
{
  const BrandProgram* brandProgram = qualifiedBrand.first;
  const BrandInfo* brandInfo = qualifiedBrand.second;
  if (!brandProgram || !brandInfo)
    return false;

  std::stringstream systemCode;
  if (brandProgram->dataSource() == BRAND_SOURCE_CBAS)
  {
    //in CBAS response system code is in programCode, and systemCode field is absent
    systemCode << brandProgram->programCode();
  }
  else
  {
    systemCode << brandProgram->systemCode();
  }

  if (fareBrandCode.empty())
    addAttribute(construct, xml2::BrandCode, U(brandInfo->brandCode()));
  else
    addAttribute(construct, xml2::BrandCode, U(fareBrandCode));

  addAttribute(construct, xml2::BrandDescription, U(brandInfo->brandName()));
  addAttribute(construct, xml2::ProgramName, U(brandProgram->programName()));
  addAttribute(construct, xml2::SystemCode, U(systemCode.str()));
  addAttribute(construct, xml2::ProgramId, U(brandProgram->programID()));
  addAttribute(construct, xml2::ProgramCodeNew, U(brandProgram->programCode()));
  return true;
}

template bool formatBrandProgramData<XMLConstruct>(XMLConstruct& construct,
                                                   const QualifiedBrand& qualifiedBrand,
                                                   const BrandCode& fareBrandCode);

template bool formatBrandProgramData<XMLWriter::Node>(XMLWriter::Node& construct,
                                                      const QualifiedBrand& qualifiedBrand,
                                                      const BrandCode& fareBrandCode);

template bool formatBrandProgramData<DiagCollector>(DiagCollector& construct,
                                                    const QualifiedBrand& qualifiedBrand,
                                                    const BrandCode& fareBrandCode);

template bool formatBrandProgramData<std::vector<std::string>>(
    std::vector<std::string>& construct,
    const QualifiedBrand& qualifiedBrand,
    const BrandCode& fareBrandCode);

} // xform
} // tse

