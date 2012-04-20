/**
 *   Copyright (c) Stephen A. Goss. All rights reserved.
 */

#ifndef PERSISTENTHASHMAP_H
#define PERSISTENTHASHMAP_H

/**
 * A C-based rendition of Rich Hickey's persistent rendition of
 * Phil Bagwell's Hash Array Mapped Trie.
 *
 * This is trimmed down from PersistentHashMap.java in the clojure repo.
 * TransientHashMap, kvreduce, nodeSeq, not available, yet.
 * Garbage collection using Boehm's GC.
 *
 * Node Polymorphism through method vtables with apologies to
 * Axel-Tobias Schreiner for stealing some ideas from the first
 * chapters of Object-Oriented Programming with ANSI-C.
 *
 * Any errors are likely my own, or possibly inherited from Rich Hickey. :)
 */

typedef void (*PHMFunc)(void * key, void * val, void * data);

/* INode */

typedef struct {
  size_t size;
  void * (* ctor) (void * self, va_list * app);
  void * (* assoc) (void * self, int shift, int hash, void * key, void * val,
      void * addedLeaf);
  void * (* without) (void * self, int shift, int hash, void * key);
  void * (* find) (void * self, int shift, int hash, void * key);
  void * (* findDef) (void * self, int shift, int hash, void * key,
      void * notFound);
  void (* foreach) (void * self, PHMFunc f, void * data);
  /* void * (* nodeSeq) (void * self); */
  /* void * (* kvreduce) (void * self, void * f, void * init); */
} INode;

void * INode_new(const void * _inode, ...);
void * INode_assoc(void * self, int shift, int hash, void * key, void * val,
    void * addedLeaf);
void * INode_without(void * self, int shift, int hash, void * key);
void * INode_find(void * self, int shift, int hash, void * key);
void * INode_findDef(void * self, int shift, int hash, void * key,
    void * notFound);
void INode_foreach(void * self, PHMFunc f, void * data);
/* void * INode_nodeSeq(void * self); */
/* void * INode_kvreduce(void * self, void * f, void * init); */
void * INode_createNode(int shift, void * key1, void * val1, int key2hash,
    void * key2, void * val2);

void init_INodes(void * equiv, void * hash);

/* Array */

void * Array_new(int size);
int Array_size(void * array);
void * Array_set(void * array, int i, void * a);
void * Array_get(void * array, int i);
void * Array_cloneAndSet(void * array, int i, void * a);
void * Array_cloneAndSet2(void * array, int i, void * a, int j, void * b);
void Array_copy(void * src, int srcPos, void * dest, int destPos, int length);
void * Array_removePair(void * array, int i);

/* Box */

void * Box_new(void * val);
void * Box_set(void * box, void * val);
void * Box_get(void * box);

/* MapEntry */

void * MapEntry_new(void * key, void * val);
void * MapEntry_key(void * self);
void * MapEntry_val(void * self);
void * MapEntry_set(void * self, void * val);

/* PersistentHashMap */

typedef struct {
  int count;
  void * root; // INode
  int hasNull;
  void * nullValue; // Object
} PersistentHashMap;

void * PersistentHashMap_EMPTY;
void * PersistentHashMap_NOT_FOUND;

PersistentHashMap * PersistentHashMap_new(int count, void * root, int HasNull,
    void * nullValue);
int PersistentHashMap_containsKey(PersistentHashMap * phm, void * key);
// returns MapEntry
void * PersistentHashMap_entryAt(PersistentHashMap * phm, void * key);
PersistentHashMap * PersistentHashMap_assoc(PersistentHashMap * phm,
    void * key, void * val);
void * PersistentHashMap_valAtDef(PersistentHashMap * phm, void * key,
    void * notFound);
void * PersistentHashMap_valAt(PersistentHashMap * phm, void * key);
PersistentHashMap * PersistentHashMap_without(PersistentHashMap * phm,
    void * key);
int PersistentHashMap_count(PersistentHashMap * phm);
void PersistentHashMap_foreach(PersistentHashMap * phm, PHMFunc f, void * data);

/* ArrayNode */

typedef struct {
  const INode * inode;
  int count;
  void * array; // INode[]
} ArrayNode;

INode * ArrayNode_class;

void * ArrayNode_ctor(void * self, va_list * app);
void * ArrayNode_assoc(void * self, int shift, int hash, void * key, void * val,
    void * addedLeaf);
void * ArrayNode_without(void * self, int shift, int hash, void * key);
void * ArrayNode_find(void * self, int shift, int hash, void * key);
void * ArrayNode_findDef(void * self, int shift, int hash, void * key,
    void * notFound);
void ArrayNode_foreach(void * self, PHMFunc f, void * data);
/* void * ArrayNode_nodeSeq(void * self); */
/* void * ArrayNode_kvreduce(void * self, void * f, void * init); */
//private
void * ArrayNode_editAndSet(ArrayNode *, int i, void * n);
void * ArrayNode_pack(ArrayNode *, int idx);

/* BitmapIndexedNode */

typedef struct {
  const INode * inode;
  int bitmap;
  void * array; // Object[]
} BitmapIndexedNode;

INode * BitmapIndexedNode_class;
void * BitmapIndexedNode_EMPTY;

void * BitmapIndexedNode_ctor(void * self, va_list * app);
void * BitmapIndexedNode_assoc(void * self, int shift, int hash, void * key, void * val,
    void * addedLeaf);
void * BitmapIndexedNode_without(void * self, int shift, int hash, void * key);
void * BitmapIndexedNode_find(void * self, int shift, int hash, void * key);
void * BitmapIndexedNode_findDef(void * self, int shift, int hash, void * key,
    void * notFound);
void BitmapIndexedNode_foreach(void * self, PHMFunc f, void * data);
/* void * BitmapIndexedNode_nodeSeq(void * self); */
/* void * BitmapIndexedNode_kvreduce(void * self, void * f, void * init); */

void * BitmapIndexedNode_editAndSet(BitmapIndexedNode * bin, int i, void * a);
void * BitmapIndexedNode_editAndSet2(BitmapIndexedNode * bin, int i, void * a,
    int j, void * b);
void * BitmapIndexedNode_editAndRemovePair(BitmapIndexedNode * bin,
    int bit, int i);
int BitmapIndexedNode_index(BitmapIndexedNode * bin, int bit);

/* HashCollisionNode */

typedef struct {
  const INode * inode;
  int hash;
  int count;
  void * array; // Object[]
} HashCollisionNode;

INode * HashCollisionNode_class;

void * HashCollisionNode_ctor(void * self, va_list * app);
void * HashCollisionNode_assoc(void * self, int shift, int hash, void * key,
    void * val, void * addedLeaf);
void * HashCollisionNode_without(void * self, int shift, int hash, void * key);
void * HashCollisionNode_find(void * self, int shift, int hash, void * key);
void * HashCollisionNode_findDef(void * self, int shift, int hash, void * key,
    void * notFound);
void HashCollisionNode_foreach(void * self, PHMFunc f, void * data);
/* void * HashCollisionNode_nodeSeq(void * self); */
/* void * HashCollisionNode_kvreduce(void * self, void * f, void * init); */

int HashCollisionNode_findIndex(HashCollisionNode * hcn, void * key);

/* Other */

int (* equiv_func) (void * o1, void * o2);
int (* hash_func) (void * p);

int mask(int hash, int shift);
int bitpos(int hash, int shift);
int equiv_default(void * o1, void * o2);
int hash_default(void * p);

#endif /* PERSISTENTHASHMAP_H */
