

#include "VertexCache.h"

VertexCache::VertexCache()
{
  VertexCache(16);
}


VertexCache::VertexCache(int size)
{
  numEntries = size;

  entries = new int[numEntries];

  for(int i = 0; i < numEntries; i++)
    entries[i] = -1;
}


VertexCache::~VertexCache()
{
  delete[] entries;
}


int VertexCache::At(int index)
{
  return entries[index];
}


void VertexCache::Set(int index, int value)
{
  entries[index] = value;
}


void VertexCache::Clear()
{
  for(int i = 0; i < numEntries; i++)
    entries[i] = -1;
}

void VertexCache::Copy(VertexCache* inVcache)
{
  for(int i = 0; i < numEntries; i++)
  {
    inVcache->Set(i, entries[i]);
  }
}

  