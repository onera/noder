import pytest
import noder.tests.array  as test_in_cpp
import noder.array.data_types as dtypes

def test_print(): return test_in_cpp.print()

def test_ArrayPrintWithNone(): return test_in_cpp.ArrayPrintWithNone()

def test_ArrayPrintWithString(): return test_in_cpp.ArrayPrintWithString()

def test_ArrayPrintWithUnicodeString(): return test_in_cpp.ArrayPrintWithUnicodeString()

def test_ArrayPrintWithContiguousArray(): return test_in_cpp.ArrayPrintWithContiguousArray()

def test_ArrayPrintWithNonContiguousArray(): return test_in_cpp.ArrayPrintWithNonContiguousArray()

def test_ArrayPrintWithLongArrayEven(): return test_in_cpp.ArrayPrintWithLongArrayEven()

def test_ArrayPrintWithLongArrayOdd(): return test_in_cpp.ArrayPrintWithLongArrayOdd()

