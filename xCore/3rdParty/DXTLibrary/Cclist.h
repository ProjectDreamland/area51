/*

	ccList Class

*/


#ifndef CCLIST_H
#define CCLIST_H


#ifndef BOOL
#define BOOL int
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

class ccMinNode;
class ccMinList;
class ccList;
class ccNode;
class ccHashNode;

#define LIST_MAGIC 0xABadCafe


class ccMinNode
{
protected:
	ccMinNode	*next, *prev;

public:
	ccMinNode();
	ccMinNode(const ccMinNode&);
	virtual ~ccMinNode();

	inline ccMinNode *GetNext(void) const { return next; }
	inline ccMinNode *GetPrev(void) const { return prev; }

	friend class ccMinList;
};


class ccNode : public ccMinNode
{
protected:
	char *name;
	unsigned long nameHash;

public:
	ccNode();
	virtual ~ccNode();
	BOOL SetName(const char *theName);

	inline const char *GetName(void) const { return(name); }
	inline ccNode *GetNext(void) const { return (ccNode *)ccMinNode::GetNext(); }
	inline ccNode *GetPrev(void) const { return (ccNode *)ccMinNode::GetPrev(); }

	friend class ccList;
};




//*********************************************************************

typedef BOOL (*NodeCheckFlipFunc)(ccMinNode *n, ccMinNode *nn);


class ccMinList
{
private:
	unsigned long numElements;

protected:
	ccMinNode	*head, *tail;

public:
	ccMinList();
	virtual ~ccMinList();

	BOOL	IsInList(ccMinNode *n) const;

	ccMinNode *RemHead(void);
	ccMinNode *RemTail(void);
	ccMinNode *RemNode(ccMinNode *n);

	long ElementNumber(ccMinNode *node);

	inline ccMinNode *GetHead(void) const { return head; }
	inline ccMinNode *GetTail(void) const { return tail; }
	inline void AddHead(ccMinNode *n) { AddNode(0,n);}

	inline void AddTail(ccMinNode *n) { AddNode(tail,n);}
	void AddNode(ccMinNode *insertPoint, ccMinNode *n);
	ccMinNode *FindNode(unsigned long ordinalnumber) const;

	void Purge(void);

	inline BOOL IsListEmpty() const {return(head ? 0 : 1);}
	inline unsigned long GetNumElements() const { return(numElements); }

	void Sort ( NodeCheckFlipFunc CheckFlip );
};

class ccList : public ccMinList
{
public:
	ccList();
	virtual ~ccList();
	ccNode *FindNode(const char *name) const;
	ccNode *FindNode(const char *name, ccNode *node) const;
	inline ccNode *FindNode(unsigned long ordinalnumber) const
		{ return (ccNode *)ccMinList::FindNode(ordinalnumber); }

	void SortAlpha();

	inline ccNode *GetHead(void) const { return (ccNode *)ccMinList::GetHead(); }
	inline ccNode *GetTail(void) const { return (ccNode *)ccMinList::GetTail(); }
	inline ccNode *RemHead(void) { return (ccNode *)ccMinList::RemHead(); }
	inline ccNode *RemTail(void) { return (ccNode *)ccMinList::RemTail(); }
	inline ccNode *RemNode(ccMinNode *n)  { return(ccNode *)ccMinList::RemNode(n); }
};


//-------------------------------------------------------------------
// Hash values are maintained for faster searches
//-------------------------------------------------------------------
unsigned long CalcHash(const char *String);

class ccHashNodePtr;
class ccHashNode;
class ccHashList;

class ccHashNodePtr : public ccMinNode
{
private:
	ccHashNode	*NodePtr;

public:
	inline ccHashNodePtr(ccHashNode *PointTo) {NodePtr = PointTo;}
	inline ccHashNode *GetNodePtr(void) {return NodePtr;}

	friend class ccHashList;
};

class ccHashNode : public ccMinNode
{
private:
	char	*Name;

	unsigned long	Hash;
	ccHashNodePtr	*myPtr;


protected:
public:
	ccHashNode();
	ccHashNode( ccHashNode& hashnode );
	virtual ~ccHashNode();

	BOOL SetName(const char *newName);

	inline const char *GetName(void) const { return Name; }

	inline ccHashNode *GetNext(void) const { return (ccHashNode *)ccMinNode::GetNext(); }
	inline ccHashNode *GetPrev(void) const { return (ccHashNode *)ccMinNode::GetPrev(); }

	friend class ccHashList;
};


class ccHashList
{
private:
	long		TableSize;
	ccMinList	*HashTable;
	ccMinList	NodeList;

	void AddHashEntry(ccHashNode *n);
	void RemHashEntry(ccHashNode *n);

protected:
public:

	ccHashList();
	ccHashList(long NewTableSize);
	virtual ~ccHashList();

	void SetTableSize(long NewTableSize);
	inline long GetTableSize() const {return TableSize;}

	void Purge(void);

	void AddHead(ccHashNode *n);
	void AddTail(ccHashNode *n);
	void AddNode(ccHashNode *insertPoint, ccHashNode *n);

	void RemNode(ccHashNode *n);

	ccHashNode *Find(const char *FindName);
	void Rename(ccHashNode *n, const char *NewName);
	inline unsigned long GetNumElements(void) const {return NodeList.GetNumElements();}

	inline ccHashNode *GetHead(void) const { return (ccHashNode *)NodeList.GetHead(); }
	inline ccHashNode *GetTail(void) const { return (ccHashNode *)NodeList.GetTail(); }
};

#endif
