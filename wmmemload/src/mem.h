/*
 * mem.h - module to get memory/swap usages in percent
 *
 * Copyright (c) 2001 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * licensed under the GPL
 */

struct mem_options {
    int ignore_buffers;
    int ignore_cached;
    int ignore_wired;
};

void mem_init(void);
void mem_getusage(int *per_mem, int *per_swap, const struct mem_options *opts);
