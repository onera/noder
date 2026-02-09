cmake_minimum_required(VERSION 3.15...3.26)

get_filename_component(CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
message(STATUS "Current Directory: ${CURRENT_DIR}") 

if(NOT DEFINED SKBUILD_PROJECT_NAME)
    set(SKBUILD_PROJECT_NAME "noder") 
endif()

if(NOT DEFINED CMAKE_INSTALL_DIR)
    message(STATUS "CMAKE_INSTALL_DIR was not defined") 
    set(CMAKE_INSTALL_DIR "${CURRENT_DIR}/dist/dev")
endif()

project(${SKBUILD_PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE Release)



if(MSVC)
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(BUILD_TYPE ${CMAKE_BUILD_TYPE})
    else()
        set(BUILD_TYPE $<CONFIGURATION>)
    endif()
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g") 
    
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(
            -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align
            -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion
            -Wformat=2 -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op
            -Wuseless-cast -Wzero-as-null-pointer-constant -Wunsafe-loop-optimizations
            -Wstrict-null-sentinel -Wfloat-equal -Wstrict-overflow=5 -Wundef -Wcast-qual
            -Wredundant-decls -Wnoexcept -Wstrict-aliasing=2 -fanalyzer -Wsign-compare -Wvla
            -Wpessimizing-move -Wredundant-move -Wunused-parameter -Wstack-protector
            -Wformat-overflow -Wformat-truncation
        )
    endif()
    
    if (APPLE)
        message(STATUS "Disabling -flto on macOS for linker compatibility")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=auto")
    endif()
  
endif()

if (APPLE)
    message(STATUS "Disabling LTO on macOS for compatibility")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

set(CMAKE_BUILD_PARALLEL_LEVEL 8)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN")


message(STATUS "CMAKE_BUILD_DIR: ${CMAKE_BUILD_DIR}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_INSTALL_DIR: ${CMAKE_INSTALL_DIR}")
message(STATUS "CMAKE_INSTALL_RPATH: ${CMAKE_INSTALL_RPATH}")

find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
    OUTPUT_VARIABLE PYBIND11_CMAKEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
list(APPEND CMAKE_PREFIX_PATH ${PYBIND11_CMAKEDIR})
find_package(pybind11 REQUIRED)
include_directories(${PYBIND11_INCLUDE_DIRS})

# Extract short Python version for versioned build outputs (e.g., cp313)
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import sys; print(f'cp{sys.version_info.major}{sys.version_info.minor}')"
    OUTPUT_VARIABLE PYTHON_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Detected Python tag: ${PYTHON_TAG}")

# Define versioned output dir for wheels
set(VERSIONED_OUTPUT_DIR "${CMAKE_BINARY_DIR}/${PYTHON_TAG}/${SKBUILD_PROJECT_NAME}")
file(MAKE_DIRECTORY ${VERSIONED_OUTPUT_DIR})
message(STATUS "VERSIONED_OUTPUT_DIR: ${VERSIONED_OUTPUT_DIR}")

# MPI LIBRARY
find_package(MPI QUIET COMPONENTS C CXX)
if(MPI_FOUND)
    set(ENABLE_MPI ON)
    include_directories(${MPI_CXX_INCLUDE_DIRS})
    set(CMAKE_C_COMPILER ${MPI_C_COMPILER})
    set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
    message(STATUS "MPI found. Using MPI include dirs at: ${MPI_CXX_INCLUDE_DIRS}")
else()
    set(ENABLE_MPI OFF)
    message(STATUS "MPI not found. Disabling MPI support.")
endif()


# --- Force HDF5 detection paths (especially with MPI build) ---
set(HDF5_PREFER_PARALLEL TRUE)


if (HDF5_C_LIBRARY AND HDF5_INCLUDE_DIR AND
    EXISTS "${HDF5_C_LIBRARY}" AND EXISTS "${HDF5_INCLUDE_DIR}/hdf5.h")

    set(HDF5_FOUND TRUE)
    set(HDF5_LIBRARIES "${HDF5_C_LIBRARY}")
    set(HDF5_INCLUDE_DIRS "${HDF5_INCLUDE_DIR}")

    include_directories(${HDF5_INCLUDE_DIRS})
    get_filename_component(HDF5_LIBRARY_DIR "${HDF5_C_LIBRARY}" DIRECTORY)
    link_directories("${HDF5_LIBRARY_DIR}")

    add_definitions(-DENABLE_HDF5_IO)
    set(ENABLE_HDF5_IO ON)

    # if(MSVC)
    #     add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)
    # endif()

    message(STATUS "HDF5 found via env overrides")
else()

    find_package(HDF5 COMPONENTS C)
    
    if (HDF5_FOUND)
        add_definitions(-DENABLE_HDF5_IO)
        set(ENABLE_HDF5_IO ON)
        message(STATUS "HDF5 found via find_package")
    else()
        set(ENABLE_HDF5_IO OFF)
        message(WARNING "HDF5 not found. Set HDF5_C_LIBRARY and HDF5_INCLUDE_DIR to override.")
    endif()
endif()

if (HDF5_FOUND)
    message(STATUS "    HDF5_INCLUDE_DIRS: ${HDF5_INCLUDE_DIRS}")
    message(STATUS "    HDF5_LIBRARIES: ${HDF5_LIBRARIES}")
elseif(MSVC)
    message(FATAL_ERROR "did not find hdf5 for Windows")
endif()

# Make visible to parent
mark_as_advanced(HDF5_FOUND)


set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/c++
    ${PYBIND11_INCLUDE_DIRS}
)

file(GLOB_RECURSE SRC_FILES "src/c++/*.cpp" "include/*.hpp")

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_library(core_shared SHARED ${SRC_FILES})
set_target_properties(core_shared PROPERTIES
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
    POSITION_INDEPENDENT_CODE ON
    LIBRARY_OUTPUT_DIRECTORY "${VERSIONED_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${VERSIONED_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${VERSIONED_OUTPUT_DIR}"
)

if(APPLE)
    set_target_properties(core_shared PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        MACOSX_RPATH ON
    )
elseif(UNIX OR MSVC)
    set_target_properties(core_shared PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
endif()

target_include_directories(core_shared PUBLIC ${PYBIND11_INCLUDE_DIRS})
target_link_libraries(core_shared PUBLIC
    pybind11::pybind11
    Python3::Module
    ${Python3_LIBRARIES}
    ${NUMPY_LIBRARIES}
)
target_compile_definitions(core_shared PUBLIC BUILDING_NODE_LIBRARY)


if(ENABLE_HDF5_IO)
    target_include_directories(core_shared PUBLIC ${HDF5_INCLUDE_DIRS})
    target_link_libraries(core_shared PUBLIC ${HDF5_LIBRARIES})
    if(MSVC)
        # Copy HDF5 DLLs into the wheel output directory so they're bundled in the .whl
        set(HDF5_DLL "${HDF5_ROOT}/bin/hdf5.dll")
        if(EXISTS "${HDF5_DLL}")
            message(STATUS "Copying HDF5 DLL: ${HDF5_DLL}")
            file(COPY "${HDF5_DLL}" DESTINATION ${VERSIONED_OUTPUT_DIR})
            set(HDF5_DLL_IN_OUTPUT "${VERSIONED_OUTPUT_DIR}/hdf5.dll")
        else()
            message(FATAL_ERROR "Required HDF5 DLL not found: ${HDF5_DLL}")
        endif()        
    endif()

    if(UNIX AND NOT APPLE)
        # Copy required HDF5 shared libraries into wheel output directory
        message(STATUS "Looking for HDF5 shared libraries to bundle...")

        get_filename_component(HDF5_LIBRARY_DIR "${HDF5_C_LIBRARY}" DIRECTORY)

        # Copy main HDF5 lib
        file(GLOB HDF5_LIB_SO "${HDF5_LIBRARY_DIR}/libhdf5.so*")
        foreach(lib ${HDF5_LIB_SO})
            message(STATUS "Copying HDF5 shared lib: ${lib}")
            file(COPY ${lib} DESTINATION ${VERSIONED_OUTPUT_DIR})
        endforeach()

        # Optionally copy other dependencies (zlib, szip, etc.)
        file(GLOB HDF5_EXTRA_DEPS
            "${HDF5_LIBRARY_DIR}/libz.so*"
            "${HDF5_LIBRARY_DIR}/libsz.so*"
            "${HDF5_LIBRARY_DIR}/libaec.so*"
            "${HDF5_LIBRARY_DIR}/libcrypto.so*"
        )
        foreach(dep ${HDF5_EXTRA_DEPS})
            if(EXISTS "${dep}")
                message(STATUS "Copying HDF5 dependency: ${dep}")
                file(COPY ${dep} DESTINATION ${VERSIONED_OUTPUT_DIR})
            endif()
        endforeach()
    endif()

endif()

if(ENABLE_MPI)
    target_link_libraries(core_shared PUBLIC ${MPI_CXX_LIBRARIES})
endif()

pybind11_add_module(core MODULE ${SRC_FILES})
target_link_libraries(core PRIVATE core_shared)
add_dependencies(core core_shared)
if(APPLE)
    target_link_options(core PRIVATE "-Wl,-force_load,$<TARGET_FILE:core_shared>")
endif()

install(TARGETS core_shared DESTINATION ${SKBUILD_PROJECT_NAME})
if(MSVC AND ENABLE_HDF5_IO)
    install(FILES "${HDF5_DLL_IN_OUTPUT}" DESTINATION ${SKBUILD_PROJECT_NAME})
endif()

install(TARGETS core DESTINATION ${SKBUILD_PROJECT_NAME})
install(FILES README.md LICENSE DESTINATION ${SKBUILD_PROJECT_NAME})
install(DIRECTORY src/python/noder/ DESTINATION ${SKBUILD_PROJECT_NAME} FILES_MATCHING PATTERN "*.py*")
install(DIRECTORY include DESTINATION ${SKBUILD_PROJECT_NAME})

# option(ENABLE_TESTS "Enable test compilation" ON)
# if(ENABLE_TESTS)
#     file(GLOB_RECURSE TEST_FILES "tests/*.cpp" "tests/*.hpp")
#     pybind11_add_module(tests MODULE ${TEST_FILES})
#     target_link_libraries(tests PRIVATE core_shared)
#     target_include_directories(tests PRIVATE ${CMAKE_INSTALL_INCLUDEDIR} ${CMAKE_SOURCE_DIR}/tests)
#     add_dependencies(tests core_shared)
#     if(APPLE)
#         target_link_options(tests PRIVATE "-Wl,-force_load,$<TARGET_FILE:core_shared>")
#     endif()

#     install(TARGETS tests DESTINATION ${SKBUILD_PROJECT_NAME})
#     install(DIRECTORY tests DESTINATION ${SKBUILD_PROJECT_NAME})
# endif()


option(ENABLE_TESTS "Enable test compilation" ON)
if(ENABLE_TESTS)
    file(GLOB_RECURSE TEST_CPP CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/tests/*.cpp")

    pybind11_add_module(tests MODULE ${TEST_CPP})
    # tests module binds functions implemented across many translation units;
    # keep default visibility so symbols are resolvable at import time.
    set_target_properties(tests PROPERTIES
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
    )
    target_link_libraries(tests PRIVATE core_shared)
    target_include_directories(tests PRIVATE
        ${CMAKE_INSTALL_INCLUDEDIR}
        ${CMAKE_SOURCE_DIR}/tests
    )
    add_dependencies(tests core_shared)

    if(APPLE)
        target_link_options(tests PRIVATE "-Wl,-force_load,$<TARGET_FILE:core_shared>")
    endif()

    install(TARGETS tests DESTINATION ${SKBUILD_PROJECT_NAME})
    install(DIRECTORY tests DESTINATION ${SKBUILD_PROJECT_NAME})
endif()
