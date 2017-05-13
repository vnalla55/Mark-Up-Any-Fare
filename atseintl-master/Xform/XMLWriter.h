//----------------------------------------------------------------------------
//
//  File:               XMLWriter.h
//  Description:        interface for writing XML documents
//  Created:            01/14/2005
//  Authors:            David White
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/Code.h"

#include <sstream>
#include <stack>
#include <string>
#include <vector>

namespace tse
{

// class XMLWriter: a class used for outputting XML. After constructing
// an XMLWriter object, XML is output by constructing XMLWriter::Node
// objects and passing the XMLWriter object to their constructor.
// When all XML has been written, the result can be obtained from
// the 'result' method.
class XMLWriter
{
public:
  XMLWriter();
  ~XMLWriter();

  friend class Node;
  friend class SubDocument;

  // class Node: an instance of this class must be created for every
  // element in the XML document. The scope of the element is determined
  // by the lifetime of the object. A newly-created Node is the child
  // of the last Node to be created which hasn't yet been destroyed. If
  // there is no such Node, then the newly-created Node will be at the
  // root of the document. Scopes {} can be used to control the creation
  // and destruction of Nodes.
  //
  // It is advised that Nodes are created as automatic variables (on the
  // stack) so that C++ language rules will guarantee they are created
  // and destroyed in LIFO order. If they are created in a different
  // manner, then the caller must guarantee that they are created
  // and destroyed in LIFO order.
  //
  // Example: to create the XML document <root><attr name="value"/></root>
  // we could use,
  // Node root(writer,"root");
  // {
  //   Node attr(writer,"attr");
  //   attr.attr("name","value");
  // }
  class Node
  {
  public:
    Node(XMLWriter& writer, const std::string& element);
    ~Node();

    // function used to emit an attribute.
    // pre-condition: this Node may not yet have any text
    // or child nodes.
    XMLWriter::Node& attr(const std::string& name, const std::string& value);

    // specializations of convertAttr
    // for some types no need to convert
    XMLWriter::Node& convertAttr(const std::string& name, const std::string& value)
    {
      return attr(name, value);
    }
    template <size_t n>
    XMLWriter::Node& convertAttr(const std::string& name, const Code<n>& value)
    {
      return attr(name, value);
    }
    XMLWriter::Node& convertAttr(const std::string& name, const char& value)
    {
      std::string str(1, value);
      return attr(name, str);
    }

    // function used to emit an attribute.
    // works with any type that can be converted to a string
    // by being passed through a stringstream.
    template <typename T>
    XMLWriter::Node& convertAttr(const std::string& name, const T& value)
    {
      std::ostringstream stream;
      stream << value;
      return attr(name, stream.str());
    }

    // function to add text as a child of this node.
    // pre-condition: this Node may not yet have any child nodes.
    XMLWriter::Node& addText(const std::string& text);

    XMLWriter::Node& addSimpleText(const std::string& text);

    // like addText(), but adds it as CDATA
    XMLWriter::Node& addData(const std::string& text);

    // function to add text as a child of this node
    // does not escape inserted text
    XMLWriter::Node& addRawText(const std::string& text);

    XMLWriter::Node& enterNode();

  private:
    Node(const Node&);
    void operator=(const Node&);

    XMLWriter& _writer;
  };

  class SubDocument
  {
  public:
    SubDocument(XMLWriter& writer) : _writer(writer) { _writer.openSubDocument(); }

    ~SubDocument() { _writer.closeSubDocument(); }

    std::string result() const { return _writer.result(); }
    void result(std::string& res) { _writer.result(res); }

  private:
    XMLWriter& _writer;
  };

  std::string result() const;
  void result(std::string& res);

private:
  void startNode(const std::string& name);
  void addAttribute(const std::string& name, const std::string& value);
  void enterNode();
  void addText(const std::string& text);
  void addSimpleText(const std::string& text);
  void addData(const std::string& text);
  void addRawText(const std::string& text);
  void endNode();

  static const std::string& escapeString(const std::string& str, std::string& buf);

  void openSubDocument();
  void closeSubDocument();

  std::stack<std::string> _streams;
  std::vector<std::string> _openElements;
  bool _inStartElement;
  bool _simpleString;
};
}

