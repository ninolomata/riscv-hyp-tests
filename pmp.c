#include <rvh_test.h>
#include <pmp.h>

#define SWITCH_CASE_READ_PMP(__csr_addr, __ret) \
        case __csr_addr:\
            __ret = CSRR(__csr_addr);\
            break;

#define SWITCH_CASE_WRITE_PMP(__csr_addr, __val) \
        case __csr_addr:\
            CSRW(__csr_addr, __val);\
            break;

unsigned long pmp_csr_read(int csr_addr)
{
	unsigned long ret = 0;
    switch (csr_addr) {
        SWITCH_CASE_READ_PMP(CSR_PMPCFG0, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPCFG2, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR0, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR1, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR2, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR3, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR4, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR5, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR6, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR7, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR8, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR9, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR10, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR11, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR12, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR13, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR14, ret)
        SWITCH_CASE_READ_PMP(CSR_PMPADDR15, ret)
        default:
            break;
    };

	return ret;
}

void pmp_csr_write(int csr_addr, unsigned long val)
{
	    switch (csr_addr) {
        SWITCH_CASE_WRITE_PMP(CSR_PMPCFG0, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPCFG2, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR0, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR1, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR2, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR3, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR4, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR5, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR6, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR7, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR8, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR9, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR10, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR11, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR12, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR13, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR14, val)
        SWITCH_CASE_WRITE_PMP(CSR_PMPADDR15, val)
        default:
            break;
    };
}

static int pow2(int n)
{
    int i; /* Variable used in loop counter */
    int ret = 1;

    for (i = 0; i < n; ++i)
        ret *= 2;

    return(ret);
}

static unsigned long ctz64(unsigned long x)
{
	unsigned long ret = 0;

	if (x == 0)
		return 8 * sizeof(x);

	while (!(x & 1UL)) {
		ret++;
		x = x >> 1;
	}

	return ret;
}

static unsigned long log2up(unsigned long num)
{
	unsigned long count;

    for(count = 0; count < 64; count++)
    {
        if(num  < (1UL << count))
            break;
    }
	return count;
}

static bool pmp_encode_napot_addr(uintptr_t* addr, uintptr_t range){
    uint64_t size = log2up(range);
    uintptr_t enc_addr;

    if(size <= PMP_SHIFT || size > 64)
        return false;

    if(size == 64){
        enc_addr = -1UL;
    } else {
         enc_addr = ((*addr  >> PMP_SHIFT) & ~PMP_NAPOT_MASK(size));
         enc_addr |= (PMP_NAPOT_MASK(size) >> 1);
    }

    *addr = enc_addr;
    return true;
}

static bool pmp_decode_napot_addr(uintptr_t* addr, uintptr_t* len){
    uint64_t temp_addr, temp_len;
    unsigned long t1;
    if (*addr == -1UL) {
			temp_addr	= 0;
			temp_len = pow2(64);
		} else {
			t1	= ctz64(~*addr);
			temp_addr	= (*addr & ~((1UL << t1) - 1)) << PMP_SHIFT;
			temp_len	= pow2(t1 + PMP_SHIFT + 1);
		}
    *addr = temp_addr;
    *len = temp_len;
    return true;
}

bool pmp_disable(uint32_t pmp_n){
    pmp_t pmp;
    int ret;
    ret = pmp_get(pmp_n, &pmp);
    if(!ret)
        return false;
    pmp.cfg.mode = OFF;
    ret = pmp_set(pmp_n, pmp);
    return ret;
}

bool pmp_set(uint32_t pmp_n, pmp_t pmp){
    int pmpcfg_n, pmpcfg_offset;
    int pmpaddr_n;
    uint64_t cfg;
    uintptr_t addr;

    /* calculate pmpcfg register address and offset for PMP n*/
	pmpcfg_n   = PMP_PMPCFG_N_CSR(pmp_n);
	pmpcfg_offset = PMP_PMPCFG_N_OFFSET(pmp_n);

    /* calculate pmpaddr register address for PMP n*/
    pmpaddr_n = PMP_PMPADDR_N_CSR(pmp_n);

    /* Encode pmpcfg permissions and lock bit*/
    cfg |= pmp.cfg.perm & 0x7;
    if(pmp.cfg.locked) {
        cfg |= PMP_L;
    }

    addr = pmp.addr;
    /* Encode pmpcfg matching mode */
    switch(pmp.cfg.mode){
        case OFF:
            addr = 0;
            cfg  = 0;
            break;
        case TOR:
            cfg |= PMP_TOR;
        	cfg = (pmp_csr_read(pmpcfg_n) & ~PMPCFG_MASK(pmpcfg_offset)) | ((cfg << pmpcfg_offset) & PMPCFG_MASK(pmpcfg_offset));
            addr = (addr + pmp.len) >> PMP_SHIFT;
            break;
        case NA4:
            cfg |= PMP_NA4;
	        cfg = (pmp_csr_read(pmpcfg_n) & ~PMPCFG_MASK(pmpcfg_offset)) | ((cfg << pmpcfg_offset) & PMPCFG_MASK(pmpcfg_offset));
            addr = (addr >> PMP_SHIFT);
            break;
        case NAPOT:
            cfg |= PMP_NAPOT;
            cfg = (pmp_csr_read(pmpcfg_n) & ~PMPCFG_MASK(pmpcfg_offset)) | ((cfg << pmpcfg_offset) & PMPCFG_MASK(pmpcfg_offset));
            pmp_encode_napot_addr(&addr,pmp.len);
            break;
        default:
            return false;
            break;
    }
    /* update pmpcfg n and pmpaddr n*/
    pmp_csr_write(pmpaddr_n, addr);
    pmp_csr_write(pmpcfg_n, cfg);

    INFO("PMP_SET: target_cfg %lx actual_cfg %lx target_addr %lx actual_addr %lx", cfg, pmp_csr_read(pmpcfg_n), addr, pmp_csr_read(pmpaddr_n));
    return true;
}

bool pmp_get(uint32_t pmp_n, pmp_t* pmp){
    int pmpcfg_n, pmpcfg_offset;
    int pmpaddr_n;
    unsigned long t1;
    uint64_t cfg;
    pmp_t temp_pmp;

	/* calculate pmpcfg register address and offset for PMP n*/
	pmpcfg_n   = PMP_PMPCFG_N_CSR(pmp_n);
	pmpcfg_offset = PMP_PMPCFG_N_OFFSET(pmp_n);

    /* calculate pmpaddr register address for PMP n*/
    pmpaddr_n = PMP_PMPADDR_N_CSR(pmp_n);

	/* decode PMP config */
	cfg = ((pmp_csr_read(pmpcfg_n) & PMPCFG_MASK(pmpcfg_offset)) >> pmpcfg_offset);
    temp_pmp.cfg.perm = cfg & 0x7;
    temp_pmp.cfg.locked = !!(cfg >> PMP_L_READ_SHIFT);


	/* Encode pmpcfg matching mode */
    temp_pmp.cfg.mode = ((cfg & PMP_A) >> PMP_A_READ_SHIFT) & 0x3;
    switch(temp_pmp.cfg.mode){
        case OFF:
            temp_pmp.addr = 0;
            temp_pmp.len  = 0;
            break;
        case TOR:
            temp_pmp.addr = pmp_csr_read(pmpaddr_n);
            temp_pmp.addr = (temp_pmp.addr << PMP_SHIFT);
            if(pmp_n == 0)
                temp_pmp.len = pmp_csr_read(pmpaddr_n);
            else
                temp_pmp.len = pmp_csr_read(pmpaddr_n) - pmp_csr_read(pmpaddr_n - 1);
            break;
        case NA4:
            temp_pmp.addr = pmp_csr_read(pmpaddr_n);
            temp_pmp.addr = (temp_pmp.addr << PMP_SHIFT);
            temp_pmp.len  = pow2(PMP_SHIFT);
            break;
        case NAPOT:
            temp_pmp.addr = pmp_csr_read(pmpaddr_n);
            pmp_decode_napot_addr(&temp_pmp.addr,&temp_pmp.len);
            break;
        default:
            return false;
            break;
    }

    *pmp = temp_pmp;
	return true;
}

bool is_pmp_locked(uint32_t pmp_n, bool* lock){
    pmp_t pmp;
    int ret;

    ret = pmp_get(pmp_n, &pmp);
    if(ret)
        return false;
    *lock = pmp.cfg.locked;
}
