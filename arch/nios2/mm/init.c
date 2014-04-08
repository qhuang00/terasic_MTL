/*
 * Copyright (C) 2010 Tobias Klauser <tklauser@distanz.ch>
 * Copyright (C) 2009 Wind River Systems Inc
 *   Implemented by fredrik.markstrom@gmail.com and ivarholmqvist@gmail.com
 * Copyright (C) 2004 Microtronix Datacom Ltd
 *
 * based on arch/m68k/mm/init.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/pagemap.h>
#include <linux/bootmem.h>
#include <linux/slab.h>

#include <asm/setup.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/sections.h>
#include <asm/tlb.h>
#include <asm/mmu_context.h>
#include <asm/cpuinfo.h>

#ifdef CONFIG_MMU
pgd_t *pgd_current;
#endif

/*
 * paging_init() continues the virtual memory environment setup which
 * was begun by the code in arch/head.S.
 * The parameters are pointers to where to stick the starting and ending
 * addresses of available kernel virtual memory.
 */
void __init paging_init(void)
{
	unsigned long zones_size[MAX_NR_ZONES];
	unsigned long start_mem, end_mem;

	memset(zones_size, 0, sizeof(zones_size));

	/*
	 * Make sure start_mem is page aligned, otherwise bootmem and
	 * page_alloc get different views of the world.
	 */
#ifdef CONFIG_MMU
	start_mem = PHYS_OFFSET;
	end_mem   = memory_end;
#else
	start_mem = PAGE_ALIGN(memory_start);
	end_mem   = memory_end & PAGE_MASK;
#endif /* CONFIG_MMU */

#ifdef CONFIG_MMU
	pagetable_init();
	pgd_current = swapper_pg_dir;
#endif

	/*
	 * Set up SFC/DFC registers (user data space).
	 */
#ifndef CONFIG_MMU
	set_fs(KERNEL_DS);
#endif

#ifdef CONFIG_MMU
	zones_size[ZONE_DMA] = ((end_mem - start_mem) >> PAGE_SHIFT);
#else
	zones_size[ZONE_DMA] = (end_mem - PAGE_OFFSET) >> PAGE_SHIFT;
#endif /* CONFIG_MMU */

	/* pass the memory from the bootmem allocator to the main allocator */
	free_area_init(zones_size);
}

void __init mem_init(void)
{
	unsigned long end_mem   = memory_end; /* this must not include
						kernel stack at top */

	pr_debug("mem_init: start=%lx, end=%lx\n", memory_start, memory_end);

	end_mem &= PAGE_MASK;
	high_memory = __va(end_mem);

#ifdef CONFIG_MMU
	max_mapnr = ((unsigned long)end_mem) >> PAGE_SHIFT;
#else
	max_mapnr = (((unsigned long)high_memory) - PAGE_OFFSET) >> PAGE_SHIFT;
#endif /* CONFIG_MMU */

	/* this will put all memory onto the freelists */
	free_all_bootmem();
	mem_init_print_info(NULL);
}

#ifdef CONFIG_MMU
void __init mmu_init(void)
{
	flush_tlb_all();
}
#endif

#ifdef CONFIG_BLK_DEV_INITRD
void __init free_initrd_mem(unsigned long start, unsigned long end)
{
	free_reserved_area((void*)start, (void*)end, -1, "initrd");
}
#endif

void __init_refok free_initmem(void)
{
	free_initmem_default(-1);
}

#ifdef CONFIG_MMU

#define __page_aligned(order) __aligned(PAGE_SIZE << (order))
pgd_t swapper_pg_dir[PTRS_PER_PGD] __page_aligned(PGD_ORDER);
pte_t invalid_pte_table[PTRS_PER_PTE] __page_aligned(PTE_ORDER);

#endif /* CONFIG_MMU */
