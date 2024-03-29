#ifndef _MEMORY_H_INCLUDED
#define _MEMORY_H_INCLUDED

#include "ipp.h"
#include "adapter_common.h"
#include "config_table_cvdr.h"
#include "cfg_table_slam.h"
#include "slam_common.h"

typedef enum _cpe_mem_id
{
    MEM_ID_CMDLST_BUF_MM   = 0,
    MEM_ID_CMDLST_ENTRY_MM = 1,
    MEM_ID_CMDLST_PARA_MM  = 2,
    MEM_ID_CVDR_CFG_TAB_MM   = 3,
    MEM_ID_SLAM_CFG_TAB   = 4,
    MEM_ID_CMDLST_BUF_SLAM   = 5,
    MEM_ID_CMDLST_ENTRY_SLAM = 6,
    MEM_ID_CMDLST_PARA_SLAM  = 7,
    MEM_ID_CVDR_CFG_TAB_SLAM   = 8,
    MEM_ID_SLAM_STAT      = 9,
    MEM_ID_MAX
} cpe_mem_id;

typedef struct _cpe_va_da
{
    unsigned long long va;
    unsigned int       da;
}cpe_va_da;

extern int cpe_mem_init(unsigned long long va, unsigned int da, unsigned int size);
extern int cpe_mem_get(cpe_mem_id mem_id, unsigned long long *va, unsigned int *da);
extern void cpe_mem_free(cpe_mem_id mem_id);

extern int cpe_init_memory(void);
extern void *get_cpe_addr_va(void);
extern unsigned int get_cpe_addr_da(void);

#endif /*_MEMORY_H_INCLUDED*/



