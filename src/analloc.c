#include "analloc.h"

static uint64_t _analloc_log_page(analloc_t alloc, uint64_t size);
static uint8_t _analloc_grab_first(analloc_t alloc, uint64_t size);

uint8_t analloc_with_chunk(analloc_t alloc,
                           void * ptr,
                           uint64_t total,
                           uint64_t used,
                           uint64_t page) {
  if (total < page) return 0;
  // calculate the actual size of memory to use
  uint64_t depth = 0;
  while (page << depth <= total) depth++;
  depth--;
  
  // make sure they haven't used too much memory already
  uint64_t size = page << depth;
  if (size < used) return 0;
  
  uint64_t treeSize = anbtree_size(depth);
  uint64_t realUsed = used + treeSize;
  
  // setup the allocator
  alloc->mem = ptr;
  alloc->tree = ((uint8_t *)alloc->mem) + used;
  alloc->page = page;
  alloc->depth = depth;
  
  // initialize the tree and allocate our used data
  anbtree_initialize(alloc->tree, treeSize);
  return _analloc_grab_first(alloc, realUsed);
}

void * analloc_alloc(analloc_t alloc, uint64_t * sizeInOut, uint8_t high) {
  uint64_t twoPower = _analloc_log_page(alloc, *sizeInOut);
  if (twoPower > alloc->depth) {
    (*sizeInOut) = 0;
    return (void *)0;
  }
  
  uint64_t depth = alloc->depth - twoPower;
  anbtree_path path;
  if (high) path = anbtree_high_path_to_leaf(alloc->tree, depth);
  else path = anbtree_path_to_leaf(alloc->tree, depth);
  
  if (path == anbtree_path_none) {
    (*sizeInOut) = 0;
    return (void *)0;
  }
  
  // split down our `depth` until we have the actual buddy we want.
  uint64_t baseDepth = anbtree_path_depth(path);
  while (baseDepth < depth) {
    anbtree_alloc_node(alloc->tree, path);
    baseDepth++;
    if (high) { 
      anbtree_free_node(alloc->tree, anbtree_path_left(path));
      path = anbtree_path_right(path);
    } else {
      anbtree_free_node(alloc->tree, anbtree_path_right(path));
      path = anbtree_path_left(path);
    } 
  }
  
  // allocate the base node and set it as a data node.
  anbtree_alloc_node(alloc->tree, path);
  if (depth < alloc->depth) {
    anbtree_free_node(alloc->tree, anbtree_path_left(path));
    anbtree_free_node(alloc->tree, anbtree_path_right(path));
  }
  
  uint64_t index = anbtree_path_local_index(path);
  uint64_t finalSize = alloc->page << twoPower;
  (*sizeInOut) = finalSize;
  return (void *)((index * finalSize) + (uint64_t)alloc->mem);
}

void analloc_free(analloc_t alloc, void * buffer, uint64_t length) {
  uint64_t twoPower = _analloc_log_page(alloc, length);
  uint64_t depth = alloc->depth - twoPower;
  
  uint64_t index = (uint64_t)buffer - (uint64_t)alloc->mem;
  index /= alloc->page << twoPower;
  
  anbtree_path path = anbtree_path_from_info(depth, index);
  
  // keep freeing paths until we can return
  while (path != anbtree_path_none) {
    anbtree_free_node(alloc->tree, path);
    anbtree_path parent = anbtree_path_parent(path);
    if (parent == anbtree_path_none) break;
    
    // if both sibling and us are now free, the parent node can be free'd.
    anbtree_path sibling = anbtree_path_sibling(path);
    if (anbtree_is_allocated(alloc->tree, sibling)) break;
    path = parent;
  }
}

/***********
 * Realloc *
 ***********/

void * analloc_realloc(analloc_t alloc,
                       void * buffer,
                       uint64_t length,
                       uint64_t * newLen) {
  (*newLen) = 0;
  return (void *)0;
}

uint8_t analloc_realloc_local(analloc_t alloc,
                              void * buffer,
                              uint64_t length,
                              uint64_t * newLen) {
  (*newLen) = 0;
  return 0;
}

/***********
 * Private *
 ***********/

static uint64_t _analloc_log_page(analloc_t alloc, uint64_t size) {
  uint64_t result = alloc->page;
  uint64_t power = 0;
  while (result < size) {
    result <<= 1L;
    power += 1;
  }
  return power;
}

static uint8_t _analloc_grab_first(analloc_t alloc, uint64_t size) {
  uint64_t pageExp = _analloc_log_page(alloc, size);
  uint64_t sizeOut = size;
  
  if (pageExp == 0) {
    analloc_alloc(alloc, &sizeOut, 0);
    return sizeOut >= size;
  } else if (alloc->page << pageExp == size) {
    analloc_alloc(alloc, &sizeOut, 0);
    return sizeOut == size;
  } else {
    uint64_t newSize = alloc->page << (pageExp - 1);
    sizeOut = newSize;
    analloc_alloc(alloc, &sizeOut, 0);
    
    if (sizeOut != newSize) return 0;
    return _analloc_grab_first(alloc, size - newSize);
  }
}
