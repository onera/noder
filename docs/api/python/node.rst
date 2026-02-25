Node (Python)
=============

.. currentmodule:: noder.core

The Python ``Node`` API is bound from C++ and follows snake_case naming.

Class
-----

.. autoclass:: Node
   :member-order: bysource

Methods
-------

``__init__``
~~~~~~~~~~~~

.. automethod:: Node.__init__

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start init_example
   :end-before: # docs:end init_example
   :dedent: 4

``pick``
~~~~~~~~

.. automethod:: Node.pick

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start pick_example
   :end-before: # docs:end pick_example
   :dedent: 4

``name``
~~~~~~~~

.. automethod:: Node.name

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start name_example
   :end-before: # docs:end name_example
   :dedent: 4

``set_name``
~~~~~~~~~~~~

.. automethod:: Node.set_name

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start set_name_example
   :end-before: # docs:end set_name_example
   :dedent: 4

``type``
~~~~~~~~

.. automethod:: Node.type

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start type_example
   :end-before: # docs:end type_example
   :dedent: 4

``set_type``
~~~~~~~~~~~~

.. automethod:: Node.set_type

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start set_type_example
   :end-before: # docs:end set_type_example
   :dedent: 4

``data``
~~~~~~~~

.. automethod:: Node.data

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start data_example
   :end-before: # docs:end data_example
   :dedent: 4

``set_data``
~~~~~~~~~~~~

.. automethod:: Node.set_data

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start set_data_example
   :end-before: # docs:end set_data_example
   :dedent: 4

``children``
~~~~~~~~~~~~

.. automethod:: Node.children

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start children_example
   :end-before: # docs:end children_example
   :dedent: 4

``has_children``
~~~~~~~~~~~~~~~~

.. automethod:: Node.has_children

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start has_children_example
   :end-before: # docs:end has_children_example
   :dedent: 4

``parent``
~~~~~~~~~~

.. automethod:: Node.parent

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start parent_example
   :end-before: # docs:end parent_example
   :dedent: 4

``root``
~~~~~~~~

.. automethod:: Node.root

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start root_example
   :end-before: # docs:end root_example
   :dedent: 4

``level``
~~~~~~~~~

.. automethod:: Node.level

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start level_example
   :end-before: # docs:end level_example
   :dedent: 4

``position``
~~~~~~~~~~~~

.. automethod:: Node.position

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start position_example
   :end-before: # docs:end position_example
   :dedent: 4

``detach``
~~~~~~~~~~

.. automethod:: Node.detach

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start detach_example
   :end-before: # docs:end detach_example
   :dedent: 4

``attach_to``
~~~~~~~~~~~~~

.. automethod:: Node.attach_to

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start attach_to_example
   :end-before: # docs:end attach_to_example
   :dedent: 4

``add_child``
~~~~~~~~~~~~~

.. automethod:: Node.add_child

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start add_child_example
   :end-before: # docs:end add_child_example
   :dedent: 4

``add_children``
~~~~~~~~~~~~~~~~

.. automethod:: Node.add_children

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start add_children_example
   :end-before: # docs:end add_children_example
   :dedent: 4

``siblings``
~~~~~~~~~~~~

.. automethod:: Node.siblings

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start siblings_example
   :end-before: # docs:end siblings_example
   :dedent: 4

``has_siblings``
~~~~~~~~~~~~~~~~

.. automethod:: Node.has_siblings

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start has_siblings_example
   :end-before: # docs:end has_siblings_example
   :dedent: 4

``get_children_names``
~~~~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.get_children_names

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start get_children_names_example
   :end-before: # docs:end get_children_names_example
   :dedent: 4

``swap``
~~~~~~~~

.. automethod:: Node.swap

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start swap_example
   :end-before: # docs:end swap_example
   :dedent: 4

``copy``
~~~~~~~~

.. automethod:: Node.copy

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start copy_example
   :end-before: # docs:end copy_example
   :dedent: 4

``get_at_path``
~~~~~~~~~~~~~~~

.. automethod:: Node.get_at_path

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start get_at_path_example
   :end-before: # docs:end get_at_path_example
   :dedent: 4

``path``
~~~~~~~~

.. automethod:: Node.path

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start path_example
   :end-before: # docs:end path_example
   :dedent: 4

``descendants``
~~~~~~~~~~~~~~~

.. automethod:: Node.descendants

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start descendants_example
   :end-before: # docs:end descendants_example
   :dedent: 4

``merge``
~~~~~~~~~

.. automethod:: Node.merge

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start merge_example
   :end-before: # docs:end merge_example
   :dedent: 4

``has_link_target``
~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.has_link_target

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start has_link_target_example
   :end-before: # docs:end has_link_target_example
   :dedent: 4

``link_target_file``
~~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.link_target_file

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start link_target_file_example
   :end-before: # docs:end link_target_file_example
   :dedent: 4

``link_target_path``
~~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.link_target_path

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start link_target_path_example
   :end-before: # docs:end link_target_path_example
   :dedent: 4

``set_link_target``
~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.set_link_target

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start set_link_target_example
   :end-before: # docs:end set_link_target_example
   :dedent: 4

``clear_link_target``
~~~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.clear_link_target

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start clear_link_target_example
   :end-before: # docs:end clear_link_target_example
   :dedent: 4

``get_links``
~~~~~~~~~~~~~

.. automethod:: Node.get_links

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start get_links_example
   :end-before: # docs:end get_links_example
   :dedent: 4

``set_parameters``
~~~~~~~~~~~~~~~~~~

.. automethod:: Node.set_parameters

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start set_parameters_example
   :end-before: # docs:end set_parameters_example
   :dedent: 4

``get_parameters``
~~~~~~~~~~~~~~~~~~

.. automethod:: Node.get_parameters

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start get_parameters_example
   :end-before: # docs:end get_parameters_example
   :dedent: 4

``reload_node_data``
~~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.reload_node_data

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start reload_node_data_example
   :end-before: # docs:end reload_node_data_example
   :dedent: 4

``save_this_node_only``
~~~~~~~~~~~~~~~~~~~~~~~

.. automethod:: Node.save_this_node_only

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start save_this_node_only_example
   :end-before: # docs:end save_this_node_only_example
   :dedent: 4

``print_tree``
~~~~~~~~~~~~~~

.. automethod:: Node.print_tree

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start print_tree_example
   :end-before: # docs:end print_tree_example
   :dedent: 4

``write``
~~~~~~~~~

.. automethod:: Node.write

.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :start-after: # docs:start write_example
   :end-before: # docs:end write_example
   :dedent: 4
