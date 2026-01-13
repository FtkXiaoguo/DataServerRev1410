/***********************************************************************
 * PXDcmJobProcError.h
 *---------------------------------------------------------------------
 * define PXDcmJobProcError Error code
 *   
 *-------------------------------------------------------------------
 */

/*
*  #693
*/
#ifndef PXDICOM_JOB_PRO_Error_H_
#define PXDICOM_JOB_PRO_Error_H_


#define DicomJobProcError_Base (100)
#define DicomJobProcError_SoketBusy (DicomJobProcError_Base+1)
#define DicomJobProcError_LicenceInvalid (DicomJobProcError_Base+2)
#define DicomJobProcError_DicomLibInitError (DicomJobProcError_Base+4)
#define DicomJobProcError_DBInitError (DicomJobProcError_Base+5)
#define DicomJobProcError_InvalidDicomAE (DicomJobProcError_Base+6)
#define DicomJobProcError_DicomAssociationError (DicomJobProcError_Base+7)
#define DicomJobProcError_DiskSpaceError (DicomJobProcError_Base+8)
#define DicomJobProcError_InvalidDicomUID (DicomJobProcError_Base+9)
#define DicomJobProcError_InvalidMediaPoint (DicomJobProcError_Base+10)
#define DicomJobProcError_SeriesMonitorError (DicomJobProcError_Base+11)
#define DicomJobProcError_SaveDicomError (DicomJobProcError_Base+12)
#define DicomJobProcError_SaveDBError (DicomJobProcError_Base+13)
#define DicomJobProcError_CFindError (DicomJobProcError_Base+14)
#define DicomJobProcError_CMoveError (DicomJobProcError_Base+15)
#define DicomJobProcError_Exception (DicomJobProcError_Base+16) //#11  2012/03/23 K.Ko
/////
// Warning

////
// Information
#define DicomJobProcInfor_Base (1000)
#define DicomJobProcInfor_DicomAssociationInfo (DicomJobProcInfor_Base+1)
#define DicomJobProcInfor_CStoreInfo (DicomJobProcInfor_Base+2)
#define DicomJobProcInfor_CMoveInfo (DicomJobProcInfor_Base+3)
#define DicomJobProcInfor_AutoRoutingInfo (DicomJobProcInfor_Base+4)  //2012/06/11 K.Ko
#define DicomJobProcInfor_SafetyCheckPatientInfo (DicomJobProcInfor_Base+5)  //#27 2012/06/14 K.Ko


#endif //PXDICOM_JOB_PRO_Error_H_
