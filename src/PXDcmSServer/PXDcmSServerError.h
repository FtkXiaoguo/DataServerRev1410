/***********************************************************************
 * PXDicomServerError.h
 *---------------------------------------------------------------------
 * define PXDicomServer Error code
 *   
 *-------------------------------------------------------------------
 */

/*
*  #693
*/
#ifndef PXDICOMServerError_H_
#define PXDICOMServerError_H_


#define DicomServError_Base (100)
#define DicomServError_SoketBusy (DicomServError_Base+1)
#define DicomServError_LicenceInvalid (DicomServError_Base+2)
#define DicomServError_DicomLibInitError (DicomServError_Base+4)
#define DicomServError_DBInitError (DicomServError_Base+5)
#define DicomServError_InvalidDicomAE (DicomServError_Base+6)
#define DicomServError_DicomAssociationError (DicomServError_Base+7)
#define DicomServError_DiskSpaceError (DicomServError_Base+8)
#define DicomServError_InvalidDicomUID (DicomServError_Base+9)
#define DicomServError_InvalidMediaPoint (DicomServError_Base+10)
#define DicomServError_SeriesMonitorError (DicomServError_Base+11)
#define DicomServError_SaveDicomError (DicomServError_Base+12)
#define DicomServError_SaveDBError (DicomServError_Base+13)
#define DicomServError_CFindError (DicomServError_Base+14)
#define DicomServError_CMoveError (DicomServError_Base+15)
#define DicomServError_Exception (DicomServError_Base+16) //#11  2012/03/23 K.Ko
#define DicomServError_AutoRoutingError (DicomServError_Base+16) //2012/05/11 K.Ko
/////
// Warning

////
// Information
#define DicomServInfo_Base (1000)
#define DicomServInfor_DicomAssociationInfo (DicomServInfo_Base+1)
#define DicomServInfor_CStoreInfo (DicomServInfo_Base+2)
#define DicomServInfor_CMoveInfo (DicomServInfo_Base+3)
#define DicomServInfor_AutoRoutingInfo (DicomServInfo_Base+4)  //2012/06/11 K.Ko
#define DicomServInfor_SafetyCheckPatientInfo (DicomServInfo_Base+5)  //#27 2012/06/14 K.Ko

#endif //PXDICOMServerError_H_
