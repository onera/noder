import numpy as np
import pytest

from noder.payload_expression import (
    PayloadExpressionError,
    evaluate_payload_expression,
)


def test_payload_expression_supports_numpy_and_brace_references():
    source = np.array([2, 4, 6], dtype=np.int32)
    resolved = []

    def resolve(filename, path):
        resolved.append((filename, path))
        return source

    result = evaluate_payload_expression(
        "np.asarray('{Source}') + np.arange(3, dtype=np.int32)", resolve
    )

    np.testing.assert_array_equal(result, np.array([2, 5, 8], dtype=np.int32))
    assert resolved == [("", "Source")]


def test_payload_expression_supports_cross_file_brace_references():
    source = np.array([1.5, 2.5], dtype=np.float64)
    resolved = []

    def resolve(filename, path):
        resolved.append((filename, path))
        return source

    result = evaluate_payload_expression(
        "'{{mesh.cgns}@{/Base/Zone/Field}}' * 2", resolve
    )

    np.testing.assert_array_equal(result, np.array([3.0, 5.0]))
    assert resolved == [("mesh.cgns", "/Base/Zone/Field")]


def test_payload_expression_reports_syntax_errors():
    with pytest.raises(PayloadExpressionError, match="Invalid expression"):
        evaluate_payload_expression("np.arange(", lambda _filename, _path: None)
