#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

class ScommonError(Exception): pass

# factory
class LinktypeError(ScommonError): pass
class NoInputError(ScommonError): pass

# utils
class NoTestsError(ScommonError): pass

# env
class EnvironmentError(ScommonError): pass
class SconsRuntimeError(EnvironmentError): pass

