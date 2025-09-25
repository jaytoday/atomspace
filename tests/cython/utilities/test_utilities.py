import os
import random
import tempfile
import filecmp

from unittest import TestCase

from opencog.type_constructors import *
from opencog.atomspace import AtomSpace
from opencog.utilities import set_default_atomspace, finalize_opencog

__author__ = 'Curtis Faith'


def write_sorted_file(path, atomspace):
    with open(path, 'wt') as f:
        for atom in sorted(list(atomspace)):
            f.write(str(atom))
            f.write('\n')


class UtilitiesTest(TestCase):

    def setUp(self):
        self.atomspace = AtomSpace()
 
    def tearDown(self):
        del self.atomspace

    def test_initialize_finalize(self):
        set_default_atomspace(self.atomspace)
        finalize_opencog()
