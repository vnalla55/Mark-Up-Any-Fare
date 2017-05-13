#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

from ..exceptions import ScommonError

class ToolkitError(ScommonError): pass


class LinkhelperError(ToolkitError): pass
class NoLibsError(LinkhelperError): pass
class DuplicateKeyError(LinkhelperError): pass


class MakefileSourcesError(ToolkitError): pass
class UnknownVariableError(MakefileSourcesError): pass
class BadInputFileError(MakefileSourcesError): pass


class MetabuilderError(ToolkitError): pass


# sdselector
class HandOver(ToolkitError): pass

