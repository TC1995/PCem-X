/*Register allocation :
        R8-R15 - emulated registers
*/

static void call(codeblock_t *block, uintptr_t func)
{
	uintptr_t diff = func - (uintptr_t)&block->data[block_pos + 5];

        codegen_reg_loaded[0] = codegen_reg_loaded[1] = codegen_reg_loaded[2] = codegen_reg_loaded[3] = 0;
        codegen_reg_loaded[4] = codegen_reg_loaded[5] = codegen_reg_loaded[6] = codegen_reg_loaded[7] = 0;

	if (diff >= -0x80000000 && diff < 0x7fffffff)
	{
	        addbyte(0xE8); /*CALL*/
	        addlong((uint32_t)diff);
	}
	else
	{
		addbyte(0x48); /*MOV RAX, func*/
		addbyte(0xb8);
		addquad(func);
		addbyte(0xff); /*CALL RAX*/
		addbyte(0xd0);
	}
}

static void call_long(uintptr_t func)
{
        codegen_reg_loaded[0] = codegen_reg_loaded[1] = codegen_reg_loaded[2] = codegen_reg_loaded[3] = 0;
        codegen_reg_loaded[4] = codegen_reg_loaded[5] = codegen_reg_loaded[6] = codegen_reg_loaded[7] = 0;

	addbyte(0x48); /*MOV RAX, func*/
	addbyte(0xb8);
	addquad(func);
	addbyte(0xff); /*CALL RAX*/
	addbyte(0xd0);
}

static void load_param_1_32(codeblock_t *block, uint32_t param)
{
#if WIN64
        addbyte(0xb9); /*MOVL $fetchdat,%ecx*/
#else
        addbyte(0xbf); /*MOVL $fetchdat,%edi*/
#endif
        addlong(param);
}
static void load_param_1_reg_32(int reg)
{
#if WIN64
        if (reg & 8)
                addbyte(0x44);
        addbyte(0x89); /*MOV ECX, EAX*/
        addbyte(0xc0 | REG_ECX | (reg << 3));
#else
        if (reg & 8)
                addbyte(0x44);
        addbyte(0x89); /*MOV EDI, EAX*/
        addbyte(0xc0 | REG_EDI | (reg << 3));
#endif
}
static void load_param_1_64(codeblock_t *block, uint64_t param)
{
	addbyte(0x48);
#if WIN64
        addbyte(0xb9); /*MOVL $fetchdat,%ecx*/
#else
        addbyte(0xbf); /*MOVL $fetchdat,%edi*/
#endif
        addquad(param);
}

static void load_param_2_32(codeblock_t *block, uint32_t param)
{
#if WIN64
        addbyte(0xba); /*MOVL $fetchdat,%edx*/
#else
        addbyte(0xbe); /*MOVL $fetchdat,%esi*/
#endif
        addlong(param);
}
static void load_param_2_reg_32(int reg)
{
#if WIN64
        if (reg & 8)
                addbyte(0x44);
        addbyte(0x89); /*MOV EDX, EAX*/
        addbyte(0xc0 | REG_EDX | (reg << 3));
#else
        if (reg & 8)
                addbyte(0x44);
        addbyte(0x89); /*MOV ESI, EAX*/
        addbyte(0xc0 | REG_ESI | (reg << 3));
#endif
}
static void load_param_2_64(codeblock_t *block, uint64_t param)
{
	addbyte(0x48);
#if WIN64
        addbyte(0xba); /*MOVL $fetchdat,%edx*/
#else
        addbyte(0xbe); /*MOVL $fetchdat,%esi*/
#endif
        addquad(param);
}

static void load_param_3_reg_32(int reg)
{
        if (reg & 8)
        {
#if WIN64
                addbyte(0x45); /*MOVL R8,reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((reg & 7) << 3));
#else
                addbyte(0x44); /*MOV EDX, reg*/
                addbyte(0x89);
                addbyte(0xc0 | REG_EDX | (reg << 3));
#endif
        }
        else
        {
#if WIN64
                addbyte(0x41); /*MOVL R8,reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((reg & 7) << 3));
#else
                addbyte(0x90);
                addbyte(0x89); /*MOV EDX, reg*/
                addbyte(0xc0 | REG_EDX | (reg << 3));
#endif
        }
}
static void load_param_3_reg_64(int reg)
{
        if (reg & 8)
        {
#if WIN64
                addbyte(0x4d); /*MOVL R8,reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((reg & 7) << 3));
#else
                addbyte(0x4c); /*MOVL EDX,reg*/
                addbyte(0x89);
                addbyte(0xc0 | REG_EDX | ((reg & 7) << 3));
#endif
        }
        else
        {
#if WIN64
                addbyte(0x49); /*MOVL R8,reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((reg & 7) << 3));
#else
                addbyte(0x48); /*MOVL EDX,reg*/
                addbyte(0x89);
                addbyte(0xc0 | REG_EDX | ((reg & 7) << 3));
#endif
        }
}

static void CALL_FUNC(uintptr_t func)
{
        codegen_reg_loaded[0] = codegen_reg_loaded[1] = codegen_reg_loaded[2] = codegen_reg_loaded[3] = 0;
        codegen_reg_loaded[4] = codegen_reg_loaded[5] = codegen_reg_loaded[6] = codegen_reg_loaded[7] = 0;

	addbyte(0x48); /*MOV RAX, func*/
	addbyte(0xb8);
	addquad(func);
	addbyte(0xff); /*CALL RAX*/
	addbyte(0xd0);
}

static void RELEASE_REG(int host_reg)
{
}

static int LOAD_REG_B(int reg)
{
        int host_reg = reg & 3;
//        host_reg_mapping[host_reg] = reg;

        if (!codegen_reg_loaded[reg & 3])
        {
                addbyte(0x44); /*MOVZX W[reg],host_reg*/
                addbyte(0x8b);
                addbyte(0x45 | (host_reg << 3));
                addbyte((uint32_t)&regs[host_reg & 3].b - (uint32_t)&EAX);
        }

        codegen_reg_loaded[reg & 3] = 1;
        
        if (reg & 4)
                return host_reg | 0x18;
                
        return host_reg | 8;
}
static int LOAD_REG_W(int reg)
{
        int host_reg = reg;
//        host_reg_mapping[host_reg] = reg;

        if (!codegen_reg_loaded[reg & 7])
        {
                addbyte(0x44); /*MOVZX W[reg],host_reg*/
                addbyte(0x8b);
                addbyte(0x45 | (host_reg << 3));
                addbyte((uint32_t)&regs[reg & 7].w - (uint32_t)&EAX);
        }

        codegen_reg_loaded[reg & 7] = 1;
        
        return host_reg | 8;
}
static int LOAD_REG_L(int reg)
{
        int host_reg = reg;
//        host_reg_mapping[host_reg] = reg;

        if (!codegen_reg_loaded[reg & 7])
        {
                addbyte(0x44); /*MOVZX W[reg],host_reg*/
                addbyte(0x8b);
                addbyte(0x45 | (host_reg << 3));
                addbyte((uint32_t)&regs[reg & 7].l - (uint32_t)&EAX);
        }

        codegen_reg_loaded[reg & 7] = 1;
        
        return host_reg | 8;
}

static int LOAD_REG_IMM(uint32_t imm)
{
        int host_reg = REG_EBX;

        addbyte(0xb8 | REG_EBX); /*MOVL EBX, imm*/
        addlong(imm);
        
        return host_reg;
}

static void STORE_REG_TARGET_B_RELEASE(int host_reg, int guest_reg)
{
        int dest_reg = LOAD_REG_L(guest_reg & 3) & 7;
        
        if (guest_reg & 4)
        {
                if (host_reg & 8)
                {
                        addbyte(0x66); /*MOV AX, host_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((host_reg & 3) << 3));
                        if (host_reg & 0x10)
                        {                        
                                addbyte(0x66); /*AND AX, 0xff00*/
                                addbyte(0x25);
                                addword(0xff00);
                        }
                        else
                        {
                                addbyte(0x66); /*SHL AX, 8*/
                                addbyte(0xc1);
                                addbyte(0xe0);
                                addbyte(0x08);
                        }
                }
                else
                {
                        if (host_reg)
                        {
                                addbyte(0x66); /*MOV AX, host_reg*/
                                addbyte(0x89);
                                addbyte(0xc0 | ((host_reg & 3) << 3));
                        }
                        addbyte(0x66); /*SHL AX, 8*/
                        addbyte(0xc1);
                        addbyte(0xe0);
                        addbyte(0x08);
                }
                addbyte(0x66); /*AND dest_reg, 0x00ff*/
                addbyte(0x41);
                addbyte(0x81);
                addbyte(0xe0 | dest_reg);
                addword(0x00ff);
                addbyte(0x66); /*OR dest_reg, AX*/
                addbyte(0x41);
                addbyte(0x09);
                addbyte(0xc0 | dest_reg);
                addbyte(0x66); /*MOVW regs[guest_reg].w, dest_reg*/
                addbyte(0x44);
                addbyte(0x89);
                addbyte(0x45 | (dest_reg << 3));
                addbyte((uint32_t)&regs[guest_reg & 3].w - (uint32_t)&EAX);
        }
        else
        {
                if (host_reg & 8)
                {
                        if (host_reg & 0x10)
                        {
                                addbyte(0x66); /*MOV AX, host_reg*/
                                addbyte(0x44);
                                addbyte(0x89);
                                addbyte(0xc0 | ((host_reg & 3) << 3));
                                addbyte(0x88); /*MOV AL, AH*/
                                addbyte(0xe0);
                                addbyte(0x41); /*MOV dest_reg, AL*/
                                addbyte(0x88);
                                addbyte(0xc0 | (dest_reg & 7));
                                addbyte(0x88); /*MOVB regs[reg].b, AH*/
                                addbyte(0x65);
                                addbyte((uint32_t)&regs[guest_reg & 3].b - (uint32_t)&EAX);
                        }
                        else
                        {
                                addbyte(0x45); /*MOVB dest_reg, host_reg*/
                                addbyte(0x88);
                                addbyte(0xc0 | (dest_reg & 7) | ((host_reg & 7) << 3));
                                addbyte(0x44); /*MOVB regs[guest_reg].b, host_reg*/
                                addbyte(0x88);
                                addbyte(0x45 | ((host_reg & 3) << 3));
                                addbyte((uint32_t)&regs[guest_reg & 3].b - (uint32_t)&EAX);
                        }
                }
                else
                {
                        if (host_reg & 0x10)
                        {
                                addbyte(0xc1); /*SHR host_reg, 8*/
                                addbyte(0xe8 | (host_reg & 7));
                                addbyte(8);
                        }
                        addbyte(0x41); /*MOVB dest_reg, host_reg*/
                        addbyte(0x88);
                        addbyte(0xc0 | (dest_reg & 7) | ((host_reg & 7) << 3));
                        addbyte(0x88); /*MOVB regs[guest_reg].b, host_reg*/
                        addbyte(0x45 | ((host_reg & 3) << 3));
                        addbyte((uint32_t)&regs[guest_reg & 3].b - (uint32_t)&EAX);
                }
        }
}
static void STORE_REG_TARGET_W_RELEASE(int host_reg, int guest_reg)
{
        int dest_reg = LOAD_REG_L(guest_reg & 7) & 7;
        
        if (host_reg & 8)
        {
                addbyte(0x66); /*MOVW guest_reg, host_reg*/
                addbyte(0x45);
                addbyte(0x89);
                addbyte(0xc0 | dest_reg | ((host_reg & 7) << 3));
                addbyte(0x66); /*MOVW regs[guest_reg].w, host_reg*/
                addbyte(0x44);
                addbyte(0x89);
                addbyte(0x45 | ((host_reg & 7) << 3));
                addbyte((uint32_t)&regs[guest_reg & 7].w - (uint32_t)&EAX);
        }
        else
        {
                addbyte(0x66); /*MOVW guest_reg, host_reg*/
                addbyte(0x41);
                addbyte(0x89);
                addbyte(0xc0 | dest_reg | (host_reg << 3));
                addbyte(0x66); /*MOVW regs[guest_reg].w, host_reg*/
                addbyte(0x89);
                addbyte(0x45 | (host_reg << 3));
                addbyte((uint32_t)&regs[guest_reg & 7].w - (uint32_t)&EAX);
        }
}
static void STORE_REG_TARGET_L_RELEASE(int host_reg, int guest_reg)
{
        if (host_reg & 8)
        {
                addbyte(0x45); /*MOVL guest_reg, host_reg*/
                addbyte(0x89);
                addbyte(0xc0 | guest_reg | (host_reg << 3));
                addbyte(0x44); /*MOVL regs[guest_reg].l, host_reg*/
                addbyte(0x89);
                addbyte(0x45 | (host_reg << 3));
                addbyte((uint32_t)&regs[guest_reg & 7].l - (uint32_t)&EAX);
        }
        else
        {
                addbyte(0x41); /*MOVL guest_reg, host_reg*/
                addbyte(0x89);
                addbyte(0xc0 | guest_reg | (host_reg << 3));
                addbyte(0x89); /*MOVL regs[guest_reg].l, host_reg*/
                addbyte(0x45 | (host_reg << 3));
                addbyte((uint32_t)&regs[guest_reg & 7].l - (uint32_t)&EAX);
        }
}

static void STORE_REG_B_RELEASE(int host_reg)
{
        if (host_reg & 0x10)
        {
                addbyte(0x66); /*MOVW [reg],host_reg*/
                addbyte(0x44);
                addbyte(0x89);
                addbyte(0x45 | ((host_reg & 7) << 3));
                addbyte((uint32_t)&regs[host_reg & 7].w - (uint32_t)&EAX);
        }
        else
        {
                addbyte(0x44); /*MOVB [reg],host_reg*/
                addbyte(0x88);
                addbyte(0x45 | ((host_reg & 7) << 3));
                addbyte((uint32_t)&regs[host_reg & 7].b - (uint32_t)&EAX);
        }
}
static void STORE_REG_W_RELEASE(int host_reg)
{
        addbyte(0x66); /*MOVW [reg],host_reg*/
        addbyte(0x44);
        addbyte(0x89);
        addbyte(0x45 | ((host_reg & 7) << 3));
        addbyte((uint32_t)&regs[host_reg & 7].w - (uint32_t)&EAX);
}
static void STORE_REG_L_RELEASE(int host_reg)
{
        addbyte(0x44); /*MOVL [reg],host_reg*/
        addbyte(0x89);
        addbyte(0x45 | ((host_reg & 7) << 3));
        addbyte((uint32_t)&regs[host_reg & 7].l - (uint32_t)&EAX);
}

static void STORE_IMM_REG_B(int reg, uint8_t val)
{
        if (reg & 4)
        {
                int host_reg = LOAD_REG_W(reg & 3) & 7;
                addbyte(0x66); /*AND host_reg, 0x00ff*/
                addbyte(0x41);
                addbyte(0x81);
                addbyte(0xe0 | host_reg);
                addword(0x00ff);
                addbyte(0x66); /*OR host_reg, val << 8*/
                addbyte(0x41);
                addbyte(0x81);
                addbyte(0xc8 | host_reg);
                addword(val << 8);
                addbyte(0x66); /*MOVW host_reg, regs[host_reg].w*/
                addbyte(0x44);
                addbyte(0x89);
                addbyte(0x45 | (host_reg << 3));
                addbyte((uint32_t)&regs[reg & 3].w - (uint32_t)&EAX);
        }
        else
        {
                addbyte(0x41); /*MOVB reg, imm*/
                addbyte(0xb0 | reg);
                addbyte(val);
                addbyte(0x44); /*MOVB reg, regs[reg].b*/
                addbyte(0x88);
                addbyte(0x45 | (reg << 3));
                addbyte((uint32_t)&regs[reg & 7].b - (uint32_t)&EAX);
        }
}
static void STORE_IMM_REG_W(int reg, uint16_t val)
{
        addbyte(0x66); /*MOVW reg, imm*/
        addbyte(0x41);
        addbyte(0xb8 | reg);
        addword(val);
        addbyte(0x66); /*MOVW reg, regs[reg].w*/
        addbyte(0x44);
        addbyte(0x89);
        addbyte(0x45 | (reg << 3));
        addbyte((uint32_t)&regs[reg & 7].w - (uint32_t)&EAX);
}
static void STORE_IMM_REG_L(int reg, uint32_t val)
{
        addbyte(0x41); /*MOVL reg, imm*/
        addbyte(0xb8 | reg);
        addlong(val);
        addbyte(0x44); /*MOVL reg, regs[reg].l*/
        addbyte(0x89);
        addbyte(0x45 | (reg << 3));
        addbyte((uint32_t)&regs[reg & 7].l - (uint32_t)&EAX);
}

static void STORE_IMM_ADDR_L(uintptr_t addr, uint32_t val)
{
        if (addr < 0x100000000)
        {
                addbyte(0xC7); /*MOVL [addr],val*/
                addbyte(0x04);
                addbyte(0x25);
                addlong(addr);
                addlong(val);
        }
        else
        {
                fatal("addr > 32-bit\n");
        }
}




static x86seg *FETCH_EA_16(x86seg *op_ea_seg, uint32_t fetchdat, int op_ssegs, uint32_t *op_pc)
{
        int mod = (fetchdat >> 6) & 3;
        int reg = (fetchdat >> 3) & 7;
        int rm = fetchdat & 7;

        if (!mod && rm == 6) 
        { 
                addbyte(0xb8); /*MOVL EAX, imm*/
                addlong((fetchdat >> 8) & 0xffff);
                (*op_pc) += 2;
        }
        else
        {
                int base_reg, index_reg;
                
                switch (rm)
                {
                        case 0: case 1: case 7:
                        base_reg = LOAD_REG_W(REG_BX);
                        break;
                        case 2: case 3: case 6:
                        base_reg = LOAD_REG_W(REG_BP);
                        break;
                        case 4:
                        base_reg = LOAD_REG_W(REG_SI);
                        break;
                        case 5:
                        base_reg = LOAD_REG_W(REG_DI);
                        break;
                }
                if (!(rm & 4))
                {
                        if (rm & 1)
                                index_reg = LOAD_REG_W(REG_DI);
                        else
                                index_reg = LOAD_REG_W(REG_SI);
                }
                base_reg &= 7;
                index_reg &= 7;
                
                switch (mod)
                {
                        case 0:
                        if (rm & 4)
                        {
                                addbyte(0x41); /*MOVZX EAX, base_reg*/
                                addbyte(0x0f);
                                addbyte(0xb7);
                                addbyte(0xc0 | base_reg);
                        }
                        else
                        {
                                addbyte(0x67); /*LEA EAX, base_reg+index_reg*/
                                addbyte(0x43);
                                addbyte(0x8d);
                                if (base_reg == 5)
                                {
                                        addbyte(0x44);
                                        addbyte(base_reg | (index_reg << 3));
                                        addbyte(0);
                                }
                                else
                                {
                                        addbyte(0x04);
                                        addbyte(base_reg | (index_reg << 3));
                                }
                        }
                        break;
                        case 1:
                        if (rm & 4)
                        {
                                addbyte(0x67); /*LEA EAX, base_reg+imm8*/
                                addbyte(0x41);
                                addbyte(0x8d);
                                addbyte(0x40 | base_reg);
                                addbyte((fetchdat >> 8) & 0xff);
                        }
                        else
                        {
                                addbyte(0x67); /*LEA EAX, base_reg+index_reg+imm8*/
                                addbyte(0x43);
                                addbyte(0x8d);
                                addbyte(0x44);
                                addbyte(base_reg | (index_reg << 3));
                                addbyte((fetchdat >> 8) & 0xff);
                        }
                        (*op_pc)++;
                        break;
                        case 2:
                        if (rm & 4)
                        {
                                addbyte(0x67); /*LEA EAX, base_reg+imm8*/
                                addbyte(0x41);
                                addbyte(0x8d);
                                addbyte(0x80 | base_reg);
                                addlong((fetchdat >> 8) & 0xffff);
                        }
                        else
                        {
                                addbyte(0x67); /*LEA EAX, base_reg+index_reg+imm16*/
                                addbyte(0x43);
                                addbyte(0x8d);
                                addbyte(0x84);
                                addbyte(base_reg | (index_reg << 3));
                                addlong((fetchdat >> 8) & 0xffff);
                        }
                        (*op_pc) += 2;
                        break;
                        
                }
                if (mod || !(rm & 4))
                {
                        addbyte(0x25); /*ANDL $0xffff, %eax*/
                        addlong(0xffff);
                }

                if (mod1seg[rm] == &ss && !op_ssegs)
                        op_ea_seg = &_ss;
        }
        return op_ea_seg;
}
static x86seg *FETCH_EA_32(x86seg *op_ea_seg, uint32_t fetchdat, int op_ssegs, uint32_t *op_pc, int stack_offset)
{
        int mod = (fetchdat >> 6) & 3;
        int reg = (fetchdat >> 3) & 7;
        int rm = fetchdat & 7;
        uint32_t new_eaaddr;

        if (rm == 4)
        {
                uint8_t sib = fetchdat >> 8;
                int base_reg = -1, index_reg = -1;

                (*op_pc)++;

                if (mod || (sib & 7) != 5)
                        base_reg = LOAD_REG_L(sib & 7) & 7;

                if (((sib >> 3) & 7) != 4)
                        index_reg = LOAD_REG_L((sib >> 3) & 7) & 7;

                if (index_reg == -1)
                {
                        switch (mod)
                        {
                                case 0:
                                if ((sib & 7) == 5)
                                {
                                        new_eaaddr = fastreadl(cs + (*op_pc) + 1);
                                        addbyte(0xb8); /*MOV EAX, imm32*/
                                        addlong(new_eaaddr);
                                        (*op_pc) += 4;
                                }
                                else
                                {
                                        addbyte(0x44); /*MOV EAX, base_reg*/
                                        addbyte(0x89);
                                        addbyte(0xc0 | (base_reg << 3));
                                }
                                break;
                                case 1:
                                addbyte(0x67); /*LEA EAX, imm8+base_reg*/
                                addbyte(0x41);
                                addbyte(0x8d);
                                if (base_reg == 4)
                                {
                                        addbyte(0x44);
                                        addbyte(0x24);
                                }
                                else
                                {
                                        addbyte(0x40 | base_reg);
                                }
                                addbyte((fetchdat >> 16) & 0xff);
                                (*op_pc)++;
                                break;
                                case 2:
                                new_eaaddr = fastreadl(cs + (*op_pc) + 1);
                                addbyte(0x67); /*LEA EAX, imm32+base_reg*/
                                addbyte(0x41);
                                addbyte(0x8d);
                                if (base_reg == 4)
                                {
                                        addbyte(0x84);
                                        addbyte(0x24);
                                }
                                else
                                {
                                        addbyte(0x80 | base_reg);
                                }
                                addlong(new_eaaddr);
                                (*op_pc) += 4;
                                break;
                        }
                }
                else
                {                
                        switch (mod)
                        {
                                case 0:
                                if ((sib & 7) == 5)
                                {
                                        new_eaaddr = fastreadl(cs + (*op_pc) + 1);
                                        if (sib >> 6)
                                        {
                                                addbyte(0x67); /*LEA EAX, imm32+index_reg*scale*/
                                                addbyte(0x42);
                                                addbyte(0x8d);
                                                addbyte(0x04);
                                                addbyte(0x05 | (sib & 0xc0) | (index_reg << 3));
                                                addlong(new_eaaddr);
                                        }
                                        else
                                        {
                                                addbyte(0x67); /*LEA EAX, imm32+index_reg*/
                                                addbyte(0x41);
                                                addbyte(0x8d);
                                                addbyte(0x80 | index_reg);
                                                addlong(new_eaaddr);
                                        }
                                        (*op_pc) += 4;
                                }
                                else
                                {
                                        addbyte(0x67); /*LEA EAX, base_reg+index_reg*scale*/
                                        addbyte(0x43);
                                        addbyte(0x8d);
                                        if (base_reg == 5)
                                        {
                                                addbyte(0x44);
                                                addbyte(base_reg | (index_reg << 3) | (sib & 0xc0));
                                                addbyte(0);
                                        }
                                        else
                                        {
                                                addbyte(0x04);
                                                addbyte(base_reg | (index_reg << 3) | (sib & 0xc0));
                                        }
                                }
                                break;
                                case 1:
                                addbyte(0x67); /*LEA EAX, imm8+base_reg+index_reg*scale*/
                                addbyte(0x43);
                                addbyte(0x8d);
                                addbyte(0x44);
                                addbyte(base_reg | (index_reg << 3) | (sib & 0xc0));
                                addbyte((fetchdat >> 16) & 0xff);
                                (*op_pc)++;
                                break;
                                case 2:
                                new_eaaddr = fastreadl(cs + (*op_pc) + 1);
                                addbyte(0x67); /*LEA EAX, imm32+base_reg+index_reg*scale*/
                                addbyte(0x43);
                                addbyte(0x8d);
                                addbyte(0x84);
                                addbyte(base_reg | (index_reg << 3) | (sib & 0xc0));
                                addlong(new_eaaddr);
                                (*op_pc) += 4;
                                break;
                        }
                }
                if (stack_offset && (sib & 7) == 4 && (mod || (sib & 7) != 5)) /*ESP*/
                {
                        addbyte(0x05);
                        addlong(stack_offset);
                }
                if (((sib & 7) == 4 || (mod && (sib & 7) == 5)) && !op_ssegs)
                        op_ea_seg = &_ss;
        }
        else
        {
                int base_reg;

                if (!mod && rm == 5)
                {                
                        new_eaaddr = fastreadl(cs + (*op_pc) + 1);
                        addbyte(0xb8); /*MOVL EAX, new_eaaddr*/
                        addlong(new_eaaddr);
                        (*op_pc) += 4;
                        return op_ea_seg;
                }
                base_reg = LOAD_REG_L(rm) & 7;
                if (mod) 
                {
                        if (rm == 5 && !op_ssegs)
                                op_ea_seg = &_ss;
                        if (mod == 1) 
                        {
                                addbyte(0x67); /*LEA EAX, base_reg+imm8*/
                                addbyte(0x41);
                                addbyte(0x8d);
                                addbyte(0x40 | base_reg);
                                addbyte((fetchdat >> 8) & 0xff);
                                (*op_pc)++; 
                        }
                        else          
                        {
                                new_eaaddr = fastreadl(cs + (*op_pc) + 1);
                                addbyte(0x67); /*LEA EAX, base_reg+imm32*/
                                addbyte(0x41);
                                addbyte(0x8d);
                                addbyte(0x80 | base_reg);
                                addlong(new_eaaddr);
                                (*op_pc) += 4;
                        }
                }
                else
                {
                        addbyte(0x44); /*MOV EAX, base_reg*/
                        addbyte(0x89);
                        addbyte(0xc0 | (base_reg << 3));
                }
        }
        return op_ea_seg;
}

static x86seg *FETCH_EA(x86seg *op_ea_seg, uint32_t fetchdat, int op_ssegs, uint32_t *op_pc, uint32_t op_32)
{
        if (op_32 & 0x200)
                return FETCH_EA_32(op_ea_seg, fetchdat, op_ssegs, op_pc, 0);
        return FETCH_EA_16(op_ea_seg, fetchdat, op_ssegs, op_pc);
}



static void CHECK_SEG_READ(x86seg *seg)
{
        /*Segments always valid in real/V86 mode*/
        if (!(cr0 & 1) || (eflags & VM_FLAG))
                return;
        /*CS and SS must always be valid*/
        if (seg == &_cs || seg == &_ss)
                return;
        if (seg->checked)
                return;

        addbyte(0x83); /*CMP seg->base, -1*/
        addbyte(0x3c);
        addbyte(0x25);
        addlong((uint32_t)&seg->base);
        addbyte(-1);
        addbyte(0x0f); /*JE end*/
        addbyte(0x84);
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
        
        seg->checked = 1;
}
static void CHECK_SEG_WRITE(x86seg *seg)
{
        /*Segments always valid in real/V86 mode*/
        if (!(cr0 & 1) || (eflags & VM_FLAG))
                return;
        /*CS and SS must always be valid*/
        if (seg == &_cs || seg == &_ss)
                return;
        if (seg->checked)
                return;
                
        addbyte(0x83); /*CMP seg->base, -1*/
        addbyte(0x3c);
        addbyte(0x25);
        addlong((uint32_t)&seg->base);
        addbyte(-1);
        addbyte(0x0f); /*JE end*/
        addbyte(0x84);
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));

        seg->checked = 1;
}
static void CHECK_SEG_LIMITS(x86seg *seg, int end_offset)
{
        addbyte(0x3b); /*CMP EAX, seg->limit_low*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uint32_t)&seg->limit_low);
        addbyte(0x0f); /*JB BLOCK_GPF_OFFSET*/
        addbyte(0x82);
        addlong(BLOCK_GPF_OFFSET - (block_pos + 4));
        if (end_offset)
        {
                addbyte(0x83); /*ADD EAX, end_offset*/
                addbyte(0xc0);
                addbyte(end_offset);
                addbyte(0x3b); /*CMP EAX, seg->limit_high*/
                addbyte(0x04);
                addbyte(0x25);
                addlong((uint32_t)&seg->limit_high);
                addbyte(0x0f); /*JNBE BLOCK_GPF_OFFSET*/
                addbyte(0x87);
                addlong(BLOCK_GPF_OFFSET - (block_pos + 4));
                addbyte(0x83); /*SUB EAX, end_offset*/
                addbyte(0xe8);
                addbyte(end_offset);
        }
}

#define IS_32_ADDR(x) !(((uintptr_t)x) & 0xffffffff00000000)

static void MEM_LOAD_ADDR_EA_B(x86seg *seg)
{
        addbyte(0x8b); /*MOVL ECX, seg->base*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong((uint32_t)&seg->base);
        addbyte(0x67); /*LEA ESI, (EAX,ECX)*/
        addbyte(0x8d);
        addbyte(0x34);
        addbyte(0x08);
        addbyte(0x89); /*MOV EDI, ESI*/
        addbyte(0xf7);
        addbyte(0xc1); /*SHR ESI, 12*/
        addbyte(0xe8 | REG_ESI);
        addbyte(12);
	if (IS_32_ADDR(readlookup2))
	{
	        addbyte(0x67); /*MOV RSI, readlookup2[ESI*8]*/
	        addbyte(0x48);
	        addbyte(0x8b);
	        addbyte(0x34);
	        addbyte(0xf5);
	        addlong((uint32_t)readlookup2);
	}
	else
	{
		addbyte(0x48); /*MOV RDX, readlookup2*/
		addbyte(0xb8 | REG_EDX);
		addquad((uint64_t)readlookup2);
		addbyte(0x48); /*MOV RSI, [RDX+RSI*8]*/
		addbyte(0x8b);
		addbyte(0x34);
		addbyte(0xf2);
	}
        addbyte(0x83); /*CMP ESI, -1*/
        addbyte(0xf8 | REG_ESI);
        addbyte(-1);
        addbyte(0x74); /*JE slowpath*/
        addbyte(3+2);
        addbyte(0x8b); /*MOV AL,[RDI+RSI]*/
        addbyte(0x04);
        addbyte(REG_EDI | (REG_ESI << 3));
        addbyte(0xeb); /*JMP done*/
        addbyte(2+2+12+8+6);
        /*slowpath:*/
        load_param_1_reg_32(REG_ECX);
        load_param_2_reg_32(REG_EAX);
        call_long(readmemb386l);
        addbyte(0x83); /*CMP abrt, 0*/
        addbyte(0x3c);
        addbyte(0x25);
        addlong((uint32_t)&abrt);
        addbyte(0);
        addbyte(0x0f); /*JNE end*/
        addbyte(0x85);
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
        /*done:*/
}
static void MEM_LOAD_ADDR_EA_W(x86seg *seg)
{
        addbyte(0x8b); /*MOVL ECX, seg->base*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong((uint32_t)&seg->base);
        addbyte(0x67); /*LEA ESI, (EAX,ECX)*/
        addbyte(0x8d);
        addbyte(0x34);
        addbyte(0x08);
        addbyte(0x67); /*LEA EDI, 1[ESI]*/
        addbyte(0x8d);
        addbyte(0x7e);
        addbyte(0x01);
        addbyte(0xc1); /*SHR ESI, 12*/
        addbyte(0xe8 | REG_ESI);
        addbyte(12);
        addbyte(0xf7); /*TEST EDI, 0xfff*/
        addbyte(0xc7);
        addlong(0xfff);
	if (IS_32_ADDR(readlookup2))
	{
	        addbyte(0x67); /*MOV RSI, readlookup2[ESI*8]*/
	        addbyte(0x48);
	        addbyte(0x8b);
	        addbyte(0x34);
	        addbyte(0xf5);
	        addlong((uint32_t)readlookup2);
	}
	else
	{
		addbyte(0x48); /*MOV RDX, readlookup2*/
		addbyte(0xb8 | REG_EDX);
		addquad((uint64_t)readlookup2);
		addbyte(0x48); /*MOV RSI, [RDX+RSI*8]*/
		addbyte(0x8b);
		addbyte(0x34);
		addbyte(0xf2);
	}
        addbyte(0x74); /*JE slowpath*/
        addbyte(3+2+5+2);
        addbyte(0x83); /*CMP ESI, -1*/
        addbyte(0xf8 | REG_ESI);
        addbyte(-1);
        addbyte(0x74); /*JE slowpath*/
        addbyte(5+2);
        addbyte(0x66); /*MOV AX,-1[RDI+RSI]*/
        addbyte(0x8b);
        addbyte(0x44);
        addbyte(REG_EDI | (REG_ESI << 3));
        addbyte(-1);
        addbyte(0xeb); /*JMP done*/
        addbyte(2+2+12+8+6);
        /*slowpath:*/
        load_param_1_reg_32(REG_ECX);
        load_param_2_reg_32(REG_EAX);
        call_long(readmemwl);
        addbyte(0x83); /*CMP abrt, 0*/
        addbyte(0x3c);
        addbyte(0x25);
        addlong((uint32_t)&abrt);
        addbyte(0);
        addbyte(0x0f); /*JNE end*/
        addbyte(0x85);
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
        /*done:*/
}
static void MEM_LOAD_ADDR_EA_L(x86seg *seg)
{
        addbyte(0x8b); /*MOVL ECX, seg->base*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong((uint32_t)&seg->base);
        addbyte(0x67); /*LEA ESI, (EAX,ECX)*/
        addbyte(0x8d);
        addbyte(0x34);
        addbyte(0x08);
        addbyte(0x67); /*LEA EDI, 3[ESI]*/
        addbyte(0x8d);
        addbyte(0x7e);
        addbyte(0x03);
        addbyte(0xc1); /*SHR ESI, 12*/
        addbyte(0xe8 | REG_ESI);
        addbyte(12);
        addbyte(0xf7); /*TEST EDI, 0xffc*/
        addbyte(0xc7);
        addlong(0xffc);
	if (IS_32_ADDR(readlookup2))
	{
	        addbyte(0x67); /*MOV RSI, readlookup2[ESI*8]*/
	        addbyte(0x48);
	        addbyte(0x8b);
	        addbyte(0x34);
	        addbyte(0xf5);
	        addlong((uint32_t)readlookup2);
	}
	else
	{
		addbyte(0x48); /*MOV RDX, readlookup2*/
		addbyte(0xb8 | REG_EDX);
		addquad((uint64_t)readlookup2);
		addbyte(0x48); /*MOV RSI, [RDX+RSI*8]*/
		addbyte(0x8b);
		addbyte(0x34);
		addbyte(0xf2);
	}
        addbyte(0x74); /*JE slowpath*/
        addbyte(3+2+4+2);
        addbyte(0x83); /*CMP ESI, -1*/
        addbyte(0xf8 | REG_ESI);
        addbyte(-1);
        addbyte(0x74); /*JE slowpath*/
        addbyte(4+2);
        addbyte(0x8b); /*MOV EAX,-3[RDI+RSI]*/
        addbyte(0x44);
        addbyte(REG_EDI | (REG_ESI << 3));
        addbyte(-3);
        addbyte(0xeb); /*JMP done*/
        addbyte(2+2+12+8+6);
        /*slowpath:*/
        load_param_1_reg_32(REG_ECX);
        load_param_2_reg_32(REG_EAX);
        call_long(readmemll);
        addbyte(0x83); /*CMP abrt, 0*/
        addbyte(0x3c);
        addbyte(0x25);
        addlong((uint32_t)&abrt);
        addbyte(0);
        addbyte(0x0f); /*JNE end*/
        addbyte(0x85);
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
        /*done:*/
}
static void MEM_LOAD_ADDR_EA_Q(x86seg *seg)
{
        addbyte(0x8b); /*MOVL ECX, seg->base*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong((uint32_t)&seg->base);
        addbyte(0x67); /*LEA ESI, (EAX,ECX)*/
        addbyte(0x8d);
        addbyte(0x34);
        addbyte(0x08);
        addbyte(0x67); /*LEA EDI, 7[ESI]*/
        addbyte(0x8d);
        addbyte(0x7e);
        addbyte(0x07);
        addbyte(0xc1); /*SHR ESI, 12*/
        addbyte(0xe8 | REG_ESI);
        addbyte(12);
        addbyte(0xf7); /*TEST EDI, 0xff8*/
        addbyte(0xc7);
        addlong(0xff8);
	if (IS_32_ADDR(readlookup2))
	{
	        addbyte(0x67); /*MOV RSI, readlookup2[ESI*8]*/
	        addbyte(0x48);
	        addbyte(0x8b);
	        addbyte(0x34);
	        addbyte(0xf5);
	        addlong((uint32_t)readlookup2);
	}
	else
	{
		addbyte(0x48); /*MOV RDX, readlookup2*/
		addbyte(0xb8 | REG_EDX);
		addquad((uint64_t)readlookup2);
		addbyte(0x48); /*MOV RSI, [RDX+RSI*8]*/
		addbyte(0x8b);
		addbyte(0x34);
		addbyte(0xf2);
	}
        addbyte(0x74); /*JE slowpath*/
        addbyte(3+2+5+2);
        addbyte(0x83); /*CMP ESI, -1*/
        addbyte(0xf8 | REG_ESI);
        addbyte(-1);
        addbyte(0x74); /*JE slowpath*/
        addbyte(5+2);
        addbyte(0x48); /*MOV RAX,-7[RDI+RSI]*/
        addbyte(0x8b);
        addbyte(0x44);
        addbyte(REG_EDI | (REG_ESI << 3));
        addbyte(-7);
        addbyte(0xeb); /*JMP done*/
        addbyte(2+2+12+8+6);
        /*slowpath:*/
        load_param_1_reg_32(REG_ECX);
        load_param_2_reg_32(REG_EAX);
        call_long(readmemql);
        addbyte(0x83); /*CMP abrt, 0*/
        addbyte(0x3c);
        addbyte(0x25);
        addlong((uint32_t)&abrt);
        addbyte(0);
        addbyte(0x0f); /*JNE end*/
        addbyte(0x85);
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
        /*done:*/
}

static void MEM_LOAD_ADDR_IMM_B(x86seg *seg, uint32_t addr)
{
        addbyte(0xb8); /*MOV EAX, addr*/
        addlong(addr);
        MEM_LOAD_ADDR_EA_B(seg);
}
static void MEM_LOAD_ADDR_IMM_W(x86seg *seg, uint32_t addr)
{
        addbyte(0xb8); /*MOV EAX, addr*/
        addlong(addr);
        MEM_LOAD_ADDR_EA_W(seg);
}
static void MEM_LOAD_ADDR_IMM_L(x86seg *seg, uint32_t addr)
{
        addbyte(0xb8); /*MOV EAX, addr*/
        addlong(addr);
        MEM_LOAD_ADDR_EA_L(seg);
}

static void MEM_STORE_ADDR_EA_B(x86seg *seg, int host_reg)
{
        if (host_reg & 0x10)
        {
                /*Handle high byte of register*/
                if (host_reg & 8)
                {
                        addbyte(0x45); /*MOVL R8, host_reg*/
                        addbyte(0x89);
                        addbyte(0xc0 | ((host_reg & 7) << 3));
                }
                else
                {
                        addbyte(0x41); /*MOVL R8, host_reg*/
                        addbyte(0x89);
                        addbyte(0xc0 | ((host_reg & 7) << 3));
                }
                addbyte(0x66); /*SHR R8, 8*/
                addbyte(0x41);
                addbyte(0xc1);
                addbyte(0xe8);
                addbyte(8);
                host_reg = 8;
        }
        addbyte(0x8b); /*MOVL ECX, seg->base*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong((uint32_t)&seg->base);
        addbyte(0x67); /*LEA ESI, (EAX,ECX)*/
        addbyte(0x8d);
        addbyte(0x34);
        addbyte(0x08);
        addbyte(0x89); /*MOV EDI, ESI*/
        addbyte(0xf7);
        addbyte(0xc1); /*SHR ESI, 12*/
        addbyte(0xe8 | REG_ESI);
        addbyte(12);
	if (IS_32_ADDR(writelookup2))
	{
	        addbyte(0x67); /*MOV RSI, writelookup2[ESI*8]*/
	        addbyte(0x48);
	        addbyte(0x8b);
	        addbyte(0x34);
	        addbyte(0xf5);
	        addlong((uint32_t)writelookup2);
	}
	else
	{
		addbyte(0x48); /*MOV RDX, writelookup2*/
		addbyte(0xb8 | REG_EDX);
		addquad((uint64_t)writelookup2);
		addbyte(0x48); /*MOV RSI, [RDX+RSI*8]*/
		addbyte(0x8b);
		addbyte(0x34);
		addbyte(0xf2);
	}
        addbyte(0x83); /*CMP ESI, -1*/
        addbyte(0xf8 | REG_ESI);
        addbyte(-1);
        addbyte(0x74); /*JE slowpath*/
        addbyte(((host_reg & 8) ? 4:3)+2);
        if (host_reg & 8)
        {
                addbyte(0x44); /*MOV [RDI+RSI],host_reg*/
                addbyte(0x88);
                addbyte(0x04 | ((host_reg & 7) << 3));
                addbyte(REG_EDI | (REG_ESI << 3));
        }
        else
        {
                addbyte(0x88); /*MOV [RDI+RSI],host_reg*/
                addbyte(0x04 | (host_reg << 3));
                addbyte(REG_EDI | (REG_ESI << 3));
        }
        addbyte(0xeb); /*JMP done*/
        addbyte(2+2+3+12+8+6);
        /*slowpath:*/
        load_param_1_reg_32(REG_ECX);
        load_param_2_reg_32(REG_EAX);
        load_param_3_reg_32(host_reg);
        call_long(writememb386l);
        addbyte(0x83); /*CMP abrt, 0*/
        addbyte(0x3c);
        addbyte(0x25);
        addlong((uint32_t)&abrt);
        addbyte(0);
        addbyte(0x0f); /*JNE end*/
        addbyte(0x85);
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
        /*done:*/
}
static void MEM_STORE_ADDR_EA_W(x86seg *seg, int host_reg)
{
        addbyte(0x8b); /*MOVL ECX, seg->base*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong((uint32_t)&seg->base);
        addbyte(0x67); /*LEA ESI, (EAX,ECX)*/
        addbyte(0x8d);
        addbyte(0x34);
        addbyte(0x08);
        addbyte(0x67); /*LEA EDI, 1[ESI]*/
        addbyte(0x8d);
        addbyte(0x7e);
        addbyte(0x01);
        addbyte(0xc1); /*SHR ESI, 12*/
        addbyte(0xe8 | REG_ESI);
        addbyte(12);
        addbyte(0xf7); /*TEST EDI, 0xfff*/
        addbyte(0xc7);
        addlong(0xfff);
	if (IS_32_ADDR(writelookup2))
	{
	        addbyte(0x67); /*MOV RSI, writelookup2[ESI*8]*/
	        addbyte(0x48);
	        addbyte(0x8b);
	        addbyte(0x34);
	        addbyte(0xf5);
	        addlong((uint32_t)writelookup2);
	}
	else
	{
		addbyte(0x48); /*MOV RDX, writelookup2*/
		addbyte(0xb8 | REG_EDX);
		addquad((uint64_t)writelookup2);
		addbyte(0x48); /*MOV RSI, [RDX+RSI*8]*/
		addbyte(0x8b);
		addbyte(0x34);
		addbyte(0xf2);
	}
        addbyte(0x74); /*JE slowpath*/
        addbyte(3+2+((host_reg & 8) ? 6:5)+2);
        addbyte(0x83); /*CMP ESI, -1*/
        addbyte(0xf8 | REG_ESI);
        addbyte(-1);
        addbyte(0x74); /*JE slowpath*/
        addbyte(((host_reg & 8) ? 6:5)+2);
        if (host_reg & 8)
        {
                addbyte(0x66); /*MOV -1[RDI+RSI],host_reg*/
                addbyte(0x44);
                addbyte(0x89);
                addbyte(0x44 | ((host_reg & 7) << 3));
                addbyte(REG_EDI | (REG_ESI << 3));
                addbyte(-1);
        }
        else
        {
                addbyte(0x66); /*MOV -1[RDI+RSI],host_reg*/
                addbyte(0x89);
                addbyte(0x44 | (host_reg << 3));
                addbyte(REG_EDI | (REG_ESI << 3));
                addbyte(-1);
        }
        addbyte(0xeb); /*JMP done*/
        addbyte(2+2+3+12+8+6);
        /*slowpath:*/
        load_param_1_reg_32(REG_ECX);
        load_param_2_reg_32(REG_EAX);
        load_param_3_reg_32(host_reg);
        call_long(writememwl);
        addbyte(0x83); /*CMP abrt, 0*/
        addbyte(0x3c);
        addbyte(0x25);
        addlong((uint32_t)&abrt);
        addbyte(0);
        addbyte(0x0f); /*JNE end*/
        addbyte(0x85);
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
        /*done:*/
}
static void MEM_STORE_ADDR_EA_L(x86seg *seg, int host_reg)
{
        addbyte(0x8b); /*MOVL ECX, seg->base*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong((uint32_t)&seg->base);
        addbyte(0x67); /*LEA ESI, (EAX,ECX)*/
        addbyte(0x8d);
        addbyte(0x34);
        addbyte(0x08);
        addbyte(0x67); /*LEA EDI, 3[ESI]*/
        addbyte(0x8d);
        addbyte(0x7e);
        addbyte(0x03);
        addbyte(0xc1); /*SHR ESI, 12*/
        addbyte(0xe8 | REG_ESI);
        addbyte(12);
        addbyte(0xf7); /*TEST EDI, 0xffc*/
        addbyte(0xc7);
        addlong(0xffc);
	if (IS_32_ADDR(writelookup2))
	{
	        addbyte(0x67); /*MOV RSI, writelookup2[ESI*8]*/
	        addbyte(0x48);
	        addbyte(0x8b);
	        addbyte(0x34);
	        addbyte(0xf5);
	        addlong((uint32_t)writelookup2);
	}
	else
	{
		addbyte(0x48); /*MOV RDX, writelookup2*/
		addbyte(0xb8 | REG_EDX);
		addquad((uint64_t)writelookup2);
		addbyte(0x48); /*MOV RSI, [RDX+RSI*8]*/
		addbyte(0x8b);
		addbyte(0x34);
		addbyte(0xf2);
	}
        addbyte(0x74); /*JE slowpath*/
        addbyte(3+2+((host_reg & 8) ? 5:4)+2);
        addbyte(0x83); /*CMP ESI, -1*/
        addbyte(0xf8 | REG_ESI);
        addbyte(-1);
        addbyte(0x74); /*JE slowpath*/
        addbyte(((host_reg & 8) ? 5:4)+2);
        if (host_reg & 8)
        {
                addbyte(0x44); /*MOV -3[RDI+RSI],host_reg*/
                addbyte(0x89);
                addbyte(0x44 | ((host_reg & 7) << 3));
                addbyte(REG_EDI | (REG_ESI << 3));
                addbyte(-3);
        }
        else
        {
                addbyte(0x89); /*MOV -3[RDI+RSI],host_reg*/
                addbyte(0x44 | (host_reg << 3));
                addbyte(REG_EDI | (REG_ESI << 3));
                addbyte(-3);
        }
        addbyte(0xeb); /*JMP done*/
        addbyte(2+2+3+12+8+6);
        /*slowpath:*/
        load_param_1_reg_32(REG_ECX);
        load_param_2_reg_32(REG_EAX);
        load_param_3_reg_32(host_reg);
        call_long(writememll);
        addbyte(0x83); /*CMP abrt, 0*/
        addbyte(0x3c);
        addbyte(0x25);
        addlong((uint32_t)&abrt);
        addbyte(0);
        addbyte(0x0f); /*JNE end*/
        addbyte(0x85);
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
        /*done:*/
}
static void MEM_STORE_ADDR_EA_Q(x86seg *seg, int host_reg, int host_reg2)
{
        addbyte(0x8b); /*MOVL ECX, seg->base*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong((uint32_t)&seg->base);
        addbyte(0x67); /*LEA ESI, (EAX,ECX)*/
        addbyte(0x8d);
        addbyte(0x34);
        addbyte(0x08);
        addbyte(0x67); /*LEA EDI, 7[ESI]*/
        addbyte(0x8d);
        addbyte(0x7e);
        addbyte(0x07);
        addbyte(0xc1); /*SHR ESI, 12*/
        addbyte(0xe8 | REG_ESI);
        addbyte(12);
        addbyte(0xf7); /*TEST EDI, 0xff8*/
        addbyte(0xc7);
        addlong(0xff8);
	if (IS_32_ADDR(writelookup2))
	{
	        addbyte(0x67); /*MOV RSI, writelookup2[ESI*8]*/
	        addbyte(0x48);
	        addbyte(0x8b);
	        addbyte(0x34);
	        addbyte(0xf5);
	        addlong((uint32_t)writelookup2);
	}
	else
	{
		addbyte(0x48); /*MOV RDX, writelookup2*/
		addbyte(0xb8 | REG_EDX);
		addquad((uint64_t)writelookup2);
		addbyte(0x48); /*MOV RSI, [RDX+RSI*8]*/
		addbyte(0x8b);
		addbyte(0x34);
		addbyte(0xf2);
	}
        addbyte(0x74); /*JE slowpath*/
        addbyte(3+2+5+2);
        addbyte(0x83); /*CMP ESI, -1*/
        addbyte(0xf8 | REG_ESI);
        addbyte(-1);
        addbyte(0x74); /*JE slowpath*/
        addbyte(5+2);
        if (host_reg & 8)
        {
                addbyte(0x4c); /*MOV -7[RDI+RSI],host_reg*/
                addbyte(0x89);
                addbyte(0x44 | ((host_reg & 7) << 3));
                addbyte(REG_EDI | (REG_ESI << 3));
                addbyte(-7);
        }
        else
        {
                addbyte(0x48); /*MOV -3[RDI+RSI],host_reg*/
                addbyte(0x89);
                addbyte(0x44 | (host_reg << 3));
                addbyte(REG_EDI | (REG_ESI << 3));
                addbyte(-7);
        }
        addbyte(0xeb); /*JMP done*/
        addbyte(2+2+3+12+8+6);
        /*slowpath:*/
        load_param_1_reg_32(REG_ECX);
        load_param_2_reg_32(REG_EAX);
        load_param_3_reg_64(host_reg);
        call_long(writememql);
        addbyte(0x83); /*CMP abrt, 0*/
        addbyte(0x3c);
        addbyte(0x25);
        addlong((uint32_t)&abrt);
        addbyte(0);
        addbyte(0x0f); /*JNE end*/
        addbyte(0x85);
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
        /*done:*/
}

static void MEM_STORE_ADDR_IMM_B(x86seg *seg, uint32_t addr, int host_reg)
{
        addbyte(0xb8); /*MOV EAX, addr*/
        addlong(addr);
        MEM_STORE_ADDR_EA_B(seg, host_reg);
}
static void MEM_STORE_ADDR_IMM_W(x86seg *seg, uint32_t addr, int host_reg)
{
        addbyte(0xb8); /*MOV EAX, addr*/
        addlong(addr);
        MEM_STORE_ADDR_EA_W(seg, host_reg);
}
static void MEM_STORE_ADDR_IMM_L(x86seg *seg, uint32_t addr, int host_reg)
{
        addbyte(0xb8); /*MOV EAX, addr*/
        addlong(addr);
        MEM_STORE_ADDR_EA_L(seg, host_reg);
}

static void STORE_HOST_REG_ADDR_BL(uintptr_t addr, int host_reg)
{
        if (host_reg & 0x10)
        {
                if (host_reg & 8)
                        addbyte(0x41); 
                addbyte(0x0f); /*MOVZX EBX, host_reg*/
                addbyte(0xb7);
                addbyte(0xc0 | (REG_ECX << 3) | (host_reg & 7));
                addbyte(0xc1); /*SHR EBX, 8*/
                addbyte(0xe8 | REG_ECX);
                addbyte(8);
        }
        else
        {
                if (host_reg & 8)
                        addbyte(0x41); 
                addbyte(0x0f); /*MOVZX EBX, host_reg*/
                addbyte(0xb6);
                addbyte(0xc0 | (REG_ECX << 3) | (host_reg & 7));
        }
        addbyte(0x89); /*MOV addr, EBX*/
        addbyte(0x04 | (REG_ECX << 3));
        addbyte(0x25);
        addlong(addr);
}
static void STORE_HOST_REG_ADDR_WL(uintptr_t addr, int host_reg)
{
        if (host_reg & 8)
                addbyte(0x41); 
        addbyte(0x0f); /*MOVZX EBX, host_reg*/
        addbyte(0xb7);
        addbyte(0xc0 | (REG_ECX << 3) | (host_reg & 7));
        addbyte(0x89); /*MOV addr, EBX*/
        addbyte(0x04 | (REG_ECX << 3));
        addbyte(0x25);
        addlong(addr);
}
static void STORE_HOST_REG_ADDR_W(uintptr_t addr, int host_reg)
{
        addbyte(0x66);
        if (host_reg & 8)
                addbyte(0x44);
        addbyte(0x89); /*MOVL addr,host_reg*/
        addbyte(0x04 | ((host_reg & 7) << 3));
        addbyte(0x25);
        addlong(addr);
}
static void STORE_HOST_REG_ADDR(uintptr_t addr, int host_reg)
{
        if (host_reg & 8)
                addbyte(0x44);
        addbyte(0x89); /*MOVL addr,host_reg*/
        addbyte(0x04 | ((host_reg & 7) << 3));
        addbyte(0x25);
        addlong(addr);
}

static void AND_HOST_REG_B(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0x66); /*MOVW AX, src_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((src_reg & 7) << 3));
                        if (!(src_reg & 0x10))
                        {
                                addbyte(0x66); /*SHL AX, 8*/
                                addbyte(0xc1);
                                addbyte(0xe0);
                                addbyte(8);
                        }
                        addbyte(0x66); /*OR AX, 0x00ff*/
                        addbyte(0x0d);
                        addword(0xff);
                        addbyte(0x66); /*ANDW dst_reg, AX*/
                        addbyte(0x41);
                        addbyte(0x21);
                        addbyte(0xc0 | (dst_reg & 7));
                }
                else if (src_reg & 0x10)
                {
                        addbyte(0x66); /*MOVW AX, src_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((src_reg & 7) << 3));
                        addbyte(0x66); /*SHR AX, 8*/
                        addbyte(0xc1);
                        addbyte(0xe8);
                        addbyte(8);
                        addbyte(0x41); /*ANDB dst_reg, AL*/
                        addbyte(0x20);
                        addbyte(0xc0 | (dst_reg & 7));
                }
                else
                {
                        addbyte(0x45); /*ANDB dst_reg, src_reg*/
                        addbyte(0x20);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else if (dst_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0x66); /*SHL src_reg, 8*/
                        addbyte(0xc1);
                        addbyte(0xe0 | src_reg);
                        addbyte(0x08);
                        addbyte(0x66); /*OR src_reg, 0xff*/
                        addbyte(0x81);
                        addbyte(0xc8 | src_reg);
                        addword(0xff);
                        addbyte(0x66); /*ANDW dst_reg, src_reg*/
                        addbyte(0x41);
                        addbyte(0x21);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
                else
                {
                        addbyte(0x41); /*ANDB dst_reg, src_reg*/
                        addbyte(0x20);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else if (src_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0xc1); /*SHR dst_reg, 8*/
                        addbyte(0xe8 | (dst_reg & 7));
                        addbyte(8);
                }
                if (src_reg & 0x10)
                {
                        addbyte(0x41); /*MOVZX EBX, src_reg*/
                        addbyte(0x0f);
                        addbyte(0xb7);
                        addbyte(0xd8 | (src_reg & 7));
                        addbyte(0xc1); /*SHR EBX, 8*/
                        addbyte(0xeb);
                        addbyte(8);
                        addbyte(0x20); /*ANDB dst_reg, EBX*/
                        addbyte(0xd8 | (dst_reg & 7));
                }
                else
                {
                        addbyte(0x44); /*ANDB dst_reg, src_reg*/
                        addbyte(0x20);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }                
        }
        else
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0xc1); /*SHR dst_reg, 8*/
                        addbyte(0xe8 | (dst_reg & 7));
                        addbyte(8);
                }
                if (src_reg & 0x10)
                {
                        addbyte(0x0f); /*MOVZX EBX, src_reg*/
                        addbyte(0xb7);
                        addbyte(0xd8 | (src_reg & 7));
                        addbyte(0xc1); /*SHR EBX, 8*/
                        addbyte(0xeb);
                        addbyte(8);
                        addbyte(0x20); /*ANDB dst_reg, EBX*/
                        addbyte(0xd8 | (dst_reg & 7));
                }
                else
                {
                        addbyte(0x20); /*ANDB dst_reg, src_reg*/
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }                
        }
#if 0
        else if (dst_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        if (src_reg & 0x10)
                        {
                                addbyte(0x66); /*SHL src_reg, 8*/
                                addbyte(0xc1);
                                addbyte(0xe0 | src_reg);
                                addbyte(0x08);
                                addbyte(0x66); /*OR src_reg, 0xff*/
                                addbyte(0x81);
                                addbyte(0xc8 | src_reg);
                                addword(0xff);
                        }
                        addbyte(0x66); /*ANDW dst_reg, src_reg*/
                        addbyte(0x41);
                        addbyte(0x21);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
                else
                {
                        addbyte(0x41); /*ANDB dst_reg, src_reg*/
                        addbyte(0x20);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else if (src_reg & 8)
        {
                if (src_reg & 0x10)
                {
                        addbyte(0x66); /*OR src_reg, 0xff*/
                        addbyte(0x81);
                        addbyte(0xc8 | src_reg);
                        addword(0xff);
                }
                        if (src_reg & 0x10)
                        {
                                addbyte(0x66); /*SHL src_reg, 8*/
                                addbyte(0xc1);
                                addbyte(0xe0 | src_reg);
                                addbyte(0x08);
                                addbyte(0x66); /*OR src_reg, 0xff*/
                                addbyte(0x81);
                                addbyte(0xc8 | src_reg);
                                addword(0xff);
                        }
                        addbyte(0x66); /*ANDW dst_reg, src_reg*/
                        addbyte(0x41);
                        addbyte(0x21);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
                else
                {
                        addbyte(0x41); /*ANDB dst_reg, src_reg*/
                        addbyte(0x20);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else
                fatal("!(dst_reg & src_reg & 8) b %i %i\n", dst_reg, src_reg);
#endif
}
static void AND_HOST_REG_W(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                addbyte(0x66); /*ANDW dst_reg, src_reg*/
                addbyte(0x45);
                addbyte(0x21);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (dst_reg & 8)
        {
                addbyte(0x66); /*ANDW dst_reg, src_reg*/
                addbyte(0x41);
                addbyte(0x21);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (src_reg & 8)
        {
                addbyte(0x66); /*ANDW dst_reg, src_reg*/
                addbyte(0x44);
                addbyte(0x21);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else
        {
                addbyte(0x66); /*ANDW dst_reg, src_reg*/
                addbyte(0x21);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
}
static void AND_HOST_REG_L(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                addbyte(0x45); /*ANDL dst_reg, src_reg*/
                addbyte(0x21);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (dst_reg & 8)
        {
                addbyte(0x41); /*ANDL dst_reg, src_reg*/
                addbyte(0x21);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (src_reg & 8)
        {
                addbyte(0x44); /*ANDL dst_reg, src_reg*/
                addbyte(0x21);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else
        {
                addbyte(0x21); /*ANDL dst_reg, src_reg*/
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
}
static void AND_HOST_REG_IMM(int host_reg, uint32_t imm)
{
        if (host_reg & 0x10)
        {
                addbyte(0x66); /*ANDW host_reg, imm<<8*/
                if (host_reg & 8)
                        addbyte(0x41);
                addbyte(0x81);
                addbyte(0xe0 | (host_reg & 7));
                addword((imm << 8) | 0xff);
        }
        else
        {
                if (host_reg & 8)
                        addbyte(0x41);
                addbyte(0x81); /*ANDL host_reg, imm*/
                addbyte(0xe0 | (host_reg & 7));
                addlong(imm);
        }
}

static int TEST_HOST_REG_B(int dst_reg, int src_reg)
{
        if (dst_reg & 8)
        {
                addbyte(0x44); /*MOV EDX, dst_reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((dst_reg & 7) << 3) | REG_EDX);
                
                dst_reg = (dst_reg & 0x10) | REG_EDX;
        }
        
        AND_HOST_REG_B(dst_reg, src_reg);
        
        return dst_reg & ~0x10;
}
static int TEST_HOST_REG_W(int dst_reg, int src_reg)
{
        if (dst_reg & 8)
        {
                addbyte(0x44); /*MOV EDX, dst_reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((dst_reg & 7) << 3) | REG_EDX);
                
                dst_reg = REG_EDX;
        }
        
        AND_HOST_REG_W(dst_reg, src_reg);
        
        return dst_reg;
}
static int TEST_HOST_REG_L(int dst_reg, int src_reg)
{
        if (dst_reg & 8)
        {
                addbyte(0x44); /*MOV EDX, dst_reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((dst_reg & 7) << 3) | REG_EDX);
                
                dst_reg = REG_EDX;
        }
        
        AND_HOST_REG_L(dst_reg, src_reg);
        
        return dst_reg;
}
static int TEST_HOST_REG_IMM(int host_reg, uint32_t imm)
{
        if (host_reg & 8)
        {
                addbyte(0x44); /*MOV EDX, host_reg*/
                addbyte(0x89);
                addbyte(0xc0 | REG_EDX | ((host_reg & 7) << 3));
                host_reg = REG_EDX | (host_reg & 0x10);
        }
        if (host_reg & 0x10)
        {
                addbyte(0x66); /*ANDW host_reg, imm<<8*/
                addbyte(0x81);
                addbyte(0xe0 | (host_reg & 7));
                addword((imm << 8) | 0xff);
        }
        else
        {
                addbyte(0x81); /*ANDL host_reg, imm*/
                addbyte(0xe0 | (host_reg & 7));
                addlong(imm);
        }
        
        return host_reg;
}

static void OR_HOST_REG_B(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0x66); /*MOVW AX, src_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((src_reg & 7) << 3));
                        if (!(src_reg & 0x10))
                        {
                                addbyte(0x66); /*SHL AX, 8*/
                                addbyte(0xc1);
                                addbyte(0xe0);
                                addbyte(8);
                        }
                        else
                        {
                                addbyte(0x66); /*AND AX, 0xff00*/
                                addbyte(0x25);
                                addword(0xff00);
                        }
                        addbyte(0x66); /*ORW dst_reg, AX*/
                        addbyte(0x41);
                        addbyte(0x09);
                        addbyte(0xc0 | (dst_reg & 7));
                }
                else if (src_reg & 0x10)
                {
                        addbyte(0x66); /*MOVW AX, src_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((src_reg & 7) << 3));
                        addbyte(0x66); /*SHR AX, 8*/
                        addbyte(0xc1);
                        addbyte(0xe8);
                        addbyte(8);
                        addbyte(0x41); /*ORB dst_reg, AL*/
                        addbyte(0x08);
                        addbyte(0xc0 | (dst_reg & 7));
                }
                else
                {
                        addbyte(0x45); /*ORB dst_reg, src_reg*/
                        addbyte(0x08);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else if (dst_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0x66); /*SHL src_reg, 8*/
                        addbyte(0xc1);
                        addbyte(0xe0 | src_reg);
                        addbyte(0x08);
                        addbyte(0x66); /*ORW dst_reg, src_reg*/
                        addbyte(0x41);
                        addbyte(0x09);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
                else
                {
                        addbyte(0x41); /*ORB dst_reg, src_reg*/
                        addbyte(0x08);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else
                fatal("!(dst_reg & src_reg & 8)\n");
}
static void OR_HOST_REG_W(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                addbyte(0x66); /*ORW dst_reg, src_reg*/
                addbyte(0x45);
                addbyte(0x09);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (dst_reg & 8)
        {
                addbyte(0x66); /*ORW dst_reg, src_reg*/
                addbyte(0x41);
                addbyte(0x09);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else
                fatal("!(dst_reg & src_reg & 8)\n");
}
static void OR_HOST_REG_L(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                addbyte(0x45); /*ORL dst_reg, src_reg*/
                addbyte(0x09);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (dst_reg & 8)
        {
                addbyte(0x41); /*ORL dst_reg, src_reg*/
                addbyte(0x09);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else
                fatal("!(dst_reg & src_reg & 8)\n");
}
static void OR_HOST_REG_IMM(int host_reg, uint32_t imm)
{
        if (host_reg & 0x10)
        {
                addbyte(0x66); /*ORW host_reg, imm<<8*/
                addbyte(0x41);
                addbyte(0x81);
                addbyte(0xc8 | (host_reg & 7));
                addword(imm << 8);
        }
        else if (host_reg & 8)
        {
                addbyte(0x41); /*ORL host_reg, imm*/
                addbyte(0x81);
                addbyte(0xc8 | (host_reg & 7));
                addlong(imm);
        }
        else
                fatal("OR to bad register\n");
}

static void XOR_HOST_REG_B(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0x66); /*MOVW AX, src_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((src_reg & 7) << 3));
                        if (!(src_reg & 0x10))
                        {
                                addbyte(0x66); /*SHL AX, 8*/
                                addbyte(0xc1);
                                addbyte(0xe0);
                                addbyte(8);
                        }
                        else
                        {
                                addbyte(0x66); /*AND AX, 0xff00*/
                                addbyte(0x25);
                                addword(0xff00);
                        }
                        addbyte(0x66); /*XORW dst_reg, AX*/
                        addbyte(0x41);
                        addbyte(0x31);
                        addbyte(0xc0 | (dst_reg & 7));
                }
                else if (src_reg & 0x10)
                {
                        addbyte(0x66); /*MOVW AX, src_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((src_reg & 7) << 3));
                        addbyte(0x66); /*SHR AX, 8*/
                        addbyte(0xc1);
                        addbyte(0xe8);
                        addbyte(8);
                        addbyte(0x41); /*XORB dst_reg, AL*/
                        addbyte(0x30);
                        addbyte(0xc0 | (dst_reg & 7));
                }
                else
                {
                        addbyte(0x45); /*XORB dst_reg, src_reg*/
                        addbyte(0x30);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else if (dst_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0x66); /*SHL src_reg, 8*/
                        addbyte(0xc1);
                        addbyte(0xe0 | src_reg);
                        addbyte(0x08);
                        addbyte(0x66); /*XORW dst_reg, src_reg*/
                        addbyte(0x41);
                        addbyte(0x31);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
                else
                {
                        addbyte(0x41); /*XORB dst_reg, src_reg*/
                        addbyte(0x30);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else
                fatal("!(dst_reg & src_reg & 8)\n");
}
static void XOR_HOST_REG_W(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                addbyte(0x66); /*XORW dst_reg, src_reg*/
                addbyte(0x45);
                addbyte(0x31);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (dst_reg & 8)
        {
                addbyte(0x66); /*XORW dst_reg, src_reg*/
                addbyte(0x41);
                addbyte(0x31);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else
                fatal("!(dst_reg & src_reg & 8)\n");
}
static void XOR_HOST_REG_L(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                addbyte(0x45); /*XORL dst_reg, src_reg*/
                addbyte(0x31);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (dst_reg & 8)
        {
                addbyte(0x41); /*XORL dst_reg, src_reg*/
                addbyte(0x31);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else
                fatal("!(dst_reg & src_reg & 8)\n");
}
static void XOR_HOST_REG_IMM(int host_reg, uint32_t imm)
{
        if (host_reg & 0x10)
        {
                addbyte(0x66); /*ORW host_reg, imm<<8*/
                addbyte(0x41);
                addbyte(0x81);
                addbyte(0xf0 | (host_reg & 7));
                addword(imm << 8);
        }
        else if (host_reg & 8)
        {
                addbyte(0x41); /*ORL host_reg, imm*/
                addbyte(0x81);
                addbyte(0xf0 | (host_reg & 7));
                addlong(imm);
        }
        else
                fatal("XOR to bad register\n");
}

static void ADD_HOST_REG_B(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0x66); /*MOVW AX, src_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((src_reg & 7) << 3));
                        if (!(src_reg & 0x10))
                        {
                                addbyte(0x66); /*SHL AX, 8*/
                                addbyte(0xc1);
                                addbyte(0xe0);
                                addbyte(8);
                        }
                        else
                        {
                                addbyte(0x66); /*AND AX, 0xff00*/
                                addbyte(0x25);
                                addword(0xff00);
                        }
                        addbyte(0x66); /*ADDW dst_reg, AX*/
                        addbyte(0x41);
                        addbyte(0x01);
                        addbyte(0xc0 | (dst_reg & 7));
                }
                else if (src_reg & 0x10)
                {
                        addbyte(0x66); /*MOVW AX, src_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((src_reg & 7) << 3));
                        addbyte(0x66); /*SHR AX, 8*/
                        addbyte(0xc1);
                        addbyte(0xe8);
                        addbyte(8);
                        addbyte(0x41); /*ADDB dst_reg, AL*/
                        addbyte(0x00);
                        addbyte(0xc0 | (dst_reg & 7));
                }
                else
                {
                        addbyte(0x45); /*ADDB dst_reg, src_reg*/
                        addbyte(0x00);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else if (dst_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0x66); /*SHL src_reg, 8*/
                        addbyte(0xc1);
                        addbyte(0xe0 | src_reg);
                        addbyte(0x08);
                        addbyte(0x66); /*ADDW dst_reg, src_reg*/
                        addbyte(0x41);
                        addbyte(0x01);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
                else
                {
                        addbyte(0x41); /*ADDB dst_reg, src_reg*/
                        addbyte(0x00);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else
                fatal("!(dst_reg & src_reg & 8)\n");
}
static void ADD_HOST_REG_W(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                addbyte(0x66); /*ADDW dst_reg, src_reg*/
                addbyte(0x45);
                addbyte(0x01);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (dst_reg & 8)
        {
                addbyte(0x66); /*ADDW dst_reg, src_reg*/
                addbyte(0x41);
                addbyte(0x01);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else
                fatal("!(dst_reg & src_reg & 8)\n");
}
static void ADD_HOST_REG_L(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                addbyte(0x45); /*ADDL dst_reg, src_reg*/
                addbyte(0x01);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (dst_reg & 8)
        {
                addbyte(0x41); /*ADDL dst_reg, src_reg*/
                addbyte(0x01);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else
                fatal("!(dst_reg & src_reg & 8)\n");
}

static void SUB_HOST_REG_B(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0x66); /*MOVW AX, src_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((src_reg & 7) << 3));
                        if (!(src_reg & 0x10))
                        {
                                addbyte(0x66); /*SHL AX, 8*/
                                addbyte(0xc1);
                                addbyte(0xe0);
                                addbyte(8);
                        }
                        else
                        {
                                addbyte(0x66); /*AND AX, 0xff00*/
                                addbyte(0x25);
                                addword(0xff00);
                        }
                        addbyte(0x66); /*SUBW dst_reg, AX*/
                        addbyte(0x41);
                        addbyte(0x29);
                        addbyte(0xc0 | (dst_reg & 7));
                }
                else if (src_reg & 0x10)
                {
                        addbyte(0x66); /*MOVW AX, src_reg*/
                        addbyte(0x44);
                        addbyte(0x89);
                        addbyte(0xc0 | ((src_reg & 7) << 3));
                        addbyte(0x66); /*SHR AX, 8*/
                        addbyte(0xc1);
                        addbyte(0xe8);
                        addbyte(8);
                        addbyte(0x41); /*SUBB dst_reg, AL*/
                        addbyte(0x28);
                        addbyte(0xc0 | (dst_reg & 7));
                }
                else
                {
                        addbyte(0x45); /*SUBB dst_reg, src_reg*/
                        addbyte(0x28);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else if (dst_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0x66); /*SHL src_reg, 8*/
                        addbyte(0xc1);
                        addbyte(0xe0 | src_reg);
                        addbyte(0x08);
                        addbyte(0x66); /*SUBW dst_reg, src_reg*/
                        addbyte(0x41);
                        addbyte(0x29);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
                else
                {
                        addbyte(0x41); /*SUBB dst_reg, src_reg*/
                        addbyte(0x28);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }
        }
        else if (src_reg & 8)
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0xc1); /*SHR dst_reg, 8*/
                        addbyte(0xe8 | (dst_reg & 7));
                        addbyte(8);
                }
                if (src_reg & 0x10)
                {
                        addbyte(0x41); /*MOVZX EBX, src_reg*/
                        addbyte(0x0f);
                        addbyte(0xb7);
                        addbyte(0xd8 | (src_reg & 7));
                        addbyte(0xc1); /*SHR EBX, 8*/
                        addbyte(0xeb);
                        addbyte(8);
                        addbyte(0x28); /*SUBB dst_reg, EBX*/
                        addbyte(0xd8 | (dst_reg & 7));
                }
                else
                {
                        addbyte(0x44); /*SUBB dst_reg, src_reg*/
                        addbyte(0x28);
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }                
        }
        else
        {
                if (dst_reg & 0x10)
                {
                        addbyte(0xc1); /*SHR dst_reg, 8*/
                        addbyte(0xe8 | (dst_reg & 7));
                        addbyte(8);
                }
                if (src_reg & 0x10)
                {
                        addbyte(0x0f); /*MOVZX EBX, src_reg*/
                        addbyte(0xb7);
                        addbyte(0xd8 | (src_reg & 7));
                        addbyte(0xc1); /*SHR EBX, 8*/
                        addbyte(0xeb);
                        addbyte(8);
                        addbyte(0x28); /*SUBB dst_reg, EBX*/
                        addbyte(0xd8 | (dst_reg & 7));
                }
                else
                {
                        addbyte(0x28); /*SUBB dst_reg, src_reg*/
                        addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
                }                
        }
        
//                fatal("!(dst_reg & src_reg & 8) subb %i %i\n", dst_reg, src_reg);
}
static void SUB_HOST_REG_W(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                addbyte(0x66); /*SUBW dst_reg, src_reg*/
                addbyte(0x45);
                addbyte(0x29);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (dst_reg & 8)
        {
                addbyte(0x66); /*SUBW dst_reg, src_reg*/
                addbyte(0x41);
                addbyte(0x29);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (src_reg & 8)
        {
                addbyte(0x66); /*SUBW dst_reg, src_reg*/
                addbyte(0x44);
                addbyte(0x29);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else
        {
                addbyte(0x66); /*SUBW dst_reg, src_reg*/
                addbyte(0x29);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
}
static void SUB_HOST_REG_L(int dst_reg, int src_reg)
{
        if (dst_reg & src_reg & 8)
        {
                addbyte(0x45); /*SUBL dst_reg, src_reg*/
                addbyte(0x29);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (dst_reg & 8)
        {
                addbyte(0x41); /*SUBL dst_reg, src_reg*/
                addbyte(0x29);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else if (src_reg & 8)
        {
                addbyte(0x44); /*SUBL dst_reg, src_reg*/
                addbyte(0x29);
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
        else
        {
                addbyte(0x29); /*SUBL dst_reg, src_reg*/
                addbyte(0xc0 | (dst_reg & 7) | ((src_reg & 7) << 3));
        }
}

static int CMP_HOST_REG_B(int dst_reg, int src_reg)
{
        if (dst_reg & 8)
        {
                addbyte(0x44); /*MOV EDX, dst_reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((dst_reg & 7) << 3) | REG_EDX);
                
                dst_reg = (dst_reg & 0x10) | REG_EDX;
        }
        
        SUB_HOST_REG_B(dst_reg, src_reg);
        
        return dst_reg & ~0x10;
}
static int CMP_HOST_REG_W(int dst_reg, int src_reg)
{
        if (dst_reg & 8)
        {
                addbyte(0x44); /*MOV EDX, dst_reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((dst_reg & 7) << 3) | REG_EDX);
                
                dst_reg = REG_EDX;
        }
        
        SUB_HOST_REG_W(dst_reg, src_reg);
        
        return dst_reg;
}
static int CMP_HOST_REG_L(int dst_reg, int src_reg)
{
        if (dst_reg & 8)
        {
                addbyte(0x44); /*MOV EDX, dst_reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((dst_reg & 7) << 3) | REG_EDX);
                
                dst_reg = REG_EDX;
        }
        
        SUB_HOST_REG_L(dst_reg, src_reg);
        
        return dst_reg;
}

static void ADD_HOST_REG_IMM_B(int host_reg, uint8_t imm)
{
        if (host_reg & 0x10)
        {
                addbyte(0x66); /*ADDW host_reg, imm*/
                addbyte(0x41);
                addbyte(0x81);
                addbyte(0xC0 | (host_reg & 7));
                addword(imm << 8);
        }
        else
        {
                addbyte(0x41); /*ADDB host_reg, imm*/
                addbyte(0x80);
                addbyte(0xC0 | (host_reg & 7));
                addbyte(imm);
        }
}
static void ADD_HOST_REG_IMM_W(int host_reg, uint16_t imm)
{
        addbyte(0x66); /*ADDW host_reg, imm*/
        addbyte(0x41);
        addbyte(0x81);
        addbyte(0xC0 | (host_reg & 7));
        addword(imm);
}
static void ADD_HOST_REG_IMM(int host_reg, uint32_t imm)
{
        addbyte(0x41); /*ADDL host_reg, imm*/
        addbyte(0x81);
        addbyte(0xC0 | (host_reg & 7));
        addlong(imm);
}

static void SUB_HOST_REG_IMM_B(int host_reg, uint8_t imm)
{
        if (host_reg & 0x10)
        {
                addbyte(0x66); /*SUBW host_reg, imm*/
                if (host_reg & 8)
                        addbyte(0x41);
                addbyte(0x81);
                addbyte(0xE8 | (host_reg & 7));
                addword(imm << 8);
        }
        else
        {
                if (host_reg & 8)
                        addbyte(0x41);
                addbyte(0x80); /*SUBB host_reg, imm*/
                addbyte(0xE8 | (host_reg & 7));
                addbyte(imm);
        }
}
static void SUB_HOST_REG_IMM_W(int host_reg, uint16_t imm)
{
        addbyte(0x66); /*SUBW host_reg, imm*/
        if (host_reg & 8)
                addbyte(0x41);
        addbyte(0x81);
        addbyte(0xE8 | (host_reg & 7));
        addword(imm);
}
static void SUB_HOST_REG_IMM(int host_reg, uint32_t imm)
{
        if (host_reg & 8)
                addbyte(0x41);
        addbyte(0x81); /*SUBL host_reg, imm*/
        addbyte(0xE8 | (host_reg & 7));
        addlong(imm);
}

static int CMP_HOST_REG_IMM_B(int host_reg, uint8_t imm)
{
        if (host_reg & 8)
        {
                addbyte(0x44); /*MOV EDX, dst_reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((host_reg & 7) << 3) | REG_EDX);
                
                host_reg = (host_reg & 0x10) | REG_EDX;
        }
       
        SUB_HOST_REG_IMM_B(host_reg, imm);
        
        return host_reg;// & ~0x10;
}
static int CMP_HOST_REG_IMM_W(int host_reg, uint16_t imm)
{
        if (host_reg & 8)
        {
                addbyte(0x44); /*MOV EDX, dst_reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((host_reg & 7) << 3) | REG_EDX);
                
                host_reg = REG_EDX;
        }
        
        SUB_HOST_REG_IMM_W(host_reg, imm);
        
        return host_reg;
}
static int CMP_HOST_REG_IMM_L(int host_reg, uint32_t imm)
{
        if (host_reg & 8)
        {
                addbyte(0x44); /*MOV EDX, dst_reg*/
                addbyte(0x89);
                addbyte(0xc0 | ((host_reg & 7) << 3) | REG_EDX);
                
                host_reg = REG_EDX;
        }
        
        SUB_HOST_REG_IMM(host_reg, imm);
        
        return host_reg;
}

static void LOAD_STACK_TO_EA(int off)
{
        if (stack32)
        {
                addbyte(0x8b); /*MOVL EAX,[ESP]*/
                addbyte(0x45 | (REG_EAX << 3));
                addbyte((uint32_t)&ESP - (uint32_t)&EAX);
                if (off)
                {
                        addbyte(0x83); /*ADD EAX, off*/
                        addbyte(0xc0 | (0 << 3) | REG_EAX);
                        addbyte(off);
                }
        }
        else
        {
                addbyte(0x0f); /*MOVZX EAX,W[ESP]*/
                addbyte(0xb7);
                addbyte(0x45 | (REG_EAX << 3));
                addbyte((uint32_t)&ESP - (uint32_t)&EAX);
                if (off)
                {
                        addbyte(0x66); /*ADD AX, off*/
                        addbyte(0x05);
                        addword(off);
                }
        }
}
static void LOAD_EBP_TO_EA(int off)
{
        if (stack32)
        {
                addbyte(0x8b); /*MOVL EAX,[EBP]*/
                addbyte(0x45 | (REG_EAX << 3));
                addbyte((uint32_t)&EBP - (uint32_t)&EAX);
                if (off)
                {
                        addbyte(0x83); /*ADD EAX, off*/
                        addbyte(0xc0 | (0 << 3) | REG_EAX);
                        addbyte(off);
                }
        }
        else
        {
                addbyte(0x0f); /*MOVZX EAX,W[EBP]*/
                addbyte(0xb7);
                addbyte(0x45 | (REG_EAX << 3));
                addbyte((uint32_t)&EBP - (uint32_t)&EAX);
                if (off)
                {
                        addbyte(0x66); /*ADD AX, off*/
                        addbyte(0x05);
                        addword(off);
                }
        }
}

static void SP_MODIFY(int off)
{
        if (stack32)
        {
                if (off < 0x80)
                {
                        addbyte(0x83); /*ADD [ESP], off*/
                        addbyte(0x45);
                        addbyte((uint32_t)&ESP - (uint32_t)&EAX);
                        addbyte(off);
                }
                else
                {
                        addbyte(0x81); /*ADD [ESP], off*/
                        addbyte(0x45);
                        addbyte((uint32_t)&ESP - (uint32_t)&EAX);
                        addlong(off);
                }
        }
        else
        {
                if (off < 0x80)
                {
                        addbyte(0x66); /*ADD [SP], off*/
                        addbyte(0x83);
                        addbyte(0x45);
                        addbyte((uint32_t)&SP - (uint32_t)&EAX);
                        addbyte(off);
                }
                else
                {
                        addbyte(0x66); /*ADD [SP], off*/
                        addbyte(0x81);
                        addbyte(0x45);
                        addbyte((uint32_t)&SP - (uint32_t)&EAX);
                        addword(off);
                }
        }
}

static void TEST_ZERO_JUMP_W(int host_reg, uint32_t new_pc, int taken_cycles)
{
        addbyte(0x66); /*CMPW host_reg, 0*/
        if (host_reg & 8)
                addbyte(0x41);
        addbyte(0x83);
        addbyte(0xc0 | 0x38 | (host_reg & 7));
        addbyte(0);
        addbyte(0x75); /*JNZ +*/
        addbyte(11+5+(taken_cycles ? 8 : 0));
        addbyte(0xC7); /*MOVL [pc], new_pc*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&pc);
        addlong(new_pc);
        if (taken_cycles)
        {
                addbyte(0x83); /*SUB $codegen_block_cycles, cyclcs*/
                addbyte(0x2c);
                addbyte(0x25);
                addlong((uintptr_t)&cycles);
                addbyte(taken_cycles);
        }
        addbyte(0xe9); /*JMP end*/
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
}
static void TEST_ZERO_JUMP_L(int host_reg, uint32_t new_pc, int taken_cycles)
{
        if (host_reg & 8)
                addbyte(0x41);
        addbyte(0x83); /*CMPW host_reg, 0*/
        addbyte(0xc0 | 0x38 | (host_reg & 7));
        addbyte(0);
        addbyte(0x75); /*JNZ +*/
        addbyte(11+5+(taken_cycles ? 8 : 0));
        addbyte(0xC7); /*MOVL [pc], new_pc*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&pc);
        addlong(new_pc);
        if (taken_cycles)
        {
                addbyte(0x83); /*SUB $codegen_block_cycles, cyclcs*/
                addbyte(0x2c);
                addbyte(0x25);
                addlong((uintptr_t)&cycles);
                addbyte(taken_cycles);
        }
        addbyte(0xe9); /*JMP end*/
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
}

static void TEST_NONZERO_JUMP_W(int host_reg, uint32_t new_pc, int taken_cycles)
{
        addbyte(0x66); /*CMPW host_reg, 0*/
        if (host_reg & 8)
                addbyte(0x41);
        addbyte(0x83);
        addbyte(0xc0 | 0x38 | (host_reg & 7));
        addbyte(0);
        addbyte(0x74); /*JZ +*/
        addbyte(11+5+(taken_cycles ? 8 : 0));
        addbyte(0xC7); /*MOVL [pc], new_pc*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&pc);
        addlong(new_pc);
        if (taken_cycles)
        {
                addbyte(0x83); /*SUB $codegen_block_cycles, cyclcs*/
                addbyte(0x2c);
                addbyte(0x25);
                addlong((uintptr_t)&cycles);
                addbyte(taken_cycles);
        }
        addbyte(0xe9); /*JMP end*/
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
}
static void TEST_NONZERO_JUMP_L(int host_reg, uint32_t new_pc, int taken_cycles)
{
        if (host_reg & 8)
                addbyte(0x41);
        addbyte(0x83); /*CMPW host_reg, 0*/
        addbyte(0xc0 | 0x38 | (host_reg & 7));
        addbyte(0);
        addbyte(0x74); /*JZ +*/
        addbyte(11+5+(taken_cycles ? 8 : 0));
        addbyte(0xC7); /*MOVL [pc], new_pc*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&pc);
        addlong(new_pc);
        if (taken_cycles)
        {
                addbyte(0x83); /*SUB $codegen_block_cycles, cyclcs*/
                addbyte(0x2c);
                addbyte(0x25);
                addlong((uintptr_t)&cycles);
                addbyte(taken_cycles);
        }
        addbyte(0xe9); /*JMP end*/
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
}

static int BRANCH_COND_BE(int pc_offset, uint32_t op_pc, uint32_t offset, int not)
{
        if (codegen_flags_changed && flags_op != FLAGS_UNKNOWN)
        {
                addbyte(0x83); /*CMP flags_res, 0*/
                addbyte(0x3c);
                addbyte(0x25);
                addlong((uintptr_t)&flags_res);
                addbyte(0);
                addbyte(0x74); /*JZ +*/
        }
        else
        {
                CALL_FUNC(ZF_SET);
                addbyte(0x85); /*TEST EAX,EAX*/
                addbyte(0xc0);
                addbyte(0x75); /*JNZ +*/
        }
        if (not)
                addbyte(12+2+2+11+5+(timing_bt ? 8 : 0));
        else
                addbyte(12+2+2);
        CALL_FUNC(CF_SET);
        addbyte(0x85); /*TEST EAX,EAX*/
        addbyte(0xc0);
        if (not)
                addbyte(0x75); /*JNZ +*/
        else
                addbyte(0x74); /*JZ +*/
        addbyte(11+5+(timing_bt ? 8 : 0));        
        addbyte(0xC7); /*MOVL [pc], new_pc*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&pc);
        addlong(op_pc+pc_offset+offset);
        if (timing_bt)
        {
                addbyte(0x83); /*SUB $codegen_block_cycles, cyclcs*/
                addbyte(0x2c);
                addbyte(0x25);
                addlong((uintptr_t)&cycles);
                addbyte(timing_bt);
        }
        addbyte(0xe9); /*JMP end*/
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
}

static int BRANCH_COND_L(int pc_offset, uint32_t op_pc, uint32_t offset, int not)
{
        CALL_FUNC(NF_SET);
        addbyte(0x85); /*TEST EAX,EAX*/
        addbyte(0xc0);
        addbyte(0x0f); /*SETNE BL*/
        addbyte(0x95);
        addbyte(0xc3);
        CALL_FUNC(VF_SET);
        addbyte(0x85); /*TEST EAX,EAX*/
        addbyte(0xc0);
        addbyte(0x0f); /*SETNE AL*/
        addbyte(0x95);
        addbyte(0xc0);
        addbyte(0x38); /*CMP AL, BL*/
        addbyte(0xd8);
        if (not)
                addbyte(0x75); /*JNZ +*/
        else
                addbyte(0x74); /*JZ +*/
        addbyte(11+5+(timing_bt ? 8 : 0));        
        addbyte(0xC7); /*MOVL [pc], new_pc*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&pc);
        addlong(op_pc+pc_offset+offset);
        if (timing_bt)
        {
                addbyte(0x83); /*SUB $codegen_block_cycles, cyclcs*/
                addbyte(0x2c);
                addbyte(0x25);
                addlong((uintptr_t)&cycles);
                addbyte(timing_bt);
        }
        addbyte(0xe9); /*JMP end*/
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
}

static int BRANCH_COND_LE(int pc_offset, uint32_t op_pc, uint32_t offset, int not)
{
        if (codegen_flags_changed && flags_op != FLAGS_UNKNOWN)
        {
                addbyte(0x83); /*CMP flags_res, 0*/
                addbyte(0x3c);
                addbyte(0x25);
                addlong((uintptr_t)&flags_res);
                addbyte(0);
                addbyte(0x74); /*JZ +*/
        }
        else
        {
                CALL_FUNC(ZF_SET);
                addbyte(0x85); /*TEST EAX,EAX*/
                addbyte(0xc0);
                addbyte(0x75); /*JNZ +*/
        }
        if (not)
                addbyte(12+2+3+12+2+3+2+2+11+5+(timing_bt ? 8 : 0));
        else
                addbyte(12+2+3+12+2+3+2+2);

        CALL_FUNC(NF_SET);
        addbyte(0x85); /*TEST EAX,EAX*/
        addbyte(0xc0);
        addbyte(0x0f); /*SETNE BL*/
        addbyte(0x95);
        addbyte(0xc3);
        CALL_FUNC(VF_SET);
        addbyte(0x85); /*TEST EAX,EAX*/
        addbyte(0xc0);
        addbyte(0x0f); /*SETNE AL*/
        addbyte(0x95);
        addbyte(0xc0);
        addbyte(0x38); /*CMP AL, BL*/
        addbyte(0xd8);
        if (not)
                addbyte(0x75); /*JNZ +*/
        else
                addbyte(0x74); /*JZ +*/
        addbyte(11+5+(timing_bt ? 8 : 0));        
        addbyte(0xC7); /*MOVL [pc], new_pc*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&pc);
        addlong(op_pc+pc_offset+offset);
        if (timing_bt)
        {
                addbyte(0x83); /*SUB $codegen_block_cycles, cyclcs*/
                addbyte(0x2c);
                addbyte(0x25);
                addlong((uintptr_t)&cycles);
                addbyte(timing_bt);
        }
        addbyte(0xe9); /*JMP end*/
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
}

static int LOAD_VAR_W(uintptr_t addr)
{
        int host_reg = REG_EBX;

        addbyte(0x0f); /*MOVZX host_reg,[reg]*/
        addbyte(0xb7);
        addbyte(0x04 | (host_reg << 3));
        addbyte(0x25);
        addlong((uint32_t)addr);

        return host_reg;
}
static int LOAD_VAR_L(uintptr_t addr)
{
        int host_reg = REG_EBX;

        addbyte(0x8b); /*MOVL host_reg,[reg]*/
        addbyte(0x04 | (host_reg << 3));
        addbyte(0x25);
        addlong((uint32_t)addr);

        return host_reg;
}

static int COPY_REG(int src_reg)
{
        if (src_reg & 8)
                addbyte(0x44);
        addbyte(0x89);
        addbyte(0xc0 | REG_ECX | ((src_reg & 7) << 3));
        
        return REG_ECX | (src_reg & 0x10);
}

static int ZERO_EXTEND_W_B(int reg)
{
        if (reg & 0x10)
        {
                addbyte(0x44); /*MOV EAX, reg*/
                addbyte(0x89);
                addbyte(0xc0 | (reg << 3));
                addbyte(0x0f); /*MOVZX EAX, AH*/
                addbyte(0xb6);
                addbyte(0xc4);
                
                return REG_EAX;
        }
        
        if (reg & 8)
                addbyte(0x41);
        addbyte(0x0f); /*MOVZX regl, regb*/
        addbyte(0xb6);
        addbyte(0xc0 | (reg & 7));
        
        return REG_EAX;
}
static int ZERO_EXTEND_L_B(int reg)
{
        if (reg & 0x10)
        {
                addbyte(0x44); /*MOV EAX, reg*/
                addbyte(0x89);
                addbyte(0xc0 | (reg << 3));
                addbyte(0x0f); /*MOVZX EAX, AH*/
                addbyte(0xb6);
                addbyte(0xc4);
                
                return REG_EAX;
        }

        if (reg & 8)
                addbyte(0x41);
        addbyte(0x0f); /*MOVZX regl, regb*/
        addbyte(0xb6);
        addbyte(0xc0 | (reg & 7));
        
        return REG_EAX;
}
static int ZERO_EXTEND_L_W(int reg)
{
        if (reg & 8)
                addbyte(0x41);
        addbyte(0x0f); /*MOVZX regl, regw*/
        addbyte(0xb7);
        addbyte(0xc0 | (reg & 7));
        
        return REG_EAX;
}

static int SIGN_EXTEND_W_B(int reg)
{
        if (reg & 0x10)
        {
                addbyte(0x44); /*MOV EAX, reg*/
                addbyte(0x89);
                addbyte(0xc0 | (reg << 3));
                addbyte(0x0f); /*MOVSX EAX, AH*/
                addbyte(0xbe);
                addbyte(0xc4);
                
                return REG_EAX;
        }

        if (reg & 8)
                addbyte(0x41);
        addbyte(0x0f); /*MOVSX regl, regb*/
        addbyte(0xbe);
        addbyte(0xc0 | (reg & 7));
        
        return REG_EAX;
}
static int SIGN_EXTEND_L_B(int reg)
{
        if (reg & 0x10)
        {
                addbyte(0x44); /*MOV EAX, reg*/
                addbyte(0x89);
                addbyte(0xc0 | (reg << 3));
                addbyte(0x0f); /*MOVSX EAX, AH*/
                addbyte(0xbe);
                addbyte(0xc4);
                
                return REG_EAX;
        }

        if (reg & 8)
                addbyte(0x41);
        addbyte(0x0f); /*MOVSX regl, regb*/
        addbyte(0xbe);
        addbyte(0xc0 | (reg & 7));
        
        return REG_EAX;
}
static int SIGN_EXTEND_L_W(int reg)
{
        if (reg & 8)
                addbyte(0x41);
        addbyte(0x0f); /*MOVSX regl, regw*/
        addbyte(0xbf);
        addbyte(0xc0 | (reg & 7));
        
        return REG_EAX;
}

static void SHL_B_IMM(int reg, int count)
{
        if (reg & 0x10)
        {
                addbyte(0x44); /*MOV EAX, reg*/
                addbyte(0x89);
                addbyte(0xc0 | REG_EAX | ((reg & 7) << 3));
                addbyte(0xc0); /*SHL AH, count*/
                addbyte(0xe0 | REG_AH);
                addbyte(count);
                addbyte(0x41); /*MOV reg, EAX*/
                addbyte(0x89);
                addbyte(0xc0 | (REG_EAX << 3) | (reg & 7));
        }
        else
        {
                if (reg & 8)
                        addbyte(0x41);
                addbyte(0xc0); /*SHL reg, count*/
                addbyte(0xc0 | (reg & 7) | 0x20);
                addbyte(count);
        }
}
static void SHL_W_IMM(int reg, int count)
{
        addbyte(0x66); /*SHL reg, count*/
        if (reg & 8)
                addbyte(0x41);
        addbyte(0xc1);
        addbyte(0xc0 | (reg & 7) | 0x20);
        addbyte(count);
}
static void SHL_L_IMM(int reg, int count)
{
        if (reg & 8)
                addbyte(0x41);
        addbyte(0xc1); /*SHL reg, count*/
        addbyte(0xc0 | (reg & 7) | 0x20);
        addbyte(count);
}
static void SHR_B_IMM(int reg, int count)
{
        if (reg & 0x10)
        {
                addbyte(0x44); /*MOV EAX, reg*/
                addbyte(0x89);
                addbyte(0xc0 | REG_EAX | ((reg & 7) << 3));
                addbyte(0xc0); /*SHR AH, count*/
                addbyte(0xe8 | REG_AH);
                addbyte(count);
                addbyte(0x41); /*MOV reg, EAX*/
                addbyte(0x89);
                addbyte(0xc0 | (REG_EAX << 3) | (reg & 7));
        }
        else
        {
                if (reg & 8)
                        addbyte(0x41);
                addbyte(0xc0); /*SHR reg, count*/
                addbyte(0xc0 | (reg & 7) | 0x28);
                addbyte(count);
        }
}
static void SHR_W_IMM(int reg, int count)
{
        addbyte(0x66); /*SHR reg, count*/
        if (reg & 8)
                addbyte(0x41);
        addbyte(0xc1);
        addbyte(0xc0 | (reg & 7) | 0x28);
        addbyte(count);
}
static void SHR_L_IMM(int reg, int count)
{
        if (reg & 8)
                addbyte(0x41);
        addbyte(0xc1); /*SHR reg, count*/
        addbyte(0xc0 | (reg & 7) | 0x28);
        addbyte(count);
}
static void SAR_B_IMM(int reg, int count)
{
        if (reg & 0x10)
        {
                addbyte(0x44); /*MOV EAX, reg*/
                addbyte(0x89);
                addbyte(0xc0 | REG_EAX | ((reg & 7) << 3));
                addbyte(0xc0); /*SAR AH, count*/
                addbyte(0xf8 | REG_AH);
                addbyte(count);
                addbyte(0x41); /*MOV reg, EAX*/
                addbyte(0x89);
                addbyte(0xc0 | (REG_EAX << 3) | (reg & 7));
        }
        else
        {
                if (reg & 8)
                        addbyte(0x41);
                addbyte(0xc0); /*SAR reg, count*/
                addbyte(0xc0 | (reg & 7) | 0x38);
                addbyte(count);
        }
}
static void SAR_W_IMM(int reg, int count)
{
        addbyte(0x66); /*SAR reg, count*/
        if (reg & 8)
                addbyte(0x41);
        addbyte(0xc1);
        addbyte(0xc0 | (reg & 7) | 0x38);
        addbyte(count);
}
static void SAR_L_IMM(int reg, int count)
{
        if (reg & 8)
                addbyte(0x41);
        addbyte(0xc1); /*SAR reg, count*/
        addbyte(0xc0 | (reg & 7) | 0x38);
        addbyte(count);
}

static void NEG_HOST_REG_B(int reg)
{
        if (reg & 0x10)
        {
                if (reg & 8)
                        addbyte(0x44);
                addbyte(0x89); /*MOV BX, reg*/
                addbyte(0xc3 | ((reg & 7) << 3));
                addbyte(0xf6); /*NEG BH*/
                addbyte(0xdf);
                if (reg & 8)
                        addbyte(0x41);
                addbyte(0x89); /*MOV reg, BX*/
                addbyte(0xd8 | (reg & 7));
        }
        else
        {
                if (reg & 8)
                        addbyte(0x41);
                addbyte(0xf6);
                addbyte(0xd8 | (reg & 7));
        }
}
static void NEG_HOST_REG_W(int reg)
{
        addbyte(0x66);
        if (reg & 8)
                addbyte(0x41);
        addbyte(0xf7);
        addbyte(0xd8 | (reg & 7));
}
static void NEG_HOST_REG_L(int reg)
{
        if (reg & 8)
                addbyte(0x41);
        addbyte(0xf7);
        addbyte(0xd8 | (reg & 7));
}
        

static void FP_ENTER()
{
        if (codegen_fpu_entered)
                return;
                
        addbyte(0xf6); /*TEST cr0, 0xc*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&cr0);
        addbyte(0xc);
        addbyte(0x74); /*JZ +*/
        addbyte(11+5+12+5);
        addbyte(0xC7); /*MOVL [oldpc],op_old_pc*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&oldpc);
        addlong(op_old_pc);
        load_param_1_32(&codeblock[block_current], 7);
        CALL_FUNC(x86_int);
        addbyte(0xe9); /*JMP end*/
        addlong(BLOCK_EXIT_OFFSET - (block_pos + 4));
        
        codegen_fpu_entered = 1;
}

static void FP_FXCH(int reg)
{
        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x48); /*MOV RSI, ST*/
        addbyte(0xbe);
        addquad((uint64_t)ST);
        addbyte(0x89); /*MOV EBX, EAX*/
        addbyte(0xc3);
        addbyte(0x83); /*ADD EAX, reg*/
        addbyte(0xc0);
        addbyte(reg);

        addbyte(0x48); /*MOV RDX, [RSI + RBX*8]*/
        addbyte(0x8b);
        addbyte(0x14);
        addbyte(0xde);
        addbyte(0x83); /*AND EAX, 7*/
        addbyte(0xe0);
        addbyte(0x07);
        addbyte(0x48); /*MOV RCX, [RSI + RAX*8]*/
        addbyte(0x8b);
        addbyte(0x0c);
        addbyte(0xc6);
        addbyte(0x48); /*MOV [RSI + RAX*8], RDX*/
        addbyte(0x89);
        addbyte(0x14);
        addbyte(0xc6);
        addbyte(0x48); /*MOV [RSI + RBX*8], RCX*/
        addbyte(0x89);
        addbyte(0x0c);
        addbyte(0xde);
        
        addbyte(0x48); /*MOVL ESI, tag*/
        addbyte(0xbe);
        addquad((uintptr_t)tag);
        addbyte(0x8a); /*MOV CL, tag[EAX]*/
        addbyte(0x14);
        addbyte(0x1e);
        addbyte(0x8a); /*MOV DL, tag[EBX]*/
        addbyte(0x0c);
        addbyte(0x06);
        addbyte(0x88); /*MOV tag[EBX], CL*/
        addbyte(0x14);
        addbyte(0x06);
        addbyte(0x88); /*MOV tag[EAX], DL*/
        addbyte(0x0c);
        addbyte(0x1e);

        addbyte(0x48); /*MOV RSI, ST_i64*/
        addbyte(0xbe);
        addquad((uint64_t)ST_i64);
        addbyte(0x48); /*MOV RDX, [RSI + RBX*8]*/
        addbyte(0x8b);
        addbyte(0x14);
        addbyte(0xde);
        addbyte(0x48); /*MOV RCX, [RSI + RAX*8]*/
        addbyte(0x8b);
        addbyte(0x0c);
        addbyte(0xc6);
        addbyte(0x48); /*MOV [RSI + RAX*8], RDX*/
        addbyte(0x89);
        addbyte(0x14);
        addbyte(0xc6);
        addbyte(0x48); /*MOV [RSI + RBX*8], RCX*/
        addbyte(0x89);
        addbyte(0x0c);
        addbyte(0xde);
        reg = reg;
}



static void FP_FLD(int reg)
{
        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x89); /*MOV EBX, EAX*/
        addbyte(0xc3);
        if (reg)
        {
                addbyte(0x83); /*ADD EAX, reg*/
                addbyte(0xc0);
                addbyte(reg);
                addbyte(0x83); /*SUB EBX, 1*/
                addbyte(0xeb);
                addbyte(0x01);
                addbyte(0x83); /*AND EAX, 7*/
                addbyte(0xe0);
                addbyte(0x07);
        }
        else
        {
                addbyte(0x83); /*SUB EBX, 1*/
                addbyte(0xeb);
                addbyte(0x01);
        }       

        addbyte(0x48); /*MOV RCX, ST[EAX*8]*/
        addbyte(0x8b);
        addbyte(0x0c);
        addbyte(0xc5);
        addlong((uintptr_t)ST);
        addbyte(0x83); /*AND EBX, 7*/
        addbyte(0xe3);
        addbyte(0x07);
        addbyte(0x48); /*MOV RDX, ST_i64[EAX*8]*/
        addbyte(0x8b);
        addbyte(0x14);
        addbyte(0xc5);
        addlong((uintptr_t)ST_i64);        
        addbyte(0x8a); /*MOV AL, [tag+EAX]*/
        addbyte(0x80);
        addlong((uintptr_t)tag);
        addbyte(0x48); /*MOV ST[EBX*8], RCX*/
        addbyte(0x89);
        addbyte(0x0c);
        addbyte(0xdd);
        addlong((uintptr_t)ST);
        addbyte(0x48); /*MOV ST_i64[EBX*8], RDX*/
        addbyte(0x89);
        addbyte(0x14);
        addbyte(0xdd);
        addlong((uintptr_t)ST_i64);
        addbyte(0x88); /*MOV [tag+EBX], AL*/
        addbyte(0x83);
        addlong((uintptr_t)tag);

        addbyte(0x89); /*MOV [TOP], EBX*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
}

static void FP_FST(int reg)
{
        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x48); /*MOV RCX, ST[EAX*8]*/
        addbyte(0x8b);
        addbyte(0x0c);
        addbyte(0xc5);
        addlong((uintptr_t)ST);
        addbyte(0x8a); /*MOV BL, [tag+EAX]*/
        addbyte(0x98);
        addlong((uintptr_t)tag);

        if (reg)
        {
                addbyte(0x83); /*ADD EAX, reg*/
                addbyte(0xc0);
                addbyte(reg);
                addbyte(0x83); /*AND EAX, 7*/
                addbyte(0xe0);
                addbyte(0x07);
        }

        addbyte(0x48); /*MOV ST[EAX*8], RCX*/
        addbyte(0x89);
        addbyte(0x0c);
        addbyte(0xc5);
        addlong((uintptr_t)ST);
        addbyte(0x88); /*MOV [tag+EAX], BL*/
        addbyte(0x98);
        addlong((uintptr_t)tag);
}

static void FP_POP()
{
        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0xc6); /*MOVB tag[EAX], 3*/
        addbyte(0x80);
        addlong((uintptr_t)tag);
        addbyte(3);
        addbyte(0x83); /*ADD AL, 1*/
        addbyte(0xc0);
        addbyte(1);
        addbyte(0x83); /*AND AL, 7*/
        addbyte(0xe0);
        addbyte(7);
        addbyte(0x89); /*MOV [TOP], EAX*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
}

static void FP_LOAD_S()
{
        addbyte(0x8b); /*MOV EBX, TOP*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x66); /*MOVD XMM0, EAX*/
        addbyte(0x0f);
        addbyte(0x6e);
        addbyte(0xc0);
        addbyte(0x83); /*SUB EBX, 1*/
        addbyte(0xeb);
        addbyte(0x01);
        addbyte(0xf3); /*CVTSS2SD XMM0, XMM0*/
        addbyte(0x0f);
        addbyte(0x5a);
        addbyte(0xc0);
        addbyte(0x83); /*AND EBX, 7*/
        addbyte(0xe3);
        addbyte(7);
        addbyte(0x85); /*TEST EAX, EAX*/
        addbyte(0xc0);
        addbyte(0x89); /*MOV TOP, EBX*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x66); /*MOVQ [ST+EBX*8], XMM0*/
        addbyte(0x0f);
        addbyte(0xd6);
        addbyte(0x04);
        addbyte(0xdd);
        addlong((uintptr_t)ST);
        addbyte(0x0f); /*SETE [tag+EBX]*/
        addbyte(0x94);
        addbyte(0x83);
        addlong((uintptr_t)tag);
}
static void FP_LOAD_D()
{
        addbyte(0x8b); /*MOV EBX, TOP*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x83); /*SUB EBX, 1*/
        addbyte(0xeb);
        addbyte(0x01);
        addbyte(0x83); /*AND EBX, 7*/
        addbyte(0xe3);
        addbyte(7);
        addbyte(0x48); /*TEST RAX, RAX*/
        addbyte(0x85);
        addbyte(0xc0);
        addbyte(0x89); /*MOV TOP, EBX*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x48); /*MOVQ [ST+EBX*8], RAX*/
        addbyte(0x89);
        addbyte(0x04);
        addbyte(0xdd);
        addlong((uintptr_t)ST);
        addbyte(0x0f); /*SETE [tag+EBX]*/
        addbyte(0x94);
        addbyte(0x83);
        addlong((uintptr_t)tag);
}

static void FP_LOAD_IW()
{
        addbyte(0x8b); /*MOV EBX, TOP*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x0f); /*MOVSX EAX, AX*/
        addbyte(0xbf);
        addbyte(0xc0);
        addbyte(0x83); /*SUB EBX, 1*/
        addbyte(0xeb);
        addbyte(0x01);
        addbyte(0xf2); /*CVTSI2SD XMM0, EAX*/
        addbyte(0x0f);
        addbyte(0x2a);
        addbyte(0xc0);
        addbyte(0x83); /*AND EBX, 7*/
        addbyte(0xe3);
        addbyte(7);
        addbyte(0x85); /*TEST EAX, EAX*/
        addbyte(0xc0);
        addbyte(0x89); /*MOV TOP, EBX*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x66); /*MOVQ [ST+EBX*8], XMM0*/
        addbyte(0x0f);
        addbyte(0xd6);
        addbyte(0x04);
        addbyte(0xdd);
        addlong((uintptr_t)ST);
        addbyte(0x0f); /*SETE [tag+EBX]*/
        addbyte(0x94);
        addbyte(0x83);
        addlong((uintptr_t)tag);
}
static void FP_LOAD_IL()
{
        addbyte(0x8b); /*MOV EBX, TOP*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x83); /*SUB EBX, 1*/
        addbyte(0xeb);
        addbyte(0x01);
        addbyte(0xf2); /*CVTSI2SD XMM0, EAX*/
        addbyte(0x0f);
        addbyte(0x2a);
        addbyte(0xc0);
        addbyte(0x83); /*AND EBX, 7*/
        addbyte(0xe3);
        addbyte(7);
        addbyte(0x85); /*TEST EAX, EAX*/
        addbyte(0xc0);
        addbyte(0x89); /*MOV TOP, EBX*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x66); /*MOVQ [ST+EBX*8], XMM0*/
        addbyte(0x0f);
        addbyte(0xd6);
        addbyte(0x04);
        addbyte(0xdd);
        addlong((uintptr_t)ST);
        addbyte(0x0f); /*SETE [tag+EBX]*/
        addbyte(0x94);
        addbyte(0x83);
        addlong((uintptr_t)tag);
}
static void FP_LOAD_IQ()
{
        addbyte(0x8b); /*MOV EBX, TOP*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x83); /*SUB EBX, 1*/
        addbyte(0xeb);
        addbyte(0x01);
        addbyte(0xf2); /*CVTSI2SDQ XMM0, RAX*/
        addbyte(0x48);
        addbyte(0x0f);
        addbyte(0x2a);
        addbyte(0xc0);
        addbyte(0x83); /*AND EBX, 7*/
        addbyte(0xe3);
        addbyte(7);
        addbyte(0x48); /*TEST RAX, RAX*/
        addbyte(0x85);
        addbyte(0xc0);
        addbyte(0x48); /*MOV [ST_i64+EBX*8], RAX*/
        addbyte(0x89);
        addbyte(0x04);
        addbyte(0xdd);
        addlong((uintptr_t)ST_i64);
        addbyte(0x89); /*MOV TOP, EBX*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x0f); /*SETE AL*/
        addbyte(0x94);
        addbyte(0xc0);
        addbyte(0x66); /*MOVQ [ST+EBX*8], XMM0*/
        addbyte(0x0f);
        addbyte(0xd6);
        addbyte(0x04);
        addbyte(0xdd);
        addlong((uintptr_t)ST);
        addbyte(0x0c); /*OR AL, TAG_UINT64*/
        addbyte(TAG_UINT64);
        addbyte(0x88); /*MOV [tag+EBX], AL*/
        addbyte(0x83);
        addlong((uintptr_t)tag);
}

static int FP_LOAD_REG(int reg)
{
        addbyte(0x8b); /*MOV EBX, TOP*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        if (reg)
        {
                addbyte(0x83); /*ADD EBX, reg*/
                addbyte(0xc0 | REG_EBX);
                addbyte(reg);
                addbyte(0x83); /*AND EBX, 7*/
                addbyte(0xe0 | REG_EBX);
                addbyte(0x07);
        }
        addbyte(0xf3); /*MOVQ XMM0, ST[EBX*8]*/
        addbyte(0x0f);
        addbyte(0x7e);
        addbyte(0x04);
        addbyte(0xdd);
        addlong((uintptr_t)ST);
        addbyte(0xf2); /*CVTSD2SS XMM0, XMM0*/
        addbyte(0x0f);
        addbyte(0x5a);
        addbyte(0xc0);
        addbyte(0x66); /*MOVD EBX, XMM0*/
        addbyte(0x0f);
        addbyte(0x7e);
        addbyte(0xc0 | REG_EBX);        
        
        return REG_EBX;
}
static void FP_LOAD_REG_D(int reg, int *host_reg1, int *host_reg2)
{
        addbyte(0x8b); /*MOV EBX, TOP*/
        addbyte(0x1c);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        if (reg)
        {
                addbyte(0x83); /*ADD EBX, reg*/
                addbyte(0xc0 | REG_EBX);
                addbyte(reg);
                addbyte(0x83); /*AND EBX, 7*/
                addbyte(0xe0 | REG_EBX);
                addbyte(0x07);
        }
        addbyte(0x48); /*MOV RBX, ST[EBX*8]*/
        addbyte(0x8b);
        addbyte(0x1c);
        addbyte(0xdd);
        addlong((uintptr_t)ST);
        
        *host_reg1 = REG_EBX;
}
static int64_t x87_fround(double b)
{
        switch ((npxc>>10)&3)
        {
                case 0: /*Nearest*/
                return (int64_t)(b+0.5);
                case 1: /*Down*/
                return (int64_t)floor(b);
                case 2: /*Up*/
                return (int64_t)ceil(b);
                case 3: /*Chop*/
                return (int64_t)b;
        }
}
static int FP_LOAD_REG_INT_W(int reg)
{
        addbyte(0x89); /*MOV EBX, EAX*/
        addbyte(0xc3);

        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        if (reg)
        {
                addbyte(0x83); /*ADD EAX, reg*/
                addbyte(0xc0);
                addbyte(reg);
                addbyte(0x83); /*AND EAX, 7*/
                addbyte(0xe0);
                addbyte(7);
        }
        addbyte(0xf3); /*MOVQ XMM0, ST[EAX*8]*/
        addbyte(0x0f);
        addbyte(0x7e);
        addbyte(0x04);
        addbyte(0xc5);
        addlong((uintptr_t)ST);

        CALL_FUNC(x87_fround);
        
        addbyte(0x93); /*XCHG EBX, EAX*/
        
        return REG_EBX;
}
static int FP_LOAD_REG_INT(int reg)
{
        addbyte(0x89); /*MOV EBX, EAX*/
        addbyte(0xc3);

        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        if (reg)
        {
                addbyte(0x83); /*ADD EAX, reg*/
                addbyte(0xc0);
                addbyte(reg);
                addbyte(0x83); /*AND EAX, 7*/
                addbyte(0xe0);
                addbyte(7);
        }
        addbyte(0xf3); /*MOVQ XMM0, ST[EAX*8]*/
        addbyte(0x0f);
        addbyte(0x7e);
        addbyte(0x04);
        addbyte(0xc5);
        addlong((uintptr_t)ST);

        CALL_FUNC(x87_fround);
        
        addbyte(0x93); /*XCHG EBX, EAX*/
        
        return REG_EBX;
}
static void FP_LOAD_REG_INT_Q(int reg, int *host_reg1, int *host_reg2)
{
        addbyte(0x89); /*MOV EBX, EAX*/
        addbyte(0xc3);

        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        if (reg)
        {
                addbyte(0x83); /*ADD EAX, reg*/
                addbyte(0xc0);
                addbyte(reg);
                addbyte(0x83); /*AND EAX, 7*/
                addbyte(0xe0);
                addbyte(7);
        }

        if (codegen_fpu_loaded_iq[TOP] && (tag[TOP] & TAG_UINT64))
        {
                /*If we know the register was loaded with FILDq in this block and
                  has not been modified, then we can skip most of the conversion
                  and just load the 64-bit integer representation directly */
                addbyte(0x48); /*MOV RAX, [ST_i64+EAX*8]*/
                addbyte(0x8b);
                addbyte(0x04);
                addbyte(0xc5);
                addlong((uint32_t)ST_i64);

                addbyte(0x48); /*XCHG RBX, RAX*/
                addbyte(0x93);

                *host_reg1 = REG_EBX;
                        
                return;
        }
        
        addbyte(0xf6); /*TEST TAG[EBX], TAG_UINT64*/
        addbyte(0x80);
        addlong((uintptr_t)tag);
        addbyte(TAG_UINT64);
        
        addbyte(0x74); /*JZ +*/
        addbyte(8+2);

        addbyte(0x48); /*MOV RAX, [ST_i64+EAX*8]*/
        addbyte(0x8b);
        addbyte(0x04);
        addbyte(0xc5);
        addlong((uint32_t)ST_i64);
        
        addbyte(0xeb); /*JMP done*/
        addbyte(9+12);

        addbyte(0xf3); /*MOVQ XMM0, ST[EAX*8]*/
        addbyte(0x0f);
        addbyte(0x7e);
        addbyte(0x04);
        addbyte(0xc5);
        addlong((uintptr_t)ST);

        CALL_FUNC(x87_fround);
        
        addbyte(0x48); /*XCHG RBX, RAX*/
        addbyte(0x93);
        
        *host_reg1 = REG_EBX;
}

#define FPU_ADD  0
#define FPU_DIV  4
#define FPU_DIVR 5
#define FPU_MUL  1
#define FPU_SUB  2
#define FPU_SUBR 3

static void FP_OP_REG(int op, int dst, int src)
{
        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x48); /*MOV RSI, ST*/
        addbyte(0xbe);
        addquad((uint64_t)ST);
        addbyte(0x89); /*MOV EBX, EAX*/
        addbyte(0xc3);
        if (dst)
        {
                addbyte(0x83); /*ADD EAX, reg*/
                addbyte(0xc0);
                addbyte(dst);
                addbyte(0x83); /*AND EAX, 7*/
                addbyte(0xe0);
                addbyte(0x07);
        }
        if (src)
        {
                addbyte(0x83); /*ADD EBX, reg*/
                addbyte(0xc0 | REG_EBX);
                addbyte(src);
                addbyte(0x83); /*AND EBX, 7*/
                addbyte(0xe0 | REG_EBX);
                addbyte(0x07);
        }
        if (op == FPU_DIVR || op == FPU_SUBR)
        {
                addbyte(0xf3); /*MOVQ XMM0, [RSI+RBX*8]*/
                addbyte(0x0f);
                addbyte(0x7e);
                addbyte(0x04);
                addbyte(0xde);
        }
        else
        {
                addbyte(0xf3); /*MOVQ XMM0, [RSI+RAX*8]*/
                addbyte(0x0f);
                addbyte(0x7e);
                addbyte(0x04);
                addbyte(0xc6);
        }
        switch (op)
        {
                case FPU_ADD:
                addbyte(0xf2); /*ADDSD XMM0, [RSI+RBX*8]*/
                addbyte(0x0f);
                addbyte(0x58);
                addbyte(0x04);
                addbyte(0xde);
                break;
                case FPU_DIV:
                addbyte(0xf2); /*DIVSD XMM0, [RSI+RBX*8]*/
                addbyte(0x0f);
                addbyte(0x5e);
                addbyte(0x04);
                addbyte(0xde);
                break;
                case FPU_DIVR:
                addbyte(0xf2); /*DIVSD XMM0, [RSI+RAX*8]*/
                addbyte(0x0f);
                addbyte(0x5e);
                addbyte(0x04);
                addbyte(0xc6);
                break;
                case FPU_MUL:
                addbyte(0xf2); /*MULSD XMM0, [RSI+RBX*8]*/
                addbyte(0x0f);
                addbyte(0x59);
                addbyte(0x04);
                addbyte(0xde);
                break;
                case FPU_SUB:
                addbyte(0xf2); /*SUBSD XMM0, [RSI+RBX*8]*/
                addbyte(0x0f);
                addbyte(0x5c);
                addbyte(0x04);
                addbyte(0xde);
                break;
                case FPU_SUBR:
                addbyte(0xf2); /*SUBSD XMM0, [RSI+RAX*8]*/
                addbyte(0x0f);
                addbyte(0x5c);
                addbyte(0x04);
                addbyte(0xc6);
                break;
        }
        addbyte(0x66); /*MOVQ [RSI+RAX*8], XMM0*/
        addbyte(0x0f);
        addbyte(0xd6);
        addbyte(0x04);
        addbyte(0xc6);
}

static void FP_OP_MEM(int op)
{
        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x48); /*MOV RSI, ST*/
        addbyte(0xbe);
        addquad((uint64_t)ST);
        addbyte(0xf3); /*MOVQ XMM0, [RSI+RAX*8]*/
        addbyte(0x0f);
        addbyte(0x7e);
        addbyte(0x04);
        addbyte(0xc6);
        addbyte(0x80); /*AND tag[EAX], ~TAG_UINT64*/
        addbyte(0xa0 | REG_EAX);
        addlong((uintptr_t)tag);
        addbyte(~TAG_UINT64);

        switch (op)
        {
                case FPU_ADD:
                addbyte(0xf2); /*ADDSD XMM0, XMM1*/
                addbyte(0x0f);
                addbyte(0x58);
                addbyte(0xc1);
                break;
                case FPU_DIV:
                addbyte(0xf2); /*DIVSD XMM0, XMM1*/
                addbyte(0x0f);
                addbyte(0x5e);
                addbyte(0xc1);
                break;
                case FPU_DIVR:
                addbyte(0xf2); /*DIVSD XMM1, XMM0*/
                addbyte(0x0f);
                addbyte(0x5e);
                addbyte(0xc8);
                break;
                case FPU_MUL:
                addbyte(0xf2); /*MULSD XMM0, XMM1*/
                addbyte(0x0f);
                addbyte(0x59);
                addbyte(0xc1);
                break;
                case FPU_SUB:
                addbyte(0xf2); /*SUBSD XMM0, XMM1*/
                addbyte(0x0f);
                addbyte(0x5c);
                addbyte(0xc1);
                break;
                case FPU_SUBR:
                addbyte(0xf2); /*SUBSD XMM1, XMM0*/
                addbyte(0x0f);
                addbyte(0x5c);
                addbyte(0xc8);
                break;
        }
        if (op == FPU_DIVR || op == FPU_SUBR)
        {
                addbyte(0x66); /*MOVQ [RSI+RAX*8], XMM1*/
                addbyte(0x0f);
                addbyte(0xd6);
                addbyte(0x0c);
                addbyte(0xc6);
        }
        else
        {
                addbyte(0x66); /*MOVQ [RSI+RAX*8], XMM0*/
                addbyte(0x0f);
                addbyte(0xd6);
                addbyte(0x04);
                addbyte(0xc6);
        }
}

static void FP_OP_S(int op)
{
        addbyte(0x66); /*MOVD XMM1, EAX*/
        addbyte(0x0f);
        addbyte(0x6e);
        addbyte(0xc8);
        addbyte(0xf3); /*CVTSS2SD XMM1, XMM1*/
        addbyte(0x0f);
        addbyte(0x5a);
        addbyte(0xc9);
        FP_OP_MEM(op);
}
static void FP_OP_D(int op)
{
        addbyte(0x66); /*MOVQ XMM1, RAX*/
        addbyte(0x48);
        addbyte(0x0f);
        addbyte(0x6e);
        addbyte(0xc8);
        FP_OP_MEM(op);
}
static void FP_OP_IW(int op)
{
        addbyte(0x0f); /*MOVSX EAX, AX*/
        addbyte(0xbf);
        addbyte(0xc0);
        addbyte(0xf2); /*CVTSI2SD XMM1, EAX*/
        addbyte(0x0f);
        addbyte(0x2a);
        addbyte(0xc8);
        FP_OP_MEM(op);
}
static void FP_OP_IL(int op)
{
        addbyte(0xf2); /*CVTSI2SD XMM1, EAX*/
        addbyte(0x0f);
        addbyte(0x2a);
        addbyte(0xc8);
        FP_OP_MEM(op);
}

#define C0 (1<<8)
#define C1 (1<<9)
#define C2 (1<<10)
#define C3 (1<<14)

static void FP_COMPARE_REG(int dst, int src)
{
        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x48); /*MOV RSI, ST*/
        addbyte(0xbe);
        addquad((uint64_t)ST);
        addbyte(0x89); /*MOV EBX, EAX*/
        addbyte(0xc3);
        if (src || dst)
        {
                addbyte(0x83); /*ADD EAX, 1*/
                addbyte(0xc0);
                addbyte(src ? src : dst);
                addbyte(0x83); /*AND EAX, 7*/
                addbyte(0xe0);
                addbyte(7);
        }

        addbyte(0x8a); /*MOV CL, [npxs+1]*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong(((uintptr_t)&npxs) + 1);
//        addbyte(0xdb); /*FCLEX*/
//        addbyte(0xe2);
        addbyte(0x80); /*AND CL, ~(C0|C2|C3)*/
        addbyte(0xe1);
        addbyte((~(C0|C2|C3)) >> 8);

        if (src)
        {
                addbyte(0xf3); /*MOVQ XMM0, [RSI+RBX*8]*/
                addbyte(0x0f);
                addbyte(0x7e);
                addbyte(0x04);
                addbyte(0xde);
                addbyte(0x66); /*COMISD XMM0, [RSI+RAX*8]*/
                addbyte(0x0f);
                addbyte(0x2f);
                addbyte(0x04);
                addbyte(0xc6);
        }
        else
        {
                addbyte(0xf3); /*MOVQ XMM0, [RSI+RAX*8]*/
                addbyte(0x0f);
                addbyte(0x7e);
                addbyte(0x04);
                addbyte(0xc6);
                addbyte(0x66); /*COMISD XMM0, [RSI+RBX*8]*/
                addbyte(0x0f);
                addbyte(0x2f);
                addbyte(0x04);
                addbyte(0xde);
        }

        addbyte(0x9f); /*LAHF*/
        addbyte(0x80); /*AND AH, (C0|C2|C3)*/
        addbyte(0xe4);
        addbyte((C0|C2|C3) >> 8);
        addbyte(0x08); /*OR CL, AH*/
        addbyte(0xe1);
        addbyte(0x88); /*MOV [npxs+1], CL*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong(((uintptr_t)&npxs) + 1);
}

static void FP_COMPARE_MEM()
{
        addbyte(0x8b); /*MOV EAX, [TOP]*/
        addbyte(0x04);
        addbyte(0x25);
        addlong((uintptr_t)&TOP);
        addbyte(0x48); /*MOV RSI, ST*/
        addbyte(0xbe);
        addquad((uint64_t)ST);

        addbyte(0x8a); /*MOV CL, [npxs+1]*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong(((uintptr_t)&npxs) + 1);
//        addbyte(0xdb); /*FCLEX*/
//        addbyte(0xe2);
        addbyte(0xf3); /*MOVQ XMM0, [RSI+RAX*8]*/
        addbyte(0x0f);
        addbyte(0x7e);
        addbyte(0x04);
        addbyte(0xc6);
        addbyte(0x80); /*AND CL, ~(C0|C2|C3)*/
        addbyte(0xe1);
        addbyte((~(C0|C2|C3)) >> 8);
        addbyte(0x66); /*COMISD XMM0, XMM1*/
        addbyte(0x0f);
        addbyte(0x2f);
        addbyte(0xc1);
        addbyte(0x9f); /*LAHF*/
        addbyte(0x80); /*AND AH, (C0|C2|C3)*/
        addbyte(0xe4);
        addbyte((C0|C2|C3) >> 8);
        addbyte(0x08); /*OR CL, AH*/
        addbyte(0xe1);
        addbyte(0x88); /*MOV [npxs+1], CL*/
        addbyte(0x0c);
        addbyte(0x25);
        addlong(((uintptr_t)&npxs) + 1);
}
static void FP_COMPARE_S()
{
        addbyte(0x66); /*MOVD XMM1, EAX*/
        addbyte(0x0f);
        addbyte(0x6e);
        addbyte(0xc8);
        addbyte(0xf3); /*CVTSS2SD XMM1, XMM1*/
        addbyte(0x0f);
        addbyte(0x5a);
        addbyte(0xc9);
        FP_COMPARE_MEM();
}
static void FP_COMPARE_D()
{
        addbyte(0x66); /*MOVQ XMM1, RAX*/
        addbyte(0x48);
        addbyte(0x0f);
        addbyte(0x6e);
        addbyte(0xc8);
        FP_COMPARE_MEM();
}
static void FP_COMPARE_IW()
{
        addbyte(0x0f); /*MOVSX EAX, AX*/
        addbyte(0xbf);
        addbyte(0xc0);
        addbyte(0xf2); /*CVTSI2SD XMM1, EAX*/
        addbyte(0x0f);
        addbyte(0x2a);
        addbyte(0xc8);
        FP_COMPARE_MEM();
}
static void FP_COMPARE_IL()
{
        addbyte(0xf2); /*CVTSI2SD XMM1, EAX*/
        addbyte(0x0f);
        addbyte(0x2a);
        addbyte(0xc8);
        FP_COMPARE_MEM();
}
