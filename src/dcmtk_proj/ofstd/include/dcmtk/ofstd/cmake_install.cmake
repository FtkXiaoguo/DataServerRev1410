# Install script for directory: C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/dcmtk/ofstd" TYPE FILE FILES
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofalgo.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofaptr.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofbmanip.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofcast.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofcmdln.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofconapp.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofcond.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofconfig.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofconsol.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofcrc32.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofdate.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofdatime.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofdefine.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/offile.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/offname.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofglobal.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/oflist.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofmap.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofoset.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofset.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofsetit.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofstack.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofstd.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofstdinc.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofstream.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofstring.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofthread.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/oftime.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/oftimer.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/oftypes.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofuoset.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/ofstd/include/dcmtk/ofstd/ofvector.h"
    )
endif()

