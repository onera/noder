.. _cpp-io-module:

IO (C++)
========

Read/write helpers for CGNS and backend file conversion workflows.

Namespace reference: :doc:`generated/namespace_io`

Header references:

- :doc:`generated/file_include_io_io.hpp`
- :doc:`generated/file_include_io_cgns_node_pycgns_converter.hpp`

Example
-------

.. literalinclude:: ../../../tests/c++/io/test_io.hpp
   :language: cpp
   :start-after: void test_write_nodes( std::string filename = "test.cgns") {
   :end-before: std::shared_ptr<Node> test_read( std::string tmp_filename = "test_read.cgns") {
