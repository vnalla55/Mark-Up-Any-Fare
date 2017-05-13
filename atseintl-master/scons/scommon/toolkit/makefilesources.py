#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

from .exceptions import UnknownVariableError, BadInputFileError

class BackslashConcatenator(object):
    BACKSLASH = '\\'
    SPACE = ' '

    def __init__(self):
        self.lines = [[]]

    def add_line(self, line):
        line = line.strip()
        if line.endswith(self.BACKSLASH):
            line = line[:-1]
            line = line.strip()
            self.lines[-1].append(line)
        else:
            self.lines[-1].append(line)
            self.lines.append([])

    def _join_chunks(self, lines):
        return [self.SPACE.join(verse) for verse in lines]

    def _eliminate_empty_lines(self, lines):
        return [l for l in lines if l.strip()]

    def get_lines(self):
        return self._eliminate_empty_lines(self._join_chunks(self.lines))


# Todo: if there is = instead of :=, raise a better
# exception than "multiple assignment operators" or sth
class MakefileSources(object):

    ASSIGNMENT_OPERATOR = ':='
    ASSIGNMENT_OPERATOR_SPLIT_LEN = 2

    def __init__(self, filename=None, text=None, concatenator_factory=None):
        text = self._retrieve_makefile_sources_text(filename, text)
        text = text.strip()
        if not text:
            raise BadInputFileError('Input file empty')
        self.concatenator_factory = BackslashConcatenator \
                if concatenator_factory is None else concatenator_factory
        self.content_dict = self._parse(text)

    def _retrieve_makefile_sources_text(self, filename, text):
        if text is not None:
            return text
        if filename is not None:
            return open(filename).read()
        raise ValueError('Neither filename nor text supplied')

    def _concatenate_on_ending_backslashes(self, lines):
        concatenator = self.concatenator_factory()
        for l in lines:
            concatenator.add_line(l)
        return concatenator.get_lines()

    def _extract_variable(self, line):
        assignment_split_result = line.split(self.ASSIGNMENT_OPERATOR)
        if len(assignment_split_result) != self.ASSIGNMENT_OPERATOR_SPLIT_LEN:
            raise BadInputFileError('Multiple assignment operators in line %s' % line)
        variable = assignment_split_result[0].strip()
        if not variable:
            raise BadInputFileError('Empty variable for line %s' % line)
        filelist = assignment_split_result[1].split()
        if not filelist:
            raise BadInputFileError('Filelist empty for variable %s' % variable)
        return variable, filelist

    def _parse(self, text):
        lines = text.splitlines()
        lines_concatenated = self._concatenate_on_ending_backslashes(lines)

        answer = {}
        for line in lines_concatenated:
            variable, filelist = self._extract_variable(line)
            if variable in answer:
                raise BadInputFileError('Duplicate of variable %s' % variable)
            answer[variable] = filelist
        return answer

    def filelist(self, variable_name):
        try:
            return self.content_dict[variable_name]
        except KeyError:
            raise UnknownVariableError('Unknown variable %s' % variable_name)

