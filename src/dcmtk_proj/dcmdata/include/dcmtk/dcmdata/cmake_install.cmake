# Install script for directory: C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/dcmtk/dcmdata" TYPE FILE FILES
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/cmdlnarg.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcbytstr.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcchrstr.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dccodec.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcdatset.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcddirif.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcdeftag.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcdicdir.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcdicent.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcdict.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcdirrec.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcelem.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcerror.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcfcache.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcfilefo.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dchashdi.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcistrma.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcistrmb.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcistrmf.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcistrmz.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcitem.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dclist.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcmetinf.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcobject.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcofsetl.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcostrma.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcostrmb.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcostrmf.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcostrmz.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcovlay.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcpath.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcpcache.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcpixel.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcpixseq.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcpxitem.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcrleccd.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcrlecce.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcrlecp.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcrledec.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcrledrg.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcrleenc.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcrleerg.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcrlerp.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcsequen.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcstack.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcswap.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dctag.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dctagkey.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dctk.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dctypes.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcuid.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvm.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvr.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrae.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvras.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrat.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrcs.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrda.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrds.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrdt.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrfd.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrfl.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvris.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrlo.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrlt.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrobow.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrof.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrpn.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrpobw.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrsh.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrsl.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrss.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrst.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrtm.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrui.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrul.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrulup.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrus.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcvrut.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcwcache.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/dcxfer.h"
    "C:/dev/PXSDataServer/src/dcmtk_src/dcmdata/include/dcmtk/dcmdata/vrscan.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/dev/PXSDataServer/src/dcmtk_proj/dcmdata/include/dcmtk/dcmdata/libi2d/cmake_install.cmake")

endif()

