Navigation (Python)
===================

.. currentmodule:: noder.core

``Navigation`` is returned by :py:meth:`noder.core.Node.pick` and provides
tree queries by name, type, data, and combined predicates.

C++ counterpart: :doc:`../cpp/navigation`

Class
-----

.. autoclass:: Navigation
   :member-order: bysource

Methods
---------------

``child_by_name``
~~~~~~~~~~~~~~~~~

.. automethod:: Navigation.child_by_name

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start child_by_name_example
   :end-before: # docs:end child_by_name_example
   :dedent: 4

``by_name``
~~~~~~~~~~~

.. automethod:: Navigation.by_name

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start by_name_example
   :end-before: # docs:end by_name_example
   :dedent: 4

``all_by_name``
~~~~~~~~~~~~~~~

.. automethod:: Navigation.all_by_name

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start all_by_name_example
   :end-before: # docs:end all_by_name_example
   :dedent: 4

``by_name_regex``
~~~~~~~~~~~~~~~~~

.. automethod:: Navigation.by_name_regex

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start by_name_regex_example
   :end-before: # docs:end by_name_regex_example
   :dedent: 4

``all_by_name_regex``
~~~~~~~~~~~~~~~~~~~~~

.. automethod:: Navigation.all_by_name_regex

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start all_by_name_regex_example
   :end-before: # docs:end all_by_name_regex_example
   :dedent: 4

``by_name_glob``
~~~~~~~~~~~~~~~~

.. automethod:: Navigation.by_name_glob

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start by_name_glob_example
   :end-before: # docs:end by_name_glob_example
   :dedent: 4

``all_by_name_glob``
~~~~~~~~~~~~~~~~~~~~

.. automethod:: Navigation.all_by_name_glob

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start all_by_name_glob_example
   :end-before: # docs:end all_by_name_glob_example
   :dedent: 4

``child_by_type``
~~~~~~~~~~~~~~~~~

.. automethod:: Navigation.child_by_type

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start child_by_type_example
   :end-before: # docs:end child_by_type_example
   :dedent: 4

``by_type``
~~~~~~~~~~~

.. automethod:: Navigation.by_type

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start by_type_example
   :end-before: # docs:end by_type_example
   :dedent: 4

``all_by_type``
~~~~~~~~~~~~~~~

.. automethod:: Navigation.all_by_type

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start all_by_type_example
   :end-before: # docs:end all_by_type_example
   :dedent: 4

``by_type_regex``
~~~~~~~~~~~~~~~~~

.. automethod:: Navigation.by_type_regex

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start by_type_regex_example
   :end-before: # docs:end by_type_regex_example
   :dedent: 4

``all_by_type_regex``
~~~~~~~~~~~~~~~~~~~~~

.. automethod:: Navigation.all_by_type_regex

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start all_by_type_regex_example
   :end-before: # docs:end all_by_type_regex_example
   :dedent: 4

``by_type_glob``
~~~~~~~~~~~~~~~~

.. automethod:: Navigation.by_type_glob

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start by_type_glob_example
   :end-before: # docs:end by_type_glob_example
   :dedent: 4

``all_by_type_glob``
~~~~~~~~~~~~~~~~~~~~

.. automethod:: Navigation.all_by_type_glob

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start all_by_type_glob_example
   :end-before: # docs:end all_by_type_glob_example
   :dedent: 4

``child_by_data``
~~~~~~~~~~~~~~~~~

.. automethod:: Navigation.child_by_data

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start child_by_data_example
   :end-before: # docs:end child_by_data_example
   :dedent: 4

``by_data``
~~~~~~~~~~~

.. automethod:: Navigation.by_data

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start by_data_example
   :end-before: # docs:end by_data_example
   :dedent: 4

``by_data_glob``
~~~~~~~~~~~~~~~~

.. automethod:: Navigation.by_data_glob

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start by_data_glob_example
   :end-before: # docs:end by_data_glob_example
   :dedent: 4

``all_by_data``
~~~~~~~~~~~~~~~

.. automethod:: Navigation.all_by_data

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start all_by_data_example
   :end-before: # docs:end all_by_data_example
   :dedent: 4

``all_by_data_glob``
~~~~~~~~~~~~~~~~~~~~

.. automethod:: Navigation.all_by_data_glob

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start all_by_data_glob_example
   :end-before: # docs:end all_by_data_glob_example
   :dedent: 4

``by_and``
~~~~~~~~~~

.. automethod:: Navigation.by_and

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start by_and_example
   :end-before: # docs:end by_and_example
   :dedent: 4

``all_by_and``
~~~~~~~~~~~~~~

.. automethod:: Navigation.all_by_and

.. literalinclude:: ../../../tests/python/node/test_navigation.py
   :language: python
   :start-after: # docs:start all_by_and_example
   :end-before: # docs:end all_by_and_example
   :dedent: 4
