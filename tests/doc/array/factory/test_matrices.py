"""Executable examples for matrix factory helpers."""

import numpy as np
import pytest

from noder.core import factory


pytestmark = pytest.mark.doc


@pytest.mark.doc_example(target="noder.core.factory.full_*")
def test_full_matrix():
    # docs:start full_matrix
    values = factory.full_double((2, 3), 4.5)
    assert values.getPyArray().shape == (2, 3)
    assert np.all(values.getPyArray() == 4.5)
    # docs:end full_matrix


@pytest.mark.doc_example(target="noder.core.factory.empty_*")
def test_empty_matrix():
    # docs:start empty_matrix
    values = factory.empty_int32((2, 3))
    assert values.getPyArray().shape == (2, 3)
    # docs:end empty_matrix


@pytest.mark.doc_example(target="noder.core.factory.zeros_*")
def test_zero_matrix():
    # docs:start zero_matrix
    values = factory.zeros_int32((2, 3))
    assert np.all(values.getPyArray() == 0)
    # docs:end zero_matrix


@pytest.mark.doc_example(target="noder.core.factory.ones_*")
def test_one_matrix():
    # docs:start one_matrix
    values = factory.ones_int32((2, 3))
    assert np.all(values.getPyArray() == 1)
    # docs:end one_matrix
