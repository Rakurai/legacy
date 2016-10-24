# cprops examples - Makefile
#
# This is a Makefile for cprops examples. If it has the name "Makefile.in" then
# it is a template for a Makefile;  to generate the actual Makefile, run 
# "./configure" in the cprops base directory. 
#
# Copyright (c) 2005-2007 by Ilan Aelion
# iaelion@users.sourceforge.net
# 
# 2007-30-11 V 0.1.7 
#
###############################################

srcdir=.
prefix=/usr/local
exec_prefix=${prefix}

libdir=$(DESTDIR)${exec_prefix}/lib
incdir=$(DESTDIR)${prefix}/include/cprops

CC = gcc
CFLAGS = -D_REENTRANT -I $(incdir) -I . -g -O2 -O2
LDFLAGS = -L../.libs -L$(libdir) 
LIBS = -lcprops -ldl  -lpthread

DBEXAMPLES = 
DBEXAMPLES_OBJ = $(DBEXAMPLES:.c=.o)
DBEXAMPLES_EXE = $(DBEXAMPLES:.c=)
DBLDFLAGS =  
DBLIBS =  

EXAMPLE_SRC = test_avl.c test_avl_multi.c test_client.c test_hashlist.c \
			  test_hashlist_iterator.c test_hashtable.c test_hashtable2.c \
			  test_httpasync.c test_httpclient.c test_list.c \
			  test_list_iterator.c test_priority_list.c test_proxy.c \
			  test_str.c test_trie.c test_trie2.c test_vector.c \
			  test_rb.c test_splay.c wf.c test_heap.c  \
			  test_mempool.c test_sorted_hash.c test_narytree.c \
			  test_multimap.c test_reindex.c test_reindex_unique.c \
			  test_wtab.c test_wtrie.c test_bstr.c test_rtrie.c

EXAMPLE_OBJ = $(EXAMPLE_SRC:.c=.o) 
EXAMPLE_EXE = $(EXAMPLE_SRC:.c=)

MAKEDEPEND=/opt/X11/bin/makedepend -Y

DEPEND=$(MAKEDEPEND) $(CFLAGS)

all: linkdir examples db_examples

linkdir:
	@if ! test -e cprops ; then ln -s .. cprops ; fi

examples: $(EXAMPLE_OBJ)

.c.o: 
	$(CC) $(CFLAGS) -c $(@:.o=.c) -o $@
	$(CC) $(LDFLAGS) $@ -o $(@:.o=) $(LIBS)

db_examples: $(DBEXAMPLES_EXE)

$(DBEXAMPLES_EXE): $(DBEXAMPLES_OBJ)
	@echo DBSTUFF
	$(CC) $(CFLAGS) -c $(@:=.c) -o $(@:=.o)
	$(CC) $(LDFLAGS) $(DBLDFLAGS) $(@:=.o) -o $@ $(LIBS) $(DBLIBS)

.PHONY: all clean linkdir

clean: 
	rm -f $(EXAMPLE_OBJ) $(EXAMPLE_OBJ:.o=) $(DBEXAMPLES_OBJ) \
	   $(DBEXAMPLES_EXE) core* *core vgcore* *.log

$(EXAMPLE_OBJ): Makefile

depend: linkdir $(EXAMPLE_SRC)
	@$(DEPEND) $(EXAMPLE_SRC) > /dev/null 2>&1
# DO NOT DELETE

test_avl.o: ./cprops/avl.h ./cprops/common.h ./cprops/collection.h
test_avl.o: ./cprops/log.h ./cprops/str.h ./cprops/config.h ./cprops/vector.h
test_avl.o: ./cprops/mempool.h ./cprops/rb.h ./cprops/hashtable.h
test_avl.o: ./cprops/util.h
test_avl_multi.o: ./cprops/vector.h ./cprops/common.h ./cprops/config.h
test_avl_multi.o: ./cprops/collection.h ./cprops/log.h ./cprops/str.h
test_avl_multi.o: ./cprops/avl.h ./cprops/vector.h ./cprops/mempool.h
test_avl_multi.o: ./cprops/rb.h ./cprops/hashtable.h
test_client.o: ./cprops/str.h ./cprops/common.h ./cprops/config.h
test_client.o: ./cprops/log.h ./cprops/str.h ./cprops/client.h
test_client.o: ./cprops/hashlist.h ./cprops/collection.h ./cprops/log.h
test_client.o: ./cprops/hashtable.h ./cprops/vector.h ./cprops/thread.h
test_client.o: ./cprops/linked_list.h ./cprops/mempool.h ./cprops/rb.h
test_client.o: ./cprops/socket.h
test_hashlist.o: ./cprops/hashlist.h ./cprops/common.h ./cprops/config.h
test_hashlist.o: ./cprops/collection.h ./cprops/log.h ./cprops/str.h
test_hashlist.o: ./cprops/hashtable.h
test_hashlist_iterator.o: ./cprops/collection.h ./cprops/common.h
test_hashlist_iterator.o: ./cprops/log.h ./cprops/str.h ./cprops/config.h
test_hashlist_iterator.o: ./cprops/hashlist.h ./cprops/collection.h
test_hashlist_iterator.o: ./cprops/hashtable.h
test_hashtable.o: ./cprops/hashtable.h ./cprops/common.h
test_hashtable.o: ./cprops/collection.h ./cprops/log.h ./cprops/str.h
test_hashtable.o: ./cprops/config.h ./cprops/util.h
test_hashtable2.o: ./cprops/thread.h ./cprops/common.h ./cprops/config.h
test_hashtable2.o: ./cprops/collection.h ./cprops/log.h ./cprops/str.h
test_hashtable2.o: ./cprops/linked_list.h ./cprops/mempool.h ./cprops/rb.h
test_hashtable2.o: ./cprops/vector.h ./cprops/hashtable.h ./cprops/hashlist.h
test_hashtable2.o: ./cprops/linked_list.h ./cprops/hashtable.h
test_httpasync.o: ./cprops/config.h ./cprops/log.h ./cprops/common.h
test_httpasync.o: ./cprops/str.h ./cprops/config.h ./cprops/str.h
test_httpasync.o: ./cprops/httpclient.h ./cprops/hashtable.h
test_httpasync.o: ./cprops/collection.h ./cprops/log.h ./cprops/vector.h
test_httpasync.o: ./cprops/thread.h ./cprops/linked_list.h ./cprops/mempool.h
test_httpasync.o: ./cprops/rb.h ./cprops/hashlist.h ./cprops/http.h
test_httpasync.o: ./cprops/trie.h ./cprops/mtab.h ./cprops/socket.h
test_httpasync.o: ./cprops/client.h
test_httpclient.o: ./cprops/config.h ./cprops/log.h ./cprops/common.h
test_httpclient.o: ./cprops/str.h ./cprops/config.h ./cprops/str.h
test_httpclient.o: ./cprops/httpclient.h ./cprops/hashtable.h
test_httpclient.o: ./cprops/collection.h ./cprops/log.h ./cprops/vector.h
test_httpclient.o: ./cprops/thread.h ./cprops/linked_list.h
test_httpclient.o: ./cprops/mempool.h ./cprops/rb.h ./cprops/hashlist.h
test_httpclient.o: ./cprops/http.h ./cprops/trie.h ./cprops/mtab.h
test_httpclient.o: ./cprops/socket.h ./cprops/client.h
test_list.o: ./cprops/linked_list.h
test_list_iterator.o: ./cprops/linked_list.h
test_priority_list.o: ./cprops/priority_list.h ./cprops/common.h
test_priority_list.o: ./cprops/config.h ./cprops/collection.h ./cprops/log.h
test_priority_list.o: ./cprops/str.h ./cprops/linked_list.h
test_priority_list.o: ./cprops/mempool.h ./cprops/rb.h ./cprops/vector.h
test_priority_list.o: ./cprops/hashtable.h ./cprops/thread.h
test_priority_list.o: ./cprops/hashlist.h
test_proxy.o: ./cprops/log.h ./cprops/common.h ./cprops/str.h
test_proxy.o: ./cprops/config.h ./cprops/str.h ./cprops/httpclient.h
test_proxy.o: ./cprops/hashtable.h ./cprops/collection.h ./cprops/log.h
test_proxy.o: ./cprops/vector.h ./cprops/thread.h ./cprops/linked_list.h
test_proxy.o: ./cprops/mempool.h ./cprops/rb.h ./cprops/hashlist.h
test_proxy.o: ./cprops/http.h ./cprops/trie.h ./cprops/mtab.h
test_proxy.o: ./cprops/socket.h ./cprops/client.h
test_str.o: ./cprops/str.h ./cprops/common.h ./cprops/config.h
test_trie.o: ./cprops/trie.h ./cprops/common.h ./cprops/config.h
test_trie.o: ./cprops/hashtable.h ./cprops/collection.h ./cprops/log.h
test_trie.o: ./cprops/str.h ./cprops/vector.h ./cprops/mtab.h
test_trie.o: ./cprops/mempool.h ./cprops/rb.h
test_trie2.o: ./cprops/trie.h ./cprops/common.h ./cprops/config.h
test_trie2.o: ./cprops/hashtable.h ./cprops/collection.h ./cprops/log.h
test_trie2.o: ./cprops/str.h ./cprops/vector.h ./cprops/mtab.h
test_trie2.o: ./cprops/mempool.h ./cprops/rb.h ./cprops/config.h
test_trie2.o: ./cprops/util.h
test_vector.o: ./cprops/vector.h ./cprops/common.h ./cprops/config.h
test_vector.o: ./cprops/collection.h ./cprops/log.h ./cprops/str.h
test_rb.o: ./cprops/rb.h ./cprops/common.h ./cprops/vector.h
test_rb.o: ./cprops/config.h ./cprops/collection.h ./cprops/log.h
test_rb.o: ./cprops/str.h ./cprops/mempool.h ./cprops/rb.h
test_rb.o: ./cprops/hashtable.h ./cprops/util.h
test_splay.o: ./cprops/splay.h ./cprops/common.h ./cprops/collection.h
test_splay.o: ./cprops/log.h ./cprops/str.h ./cprops/config.h
test_splay.o: ./cprops/vector.h ./cprops/mempool.h ./cprops/rb.h
test_splay.o: ./cprops/hashtable.h ./cprops/util.h
wf.o: ./cprops/avl.h ./cprops/common.h ./cprops/collection.h ./cprops/log.h
wf.o: ./cprops/str.h ./cprops/config.h ./cprops/vector.h ./cprops/mempool.h
wf.o: ./cprops/rb.h ./cprops/hashtable.h ./cprops/str.h ./cprops/util.h
test_heap.o: ./cprops/heap.h ./cprops/config.h ./cprops/common.h
test_heap.o: ./cprops/collection.h ./cprops/log.h ./cprops/str.h
test_heap.o: ./cprops/log.h ./cprops/util.h ./cprops/hashtable.h
test_mempool.o: ./cprops/util.h ./cprops/common.h ./cprops/config.h
test_mempool.o: ./cprops/mempool.h ./cprops/collection.h ./cprops/log.h
test_mempool.o: ./cprops/str.h ./cprops/rb.h ./cprops/vector.h
test_mempool.o: ./cprops/mempool.h ./cprops/hashtable.h
test_sorted_hash.o: ./cprops/sorted_hash.h ./cprops/common.h
test_sorted_hash.o: ./cprops/collection.h ./cprops/log.h ./cprops/str.h
test_sorted_hash.o: ./cprops/config.h ./cprops/vector.h ./cprops/mempool.h
test_sorted_hash.o: ./cprops/rb.h ./cprops/hashtable.h ./cprops/util.h
test_narytree.o: ./cprops/nary.h ./cprops/common.h ./cprops/collection.h
test_narytree.o: ./cprops/log.h ./cprops/str.h ./cprops/config.h
test_narytree.o: ./cprops/util.h
test_multimap.o: ./cprops/multimap.h ./cprops/common.h ./cprops/rb.h
test_multimap.o: ./cprops/vector.h ./cprops/config.h ./cprops/collection.h
test_multimap.o: ./cprops/log.h ./cprops/str.h ./cprops/mempool.h
test_multimap.o: ./cprops/hashtable.h ./cprops/hashlist.h ./cprops/util.h
test_reindex.o: ./cprops/multimap.h ./cprops/common.h ./cprops/rb.h
test_reindex.o: ./cprops/vector.h ./cprops/config.h ./cprops/collection.h
test_reindex.o: ./cprops/log.h ./cprops/str.h ./cprops/mempool.h
test_reindex.o: ./cprops/hashtable.h ./cprops/hashlist.h ./cprops/util.h
test_reindex_unique.o: ./cprops/multimap.h ./cprops/common.h ./cprops/rb.h
test_reindex_unique.o: ./cprops/vector.h ./cprops/config.h
test_reindex_unique.o: ./cprops/collection.h ./cprops/log.h ./cprops/str.h
test_reindex_unique.o: ./cprops/mempool.h ./cprops/hashtable.h
test_reindex_unique.o: ./cprops/hashlist.h ./cprops/util.h
test_wtab.o: cprops/wtab.h ./cprops/common.h ./cprops/collection.h
test_wtab.o: ./cprops/log.h ./cprops/str.h ./cprops/config.h
test_wtrie.o: ./cprops/wtrie.h ./cprops/common.h ./cprops/config.h
test_wtrie.o: ./cprops/hashtable.h ./cprops/collection.h ./cprops/log.h
test_wtrie.o: ./cprops/str.h ./cprops/vector.h ./cprops/wtab.h
test_wtrie.o: ./cprops/mempool.h ./cprops/rb.h
test_bstr.o: cprops/bstr.h ./cprops/collection.h ./cprops/common.h
test_bstr.o: ./cprops/log.h ./cprops/str.h ./cprops/config.h
test_rtrie.o: cprops/rtrie.h ./cprops/common.h ./cprops/config.h
test_rtrie.o: ./cprops/vector.h ./cprops/collection.h ./cprops/log.h
test_rtrie.o: ./cprops/str.h cprops/bstr.h ./cprops/mempool.h ./cprops/rb.h
test_rtrie.o: ./cprops/hashtable.h ./cprops/vector.h
