#ifdef DYNAREC

#ifdef __amd64__
#include "codegen_x86-64.h"
#elif defined i386 || defined __i386 || defined __i386__ || defined _X86_ || defined WIN32 || defined _WIN32 || defined _WIN32
#include "codegen_x86.h"
#else
#error Dynamic recompiler not implemented on your platform
#endif

/*Handling self-modifying code (of which there is a lot on x86) :

  PCem tracks a 'dirty mask' for each physical page, in which each bit
  represents 64 bytes. This is only tracked for pages that have code in - when a
  page first has a codeblock generated, it is evicted from the writelookup and
  added to the page_lookup for this purpose. When in the page_lookup, each write
  will go through the mem_write_ram*_page() functions and set the dirty mask
  appropriately.
  
  Each codeblock also contains a code mask (actually two masks, one for each
  page the block is/may be in), again with each bit representing 64 bytes.
  
  Each page has a list of codeblocks present in it. As each codeblock can span
  up to two pages, two lists are present.
  
  When a codeblock is about to be executed, the code masks are compared with the
  dirty masks for the relevant pages. If either intersect, then
  codegen_check_flush() is called on the affected page(s), and all affected
  blocks are evicted.
  
  The 64 byte granularity appears to work reasonably well for most cases,
  avoiding most unnecessary evictions (eg when code & data are stored in the
  same page).
*/

typedef struct codeblock_t
{
        /*Previous and next pointers, for the codeblock list associated with
          each physical page. Two sets of pointers, as a codeblock can be
          present in two pages.*/
        struct codeblock_t *prev, *next;
        struct codeblock_t *prev_2, *next_2;
        
        uint32_t pc;
        uint32_t _cs;
        uint32_t endpc;
        uint32_t phys, phys_2;
        uint32_t use32;
        int stack32;
        int pnt;
        int ins;
        uint64_t page_mask, page_mask2;
        
        uint8_t data[2048];
} codeblock_t;

#define PAGE_MASK_MASK 63
#define PAGE_MASK_SHIFT 6

extern codeblock_t *codeblock;

extern codeblock_t **codeblock_hash;

void codegen_init();
void codegen_reset();
void codegen_block_init(uint32_t phys_addr);
void codegen_block_remove();
void codegen_generate_call(uint8_t opcode, OpFn op, uint32_t fetchdat, uint32_t new_pc, uint32_t old_pc);
void codegen_generate_seg_restore();
void codegen_check_abrt();
void codegen_set_op32();
void codegen_flush();
void codegen_check_flush(struct page_t *page, uint64_t mask, uint32_t phys_addr);

extern int cpu_block_end;

extern int cpu_recomp_blocks, cpu_recomp_ins, cpu_recomp_full_ins, cpu_new_blocks;
extern int cpu_recomp_blocks_latched, cpu_recomp_ins_latched, cpu_recomp_full_ins_latched, cpu_new_blocks_latched;
extern int cpu_recomp_flushes, cpu_recomp_flushes_latched;
extern int cpu_recomp_evicted, cpu_recomp_evicted_latched;
extern int cpu_recomp_reuse, cpu_recomp_reuse_latched;
extern int cpu_recomp_removed, cpu_recomp_removed_latched;

extern int cpu_reps, cpu_reps_latched;
extern int cpu_notreps, cpu_notreps_latched;

extern int codegen_block_cycles;

extern void (*codegen_timing_start)();
extern void (*codegen_timing_prefix)(uint8_t prefix, uint32_t fetchdat);
extern void (*codegen_timing_opcode)(uint8_t opcode, uint32_t fetchdat, int op_32);
extern void (*codegen_timing_block_start)();
extern void (*codegen_timing_block_end)();

typedef struct codegen_timing_t
{
        void (*start)();
        void (*prefix)(uint8_t prefix, uint32_t fetchdat);
        void (*opcode)(uint8_t opcode, uint32_t fetchdat, int op_32);
        void (*block_start)();
        void (*block_end)();
} codegen_timing_t;

extern codegen_timing_t codegen_timing_pentium;
extern codegen_timing_t codegen_timing_486;

void codegen_timing_set(codegen_timing_t *timing);

extern int block_current;
extern int block_pos;

#define CPU_BLOCK_END() cpu_block_end = 1

static inline void addbyte(uint8_t val)
{
        codeblock[block_current].data[block_pos++] = val;
        if (block_pos >= 1760)
        {
                CPU_BLOCK_END();
        }
}

static inline void addword(uint16_t val)
{
        *(uint16_t *)&codeblock[block_current].data[block_pos] = val;
        block_pos += 2;
        if (block_pos >= 1760)
        {
                CPU_BLOCK_END();
        }
}

static inline void addlong(uint32_t val)
{
        *(uint32_t *)&codeblock[block_current].data[block_pos] = val;
        block_pos += 4;
        if (block_pos >= 1760)
        {
                CPU_BLOCK_END();
        }
}

static inline void addquad(uint64_t val)
{
        *(uint64_t *)&codeblock[block_current].data[block_pos] = val;
        block_pos += 8;
        if (block_pos >= 1760)
        {
                CPU_BLOCK_END();
        }
}

/*Current physical page of block being recompiled. -1 if no recompilation taking place */
extern uint32_t recomp_page;

extern x86seg *op_ea_seg;
extern int op_ssegs;
extern uint32_t op_old_pc;

/*Set to 1 if flags have been changed in the block being recompiled, and hence
  flags_op is known and can be relied on */
extern int codegen_flags_changed;

extern int codegen_fpu_entered;

extern int codegen_fpu_loaded_iq[8];
extern int codegen_reg_loaded[8];

#endif
