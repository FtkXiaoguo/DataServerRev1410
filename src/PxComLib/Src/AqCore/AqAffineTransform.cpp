
/*********************************************************************
 *********************************************************************
 **************											**************
 **************	This document contains information that	**************
 **************	   is proprietary to PreXion 	**************
 **************		     All rights reserved.			**************
 **************			Copyright PreXion 		**************
 **************				 2005-2006					**************
 **************											************** 
 *********************************************************************
 *********************************************************************
 *********************************************************************
 ****															  ****
 **** File       : AqAffineTransform.cpp						  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Implementation of AqAffineTransform class.	  ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 **** Modification log [Please insert recent one on the top]	  ****

 Prashant Chopra 2005-12-16
 Removed redundant inclusion of stdafx.h. 

 ********************************************************************/

//////////////////////////////////////////////////////////////////////
 
#include "AqAffineTransform.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/******
 * NAME
 *    AqAffineTransform
 * SYNOPSIS
 *    AqAffineTransform
 * DESCRIPTION
 *    Class constructor
 * PARAMETERS
 *    None
 * OUTPUT
 *    An instance of the class
 * RETURN VALUE
 *    Handle to an instance of the class.
 * EXAMPLE
 *    % AqAffineTransform affTrans;    
 ***/
AqAffineTransform::AqAffineTransform()
{
	//AqResultBase(kAqResultDataTypeTransform);

	type = kAqAffine;	
	this->Matrix = new AqMatrix4x4;
	this->tempMatrix1 = new AqMatrix4x4;
	this->tempMatrix2 = new AqMatrix4x4;	
	this->tOrigin = new AqVector;
	this->tOrigin->init(0,0,0);
	/*
	this->setShearFlagDoF(!TR_NO_SHEAR);	
	this->setScalingFlagDoF(!TR_NO_SCALING);	
	this->setTranslationFlagDoF(!TR_NO_TRANSLATION);
	this->setRotationFlagDoF(!TR_NO_ROTATION);	*/

	this->activateAllDoF();

	this->identity();
}

/******
 * NAME
 *    ~AqAffineTransform
 * SYNOPSIS
 *    delete <instanceName>
 * DESCRIPTION
 *    Class destructor
 * PARAMETERS
 *    None
 * OUTPUT
 *    N.A.
 * RETURN VALUE
 *    N.A. 
 * EXAMPLE
 *    % delete affTrans;    
 ***/
AqAffineTransform::~AqAffineTransform()
{
	if (this->tOrigin){
		delete this->tOrigin;
    }
	if (this->Matrix){
		delete this->Matrix;
    }
	if (this->tempMatrix1){
		delete this->tempMatrix1;
    }
	if (this->tempMatrix2){
		delete this->tempMatrix2;
    }
}



/******
 * NAME
 *    isActiveDoF
 * SYNOPSIS
 *    isActiveDoF <index>
 * DESCRIPTION
 *    Tells if degree of freedom for <index> is active.
 * PARAMETERS
 *    <index>		:	index of the degree of freedom. 
 * OUTPUT
 *    Activity status of the specified degree of freedom.
 * RETURN VALUE
 *   true or false. 
 * EXAMPLE 
 *    % bool status = affTrans.isActiveDoF(0); 
 ***/
bool AqAffineTransform::isActiveDoF(unsigned int i) {	
	bool res = 0;
	if (i < kAqDoFCount) {	
		res = (this->activeFlagsDoF[i] == 1 ? true : false);
	}	
	return res;
}

/******
 * NAME
 *    isActiveDoF
 * SYNOPSIS
 *    isActiveDoF <tag>
 * DESCRIPTION
 *    Tells if degree of freedom for <tag> is active.
 * PARAMETERS
 *    <tag>		:	tag of the degree of freedom. 
 * OUTPUT
 *    Activity status of the specified degree of freedom.
 * RETURN VALUE
 *   true or false. 
 * EXAMPLE 
 *    % bool status = affTrans.isActiveDoF(kAqTransX); 
 ***/
bool AqAffineTransform::isActiveDoF(eAqTagDoF i) {
	bool res = 0;
	if (i < kAqDoFCount) {
		res = (this->activeFlagsDoF[i] == 1 ? true : false);	
	}	
	return res;
}

/******
 * NAME
 *    setFlagDoF
 * SYNOPSIS
 *    setFlagDoF <index> <flag>
 * DESCRIPTION
 *    Sets the activity flag for degree of freedom corresponding to <index> to <flag>.
 * PARAMETERS
 *    <index>		:	index of the degree of freedom. 
 *	  <flag>		:	flag to set (true/false).
 * OUTPUT
 *    Sets the activity flag for degree of freedom corresponding to <index> to <flag>.
 * RETURN VALUE
 *   Execution status of int.
 * EXAMPLE 
 *    % int ret = affTrans.setFlagDoF(0, false); 
 ***/
int AqAffineTransform::setFlagDoF(unsigned int i, unsigned char flag) {	
	int res = kAqSuccess;

	if (i< kAqDoFCount) {
		this->activeFlagsDoF[i] = flag;
	} else {
		res = kAqFailure;
		//#ifdef TR_DEBUG	
			//TR_LOG(this->logptr(),"WARNING(AqAffineTransform::setFlagDoF:<%d>): Index out of bound!", res);
			//this->warning(TR_IGNORE);				
		//#endif
	}

	return res;
}

/******
 * NAME
 *    activateAllDoF
 * SYNOPSIS
 *    activateAllDoF 
 * DESCRIPTION
 *    Sets the activity flag for all degrees of freedom to true.
 * PARAMETERS
 *	  None.
 * OUTPUT
 *    Sets the activity flag for all degrees of freedom to true.
 * RETURN VALUE
 *   None.
 * EXAMPLE 
 *    % affTrans.activateAllDoF(); 
 ***/
void AqAffineTransform::activateAllDoF(void) {
	unsigned int i;
	for (i=0; i< kAqDoFCount; i++) {
		this->setFlagDoF(i, 1);
	}
}

/******
 * NAME
 *    deactivateAllDoF
 * SYNOPSIS
 *    deactivateAllDoF 
 * DESCRIPTION
 *    Sets the activity flag for all degrees of freedom to false.
 * PARAMETERS
 *	  None.
 * OUTPUT
 *    Sets the activity flag for all degrees of freedom to false.
 * RETURN VALUE
 *   None.
 * EXAMPLE 
 *    % affTrans.deactivateAllDoF(); 
 ***/
void AqAffineTransform::deactivateAllDoF(void) {
	unsigned int i;
	for (i=0; i< kAqDoFCount; i++) {
		this->setFlagDoF(i, 0);
	}
}

/******
 * NAME
 *    update
 * SYNOPSIS
 *    update
 * DESCRIPTION
 *    Updates (recomputes/synchronizes) the transform matrix with the current degree of
 *	  parameter values. 	
 * PARAMETERS
 *	  None.
 * OUTPUT
 *	  Updates (recomputes/synchronizes) the transform matrix with the current degree of
 *	  parameter values. 	
 * RETURN VALUE
 *    Execution status of int.
 * EXAMPLE 
 *    % affTrans.update(); 
 ***/
int AqAffineTransform::update() { 	
	int res = kAqSuccess;

	// update the transform matrix here from dof parameters
	//double px, py, pz, pu, pv, pw;
	double px, py, pz;

	this->Matrix->Identity();

//	if (this->isActiveTranslationDoF()) {
		px = this->getDoFParameter(kAqTransX);
		py = this->getDoFParameter(kAqTransY);
		pz = this->getDoFParameter(kAqTransZ);
		if (fabs(px)>kAqEpsilon || fabs(py)>kAqEpsilon || fabs(pz)>kAqEpsilon) {
			this->_getTranslationMatrix(px, py, pz, this->tempMatrix1);
			this->postApplyTransform(this->tempMatrix1);
		}
	//}	

	this->postTranslate(-this->tOrigin->tuple[0], -this->tOrigin->tuple[1], -this->tOrigin->tuple[2]); 
	
	//if (this->isActiveRotationDoF()) {		
		px = this->getDoFParameter(kAqRotX);
		py = this->getDoFParameter(kAqRotY);
		pz = this->getDoFParameter(kAqRotZ);
		if (fabs(px)>kAqEpsilon || fabs(py)>kAqEpsilon || fabs(pz)>kAqEpsilon) {
			this->_getRotationMatrix(px, py, pz, this->tempMatrix1);
			this->postApplyTransform(this->tempMatrix1);
		}
	//}

	//if (this->isActiveScalingDoF()) {
		px = this->getDoFParameter(kAqScaleX);
		py = this->getDoFParameter(kAqScaleY);
		pz = this->getDoFParameter(kAqScaleZ);
		//if (fabs(px)!=1 || fabs(py)!=1 || fabs(pz)!=1) {
		if (fabs(px)>kAqEpsilon || fabs(py)>kAqEpsilon || fabs(pz)>kAqEpsilon) {
			this->_getScalingMatrix(px, py, pz, this->tempMatrix1);
			this->postApplyTransform(this->tempMatrix1);
		}
	//}


	//if (this->isActiveShearDoF()) {
		/*px = this->getDoFParameter(kAqShearXY);
		py = this->getDoFParameter(kAqShearYX);
		pz = this->getDoFParameter(kAqShearYZ);
		pu = this->getDoFParameter(kAqShearZY);
		pv = this->getDoFParameter(kAqShearZX);
		pw = this->getDoFParameter(kAqShearXZ);	

		this->_getShearingMatrix(px, py, pz, pu, pv, pw, this->tempMatrix1);*/
		
		pz = this->getDoFParameter(kAqShearXY);		
		px = this->getDoFParameter(kAqShearYZ);		
		py = this->getDoFParameter(kAqShearXZ);	
		if (fabs(px)>kAqEpsilon || fabs(py)>kAqEpsilon || fabs(pz)>kAqEpsilon) {
			this->_getShearingMatrix(pz, px, py, this->tempMatrix1);
			this->postApplyTransform(this->tempMatrix1);
		}
	//}

	this->postTranslate(this->tOrigin->tuple[0], this->tOrigin->tuple[1], this->tOrigin->tuple[2]); 

	// the result would be:
	// T' = (TRToOrigin)-1. (SHz . SHy . SHx) . (SC) . (R). (TRToOrigin) . (TR) . I
	//
	return res;
}


/******
 * NAME
 *    invert
 * SYNOPSIS
 *    invert
 * DESCRIPTION
 *    Inverts the current transform matrix.
 * PARAMETERS
 *	  None.
 * OUTPUT
 *    Inverts the current transform matrix.
 * RETURN VALUE
 *    Execution status of int.
 * EXAMPLE 
 *    % affTrans.invert(); 
 ***/
int AqAffineTransform::invert() {
	int res = kAqSuccess;

	
	res= this->Matrix->Invert();
	if (res != kAqSuccess) {		
		// log the event	
		#ifdef TR_DEBUG	
			TR_LOG(this->logptr(), "ERROR(AqAffineTransform::invert:<%d>): Matrix inversion failure!", res);
			this->error();			
		#endif
	}
	//this->_updated = true;
	return res;
}

/******
 * NAME
 *    getDoFCount
 * SYNOPSIS
 *    getDoFCount
 * DESCRIPTION
 *    Returns the total degrees of freedom in scope of this transform object.
 * PARAMETERS
 *	  None.
 * OUTPUT
 *    The total degrees of freedom in scope of this transform object.
 * RETURN VALUE
 *    The total degrees of freedom in scope of this transform object.
 * EXAMPLE 
 *    % unsigned short dCount = affTrans.getDoFCount(); 
 ***/
unsigned short AqAffineTransform::getDoFCount() {	
	return kAqDoFCount;
}

/******
 * NAME
 *    getActiveDoFCount
 * SYNOPSIS
 *    getActiveDoFCount
 * DESCRIPTION
 *    Returns the total count of active degrees of freedom in scope of this transform object.
 * PARAMETERS
 *	  None.
 * OUTPUT
 *    The total count of active degrees of freedom in scope of this transform object.
 * RETURN VALUE
 *    The total count of active degrees of freedom in scope of this transform object.
 * EXAMPLE 
 *    % unsigned short activeDofCount = affTrans.getActiveDoFCount(); 
 ***/
unsigned short AqAffineTransform::getActiveDoFCount() {
	unsigned short i = 0, res = 0;
	// loop over the DoFs and see which ones are active
	for (i=0; i< kAqDoFCount; i++) {
		res+= this->isActiveDoF(i);		
	}
	return res;
}


/******
 * NAME
 *    identity
 * SYNOPSIS
 *    identity
 * DESCRIPTION
 *    Initializes the transform to 'identity'.
 *	  All dof parameters are initialized first, then matrix recomputed.
 * PARAMETERS
 *	  None.
 * OUTPUT
 *    Initializes the transform to 'identity'.
 * RETURN VALUE
 *    None.
 * EXAMPLE 
 *    % affTrans.identity(); 
 ***/
void AqAffineTransform::identity() {
	// reset all dofs
	this->setDoFParameters(kAqIdentityDoFValues);
	//this->_updated = true;
}



/******************************************* PRE operations *****************************/
// apply the incoming transform before the existing matrix
// i.e. if existing matrix is A, and incoming transform is T,
// a pre-transform would mean 
//		A = A.T

/******
 * NAME
 *    preTranslate
 * SYNOPSIS
 *    preTranslate <X> <Y> <Z>
 * DESCRIPTION
 *    Applies a translation by given factors BEFORE the current transform.
 * PARAMETERS
 *	  <X>	:	translation factor along X axis.
 *	  <Y>	:	translation factor along Y axis.
 *	  <Z>	:	translation factor along Z axis.
 * OUTPUT
 *    Applies a translation by given factors BEFORE the current transform.
 * RETURN VALUE
 *    None.
 * EXAMPLE 
 *    % affTrans.preTranslate(10, -8, 9.65);
 ***/
void AqAffineTransform::preTranslate(double x, double y, double z) {	
	this->_getTranslationMatrix(x, y, z, this->tempMatrix1);
	this->preApplyTransform(this->tempMatrix1);
};

void AqAffineTransform::preTranslate(const double x[3]) {
		this->preTranslate(x[0], x[1], x[2]);
};

void AqAffineTransform::preTranslate(const float x[3]) {
		this->preTranslate(x[0], x[1], x[2]);
};

/******
 * NAME
 *    preRotate 
 * SYNOPSIS
 *    preRotate <X> <Y> <Z>
 * DESCRIPTION
 *    Applies a rotation by given factors BEFORE the current transform.
 *
 *	  Note: To avoid "gimbal lock" situations, define TR_USE_QUATERNIONS flag
 *	  when compiling:
 *			#define TR_USE_QUATERNIONS
 *
 * PARAMETERS
 *	  <X>	:	rotation angle in degrees about X axis.
 *	  <Y>	:	rotation angle in degrees about Y axis.
 *	  <Z>	:	rotation angle in degrees about Z axis.
 * OUTPUT
 *    Applies a rotation by given factors BEFORE the current transform.
 * RETURN VALUE
 *    None.
 * EXAMPLE 
 *    % affTrans.preRotate(10, -8, 9.65);
 ***/
void AqAffineTransform::preRotate(double x, double y, double z) {	
	this->_getRotationMatrix(x, y, z, this->tempMatrix1);
	this->preApplyTransform(this->tempMatrix1);
};


void AqAffineTransform::preRotate(const double x[3]) {
		this->preRotate(x[0], x[1], x[2]);
};


void AqAffineTransform::preRotate(const float x[3]) {
		this->preRotate(x[0], x[1], x[2]);
};	
	
/******
 * NAME
 *    preScale
 * SYNOPSIS
 *    preScale <X> <Y> <Z>
 * DESCRIPTION
 *    Applies a scaling by given factors BEFORE the current transform.
 * PARAMETERS
 *	  <X>	:	scale factor along X axis.
 *	  <Y>	:   scale factor along Y axis.
 *	  <Z>	:	scale factor along Z axis.
 * OUTPUT
 *    Applies a scaling by given factors BEFORE the current transform.
 * RETURN VALUE
 *    None.
 * EXAMPLE 
 *    % affTrans.preScale(1, 8, 9);
 ***/
void AqAffineTransform::preScale(double x, double y, double z) {	
	this->_getScalingMatrix(x, y, z, this->tempMatrix1);
	this->preApplyTransform(this->tempMatrix1);
};


void AqAffineTransform::preScale(const double x[3]) {
		this->preScale(x[0], x[1], x[2]);
};

void AqAffineTransform::preScale(const float x[3]) {
		this->preScale(x[0], x[1], x[2]);
};


/******
 * NAME
 *    preApplyTransform
 * SYNOPSIS
 *    preApplyTransform <* inMatrix>
 * DESCRIPTION
 *    Applies <* inMatrx> BEFORE the current transform.
 * PARAMETERS
 *	  <* inMatrix>	:	Pointer to a AqMatrix4x4 object. 
 * OUTPUT
 *    Applies <* inMatrx> BEFORE the current transform.
 * RETURN VALUE
 *    None.
 * EXAMPLE 
 *	  % AqMatrix4x4 otherMat;	
 *    % affTrans.preApplyTransform(&otherMat);
 ***/	
void AqAffineTransform::preApplyTransform(AqMatrix4x4 *inMatrix) {
	this->Matrix->MultiplyMatrix(this->Matrix, inMatrix, this->Matrix);	
	//this->_updated = true;
}


void AqAffineTransform::preApplyTransform(AqAffineTransform *other) {
		other->getMatrix(tempMatrix1);
		preApplyTransform(tempMatrix1);
}	
	
/******************************************* POST operations *****************************/
// apply the incoming transform *after* the existing matrix
// i.e. if existing matrix is A, and incoming transform is T,
// a post-transform would mean 
//		A = T.A

/******
 * NAME
 *    postTranslate
 * SYNOPSIS
 *    postTranslate <X> <Y> <Z>
 * DESCRIPTION
 *    Applies a translation by given factors AFTER the current transform.
 * PARAMETERS
 *	  <X>	:	translation factor along X axis.
 *	  <Y>	:	translation factor along Y axis.
 *	  <Z>	:	translation factor along Z axis.
 * OUTPUT
 *    Applies a translation by given factors AFTER the current transform.
 * RETURN VALUE
 *    None.
 * EXAMPLE 
 *    % affTrans.postTranslate(10, -8, 9.65);
 ***/
void AqAffineTransform::postTranslate(double x, double y, double z) {
	this->_getTranslationMatrix(x, y, z, tempMatrix1);
	this->Matrix->MultiplyMatrix(tempMatrix1, this->Matrix, this->Matrix);
};


void AqAffineTransform::postTranslate(const double x[3]) {
		this->postTranslate(x[0], x[1], x[2]);
};

void AqAffineTransform::postTranslate(const float x[3]) {
		this->postTranslate(x[0], x[1], x[2]);
};

/******
 * NAME
 *    postRotate 
 * SYNOPSIS
 *    postRotate <X> <Y> <Z>
 * DESCRIPTION
 *    Applies a rotation by given factors AFTER the current transform.
 *
 *	  Note: To avoid "gimbal lock" situations, define TR_USE_QUATERNIONS flag
 *	  when compiling:
 *			#define TR_USE_QUATERNIONS
 *
 * PARAMETERS
 *	  <X>	:	rotation angle in degrees about X axis.
 *	  <Y>	:	rotation angle in degrees about Y axis.
 *	  <Z>	:	rotation angle in degrees about Z axis.
 * OUTPUT
 *    Applies a rotation by given factors AFTER the current transform.
 * RETURN VALUE
 *    None.
 * EXAMPLE 
 *    % affTrans.postRotate(10, -8, 9.65);
 ***/
void AqAffineTransform::postRotate(double x, double y, double z) {	
	this->_getRotationMatrix(x, y, z, this->tempMatrix1);
	this->postApplyTransform(this->tempMatrix1);
};



void AqAffineTransform::postRotate(const double x[3]) {
		this->postRotate(x[0], x[1], x[2]);
};

void AqAffineTransform::postRotate(const float x[3]) {
		this->postRotate(x[0], x[1], x[2]);
};	



/******
 * NAME
 *    postScale
 * SYNOPSIS
 *    postScale <X> <Y> <Z>
 * DESCRIPTION
 *    Applies a scaling by given factors AFTER the current transform.
 * PARAMETERS
 *	  <X>	:	scale factor along X axis.
 *	  <Y>	:   scale factor along Y axis.
 *	  <Z>	:	scale factor along Z axis.
 * OUTPUT
 *    Applies a scaling by given factors AFTER the current transform.
 * RETURN VALUE
 *    None.
 * EXAMPLE 
 *    % affTrans.postScale(1, 8, 9);
 ***/
void AqAffineTransform::postScale(double x, double y, double z) {	
	this->_getScalingMatrix(x, y, z, this->tempMatrix1);
	this->postApplyTransform(this->tempMatrix1);
};


void AqAffineTransform::postScale(const double x[3]) {
		this->postScale(x[0], x[1], x[2]);
};

void AqAffineTransform::postScale(const float x[3]) {
		this->postScale(x[0], x[1], x[2]);
};

/******
 * NAME
 *    postApplyTransform
 * SYNOPSIS
 *    postApplyTransform <* inMatrix>
 * DESCRIPTION
 *    Applies <* inMatrx> AFTER the current transform.
 * PARAMETERS
 *	  <* inMatrix>	:	Pointer to a AqMatrix4x4 object. 
 * OUTPUT
 *    Applies <* inMatrx> AFTER the current transform.
 * RETURN VALUE
 *    None.
 * EXAMPLE 
 *	  % AqMatrix4x4 otherMat;	
 *    % affTrans.postApplyTransform(&otherMat);
 ***/
void AqAffineTransform::postApplyTransform(AqMatrix4x4 *inMatrix) {
	this->Matrix->MultiplyMatrix(inMatrix, this->Matrix, this->Matrix);
	//this->_updated = true;
}

void AqAffineTransform::postApplyTransform(AqAffineTransform *other) {
		other->getMatrix(tempMatrix1);
		postApplyTransform(tempMatrix1);
}		
	


	
/**************** set/get parameters ******************************************/

/******
 * NAME
 *    setDoFParameters
 * SYNOPSIS
 *    setDoFParameters <dofParamArray>
 * DESCRIPTION
 *    Sets the degree of freedom parameter values from <dofParamArray>.
 * PARAMETERS
 *	  <dofParamArray>	:	double array of kAqDoFCount elements.
 * OUTPUT
 *    Sets the degree of freedom parameter values from <dofParamArray>.
 *	  Updates the transform matrix immediately.	
 * RETURN VALUE
 *   Execution status of int.
 * EXAMPLE 
 *	  % double testDoFParams[kAqDoFCount];
 *    % int ret = affTrans.setDoFParameters(testDofParams); 
 ***/
int AqAffineTransform::setDoFParameters(double parameters[kAqDoFCount]) {
	int res = kAqSuccess;

	
	memcpy(this->_dofParameters, parameters, kAqDoFCount*sizeof(double));

	this->update();
	return res;
}

/******
 * NAME
 *    getDoFParameters
 * SYNOPSIS
 *    getDoFParameters <* dofParamArray>
 * DESCRIPTION
 *    Gets the degree of freedom parameter values into <* dofParamArray>.
 * PARAMETERS
 *	  <* dofParamArray>	:	Pointer to double array of kAqDoFCount elements.
 * OUTPUT
 *    Sets the degree of freedom parameter values of <dofParamArray> to its own current values. 
 * RETURN VALUE
 *   None.
 * EXAMPLE 
 *	  % double testDoFParams[kAqDoFCount];
 *    % int ret = affTrans.getDoFParameters(&(testDofParams[0])); 
 ***/
void AqAffineTransform::getDoFParameters(double *out) {
		memcpy((void *)out, (const void *)&(this->_dofParameters[0]), kAqDoFCount*sizeof(double));		
}

/******
 * NAME
 *    setDoFParameter
 * SYNOPSIS
 *    setDoFParameter <index> <value>
 * DESCRIPTION
 *    Sets the degree of freedom parameter value for <index> to <value>.
 * PARAMETERS
 *	  <index>	:	Index of parameter to set. [0 ... kAqDoFCount-1 ]
 *	  <value>	:	Value to set to.
 * OUTPUT
 *    Sets the degree of freedom parameter value for <index> to <value>.
 *	  Updates the transform matrix immediately.	
 * RETURN VALUE
 *   Execution status of int.
 * EXAMPLE  
 *    % int ret = affTrans.setDoFParameter(1, 10.87); 
 ***/
int AqAffineTransform::setDoFParameter(unsigned int i, double value) {
	int res = kAqSuccess;

	if (i >= 0 && i <kAqDoFCount) {
		//if (this->isActiveDoF(i)==1) {
			this->_dofParameters[i] = value;
		//}
	} else {
		res = kAqErrBadInputParameters;
		//#ifdef TR_DEBUG	
			//TR_LOG(this->logptr(), "WARNING(AqAffineTransform::setDoFParameter:<%d>): Error setting DoF parameter for %d!", res, i);
			//this->warning(TR_IGNORE);				
		//#endif
	}	
	//this->update();
	return res;
}

/******
 * NAME
 *    getDoFParameter
 * SYNOPSIS
 *    getDoFParameter <index>
 * DESCRIPTION
 *    Retrieves current degree of freedom parameter value for <index>.
 * PARAMETERS
 *	  <index>	:	Index of parameter to set. [0 ... kAqDoFCount-1 ] 
 * OUTPUT
 *    Retrieves current degree of freedom parameter value for <index>.
 * RETURN VALUE
 *    Degree of freedom parameter value for <index>.
 * EXAMPLE  
 *    % double dofValueTransZ = affTrans.getDoFParameter(2); 
 ***/
double AqAffineTransform::getDoFParameter(unsigned int i) {
	double res = 0.0;

	if (i >= 0 && i < kAqDoFCount) {
		//if (this->isActiveDoF(i)==1) {
			res = this->_dofParameters[i];
		//} else {
			// disabled! return a 0.0 value for rotation + shear + translation
			//res = 0.0;
			// return 1.0 if scaling dof
			//if (i>=6 && i <=8) {
				//res = 1.0;
			//}  
		//}
	} else {
		#ifdef TR_DEBUG	
			TR_LOG(this->logptr(),"WARNING(\nAqAffineTransform::getDoFParameter:<4>): \
				Error getting DoF parameter for %d. Index out of bound.!",i);
			this->warning(TR_IGNORE);			
		#endif
	}// end if
	return res;
}

/******
 * NAME
 *    setDoFParameter
 * SYNOPSIS
 *    setDoFParameter <tag> <value>
 * DESCRIPTION
 *    Sets the degree of freedom parameter value for <tag> to <value>.
 * PARAMETERS
 *	  <tag>		:	Tag of parameter to set. e.g. kAqTransX. Type: eAqTagDoF.
 *	  <value>	:	Value to set to.
 * OUTPUT
 *    Sets the degree of freedom parameter value for <tag> to <value>.
 *	  Updates the transform matrix immediately.	
 * RETURN VALUE
 *   Execution status of int.
 * EXAMPLE  
 *    % int ret = affTrans.setDoFParameter(kAqTransY, 10.87); 
 ***/
int AqAffineTransform::setDoFParameter(eAqTagDoF i, double value) {
	int res=kAqSuccess;

	if (i >= 0 && i < kAqDoFCount) {
		_dofParameters[i] = value;	

	} else {
		res = kAqErrBadInputParameters;
	//#ifdef TR_DEBUG	
		//TR_LOG(this->logptr(), "(AqAffineTransform::setDoFParameter:<%d>): Index out of bound!", res);
		//this->warning(TR_IGNORE);			
	//#endif
	}	
	//this->update();
	return res;
}

/******
 * NAME
 *    getDoFParameter
 * SYNOPSIS
 *    getDoFParameter <tag> 
 * DESCRIPTION
 *    Retrieves current degree of freedom parameter value for <tag> .
 * PARAMETERS
 *	  <tag> 	:	tag of parameter to set. [0 ... kAqDoFCount-1 ] 
 * OUTPUT
 *    Retrieves current degree of freedom parameter value for <tag>.
 * RETURN VALUE
 *    Degree of freedom parameter value for <tag>.
 * EXAMPLE  
 *    % double dofValueTransZ = affTrans.getDoFParameter(kAqTransZ); 
 ***/
double AqAffineTransform::getDoFParameter(eAqTagDoF i) {	
	return this->getDoFParameter((unsigned int) i);
}


/******
 * NAME
 *    isEqual
 * SYNOPSIS
 *    AqAffineTransform::isEqual <* transform1> <* transform2>
 * DESCRIPTION
 *    Checks whether matrices of <* transform1> and <* transform2> are equal.
 * PARAMETERS
 *	  <* transform1>	:	pointer to first transform
 *	  <* transform2>	:	pointer to second transform
 * OUTPUT
 *    Tells whether the two transforms are equal.
 * RETURN VALUE
 *    true/false.
 * EXAMPLE  
 *    % bool equalFlag = AqAffineTransform::isEqual(&affTrans1, &affTrans2); 
 ***/
bool AqAffineTransform::isEqual(AqAffineTransform *other1, AqAffineTransform *other2) {
	other1->getMatrix(this->tempMatrix1);
	other2->getMatrix(this->tempMatrix2);
	return this->tempMatrix1->IsEqual(this->tempMatrix2);
}

bool AqAffineTransform::isEqual(AqAffineTransform *other) {
		return isEqual(this, other);	
}

/******
 * NAME
 *    getMatrix
 * SYNOPSIS
 *    getMatrix <* mat>
 * DESCRIPTION
 *    Copies current matrix to <* mat>.
 * PARAMETERS 
 *	  <* mat>	:	pointer to output matrix object.
 * OUTPUT
 *    Copies current matrix to <* mat>.
 * RETURN VALUE
 *    None.
 * EXAMPLE  
 *    % affTrans.getMatrix(&affMat); 
 ***/
void AqAffineTransform::getMatrix(AqMatrix4x4 *mat) {
	mat->Copy(this->Matrix);
}

/******
 * NAME
 *    randomize
 * SYNOPSIS
 *    randomize 
 * DESCRIPTION
 *    Randomizes the current transform matrix.
 *	  NOTE: For now only rotation is randomized.	
 * PARAMETERS 
 *	  None
 * OUTPUT
 *    Randomizes the current transform matrix.
 * RETURN VALUE
 *    None.
 * EXAMPLE  
 *    % affTrans.randomize(); 
 ***/
void AqAffineTransform::randomize() {
	this->Matrix->Randomize();
	this->preRotate(rand(), rand(), rand());
#ifdef TR_DEBUG
	TR_LOG(this->logptr(),"\nMatrix after transform randomize: ");
	this->dump();	
	this->print();
#endif
}

/******
 * NAME
 *    copy
 * SYNOPSIS
 *    copy <* from> 
 * DESCRIPTION
 *    Copies the transform (degree values/ matrix) from <* from>.
 * PARAMETERS
 *	  <* from>		:	pointer to a AqAffineTransform object.
 * OUTPUT
 *    Copies the transform (degree values/ matrix) from <* from>.
 * RETURN VALUE
 *	  None.
 * EXAMPLE     
 *	  % AqAffineTransform otherTrans;
 *	  %	affTrans.copy(&otherTrans);
 ***/
void AqAffineTransform::copy(AqAffineTransform *from) {		
		from->getDoFParameters(&(this->_dofParameters[0]));
		from->getOrigin(this->tOrigin);		
		this->copyMatrix(from);		
};

	/* Use ONLY when not worried about inconsistency between dOf parameters and the matrix */
void AqAffineTransform::copyMatrix(AqMatrix4x4 *from) {
		this->Matrix->Copy(from);		
	//	this->_updated = true;
};
	

void AqAffineTransform::copyMatrix(AqAffineTransform *from) {
		from->getMatrix(this->Matrix);		
	//	this->_updated = true;
};



void AqAffineTransform::setShearFlagDoF(unsigned char inflag) {
		for (unsigned char i=9; i<=11; i++) {
			this->setFlagDoF(i, inflag);
		}
}
	
void AqAffineTransform::setTranslationFlagDoF(unsigned char inflag) {
		for (unsigned char i=0; i<=2; i++) {
			this->setFlagDoF(i, inflag);
		}
}


void AqAffineTransform::setRotationFlagDoF(unsigned char inflag) {
		for (unsigned char i=3; i<=5; i++) {
			this->setFlagDoF(i, inflag);
		}
}
	
void AqAffineTransform::setScalingFlagDoF(unsigned char inflag) {
		for (unsigned char i=6; i<=8; i++) {
			this->setFlagDoF(i, inflag);
		}
}
	

bool AqAffineTransform::isActiveShearDoF() {		
		for (unsigned char i=9; i<=11; i++) {
			if (!this->isActiveDoF(i)) {
				return false;
			}
		}
		return true;
}


bool AqAffineTransform::isActiveTranslationDoF() {		
		for (unsigned char i=0; i<=2; i++) {
			if (!this->isActiveDoF(i)) {
				return false;
			}
		}
		return true;
}

	
bool AqAffineTransform::isActiveRotationDoF() {
		for (unsigned char i=3; i<=5; i++) {
			if (!this->isActiveDoF(i)) {
				return false;
			}
		}
		return true;
}
	
bool AqAffineTransform::isActiveScalingDoF() {
		for (unsigned char i=6; i<=8; i++) {
			if (!this->isActiveDoF(i)) {
				return false;
			}
		}
		return true;
}

unsigned char AqAffineTransform::firstActiveDegreeIndex(char from) {
		unsigned char res = from+1;
		for (unsigned char i=(from+1); i<kAqDoFCount; i++) {
			if (this->isActiveDoF(i)) {
				res = i;
				break;
			}
		}
		return res;
}




// point transformations
// note that if p[4] == 0.0, it is considered a vector hence not scaled/translated etc
//void AqAffineTransform::transformPoint(const float in[4], float out[4]) {		
//			this->Matrix->MultiplyPoint(in, out);		
//}

void AqAffineTransform::transformPoint(double in[4]) {
		this->Matrix->MultiplyPoint(in, in);
}

void AqAffineTransform::transformPoint(const double in[4], double out[4]) {
		this->Matrix->MultiplyPoint(in, out);
}

void AqAffineTransform::setOrigin(AqVector *tIn) {
		this->tOrigin->copy(tIn);
}

void AqAffineTransform::getOrigin(AqVector *tOut) {
		tOut->copy(this->tOrigin);
}
	



/******************************** Private methods ******************************************************************/


void AqAffineTransform::_getTranslationMatrix(double tx, double ty, double tz, AqMatrix4x4 *outMat) {
	outMat->Identity();
	outMat->SetElement(0,3,tx);
	outMat->SetElement(1,3,ty);
	outMat->SetElement(2,3,tz);
}

// assumes the angles are in degrees
void AqAffineTransform::_getRotationMatrix(double rx, double ry, double rz, AqMatrix4x4 *outMat) {

#ifdef TR_USE_QUATERNIONS
	_quatX.initAxisAngle(1, 0, 0, rx);
	_quatY.initAxisAngle(0, 1, 0, ry);
	_quatZ.initAxisAngle(0, 0, 1, rz);

	_quatX.multiply(&_quatY);
	_quatX.multiply(&_quatZ);

	_quatX.getMatrix(outMat);
#else 
	double cosrx = cos(rx*kAqPI/180.0);
	double cosry = cos(ry*kAqPI/180.0);
	double cosrz = cos(rz*kAqPI/180.0);
	double sinrx = sin(rx*kAqPI/180.0);
	double sinry = sin(ry*kAqPI/180.0);
	double sinrz = sin(rz*kAqPI/180.0);

	outMat->Identity();	
	outMat->SetElement(0,0,cosry*cosrz);
	outMat->SetElement(0,1,cosry*sinrz);
	outMat->SetElement(0,2,-sinry);
	outMat->SetElement(1,0,(sinrx*sinry*cosrz-cosrx*sinrz));
	outMat->SetElement(1,1,(sinrx*sinry*sinrz+cosrx*cosrz));
	outMat->SetElement(1,2,sinrx*cosry);
	outMat->SetElement(2,0,(cosrx*sinry*cosrz+sinrx*sinrz));
	outMat->SetElement(2,1,(cosrx*sinry*sinrz-sinrx*cosrz));
	outMat->SetElement(2,2,cosrx*cosry);
#endif
}

void AqAffineTransform::_getScalingMatrix(double sx, double sy, double sz, AqMatrix4x4 *outMat) {
	outMat->Identity();
	outMat->SetElement(0,0,sx);	
	outMat->SetElement(1,1,sy);
	outMat->SetElement(2,2,sz);		
}


void AqAffineTransform::_getShearingMatrix(double sxy, double syz, double sxz, 
						AqMatrix4x4 *outMat) {
	AqMatrix4x4 *tMat = new AqMatrix4x4;
	tMat->Identity();
	outMat->Identity();	

	// get skew about Y, premultiply with result
	this->_getShearingMatrixY(sxz, tMat);
	outMat->MultiplyMatrix(outMat, tMat, outMat);	

	// get skew about Z, premultiply with result
	this->_getShearingMatrixZ(sxy, tMat);
	outMat->MultiplyMatrix(outMat, tMat, outMat);

	// get skew about X, premultiply with result
	this->_getShearingMatrixX(syz, tMat);
	outMat->MultiplyMatrix(outMat, tMat, outMat);

	// the result would be:
	// S = Tz.Ty.Tx.I

	delete tMat;
}


// all the angles are assumed to be in degrees
void AqAffineTransform::_getShearingMatrixX(double syz, AqMatrix4x4 *outMat) {
	outMat->Identity();	
	outMat->SetElement(1, 2,tan(syz*(kAqPI/180.0))); 
} 

// all the angles are assumed to be in degrees
void AqAffineTransform::_getShearingMatrixY(double sxz, AqMatrix4x4 *outMat) {
	outMat->Identity(); 	
	outMat->SetElement(0, 2,tan(sxz*(kAqPI/180.0)));  
}

// all the angles are assumed to be in degrees
void AqAffineTransform::_getShearingMatrixZ(double sxy, AqMatrix4x4 *outMat) {
	outMat->Identity();	
	outMat->SetElement(1, 0,tan(sxy*(kAqPI/180.0)));  	
}