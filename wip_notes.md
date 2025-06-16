So essentially two choices to verify CGNS conversion and DIP:

1.  Node -> pybind11 
    Array -> pybind11
    Array -> Data
    Node -> Data
    (Node  -X-> Array)

    Requires:
    - allow for virtual "asPyArray" reprs in Data
    
    Pros: 
    - continue using many robust pybind11 classes, and functions depending on them (like Node toPyCGNS)
    - verifies DIP with low-level layers inside project

    Cons:
    - violates DIP with external low-level layer pybind11
    - cannot suppress or replace easily pybind11

2.  Node -X-> pybind11
    Array -> pybind11
    Array -> Data
    Node -> Data
    Data -X-> pybind11
    Node -X-> Array

    Requires
    - replacing pybind11 classes inside Node

    Pros
    - pybind11 dependency is confined inside low-level layers, properly verifying DIP
    - theretically, can replace pybind11 more easily

    Cons
    - cannot use pybind11 classes in functions requiring them (like Node toPyCGNS), which requires to be isolated in separated low-level module or something
    