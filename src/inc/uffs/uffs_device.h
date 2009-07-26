/*
  This file is part of UFFS, the Ultra-low-cost Flash File System.
  
  Copyright (C) 2005-2009 Ricky Zheng <ricky_gz_zheng@yahoo.co.nz>

  UFFS is free software; you can redistribute it and/or modify it under
  the GNU Library General Public License as published by the Free Software 
  Foundation; either version 2 of the License, or (at your option) any
  later version.

  UFFS is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  or GNU Library General Public License, as applicable, for more details.
 
  You should have received a copy of the GNU General Public License
  and GNU Library General Public License along with UFFS; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA  02110-1301, USA.

  As a special exception, if other files instantiate templates or use
  macros or inline functions from this file, or you compile this file
  and link it with other works to produce a work based on this file,
  this file does not by itself cause the resulting work to be covered
  by the GNU General Public License. However the source code for this
  file must still be made available in accordance with section (3) of
  the GNU General Public License v2.
 
  This exception does not invalidate any other reasons why a work based
  on this file might be covered by the GNU General Public License.
*/

/** 
 * \file uffs_device.h
 * \brief uffs device structures definition
 * \author Ricky Zheng
 */

#ifndef UFFS_DEVICE_H
#define UFFS_DEVICE_H


#include "uffs/uffs_types.h"
#include "uffs/uffs_config.h"
#include "uffs/uffs_buf.h"
#include "uffs/uffs_blockinfo.h"
#include "uffs/uffs_pool.h"
#include "uffs/uffs_tree.h"
#include "uffs/uffs_mem.h"
#include "uffs/uffs_core.h"
#include "uffs/uffs_flash.h"

#ifdef __cplusplus
extern "C"{
#endif


/** UFFS device type: uffs_DeviceSt.dev_type */
#define UFFS_DEV_NULL		0
#define UFFS_DEV_NAND		1
#define UFFS_DEV_SM			2
#define UFFS_DEV_RAM		3
#define UFFS_DEV_ROM		4
#define UFFS_DEV_EMU		5


/** 
 * \struct uffs_BlockInfoCacheSt
 * \brief block information structure, used to manager block information caches
 */
struct uffs_BlockInfoCacheSt {
	uffs_BlockInfo *head;			//!< buffer head of block info(spares)
	uffs_BlockInfo *tail;			//!< buffer tail
	void *mem_pool;					//!< internal memory pool, used for release whole buffer
};

/** 
 * \struct uffs_PartitionSt
 * \brief partition basic information
 */
struct uffs_PartitionSt {
	u16 start;		//!< start block number of partition
	u16 end;		//!< end block number of partiton
};

/** 
 * \struct uffs_LockSt
 * \brief lock stuffs
 */
struct uffs_LockSt {
	u32 sem;
	u32 task_id;
	int counter;
};

/** 
 * \struct uffs_DirtyGroupSt
 * \brief manager dirty page buffers
 */
struct uffs_DirtyGroupSt {
	int count;					//!< dirty buffers count
	uffs_Buf *dirty;			//!< dirty buffer list
};

/** 
 * \struct uffs_PageBufDescSt
 * \brief uffs page buffers descriptor
 */
struct uffs_PageBufDescSt {
	uffs_Buf *head;			//!< head of buffers
	uffs_Buf *tail;			//!< tail of buffers
	struct uffs_DirtyGroupSt dirtyGroup[MAX_DIRTY_BUF_GROUPS];	//!< dirty buffer groups
	int buf_max;			//!< maximum buffers
	int dirty_buf_max;		//!< maximum dirty buffer allowed
	void *pool;				//!< memory pool for buffers
};


/** 
 * \struct uffs_PageCommInfoSt
 * \brief common data for device, should be initialized at early
 * \note it is possible that pg_size is smaller than physical page size, but normally they are the same
 */
struct uffs_PageCommInfoSt {
	u32 pg_data_size;			//!< page data size
	u32 ecc_size;				//!< ecc size
	u32 pg_size;				//!< page size
};

/** 
 * \struct uffs_NewBadBlockSt
 * \brief holding new discovered bad block
 */
struct uffs_NewBadBlockSt {
	u16 block;				//!< bad block, FIX ME to process more than one bad block
};

/**
 * \struct uffs_FlashStatSt
 * \typedef uffs_FlashStat
 * \brief statistic data of flash read/write/erase activities
 */
typedef struct uffs_FlashStatSt {
	int block_erase_count;
	int page_write_count;
	int page_read_count;
	int spare_write_count;
	int spare_read_count;
} uffs_FlashStat;


/** 
 * \struct uffs_DeviceSt
 * \brief The core data structure of UFFS, all information needed by manipulate UFFS object
 * \note one partition corresponding one uffs device.
 */
struct uffs_DeviceSt {
	URET (*Init)(uffs_Device *dev);				//!< low level initialization
	URET (*Release)(uffs_Device *dev);			//!< low level release
	void *private;								//!< private data for device

	struct uffs_StorageAttrSt		*attr;		//!< storage attribute
	struct uffs_PartitionSt			par;		//!< partition information
	struct uffs_FlashOpsSt			*ops;		//!< flash operations
	struct uffs_BlockInfoCacheSt	bc;			//!< block info cache
	struct uffs_LockSt				lock;		//!< lock data structure
	struct uffs_PageBufDescSt		buf;		//!< page buffers
	struct uffs_PageCommInfoSt		com;		//!< common information
	struct uffs_TreeSt				tree;		//!< tree list of block
	struct uffs_NewBadBlockSt		bad;		//!< new bad block
	struct uffs_FlashStatSt			st;			//!< statistic (counters)
	struct uffs_memAllocatorSt		mem;		//!< uffs native memory allocator
	u32 ref_count;								//!< device reference count
};

/** create the lock for uffs device */
URET uffs_DeviceInitLock(uffs_Device *dev);

/** delete the lock of uffs device */
URET uffs_DeviceReleaseLock(uffs_Device *dev);

/** lock uffs device */
URET uffs_DeviceLock(uffs_Device *dev);

/** unlock uffs device */
URET uffs_DeviceUnLock(uffs_Device *dev);


#ifdef __cplusplus
}
#endif


#endif

