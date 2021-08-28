#include "stdafx.h"
#include "ptr_list.hpp"

//=========================================================================
// ptr_node_data
//=========================================================================

ptr_node_data* __stdcall ptr_node_data::Create(ptr_node_data*& pHead, UINT nMax, UINT uElement)
{
	ASSERT(nMax > 0 && uElement > 0);
	ptr_node_data* p = (ptr_node_data*) new BYTE[sizeof(ptr_node_data) + nMax * uElement];

	// may throw exception
	p->pNext = pHead;
	pHead = p;  // change head
	return p;
}

//=========================================================================

void ptr_node_data::FreeNodeListMem()
{
	ptr_node_data* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		ptr_node_data* pNext = p->pNext;
		delete[] bytes;
		p = pNext;
	}
}

//=========================================================================
// ptr_list
//=========================================================================

ptr_list::ptr_list(int nBlockSize)
{
	ASSERT(nBlockSize > 0);

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

//=========================================================================

void ptr_list::RemoveAll()
{
	// destroy elements
	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks->FreeNodeListMem();
	m_pBlocks = NULL;
}

//=========================================================================

ptr_list::~ptr_list()
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

//=========================================================================

ptr_list::ptr_node* ptr_list::NewNode(ptr_list::ptr_node* pPrev, ptr_list::ptr_node* pNext)
{
	if (m_pNodeFree == NULL)
	{
		// add another block
		ptr_node_data* pNewBlock = ptr_node_data::Create(m_pBlocks, m_nBlockSize, sizeof(ptr_node));

		// add them into free list
		ptr_node* pNode = (ptr_node*) pNewBlock->data();

		// free in reverse order
		pNode += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pNode--)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}
	ASSERT(m_pNodeFree != NULL);  

	ptr_list::ptr_node* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
	
    m_nCount++;
	ASSERT(m_nCount > 0);  // make sure we don't overflow

    pNode->data = 0; // start with zero

	return pNode;
}

//=========================================================================

void ptr_list::FreeNode(ptr_list::ptr_node* pNode)
{
	pNode->pNext = m_pNodeFree;
	m_pNodeFree = pNode;
	m_nCount--;
	ASSERT(m_nCount >= 0);  // make sure we don't underflow

	// if no more elements, cleanup completely
	if (m_nCount == 0)
    {
		RemoveAll();
    }
}

//=========================================================================

DATAPOS ptr_list::AddHead(void* newElement)
{
	ptr_node* pNewNode = NewNode(NULL, m_pNodeHead);
	pNewNode->data = newElement;
	if (m_pNodeHead != NULL)
    {
		m_pNodeHead->pPrev = pNewNode;
    }
	else
    {
		m_pNodeTail = pNewNode;
    }
	m_pNodeHead = pNewNode;
	return (DATAPOS) pNewNode;
}

//=========================================================================

DATAPOS ptr_list::AddTail(void* newElement)
{
	ptr_node* pNewNode = NewNode(m_pNodeTail, NULL);
	pNewNode->data = newElement;
	if (m_pNodeTail != NULL)
    {
		m_pNodeTail->pNext = pNewNode;
    }
	else
    {
		m_pNodeHead = pNewNode;
    }
	m_pNodeTail = pNewNode;
	return (DATAPOS) pNewNode;
}

//=========================================================================

void ptr_list::AddHead(ptr_list* pNewList)
{
	ASSERT(pNewList != NULL);

	// add a list of same elements to head (maintain order)
	DATAPOS pos = pNewList->GetTailPosition();
	while (pos != NULL)
    {
		AddHead(pNewList->GetPrev(pos));
    }
}

//=========================================================================

void ptr_list::AddTail(ptr_list* pNewList)
{
	ASSERT(pNewList != NULL);

	// add a list of same elements
	DATAPOS pos = pNewList->GetHeadPosition();
	while (pos != NULL)
    {
		AddTail(pNewList->GetNext(pos));
    }
}

//=========================================================================

void* ptr_list::RemoveHead()
{
	ASSERT(m_pNodeHead != NULL);  
	ASSERT(AfxIsValidAddress(m_pNodeHead, sizeof(ptr_node)));

	ptr_node* pOldNode = m_pNodeHead;
	void* returnValue = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
    {
		m_pNodeHead->pPrev = NULL;
    }
	else
    {
		m_pNodeTail = NULL;
    }
	FreeNode(pOldNode);
	return returnValue;
}

//=========================================================================

void* ptr_list::RemoveTail()
{
	ASSERT(m_pNodeTail != NULL); 
	ASSERT(AfxIsValidAddress(m_pNodeTail, sizeof(ptr_node)));

	ptr_node* pOldNode = m_pNodeTail;
	void* returnValue = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
    {
		m_pNodeTail->pNext = NULL;
    }
	else
    {
		m_pNodeHead = NULL;
    }
	FreeNode(pOldNode);
	return returnValue;
}

//=========================================================================

DATAPOS ptr_list::InsertBefore(DATAPOS position, void* newElement)
{
	if (position == NULL)
		return AddHead(newElement); // insert before nothing, so insert head of the list

	// Insert it before position
	ptr_node* pOldNode = (ptr_node*) position;
	ptr_node* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
	pNewNode->data = newElement;

	if (pOldNode->pPrev != NULL)
	{
		ASSERT(AfxIsValidAddress(pOldNode->pPrev, sizeof(ptr_node)));
		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeHead);
		m_pNodeHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;
	return (DATAPOS) pNewNode;
}

//=========================================================================

DATAPOS ptr_list::InsertAfter(DATAPOS position, void* newElement)
{
	if (position == NULL)
		return AddTail(newElement); // insert after nothing, so insert tail of the list

	// Insert it before position
	ptr_node* pOldNode = (ptr_node*) position;
	ASSERT(AfxIsValidAddress(pOldNode, sizeof(ptr_node)));
	ptr_node* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;

	if (pOldNode->pNext != NULL)
	{
		ASSERT(AfxIsValidAddress(pOldNode->pNext, sizeof(ptr_node)));
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeTail);
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	return (DATAPOS) pNewNode;
}

//=========================================================================

void ptr_list::RemoveAt(DATAPOS position)
{
	ptr_node* pOldNode = (ptr_node*) position;
	ASSERT(AfxIsValidAddress(pOldNode, sizeof(ptr_node)));

	// remove pOldNode from list
	if (pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->pNext;
	}
	else
	{
		ASSERT(AfxIsValidAddress(pOldNode->pPrev, sizeof(ptr_node)));
		pOldNode->pPrev->pNext = pOldNode->pNext;
	}
	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->pPrev;
	}
	else
	{
		ASSERT(AfxIsValidAddress(pOldNode->pNext, sizeof(ptr_node)));
		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}
	FreeNode(pOldNode);
}

//=========================================================================

int ptr_list::GetCount() const
{   
    return m_nCount; 
}

//=========================================================================

BOOL ptr_list::IsEmpty() const
{ 
    return m_nCount == 0; 
}

//=========================================================================

void*& ptr_list::GetHead()	
{ 
    ASSERT(m_pNodeHead != NULL);		
    return m_pNodeHead->data; 
}

//=========================================================================

void* ptr_list::GetHead() const	
{ 
    ASSERT(m_pNodeHead != NULL);		
    return m_pNodeHead->data; 
}

//=========================================================================

void*& ptr_list::GetTail()	
{ 
    ASSERT(m_pNodeTail != NULL);		
    return m_pNodeTail->data; 
}

//=========================================================================

void* ptr_list::GetTail() const
{ 
    ASSERT(m_pNodeTail != NULL);	
    return m_pNodeTail->data; 
}

//=========================================================================

DATAPOS ptr_list::GetHeadPosition() const	
{ 
    return (DATAPOS) m_pNodeHead; 
}

//=========================================================================

DATAPOS ptr_list::GetTailPosition() const	
{ 
    return (DATAPOS) m_pNodeTail; 
}

//=========================================================================

void*& ptr_list::GetNext(DATAPOS& rPosition) // return *Position++	
{ 
    ptr_node* pNode = (ptr_node*) rPosition;		
    ASSERT(AfxIsValidAddress(pNode, sizeof(ptr_node)));		
    rPosition = (DATAPOS) pNode->pNext;		
    return pNode->data; 
}

//=========================================================================

void* ptr_list::GetNext(DATAPOS& rPosition) const // return *Position++	
{ 
    ptr_node* pNode = (ptr_node*) rPosition;		
    ASSERT(AfxIsValidAddress(pNode, sizeof(ptr_node)));		
    rPosition = (DATAPOS) pNode->pNext;		
    return pNode->data; 
}

//=========================================================================

void*& ptr_list::GetPrev(DATAPOS& rPosition) // return *Position--	
{ 
    ptr_node* pNode = (ptr_node*) rPosition;		
    ASSERT(AfxIsValidAddress(pNode, sizeof(ptr_node)));		
    rPosition = (DATAPOS) pNode->pPrev;		
    return pNode->data; 
}

//=========================================================================

void* ptr_list::GetPrev(DATAPOS& rPosition) const // return *Position--	
{ 
    ptr_node* pNode = (ptr_node*) rPosition;		
    ASSERT(AfxIsValidAddress(pNode, sizeof(ptr_node)));		
    rPosition = (DATAPOS) pNode->pPrev;		
    return pNode->data; 
}

//=========================================================================

void*& ptr_list::GetAt(DATAPOS position)	
{ 
    ptr_node* pNode = (ptr_node*) position;		
    ASSERT(AfxIsValidAddress(pNode, sizeof(ptr_node)));		
    return pNode->data; 
}

//=========================================================================

void* ptr_list::GetAt(DATAPOS position) const	
{ 
    ptr_node* pNode = (ptr_node*) position;		
    ASSERT(AfxIsValidAddress(pNode, sizeof(ptr_node)));		
    return pNode->data; 
}

//=========================================================================

void ptr_list::SetAt(DATAPOS pos, void* newElement)	
{ 
    ptr_node* pNode = (ptr_node*) pos;		
    ASSERT(AfxIsValidAddress(pNode, sizeof(ptr_node)));		
    pNode->data = newElement; 
}

//=========================================================================

DATAPOS ptr_list::FindIndex(int nIndex) const
{
	if (nIndex >= m_nCount || nIndex < 0)
		return NULL;  // went too far

	ptr_node* pNode = m_pNodeHead;
	while (nIndex--)
	{
		ASSERT(AfxIsValidAddress(pNode, sizeof(ptr_node)));
		pNode = pNode->pNext;
	}
	return (DATAPOS) pNode;
}

//=========================================================================

DATAPOS ptr_list::Find(void* searchValue, DATAPOS startAfter) const
{
	ptr_node* pNode = (ptr_node*) startAfter;
	if (pNode == NULL)
	{
		pNode = m_pNodeHead;  // start at head
	}
	else
	{
		ASSERT(AfxIsValidAddress(pNode, sizeof(ptr_node)));
		pNode = pNode->pNext;  // start after the one specified
	}

	for (; pNode != NULL; pNode = pNode->pNext)
    {
		if (pNode->data == searchValue)
			return (DATAPOS) pNode;
    }
	return NULL;
}


