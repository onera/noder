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

if(DEFINED SKBUILD_PROJECT_VERSION)
    string(REGEX MATCH "^[0-9]+(\\.[0-9]+){0,3}" NODER_CMAKE_PACKAGE_VERSION "${SKBUILD_PROJECT_VERSION}")
endif()
if(NOT NODER_CMAKE_PACKAGE_VERSION)
    execute_process(
        COMMAND git describe --tags --abbrev=0 --match "v[0-9]*"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE NODER_GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NODER_GIT_TAG MATCHES "^v([0-9]+\\.[0-9]+\\.[0-9]+)")
        set(NODER_CMAKE_PACKAGE_VERSION "${CMAKE_MATCH_1}")
    endif()
endif()
if(NOT NODER_CMAKE_PACKAGE_VERSION)
    set(NODER_CMAKE_PACKAGE_VERSION "0.0.0")
endif()

project(${SKBUILD_PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE Release)

# C++17 filesystem library linking
# For GCC < 9 and Intel compilers, libstdc++fs needs explicit linking
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Intel" AND NOT WIN32)
    # Try to find libstdc++fs explicitly
    find_library(STDCXXFS_LIBRARY stdc++fs)
    if(STDCXXFS_LIBRARY)
        message(STATUS "Found libstdc++fs: ${STDCXXFS_LIBRARY}")
    else()
        # Fallback: use the library name directly (works for most GCC/Intel setups)
        set(STDCXXFS_LIBRARY "stdc++fs")
        message(STATUS "Using libstdc++fs by name (will be resolved at link time)")
    endif()
else()
    set(STDCXXFS_LIBRARY "")
endif()

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
if(APPLE)
    # Python extensions and libcore_shared.dylib are installed side by side.
    # macOS uses @loader_path (not the ELF $ORIGIN token) to find that library.
    # This must be set before targets are created because stub generation imports
    # the extensions from the install tree during wheel construction.
    set(CMAKE_INSTALL_RPATH "@loader_path")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()


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
find_package(pybind11 3.0 CONFIG REQUIRED)
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

# MPI is deliberately opt-in.  Merely finding MPI on a build machine must not
# make the regular noder/cgnsvi build depend on it: cgnsvi does not create an
# MPI session, and parallel HDF5 may require one before it can be used.
option(ENABLE_MPI "Enable MPI support for the noder core" OFF)
if(ENABLE_MPI)
    find_package(MPI REQUIRED COMPONENTS C CXX)
    include_directories(${MPI_CXX_INCLUDE_DIRS})
    message(STATUS "MPI explicitly enabled. Using MPI include dirs at: ${MPI_CXX_INCLUDE_DIRS}")
else()
    message(STATUS "MPI disabled (set ENABLE_MPI=ON to opt in)")
endif()


# Use serial HDF5 for the normal build.  In particular, do not silently pick
# the parallel HDF5 installation just because MPI happens to be installed.
# A future MPI-enabled noder build can opt in together with ENABLE_MPI.
set(HDF5_PREFER_PARALLEL ${ENABLE_MPI})

if(MSVC)
    option(NODER_STATIC_HDF5 "Link HDF5 statically to avoid Windows DLL conflicts" ON)
else()
    option(NODER_STATIC_HDF5 "Link HDF5 statically" OFF)
endif()

if(NODER_STATIC_HDF5)
    set(HDF5_USE_STATIC_LIBRARIES TRUE)
endif()

set(NODER_HDF5_IS_STATIC OFF)

if (HDF5_C_LIBRARY AND HDF5_INCLUDE_DIR AND
    EXISTS "${HDF5_C_LIBRARY}" AND EXISTS "${HDF5_INCLUDE_DIR}/hdf5.h")

    set(NODER_HDF5_LIBRARIES "${HDF5_C_LIBRARY}")
    if(MSVC AND NODER_STATIC_HDF5)
        get_filename_component(HDF5_LIBRARY_DIR "${HDF5_C_LIBRARY}" DIRECTORY)
        get_filename_component(HDF5_LIBRARY_NAME "${HDF5_C_LIBRARY}" NAME)
        if(HDF5_LIBRARY_NAME STREQUAL "hdf5.lib" AND EXISTS "${HDF5_LIBRARY_DIR}/libhdf5.lib")
            set(HDF5_C_LIBRARY "${HDF5_LIBRARY_DIR}/libhdf5.lib")
        endif()
        set(NODER_HDF5_LIBRARIES "${HDF5_C_LIBRARY}")
        foreach(HDF5_STATIC_DEP zlib-static.lib libszaec.lib libaec.lib)
            if(EXISTS "${HDF5_LIBRARY_DIR}/${HDF5_STATIC_DEP}")
                list(APPEND NODER_HDF5_LIBRARIES "${HDF5_LIBRARY_DIR}/${HDF5_STATIC_DEP}")
            endif()
        endforeach()
        list(APPEND NODER_HDF5_LIBRARIES shlwapi)
        set(NODER_HDF5_IS_STATIC ON)
    endif()

    set(HDF5_FOUND TRUE)
    set(HDF5_LIBRARIES "${NODER_HDF5_LIBRARIES}")
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
        if(NODER_STATIC_HDF5 AND TARGET hdf5::hdf5-static)
            set(HDF5_LIBRARIES hdf5::hdf5-static)
            set(NODER_HDF5_IS_STATIC ON)
        endif()
        add_definitions(-DENABLE_HDF5_IO)
        set(ENABLE_HDF5_IO ON)
        message(STATUS "HDF5 found via find_package")
    else()
        set(ENABLE_HDF5_IO OFF)
        message(WARNING "HDF5 not found. Set HDF5_C_LIBRARY and HDF5_INCLUDE_DIR to override.")
    endif()
endif()

if (HDF5_FOUND)
    include_directories(${HDF5_INCLUDE_DIRS})
    message(STATUS "    HDF5_INCLUDE_DIRS: ${HDF5_INCLUDE_DIRS}")
    message(STATUS "    HDF5_LIBRARIES: ${HDF5_LIBRARIES}")
    message(STATUS "    NODER_HDF5_IS_STATIC: ${NODER_HDF5_IS_STATIC}")
elseif(MSVC)
    message(FATAL_ERROR "did not find hdf5 for Windows")
endif()

if(ENABLE_HDF5_IO)
    set(NODER_HDF5_VERSION "${HDF5_VERSION}")
    if(NOT NODER_HDF5_VERSION)
        set(NODER_HDF5_MAJOR "")
        set(NODER_HDF5_MINOR "")
        set(NODER_HDF5_RELEASE "")
        foreach(NODER_HDF5_INCLUDE_DIR IN LISTS HDF5_INCLUDE_DIRS)
            if(EXISTS "${NODER_HDF5_INCLUDE_DIR}/H5public.h")
                file(STRINGS "${NODER_HDF5_INCLUDE_DIR}/H5public.h" NODER_HDF5_VERSION_LINES
                    REGEX "^#define H5_VERS_(MAJOR|MINOR|RELEASE)[ \t]+[0-9]+")
                foreach(NODER_HDF5_VERSION_LINE IN LISTS NODER_HDF5_VERSION_LINES)
                    if(NODER_HDF5_VERSION_LINE MATCHES "H5_VERS_MAJOR[ \t]+([0-9]+)")
                        set(NODER_HDF5_MAJOR "${CMAKE_MATCH_1}")
                    elseif(NODER_HDF5_VERSION_LINE MATCHES "H5_VERS_MINOR[ \t]+([0-9]+)")
                        set(NODER_HDF5_MINOR "${CMAKE_MATCH_1}")
                    elseif(NODER_HDF5_VERSION_LINE MATCHES "H5_VERS_RELEASE[ \t]+([0-9]+)")
                        set(NODER_HDF5_RELEASE "${CMAKE_MATCH_1}")
                    endif()
                endforeach()
                break()
            endif()
        endforeach()
        if(NOT "${NODER_HDF5_MAJOR}" STREQUAL "" AND
           NOT "${NODER_HDF5_MINOR}" STREQUAL "" AND
           NOT "${NODER_HDF5_RELEASE}" STREQUAL "")
            set(NODER_HDF5_VERSION "${NODER_HDF5_MAJOR}.${NODER_HDF5_MINOR}.${NODER_HDF5_RELEASE}")
        else()
            set(NODER_HDF5_VERSION "unknown")
        endif()
    endif()

    set(NODER_HDF5_LINKAGE_IS_STATIC ${NODER_HDF5_IS_STATIC})
    if(NOT NODER_HDF5_LINKAGE_IS_STATIC)
        foreach(NODER_HDF5_LIBRARY IN LISTS HDF5_LIBRARIES)
            if(NODER_HDF5_LIBRARY MATCHES "\\.a$|hdf5-static")
                set(NODER_HDF5_LINKAGE_IS_STATIC ON)
                break()
            endif()
        endforeach()
    endif()

    if(NODER_HDF5_LINKAGE_IS_STATIC)
        set(NODER_HDF5_LINKAGE "static")
    else()
        set(NODER_HDF5_LINKAGE "shared (wheel repair bundles runtime dependencies)")
    endif()

    if(NODER_HDF5_VERSION MATCHES "^1\\.")
        set(NODER_HDF5_LICENSE_NAME "HDF5-COPYING.txt")
    else()
        set(NODER_HDF5_LICENSE_NAME "HDF5-LICENSE.txt")
    endif()

    foreach(NODER_HDF5_LICENSE_FILE
            "${CMAKE_CURRENT_SOURCE_DIR}/licenses/HDF5-COPYING.txt"
            "${CMAKE_CURRENT_SOURCE_DIR}/licenses/HDF5-LICENSE.txt"
            "${CMAKE_CURRENT_SOURCE_DIR}/licenses/zlib-LICENSE.txt"
            "${CMAKE_CURRENT_SOURCE_DIR}/licenses/libaec-LICENSE.txt"
            "${CMAKE_CURRENT_SOURCE_DIR}/licenses/OpenSSL-1.1.1-LICENSE.txt"
            "${CMAKE_CURRENT_SOURCE_DIR}/licenses/OpenSSL-3-LICENSE.txt"
            "${CMAKE_CURRENT_SOURCE_DIR}/licenses/THIRD_PARTY_NOTICES.md")
        if(NOT EXISTS "${NODER_HDF5_LICENSE_FILE}")
            message(FATAL_ERROR "Required HDF5 redistribution notice is missing: ${NODER_HDF5_LICENSE_FILE}")
        endif()
    endforeach()

    set(NODER_HDF5_BUILD_INFO "${CMAKE_CURRENT_BINARY_DIR}/HDF5-BUILD-INFO.txt")
    file(WRITE "${NODER_HDF5_BUILD_INFO}"
        "HDF5 version: ${NODER_HDF5_VERSION}\n"
        "HDF5 linkage: ${NODER_HDF5_LINKAGE}\n"
        "HDF5 license file: ${NODER_HDF5_LICENSE_NAME}\n")
    message(STATUS "HDF5 build version: ${NODER_HDF5_VERSION}")
endif()

# Intel LLVM binaries use libmmd.dll on Windows. Keep that runtime beside the
# Python extensions so importing Noder does not depend on a oneAPI PATH.
set(NODER_INTEL_RUNTIME_DLLS)
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
    get_filename_component(NODER_INTEL_COMPILER_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(NODER_INTEL_RUNTIME_NAME libmmd.dll libiomp5md.dll svml_dispmd.dll)
        set(NODER_INTEL_RUNTIME_DLL "${NODER_INTEL_COMPILER_BIN}/${NODER_INTEL_RUNTIME_NAME}")
        if(EXISTS "${NODER_INTEL_RUNTIME_DLL}")
            file(COPY "${NODER_INTEL_RUNTIME_DLL}" DESTINATION "${VERSIONED_OUTPUT_DIR}")
            list(APPEND NODER_INTEL_RUNTIME_DLLS "${NODER_INTEL_RUNTIME_DLL}")
        endif()
    endforeach()
endif()

# Make visible to parent
mark_as_advanced(HDF5_FOUND)


set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")
set(NODER_PACKAGE_DIR "${SKBUILD_PROJECT_NAME}")
set(NODER_INCLUDE_INSTALL_DIR "${NODER_PACKAGE_DIR}/include")
set(NODER_CMAKE_INSTALL_DIR "${NODER_PACKAGE_DIR}/cmake")

include(CMakePackageConfigHelpers)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/c++
    ${PYBIND11_INCLUDE_DIRS}
)

file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS "src/c++/*.cpp")
file(GLOB_RECURSE HPP_FILES "include/*.hpp")
file(GLOB_RECURSE PYBIND_CPP_FILES "src/c++/*_pybind.cpp")
file(GLOB_RECURSE APP_CPP_FILES "src/c++/apps/*.cpp")
set(CORE_PYTHON_ONLY_SRC
    ${CMAKE_SOURCE_DIR}/src/c++/io/hdf5/cgns/node_pycgns_converter.cpp
)
set(CORE_SHARED_SRC ${CPP_FILES} ${HPP_FILES})
list(REMOVE_ITEM CORE_SHARED_SRC ${PYBIND_CPP_FILES})
list(REMOVE_ITEM CORE_SHARED_SRC ${CORE_PYTHON_ONLY_SRC})
list(REMOVE_ITEM CORE_SHARED_SRC ${APP_CPP_FILES})

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_library(core_shared SHARED ${CORE_SHARED_SRC})
set_target_properties(core_shared PROPERTIES
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
    POSITION_INDEPENDENT_CODE ON
    LIBRARY_OUTPUT_DIRECTORY "${VERSIONED_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${VERSIONED_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${VERSIONED_OUTPUT_DIR}"
)

target_include_directories(core_shared
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${NODER_INCLUDE_INSTALL_DIR}>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/c++
)

if(APPLE)
    set_target_properties(core_shared PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        MACOSX_RPATH ON
    )
elseif(UNIX OR MSVC)
    # Ensure RPATH includes C++ standard library location
    # This helps resolve libstdc++.so at runtime
    set_target_properties(core_shared PROPERTIES
        INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

target_compile_definitions(core_shared PRIVATE BUILDING_NODE_LIBRARY)
if(MSVC OR CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
    # Keep the existing UTF-8 conversion behavior while silencing the MSVC STL
    # deprecation notice for std::codecvt, which has no standard replacement.
    target_compile_definitions(
        core_shared PRIVATE _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
endif()


if(ENABLE_HDF5_IO)
    target_include_directories(core_shared PRIVATE ${HDF5_INCLUDE_DIRS})
    target_link_libraries(core_shared PRIVATE ${HDF5_LIBRARIES})
if(STDCXXFS_LIBRARY)
    target_link_libraries(core_shared PRIVATE ${STDCXXFS_LIBRARY})
endif()
    if(MSVC AND NOT NODER_HDF5_IS_STATIC)
        # Copy HDF5 DLLs into the wheel output directory so they're bundled in the .whl
        set(HDF5_DLL "${HDF5_ROOT}/bin/hdf5.dll")
        if(DEFINED HDF5_C_LIBRARY AND EXISTS "${HDF5_C_LIBRARY}")
            set(HDF5_IMPLIB "${HDF5_C_LIBRARY}")
        else()
            set(HDF5_IMPLIB "${HDF5_ROOT}/lib/hdf5.lib")
        endif()
        if(EXISTS "${HDF5_DLL}")
            message(STATUS "Copying HDF5 DLL: ${HDF5_DLL}")
            file(COPY "${HDF5_DLL}" DESTINATION ${VERSIONED_OUTPUT_DIR})
            set(HDF5_DLL_IN_OUTPUT "${VERSIONED_OUTPUT_DIR}/hdf5.dll")
        else()
            message(FATAL_ERROR "Required HDF5 DLL not found: ${HDF5_DLL}")
        endif()
        if(NOT EXISTS "${HDF5_IMPLIB}")
            message(FATAL_ERROR "Required HDF5 import library not found: ${HDF5_IMPLIB}")
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
    target_link_libraries(core_shared PRIVATE ${MPI_CXX_LIBRARIES})
endif()

pybind11_add_module(core MODULE
    src/c++/core_pybind.cpp
    ${CORE_PYTHON_ONLY_SRC}
)
target_link_libraries(core PRIVATE core_shared)
if(STDCXXFS_LIBRARY)
    target_link_libraries(core PRIVATE ${STDCXXFS_LIBRARY})
endif()
add_dependencies(core core_shared)
if(APPLE)
    target_link_options(core PRIVATE "-Wl,-force_load,$<TARGET_FILE:core_shared>")
endif()
if(ENABLE_HDF5_IO)
    target_include_directories(core PRIVATE ${HDF5_INCLUDE_DIRS})
    target_link_libraries(core PRIVATE ${HDF5_LIBRARIES})
endif()

pybind11_add_module(_cgns MODULE src/c++/cgns_pybind.cpp)
target_link_libraries(_cgns PRIVATE core_shared)
if(STDCXXFS_LIBRARY)
    target_link_libraries(_cgns PRIVATE ${STDCXXFS_LIBRARY})
endif()
add_dependencies(_cgns core_shared)
if(APPLE)
    target_link_options(_cgns PRIVATE "-Wl,-force_load,$<TARGET_FILE:core_shared>")
endif()

option(ENABLE_CGNSVI "Build the optional terminal CGNS/HDF5 explorer" ON)
if(ENABLE_CGNSVI)
    if(NOT ENABLE_HDF5_IO)
        message(FATAL_ERROR "ENABLE_CGNSVI requires ENABLE_HDF5_IO")
    endif()

    add_executable(cgnsvi ${APP_CPP_FILES})
    target_compile_definitions(cgnsvi PRIVATE
        NODER_VERSION="${NODER_CMAKE_PACKAGE_VERSION}"
    )
    target_include_directories(cgnsvi PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src/c++
    )
    target_link_libraries(cgnsvi PRIVATE core_shared ${HDF5_LIBRARIES})
    if(STDCXXFS_LIBRARY)
        target_link_libraries(cgnsvi PRIVATE ${STDCXXFS_LIBRARY})
    endif()
    if(MSVC)
        set_target_properties(cgnsvi PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS OFF)
    elseif(APPLE)
        set_target_properties(cgnsvi PROPERTIES
            INSTALL_RPATH "@loader_path"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    elseif(UNIX)
        set_target_properties(cgnsvi PROPERTIES
            INSTALL_RPATH "$ORIGIN"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()
    # Keep the native executable beside core_shared and the Python launcher.
    # The PEP 621 console-script entry point exposes it through the active
    # Python environment's Scripts/bin directory without duplicating DLLs.
    install(TARGETS cgnsvi RUNTIME DESTINATION ${NODER_PACKAGE_DIR})
endif()

install(TARGETS core_shared EXPORT noderTargets DESTINATION ${NODER_PACKAGE_DIR})
if(ENABLE_HDF5_IO)
    install(DIRECTORY licenses/ DESTINATION "${NODER_PACKAGE_DIR}/licenses")
    install(FILES "${NODER_HDF5_BUILD_INFO}" DESTINATION "${NODER_PACKAGE_DIR}/licenses")
endif()
if(NODER_INTEL_RUNTIME_DLLS)
    install(FILES ${NODER_INTEL_RUNTIME_DLLS} DESTINATION ${NODER_PACKAGE_DIR})
endif()
if(MSVC AND ENABLE_HDF5_IO AND NOT NODER_HDF5_IS_STATIC)
    install(FILES "${HDF5_DLL_IN_OUTPUT}" DESTINATION ${NODER_PACKAGE_DIR})
    install(FILES "${HDF5_IMPLIB}" DESTINATION ${NODER_PACKAGE_DIR})
endif()

install(TARGETS core DESTINATION ${NODER_PACKAGE_DIR})
install(TARGETS _cgns DESTINATION ${NODER_PACKAGE_DIR})
install(FILES README.md LICENSE DESTINATION ${NODER_PACKAGE_DIR})
install(DIRECTORY src/python/noder/ DESTINATION ${NODER_PACKAGE_DIR} FILES_MATCHING PATTERN "*.py*")

# Generate stubs from the extension modules in this exact install tree. This
# runs for both local CMake installs and scikit-build-core wheel installs.
file(TO_CMAKE_PATH "${Python3_EXECUTABLE}" NODER_STUBGEN_PYTHON)
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_stubs.py" NODER_STUBGEN_SCRIPT)
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts/stubgen_config.json" NODER_STUBGEN_CONFIG)
set(NODER_STUBGEN_INSTALL_CODE
    "set(_noder_stubgen_prefix \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}\")
execute_process(
    COMMAND \"${NODER_STUBGEN_PYTHON}\" \"${NODER_STUBGEN_SCRIPT}\"
        --config \"${NODER_STUBGEN_CONFIG}\"
        --import-root \"\${_noder_stubgen_prefix}\"
        --output-dir \"\${_noder_stubgen_prefix}/${NODER_PACKAGE_DIR}\"
    RESULT_VARIABLE _noder_stubgen_result
)
if(NOT _noder_stubgen_result EQUAL 0)
    message(FATAL_ERROR \"Failed to generate Noder stubs for the install tree.\")
endif()")
install(CODE "${NODER_STUBGEN_INSTALL_CODE}")

install(DIRECTORY include DESTINATION ${NODER_PACKAGE_DIR})

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/noderConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/noderConfig.cmake
    INSTALL_DESTINATION ${NODER_CMAKE_INSTALL_DIR}
    PATH_VARS NODER_PACKAGE_DIR NODER_INCLUDE_INSTALL_DIR
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/noderConfigVersion.cmake
    VERSION ${NODER_CMAKE_PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(EXPORT noderTargets
    FILE noderTargets.cmake
    NAMESPACE noder::
    DESTINATION ${NODER_CMAKE_INSTALL_DIR}
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/noderConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/noderConfigVersion.cmake
    DESTINATION ${NODER_CMAKE_INSTALL_DIR}
)

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
    if(STDCXXFS_LIBRARY)
        target_link_libraries(tests PRIVATE ${STDCXXFS_LIBRARY})
    endif()
    if(ENABLE_HDF5_IO)
        target_link_libraries(tests PRIVATE ${HDF5_LIBRARIES})
    endif()
    target_include_directories(tests PRIVATE
        ${CMAKE_INSTALL_INCLUDEDIR}
        ${CMAKE_SOURCE_DIR}/tests
    )
    add_dependencies(tests core_shared)

    if(APPLE)
        target_link_options(tests PRIVATE "-Wl,-force_load,$<TARGET_FILE:core_shared>")
    endif()

    install(TARGETS tests DESTINATION ${NODER_PACKAGE_DIR})
    install(DIRECTORY tests DESTINATION ${NODER_PACKAGE_DIR})
endif()
