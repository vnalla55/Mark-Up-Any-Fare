#! /usr/bin/env python2.5

import unittest
from release_parser import ReleaseNoteFileParser, ReleaseNoteFileDiffer


class ReleaseNoteFileDifferTest(unittest.TestCase):
    def setUp(self):
        self._differ = ReleaseNoteFileDiffer()

    def testEmptyTrees(self):
        res = self._differ.compare({}, {})
        self.assertFalse(res)

    def testNewGroup(self):
        first = {}
        second = {'newGroup' : ["item1"]}

        res = self._differ.compare(first, second)
        self.assertEqual(len(res), 1)
        self.assertTrue('newGroup' in res)
        self.assertEqual(len(res['newGroup']), 1)

    def testRemovedGroup(self):
        first = {'oldGroup' : ["item1"]}
        second = {}

        res = self._differ.compare(first, second)
        self.assertFalse(res)

    def testSameGroup(self):
        first = {'oldGroup' : ["item1"]}
        second = {'oldGroup' : ["item1"]}

        res = self._differ.compare(first, second)
        self.assertFalse(res)

    def testChangedOrder(self):
        first = {'group1' : ["item1"], 'group2' : ["item2"]}
        second = {'group2' : ["item2"], 'group1' : ["item1"]}

        res = self._differ.compare(first, second)
        self.assertFalse(res)

    def testNonEmptyList(self):
        first = {}
        second = {'newGroup' : ["item1", "item2"] }

        res = self._differ.compare(first, second)
        self.assertEqual(len(res), 1)
        self.assertTrue('newGroup' in res)
        self.assertEqual(len(res['newGroup']), 2)
        self.assertEqual(res['newGroup'][0], "item1")
        self.assertEqual(res['newGroup'][1], "item2")

    def testNewItemsInGroup(self):
        first = {'sameGroup': ["item1"] }
        second = {'sameGroup': ["item1", "item2"] }

        res = self._differ.compare(first, second)
        self.assertEqual(len(res['sameGroup']), 1)
        self.assertEqual(res['sameGroup'][0], "item2")
    
    def testRemovedItemInGroup(self):
        first = {'sameGroup': ["item1"] }
        second = {'sameGroup': [] }

        res = self._differ.compare(first, second)
        self.assertFalse(res)

    def testSameItemsInGroup(self):
        first = {'sameGroup': ["item1"] }
        second = {'sameGroup': ["item1"] }

        res = self._differ.compare(first, second)
        self.assertFalse(res)

    def testChangedItemOrder(self):
        first = {'sameGroup' : ["item1", "item2"]}
        second = {'sameGroup' : ["item2", "item1"]}

        res = self._differ.compare(first, second)
        self.assertFalse(res)

class ReleaseNoteFileParserTest(unittest.TestCase):
    def setUp(self):
        self._parser = ReleaseNoteFileParser()

    def test_empty_file(self):
        fcontent = """

        """
        tree = self._parser.parse(fcontent)
        self.assertFalse(tree) # empty dict

    def test_single_group(self):
        fcontent = """
[ABCD]
"""
        tree = self._parser.parse(fcontent)
        self.assertEqual(len(tree), 1)
        self.assertTrue("ABCD" in tree)
        self.assertFalse(tree["ABCD"]) # empty

    def test_multi_groups(self):
        fcontent = """
[ABCD]

[BCDEF]

[FG EE]
        """
        tree = self._parser.parse(fcontent)
        self.assertEqual(len(tree), 3)
        self.assertTrue("ABCD" in tree)
        self.assertTrue("BCDEF" in tree)
        self.assertTrue("FG EE" in tree)

        self.assertFalse(tree["ABCD"]) # empty
        self.assertFalse(tree["BCDEF"]) # empty
        self.assertFalse(tree["FG EE"]) # empty

    def test_two_sectioned_group(self):
        fcontent = """
[ABCD]

[BCDEF]

[ABCD]
        """
        tree = self._parser.parse(fcontent)
        self.assertEqual(len(tree), 2)
        self.assertTrue("ABCD" in tree)
        self.assertTrue("BCDEF" in tree)

        self.assertFalse(tree["ABCD"]) # empty
        self.assertFalse(tree["BCDEF"]) # empty

    def test_items_in_groups(self):
        fcontent = """
[ABCD]

item1-line1
item1-line2
item1: different style

item2-line1
item2: sdfdfsdf

[BCDEF]

item3-line1
item2-line3
"""

        tree = self._parser.parse(fcontent)

        self.assertEqual(len(tree["ABCD"]), 2) 
        self.assertEqual(tree["ABCD"][0], 
"""item1-line1
item1-line2
item1: different style
""")

        self.assertEqual(tree["ABCD"][1], 
"""item2-line1
item2: sdfdfsdf
""")

        self.assertEqual(len(tree["BCDEF"]), 1)

        self.assertEqual(tree["BCDEF"][0], 
"""item3-line1
item2-line3
""")

    def test_items_in_separate_groups(self):
        fcontent = """
[ABCD]

item2-line1
item2: sdfdfsdf

[ABCD]

item3-line1
item2-line3
"""
        tree = self._parser.parse(fcontent)
        self.assertEqual(len(tree["ABCD"]), 2)

    def test_item_with_indentation(self):
        fcontent = """
[ABCD]

item2-line1
  subitem2: sdfdfsdf
  subitem3
item2-line2

nextitem
  xx
"""
        tree = self._parser.parse(fcontent)
        self.assertEqual(len(tree["ABCD"]), 2)

    def test_default_group(self):
        fcontent = """
item2-line1
  subitem2: sdfdfsdf
  subitem3
item2-line2

nextitem
  xx
"""
        tree = self._parser.parse(fcontent)
        self.assertTrue("*" in tree)
        self.assertEqual(len(tree["*"]), 2)

    def test_blank_lines_behaviour(self):
        fcontent = """

[ABCD]
item1

[BCDE]

item2

item3
[CDEF]
item4
[DEFG]
[EFGH]"""
        tree = self._parser.parse(fcontent)

        self.assertEqual(len(tree), 5)
        self.assertEqual(len(tree["ABCD"]), 1)
        self.assertEqual(len(tree["BCDE"]), 2)
        self.assertEqual(len(tree["CDEF"]), 1)
        self.assertEqual(len(tree["DEFG"]), 0)
        self.assertEqual(len(tree["EFGH"]), 0)

    def test_whitespaces_at_endings_may_vary(self):
        fcontent = """
[ABCD]
item1  \t"""

        tree = self._parser.parse(fcontent)

        self.assertEqual(len(tree), 1)
        self.assertEqual(len(tree["ABCD"]), 1)
        self.assertEqual(tree["ABCD"][0], "item1\n")

if __name__ == '__main__':
    unittest.main()