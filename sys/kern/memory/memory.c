// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/kern/memory/memory.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include "memory.h"

#include "kern/panic/panic.h"

#include <lib/kstdio/kstdio.h>
#include <lib/kstring/kstring.h>

struct multiboot_tag {
	kuint32_t type;
	kuint32_t size;
};

// Global memory state
static page_frame_t *free_page_list = KNULL;
static page_frame_t *page_frames    = KNULL;
static kuint64_t     total_pages    = 0;
static kuint64_t     free_pages     = 0;
static kuint64_t     used_pages     = 0;

// Heap state
static heap_block_t *heap_start = KNULL;
static heap_block_t *heap_end   = KNULL;
static kuint64_t     heap_size  = 0;
static kuint64_t     heap_used  = 0;

// Current page table
static pml4_t *current_pml4 = KNULL;

// Memory map from multiboot
static memory_map_entry_t *memory_map         = KNULL;
static kuint32_t           memory_map_entries = 0;

// Multiboot2 memory map tag
struct multiboot_tag_mmap {
	kuint32_t type;
	kuint32_t size;
	kuint32_t entry_size;
	kuint32_t entry_version;
};

#define MULTIBOOT_TAG_TYPE_MMAP 6

// Physical memory management

void
    kmemory_init (void *multiboot_info) {
	if (!multiboot_info) {
		panic.main(1, KNULL);
		return;
	}

	// Parse multiboot memory map
	kuint32_t total_size  = *(kuint32_t *) multiboot_info;
	kuint8_t *current_tag = (kuint8_t *) ((kuintptr_t) multiboot_info + 8);
	kuint8_t *end_tags    = (kuint8_t *) ((kuintptr_t) multiboot_info + total_size);

	while (current_tag < end_tags) {
		struct multiboot_tag *tag = (struct multiboot_tag *) current_tag;

		if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
			struct multiboot_tag_mmap *mmap_tag =
			    (struct multiboot_tag_mmap *) tag;
			memory_map =
			    (memory_map_entry_t *) ((kuint8_t *) mmap_tag
			                            + sizeof(struct multiboot_tag_mmap));
			memory_map_entries =
			    (kuint32_t) ((mmap_tag->size
			                  - sizeof(struct multiboot_tag_mmap))
			                 / mmap_tag->entry_size);
			break;
		}

		current_tag += (kuint32_t) ((tag->size + 7) & ~(kuint32_t) 0x7);
	}

	if (!memory_map) {
		panic.main(2, KNULL);
		return;
	}

	// Calculate total usable memory
	total_pages = 0;
	for (kuint32_t i = 0; i < memory_map_entries; i++) {
		if (memory_map[i].type == MEMORY_USABLE) {
			total_pages += memory_map[i].length / PAGE_SIZE;
		}
	}

	free_pages = total_pages;
	used_pages = 0;

	// Initialize page frame array
	page_frames = (page_frame_t *) KERNEL_HEAP_BASE;

	/*
	 * Map the page frames array:
	 * The first 4 GiB is identity-mapped by the bootstrap page tables, so
	 * the 16 MiB heap area we chose (0x01000000-0x05000000) is already
	 * accessible.  Therefore we don’t need extra page-table work here --
	 * attempting to allocate page frames before the free list exists would
	 * fail. We can safely skip explicit mapping at this early stage.
	 */

	// Initialize page frame structures
	for (kuint64_t i = 0; i < total_pages; i++) {
		page_frames[i].next          = KNULL;
		page_frames[i].physical_addr = i * PAGE_SIZE;
		page_frames[i].ref_count     = 0;
		page_frames[i].is_free       = ktrue;
	}

	// Build free page list
	free_page_list = KNULL;
	for (kuint64_t i = 0; i < total_pages; i++) {
		// Skip pages used by kernel
		if (page_frames[i].physical_addr < KERNEL_HEAP_BASE) {
			page_frames[i].is_free = kfalse;
			used_pages++;
			free_pages--;
			continue;
		}

		page_frames[i].next = free_page_list;
		free_page_list      = &page_frames[i];
	}

	// Initialize virtual memory
	vm_init();

	// Initialize heap
	heap_init();
}

page_frame_t *
    allocate_page_frame (void) {
	if (!free_page_list) {
		return KNULL;  // Out of memory
	}

	page_frame_t *frame = free_page_list;
	free_page_list      = frame->next;

	frame->next      = KNULL;
	frame->ref_count = 1;
	frame->is_free   = kfalse;

	free_pages--;
	used_pages++;

	return frame;
}

void
    free_page_frame (page_frame_t *frame) {
	if (!frame || frame->is_free) {
		return;
	}

	frame->ref_count--;
	if (frame->ref_count > 0) {
		return;  // Still referenced
	}

	frame->next    = free_page_list;
	free_page_list = frame;
	frame->is_free = ktrue;

	free_pages++;
	used_pages--;
}

kuint64_t
    get_physical_addr (page_frame_t *frame) {
	return frame ? frame->physical_addr : 0;
}

page_frame_t *
    get_page_frame (kuint64_t physical_addr) {
	kuint64_t page_index = physical_addr / PAGE_SIZE;
	if (page_index >= total_pages) {
		return KNULL;
	}
	return &page_frames[page_index];
}

// Virtual memory management
void
    vm_init (void) {
	// Get current page table from CR3
	kuint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	current_pml4 = (pml4_t *) (cr3 & ~(kuint64_t) 0xFFF);
}

kbool
    vm_map_page (kuint64_t virtual_addr, kuint64_t physical_addr, kuint64_t flags) {
	if ((virtual_addr & 0xFFF) || (physical_addr & 0xFFF))
		return kfalse;

	kuint64_t pml4_index = (virtual_addr >> 39) & 0x1FF;
	kuint64_t pml3_index = (virtual_addr >> 30) & 0x1FF;
	kuint64_t pml2_index = (virtual_addr >> 21) & 0x1FF;
	kuint64_t pml1_index = (virtual_addr >> 12) & 0x1FF;

	pml3_t *pml3 = KNULL;
	if (!(current_pml4->entries[pml4_index] & PAGE_PRESENT)) {
		page_frame_t *frame = allocate_page_frame();
		if (!frame)
			return kfalse;

		kuint64_t phys = get_physical_addr(frame);
		pml3           = (pml3_t *) (phys + KERNEL_BASE);
		kmemset(pml3, 0, PAGE_SIZE);

		current_pml4->entries[pml4_index] = phys | PAGE_PRESENT | PAGE_WRITABLE;
	} else {
		kuint64_t pml3_phys =
		    current_pml4->entries[pml4_index] & ~(kuint64_t) 0xFFF;
		pml3 = (pml3_t *) (pml3_phys + KERNEL_BASE);
	}

	pml2_t *pml2 = KNULL;
	if (!(pml3->entries[pml3_index] & PAGE_PRESENT)) {
		page_frame_t *frame = allocate_page_frame();
		if (!frame)
			return kfalse;

		kuint64_t phys = get_physical_addr(frame);
		pml2           = (pml2_t *) (phys + KERNEL_BASE);
		kmemset(pml2, 0, PAGE_SIZE);

		pml3->entries[pml3_index] = phys | PAGE_PRESENT | PAGE_WRITABLE;
	} else {
		kuint64_t pml2_phys = pml3->entries[pml3_index] & ~(kuint64_t) 0xFFF;
		pml2                = (pml2_t *) (pml2_phys + KERNEL_BASE);
	}

	pml1_t *pml1 = KNULL;
	if (!(pml2->entries[pml2_index] & PAGE_PRESENT)) {
		page_frame_t *frame = allocate_page_frame();
		if (!frame)
			return kfalse;

		kuint64_t phys = get_physical_addr(frame);
		pml1           = (pml1_t *) (phys + KERNEL_BASE);
		kmemset(pml1, 0, PAGE_SIZE);

		pml2->entries[pml2_index] = phys | PAGE_PRESENT | PAGE_WRITABLE;
	} else {
		kuint64_t pml1_phys = pml2->entries[pml2_index] & ~(kuint64_t) 0xFFF;
		pml1                = (pml1_t *) (pml1_phys + KERNEL_BASE);
	}

	if (pml1->entries[pml1_index] & PAGE_PRESENT) {
		// return kfalse;
	}

	pml1->entries[pml1_index] = physical_addr | flags;

	__asm__ volatile("" ::: "memory");

	vm_invalidate_page(virtual_addr);

	return ktrue;
}

kbool
    vm_unmap_page (kuint64_t virtual_addr) {
	kuint64_t pml4_index = (virtual_addr >> 39) & 0x1FF;
	kuint64_t pml3_index = (virtual_addr >> 30) & 0x1FF;
	kuint64_t pml2_index = (virtual_addr >> 21) & 0x1FF;
	kuint64_t pml1_index = (virtual_addr >> 12) & 0x1FF;

	if (!(current_pml4->entries[pml4_index] & PAGE_PRESENT)) {
		return kfalse;
	}

	kuint64_t pml3_phys = current_pml4->entries[pml4_index] & ~(kuint64_t) 0xFFF;
	pml3_t   *pml3      = (pml3_t *) (pml3_phys + KERNEL_BASE);

	if (!(pml3->entries[pml3_index] & PAGE_PRESENT)) {
		return kfalse;
	}

	kuint64_t pml2_phys = pml3->entries[pml3_index] & ~(kuint64_t) 0xFFF;
	pml2_t   *pml2      = (pml2_t *) (pml2_phys + KERNEL_BASE);

	if (!(pml2->entries[pml2_index] & PAGE_PRESENT)) {
		return kfalse;
	}

	kuint64_t pml1_phys = pml2->entries[pml2_index] & ~(kuint64_t) 0xFFF;
	pml1_t   *pml1      = (pml1_t *) (pml1_phys + KERNEL_BASE);

	if (!(pml1->entries[pml1_index] & PAGE_PRESENT)) {
		return kfalse;
	}

	// Get physical address and free the page frame
	kuint64_t     physical_addr = pml1->entries[pml1_index] & ~(kuint64_t) 0xFFF;
	page_frame_t *frame         = get_page_frame(physical_addr);
	if (frame) {
		free_page_frame(frame);
	}

	// Clear the page table entry
	pml1->entries[pml1_index] = 0;

	// Invalidate TLB entry
	vm_invalidate_page(virtual_addr);

	return ktrue;
}

kuint64_t
    vm_get_physical_addr (kuint64_t virtual_addr) {
	kuint64_t pml4_index = (virtual_addr >> 39) & 0x1FF;
	kuint64_t pml3_index = (virtual_addr >> 30) & 0x1FF;
	kuint64_t pml2_index = (virtual_addr >> 21) & 0x1FF;
	kuint64_t pml1_index = (virtual_addr >> 12) & 0x1FF;

	if (!(current_pml4->entries[pml4_index] & PAGE_PRESENT)) {
		return 0;
	}

	kuint64_t pml3_phys = current_pml4->entries[pml4_index] & ~(kuint64_t) 0xFFF;
	pml3_t   *pml3      = (pml3_t *) (pml3_phys + KERNEL_BASE);

	if (!(pml3->entries[pml3_index] & PAGE_PRESENT)) {
		return 0;
	}

	kuint64_t pml2_phys = pml3->entries[pml3_index] & ~(kuint64_t) 0xFFF;
	pml2_t   *pml2      = (pml2_t *) (pml2_phys + KERNEL_BASE);

	if (!(pml2->entries[pml2_index] & PAGE_PRESENT)) {
		return 0;
	}

	kuint64_t pml1_phys = pml2->entries[pml2_index] & ~(kuint64_t) 0xFFF;
	pml1_t   *pml1      = (pml1_t *) (pml1_phys + KERNEL_BASE);

	if (!(pml1->entries[pml1_index] & PAGE_PRESENT)) {
		return 0;
	}

	return (pml1->entries[pml1_index] & ~(kuint64_t) 0xFFF) | (virtual_addr & 0xFFF);
}

void
    vm_invalidate_page (kuint64_t virtual_addr) {
	__asm__ volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}

void
    vm_switch_pagetable (pml4_t *new_pml4) {
	current_pml4 = new_pml4;
	__asm__ volatile("mov %0, %%cr3" : : "r"((kuint64_t) new_pml4) : "memory");
}

// Heap management

void
    heap_init (void) {
	heap_start = (heap_block_t *) KERNEL_HEAP_BASE;
	heap_size  = KERNEL_HEAP_SIZE;
	heap_used  = 0;

	// Initialize first block
	heap_start->size    = heap_size - sizeof(heap_block_t);
	heap_start->is_free = ktrue;
	heap_start->next    = KNULL;
	heap_start->prev    = KNULL;

	heap_end = heap_start;
}

void *
    kmalloc (ksize_t size) {
	if (size == 0)
		return KNULL;

	// Add header size and align to 8 bytes
	ksize_t total_size = (size + sizeof(heap_block_t) + 7) & ~(ksize_t) 0x7;

	heap_block_t *current = heap_start;
	while (current) {
		if (current->is_free && current->size >= total_size) {
			// Split block if it's much larger than needed
			if (current->size >= total_size + sizeof(heap_block_t) + 64) {
				heap_block_t *new_block =
				    (heap_block_t *) ((kuint8_t *) current
				                      + sizeof(heap_block_t)
				                      + total_size);
				new_block->size =
				    current->size - total_size - sizeof(heap_block_t);
				new_block->is_free = ktrue;
				new_block->next    = current->next;
				new_block->prev    = current;

				if (current->next) {
					current->next->prev = new_block;
				}
				current->next = new_block;
				current->size = total_size;
			}

			current->is_free = kfalse;
			heap_used += current->size;

			return (kuint8_t *) current + sizeof(heap_block_t);
		}
		current = current->next;
	}

	return KNULL;  // Out of memory
}

void *
    kcalloc (ksize_t count, ksize_t size) {
	ksize_t total_size = count * size;
	void   *ptr        = kmalloc(total_size);
	if (ptr) {
		kmemset(ptr, 0, total_size);
	}
	return ptr;
}

void *
    krealloc (void *ptr, ksize_t size) {
	if (!ptr)
		return kmalloc(size);
	if (size == 0) {
		kfree(ptr);
		return KNULL;
	}

	heap_block_t *block = (heap_block_t *) ((kuint8_t *) ptr - sizeof(heap_block_t));
	if (block->size >= size) {
		return ptr;  // No need to reallocate
	}

	void *new_ptr = kmalloc(size);
	if (new_ptr) {
		kmemcpy(new_ptr, ptr, block->size);
		kfree(ptr);
	}
	return new_ptr;
}

void
    kfree (void *ptr) {
	if (!ptr)
		return;

	heap_block_t *block = (heap_block_t *) ((kuint8_t *) ptr - sizeof(heap_block_t));
	if (block->is_free)
		return;  // Already freed

	block->is_free = ktrue;
	heap_used -= block->size;

	// Merge with next block if it's free
	if (block->next && block->next->is_free) {
		block->size += sizeof(heap_block_t) + block->next->size;
		block->next = block->next->next;
		if (block->next) {
			block->next->prev = block;
		}
	}

	// Merge with previous block if it's free
	if (block->prev && block->prev->is_free) {
		block->prev->size += sizeof(heap_block_t) + block->size;
		block->prev->next = block->next;
		if (block->next) {
			block->next->prev = block->prev;
		}
	}
}

void
    heap_defrag (void) {
	// Simple defragmentation: move all free blocks to the end
	heap_block_t *current   = heap_start;
	heap_block_t *last_free = KNULL;

	while (current) {
		if (current->is_free) {
			if (last_free) {
				// Move this free block after last_free
				heap_block_t *next = current->next;

				if (current->prev) {
					current->prev->next = current->next;
				}
				if (current->next) {
					current->next->prev = current->prev;
				}

				current->next   = last_free->next;
				current->prev   = last_free;
				last_free->next = current;
				if (current->next) {
					current->next->prev = current;
				}

				current = next;
			} else {
				last_free = current;
				current   = current->next;
			}
		} else {
			current = current->next;
		}
	}
}

// Memory statistics

memory_stats_t
    memory_get_stats (void) {
	memory_stats_t stats;
	stats.total_physical_pages = total_pages;
	stats.free_physical_pages  = free_pages;
	stats.used_physical_pages  = used_pages;
	stats.total_heap_size      = heap_size;
	stats.free_heap_size       = heap_size - heap_used;
	stats.used_heap_size       = heap_used;
	return stats;
}

void
    memory_print_stats (void) {
	memory_stats_t stats = memory_get_stats();
	kprintf("Memory Statistics:\n");
	kprintf("  Physical Memory: %llu pages total, %llu free, %llu used\n",
	        stats.total_physical_pages,
	        stats.free_physical_pages,
	        stats.used_physical_pages);
	kprintf("  Heap Memory: %llu bytes total, %llu free, %llu used\n",
	        stats.total_heap_size,
	        stats.free_heap_size,
	        stats.used_heap_size);
}

// Kernel memory management

void *
    kernel_alloc (ksize_t size) {
	return kmalloc(size);
}

void *
    kernel_alloc_aligned (ksize_t size, ksize_t alignment) {
	(void) alignment;
	// For now, just use regular allocation
	// TODO: Implement proper aligned allocation
	return kmalloc(size);
}

void
    kernel_free (void *ptr) {
	kfree(ptr);
}

// Memory mapping
void *
    memory_map_physical (kuint64_t physical_addr, ksize_t size, kuint64_t flags) {
	// Map physical memory to kernel virtual space
	kuint64_t virtual_addr = KERNEL_BASE + 0x1000000;  // Temporary mapping area
	for (ksize_t i = 0; i < (size + PAGE_SIZE - 1) / PAGE_SIZE; i++) {
		kuint64_t page_virt = virtual_addr + i * PAGE_SIZE;
		kuint64_t page_phys = physical_addr + i * PAGE_SIZE;

		if (!vm_map_page(page_virt, page_phys, flags)) {
			return KNULL;
		}
	}

	return (void *) virtual_addr;
}

void
    memory_unmap_physical (void *virtual_addr, ksize_t size) {
	kuint64_t addr = (kuint64_t) virtual_addr;

	for (ksize_t i = 0; i < (size + PAGE_SIZE - 1) / PAGE_SIZE; i++) {
		vm_unmap_page(addr + i * PAGE_SIZE);
	}
}
