// -*- C++ -*-
// Copyright 2006 TeraRecon, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF TERARECON, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART,
// IS STRICTLY PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN
// PERMISSION OF TERARECON, INC.
//
// Filename:	TRThreadJob.h
// Author:		David Guigonis
// Created:		Friday, March 24, 2006 at 11:49:33 AM
//

#ifndef TRTHREADJOB_H
#define TRTHREADJOB_H
///////////////////////////////////////////////////////////////////////////////

class TRJobBase
{
public:

	virtual void execute() = 0;

	unsigned int id;
};

///////////////////////////////////////////////////////////////////////////////
//EOF
#endif	// TRTHREADJOB_H
