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
 * \file uffs_public.h
 * \brief flash interface for UFFS
 * \author Ricky Zheng
 */

#ifndef _UFFS_FLASH_H_
#define _UFFS_FLASH_H_

#include "uffs/uffs_types.h"
#include "uffs/uffs_config.h"
#include "uffs/uffs_core.h"
#include "uffs/uffs_device.h"

#ifdef __cplusplus
extern "C"{
#endif


/** ECC options (uffs_StorageAttrSt.ecc_opt) */
#define UFFS_ECC_NONE		0	//!< do not use ECC
#define UFFS_ECC_SOFT		1	//!< UFFS calculate the ECC
#define UFFS_ECC_HW			2	//!< Flash driver(or by hardware) calculate the ECC
#define UFFS_ECC_HW_AUTO	3	//!< Hardware calculate the ECC and automatically write to spare.


/** spare layout options (uffs_StorageAttrSt.layout_opt) */
#define UFFS_LAYOUT_UFFS	0	//!< do layout by dev->attr information
#define UFFS_LAYOUT_FLASH	1	//!< flash driver do the layout


/** 
 * \struct uffs_StorageAttrSt
 * \brief uffs device storage attribute, provide by nand specific file
 */
struct uffs_StorageAttrSt {
	u32 total_blocks;		//!< total blocks in this chip
	u16 page_data_size;		//!< page data size (physical page data size, e.g. 512)
	u16 spare_size;			//!< page spare size (physical page spare size, e.g. 16)
	u16 pages_per_block;	//!< pages per block
	u16 block_status_offs;	//!< block status byte offset in spare
	int ecc_opt;			//!< ecc option ( #UFFS_ECC_[NONE|SOFT|HW|HW_AUTO] )
	int layout_opt;			//!< layout option
	const u8 *ecc_layout;	//!< page data ECC layout: [ofs1, size1, ofs2, size2, ..., 0xFF, 0]
	const u8 *s_ecc_layout;	//!< spare data ECC layout: [ofs1, size1, ofs2, size2, ..., 0xFF, 0]
	const u8 *data_layout;	//!< spare data layout: [ofs1, size1, ofs2, size2, ..., 0xFF, 0]
	void *private;			//!< private data for storage attribute
};


/**
 * \struct uffs_FlashOpsSt 
 * \brief lower level flash operations, should be implement in flash driver
 */
struct uffs_FlashOpsSt {
	/**
	 * Read page data.
	 * 
	 * if ecc_opt is UFFS_ECC_HW, flash driver must calculate and return ecc (if ecc != NULL).
	 *
	 * if ecc_opt is UFFS_ECC_HW or UFFS_ECC_HW_AUTO, flash driver do ecc
	 * correction with stored ecc in spare area.
	 *
	 * if ecc_opt is UFFS_ECC_HW_AUTO, not neccessary return ecc.
	 *
	 * \return 0: success and/or has no flip bits, otherwise:
	 *		-1: I/O error, expect retry ?
	 *		-2: page data has flip bits and ecc correct failed.
	 *		>0: page data has flip bits and corrected by ecc.
	 *
	 * \note pad 0xFF for calculating ECC if len < page_data_size
	 */
	int (*ReadPageData)(uffs_Device *dev, u32 block, u32 page, u8 *data, int len, u8 *ecc);


	/**
	 * Read page spare.
	 *
	 * \note flash driver must privide this function when layout_opt is UFFS_LAYOUT_UFFS.
	 *
	 * \return 0: success and/or has no flip bits, otherwise:
	 *		-1: I/O error, expect retry ?
	 *		-2: spare data has flip bits and can't be corrected by ecc.
	 *		>0: spare data has flip bit and corrected by ECC.
	 */
	int (*ReadPageSpare)(uffs_Device *dev, u32 block, u32 page, u8 *spare, int len);

	/**
	 * Read page spare and unload to tag.
	 *
	 * \note flash driver must provide this function if layout_opt is UFFS_LAYOUT_FLASH.
	 *
	 * \return 0: success and/or has no flip bits, otherwise:
	 *		-1: I/O error, expect retyr ?
	 *		-2: spare data has flip bits and can't be corrected by ecc.
	 *		>0: spare data has flip bit and corrected by ECC.
	 */
	int (*ReadPageSpareLayout)(uffs_Device *dev, u32 block, u32 page, u8 *tag, int len, u8 *ecc);

	/**
	 * Write page data.
	 *
	 * if ecc_opt is UFFS_ECC_HW, flash driver must calculate and return the ecc.
	 * if ecc_opt is UFFS_ECC_HW_AUTO, do not need to return ecc.
	 *
	 * \return 0: success,
	 *		  -1: I/O error, expect retry ?
	 *		  -2: a bad block detected
	 *
	 * \note pad 0xFF for calculating ECC if len < page_data_size
	 */
	int (*WritePageData)(uffs_Device *dev, u32 block, u32 page, const u8 *data, int len, u8 *ecc);


	/**
	 * Write page spare.
	 *
	 * \note flash driver must privide this function when layout_opt is UFFS_LAYOUT_UFFS.
	 *
	 * \return 0: success
	 *		-1: I/O error, expect retry ?
	 *		-2: a bad block detected
	 */
	int (*WritePageSpare)(uffs_Device *dev, u32 block, u32 page, const u8 *spare, int len);

	/**
	 * Write page spare, flash driver do the layout.
	 *
	 * \note flash driver must provide this function if layout_opt is UFFS_LAYOUT_FLASH.
	 *
	 * \return 0: success
	 *		-1: I/O error, expect retyr ?
	 *		-2: a bad block is detected
	 */
	int (*WritePageSpareLayout)(uffs_Device *dev, u32 block, u32 page, const u8 *tag, int len, const u8 *ecc);

	/**
	 * check block status.
	 *
	 * \note flash driver may maintain a bad block table to speed up bad block checking or
	 *		it will require one or two read spare I/O to check block status.
	 *
	 * \note if this function is not provided, UFFS check the block_status byte in spare.
	 *
	 * \return 1 if it's a bad block, 0 if it's not.
	 */
	int (*IsBadBlock)(uffs_Device *dev, u32 block);

	/**
	 * Mark a new bad block.
	 *
	 * \return 0 if success, otherwise return -1.
	 */
	int (*MarkBadBlock)(uffs_Device *dev, u32 block);

	/**
	 * Erase a block.
	 *
	 * \return 0: erase success
	 *		  -1: a bad block detected.
	 *		  -2: unknown error, probably expect a retry
	 */
	int (*EraseBlock)(uffs_Device *dev, u32 block);
};


/** read page spare, fill tag and ECC */
URET uffs_FlashReadPageSpare(uffs_Device *dev, int block, int page, uffs_Tags *tag, u8 *ecc);

/** read page data to page buf and do ECC correct */
URET uffs_FlashReadPage(uffs_Device *dev, int block, int page, uffs_Buf *buf);

/** write page data and spare */
URET uffs_FlashWritePageCombine(uffs_Device *dev, int block, int page, uffs_Buf *buf, uffs_Tags *tag);

/** Mark this block as bad block */
URET uffs_FlashMarkBadBlock(uffs_Device *dev, int block);

/** Is this block a bad block ? */
UBOOL uffs_FlashIsBadBlock(uffs_Device *dev, int block);

/** Erase flash block */
URET uffs_FlashEraseBlock(uffs_Device *dev, int block);


#ifdef __cplusplus
}
#endif
#endif
