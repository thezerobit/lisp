#ifndef MUTABLE_HASH_H
#define MUTABLE_HASH_H

#include <glib.h>
#include "lisp.h"

guint object_hash(gconstpointer p);
gboolean object_compare(gconstpointer v1, gconstpointer v2);

int is_mutable_hash(pointer p);
pointer new_mutable_hash();
MutableHash get_mutable_hash(pointer p);
int is_mutable_hash_equal(pointer p, pointer o);
pointer mutable_hash_get(pointer hash, pointer key);
pointer mutable_hash_set(pointer hash, pointer key, pointer val);
void print_mutable_hash(pointer p);


#endif /* MUTABLE_HASH_H */

