#include <rvh_test.h>
#include <page_tables.h>
#include <math.h>
#include <smepmp.h>
#include <pmp.h>

extern unsigned long __text_begin, __text_end;
extern unsigned long __data_begin, __stack_top;
static uint32_t pmp_test_index = 0;

struct {
    unsigned int pmpcfg;
    uintptr_t base_addr;
    uintptr_t len;
} smepmp_test_table[SMEPMP_TEST_MAX] = {
    [MI_SUI]                =   {0,                                 MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MI_SUI   ,  PAGE_SIZE },
    [MI_SUX]                =   {PMP_X,                             MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MI_SUX   ,  PAGE_SIZE },
    [MRW_SUR]               =   {PMP_W,                             MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MRW_SUR  ,  PAGE_SIZE },
    [MRW_SURW]              =   {PMP_W | PMP_X,                     MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MRW_SURW ,  PAGE_SIZE },
    [MI_SUR]                =   {PMP_R,                             MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MI_SUR   ,  PAGE_SIZE },
    [MI_SURX]               =   {PMP_R | PMP_X,                     MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MI_SURX  ,  PAGE_SIZE },
    [MI_SURW]               =   {PMP_R | PMP_W,                     MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MI_SURW  ,  PAGE_SIZE },
    [MI_SURWX]              =   {PMP_R | PMP_W | PMP_X,             MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MI_SURWX ,  PAGE_SIZE },
    [MLI_SULI]              =   {PMP_L,                             MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MLI_SULI ,  PAGE_SIZE },
    [MLX_SULI]              =   {PMP_L | PMP_X,                     MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MLX_SULI ,  PAGE_SIZE },
    [MLX_SULX]              =   {PMP_L | PMP_W,                     MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MLX_SULX ,  PAGE_SIZE },
    [MLRX_SULX]             =   {PMP_L | PMP_W | PMP_X,             MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MLRX_SULX,  PAGE_SIZE },
    [MLR_SULI]              =   {PMP_L | PMP_R,                     MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MLR_SULI ,  PAGE_SIZE },
    [MLRX_SULI]             =   {PMP_L | PMP_R | PMP_X,             MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MLRX_SULI,  PAGE_SIZE },
    [MLRW_SULI]             =   {PMP_L | PMP_R | PMP_W,             MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MLRW_SULI,  PAGE_SIZE },
    [MLR_SULR]              =   {PMP_L | PMP_R | PMP_W | PMP_X,     MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * MLR_SULR ,  PAGE_SIZE },
    [NO_RULE]               =   {0,                                 MEM_BASE + SMEPMP_MEM_TEST_BASE + PAGE_SIZE * NO_RULE  ,  PAGE_SIZE },
};

bool init_smepmp_test(uint32_t test_num){
    pmp_t pmp;

    if(test_num > SMEPMP_TEST_MAX)
        return false;

    pmp.cfg.perm = smepmp_test_table[test_num].pmpcfg & 0x7;
    pmp.cfg.locked = !!((smepmp_test_table[test_num].pmpcfg >> PMP_L_READ_SHIFT) & 0x1);
    pmp.cfg.mode = TOR;
    pmp.addr = smepmp_test_table[test_num].base_addr;
    pmp.len = smepmp_test_table[test_num].len;

    return pmp_set(pmp_test_index, pmp);
}
bool init_smepmp(){
    pmp_t pmp;

    // Set RLB bit
    CSRW(CSR_MSECCFG, MSECCFG_RLB);

    //Initialize common shared regions
    pmp.cfg.perm = (PMP_L | PMP_R | PMP_W | PMP_X) & 0x7;
    pmp.cfg.locked = !!(((PMP_L | PMP_R | PMP_W | PMP_X) >> PMP_L_READ_SHIFT) & 0x1);
    pmp.cfg.mode = TOR;

    pmp.addr = 0;
    pmp.len = (uintptr_t) &__text_begin;
    INFO("ADDR %lx LEN %lx",pmp.addr, pmp.len);
    pmp_set(pmp_test_index, pmp);
    pmp_test_index ++;

    pmp.addr =  (uintptr_t) &__text_begin;
    pmp.len = (uintptr_t) &__text_end - (uintptr_t) &__text_begin;
    INFO("ADDR %lx LEN %lx",pmp.addr, pmp.len);
    pmp_set(pmp_test_index, pmp);
    pmp_test_index ++;

    pmp.addr = (uintptr_t) &__data_begin;
    pmp.len = (uintptr_t) &__stack_top - (uintptr_t) &__data_begin;
    INFO("ADDR %lx LEN %lx",pmp.addr, pmp.len);
    pmp_set(pmp_test_index, pmp);
    pmp_test_index ++;

}
bool test_smepmps_mml_bit(){
    pmp_t pmp;
    uintptr_t addr;

    TEST_START();

    // M-mode Tests
    init_smepmp();
    // Set/clear MML bit
    CSRW(CSR_MSECCFG, CSRR(CSR_MSECCFG) | MSECCFG_MML );
    CSRW(CSR_MSECCFG, CSRR(CSR_MSECCFG) & ~MSECCFG_MML);
    // Check if MML bit is a sticky bit
    TEST_SETUP_EXCEPT();
    TEST_ASSERT("mseccfg.mml is a sticky bit.",
        excpt.triggered == false &&
        (CSRR(CSR_MSECCFG) >> MSECCFG_MML_OFF) & 0x1
    );

    // Test S-mode only inaccessible region
    init_smepmp_test(MI_SUI);
    addr = smepmp_test_table[MI_SUI].base_addr;
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from M-mode in a S-mode only inaccessible region causes a load access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from M-mode in a S-mode only inaccessible region causes a store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from S-mode in a S-mode only inaccessible region causes a load access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from S-mode in a S-mode only inaccessible region causes a store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    // Test Shared data region: Read/write on M mode, read-only on S/U mode
    goto_priv(PRIV_M);
    init_smepmp_test(MRW_SUR);
    addr = smepmp_test_table[MRW_SUR].base_addr;

    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from M-mode in Shared M-mode RW region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from M-mode in Shared M-mode RW region causes no exception.",
        excpt.triggered == false
    );

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from S-mode in Shared S-mode R-only region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from S-mode in Shared S-mode R-only region causes a store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    // Test Shared data region: Read/write for both M and S/U mode Access
    goto_priv(PRIV_M);
    init_smepmp_test(MRW_SURW);
    addr = smepmp_test_table[MRW_SURW].base_addr;

    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from M-mode in Shared RW region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from M-mode in Shared RW region causes no exception.",
        excpt.triggered == false
    );

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from S-mode in Shared RW region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from S-mode in Shared RW region causes no exception.",
        excpt.triggered == false
    );

    // Test S-mode only read-only region
    goto_priv(PRIV_M);
    init_smepmp_test(MI_SUR);
    addr = smepmp_test_table[MI_SUR].base_addr;

    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from M-mode in a S-mode only read-only region causes load access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from M-mode in a S-mode only read-only region causes store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from S-mode in a S-mode only read-only region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from S-mode in a S-mode only read-only region causes store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    // TODO: Test S-mode only read/execute region
    // Test S-mode only read/write region
    goto_priv(PRIV_M);
    init_smepmp_test(MI_SURW);
    addr = smepmp_test_table[MI_SURW].base_addr;

    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from M-mode in a S-mode only read/write region causes load access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from M-mode in a S-mode only read/write region causes store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from S-mode in a S-mode only read/write region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from S-mode in a S-mode only read/write region causes a store access exception.",
        excpt.triggered == false
    );

    // Test S-mode only read/write/execute region
    goto_priv(PRIV_M);
    init_smepmp_test(MI_SURWX);
    addr = smepmp_test_table[MI_SURWX].base_addr;

    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from M-mode in a S-mode only read/write/execute region causes load access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from M-mode in a S-mode only read/write/execute region causes store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from S-mode in a S-mode only read/write region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from S-mode in a S-mode only read/write region causes a store access exception.",
        excpt.triggered == false
    );

    //TODO: Tests for execute privileges

    // Test M-mode only locked inaccessible region
    goto_priv(PRIV_M);
    init_smepmp_test(MLI_SULI);
    addr = smepmp_test_table[MLI_SULI].base_addr;

    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from M-mode in a M-mode locked inaccessible region causes load access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from M-mode in a M-mode locked inaccessible region causes store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from S-mode in a M-mode locked inaccessible region causes load access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from S-mode in a M-mode locked inaccessible region causes store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    //TODO: tests for locked m-mode only execute regions.
    //TODO: tests for shared execute only.
    //TODO: tests for shared S/U execute only and read/execute M-mode.

    // Tests for m-mode only read-only regions
    goto_priv(PRIV_M);
    init_smepmp_test(MLR_SULI);
    addr = smepmp_test_table[MLR_SULI].base_addr;

    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from M-mode in a M-mode locked read-only region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from M-mode in a M-mode locked read-only region causes store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from S-mode in a M-mode locked read-only region causes load access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from S-mode in a M-mode locked read-only region causes store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    // TODO: Tests for m-mode only read/execute regions
    // Tests for m-mode only read/write regions
    goto_priv(PRIV_M);
    init_smepmp_test(MLRW_SULI);
    addr = smepmp_test_table[MLRW_SULI].base_addr;

    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from M-mode in a M-mode locked read/write region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from M-mode in a M-mode locked read/write region causes no exception.",
        excpt.triggered == false
    );

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from S-mode in a M-mode locked read/write region causes load access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from S-mode in a M-mode locked read/write region causes store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    // Tests for shared read only regions
    goto_priv(PRIV_M);
    init_smepmp_test(MLR_SULR);
    addr = smepmp_test_table[MLR_SULR].base_addr;

    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from M-mode in a shared locked read-only region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from M-mode in a shared locked read-only region causes a store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    read64(addr);
    TEST_ASSERT("Load from S-mode in a shared locked read-only region causes no exception.",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    write64(addr,0xffff);
    TEST_ASSERT("Store from S-mode in a shared locked read-only region causes store access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_SAF
    );

    //TODO: Test if we can't add a new M-mode-only or locked Shared-Region
    //      with executable privileges when mseccfg.MML=1 and mseccfg.RLB=0.
    TEST_END();
}

bool test_smepmps_mmwp_bit(){
    uintptr_t addr;

    TEST_START();

    init_smepmp();
    goto_priv(PRIV_M);

    // Check if no-match region is ignored in M-mode
    TEST_SETUP_EXCEPT();
    init_smepmp_test(NO_RULE);
    addr = smepmp_test_table[NO_RULE].base_addr;
    read64(addr);
    TEST_ASSERT("Load from M-mode in non-match pmp rule is ignored when mseccfg.mmwp = 0.",
        excpt.triggered == false
    );

    // Check if MMWP bit is a sticky bit
    // Set MMWP bit
    CSRW(CSR_MSECCFG, CSRR(CSR_MSECCFG) | MSECCFG_MMWP);
    // Clear MMWP bit
    CSRW(CSR_MSECCFG, CSRR(CSR_MSECCFG) & ~MSECCFG_MMWP);
    TEST_SETUP_EXCEPT();
    TEST_ASSERT("mseccfg.mmwp is a sticky bit.",
        excpt.triggered == false &&
        ((CSRR(CSR_MSECCFG) >> MSECCFG_MMWP_OFF) & 0x1)
    );

    // Check if no-match region is denied in M-mode
    TEST_SETUP_EXCEPT();
    addr = smepmp_test_table[NO_RULE].base_addr;
    read64(addr);
    TEST_ASSERT("Load from M-mode in non-match region causes a load access exception when mseccfg.mmwp = 1.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );

    TEST_END();

}

bool test_smepmps_rlb_bit(){
    int ret;
    uintptr_t addr;
    pmp_t pmp;
    uint32_t pmp_n;

    TEST_START();

    init_smepmp();
    pmp_n = pmp_test_index;

    // Check if we can unlock on a pmp rule when mseccfg.rlb = 1
    // Set RLB bit
    init_smepmp_test(MI_SUI);
    CSRW(CSR_MSECCFG, (CSRR(CSR_MSECCFG) & ~MSECCFG_RLB & 0x7) | MSECCFG_RLB);
    ret = pmp_get(pmp_n, &pmp);
    bool prev_locked = pmp.cfg.locked;
    if(!ret)
        ERROR("Could not get pmp %d rule",pmp_n);
    pmp.cfg.locked = false;
    ret = pmp_set(pmp_n, pmp);
    if(!ret)
        ERROR("Could not set pmp %d rule",pmp_n);
    ret = pmp_get(pmp_n, &pmp);
    if(!ret)
        ERROR("Could not get pmp %d rule",pmp_n);
    TEST_ASSERT("Locked bit is cleared when mseccfg.rlb=1.",
        excpt.triggered == false &&
        pmp.cfg.locked == false &&
        prev_locked == true
    );
    pmp.cfg.locked = true;
    ret = pmp_set(pmp_n, pmp);
    if(!ret)
        ERROR("Could not set pmp %d rule",pmp_n);

    // Check if we can disable a PMP rule
    TEST_SETUP_EXCEPT();
    addr = smepmp_test_table[MI_SUI].base_addr;
    read64(addr);
    TEST_ASSERT("Load from M-mode in non-readable region causes a load access exception.",
        excpt.triggered == true &&
        excpt.cause == CAUSE_LAF
    );
    TEST_SETUP_EXCEPT();
    ret = pmp_get(pmp_n, &pmp);
    pmp_disable(pmp_n);
    if(!ret)
        ERROR("Could not set pmp %d rule",pmp_n);
    addr = smepmp_test_table[MI_SUI].base_addr;
    read64(addr);
    TEST_ASSERT("Load from M-mode in disabled non-readable region causes no exception.",
        excpt.triggered == false
    );

    // Restore pmp rule
    pmp.cfg.locked = true;
    ret = pmp_set(pmp_n, pmp);
    if(!ret)
        ERROR("Could not set pmp %d rule",pmp_n);

    // Check if RLB bit remains zero after being clear
    // Clear RLB bit
    CSRW(CSR_MSECCFG, (CSRR(CSR_MSECCFG) & ~MSECCFG_RLB & 0x3));
    TEST_SETUP_EXCEPT();
    TEST_ASSERT("mseccfg.rlb sticks to 0 when cleared.",
        excpt.triggered == false &&
        !((CSRR(CSR_MSECCFG) >> MSECCFG_RLB_OFF) & 0x1)
    );

    // Check if we can't unlock on a pmp rule when mseccfg.rlb = 0
    pmp.cfg.locked = false;
    ret = pmp_set(pmp_n, pmp);
    if(!ret)
        ERROR("Could not set pmp %d rule",pmp_n);
    ret = pmp_get(pmp_n, &pmp);
    if(!ret)
        ERROR("Could not get pmp %d rule",pmp_n);
    TEST_ASSERT("Locked bit is not cleared when mseccfg.rlb=0.",
        excpt.triggered == false &&
        pmp.cfg.locked == true
    );

    TEST_END();
}