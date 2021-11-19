#ifndef SMEPMP_H
#define SMEPMP_H

/* 
    M - M-mode
    S - S-mode
    U - U-mode
    I - Inaccessible Region
    L - Locked Rule
    X - Execute Region
    R - Read Region
    W - Write Region
*/
#define PMP_MAX 16
#define SMEPMP_TEST_MAX 17
#define SMEPMP_MEM_TEST_BASE 0x20000000

enum test_smepmp { 
    MI_SUI                  ,
    MI_SUX                  ,
    MRW_SUR                 ,
    MRW_SURW                ,
    MI_SUR                  ,
    MI_SURX                 ,
    MI_SURW                 ,
    MI_SURWX                ,
    MLI_SULI                ,
    MLX_SULI                ,
    MLX_SULX                ,
    MLRX_SULX               ,
    MLR_SULI                ,
    MLRX_SULI               ,
    MLRW_SULI               ,
    MLR_SULR                ,
    NO_RULE                 ,
};

#endif
