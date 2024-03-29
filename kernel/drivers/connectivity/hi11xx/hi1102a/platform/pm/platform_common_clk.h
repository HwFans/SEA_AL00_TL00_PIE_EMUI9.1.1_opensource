

#ifndef __PLATFORM_COMMON_CLK_H__
#define __PLATFORM_COMMON_CLK_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "plat_type.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLATFORM_COMMON_CLK_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define HI_STATIC_ASSERT(cond_, name_) typedef char assert_failed_##name_[(cond_) ? 1 : -1]

#define DCXO_DEVICE_MAX_BUF_LEN 64
#define DCXO_CALI_DATA_BUF_LEN  (sizeof(dcxo_dl_para_stru))

/* 以下是DCXO 读取的NV中的参数 */

#define MALLOC_LEN            104
#define NV_GNSS_ID            236
#define NV_GNSS_SIZE          32
#define NV_GNSS_NAME          "XOCOE"
#define NV_FAC_ID             235
#define NV_FAC_SIZE           88
#define NV_FAC_NAME           "XOA1A0"
#define PLAT_DEF_ID           (-1)
#define PLAT_DEF_NAME         "PLAT_DEF_COEFF"
#define NVRAM_READ            1
#define NV_CHK_PAIR           2
#define GNSS_COEFF_NUM        4
#define RSV_NUM               2
#define DCXO_NV_CHK_OK        1
#define T0_BIT_WIDTH_10       0
#define T0_WIDTH10_TO_WIDTH16 (1 << 6)
#define DCXO_PARA_READ_OK     0x01

#define PLAT_MAX_REPORT_CNT       10
#define PLAT_CHR_ERRNO_DCXO_NV_RD 910800034

/** 参数配置 **/
#define INI_BUF_SIZE   16
#define INI_TCXO_STR   "tcxo"
#define INI_DCXO_STR   "dcxo"
#define TCXO_DCXO_MODE "tcxo_dcxo_mode"

// 默认下发给device的参数
#define DEFAULT_T0       0x1E0000  // 30*2^16
#define DEFAULT_A0       0
#define DEFAULT_A1       0xFFFc0000  // -0.25*2^20
#define DEFAULT_A2       0
#define DEFAULT_A3       0x68db8b  // 0.0001*2^36
#define DEFAULT_MODE     0
#define DCXO_CHECK_MAGIC 0xA0A0A0A0

// 默认a0~t0的min max参数
#define DEF_A0_MIN (-3145728)  // -12*2^18
#define DEF_A0_MAX 3145728     // 12*2^18
#define DEF_A1_MIN (-419430)   // -0.4*2^20
#define DEF_A1_MAX (-104857)   // -0.1*2^20
#define DEF_A2_MIN (-30923764) // -0.00045*2^36
#define DEF_A2_MAX 30923764    // 0.00045*2^36
#define DEF_A3_MIN 5841155     // 0.000085*2^36
#define DEF_A3_MAX 7902739     // 0.000115*2^36
#define DEF_T0_MIN 1638400     // 25*2^16
#define DEF_T0_MAX 2293760     // 35*2^16

// a0~t0 min max 异常范围检测参数
#define A0_EXCEP_PROT_MIN (-13107200)   // -50*2^18
#define A0_EXCEP_PROT_MAX 13107200      // 50*2^18
#define A1_EXCEP_PROT_MIN (-1048576)    // -1*2^20
#define A1_EXCEP_PROT_MAX 0             // 0*2^20
#define A2_EXCEP_PROT_MIN (-1374389535) // -0.02*2^36
#define A2_EXCEP_PROT_MAX 1374389535    // 0.02*2^36
#define A3_EXCEP_PROT_MIN 0             // 0*2^36
#define A3_EXCEP_PROT_MAX 1374389535    // 0.02*2^36
#define T0_EXCEP_PROT_MIN 1310720       // 20*2^16
#define T0_EXCEP_PROT_MAX 2621440       // 40*2^16

/* PMU 参数配置 */
#define INI_PMU_CHIP_TYPE "pmu_version"
#define INI_6555V300_STR  "Hi6555V300"
#define INI_6421V700_STR  "Hi6421V700"

// PMU 6555V300
#define PMU_6555V300_CALI_END  0x3A0
#define PMU_6555V300_CALI_AVE0 0x3A1
#define PMU_6555V300_CALI_AVE1 0x3A2
#define PMU_6555V300_CALI_AVE2 0x3A3
#define PMU_6555V300_CALI_AVE3 0x3A4
#define PMU_6555V300_ANA_EN    0x3AA
#define PMU_6555V300_STATE     0x3AC
#define PMU_6555V300_LOW_BYTE  0x3AD
#define PMU_6555V300_HIGH_BYTE 0x3AE

// PMU 6421V700
#define PMU_6421V700_CALI_END  0x3A0
#define PMU_6421V700_CALI_AVE0 0x3A1
#define PMU_6421V700_CALI_AVE1 0x3A2
#define PMU_6421V700_CALI_AVE2 0x3A3
#define PMU_6421V700_CALI_AVE3 0x3A4
#define PMU_6421V700_ANA_EN    0x3AA
#define PMU_6421V700_STATE     0x3AC
#define PMU_6421V700_LOW_BYTE  0x3AD
#define PMU_6421V700_HIGH_BYTE 0x3AE

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  5 消息头定义
*****************************************************************************/

/*****************************************************************************
  6 消息定义
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
enum coeff_index_enum {
    COEFF_A0 = 0,
    COEFF_A1 = 1,
    COEFF_A2 = 2,
    COEFF_A3 = 3,
    COEFF_T0 = 4,
    COEFF_NUM,
};

enum pmu_type_enum {
    PMU_HI655V300 = 0,
    PMU_HI6421V700 = 1,
    PMU_TYPE_BUFF,
};

enum coeff_type_enum {
    INIT_TYPE_MIN = 0,
    INIT_TYPE_MAX = 1,
    COEFF_TYPE_BUFF,
};

enum dcxo_tcxo_enum {
    INI_TCXO_MODE = 0,
    INI_DCXO_MODE = 1,
    INIT_MODE_BUFF,
};

typedef struct {
    oal_int32 para[COEFF_NUM];  // 存放a0, a1, a2, a3, t0
    oal_int32 reserve[RSV_NUM];
} dcxo_coeff_stru;

typedef struct {
    oal_int32 reserve[COEFF_NUM][NV_CHK_PAIR];  // 保留，之前是存放判断三项式系数的阈值
    oal_int32 t0_bit_width_fix;                 // 0: t0存储的是10位，需要乘以2^6修正, 1:t0存储的是16位，不需要修正
} dcxo_nv_coeff_th_stru;

typedef struct {
    oal_int32 valid;
    union {
        dcxo_coeff_stru fac;
        struct {
            oal_uint32 para[GNSS_COEFF_NUM];
        } gnss;
    } coeff_u;
    dcxo_nv_coeff_th_stru coeff_th;
} dcxo_nv_info_stru;

typedef struct {
    oal_uint16 cali_end;
    oal_uint16 auto_cali_ave0;
    oal_uint16 auto_cali_ave1;
    oal_uint16 auto_cali_ave2;
    oal_uint16 auto_cali_ave3;
    oal_uint16 wifi_ana_en;
    oal_uint16 xoadc_state;
    oal_uint16 reg_addr_low;
    oal_uint16 reg_addr_high;
    oal_uint16 pmu_type;
} dcxo_pmu_addr_stru;

typedef struct {
    dcxo_coeff_stru coeff;
    oal_uint32 tcxo_dcxo_flag;
    dcxo_pmu_addr_stru pmu_addr;
    oal_uint32 check_data;
} dcxo_dl_para_stru;

// 从nv中读取的参数异常
typedef struct {
    oal_int32 l_struct_len;       // 结构体的大小
    oal_int32 l_para[COEFF_NUM];  // 存储依次为 a0, a1, a2, a3, t0
} plat_chr_nv_rd_excep_stru;

// 由于devcie划定的存储空间限制，这里需要做判断
HI_STATIC_ASSERT((DCXO_DEVICE_MAX_BUF_LEN > (sizeof(dcxo_dl_para_stru))), device_mem_must_big_than_host);

typedef struct {
    oal_uint32 use_part_id;
    oal_uint32 chr_nv_read_err;
    int64 nv_init_flag;
    oal_uint32 dcxo_tcxo_mode;
    oal_uint32 pmu_chip_type;
    oal_int32 check_value[COEFF_NUM][NV_CHK_PAIR];  // 最大值/最小值判断
} dcxo_manage_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
extern oal_uint8 *pucDcxoDataBuf;
extern dcxo_manage_stru dcxo_info;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_int32 dcxo_data_buf_malloc(oal_void);
extern void dcxo_data_buf_free(void);
extern oal_void *get_dcxo_data_buf_addr(oal_void);
extern void read_tcxo_dcxo_ini_file(void);
extern int32 get_tcxo_dcxo_mode(void);
extern oal_void update_dcxo_coeff(oal_uint32 *coeff);
extern oal_int32 read_dcxo_cali_data(oal_void);

#endif /* end of platform_common_clk.h */
