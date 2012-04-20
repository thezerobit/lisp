/**
 *   Copyright (c) Stephen A. Goss. All rights reserved.
 */

#include <gc.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

#include "persistenthashmap.h"

/* INode */

void * INode_new(const void * _inode, ...) {
  const INode * inode = _inode;
  void * p = GC_MALLOC(inode->size);
  assert(p);
  * (const INode **) p = inode;
  if(inode->ctor) {
    va_list ap;
    va_start(ap, _inode);
    p = inode->ctor(p, &ap);
    va_end(ap);
  }
  return p;
}

void * INode_assoc(void * self, int shift, int hash, void * key, void * val,
    void * addedLeaf) {
  const INode * const * cp = self;
  assert(self && * cp && (*cp)->assoc);
  return (*cp)->assoc(self, shift, hash, key, val, addedLeaf);
}

void * INode_without(void * self, int shift, int hash, void * key) {
  const INode * const * cp = self;
  assert(self && * cp && (*cp)->without);
  return (*cp)->without(self, shift, hash, key);
}

void * INode_find(void * self, int shift, int hash, void * key) {
  const INode * const * cp = self;
  assert(self && * cp && (*cp)->find);
  return (*cp)->find(self, shift, hash, key);
}

void * INode_findDef(void * self, int shift, int hash, void * key,
    void * notFound) {
  const INode * const * cp = self;
  assert(self && * cp && (*cp)->findDef);
  return (*cp)->findDef(self, shift, hash, key, notFound);
}

void INode_foreach(void * self, PHMFunc f, void * data) {
  const INode * const * cp = self;
  assert(self && * cp && (*cp)->foreach);
  (*cp)->foreach(self, f, data);
}

/* void * INode_nodeSeq(void * self) { */
/*   const INode * const * cp = self; */
/*   assert(self && * cp && (*cp)->nodeSeq); */
/*   return (*cp)->nodeSeq(self); */
/* } */

/* void * INode_kvreduce(void * self, void * f, void * init) { */
/*   const INode * const * cp = self; */
/*   assert(self && * cp && (*cp)->kvreduce); */
/*   return (*cp)->kvreduce(self, f, init); */
/* } */

void * INode_createNode(int shift, void * key1, void * val1, int key2hash,
    void * key2, void * val2) {
  int key1hash = hash_func(key1);
  if(key1hash == key2hash) {
    void * temp_array = Array_new(4);
    Array_set(temp_array, 0, key1);
    Array_set(temp_array, 1, val1);
    Array_set(temp_array, 2, key2);
    Array_set(temp_array, 3, val2);
    return INode_new(HashCollisionNode_class, key1hash, 2, temp_array);
  }
  void * _ = Box_new(NULL);
  void * temp_node = INode_assoc(BitmapIndexedNode_EMPTY, shift, key1hash, key1, val1, _);
  return INode_assoc(temp_node, shift, key2hash, key2, val2, _);
}

void init_INodes(void * equiv, void * hash) {
  if(equiv == NULL) {
    equiv_func = equiv_default;
  } else {
    equiv_func = equiv;
  }
  if(hash == NULL) {
    hash_func = hash_default;
  } else {
    hash_func = hash;
  }
  ArrayNode_class = (INode *)GC_MALLOC(sizeof(INode));
  ArrayNode_class->size = sizeof(ArrayNode);
  ArrayNode_class->ctor = ArrayNode_ctor;
  ArrayNode_class->assoc = ArrayNode_assoc;
  ArrayNode_class->without = ArrayNode_without;
  ArrayNode_class->find = ArrayNode_find;
  ArrayNode_class->findDef = ArrayNode_findDef;
  ArrayNode_class->foreach = ArrayNode_foreach;

  BitmapIndexedNode_class = (INode *)GC_MALLOC(sizeof(INode));
  BitmapIndexedNode_class->size = sizeof(BitmapIndexedNode);
  BitmapIndexedNode_class->ctor = BitmapIndexedNode_ctor;
  BitmapIndexedNode_EMPTY = INode_new(BitmapIndexedNode_class, NULL, 0,
      Array_new(0));
  BitmapIndexedNode_class->assoc = BitmapIndexedNode_assoc;
  BitmapIndexedNode_class->without = BitmapIndexedNode_without;
  BitmapIndexedNode_class->find = BitmapIndexedNode_find;
  BitmapIndexedNode_class->findDef = BitmapIndexedNode_findDef;
  BitmapIndexedNode_class->foreach = BitmapIndexedNode_foreach;

  HashCollisionNode_class = (INode *)GC_MALLOC(sizeof(INode));
  HashCollisionNode_class->size = sizeof(HashCollisionNode);
  HashCollisionNode_class->ctor = HashCollisionNode_ctor;
  HashCollisionNode_class->assoc = HashCollisionNode_assoc;
  HashCollisionNode_class->without = HashCollisionNode_without;
  HashCollisionNode_class->find = HashCollisionNode_find;
  HashCollisionNode_class->findDef = HashCollisionNode_findDef;
  HashCollisionNode_class->foreach = HashCollisionNode_foreach;

  PersistentHashMap_EMPTY = PersistentHashMap_new(0, NULL, 0, NULL);
  PersistentHashMap_NOT_FOUND = GC_MALLOC(sizeof(void *));
}

/* Array */

void * Array_new(int size) {
  void * array = (void *)GC_MALLOC(sizeof(void *) * (size + 1));
  * (int *) array = size;
  return array;
}

int Array_size(void * array) {
  return ((int *)array)[0];
}

void * Array_set(void * array, int i, void * a) {
  int size = Array_size(array);
  assert(i < size);
  ((void **)array)[i + 1] = a;
  return array;
}

void * Array_get(void * array, int i) {
  int size = Array_size(array);
  assert(i < size);
  return ((void **)array)[i + 1];
}

void * Array_cloneAndSet(void * array, int i, void * a) {
  int size = Array_size(array);
  void * new_array = Array_new(size);
  memcpy(new_array, array, sizeof(void *) * (size + 1));
  return Array_set(new_array, i, a);
}

void * Array_cloneAndSet2(void * array, int i, void * a, int j, void * b) {
  int size = Array_size(array);
  void * new_array = Array_new(size);
  memcpy(new_array, array, sizeof(void *) * (size + 1));
  Array_set(new_array, i, a);
  return Array_set(new_array, j, b);
}

void * Array_removePair(void * array, int i) {
  int new_array_size = Array_size(array) - 2;
  void  * newArray = Array_new(new_array_size);
  Array_copy(array, 0, newArray, 0, 2 * i);
  Array_copy(array, 2 * (i + 1), newArray, 2 * i, new_array_size - 2 * i);
  return newArray;
}

void Array_copy(void * src, int srcPos, void * dest, int destPos,
    int length) {
  void * from = src + (srcPos + 1) * sizeof(void *);
  void * to = dest + (destPos + 1) * sizeof(void *);
  size_t bytes = length * sizeof(void *);
  memcpy(to, from, bytes);
}

/* Box */

void * Box_new(void * val) {
  void ** new_box = (void **)GC_MALLOC(sizeof(void *));
  *(new_box) = val;
  return new_box;
}

void * Box_set(void * box, void * val) {
  *((void **)box) = val;
  return val;
}

void * Box_get(void * box) {
  return *((void **)box);
}

/* MapEntry */

void * MapEntry_new(void * key, void * val) {
  void ** new_map = (void **)GC_MALLOC(sizeof(void *) * 2);
  *(new_map) = key;
  *(new_map + sizeof(void *)) = val;
  return new_map;
}

void * MapEntry_key(void * self) {
  return *((void **)self);
}

void * MapEntry_val(void * self) {
  return *((void **)(self + sizeof(void *)));
}

void * MapEntry_set(void * self, void * val) {
  *((void **)(self + sizeof(void *))) = val;
  return val;
}

/* PersistentHashMap */

PersistentHashMap * PersistentHashMap_new(int count, void * root, int hasNull,
    void * nullValue) {
  PersistentHashMap * new_phm = (PersistentHashMap *)GC_MALLOC(sizeof(
        PersistentHashMap));
  new_phm->count = count;
  new_phm->root = root;
  new_phm->hasNull = hasNull;
  new_phm->nullValue = nullValue;
}

int PersistentHashMap_containsKey(PersistentHashMap * phm, void * key) {
  if(key == NULL) {
    return phm->hasNull;
  }
  if(phm->root == NULL) {
    return 0;
  }
  return INode_findDef(phm->root, 0, hash_func(key), key,
      PersistentHashMap_NOT_FOUND) != PersistentHashMap_NOT_FOUND;
}

// returns MapEntry
void * PersistentHashMap_entryAt(PersistentHashMap * phm, void * key) {
  if(key == NULL) {
    return phm->hasNull ? MapEntry_new(NULL, phm->nullValue) : NULL;
  }
  if(phm->root == NULL) {
    return 0;
  }
  return INode_find(phm->root, 0, hash_func(key), key);
}

PersistentHashMap * PersistentHashMap_assoc(PersistentHashMap * phm,
    void * key, void * val) {
  if(key == NULL) {
    if(phm->hasNull && val == phm->nullValue) {
      return phm;
    }
    return PersistentHashMap_new(phm->hasNull ? phm->count : phm->count + 1,
        phm->root, 1, val);
  }
  void * addedLeaf = Box_new(NULL);
  void * temp_node = (phm->root == NULL ? BitmapIndexedNode_EMPTY : phm->root);
  void * newroot = INode_assoc(temp_node, 0, hash_func(key), key, val,
      addedLeaf);
  if(newroot == phm->root) {
    return phm;
  }
  return PersistentHashMap_new(
      Box_get(addedLeaf) == NULL ? phm->count : phm->count + 1,
      newroot, phm->hasNull, phm->nullValue);
}

void * PersistentHashMap_valAtDef(PersistentHashMap * phm, void * key,
    void * notFound) {
  if(key == NULL) {
    return phm->hasNull ? phm->nullValue : notFound;
  }
  if(phm->root == NULL) {
    return notFound;
  }
  return INode_findDef(phm->root, 0, hash_func(key), key, notFound);
}

void * PersistentHashMap_valAt(PersistentHashMap * phm, void * key) {
  return PersistentHashMap_valAtDef(phm, key, NULL);
}

PersistentHashMap * PersistentHashMap_without(PersistentHashMap * phm,
    void * key) {
  if(key == NULL) {
    if(!phm->hasNull) {
      return phm;
    }
    return PersistentHashMap_new(phm->count - 1, phm->root, 0, NULL);
  }
  if(phm->root == NULL) {
    return phm;
  }
  void * newroot = INode_without(phm->root, 0, hash_func(key), key);
  if(newroot == phm->root) {
    return phm;
  }
  return PersistentHashMap_new(phm->count - 1, newroot, phm->hasNull,
      phm->nullValue);
}

int PersistentHashMap_count(PersistentHashMap * phm) {
  return phm->count;
}

void PersistentHashMap_foreach(PersistentHashMap * phm, PHMFunc f, void * data) {
  if(phm->hasNull) {
    f(NULL, phm->nullValue, data);
  }
  if(phm->root != NULL) {
    INode_foreach(phm->root, f, data);
  }
}

/* ArrayNode */

void * ArrayNode_ctor(void * self, va_list * app) {
  ((ArrayNode *)self)->count = va_arg((*app), int);
  ((ArrayNode *)self)->array = va_arg((*app), void *);
}

void * ArrayNode_assoc(void * self, int shift, int hash, void * key, void * val,
    void * addedLeaf) {
  ArrayNode * an = (ArrayNode *)self;
  int idx = mask(hash, shift);
  void * node = Array_get(an->array, idx);
  if(node == NULL) {
    return INode_new(ArrayNode_class, NULL, an->count + 1,
        Array_cloneAndSet(an->array, idx, INode_new(
            INode_assoc(BitmapIndexedNode_EMPTY, shift + 5, hash, key, val,
              addedLeaf))));
  }
  void * n = INode_assoc(node, shift + 5, hash, key, val, addedLeaf);
  if(n == node) {
    return an;
  }
  return INode_new(ArrayNode_class, NULL, an->count,
      Array_cloneAndSet(an->array, idx, n));
}

void * ArrayNode_without(void * self, int shift, int hash, void * key) {
  ArrayNode * an = (ArrayNode *)self;
  int idx = mask(hash, shift);
  void * node = Array_get(an->array, idx);
  if(node == NULL) {
    return self;
  }
  void * n = INode_without(node, shift + 5, hash, key);
  if(n == node) {
    return self;
  }
  if(n == NULL) {
    if(an->count <= 8) {
      return ArrayNode_pack(an, idx);
    }
    return INode_new(ArrayNode_class, an->count - 1,
        Array_cloneAndSet(an->array, idx, n));
  } else {
    return INode_new(ArrayNode_class, an->count,
        Array_cloneAndSet(an->array, idx, n));
  }
}

void * ArrayNode_find(void * self, int shift, int hash, void * key) {
  ArrayNode * an = (ArrayNode *)self;
  int idx = mask(hash, shift);
  void * node = Array_get(an->array, idx);
  if(node == NULL) {
    return NULL;
  }
  return INode_find(node, shift + 5, hash, key);
}

void * ArrayNode_findDef(void * self, int shift, int hash, void * key,
    void * notFound) {
  ArrayNode * an = (ArrayNode *)self;
  int idx = mask(hash, shift);
  void * node = Array_get(an->array, idx);
  if(node == NULL) {
    return notFound;
  }
  return INode_findDef(node, shift + 5, hash, key, notFound);
}

void ArrayNode_foreach(void * self, PHMFunc f, void * data) {
  ArrayNode * an = (ArrayNode *)self;
  int i;
  void * n;
  int count = an->count;
  for(i = 0; i < count; i++) {
    n = Array_get(an->array, i);
    if(n != NULL) {
      INode_foreach(n, f, data);
    }
  }
}

/* void * ArrayNode_nodeSeq(void * self); */

/* void * ArrayNode_kvreduce(void * self, void * f, void * init) { */
/*   ArrayNode * an = (ArrayNode *)self; */
/*   int array_size = Array_size(an->array); */
/*   void * node; */
/*   int i; */
/*   for(i = 0; i < array_size; i++) { */
/*     node = Array_get(an->array, i); */
/*     init = INode_kvreduce(node, f, init); */
/*   } */
/*   return init; */
/* } */

//private

void * ArrayNode_editAndSet(ArrayNode * an, int i, void * n) {
  Array_set(an->array, i, n);
  return an;
}

void * ArrayNode_pack(ArrayNode * an, int idx) {
  void * newArray = Array_new(2*(an->count - 1));
  int j = 1;
  int bitmap = 0;
  int i;
  for(i = 0; i < idx; i++) {
    if(Array_get(an->array, i) != NULL) {
      Array_set(newArray, j, Array_get(an->array, i));
      bitmap |= (1 << i);
      j += 2;
    }
  }
  int array_length = Array_size(an->array);
  for(i = idx + 1; i < array_length; i++) {
    if(Array_get(an->array, i) != NULL) {
      Array_set(newArray, j, Array_get(an->array, i));
      bitmap |= (1 << i);
      j += 2;
    }
  }
  return INode_new(BitmapIndexedNode_class, bitmap, newArray);
}

/* BitmapIndexedNode */

void * BitmapIndexedNode_ctor(void * self, va_list * app) {
  ((BitmapIndexedNode *)self)->bitmap = va_arg((*app), int);
  ((BitmapIndexedNode *)self)->array = va_arg((*app), void *);
}

void * BitmapIndexedNode_assoc(void * self, int shift, int hash, void * key,
    void * val, void * addedLeaf) {
  BitmapIndexedNode * bin = (BitmapIndexedNode *)self;
  int bit = bitpos(hash, shift);
  int idx = BitmapIndexedNode_index(self, bit);
  if((bin->bitmap & bit) != 0) {
    void * keyOrNull = Array_get(bin->array, 2 * idx);
    void * valOrNode = Array_get(bin->array, 2 * idx + 1);
    if(keyOrNull == NULL) {
      void * n = INode_assoc(valOrNode, shift + 5, hash, key, val, addedLeaf);
      if(n == valOrNode) {
        return self;
      }
      return INode_new(BitmapIndexedNode_class, bin->bitmap,
          Array_cloneAndSet(bin->array, 2 * idx + 1, n));
    }
    if(equiv_func(key, keyOrNull)) {
      if(val == valOrNode) {
        return self;
      }
      return INode_new(BitmapIndexedNode_class, bin->bitmap,
          Array_cloneAndSet(bin->array, 2 * idx + 1, val));
    }
    Box_set(addedLeaf, addedLeaf);
    return INode_new(BitmapIndexedNode_class, bin->bitmap,
        Array_cloneAndSet2(bin->array, 2 * idx, NULL, 2 * idx + 1,
            INode_createNode(shift + 5, keyOrNull, valOrNode, hash,
                key, val)));
  } else {
    int n = __builtin_popcount((unsigned int)(bin->bitmap));
    if(n >= 16) {
      void * nodes = Array_new(32);
      int jdx = mask(hash, shift);
      Array_set(nodes, jdx, INode_assoc(BitmapIndexedNode_EMPTY,
            shift + 5, hash, key, val, addedLeaf));
      int j = 0;
      int i;
      for(i = 0; i < 32; i++) {
        if(((((unsigned int)bin->bitmap) >> i) & 1) != 0) {
          if(Array_get(bin->array, j) == NULL) {
            Array_set(nodes, i, Array_get(bin->array, j+1));
          } else {
            Array_set(nodes, i, INode_assoc(BitmapIndexedNode_EMPTY,
                  shift + 5, hash_func(Array_get(bin->array, j)),
                  Array_get(bin->array, j), Array_get(bin->array, j+1),
                  addedLeaf));
          }
          j += 2;
        }
      }
      return INode_new(ArrayNode_class, n + 1, nodes);
    } else {
      void * newArray = Array_new(2 * (n + 1));
      Array_copy(bin->array, 0, newArray, 0, 2 * idx);
      Array_set(newArray, 2 * idx, key);
      Box_set(addedLeaf, addedLeaf);
      Array_set(newArray, 2 * idx + 1, val);
      Array_copy(bin->array, 2 * idx, newArray, 2 * (idx + 1), 2 * (n - idx));
      return INode_new(BitmapIndexedNode_class, bin->bitmap | bit, newArray);
    }
  }
}

void * BitmapIndexedNode_without(void * self, int shift, int hash,
    void * key) {
  BitmapIndexedNode * bin = (BitmapIndexedNode *)self;
  int bit = bitpos(hash, shift);
  if((bin->bitmap & bit) == 0) {
    return self;
  }
  int idx = BitmapIndexedNode_index(bin, bit);
  void * keyOrNull = Array_get(bin->array, 2 * idx);
  void * valOrNode = Array_get(bin->array, 2 * idx + 1);
  if(keyOrNull == NULL) {
    void * n = INode_without(valOrNode, shift + 5, hash, key);
    if(n == valOrNode) {
      return self;
    }
    if(n != NULL) {
      return INode_new(BitmapIndexedNode_class, bin->bitmap,
          Array_cloneAndSet(bin->array, 2 * idx + 1, n));
    }
    if(bin->bitmap == bit) {
      return NULL;
    }
    return INode_new(BitmapIndexedNode_class, bin->bitmap ^ bit,
        Array_removePair(bin->array, idx));
  }
  if(equiv_func(key, keyOrNull)) {
    return INode_new(BitmapIndexedNode_class, bin->bitmap ^ bit,
        Array_removePair(bin->array, idx));
  }
  return self;
}

void * BitmapIndexedNode_find(void * self, int shift, int hash, void * key) {
  BitmapIndexedNode * bin = (BitmapIndexedNode *)self;
  int bit = bitpos(hash, shift);
  if((bin->bitmap & bit) == 0) {
    return NULL;
  }
  int idx = BitmapIndexedNode_index(bin, bit);
  void * keyOrNull = Array_get(bin->array, 2 * idx);
  void * valOrNode = Array_get(bin->array, 2 * idx + 1);
  if(keyOrNull == NULL) {
    return INode_find(valOrNode, shift + 5, hash, key);
  }
  if(equiv_func(key, keyOrNull)) {
    return MapEntry_new(keyOrNull, valOrNode);
  }
  return NULL;
}

void * BitmapIndexedNode_findDef(void * self, int shift, int hash, void * key,
    void * notFound) {
  BitmapIndexedNode * bin = (BitmapIndexedNode *)self;
  int bit = bitpos(hash, shift);
  if((bin->bitmap & bit) == 0) {
    return notFound;
  }
  int idx = BitmapIndexedNode_index(bin, bit);
  void * keyOrNull = Array_get(bin->array, 2 * idx);
  void * valOrNode = Array_get(bin->array, 2 * idx + 1);
  if(keyOrNull == NULL) {
    return INode_findDef(valOrNode, shift + 5, hash, key, notFound);
  }
  if(equiv_func(key, keyOrNull)) {
    return valOrNode;
  }
  return notFound;
}

void BitmapIndexedNode_foreach(void * self, PHMFunc f, void * data) {
  BitmapIndexedNode * bin = (BitmapIndexedNode *)self;
  int i;
  int count = Array_size(bin->array);
  void * key;
  for(i = 0; i < count; i += 2) {
    key = Array_get(bin->array, i);
    if(key != NULL) {
      void * value = Array_get(bin->array, i + 1);
      f(key, value, data);
    } else {
      void * node = Array_get(bin->array, i + 1);
      if(node != NULL) {
        INode_foreach(node, f, data);
      }
    }
  }
}

/* void * BitmapIndexedNode_nodeSeq(void * self); */
/* void * BitmapIndexedNode_kvreduce(void * self, void * f, void * init); */

void * BitmapIndexedNode_editAndSet(BitmapIndexedNode * bin, int i, void * a) {
  Array_set(bin->array, i, a);
  return bin;
}

void * BitmapIndexedNode_editAndSet2(BitmapIndexedNode * bin, int i, void * a,
    int j, void * b) {
  Array_set(bin->array, i, a);
  Array_set(bin->array, j, b);
  return bin;
}

void * BitmapIndexedNode_editAndRemovePair(BitmapIndexedNode * bin,
    int bit, int i) {
  if(bin->bitmap == bit) {
    return NULL;
  }
}

int BitmapIndexedNode_index(BitmapIndexedNode * bin, int bit) {
  /* return Integer.bitCount(bitmap & (bit - 1)); */
  return __builtin_popcount((unsigned int)(bin->bitmap & (bit - 1)));
}

/* HashCollisionNode */

void * HashCollisionNode_ctor(void * self, va_list * app) {
  ((HashCollisionNode *)self)->hash = va_arg((*app), int);
  ((HashCollisionNode *)self)->count = va_arg((*app), int);
  ((HashCollisionNode *)self)->array = va_arg((*app), void *);
}

void * HashCollisionNode_assoc(void * self, int shift, int hash, void * key,
    void * val, void * addedLeaf) {
  HashCollisionNode * hcn = (HashCollisionNode *)self;
  if(hash == hcn->hash) {
    int idx = HashCollisionNode_findIndex(hcn, key);
    if(idx != -1) {
      if(Array_get(hcn->array, idx + 1) == val) {
        return self;
      }
      return INode_new(HashCollisionNode_class, hash, hcn->count,
          Array_cloneAndSet(hcn->array, idx + 1, val));
    }
    int array_length = Array_size(hcn->array);
    void * newArray = Array_new(array_length + 2);
    Array_copy(hcn->array, 0, newArray, 0, array_length);
    Array_set(newArray, array_length, key);
    Array_set(newArray, array_length + 1, val);
    MapEntry_set(addedLeaf, addedLeaf);
    return INode_new(HashCollisionNode_class, hash, hcn->count + 1, newArray);
  }
  // ridiculous one-liner broken into 5
  void * temp_array = Array_new(2);
  Array_set(temp_array, 0, NULL);
  Array_set(temp_array, 1, hcn);
  void * temp_bin = INode_new(BitmapIndexedNode_class,
      bitpos(hcn->hash, shift), temp_array);
  return INode_assoc(temp_bin, shift, hash, key, val, addedLeaf);
}

void * HashCollisionNode_without(void * self, int shift, int hash,
    void * key) {
  HashCollisionNode * hcn = (HashCollisionNode *)self;
  int idx = HashCollisionNode_findIndex(hcn, key);
  if(idx == -1) {
    return self;
  }
  if(hcn->count == 1) {
    return NULL;
  }
  return INode_new(HashCollisionNode_class, hash, hcn->count - 1,
      Array_removePair(hcn->array, idx / 2));
}

void * HashCollisionNode_find(void * self, int shift, int hash, void * key) {
  HashCollisionNode * hcn = (HashCollisionNode *)self;
  int idx = HashCollisionNode_findIndex(hcn, key);
  if(idx < 0) {
    return NULL;
  }
  if(equiv_func(key, Array_get(hcn->array, idx))) {
    return MapEntry_new(Array_get(hcn->array, idx),
        Array_get(hcn->array, idx + 1));
  }
  return NULL;
}

void * HashCollisionNode_findDef(void * self, int shift, int hash, void * key,
    void * notFound) {
  HashCollisionNode * hcn = (HashCollisionNode *)self;
  int idx = HashCollisionNode_findIndex(hcn, key);
  if(idx < 0) {
    return notFound;
  }
  if(equiv_func(key, Array_get(hcn->array, idx))) {
    return Array_get(hcn->array, idx + 1);
  }
  return notFound;
}

void HashCollisionNode_foreach(void * self, PHMFunc f, void * data) {
  HashCollisionNode * hcn = (HashCollisionNode *)self;
  int i;
  int count = Array_size(hcn->array);
  void * key;
  for(i = 0; i < count; i += 2) {
    key = Array_get(hcn->array, i);
    if(key != NULL) {
      void * value = Array_get(hcn->array, i + 1);
      f(key, value, data);
    } else {
      void * node = Array_get(hcn->array, i + 1);
      if(node != NULL) {
        INode_foreach(node, f, data);
      }
    }
  }
}

/* void * HashCollisionNode_nodeSeq(void * self); */
/* void * HashCollisionNode_kvreduce(void * self, void * f, void * init); */

int HashCollisionNode_findIndex(HashCollisionNode * hcn, void * key) {
  int i;
  for(i = 0; i < 2 * hcn->count; i += 2) {
    if(equiv_func(key, Array_get(hcn->array, i))) {
      return i;
    }
  }
  return -1;
}

/* Other */

int mask(int hash, int shift) {
  return (int)(((unsigned int)hash >> shift) & 0x01f);
}

int bitpos(int hash, int shift) {
  return 1 << mask(hash, shift);
}

int equiv_default(void * o1, void * o2) {
  return o1 == o2;
}


int hash_default(void * p) {
  return (int)((uint64_t)p ^ ((uint64_t)p >> 32));
}

