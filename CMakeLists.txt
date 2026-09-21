cmake_minimum_required(VERSION 3.5)

project(
  TeamCAMSLogProcessor
  VERSION 2026.9
  LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_COMPONENTS Widgets LinguistTools Svg Qml)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CONFIG MATCHES "Debug")
  set(ENABLE_QT_TEST TRUE)
  list(APPEND QT_COMPONENTS Test)
else()
  set(ENABLE_QT_TEST FALSE)
endif()

if(EMSCRIPTEN)
  list(APPEND QT_COMPONENTS WebSockets)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${QT_COMPONENTS})

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS})

set(TS_FILES i18n/teamcamslogprocessor_it.ts i18n/teamcamslogprocessor_fr.ts
             i18n/teamcamslogprocessor_de.ts i18n/teamcamslogprocessor_es.ts)

add_custom_target(
  release_translations
  COMMAND lrelease ${TS_FILES}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

set(TEST_SOURCES)

set(PROJECT_SOURCES
    ${TS_FILES}
    src/resources.qrc
    src/processingthread.cpp
    src/processingthread.h
    src/mainwindow.ui
    src/mainwindow.cpp
    src/main.cpp
    src/mainwindow.h)

if(ENABLE_QT_TEST)
  list(APPEND PROJECT_SOURCES ${TEST_SOURCES})
endif()

set(ICON_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/assets/app_icon.ico")
set(ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/assets/app_icon.icns")
set(ICON_LINUX "${CMAKE_CURRENT_SOURCE_DIR}/assets/app_icon.png")

if(WIN32)
  set(RES_FILE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
  file(WRITE "${RES_FILE}" "IDI_ICON1 ICON DISCARDABLE \"${ICON_WINDOWS}\"\n")
  list(APPEND PROJECT_SOURCES "${RES_FILE}")
endif()

if(APPLE)
  list(APPEND PROJECT_SOURCES "${ICON_MACOS}")
  set_source_files_properties("${ICON_MACOS}" PROPERTIES MACOSX_PACKAGE_LOCATION
                                                         Resources)
endif()

qt_add_executable(TeamCAMSLogProcessor MANUAL_FINALIZATION ${PROJECT_SOURCES}
                  src/resources.qrc)

target_compile_definitions(
  TeamCAMSLogProcessor PRIVATE $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
                               $<$<CONFIG:MinSizeRel>:QT_NO_DEBUG_OUTPUT>)

if(ENABLE_QT_TEST)
  target_compile_definitions(TeamCAMSLogProcessor PRIVATE QT_TEST_ENABLED)
else()
  target_compile_definitions(TeamCAMSLogProcessor PRIVATE)
endif()

# Enable if required target_compile_options( TeamCAMSLogProcessor PRIVATE
# -fsanitize=address -fno-omit-frame-pointer) target_link_options(
# TeamCAMSLogProcessor PRIVATE -fsanitize=address)

if(EMSCRIPTEN)
  target_compile_definitions(TeamCAMSLogProcessor PRIVATE NO_WEBSOCKET_SERVER)
  target_link_options(TeamCAMSLogProcessor PRIVATE "-lidbfs.js")
endif()

qt_add_translations(TeamCAMSLogProcessor TS_FILES ${TS_FILES})

target_include_directories(TeamCAMSLogProcessor
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(TEAM_CAMS_PRIVATE_LIBS
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml)

if(ENABLE_QT_TEST)
  list(APPEND TEAM_CAMS_PRIVATE_LIBS Qt${QT_VERSION_MAJOR}::Test)
endif()

target_link_libraries(TeamCAMSLogProcessor PRIVATE ${TEAM_CAMS_PRIVATE_LIBS})

set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER
                     "ch.unifr.teamcams.scripteditor")

set_target_properties(
  TeamCAMSLogProcessor
  PROPERTIES WIN32_EXECUTABLE TRUE
             MACOSX_BUNDLE TRUE
             ${BUNDLE_ID_OPTION} MACOSX_BUNDLE_BUNDLE_VERSION
             "${PROJECT_VERSION}" MACOSX_BUNDLE_SHORT_VERSION_STRING
             "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
             MACOSX_BUNDLE_ICON_FILE
             "app_icon.icns")

if(NOT APPLE AND NOT WIN32)
  install(
    FILES "${ICON_LINUX}"
    DESTINATION "share/icons/hicolor/512x512/apps"
    RENAME "TeamCAMSLogProcessor.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/TeamCAMSLogProcessor.desktop"
          DESTINATION "share/applications")
endif()

include(GNUInstallDirs)

install(
  TARGETS TeamCAMSLogProcessor
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(TeamCAMSLogProcessor)
endif()

if(QT_VERSION_MAJOR EQUAL 6
   AND NOT EMSCRIPTEN
   AND NOT APPLE
   AND NOT WIN32)
  install(TARGETS TeamCAMSLogProcessor
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
elseif(QT_VERSION_MAJOR EQUAL 6 AND NOT EMSCRIPTEN)
  include(QtCMakeMacros OPTIONAL)
  qt_generate_deploy_app_script(
    TARGET TeamCAMSLogProcessor OUTPUT_SCRIPT deploy_script.cmake
    NO_UNSUPPORTED_PLATFORM_ERROR)
  install(SCRIPT ${deploy_script.cmake})
endif()
