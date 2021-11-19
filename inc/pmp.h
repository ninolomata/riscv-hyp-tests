#ifndef PMP_H
#define PMP_H

#include <rvh_test.h>
#define PMP_MAX 16

#define CSR_PMPCFG0 0x3A0
#define CSR_PMPCFG2 0x3A2

#define CSR_PMPADDR0 0x3B0
#define CSR_PMPADDR1 0x3B1
#define CSR_PMPADDR2 0x3B2
#define CSR_PMPADDR3 0x3B3
#define CSR_PMPADDR4 0x3B4
#define CSR_PMPADDR5 0x3B5
#define CSR_PMPADDR6 0x3B6
#define CSR_PMPADDR7 0x3B7
#define CSR_PMPADDR8 0x3B8
#define CSR_PMPADDR9 0x3B9
#define CSR_PMPADDR10 0x3BA
#define CSR_PMPADDR11 0x3BB
#define CSR_PMPADDR12 0x3BC
#define CSR_PMPADDR13 0x3BD
#define CSR_PMPADDR14 0x3BE
#define CSR_PMPADDR15 0x3BF

#define PMP_R     (0x01)
#define PMP_W     (0x02)
#define PMP_X     (0x04)
#define PMP_A     (0x18)
#define PMP_L     (0x80)
#define PMP_R_READ_SHIFT    (0)
#define PMP_W_READ_SHIFT    (1)
#define PMP_X_READ_SHIFT    (2)
#define PMP_A_READ_SHIFT    (3)
#define PMP_L_READ_SHIFT    (7)
#define PMP_SHIFT (2)

#define PMP_TOR   (0x08)
#define PMP_NA4   (0x10)
#define PMP_NAPOT (0x18)
#define PMP_NAPOT_MASK(size) ((1UL << (size- PMP_SHIFT)) - 1)

#define PMP_A_OFF 3
#define PMP_A_LEN 2
#define PMP_A_MSK (PMP_A_LEN << PMP_A_OFF)

#define PMPCFG_MASK(offset) (0xffUL << offset)
#define PMP_PMPCFG_N_OFFSET(index) ((index & 7) << 3)
#define PMP_PMPCFG_N_CSR(index) (CSR_PMPCFG0 + ((index >> 3) << 1))
#define PMP_PMPADDR_N_CSR(index) (CSR_PMPADDR0 + index)

typedef enum {
    OFF = 0,
    TOR = 1,
    NA4 = 2,
    NAPOT = 3,
} matching_mode_t;

typedef struct {
    bool locked;
    matching_mode_t mode;
    uint8_t perm;
} pmpcfg_t;

typedef struct {
    pmpcfg_t cfg;
    unsigned long addr;
    unsigned long len;
} pmp_t;

bool pmp_disable(uint32_t pmp_n);
bool pmp_set(uint32_t pmp_n, pmp_t pmp);
bool pmp_get(uint32_t pmp_n, pmp_t* pmp);
bool is_pmp_locked(uint32_t pmp_n, bool* lock);

#endif