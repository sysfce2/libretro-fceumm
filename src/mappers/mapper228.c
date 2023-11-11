/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2012 CaH4e3
 *  Copyright (C) 2023
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 Mapper 228:
 Action 52 is highly uncommon in that its PRG ROM has a non-power-of-two ROM size: three 512 KiB PRG ROMs alongside one
 512 KiB CHR ROM.

 It is claimed that there are four 4-bit RAM locations at $4020-$4023, mirrored throughout $4020-$5FFF. This 16-bit RAM
 is definitely not present on either cartridge, Nestopia does not implement it at all, and neither cartridge ever writes
 to these addresses.
 */

#include "mapinc.h"
#include "latch.h"

static uint8 *prgromData;
static uint32 prgromSize;

static void Sync(void) {
	if (latch.addr & 0x20) {
		setprg16(0x8000, (latch.addr >> 6) & 0x7F);
		setprg16(0xC000, (latch.addr >> 6) & 0x7F);
	} else {
		setprg32(0x8000, (latch.addr >> 7) & 0x3F);
	}
	setchr8(((latch.addr << 2) & 0x3C) | (latch.data & 0x3));
	setmirror(((latch.addr >> 13) & 0x01) ^ 0x01);
}

static void M228Close(void) {
	Latch_Close();
	if (prgromData) {
		FCEU_gfree(prgromData);
		prgromData = NULL;
		prgromSize = 0;
	}
}

void Mapper228_Init(CartInfo *info) {
	Latch_Init(info, Sync, NULL, 0, 0);
	info->Close = M228Close;
	if ((ROM.prg.size * 16 * 1024) == 0x180000) {
		int i;
		prgromSize = 0x200000;
		prgromData = (uint8 *)FCEU_gmalloc(prgromSize);
		for (i = 0; i < (ROM.prg.size * 16 * 1024); i++) {
			prgromData[i] = ROM.prg.data[i];
		}
		for (i = 0x000000; i < 0x080000; i++) {
			prgromData[0x180000 + i] = prgromData[0x100000 + i];
			prgromData[0x100000 + i] = (i >> 8) & 0xFF;
		}
		SetupCartPRGMapping(0, prgromData, prgromSize, 0);
	}
}
