"""Executable examples for one-dimensional array factories."""

import numpy as np
import pytest

from noder.core import factory


pytestmark = pytest.mark.doc


@pytest.mark.doc_example(target="noder.core.factory.uniformFromStep_*")
def test_uniform_from_step():
    # docs:start uniform_from_step
    values = factory.uniformFromStep_float(0.0, 1.0, 0.25)
    np.testing.assert_array_equal(values.getPyArray(), [0.0, 0.25, 0.5, 0.75])
    # docs:end uniform_from_step


@pytest.mark.doc_example(target="noder.core.factory.uniformFromCount_*")
def test_uniform_from_count():
    # docs:start uniform_from_count
    values = factory.uniformFromCount_float(0.0, 1.0, 5, True)
    np.testing.assert_array_equal(values.getPyArray(), [0.0, 0.25, 0.5, 0.75, 1.0])
    # docs:end uniform_from_count
