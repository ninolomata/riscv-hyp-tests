#include <rvh_test.h>
#include <page_tables.h>

TEST_REGISTER(two_stage_translation);
TEST_REGISTER(second_stage_only_translation);
TEST_REGISTER(m_and_hs_using_vs_access);
TEST_REGISTER(check_xip_regs);
TEST_REGISTER(interrupt_tests);
TEST_REGISTER(virtual_instruction);
TEST_REGISTER(hfence_test);
TEST_REGISTER(wfi_exception_tests);
TEST_REGISTER(tinst_tests);
TEST_REGISTER(test_smepmps_mml_bit);
TEST_REGISTER(test_smepmps_rlb_bit);
TEST_REGISTER(test_smepmps_mmwp_bit);
