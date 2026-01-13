// AqTree.cpp: implementation of the AqTree namespace.
//
// -*- C++ -*-
// Copyright 2006 PreXion 
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
// Filename:	AqTree.cpp
// Author:		Sha He
// Created:		Monday, May 08, 2006 at 10:15:55 AM
//
//////////////////////////////////////////////////////////////////////

// Sha 2006.5.8 Save trees in COF (ID:5592) 
// DavidG 6-05-2006 Add extra const functions, and IsRoot, HasChildren

#include "AqTree.h"
using namespace AqTree;

//------------------------------------------------------------------------------
AqTreeNode::AqTreeNode()
{
	m_pParent = NULL;
	m_itForNextChild = m_Children.end();
}


//------------------------------------------------------------------------------
AqTreeNode::~AqTreeNode()
{
	m_pParent = NULL;

	// Delete all children:
	CChildList::iterator itChild;
	for (itChild = m_Children.begin(); itChild != m_Children.end(); itChild++)
	{
		AqTreeNode* p = *itChild;
		delete p;
	}
	m_Children.clear();
	m_itForNextChild = m_Children.end();
}

//------------------------------------------------------------------------------
AqTreeNode* AqTreeNode::CreateChild()
{
	AqTreeNode* pNewNode = 0;
	try
	{
		pNewNode = AllocateNewNode();
	}
	catch (...)
	{
		pNewNode = 0;
	}

	if (pNewNode)
	{
		pNewNode->m_pParent = this;
		m_itForNextChild = m_Children.insert(m_Children.end(), pNewNode);
	}
	return pNewNode;
}

//------------------------------------------------------------------------------
bool AqTreeNode::IsRoot() const
{
	return m_pParent == NULL;
}

//------------------------------------------------------------------------------
bool AqTreeNode::HasChildren() const
{
	return (m_Children.size() != 0);
}

//------------------------------------------------------------------------------
AqTreeNode* AqTreeNode::GetParent()
{
	return m_pParent;
}

const AqTreeNode* AqTreeNode::GetParent() const
{
	return m_pParent;
}

//------------------------------------------------------------------------------
AqTreeNode* AqTreeNode::GetFirstChild()
{
	m_itForNextChild = m_Children.begin();
	if (m_itForNextChild == m_Children.end())
	{
		return 0;
	}
	return *m_itForNextChild;
}

//------------------------------------------------------------------------------
AqTreeNode* AqTreeNode::GetNextChild()
{
	if (m_itForNextChild == m_Children.end())
	{
		return 0;
	}

	m_itForNextChild++;
	if (m_itForNextChild == m_Children.end())
	{
		return 0;
	}
	return *m_itForNextChild;
}
