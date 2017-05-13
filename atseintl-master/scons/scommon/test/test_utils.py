#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import unittest
from mock import Mock, patch, call
import sconstestenv
import scommon.utils
import pytools

from textwrap import dedent
import os

class TestSplitSources(sconstestenv.Test):

    FILE = 'something.cpp'
    FILE_LIST = ['one.cpp', 'two.cpp', 'three.cpp']

    def up(self):
        self.sf.add_import('scommon.utils')
        #self.sf.add_env_creation()

    def test_single_file(self):
        self.sf.append("print 'S = %s' % scommon.utils.split_sources({filename})".format(filename=repr(self.FILE)))
        self.run_file()
        self.assert_output_contains('S = %s' % repr([self.FILE]))

    def test_file_list(self):
        self.sf.append("print 'S = %s' % scommon.utils.split_sources({filename})".format(filename=repr(self.FILE_LIST)))
        self.run_file()
        self.assert_output_contains('S = %s' % repr(self.FILE_LIST))

    def test_multiline(self):
        self.sf.append("MULTILINE = '''")
        for f in self.FILE_LIST:
            self.sf.append("    %s" % f)
        self.sf.append("'''")
        self.sf.append("print 'S = %s' % scommon.utils.split_sources(MULTILINE)")
        self.run_file()
        self.assert_output_contains('S = %s' % repr(self.FILE_LIST))


class TestNodePath(sconstestenv.Test):
    OBJECT_EXT = '.o'

    def up(self):
        self.sf.add_import('scommon.utils')

    def test_calculates_path_correctly(self):
        self.sf.append("o = Object(%s)" % repr(sconstestenv.SOURCE_NAME), 1)
        self.sf.append("print 'path = %s' % scommon.utils.node_path(o[0])")
        self.run_file()
        sourcepath = os.path.join(sconstestenv.get_build_dir(),
                sconstestenv.SOURCE_NAME)
        objpath = os.path.splitext(sourcepath)[0] + self.OBJECT_EXT
        self.assert_output_contains(objpath)


class TestSconsHome(sconstestenv.Test):
    def up(self):
        self.sf.add_import('scommon.utils')
        self.sf.add_env_creation()

    def test_calculates_scons_home_correctly(self):
        self.sf.append("print 'HOME =<%s>' % scommon.utils.get_scons_home(env)")
        self.run_file()
        self.assert_output_contains('<%s>' % sconstestenv.get_build_dir())


class TestMakefileSources(unittest.TestCase):
    MFSOURCES_DIR_NAME = 'mfsources'
    MFSOURCES_FILE_NAME = 'Makefile.sources'
    MFSOURCES_FULL_PATH = pytools.path_from_fdir(
            __file__, MFSOURCES_DIR_NAME, MFSOURCES_FILE_NAME)

    MS_CONTENT = {
        'XFORM_SOURCES': ['Xform.cpp'],

        'XFORM_CLIENT_XML_SOURCES': [
            'MileageModelMap.cpp',
            'CurrencyModelMap.cpp',
            'PricingDisplayModelMap.cpp',
            'PricingModelMap.cpp',
            'PricingDetailModelMap.cpp',
            'MileageContentHandler.cpp',
            'CurrencyContentHandler.cpp',
            'PricingDisplayContentHandler.cpp'],

        'XFORM_CLIENT_SHOPPING_XML_SOURCES': [
            'XMLRexShoppingResponse.cpp',
            'GenerateSIDForShortCutPricing.cpp',
            'XMLWriter.cpp',
            'BrandingResponseBuilder.cpp',
            'XMLBrandingResponse.cpp',
            'PreferredCabin.cpp'],

        'XFORM_CACHE_MESSAGE_SOURCES': [
            'XformCacheMessageXML.cpp',
            'CacheMessageContentHandler.cpp',
            'CacheMessageXMLParser.cpp']
    }

    NONEXISTENT_VARIABLE = 'A_VARIABLE_THAT_DOES_NOT_EXIST'

    def setUp(self):
        self.ms = scommon.utils.MakefileSources(self.MFSOURCES_FULL_PATH)

    def test_returns_filelist_correctly(self):
        for variable, expected_filelist in self.MS_CONTENT.iteritems():
            self.assertEqual(self.ms.filelist(variable), expected_filelist)

    def test_raises_on_unknown_variable(self):
        with self.assertRaises(scommon.utils.MakefileSources.UnknownVariable):
            self.ms.filelist(self.NONEXISTENT_VARIABLE)

    def test_raises_if_no_filename_and_no_text(self):
        with self.assertRaises(ValueError):
            scommon.utils.MakefileSources()

    def test_raises_on_empty_input_file(self):
        with self.assertRaisesRegexp(scommon.utils.MakefileSources.BadInputFile, 'empty'):
            self.ms = scommon.utils.MakefileSources(text='')

    def _assert_bad_input_file(self, text):
        with self.assertRaises(scommon.utils.MakefileSources.BadInputFile):
            scommon.utils.MakefileSources(text=text)

    def test_raises_if_no_assignment(self):
        text = dedent('''
            XFORM_CACHE_MESSAGE_SOURCES \
                XformCacheMessageXML.cpp \
                CacheMessageContentHandler.cpp \
                CacheMessageXMLParser.cpp
            ''')
        self._assert_bad_input_file(text)

    def test_raises_if_no_backslash(self):
        text = dedent('''
            XFORM_CACHE_MESSAGE_SOURCES := \
                XformCacheMessageXML.cpp \
                CacheMessageContentHandler.cpp
                CacheMessageXMLParser.cpp
            ''')
        self._assert_bad_input_file(text)

    def test_raises_if_empty_filelist(self):
        text = dedent('''
            EMPTY_SOURCES := \

            XFORM_CLIENT_XML_SOURCES := \
                MileageModelMap.cpp \
                CurrencyModelMap.cpp\
                PricingDisplayModelMap.cpp''')
        self._assert_bad_input_file(text)

    def test_raises_if_no_variable_name(self):
        text = dedent('''
            := \
                Niceone.cpp
            XFORM_CLIENT_XML_SOURCES := \
                MileageModelMap.cpp \
                CurrencyModelMap.cpp\
                PricingDisplayModelMap.cpp''')
        self._assert_bad_input_file(text)

    def test_raises_on_variable_duplicate(self):
        text = dedent('''
            XFORM_CLIENT_XML_SOURCES := \
                Something.cpp
            XFORM_CLIENT_XML_SOURCES := \
                MileageModelMap.cpp''')
        self._assert_bad_input_file(text)
