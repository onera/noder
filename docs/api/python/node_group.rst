NodeGroup (Python)
==================

.. currentmodule:: noder.core

``_NodeGroup`` is the temporary container used by ``Node`` operator
expressions (``+`` and ``/``).

C++ counterpart: :doc:`../cpp/node_group`

Class
-----

.. autoclass:: _NodeGroup
   :member-order: bysource

Methods
-------

``Node.__add__`` (create group)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.__add__

.. literalinclude:: ../../../tests/python/node/test_node_group.py
   :language: python
   :start-after: # docs:start node_group_operator_plus_example
   :end-before: # docs:end node_group_operator_plus_example
   :dedent: 4

``Node.__truediv__`` (chain)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.__truediv__

.. literalinclude:: ../../../tests/python/node/test_node_group.py
   :language: python
   :start-after: # docs:start node_group_operator_div_chain_example
   :end-before: # docs:end node_group_operator_div_chain_example
   :dedent: 4

``Node.__truediv__`` with group
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.__truediv__
   :no-index:

.. literalinclude:: ../../../tests/python/node/test_node_group.py
   :language: python
   :start-after: # docs:start node_group_operator_div_group_example
   :end-before: # docs:end node_group_operator_div_group_example
   :dedent: 4

``_NodeGroup.nodes``
~~~~~~~~~~~~~~~~~~~~

.. automethod:: _NodeGroup.nodes

.. literalinclude:: ../../../tests/python/node/test_node_group.py
   :language: python
   :start-after: # docs:start node_group_nodes_example
   :end-before: # docs:end node_group_nodes_example
   :dedent: 4
