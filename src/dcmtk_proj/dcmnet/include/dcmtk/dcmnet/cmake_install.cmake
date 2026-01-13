# Install script for directory: C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/dcmtk/dcmnet" TYPE FILE FILES
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/assoc.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/cond.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dcasccff.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dcasccfg.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dccfenmp.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dccfpcmp.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dccfprmp.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dccfrsmp.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dccftsmp.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dccfuidh.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dcmlayer.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dcmsmap.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dcmtrans.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dcompat.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dcuserid.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dfindscu.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dicom.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dimse.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/diutil.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/dul.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/extneg.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/lst.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/scp.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmnet/include/dcmtk/dcmnet/scu.h"
    )
endif()

