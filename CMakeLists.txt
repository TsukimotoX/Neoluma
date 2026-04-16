cmake_minimum_required(VERSION 4.0)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")

# ---- Project Setup ----
project(neoluma LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(NEOLUMA_PORTABLE "Portable layout (resources near executable)" ON)

# Set up LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

if(WIN32)
    find_program(NEOLUMA_POWERSHELL_EXECUTABLE NAMES pwsh powershell REQUIRED)
else()
    find_program(NEOLUMA_POWERSHELL_EXECUTABLE NAMES pwsh REQUIRED)
endif()

# ---- Sources ----
file(GLOB_RECURSE CORE_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/*.hpp
)
file(GLOB_RECURSE CLI_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CLI/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CLI/*.hpp
)
file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Libraries/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Libraries/*.hpp
)

add_library(NeolumaCore ${CORE_FILES})
add_library(NeolumaCLI ${CLI_FILES})
add_library(NeolumaLibs ${LIB_FILES})

set(NEOLUMA_ICON_RC "")
if(WIN32)
    set(NEOLUMA_ICON_RC "${CMAKE_SOURCE_DIR}/resources/app.rc")
endif()

add_executable(neoluma
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HelperFunctions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HelperFunctions.cpp
    ${NEOLUMA_ICON_RC}
)

if(APPLE)
    set_target_properties(neoluma PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "app.icns"
    )

    set(APP_ICON_MACOS "${CMAKE_SOURCE_DIR}/resources/app.icns")
    set_source_files_properties(${APP_ICON_MACOS} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )

    target_sources(neoluma PRIVATE ${APP_ICON_MACOS})
endif()


# ---- Include dirs (target-based) ----
target_include_directories(NeolumaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(NeolumaCLI  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(NeolumaLibs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(neoluma PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# LLVM includes/defs
target_include_directories(neoluma PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(neoluma PRIVATE ${LLVM_DEFINITIONS})
target_include_directories(NeolumaCore PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(NeolumaCore PRIVATE ${LLVM_DEFINITIONS})

# ---- LLVM libs ----
llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Support
    IRReader
)

target_link_libraries(neoluma PRIVATE
    ${LLVM_LIBS}
    NeolumaCLI
    NeolumaCore
    NeolumaLibs
)

# ---- Output layout for executable ----
set(OutputRoot "${CMAKE_CURRENT_SOURCE_DIR}/.build")

# config name: Debug/Release/etc
set_target_properties(neoluma PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OutputRoot}/.executables/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${OutputRoot}/.libs/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${OutputRoot}/.libs/$<CONFIG>"
)

# ---- Installation rules ----
include(GNUInstallDirs)

install(TARGETS neoluma
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/Localization/ DESTINATION ${CMAKE_INSTALL_DATADIR}/locales)

set(NEOLUMA_TEST_RUNNER "${CMAKE_SOURCE_DIR}/tests/runner/run-tests.ps1")
set(INSTALLER_RES_DIR "${CMAKE_SOURCE_DIR}/src/Installer/src-tauri/resources")
set(INSTALLER_PAYLOAD "${INSTALLER_RES_DIR}/payload.zip")

add_custom_target(frontend_tests
    COMMAND "${NEOLUMA_POWERSHELL_EXECUTABLE}"
        -ExecutionPolicy Bypass
        -File "${NEOLUMA_TEST_RUNNER}"
        -ExePath "$<TARGET_FILE:neoluma>"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    DEPENDS neoluma
    COMMENT "Running frontend regression tests..."
    VERBATIM
)

set(NEOLUMA_INSTALL_HOOK_CODE [=[
    set(NEOLUMA_INSTALLED_ROOT "$ENV{DESTDIR}@CMAKE_INSTALL_PREFIX@")
    set(NEOLUMA_INSTALLED_EXE "${NEOLUMA_INSTALLED_ROOT}/@CMAKE_INSTALL_BINDIR@/neoluma@CMAKE_EXECUTABLE_SUFFIX@")
    set(NEOLUMA_PAYLOAD_ROOT "@CMAKE_SOURCE_DIR@/.build/payload/@CMAKE_SYSTEM_NAME@-@CMAKE_SYSTEM_PROCESSOR@/@CMAKE_BUILD_TYPE@")
    set(NEOLUMA_PAYLOAD_ZIP "${NEOLUMA_PAYLOAD_ROOT}/payload.zip")
    set(NEOLUMA_INSTALLER_RES_DIR "@INSTALLER_RES_DIR@")
    set(NEOLUMA_INSTALLER_PAYLOAD "${NEOLUMA_INSTALLER_RES_DIR}/payload.zip")

    message(STATUS "Running frontend regression tests...")
    execute_process(
        COMMAND "@NEOLUMA_POWERSHELL_EXECUTABLE@" -ExecutionPolicy Bypass -File "@NEOLUMA_TEST_RUNNER@" -ExePath "${NEOLUMA_INSTALLED_EXE}"
        WORKING_DIRECTORY "@CMAKE_SOURCE_DIR@"
        RESULT_VARIABLE NEOLUMA_FRONTEND_TESTS_RESULT
    )
    if(NOT NEOLUMA_FRONTEND_TESTS_RESULT EQUAL 0)
        message(FATAL_ERROR "Frontend regression tests failed.")
    endif()

    message(STATUS "Packaging installed runtime into payload.zip...")
    file(MAKE_DIRECTORY "${NEOLUMA_PAYLOAD_ROOT}")
    execute_process(
        COMMAND "@CMAKE_COMMAND@" -E rm -f "${NEOLUMA_PAYLOAD_ZIP}"
        RESULT_VARIABLE NEOLUMA_PAYLOAD_CLEAN_RESULT
    )
    if(NOT NEOLUMA_PAYLOAD_CLEAN_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clean previous payload.zip.")
    endif()

    execute_process(
        COMMAND "@CMAKE_COMMAND@" -E chdir "${NEOLUMA_INSTALLED_ROOT}" "@CMAKE_COMMAND@" -E tar "cf" "${NEOLUMA_PAYLOAD_ZIP}" --format=zip -- .
        RESULT_VARIABLE NEOLUMA_PAYLOAD_PACK_RESULT
    )
    if(NOT NEOLUMA_PAYLOAD_PACK_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to package installed runtime into payload.zip.")
    endif()

    execute_process(
        COMMAND "@CMAKE_COMMAND@" -E make_directory "${NEOLUMA_INSTALLER_RES_DIR}"
        RESULT_VARIABLE NEOLUMA_INSTALLER_RES_DIR_RESULT
    )
    if(NOT NEOLUMA_INSTALLER_RES_DIR_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to create installer resources directory.")
    endif()

    execute_process(
        COMMAND "@CMAKE_COMMAND@" -E copy_if_different "${NEOLUMA_PAYLOAD_ZIP}" "${NEOLUMA_INSTALLER_PAYLOAD}"
        RESULT_VARIABLE NEOLUMA_PAYLOAD_COPY_RESULT
    )
    if(NOT NEOLUMA_PAYLOAD_COPY_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to copy payload.zip into installer resources.")
    endif()
]=])
string(CONFIGURE "${NEOLUMA_INSTALL_HOOK_CODE}" NEOLUMA_INSTALL_HOOK_CODE @ONLY)
install(CODE "${NEOLUMA_INSTALL_HOOK_CODE}")

# ---- Payload creation. ----
# payload is bundled into installer resources.
# make payload dirs unique per platform+arch+config to avoid overwriting when switching OS/builds
set(NEOLUMA_PAYLOAD_ROOT "${CMAKE_SOURCE_DIR}/.build/payload/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/$<CONFIG>")
set(NEOLUMA_STAGE_DIR   "${NEOLUMA_PAYLOAD_ROOT}/stage")
set(NEOLUMA_PAYLOAD_ZIP "${NEOLUMA_PAYLOAD_ROOT}/payload.zip")

# multi-config generators require --config. single-config i use (ninja) doesn't.
set(NEOLUMA_INSTALL_CONFIG_ARG "")
if(CMAKE_CONFIGURATION_TYPES)
    set(NEOLUMA_INSTALL_CONFIG_ARG --config $<CONFIG>)
endif()

# uninstaller creation
if(WIN32)
    add_executable(uninstaller
        ${CMAKE_SOURCE_DIR}/src/uninstaller.cpp
    )
    target_compile_features(uninstaller PRIVATE cxx_std_23)

    install(TARGETS uninstaller
        RUNTIME DESTINATION "${NEOLUMA_STAGE_DIR}"
    )
else()
    install(PROGRAMS "${CMAKE_SOURCE_DIR}/resources/uninstall.sh"
        DESTINATION "${NEOLUMA_STAGE_DIR}"
        RENAME "uninstall.sh"
    )
endif()

add_custom_command(
    OUTPUT "${NEOLUMA_PAYLOAD_ZIP}"

    DEPENDS neoluma
    $<$<PLATFORM_ID:Windows>:uninstaller>

    # clean stage folder
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${NEOLUMA_STAGE_DIR}"

    # install to stage
    COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}"
    --prefix "${NEOLUMA_STAGE_DIR}"
    ${NEOLUMA_INSTALL_CONFIG_ARG}

    COMMENT "Installing staged runtime and producing payload.zip for ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR} ($<CONFIG>)..."
    VERBATIM
)

add_custom_target(payload DEPENDS "${NEOLUMA_PAYLOAD_ZIP}")
