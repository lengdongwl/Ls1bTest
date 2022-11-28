/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cpu.h"

extern void resettlb(int i);

void init_tlb(void)
{
	int i;
	for (i = 0; i < N_TLB_ENTRIES; i++)
		resettlb(i);
}

