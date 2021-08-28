
#ifndef VERTEX_CACHE_H

#define VERTEX_CACHE_H

class VertexCache
{

public:

  VertexCache(int size);
  VertexCache();
  ~VertexCache();

  bool InCache(int entry);
  int AddEntry(int entry);
  void Clear();

  void Copy(VertexCache* inVcache);
  int At(int index);
  void Set(int index, int value);

private:

  int *entries;
  int numEntries;

};



inline bool VertexCache::InCache(int entry)
{
  bool returnVal = false;

  for(int i = 0; i < numEntries; i++)
  {
    if(entries[i] == entry)
    {
      returnVal = true;
      break;
    }
  }

  return returnVal;
}


inline int VertexCache::AddEntry(int entry)
{
  int removed;

  removed = entries[numEntries - 1];

  //push everything right one
  for(int i = numEntries - 2; i >= 0; i--)
  {
    entries[i + 1] = entries[i];
  }

  entries[0] = entry;

  return removed;
}


#endif