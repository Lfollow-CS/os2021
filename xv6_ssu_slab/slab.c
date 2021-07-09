#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "slab.h"
#include "stdbool.h"
#include "stdio.h" 

struct {
	struct spinlock lock;
	struct slab slab[NSLAB];
} stable;

void slabinit(){
	initlock(&stable.lock, "stable");
	int i = 0;
	acquire(&stable.lock);
	for(struct slab* sl = stable.slab;sl < &stable.slab[NSLAB];sl++){
		sl->size = 1 << (i+3);
		sl->num_free_objects = PGSIZE >> (i + 3);
		sl->num_pages = 1;
		sl->num_used_objects = 0;
		sl->num_objects_per_page = sl->num_free_objects;
		sl->bitmap = kalloc();
		memset(sl->bitmap, 0, PGSIZE);
		sl->page[0] = kalloc();
		memset(sl->page[0], 0, PGSIZE);
		i++;
	}
	release(&stable.lock);
}

char *kmalloc(int size){
	if(size <= 0 || size >2048){
		cprintf("invalis size");
		return -1;
	}
	int req = -1;
	int resize;
	for(int i=0;i<NSLAB;i++){
		resize = size >> (i + 3);
		if(resize == 0){
			req = i;
			break;
		}
	}
	if(req == -1)
		return -1;

	acquire(&stable.lock);
	if(stable.slab[req].num_free_objects == 0 && stable.slab[req].num_pages < MAX_PAGES_PER_SLAB){
		stable.slab[req].num_pages += 1;
		stable.slab[req].num_free_objects += stable.slab[req].num_objects_per_page;
		stable.slab[req].page[stable.slab[req].num_pages - 1] = kalloc();
		memset(stable.slab[req].page[stable.slab[req].num_pages - 1], 0, PGSIZE);
	}
	char* ret = 0;
	for(int i=0;i<stable.slab[req].num_pages;i++){
		for(int k=0;k < stable.slab[req].num_objects_per_page;k++){
			char* chbit1 = (stable.slab[req].bitmap + stable.slab[req].num_objects_per_page*i+ k);
			char chbit2 = *chbit1 & 1;
			if(chbit2 == 0){
				stable.slab[req].num_used_objects++;
				stable.slab[req].num_free_objects--;	
				*(stable.slab[req].bitmap + stable.slab[req].num_objects_per_page*i + k) |= 1;	
				ret = stable.slab[req].page[i] + k*stable.slab[req].size;
				i = stable.slab[req].num_pages;
				break;
			}
		}
	}
	release(&stable.lock);
	return ret;
}

void kmfree(char *addr, int size){
	if(size < 0 || size > 2048){
		cprintf("invalis size");
		exit();
	}
	
	int req = -1;
	int resize;
	for(int i=0;i<NSLAB;i++){
		resize = size >> (i + 3);
		if(resize == 0){
			req = i;
			break;
		}
	}
	if(req == -1){
		cprintf("invalis slab");
		exit();
	}
	
	acquire(&stable.lock);
	for(int i=0;i<stable.slab[req].num_pages;i++){
		for(int k=0;k < stable.slab[req].num_objects_per_page;k++){
			if(addr == stable.slab[req].page[i] + k*stable.slab[req].size){	
				stable.slab[req].num_used_objects--;
				stable.slab[req].num_free_objects++;	
				*(stable.slab[req].bitmap + stable.slab[req].num_objects_per_page*i + k) &= ~1;		
				memset(addr, 0, stable.slab[req].size);
				i = stable.slab[req].num_pages;
				break;
			}
		}
	}
	release(&stable.lock);
	
}

void slabdump(){
	cprintf("__slabdump__\n");

	struct slab *s;

	cprintf("size\tnum_pages\tused_objects\tfree_objects\n");

	for(s = stable.slab; s < &stable.slab[NSLAB]; s++){
		cprintf("%d\t%d\t\t%d\t\t%d\n", 
			s->size, s->num_pages, s->num_used_objects, s->num_free_objects);
	}
}
