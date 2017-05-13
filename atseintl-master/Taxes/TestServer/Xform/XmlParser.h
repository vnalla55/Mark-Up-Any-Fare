// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <iosfwd>
#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <rapidxml_wrapper.hpp>

namespace tax
{
class Request;
class InputRequest;
class XmlCache;
class XmlTagsList;

class XmlParser
{
public:
  XmlParser();
  ~XmlParser();
  XmlParser& parse(InputRequest& request, XmlCache& xmlCache, rapidxml::xml_document<> const& xml,
                   const XmlTagsList& tagsList);
  template <typename T> std::string getName();

private:
  template <typename T> bool
  parse(T& destination, rapidxml::xml_node<>* root); // node
  template <typename T> bool
  parse(boost::ptr_vector<T>& destination, rapidxml::xml_node<>* root); // node sequence
  template <typename T> bool
  parse(std::vector<T>& destination, rapidxml::xml_node<>* root); // node sequence
  template <typename T> bool
  parse(boost::optional<T>& destination, rapidxml::xml_node<>* root); // optional node
  template <typename T> bool
  parse(T& destination, rapidxml::xml_attribute<>* attribute,
        std::string const& name); // attribute
  template <typename T, typename Conv> bool
  parse(T& destination, rapidxml::xml_attribute<>* attribute, std::string const& name,
        Conv converter); // attribute

  template <typename T> bool parseAnyChild(T& destination, rapidxml::xml_node<>* node);
  template <typename T> bool parseAnyAttribute(T& destination, rapidxml::xml_attribute<>* node);
  const XmlTagsList* _l;
};

} // namespace tax

