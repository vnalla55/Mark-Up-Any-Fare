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

#pragma once

#include "Common/TseStlTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/XMLConstruct.h"
#include "Diagnostic/DiagCollector.h"
#include "Xform/XMLCommonTags.h"
#include "Xform/XMLWriter.h"

#include <string>
#include <vector>

namespace tse
{
namespace xform
{

template <typename T>
void addAttribute(T& construct, const std::string& key, const std::string& value); // not defined

template <>
void addAttribute<XMLConstruct>(XMLConstruct& construct,
                                const std::string& key,
                                const std::string& value);

template <>
void addAttribute<XMLWriter::Node>(XMLWriter::Node& construct,
                                   const std::string& key,
                                   const std::string& value);

template <>
void addAttribute<DiagCollector>(DiagCollector& construct,
                                 const std::string& key,
                                 const std::string& value);

template <>
void addAttribute<std::vector<std::string>>(std::vector<std::string>& construct,
                                            const std::string& key,
                                            const std::string& value);

template <typename T>
bool formatBrandProgramData(T& construct,
                            const QualifiedBrand& qualifiedBrand,
                            const BrandCode& fareBrandCode = BrandCode());

} // xform
} // tse

