cmake_minimum_required(VERSION 3.5)

project(SumuriHexViewerVersion1 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Sql)

set(ADDITIONAL_LIBS_DIR ${CMAKE_SOURCE_DIR}/libs/tsk/additionallibs)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/sumuri.qrc")

set(TS_FILES SumuriHexViewerV1_en_US.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        ${CMAKE_SOURCE_DIR}/headers/mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SumuriHexViewerVersion1
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        mainwindow.ui
        ${CMAKE_SOURCE_DIR}/headers/hexviewerform.h
        hexviewerform.cpp
        hexviewerform.ui
        ${CMAKE_SOURCE_DIR}/headers/datatypeviewmodel.h
        datatypeviewmodel.cpp
        headers/hexeditor.h
        hexeditor.cpp
        ${CMAKE_SOURCE_DIR}/headers/openingdialog.h
        openingdialog.cpp
        openingdialog.ui
        ${CMAKE_SOURCE_DIR}/headers/gotooffsetdialog.h
        gotooffsetdialog.cpp
        gotooffsetdialog.ui
        headers/filesystemtabwidget.h
        filesystemtabwidget.cpp
        headers/filesystemtablemodel.h
        filesystemtablemodel.cpp
        ${CMAKE_SOURCE_DIR}/headers/filesystemhandler.h
        filesystemhandler.cpp
        headers/MarkersTableModel.h
        MarkersTableModel.cpp
        headers/FileSystemException.h
        headers/LoadingDialog.h
        LoadingDialog.cpp
        sumuri.qrc
        headers/testfilesystemhandler.h
        testfilesystemhandler.cpp
        headers/tagshandler.h
        tagshandler.cpp
         ${CMAKE_SOURCE_DIR}/headers/newtagdialog.h
        newtagdialog.cpp
        newtagdialog.ui
        headers/tag.h
        headers/tagstablemodel.h
        tagstablemodel.cpp
        headers/tagdialogmodel.h
        tagdialogmodel.cpp
        libs/tags_sql.sql
        headers/ewfdevice.h
        ewfdevice.cpp
        headers/physicaldrivesdialog.h
        physicaldrivesdialog.cpp
        physicaldrivesdialog.ui
        headers/windowsdrivedevice.h
        windowsdrivedevice.cpp
          ${CMAKE_SOURCE_DIR}/headers/searchform.h
        searchform.cpp
        searchform.ui
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(SumuriHexViewerVersion1 SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(SumuriHexViewerVersion1
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# Add TSK library
set(TSK_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/tsk/headers)
set(TSK_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/libs/tsk/library)
include_directories(${TSK_INCLUDE_DIR})
link_directories(${TSK_LIBRARY_DIR})

# Add additional libraries
include_directories(${ADDITIONAL_LIBS_DIR})
link_directories(${ADDITIONAL_LIBS_DIR})

# Debugging messages
message(STATUS "TSK include directory: ${TSK_INCLUDE_DIR}")
message(STATUS "TSK library directory: ${TSK_LIBRARY_DIR}")

# Check if library exists
find_library(TSK_LIB  tsk libtsk  HINTS ${TSK_LIBRARY_DIR} REQUIRED)
if(NOT TSK_LIB)
    message(FATAL_ERROR "TSK library not found")
else()
    message(STATUS "TSK library found: ${TSK_LIB}")
endif()

# Check if additional library exists
find_library(LIB_EWF NAMES ewf libewf  HINTS   ${ADDITIONAL_LIBS_DIR})
if(NOT LIB_EWF)
    message(FATAL_ERROR "Additional library (libewf) not found at ${ADDITIONAL_LIBS_DIR}")
else()
    message(STATUS "Additional library (libewf) found: ${LIB_EWF}")
endif()

find_library(LIB_VMDK NAMES vmdk libvmdk  HINTS   ${ADDITIONAL_LIBS_DIR})
if(NOT LIB_VMDK)
    message(FATAL_ERROR "Additional library (LIB_VMDK) not found at ${ADDITIONAL_LIBS_DIR}")
else()
    message(STATUS "Additional library (LIB_VMDK) found: ${LIB_VMDK}")
endif()

find_library(LIB_VHDI NAMES vhdi libvhdi  HINTS   ${ADDITIONAL_LIBS_DIR})
if(NOT LIB_VHDI)
    message(FATAL_ERROR "Additional library (LIB_VHDI) not found at ${ADDITIONAL_LIBS_DIR}")
else()
    message(STATUS "Additional library (LIB_VHDI) found: ${LIB_VHDI}")
endif()

# Debug and Release versions of additional libraries
find_library(LIB_ZLIB_RELEASE NAMES zlib   HINTS   ${ADDITIONAL_LIBS_DIR})
find_library(LIB_ZLIB_DEBUG NAMES zlibd zlib   HINTS   ${ADDITIONAL_LIBS_DIR})
find_library(LIB_CRYPTO_RELEASE NAMES crypto libcrypto  HINTS   ${ADDITIONAL_LIBS_DIR})
find_library(LIB_CRYPTO_DEBUG NAMES cryptod crypto libcrypto  HINTS   ${ADDITIONAL_LIBS_DIR})
find_library(LIB_SSL_RELEASE NAMES ssl libssl  HINTS   ${ADDITIONAL_LIBS_DIR})
find_library(LIB_SSL_DEBUG NAMES ssld ssl libssl  HINTS   ${ADDITIONAL_LIBS_DIR})

# Link different libraries depending on the build type
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(LIB_ZLIB ${LIB_ZLIB_DEBUG})
    set(LIB_CRYPTO ${LIB_CRYPTO_DEBUG})
    set(LIB_SSL ${LIB_SSL_DEBUG})
else()
    set(LIB_ZLIB ${LIB_ZLIB_RELEASE})
    set(LIB_CRYPTO ${LIB_CRYPTO_RELEASE})
    set(LIB_SSL ${LIB_SSL_RELEASE})
endif()

if(NOT LIB_ZLIB)
    message(FATAL_ERROR "ZLIB library not found")
else()
    message(STATUS "ZLIB library found: ${LIB_ZLIB}")
endif()

if(NOT LIB_CRYPTO)
    message(FATAL_ERROR "Crypto library not found")
else()
    message(STATUS "Crypto library found: ${LIB_CRYPTO}")
endif()

if(NOT LIB_SSL)
    message(FATAL_ERROR "SSL library not found")
else()
    message(STATUS "SSL library found: ${LIB_SSL}")
endif()

target_link_libraries(SumuriHexViewerVersion1 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql  ${LIB_EWF} ${TSK_LIB} ${LIB_VMDK} ${LIB_VHDI} ${LIB_ZLIB} ${LIB_CRYPTO} ${LIB_SSL})




if(WIN32)
    target_link_libraries(SumuriHexViewerVersion1 PRIVATE shlwapi)
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SumuriHexViewerVersion1)
endif()
set_target_properties(SumuriHexViewerVersion1 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SumuriHexViewerVersion1
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SumuriHexViewerVersion1)
endif()
