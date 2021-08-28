#ifndef ptr_list_HPP
#define ptr_list_HPP

//=========================================================================

// abstract iteration position
struct __DATAPOS { };
typedef __DATAPOS* DATAPOS;

//=========================================================================

struct ptr_node_data    
{
	ptr_node_data* pNext;

	void* data() 
    { 
        return this+1; 
    }
	void FreeNodeListMem();       

	static ptr_node_data* __stdcall Create(ptr_node_data*& head, UINT nMax, UINT uElement);
};

//=========================================================================

class ptr_list
{
protected:
	struct ptr_node
	{
		ptr_node* pNext;
		ptr_node* pPrev;
		void* data;
	};

public:
	ptr_list(int nBlockSize = 10);
	~ptr_list();

	int         GetCount() const;
	BOOL        IsEmpty() const;
	void*&      GetHead();
	void*       GetHead() const;
	void*&      GetTail();
	void*       GetTail() const;
	void*       RemoveHead();
	void*       RemoveTail();
	DATAPOS     AddHead(void* newElement);
	DATAPOS     AddTail(void* newElement);
	void        AddHead(ptr_list* pNewList);
	void        AddTail(ptr_list* pNewList);
	void        RemoveAll();

	DATAPOS     GetHeadPosition() const;
	DATAPOS     GetTailPosition() const;
	void*&      GetNext(DATAPOS& rPosition); 
	void*       GetNext(DATAPOS& rPosition) const; 
	void*&      GetPrev(DATAPOS& rPosition); 
	void*       GetPrev(DATAPOS& rPosition) const; 

	void*&      GetAt(DATAPOS position);
	void*       GetAt(DATAPOS position) const;
	void        SetAt(DATAPOS pos, void* newElement);
	void        RemoveAt(DATAPOS position);
	DATAPOS     InsertBefore(DATAPOS position, void* newElement);
	DATAPOS     InsertAfter(DATAPOS position, void* newElement);
	DATAPOS     Find(void* searchValue, DATAPOS startAfter = NULL) const;
	DATAPOS     FindIndex(int nIndex) const;

protected:
//methods
	ptr_node*               NewNode(ptr_node*, ptr_node*);
	void                    FreeNode(ptr_node*);

//member variables
	int                     m_nCount;
	ptr_node*               m_pNodeHead;
	ptr_node*               m_pNodeTail;
	ptr_node*               m_pNodeFree;

	struct ptr_node_data*   m_pBlocks;
	int                     m_nBlockSize;
};

//=========================================================================

#endif