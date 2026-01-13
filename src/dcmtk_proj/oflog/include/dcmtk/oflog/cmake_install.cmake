# Install script for directory: C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/DCMTK")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/dcmtk/oflog" TYPE FILE FILES
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/appender.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/config.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/configrt.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/consap.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/fileap.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/fstreams.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/hierarchy.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/hierlock.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/layout.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/logger.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/loglevel.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/logmacro.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/ndc.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/ntelogap.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/nullap.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/oflog.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/socketap.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/streams.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/syslogap.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/tstring.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/winconap.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/oflog/include/dcmtk/oflog/windebap.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/dev/PXSDataServer/src/dcmtk_proj/oflog/include/dcmtk/oflog/config/cmake_install.cmake")
  include("C:/dev/PXSDataServer/src/dcmtk_proj/oflog/include/dcmtk/oflog/helpers/cmake_install.cmake")
  include("C:/dev/PXSDataServer/src/dcmtk_proj/oflog/include/dcmtk/oflog/spi/cmake_install.cmake")

endif()

