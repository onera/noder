.. _cpp-nodegroup-class:

NodeGroup (C++)
===============

Temporary grouping object used by ``Node`` operator expressions.

Python counterpart: :py:class:`noder.core._NodeGroup`

Class reference: :doc:`generated/class_node_group`

Operator/function reference: :doc:`generated/file_include_node_node_group.hpp`

Methods and Operators
---------------------

``operator+`` (group construction)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Signatures:

- ``NodeGroup operator+(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs)``
- ``NodeGroup operator+(NodeGroup lhs, const std::shared_ptr<Node>& rhs)``
- ``NodeGroup operator+(const std::shared_ptr<Node>& lhs, NodeGroup rhs)``
- ``NodeGroup operator+(NodeGroup lhs, NodeGroup rhs)``

Example
^^^^^^^

.. literalinclude:: ../../../tests/c++/node/test_node_group.cpp
   :language: cpp
   :start-after: void test_operator_plus_attachToFirstParent() {
   :end-before: void test_operator_div_addChild_chain() {

``operator/`` (chain)
~~~~~~~~~~~~~~~~~~~~~

Signature:

- ``std::shared_ptr<Node> operator/(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs)``

Example
^^^^^^^

.. literalinclude:: ../../../tests/c++/node/test_node_group.cpp
   :language: cpp
   :start-after: void test_operator_div_addChild_chain() {
   :end-before: void test_operator_div_with_plus_group() {

``operator/`` with group
~~~~~~~~~~~~~~~~~~~~~~~~

Signature:

- ``std::shared_ptr<Node> operator/(const std::shared_ptr<Node>& lhs, const NodeGroup& rhs)``

Example
^^^^^^^

.. literalinclude:: ../../../tests/c++/node/test_node_group.cpp
   :language: cpp
   :start-after: void test_operator_div_with_plus_group() {
   :end-before: void test_operator_nested_expression_tree() {

``nodes``
~~~~~~~~~

Signature: ``const std::vector<std::shared_ptr<Node>>& NodeGroup::nodes() const``

Example
^^^^^^^

.. literalinclude:: ../../../tests/c++/node/test_node_group.cpp
   :language: cpp
   :start-after: void test_nodes_accessor() {
   :end-before: void test_empty_group() {

``empty``
~~~~~~~~~

Signature: ``bool NodeGroup::empty() const``

Example
^^^^^^^

.. literalinclude:: ../../../tests/c++/node/test_node_group.cpp
   :language: cpp
   :start-after: void test_empty_group() {
