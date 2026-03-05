/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright (c) 2026, "뿌댕이"
 * Project: ToolOS
 * File: Bootinfo.h
 * * Description:
 * ToolOS Kernel structure
 * 
*/

#ifndef Bootinfo_h
#define Bootinfo_h

#include <stdint.h>
// 8 + 8 + 8 + 8 = 32byte
typedef struct {
  uint64_t PhysicalStart;
  uint64_t NumberOfPages;
  uint64_t Type;
  uint64_t TotalNums;
} TOOLOS_MEMORYMAP_INFO;

// 8 + 8 + 4 + 4 + 4 + 4 = 32byte
typedef struct  {
  uint64_t FrameBufferBase;
  uint64_t FrameBufferSize;
  uint32_t VerticalResolution;
  uint32_t HorizontalResolution;
  uint32_t Reserved;
  uint32_t PixelsPerScanLine; 
} TOOLOS_GRAPHICS_INFO;
#endif