// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/fs/ext2/ext2.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include "ext2.h"

#include <kern/memory/memory.h>
#include <lib/kstdio/kstdio.h>
#include <lib/kstring/kstring.h>

static kuint64_t          g_base_lba = 0;  // Partition/LBA base of filesystem
static ext2_superblock_t  g_superblock;
static ext2_group_desc_t *g_group_desc = KNULL;
static kuint32_t          g_block_size = 0;
kbool                     is_mounted   = kfalse;

// Caller-supplied I/O functions (sector based, 512-byte)
static int (*g_read_sector)(kuint64_t lba, kuint32_t count, void *buf)        = KNULL;
static int (*g_write_sector)(kuint64_t lba, kuint32_t count, const void *buf) = KNULL;

// Convenience wrapper: read N sectors into dst
static int
    kread_sectors (kuint64_t lba, kuint32_t cnt, void *dst) {
	if (!g_read_sector) {
		return EXT2_ERR_IO;
	}
	return g_read_sector(lba, cnt, dst);
}

// Read an aligned filesystem block into dst. Assumes dst has at least block_size bytes.
static int
    kread_block (kuint32_t block_id, void *dst) {
	kuint64_t first_lba = g_base_lba + ((kuint64_t) block_id * g_block_size) / 512;
	kuint32_t cnt       = g_block_size / 512;
	return kread_sectors(first_lba, cnt, dst);
}

// Forward declaration for functions used before definition
static int
    kread_inode (kuint32_t ino, ext2_inode_t *out);

void
    ext2_set_block_device (int (*read)(kuint64_t, kuint32_t, void *),
                           int (*write)(kuint64_t, kuint32_t, const void *)) {
	g_read_sector  = read;
	g_write_sector = write;  // May be KNULL for read-only volumes
}

int
    ext2_mount (kuint64_t base_lba) {
	if (!g_read_sector) {
		return EXT2_ERR_IO;
	}

	g_base_lba = base_lba;

	/*
	 * Read raw 1024-byte superblock into temp buffer and copy only the
	 * structure portion we need. This avoids overflowing g_superblock
	 * (struct is ~128 bytes).
	 */
	kuint8_t sb_raw[1024];
	if (kread_sectors(base_lba + EXT2_SUPERBLOCK_OFFSET / 512, 2, sb_raw) != 0) {
		return EXT2_ERR_IO;
	}
	kmemcpy(&g_superblock, sb_raw, sizeof(ext2_superblock_t));

	if (g_superblock.magic != EXT2_SUPER_MAGIC) {
#ifdef EXT2_DEBUG
		kprintf("EXT2: bad magic 0x%04x (expected 0x%04x)\n",
		        g_superblock.magic,
		        EXT2_SUPER_MAGIC);
#endif
		return EXT2_ERR_BAD_MAGIC;
	}

	/*
	 * Compute block size and validate (EXT2 supports 1K,2K,4K). Anything
	 * larger is non-standard and can easily blow up our simple kmalloc.
	 * Reject filesystems with block sizes >4 KiB.
	 */
	if (g_superblock.log_block_size > 2) {
#ifdef EXT2_DEBUG
		kprintf("EXT2: unsupported block size (log=%u)\n",
		        g_superblock.log_block_size);
#endif
		return EXT2_ERR_UNSUPPORTED;
	}
	g_block_size = 1024U << g_superblock.log_block_size;

	kuint32_t groups = (g_superblock.blocks_count + g_superblock.blocks_per_group - 1)
	                   / g_superblock.blocks_per_group;

	ksize_t gd_table_bytes  = groups * sizeof(ext2_group_desc_t);
	ksize_t gd_table_blocks = (gd_table_bytes + g_block_size - 1) / g_block_size;

	g_group_desc = (ext2_group_desc_t *) kmalloc(gd_table_blocks * g_block_size);
	if (!g_group_desc) {
		return EXT2_ERR_IO;
	}

	/*
	 * Group descriptor table starts right after superblock (block 2 for 1K blk)
	 * For 1 KiB block size the layout is: block0=boot, block1=superblock,
	 * block2=group descriptor table. For 2 KiB/4 KiB, superblock is at
	 * byte 1024 inside block0, so the descriptor table immediately follows
	 * in block1.
	 */
	kuint32_t gd_start_blk = (g_block_size == 1024) ? 2 : 1;
	for (kuint32_t i = 0; i < gd_table_blocks; i++) {
		if (kread_block(gd_start_blk + i,
		                (kuint8_t *) g_group_desc + i * g_block_size)
		    != 0) {
			return EXT2_ERR_IO;
		}
	}

#ifdef EXT2_DEBUG
	kprintf("EXT2: mounted. Block size %u, groups %u\n", g_block_size, groups);
#endif
	return EXT2_OK;
}

int
    ext2_open (const char *path, ext2_file_t *out_file) {
	(void) path;
	(void) out_file;
	if (!path || !out_file)
		return EXT2_ERR_INVALID;

	// If path is just "/" open root directory
	if (kstrcmp(path, "/") == 0) {
		if (kread_inode(EXT2_ROOT_INODE, &out_file->inode) != EXT2_OK)
			return EXT2_ERR_IO;
		out_file->pos = 0;
		return EXT2_OK;
	}

	// Make a modifiable copy of path without leading '/'
	char    tmp[256];
	ksize_t plen = (ksize_t) kstrlen(path);
	if (plen >= sizeof(tmp))
		return EXT2_ERR_INVALID;
	kstrcpy(tmp, path[0] == '/' ? path + 1 : path);

	// Current inode when traversing
	ext2_inode_t cur_inode;
	if (kread_inode(EXT2_ROOT_INODE, &cur_inode) != EXT2_OK)
		return EXT2_ERR_IO;

	char *tok = kstrtok(tmp, "/");
	char *next_tok;
	while (tok) {
		next_tok = kstrtok(KNULL, "/");

		// Search for tok in cur_inode directory entries (direct blocks only)
		kuint32_t found_ino = 0;

		kuint8_t *blk_buf = kmalloc(g_block_size);
		if (!blk_buf)
			return EXT2_ERR_IO;

		for (int i = 0; i < 12 && found_ino == 0; i++) {
			kuint32_t blk = cur_inode.block[i];
			if (blk == 0)
				continue;
			if (kread_block(blk, blk_buf) != 0) {
				kfree(blk_buf);
				return EXT2_ERR_IO;
			}
			kuint32_t off = 0;
			while (off < g_block_size) {
				ext2_dir_entry_t *ent =
				    (ext2_dir_entry_t *) (blk_buf + off);
				if (ent->rec_len == 0)
					break;
				if (ent->inode != 0
				    && ent->name_len == (kuint8_t) kstrlen(tok)
				    && kmemcmp(ent->name, tok, ent->name_len) == 0) {
					found_ino = ent->inode;
					break;
				}
				off += ent->rec_len;
			}
		}
		kfree(blk_buf);
		if (found_ino == 0)
			return EXT2_ERR_PATH_NOT_FOUND;

		// Load inode
		if (kread_inode(found_ino, &cur_inode) != EXT2_OK)
			return EXT2_ERR_IO;

		if (next_tok) {
			// Expect directory
			if (!(cur_inode.mode & 0x4000))
				return EXT2_ERR_INVALID;  // Not a directory
		} else {
			// Last component -- should be file (or directory, we allow both)
			out_file->inode = cur_inode;
			out_file->pos   = 0;
			return EXT2_OK;
		}

		tok = next_tok;
	}
	return EXT2_ERR_INVALID;
}

static int
    kread_inode (kuint32_t ino, ext2_inode_t *out) {
	if (ino == 0)
		return EXT2_ERR_INVALID;

	kuint32_t idx0  = ino - 1;
	kuint32_t group = idx0 / g_superblock.inodes_per_group;
	kuint32_t index = idx0 % g_superblock.inodes_per_group;

	if (!g_group_desc)
		return EXT2_ERR_INVALID;

	kuint32_t inode_tbl_blk = g_group_desc[group].inode_table;
	kuint32_t inode_size    = g_superblock.inode_size ? g_superblock.inode_size : 128;

	kuint64_t offset       = (kuint64_t) index * inode_size;
	kuint32_t blk_offset   = (kuint32_t) (offset / g_block_size);
	kuint32_t off_in_block = (kuint32_t) (offset % g_block_size);

	kuint8_t *tmp = kmalloc(g_block_size);
	if (!tmp)
		return EXT2_ERR_IO;

	// Read the first block that contains (part of) the inode
	if (kread_block(inode_tbl_blk + blk_offset, tmp) != 0) {
		kfree(tmp);
		return EXT2_ERR_IO;
	}

	kuint32_t bytes_first = g_block_size - off_in_block;
	kuint32_t need        = sizeof(ext2_inode_t);

	if (bytes_first >= need) {
		// Inode fits entirely in this block
		kmemcpy(out, tmp + off_in_block, need);
	} else {
		// Copy first portion
		kmemcpy(out, tmp + off_in_block, bytes_first);

		// Read next block for the remaining bytes
		if (kread_block(inode_tbl_blk + blk_offset + 1, tmp) != 0) {
			kfree(tmp);
			return EXT2_ERR_IO;
		}
		kmemcpy((kuint8_t *) out + bytes_first, tmp, need - bytes_first);
	}

	kfree(tmp);
	return EXT2_OK;
}

static void
    kprocess_dir_block (kuint8_t *block_buf, kuint32_t block_size, ext2_list_cb_t cb) {
	kuint32_t off = 0;
	while (off < block_size) {
		ext2_dir_entry_t *ent     = (ext2_dir_entry_t *) (block_buf + off);
		kuint16_t         rec_len = ent->rec_len;
		kuint8_t          nlen    = ent->name_len;
		if (rec_len < 8 || rec_len > block_size - off)
			break;
		if (ent->inode != 0 && nlen == 0)
			break;
		kuint16_t min_len = (kuint16_t) (((8 + nlen + 3) / 4) * 4);
		if (rec_len < min_len)
			break;
		if (ent->inode != 0 && nlen > 0) {
			char name[256] = {0};
			kmemcpy(name, ent->name, nlen);
			name[nlen] = '\0';
			cb(name, ent->file_type);
		}
		off += rec_len;
	}
}

static int
    klist_dir_entries (const ext2_inode_t *dir_inode, ext2_list_cb_t cb) {
	if (!dir_inode || !cb)
		return EXT2_ERR_INVALID;

	if (!(dir_inode->mode & 0x4000))  // not a directory
		return EXT2_ERR_INVALID;

	kuint8_t *block_buf = kmalloc(g_block_size);
	if (!block_buf)
		return EXT2_ERR_IO;

	for (int i = 0; i < 12; i++) {
		kuint32_t blk_id = dir_inode->block[i];
		if (blk_id == 0)
			continue;
		if (kread_block(blk_id, block_buf) != 0) {
			kfree(block_buf);
			return EXT2_ERR_IO;
		}
		kprocess_dir_block(block_buf, g_block_size, cb);
	}
	kfree(block_buf);
	return EXT2_OK;
}

int
    ext2_list (const char *path, ext2_list_cb_t cb) {
	if (!path || !cb)
		return EXT2_ERR_INVALID;

	// Special case: root directory
	if (kstrcmp(path, "/") == 0 || path[0] == '\0') {
		ext2_inode_t root;
		if (kread_inode(EXT2_ROOT_INODE, &root) != EXT2_OK)
			return EXT2_ERR_IO;
		if (!(root.mode & 0x4000))
			return EXT2_ERR_INVALID;
		return klist_dir_entries(&root, cb);
	}

	char    tmp[256];
	ksize_t plen = (ksize_t) kstrlen(path);
	if (plen >= sizeof(tmp))
		return EXT2_ERR_INVALID;
	kstrcpy(tmp, path[0] == '/' ? path + 1 : path);

	ext2_inode_t cur_inode;
	if (kread_inode(EXT2_ROOT_INODE, &cur_inode) != EXT2_OK)
		return EXT2_ERR_IO;

	char *tok = kstrtok(tmp, "/");
	char *next_tok;
	while (tok) {
		next_tok = kstrtok(KNULL, "/");

		kuint32_t found_ino = 0;
		kuint8_t *blk_buf   = kmalloc(g_block_size);
		if (!blk_buf)
			return EXT2_ERR_IO;

		for (int i = 0; i < 12 && found_ino == 0; i++) {
			kuint32_t blk = cur_inode.block[i];
			if (blk == 0)
				continue;
			if (kread_block(blk, blk_buf) != 0) {
				kfree(blk_buf);
				return EXT2_ERR_IO;
			}
			kuint32_t off = 0;
			while (off < g_block_size) {
				ext2_dir_entry_t *ent =
				    (ext2_dir_entry_t *) (blk_buf + off);
				if (ent->rec_len == 0)
					break;
				if (ent->inode != 0
				    && ent->name_len == (kuint8_t) kstrlen(tok)
				    && kmemcmp(ent->name, tok, ent->name_len) == 0) {
					found_ino = ent->inode;
					break;
				}
				off += ent->rec_len;
			}
		}
		kfree(blk_buf);
		if (found_ino == 0)
			return EXT2_ERR_PATH_NOT_FOUND;

		if (kread_inode(found_ino, &cur_inode) != EXT2_OK)
			return EXT2_ERR_IO;

		if (next_tok) {
			if (!(cur_inode.mode & 0x4000))
				return EXT2_ERR_INVALID;
		} else {
			if (!(cur_inode.mode & 0x4000))
				return EXT2_ERR_INVALID;
			return klist_dir_entries(&cur_inode, cb);
		}

		tok = next_tok;
	}
	return EXT2_ERR_INVALID;
}

int
    ext2_list_dir (ext2_list_cb_t cb) {
	if (!cb)
		return EXT2_ERR_INVALID;

	ext2_inode_t root;
	if (kread_inode(EXT2_ROOT_INODE, &root) != EXT2_OK)
		return EXT2_ERR_IO;

	if (!(root.mode & 0x4000))  // not a directory
		return EXT2_ERR_INVALID;

	kuint8_t *block_buf = kmalloc(g_block_size);
	if (!block_buf)
		return EXT2_ERR_IO;

	// Direct blocks
	for (int i = 0; i < 12; i++)  // Only direct blocks for now
	{
		kuint32_t blk_id = root.block[i];
		if (blk_id == 0)
			continue;
		if (kread_block(blk_id, block_buf) != 0) {
			kfree(block_buf);
			return EXT2_ERR_IO;
		}
		kprocess_dir_block(block_buf, g_block_size, cb);
	}
	kfree(block_buf);
	return EXT2_OK;
}

// Helper function to get the physical block ID for a logical block within a file
static kuint32_t
    kget_file_block (const ext2_inode_t *inode, kuint64_t logical_block) {
	kuint32_t blocks_per_indirect = g_block_size / sizeof(kuint32_t);

	// Direct blocks (0-11)
	if (logical_block < 12) {
		return inode->block[logical_block];
	}

	// Single indirect blocks (12 to 12 + blocks_per_indirect - 1)
	kuint64_t single_indirect_start = 12;
	kuint64_t single_indirect_end   = single_indirect_start + blocks_per_indirect;

	if (logical_block < single_indirect_end) {
		kuint32_t indirect_block = inode->block[12];
		if (indirect_block == 0)
			return 0;  // Sparse

		// Read the indirect block
		kuint32_t *indirect_buf = (kuint32_t *) kmalloc(g_block_size);
		if (!indirect_buf)
			return 0;

		if (kread_block(indirect_block, indirect_buf) != 0) {
			kfree(indirect_buf);
			return 0;
		}

		kuint32_t index  = (kuint32_t) (logical_block - single_indirect_start);
		kuint32_t result = indirect_buf[index];
		kfree(indirect_buf);
		return result;
	}

	// Double indirect blocks
	kuint64_t double_indirect_start = single_indirect_end;
	kuint64_t double_indirect_end =
	    double_indirect_start + (kuint64_t) blocks_per_indirect * blocks_per_indirect;

	if (logical_block < double_indirect_end) {
		kuint32_t double_indirect_block = inode->block[13];
		if (double_indirect_block == 0)
			return 0;  // Sparse

		// Calculate which single indirect block we need
		kuint64_t offset_in_double = logical_block - double_indirect_start;
		kuint32_t single_indirect_index =
		    (kuint32_t) (offset_in_double / blocks_per_indirect);
		kuint32_t data_block_index =
		    (kuint32_t) (offset_in_double % blocks_per_indirect);

		// Read the double indirect block to get the single indirect block pointer
		kuint32_t *double_indirect_buf = (kuint32_t *) kmalloc(g_block_size);
		if (!double_indirect_buf)
			return 0;

		if (kread_block(double_indirect_block, double_indirect_buf) != 0) {
			kfree(double_indirect_buf);
			return 0;
		}

		kuint32_t single_indirect_block =
		    double_indirect_buf[single_indirect_index];
		kfree(double_indirect_buf);

		if (single_indirect_block == 0)
			return 0;  // Sparse

		// Now read the single indirect block to get the data block pointer
		kuint32_t *single_indirect_buf = (kuint32_t *) kmalloc(g_block_size);
		if (!single_indirect_buf)
			return 0;

		if (kread_block(single_indirect_block, single_indirect_buf) != 0) {
			kfree(single_indirect_buf);
			return 0;
		}

		kuint32_t result = single_indirect_buf[data_block_index];
		kfree(single_indirect_buf);
		return result;
	}

	// Triple indirect blocks
	kuint64_t triple_indirect_start = double_indirect_end;
	kuint64_t triple_indirect_end =
	    triple_indirect_start
	    + (kuint64_t) blocks_per_indirect * blocks_per_indirect * blocks_per_indirect;

	if (logical_block < triple_indirect_end) {
		kuint32_t triple_indirect_block = inode->block[14];
		if (triple_indirect_block == 0)
			return 0;  // Sparse

		// Calculate indices for triple indirection
		kuint64_t offset_in_triple = logical_block - triple_indirect_start;
		kuint32_t double_indirect_index =
		    (kuint32_t) (offset_in_triple
		                 / (blocks_per_indirect * blocks_per_indirect));
		kuint32_t single_indirect_index =
		    (kuint32_t) ((offset_in_triple
		                  % (blocks_per_indirect * blocks_per_indirect))
		                 / blocks_per_indirect);
		kuint32_t data_block_index =
		    (kuint32_t) (offset_in_triple % blocks_per_indirect);

		// Read the triple indirect block to get the double indirect block pointer
		kuint32_t *triple_indirect_buf = (kuint32_t *) kmalloc(g_block_size);
		if (!triple_indirect_buf)
			return 0;

		if (kread_block(triple_indirect_block, triple_indirect_buf) != 0) {
			kfree(triple_indirect_buf);
			return 0;
		}

		kuint32_t double_indirect_block =
		    triple_indirect_buf[double_indirect_index];
		kfree(triple_indirect_buf);

		if (double_indirect_block == 0)
			return 0;  // Sparse

		// Read the double indirect block to get the single indirect block pointer
		kuint32_t *double_indirect_buf = (kuint32_t *) kmalloc(g_block_size);
		if (!double_indirect_buf)
			return 0;

		if (kread_block(double_indirect_block, double_indirect_buf) != 0) {
			kfree(double_indirect_buf);
			return 0;
		}

		kuint32_t single_indirect_block =
		    double_indirect_buf[single_indirect_index];
		kfree(double_indirect_buf);

		if (single_indirect_block == 0)
			return 0;  // Sparse

		// Finally read the single indirect block to get the data block pointer
		kuint32_t *single_indirect_buf = (kuint32_t *) kmalloc(g_block_size);
		if (!single_indirect_buf)
			return 0;

		if (kread_block(single_indirect_block, single_indirect_buf) != 0) {
			kfree(single_indirect_buf);
			return 0;
		}

		kuint32_t result = single_indirect_buf[data_block_index];
		kfree(single_indirect_buf);
		return result;
	}

	// Fuck you if you want more than triple indirect blocks.
	return 0;
}

int
    ext2_read (ext2_file_t *file, void *buf, ksize_t len) {
	if (!file || !buf)
		return EXT2_ERR_INVALID;

	// Clamp length to remaining bytes in file
	kuint64_t size = (kuint64_t) file->inode.size_lo
	                 | ((kuint64_t) file->inode.dir_acl_or_size_high << 32);
	if (file->pos >= size)
		return 0;  // EOF
	if (file->pos + len > size)
		len = size - file->pos;

	kuint8_t *dst       = (kuint8_t *) buf;
	ksize_t   remaining = len;

	kuint8_t *block_buf = kmalloc(g_block_size);
	if (!block_buf)
		return EXT2_ERR_IO;

	while (remaining > 0) {
		kuint64_t block_idx    = file->pos / g_block_size;
		kuint32_t off_in_block = (kuint32_t) (file->pos % g_block_size);

		// Calculate maximum supported blocks
		kuint32_t blocks_per_indirect = g_block_size / sizeof(kuint32_t);
		kuint64_t max_blocks =
		    12 + blocks_per_indirect
		    + (kuint64_t) blocks_per_indirect * blocks_per_indirect
		    + (kuint64_t) blocks_per_indirect * blocks_per_indirect
		          * blocks_per_indirect;
		// Direct + single indirect + double indirect + triple indirect
		if (block_idx >= max_blocks) {
			kfree(block_buf);
			return EXT2_ERR_UNSUPPORTED;
		}

		kuint32_t blk_id = kget_file_block(&file->inode, block_idx);
		if (blk_id == 0) {
			// Sparse region -- treat as zero
			ksize_t chunk = g_block_size - off_in_block;
			if (chunk > remaining)
				chunk = remaining;
			kmemset(dst, 0, chunk);
			dst += chunk;
			file->pos += chunk;
			remaining -= chunk;
			continue;
		}
		if (kread_block(blk_id, block_buf) != 0) {
			kfree(block_buf);
			return EXT2_ERR_IO;
		}
		ksize_t chunk = g_block_size - off_in_block;
		if (chunk > remaining)
			chunk = remaining;
		kmemcpy(dst, block_buf + off_in_block, chunk);
		dst += chunk;
		file->pos += chunk;
		remaining -= chunk;
	}
	kfree(block_buf);
	return (int) len;
}

struct Ext2 ext2 = {.set_block_device = ext2_set_block_device,
                    .mount            = ext2_mount,
                    .open             = ext2_open,
                    .read             = ext2_read,
                    .list             = ext2_list,
                    .list_dir         = ext2_list_dir};
