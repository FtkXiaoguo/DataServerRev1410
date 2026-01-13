// AqTree.h: interface for the AqTree namespace.
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
// Filename:	AqTree.h
// Author:		Sha He
// Created:		Monday, May 08, 2006 at 10:15:55 AM
//

#ifndef _AQTREE_H_
#define _AQTREE_H_
//////////////////////////////////////////////////////////////////////////

// Sha 2006.5.8 Save trees in COF (ID:5592) 
// DavidG 6-05-2006 Add extra const functions, and IsRoot, HasChildren

#include <list>

namespace AqTree  
{

//-------------------------------------------------------------------
// A base node class to maintain parent/children relationship
class AqTreeNode
{
public:
	AqTreeNode();
	virtual ~AqTreeNode();

	AqTreeNode* CreateChild();  // You can then fill in the returned object. Returns NULL if faild.

	bool IsRoot() const;
	bool HasChildren() const;

	AqTreeNode*       GetParent();        // Returns parent of this. NULL if this is root.
	const AqTreeNode* GetParent() const;  // Returns parent of this. NULL if this is root.

	AqTreeNode* GetFirstChild();
	AqTreeNode* GetNextChild();        // Call "GetFirstChild" first, then call this function in a loop.
	
	// Children as a list:
	typedef std::list<AqTreeNode*> CChildList;
	CChildList&       GetChildList()       { return m_Children; };
	const CChildList& GetChildList() const { return m_Children; };

protected:
	AqTreeNode* m_pParent;  // Parent of this.
	CChildList m_Children;  // Children of this.

	mutable CChildList::iterator m_itForNextChild;  // Used by GetNextChild.
	
	virtual AqTreeNode* AllocateNewNode() const =0;  // Has to be overloaded as { return new DerivedType; }; See example below.
};


//-------------------------------------------------------------------
// A branch template class
template <class ElementType>
class AqTreeBranch :
	public std::list<ElementType>,  // Contains a continuous set of elements without bifurcation.
	public AqTreeNode
{
public:
	AqTreeBranch() {};
	~AqTreeBranch() {};

	//===============================
	// Overloads from AqTreeNode:

	AqTreeBranch* CreateChild()  // You can then fill in the returned object. Returns NULL if faild.
	{
		return (AqTreeBranch*)AqTreeNode::CreateChild();
	};

	AqTreeBranch* GetParent()  // Returns parent of this. NULL if this is root.
	{
		return (AqTreeBranch*)AqTreeNode::GetParent();
	};

	const AqTreeBranch* GetParent() const  // Returns parent of this. NULL if this is root.
	{
		return (AqTreeBranch*)AqTreeNode::GetParent();
	};

	AqTreeBranch* GetFirstChild()
	{
		return (AqTreeBranch*)AqTreeNode::GetFirstChild();
	};

	AqTreeBranch* GetNextChild()  // Call "GetFirstChild" first, then call this function in a loop.
	{
		return (AqTreeBranch*)AqTreeNode::GetNextChild();
	};


//		Here are some operations we want to support in the tree class in the future:
//		1. Traverse the tree and find if a point is inside
//		2. Append a branch to the tree, but need to make sure no conflict
//		3. Get a branch segment or a partial tree out (clone)
//		4. Merge two trees into one when necessary

protected:

	virtual AqTreeNode* AllocateNewNode() const
	{
		return new AqTreeBranch<ElementType>;
	};
};



}; // end namespace AqTree

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQTREE_H_
