/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */



#ifndef __SI_APP_PIH_H__
#define __SI_APP_PIH_H__

#ifdef  __cplusplus
#if  __cplusplus
extern "C"{
#endif
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "product_config.h"
#include "UsimPsInterface.h"
#include "sitypedef.h"

#if (OSA_CPU_NRCPU != VOS_OSA_CPU)
#include "MnClient.h"
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "ccore_nv_stru_pam.h"
#endif
#include "nv_stru_pam.h"

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define SI_PIH_POLL_TIMER_LEN           28000
#define SI_PIH_IMSI_TIMER_LEN           5000
#define SI_PIH_CALL_TIMER_LEN           4000
#define SI_PIH_TIMER_LEN_NO_CHANGE      0


#define SI_PH_PINCODELENMAX             8
#define SI_MAX_IMSI_LEN                 9

#define SI_MAX_PLMN_ID_LEN              3       /* PLMN ID长度 */
#define SI_MAX_PLMN_ID_NUM              4       /* 锁网时支持的PLMN ID数 */

#define SI_APDU_MAX_LEN                 260     /* APDU数据的最大长度 */

#define SI_PRIVATECGLA_APDU_MAX_LEN     (256*3)    /* APDU数据的最大长度 */

#define SI_ATR_MAX_LEN                  (80)    /* ATR的最大长度 */

#define SI_PIH_VSIMAPN_MAX              (100)   /* VSIM APN 的最大长度，按照TAF的容量定义 */

typedef VOS_UINT32 SI_PIH_ERROR;

#define SI_RSFW_MAX_INDEX               (255)   /* 支持RSFWX分包的最大个数 */
#define SI_PIH_HPLMN_MAX_NUM            (32)    /* 支持HPLMN/EHPLMN的最大个数 */
#define SI_PIH_EHPLMN_BYTE_NUM          (3)     /* EHPLMN文件中每个EHPLMN单元占用3个字节 */

#define SI_AUTH_DATA_MAX                (256)   /* 支持鉴权数据的最大长度 */

#define SI_IMSI_MAX_LEN                 (15 + 1)    /* IMSI 转换成可见字符长度 */

#define SI_PIH_KEYFILE_MAX_NUM          (2)         /* 最大支持2个关键文件检 */

#define SI_PIH_KEYFILE_SEND_PARA        (0xABABAB)  /* 关键文件检测读文件send para */

#define SI_PIH_KEYFILE_INIT_TIME_LEN    (5)         /* 关键文件检测初始定时器时长，多次检测失败后，定时器拉长 */

#define SI_PIH_KEYFILE_INIT_CHECK_TIMER (5)         /* 关键文件按照初始定时器时长，检测次数 */

#define SI_PIH_KEYFILE_TIME_LEN         (60)        /* 关键文件检测拉长后定时器时长 */

#define SI_CRYPTO_CBC_PIN_LEN           (16)  /* 密码的密文长度AES_CCB加密 */

#define SI_SIGNATURE_LEN                (32)  /* 签名长度 */

#define SI_PIH_SEND_PAPA(ClientId, EventType)   (ClientId | (EventType << 16))
#define SI_PIH_CLIENT_ID(Para)                  (Para & 0xFFFF)
#define SI_PIH_EVENT_TYPE(Para)                 ((Para & 0xFFFF0000) >> 16)

/*******************************************************************************
  3 枚举定义
*******************************************************************************/

enum SI_PIH_PIN_TYPE_ENUM
{
    SI_PIN                          = 0,      /* PIN1*/
    SI_PUK                          = 1,      /* PUK1*/
    SI_PIN2                         = 6,      /* PIN2*/
    SI_PUK2                         = 7,      /* PUK2 */
    SI_PHNET_PIN                    = 8,      /* PH-NET PIN*/
    SI_PHSIM_PIN                    = 9,      /* PH-SIM PIN*/
    SI_SIM_NON                      = 255,    /* 不确定操作PIN的类型时填入此值*/
    SI_PIN_BUTT
};
typedef VOS_UINT32 SI_PIH_PIN_TYPE;

enum SI_PIH_RESTRIC_CMD_ENUM
{
    SI_PIH_READ_BINARY              = 176,
    SI_PIH_READ_RECORD              = 178,
    SI_PIH_UPDATE_BINARY            = 214,
    SI_PIH_UPDATE_RECORD            = 220,
    SI_PIH_STATUS                   = 242,
    SI_PIH_CMD_BUTT
};

typedef VOS_UINT32 SI_PIH_RESTRIC_CMD;

enum SI_PIH_EVENT_ENUM
{
    SI_PIH_EVENT_PIN_OPER_CNF               = 1,      /* 操作PIN码返回结果        */
    SI_PIH_EVENT_PIN_QUERY_CNF              = 2,      /* 查询PIN码返回结果        */
    SI_PIH_EVENT_PIN_INFO_IND               = 3,      /* 开机PIN码主动上报        */
    SI_PIH_EVENT_SIM_INFO_IND               = 4,      /* SIM卡信息上报            */
    SI_PIH_EVENT_GENERIC_ACCESS_CNF         = 5,      /* +CSIM 回复               */
    SI_PIH_EVENT_RESTRIC_ACCESS_CNF         = 6,      /* +CRSM 回复               */
    SI_PIH_EVENT_FDN_CNF                    = 7,      /* FDN回复                  */
    SI_PIH_EVENT_BDN_CNF                    = 8,      /* BDN回复                  */
    SI_PIH_EVENT_PERO_LOCK_CNF              = 9,      /* 锁卡回复                 */
    SI_PIH_EVENT_ISDB_ACCESS_CNF            = 10,     /* ISDB透传回复             */
    SI_PIH_EVENT_HVSST_QUERY_CNF            = 11,       /* ^HVSST查询命令返回       */
    SI_PIH_EVENT_HVSST_SET_CNF              = 12,       /* ^HVSST设置命令返回       */
    SI_PIH_EVENT_HVSDH_SET_CNF              = 13,       /* ^HVSDH设置命令返回       */
    SI_PIH_EVENT_HVSDH_QRY_CNF              = 14,       /* ^HVSDH查询命令返回       */
    SI_PIH_EVENT_HVSCONT_QUERY_CNF          = 15,       /* ^HVSCONT查询命令返回     */
    SI_PIH_EVENT_FILE_WRITE_CNF             = 16,       /* ^RSFW设置命令返回        */
    SI_PIH_EVENT_CCHO_SET_CNF               = 17,       /* +CCHO设置命令返回        */
    SI_PIH_EVENT_CCHC_SET_CNF               = 18,       /* +CCHC设置命令返回        */
    SI_PIH_EVENT_CGLA_SET_CNF               = 19,       /* +CGLA设置命令返回        */
    SI_PIH_EVENT_CARD_ATR_QRY_CNF           = 20,       /* ^CARDATR查询命令返回     */
    SI_PIH_EVENT_HVRDH_IND                  = 21,       /* ^HVRDH指示上报           */
    SI_PIH_EVENT_UICCAUTH_CNF               = 22,       /* ^UICCAUTH设置命令返回    */
    SI_PIH_EVENT_URSM_CNF                   = 23,       /* ^URSM设置命令返回        */
    SI_PIH_EVENT_CARDTYPE_QUERY_CNF         = 24,
    SI_PIH_EVENT_CRSM_SET_CNF               = 25,
    SI_PIH_EVENT_CRLA_SET_CNF               = 26,
    SI_PIH_EVENT_SESSION_QRY_CNF            = 27,

    SI_PIH_EVENT_SCICFG_SET_CNF             = 28,
    SI_PIH_EVENT_SCICFG_QUERY_CNF           = 29,
    SI_PIH_EVENT_HVTEE_SET_CNF              = 30,
    SI_PIH_EVENT_TEETIMEOUT_IND             = 31,
    SI_PIH_EVENT_HVCHECKCARD_CNF            = 32,
    SI_PIH_EVENT_CIMI_QRY_CNF               = 33,
    SI_PIH_EVENT_CCIMI_QRY_CNF              = 34,
    SI_PIH_EVENT_SIM_ERROR_IND              = 35,     /* SIM卡Error信息上报       */

    SI_PIH_EVENT_SIM_ICCID_IND              = 36,
    SI_PIH_CALL_BACK_TYPE_ICC_SEC_CHANNEL   = 37,  /* icc sec ch call back */

    SI_PIH_EVENT_SIM_HOTPLUG_IND            = 38,

    SI_PIH_EVENT_CCHP_SET_CNF               = 39,        /* +CCHP设置命令返回 */

    SI_PIH_EVENT_SW_CHECK_IND               = 40,        /* Apdu SW check */

    SI_PIH_EVENT_CARDVOLTAGE_QUERY_CNF      = 41,
    SI_PIH_EVENT_PRIVATECGLA_SET_IND        = 42,       /* ^CGLA设置命令上报        */
    SI_PIH_EVENT_PRIVATECGLA_SET_CNF        = 43,       /* ^CGLA设置命令返回        */
    SI_PIH_EVENT_CARDTYPEEX_QUERY_CNF       = 44,
#if (FEATURE_ON == FEATURE_PHONE_SC)
    SI_PIH_EVENT_SILENT_PIN_SET_CNF         = 45,
    SI_PIH_EVENT_SILENT_PININFO_SET_CNF     = 46,
#endif
    SI_PIH_EVENT_IMSI_POLLING_SET_CNF       = 47, /* PIH周期性IMSI轮询使用，为了格式统一 */

    SI_PIH_EVENT_CARDSTATUS_IND             = 48,

    SI_PIH_EVENT_ESIMSWITCH_SET_CNF         = 49,
    SI_PIH_EVENT_ESIMSWITCH_QRY_CNF         = 50,
    SI_PIH_EVENT_BWT_SET_CNF                = 51,

    /*从500开始作为PIH内部可维可测消息的勾包*/
    SI_PIH_USIMREG_PID_HOOK                 = 500,
    SI_PIH_REFRESHREG_PID_HOOK              = 501,
    SI_PIH_ISIMREG_PID_HOOK                 = 502,
    SI_PIH_HVTEE_DATA_HOOK                  = 503,
#if (FEATURE_VSIM == FEATURE_ON)
    SI_PIH_VSIM_APN_DATA_HOOK               = 504,
    SI_PIH_VSIM_FILE_DATA_HOOK              = 505,
#endif
    SI_PIH_EVENT_BUTT
};
typedef VOS_UINT32  SI_PIH_EVENT;

enum SI_PIH_POLLTIMER_STATE_ENUM
{
    SI_PIH_POLLTIMER_DISABLE        = 0,
    SI_PIH_POLLTIMER_ENABLE         = 1,
    SI_PIH_POLLTIMER_BUTT
};
typedef VOS_UINT32      SI_PIH_POLLTIMER_STATE_ENUM_UINT32;

enum SI_PIH_TIMER_NAME_ENUM
{
    SI_PIH_TIMER_NAME_CHECKSTATUS       = 0,
    SI_PIH_TIMER_NAME_CALL              = 1,
    SI_PIH_TIMER_NAME_CHECKIMSI         = 2,
    SI_PIH_TIMER_NAME_CHECK_KEYFILE     = 3,

    SI_PIH_TIMER_NAME_BUTT
};
typedef VOS_UINT32      SI_PIH_TIMER_NAME_ENUM_UINT32;

enum SI_PIH_QUERY_TYPE_ENUM
{
    SI_PIH_BDN_QUERY                = 0,
    SI_PIH_FDN_QUERY                = 1,
    SI_PIH_QUERY_BUTT
};
typedef VOS_UINT32      SI_PIH_QUERY_TYPE_ENUM_UINT32;

enum SI_PIH_FDN_BDN_STATE_ENUM
{
    SI_PIH_STATE_FDN_BDN_DISABLE    = 0,     /*FDN/BDN功能未使能*/
    SI_PIH_STATE_FDN_BDN_ENABLE     = 1,     /*FDN/BDN功能使能*/
    SI_PIH_STATE_FDN_BDN_BUTT
};
typedef VOS_UINT32   SI_PIH_FDN_BDN_STATE;


enum SI_PIH_FDN_BDN_CMD_ENUM
{
    SI_PIH_FDN_BDN_DISABLE          = 0,     /*去激活FDN/BDN功能*/
    SI_PIH_FDN_BDN_ENABLE           = 1,     /*激活FDN/BDN功能*/
    SI_PIH_FDN_BDN_QUERY            = 2,     /*去激活FDN/BDN状态查询*/
    SI_PIH_FDN_BDN_CMD_BUTT
};
typedef VOS_UINT32   SI_PIH_FDN_BDN_CMD;

enum SI_PIH_SIM_INDEX_ENUM
{
    SI_PIH_SIM_REAL_SIM1          = 1,
    SI_PIH_SIM_VIRT_SIM1          = 11,
    SI_PIH_SIM_INDEX_BUTT
};
typedef VOS_UINT8 SI_PIH_SIM_INDEX_ENUM_UINT8;

enum SI_PIH_SIM_SLOT_ENUM
{
    SI_PIH_SIM_SLOT1                = 1,
    SI_PIH_SIM_SLOT2                = 2,
    SI_PIH_SIM_SLOT3                = 3,
    SI_PIH_SIM_SLOT_BUTT
};
typedef VOS_UINT8 SI_PIH_SIM_SLOT_ENUM_UINT8;

enum SI_PIH_SIM_STATE_ENUM
{
    SI_PIH_SIM_DISABLE              = 0,
    SI_PIH_SIM_ENABLE               = 1,
    SI_PIH_SIM_STATE_BUTT
};
typedef VOS_UINT8 SI_PIH_SIM_STATE_ENUM_UINT8;

enum SI_PIH_CARD_USE_ENUM
{
    SI_PIH_CARD_NOUSE               = 0,
    SI_PIH_CARD_USE                 = 1,
    SI_PIH_CARD_USE_BUTT
};
typedef VOS_UINT8 SI_PIH_CARD_USE_ENUM_UINT8;

enum SI_PIH_CARD_TYPE_ENUM
{
    SI_PIH_RSIM = 0,
    SI_PIH_VSIM,
    SI_PIH_CARD_BUTT,
};

enum SI_PIH_ATUICCAUTH_TYPE_ENUM
{
    SI_PIH_ATUICCAUTH_USIMAKA   = 0,
    SI_PIH_ATUICCAUTH_ISIMAKA,
    SI_PIH_ATUICCAUTH_USIMGBA,
    SI_PIH_ATUICCAUTH_ISIMGBA,
    SI_PIH_ATUICCAUTH_BUTT
};

enum SI_PIH_ATKSNAFAUTH_TYPE_ENUM
{
    SI_PIH_ATKSNAFAUTH_USIM     = 0,
    SI_PIH_ATKSNAFAUTH_ISIM,
    SI_PIH_ATKSNAFAUTH_BUTT
};

enum SI_PIH_UICCAUTH_ENUM
{
    SI_PIH_UICCAUTH_AKA,
    SI_PIH_UICCAUTH_GBA,
    SI_PIH_UICCAUTH_NAF,
    SI_PIH_UICCAUTH_BUTT
};
typedef VOS_UINT32 SI_PIH_UICCAUTH_ENUM_UINT32;

enum SI_PIH_UICCAPP_ENUM
{
    SI_PIH_UICCAPP_USIM,
    SI_PIH_UICCAPP_ISIM,
    SI_PIH_UICCAPP_BUTT
};
typedef VOS_UINT32 SI_PIH_UICCAPP_ENUM_UINT32;

enum SI_PIH_ACCESSTYPE_ENUM
{
    SI_PIH_ACCESS_READ,
    SI_PIH_ACCESS_WRITE,
    SI_PIH_ACCESS_BUTT
};
typedef VOS_UINT8 SI_PIH_ACCESSTYPE_ENUM_UINT8;

enum SI_PIH_AUTHSTATUS_ENUM
{
    SI_PIH_AUTH_SUCCESS,
    SI_PIH_AUTH_FAIL,
    SI_PIH_AUTH_SYNC,
    SI_PIH_AUTH_UNSUPPORT,
    SI_PIH_AUTHSTATUS_BUTT
};
typedef VOS_UINT32 SI_PIH_AUTHSTATUS_ENUM_UINT32;

enum SI_PIH_CARD_VERSION_TYPE
{
    SI_PIH_CARD_NON_TYPE             = 0,
    SI_PIH_CARD_ICC_TYPE             = 1,
    SI_PIH_CARD_UICC_TYPE            = 2,
    SI_PIH_CARD_VERSION_TYPE_BUTT
};
typedef  VOS_UINT8  SI_PIH_CARD_VERSION_TYPE_UINT8;

enum SI_PIH_HVTEE_ERROR_ENUM
{
    SI_PIH_HVTEE_NOERROR    = VOS_OK,
    SI_PIH_HVTEE_ADDR_ERROR,
    SI_PIH_HVTEE_HEAD_ERROR,
    SI_PIH_HVTEE_END_ERROR,
    SI_PIH_HVTEE_DATA_ERROR,
    SI_PIH_HVTEE_LEN_ERROR,
    SI_PIH_HVTEE_ENCODE_ERROR,
    SI_PIH_HVTEE_ERROR_BUTT
};
typedef VOS_UINT32  SI_PIH_HVTEE_ERROR_ENUM_UINT32;

enum SI_PIH_HVCHECKCARD_STATUS_ENUM
{
    SI_PIH_HVCHECKCARD_CARDIN   = 0,
    SI_PIH_HVCHECKCARD_ABSENT   = 1,
    SI_PIH_HVCHECKCARD_BUTT
};
typedef VOS_UINT32  SI_PIH_HVCHECKCARD_STATUS_ENUM_UINT32;

enum SI_PIH_CHANGEPOLLTIMER_ENUM
{
    SI_PIH_CHANGEPOLLTIMER_TIMERLEN = 0,
    SI_PIH_CHANGEPOLLTIMER_TIMEROFF = 1,
    SI_PIH_CHANGEPOLLTIMER_CALLON   = 2,
    SI_PIH_CHANGEPOLLTIMER_CALLOFF  = 3,
    SI_PIH_CHANGEPOLLTIMER_BUTT
};
typedef VOS_UINT32  SI_PIH_CHANGEPOLLTIMER_ENUM_UINT32;

/*****************************************************************************
 枚举名    : SI_PIH_CMD_REQ_TYPE_ENUM
 结构说明  : SI_PIH对外消息名称枚举
*****************************************************************************/
enum SI_PIH_CMD_REQ_TYPE_ENUM
{
    SI_PIH_QUERY_CARDSTATUS_REQ     = 0x1000,
    SI_PIH_START_CHECK_KEYFILE_NTF,
    SI_PIH_STOP_CHECK_KEYFILE_NTF,
    SI_PIH_CMD_REQ_TYPE_ENUM_BUTT
};
typedef VOS_UINT32  SI_PIH_CMD_REQ_TYPE_ENUM_UINT32;

/*****************************************************************************
 枚举名    : SI_PIH_CMD_CNF_TYPE_ENUM
 结构说明  : SI_PIH对外回复消息名称枚举
*****************************************************************************/
enum SI_PIH_CMD_CNF_TYPE_ENUM
{
    SI_PIH_QUERY_CARDSTATUS_CNF = 0x2000,

    SI_PIH_CHECK_KEYFILE_RLST_IND,

    SI_PIH_VSIM_APN_VALUE_IND,

    SI_PIH_CMD_CNF_TYPE_ENUM_BUTT
};
typedef VOS_UINT32  SI_PIH_CMD_CNF_TYPE_ENUM_UINT32;

/*****************************************************************************
  4 数据结构定义
*****************************************************************************/


typedef struct
{
    VOS_UINT32                          ulLen;                                  /* 输入APDU数据长度 */
    VOS_UINT8                           aucCommand[SI_APDU_MAX_LEN];            /* 输入APDU数据内容 */
}SI_PIH_ISDB_ACCESS_COMMAND_STRU;

#if  ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT))) || (defined(__PC_UT__))

typedef struct
{
    VOS_UINT32                          ulAIDLen;                               /* AID的长度 */
    VOS_UINT32                          ulRsv;
    VOS_UINT8                           *pucADFName;                             /* 保存ADF的名字 */
}SI_PIH_CCHO_COMMAND_STRU;


typedef struct
{
    VOS_UINT32                          ulAIDLen;                               /* AID的长度 */
    VOS_UINT8                           ucAPDUP2;                               /* Save APDU para P2 for OMA3.0 */
    VOS_UINT8                           ucRsv[3];
    VOS_UINT8                          *pucADFName;                             /* 保存ADF的名字 */
}SI_PIH_CCHP_COMMAND_STRU;


typedef struct
{
    VOS_UINT32                          ulSessionID;                            /* 通道号 */
    VOS_UINT32                          ulLen;                                  /* 输入APDU数据长度 */
    VOS_UINT8                           *pucCommand;                             /* 输入APDU数据内容 */
}SI_PIH_CGLA_COMMAND_STRU;
#endif


typedef struct
{
    VOS_UINT16                          usLen;                                    /* 输出APDU数据长度 */
    VOS_UINT8                           ucSW1;                                    /* 返回状态字1      */
    VOS_UINT8                           ucSW2;                                    /* 返回状态字2      */
    VOS_UINT8                           aucCommand[SI_APDU_MAX_LEN];              /* 输出APDU数据内容 */
}SI_PIH_CGLA_COMMAND_CNF_STRU;


typedef struct
{
    VOS_UINT16                          usLen;                                    /* 输出APDU数据长度 */
    VOS_UINT8                           ucLastDataFlag;
    VOS_UINT8                           ucSW1;                                    /* 返回状态字1      */
    VOS_UINT8                           ucSW2;                                    /* 返回状态字2      */
    VOS_UINT8                           aucRsv[3];
    VOS_UINT8                           aucCommand[SI_PRIVATECGLA_APDU_MAX_LEN];  /* 输出APDU数据内容 */
}SI_PIH_CGLA_HANDLE_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulLen;                                    /* 输出APDU数据长度 */
    VOS_UINT8                           aucCommand[SI_ATR_MAX_LEN];            /* 输出APDU数据内容 */
}SI_PIH_ATR_QRY_CNF_STRU;

/*****************************************************************************
 结构名    : SI_PIH_ISDB_ACCESS_COMMAND_CNF_STRU
 结构说明  : ISDB透传APDU的数据回复结果
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usLen;                                    /* 输出APDU数据长度 */
    VOS_UINT8                           ucSW1;                                    /* 返回状态字1      */
    VOS_UINT8                           ucSW2;                                    /* 返回状态字2      */
    VOS_UINT8                           aucCommand[SI_APDU_MAX_LEN];               /* 输出APDU数据内容 */
}SI_PIH_ISDB_ACCESS_COMMAND_CNF_STRU;

typedef struct
{
    VOS_UINT32                          ulLen;
    VOS_UINT8                           aucCommand[SI_APDU_MAX_LEN];
}SI_PIH_CSIM_COMMAND_STRU;

typedef struct
{
    SI_PIH_FDN_BDN_CMD                  FdnCmd;
    SI_PIH_FDN_BDN_STATE                FdnState;
}SI_PIH_EVENT_FDN_CNF_STRU;

typedef struct
{
    SI_PIH_FDN_BDN_CMD                  BdnCmd;
    SI_PIH_FDN_BDN_STATE                BdnState;
}SI_PIH_EVENT_BDN_CNF_STRU;

typedef struct
{
    VOS_UINT16                          Len;
    VOS_UINT8                           SW1;
    VOS_UINT8                           SW2;
    VOS_UINT8                           Command[256];
}SI_PIH_CSIM_COMMAND_CNF_STRU;

typedef struct
{
    SI_PIH_RESTRIC_CMD                  CmdType;
    VOS_UINT16                          fileId;
    VOS_UINT8                           Para1;
    VOS_UINT8                           Para2;
    VOS_UINT8                           Para3;
    VOS_UINT8                           Rsv[3];
    VOS_UINT8                           Command[256];
}SI_RESTRIC_ACCESS_STRU;

typedef struct
{
    SI_PIH_SIM_STATE_ENUM_UINT8         enVSimState;    /*vSIM卡状态，和硬卡状态互斥*/
    SI_PIH_CARD_USE_ENUM_UINT8          enCardUse;      /*卡能否使用*/
    VOS_UINT8                           aucRsv[2];
}SI_PIH_EVENT_HVSST_QUERY_CNF_STRU;

typedef struct
{
    VOS_UINT8                           ucIndex;
    SI_PIH_SIM_STATE_ENUM_UINT8         enSIMSet;
    VOS_UINT8                           ucSlot;
    VOS_UINT8                           ucRsv;
} SI_PIH_HVSST_SET_STRU;

/*****************************************************************************
 结构名    : SI_PIH_ESIMSWITCH_SET_STRU
 结构说明  : AT^ESIMSWITCH设置消息结构体
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucSlot;
    VOS_UINT8                           ucCardType;
    VOS_UINT8                           aucRsv[2];
} SI_PIH_ESIMSWITCH_SET_STRU;

typedef struct
{
    VOS_UINT32                          ulMCC;
    VOS_UINT32                          ulMNC;
}SI_PIH_PLMN_STRU;

typedef struct
{
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           ucCardCap;  /* 按BIT位进行存放，BIT1:SIM,BIT2:USIM */
    VOS_UINT8                           ucCardType; /* 按BIT位进行存放，BIT1:SIM,BIT2:USIM */
    VOS_UINT8                           ucRsv;
    VOS_UINT8                           aucImsi[USIMM_EF_IMSI_LEN*2];   /* IMSI,需要传送ASCII码长度增大一倍 */
    VOS_UINT16                          usPlmnNum;  /* PLMN个数 */
    SI_PIH_PLMN_STRU                    astPlmn[SI_PIH_HPLMN_MAX_NUM]; /* PLMN内容 */
}SI_PIH_CARD_CONTENT_STRU;

typedef struct
{
    SI_PIH_CARD_CONTENT_STRU            astSimCard[SI_PIH_CARD_BUTT];   /* 支持卡的最大个数包括硬卡和虚拟卡 */
}SI_PIH_HVSCONT_QUERY_CNF_STRU;

typedef struct
{
    VOS_BOOL                            ulReDhFlag;
}SI_PIH_HVRDH_IND_STRU;

typedef struct
{
    VOS_UINT8                       aucIK[17];          /*Len+Data*/
    VOS_UINT8                       aucCK[17];          /*Len+Data*/
    VOS_UINT8                       aucAuts[15];        /*Len+Data*/
    VOS_UINT8                       aucAuthRes[17];     /*Len+Data*/
    VOS_UINT8                       aucRsv[2];
}SI_PIH_UICCAKA_DATA_STRU;

typedef struct
{
    VOS_UINT8                       aucKs_ext_NAF[USIMM_T0_APDU_MAX_LEN];   /*Len+Data*/
}SI_PIH_UICCNAF_DATA_STRU;

typedef struct
{
    SI_PIH_AUTHSTATUS_ENUM_UINT32       enStatus;
    SI_PIH_UICCAPP_ENUM_UINT32          enAppType;      /* 应用类型 */
    SI_PIH_UICCAUTH_ENUM_UINT32         enAuthType;     /* 鉴权类型 */
    SI_PIH_UICCAKA_DATA_STRU            stAkaData;
    SI_PIH_UICCNAF_DATA_STRU            stNAFData;
}SI_PIH_UICCAUTH_CNF_STRU;

typedef struct
{
    SI_PIH_ACCESSTYPE_ENUM_UINT8        enCmdType;
    VOS_UINT8                           ucRecordNum;
    VOS_UINT16                          usEfId;
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulDataLen;
    VOS_UINT8                           aucCommand[USIMM_T0_APDU_MAX_LEN];
}SI_PIH_UICCACCESSFILE_CNF_STRU;

typedef struct
{
    VOS_UINT32                          ulDataLen;
    VOS_UINT32                          ulRsv;
    VOS_UINT8                           *pucData;
}SI_PIH_U8LV_DATA_STRU;

typedef struct
{
    SI_PIH_U8LV_DATA_STRU               stFileName;
    SI_PIH_U8LV_DATA_STRU               stFileSubName;
    VOS_UINT32                          ulRef;
    VOS_UINT32                          ulTotalNum;
    VOS_UINT32                          ulIndex;
    VOS_UINT32                          ulRsv;
    SI_PIH_U8LV_DATA_STRU               stFileData;
}SI_PIH_FILEWRITE_DATA_STRU;

typedef struct
{
    VOS_UINT32                          ulRandLen;
    VOS_UINT8                           aucRand[SI_AUTH_DATA_MAX];
    VOS_UINT32                          ulAuthLen;
    VOS_UINT8                           aucAuth[SI_AUTH_DATA_MAX];
}SI_PIH_UICCAUTH_AKA_STRU;

typedef struct
{
    VOS_UINT32                          ulNAFIDLen;
    VOS_UINT8                           aucNAFID[SI_AUTH_DATA_MAX];
    VOS_UINT32                          ulIMPILen;
    VOS_UINT8                           aucIMPI[SI_AUTH_DATA_MAX];
}SI_PIH_UICCAUTH_NAF_STRU;

typedef struct
{
    SI_PIH_UICCAUTH_ENUM_UINT32         enAuthType;
    SI_PIH_UICCAPP_ENUM_UINT32          enAppType;
    union
    {
        SI_PIH_UICCAUTH_AKA_STRU        stAka;
        SI_PIH_UICCAUTH_NAF_STRU        stKsNAF;
    }uAuthData;
}SI_PIH_UICCAUTH_STRU;

typedef struct
{
    SI_PIH_UICCAPP_ENUM_UINT32          enAppType;
    SI_PIH_ACCESSTYPE_ENUM_UINT8        enCmdType;
    VOS_UINT8                           ucRecordNum;
    VOS_UINT16                          usEfId;
    VOS_UINT32                          ulRsv;
    VOS_UINT32                          ulDataLen;
    VOS_UINT8                           aucCommand[USIMM_T0_APDU_MAX_LEN];
    VOS_UINT16                          usPathLen;
    VOS_UINT16                          ausPath[USIMM_MAX_PATH_LEN];
    VOS_UINT16                          usRsv;
}SI_PIH_ACCESSFILE_STRU;


typedef struct
{
    USIMM_RESTRIC_CMD_ENUM_UINT32       enCmdType;
    VOS_UINT16                          usEfId;
    VOS_UINT16                          usPathLen;
    VOS_UINT8                           ucP1;
    VOS_UINT8                           ucP2;
    VOS_UINT8                           ucP3;
    VOS_UINT8                           ucRsv;
    VOS_UINT16                          ausPath[USIMM_MAX_PATH_LEN];
    VOS_UINT8                           aucContent[USIMM_T0_APDU_MAX_LEN+1];
    VOS_UINT8                           aucRsv[3];
}SI_PIH_CRSM_STRU;


typedef struct
{
    VOS_UINT32                          ulSessionID;
    USIMM_RESTRIC_CMD_ENUM_UINT32       enCmdType;
    VOS_UINT16                          usEfId;
    VOS_UINT16                          usPathLen;
    VOS_UINT8                           ucP1;
    VOS_UINT8                           ucP2;
    VOS_UINT8                           ucP3;
    VOS_UINT8                           ucRsv;
    VOS_UINT16                          ausPath[USIMM_MAX_PATH_LEN];
    VOS_UINT8                           aucContent[USIMM_T0_APDU_MAX_LEN+1];
    VOS_UINT8                           aucRsv[3];
}SI_PIH_CRLA_STRU;

typedef struct
{
    VOS_BOOL                            bAPNFlag;
    VOS_BOOL                            bDHParaFlag;
    VOS_BOOL                            bVSIMDataFlag;
    VOS_UINT32                          ulRfu;
}SI_PIH_HVTEE_SET_STRU;

typedef struct
{
    SI_PIH_CARD_VERSION_TYPE_UINT8      ucMode;
    VOS_UINT8                           ucHasCModule;
    VOS_UINT8                           ucHasGModule;
    VOS_UINT8                           ucRfu;
} SI_PIH_EVENT_CARDTYPE_QUERY_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucSW1;                                      /* Status Word 1*/
    VOS_UINT8                           ucSW2;                                      /* Status Word 2 */
    VOS_UINT16                          usLen;                                      /* 返回数据长度,不包含SW1和SW2       */
    VOS_UINT8                           aucContent[USIMM_APDU_RSP_MAX_LEN];         /* 返回Data*/
}SI_PIH_RACCESS_CNF_STRU;

typedef struct
{
    SI_PIH_CARD_SLOT_ENUM_UINT32        enCard0Slot;
    SI_PIH_CARD_SLOT_ENUM_UINT32        enCard1Slot;
    SI_PIH_CARD_SLOT_ENUM_UINT32        enCard2Slot;
}SI_PIH_SCICFG_QUERY_CNF_STRU;

typedef struct
{
    VOS_UINT32                          ulAPNResult;
    VOS_UINT32                          ulDHResult;
    VOS_UINT32                          ulVSIMResult;
    VOS_UINT32                          ulRsv;
}SI_PIH_HVTEE_SET_CNF_STRU;

typedef struct
{
    VOS_UINT32                          ulData;
    VOS_UINT32                          ulRsv;
}SI_PIH_TEETIMEOUT_IND_STRU;

typedef struct
{
    SI_PIH_HVCHECKCARD_STATUS_ENUM_UINT32   enData;
}SI_PIH_HVCHECKCARD_CNF_STRU;

/* +CIMI - 获取IMSI */
typedef struct
{
    VOS_UINT8  aucImsi[SI_IMSI_MAX_LEN];
}SI_PIH_EVENT_IMSI_STRU;

typedef struct
{
    VOS_UINT32                          ulVoltage;
    VOS_UINT8                           ucCharaByte;
    VOS_UINT8                           aucRsv[3];
} SI_PIH_EVENT_CARDVOLTAGE_QUERY_CNF_STRU;


typedef struct
{
    VOS_UINT8                            aucCryptoPin[SI_CRYPTO_CBC_PIN_LEN];   /* PIN密秘文 */
    VOS_UINT32                           aulPinIv[4];                           /* IV值 */
    VOS_UINT8                            aucHmacValue[SI_SIGNATURE_LEN];        /* hmac签名 */
} SI_PIH_CRYPTO_PIN_STRU;

/*****************************************************************************
  5 回调函数数据结构定义
*****************************************************************************/

typedef struct
{
    VOS_UINT16                          ClientId;
    VOS_UINT8                           OpId;
    VOS_UINT8                           Reserve;
    SI_PIH_EVENT                        EventType;
    SI_PIH_ERROR                        PIHError;
    union
    {
        SI_PIH_EVENT_FDN_CNF_STRU                           FDNCnf;
        SI_PIH_EVENT_BDN_CNF_STRU                           BDNCnf;
        SI_PIH_CSIM_COMMAND_CNF_STRU                        GAccessCnf;
        SI_PIH_ISDB_ACCESS_COMMAND_CNF_STRU                 IsdbAccessCnf;
        VOS_UINT32                                          ulSessionID;
        SI_PIH_CGLA_COMMAND_CNF_STRU                        stCglaCmdCnf;
        SI_PIH_ATR_QRY_CNF_STRU                             stATRQryCnf;
        SI_PIH_EVENT_HVSST_QUERY_CNF_STRU                   HVSSTQueryCnf;

        SI_PIH_HVSCONT_QUERY_CNF_STRU                       HvsContCnf;
        SI_PIH_HVRDH_IND_STRU                               HvrdhInd;
        SI_PIH_UICCAUTH_CNF_STRU                            UiccAuthCnf;
        SI_PIH_UICCACCESSFILE_CNF_STRU                      UiccAcsFileCnf;
        SI_PIH_EVENT_CARDTYPE_QUERY_CNF_STRU                CardTypeCnf;
        SI_PIH_RACCESS_CNF_STRU                             RAccessCnf;
        VOS_UINT32                                          aulSessionID[USIMM_CARDAPP_BUTT];
        SI_PIH_SCICFG_QUERY_CNF_STRU                        SciCfgCnf;
        SI_PIH_HVTEE_SET_CNF_STRU                           HVTEECnf;
        SI_PIH_TEETIMEOUT_IND_STRU                          TEETimeOut;
        SI_PIH_HVCHECKCARD_CNF_STRU                         HvCheckCardCnf;
        SI_PIH_EVENT_IMSI_STRU                              stImsi;
        USIMM_ERRORINFO_DATA_STRU                           UsimmErrorInd;
        VOS_UINT8                                           aucIccidContent[USIMM_ICCID_FILE_LEN];

        VOS_UINT32                                          ulSimHotPlugStatus;

        VOS_UINT32                                          ulApduSWCheckResult;

        SI_PIH_EVENT_CARDVOLTAGE_QUERY_CNF_STRU             stCardVoltageCnf;
        SI_PIH_CGLA_HANDLE_CNF_STRU                         stCglaHandleCnf;

#if (FEATURE_ON == FEATURE_PHONE_SC)
        SI_PIH_CRYPTO_PIN_STRU                              stCryptoPin;
#endif
        USIMM_CARDSTATUS_IND_STRU                           stCardStatusInd;
        USIMM_SLOT_CARDTYPE_STRU                            stSlotCardType;

    }PIHEvent;
}SI_PIH_EVENT_INFO_STRU;

typedef struct
{
    VOS_MSG_HEADER
    SI_PIH_CHANGEPOLLTIMER_ENUM_UINT32  enMsgName;       /* 消息名 */
    VOS_UINT32                          ulTimerLen;
}SI_PIH_CHANGEPOLLTIMER_REQ_STRU;

/*****************************************************************************
 结构名    : SI_PIH_FILE_INFO_STRU
 结构说明  : NAS关键文件检通列表结构体
*****************************************************************************/
typedef struct
{
    USIMM_CARDAPP_ENUM_UINT32           enAppType;
    USIMM_FILEPATH_INFO_STRU            stFilePath;
}SI_PIH_FILE_INFO_STRU;

/*****************************************************************************
 结构名    : SI_PIH_START_CHECK_KEYFILE_NTF_STRU
 结构说明  : NAS关键文件检通知PIH消息结构体
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    SI_PIH_CMD_REQ_TYPE_ENUM_UINT32     enMsgName;
    VOS_UINT32                          ulFileNum;
    SI_PIH_FILE_INFO_STRU               astFileInfo[SI_PIH_KEYFILE_MAX_NUM];
}SI_PIH_START_CHECK_KEYFILE_NTF_STRU;

/*****************************************************************************
 结构名    : SI_PIH_STOP_CHECK_KEYFILE_NTF_STRU
 结构说明  : 关键文件检停止消息结构体
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    SI_PIH_CMD_REQ_TYPE_ENUM_UINT32     enMsgName;
}SI_PIH_STOP_CHECK_KEYFILE_NTF_STRU;

/*****************************************************************************
 结构名    : SI_PIH_CHECK_KEYFILE_RLST_IND_STRU
 结构说明  : 关键文件检测通知NAS消息结构体
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    SI_PIH_CMD_CNF_TYPE_ENUM_UINT32     enMsgName;
    USIMM_CARDAPP_ENUM_UINT32           enAppType;
    USIMM_FILEPATH_INFO_STRU            stFilePath;
    VOS_UINT32                          ulFileLen;
    VOS_UINT8                           aucFileData[4];
}SI_PIH_CHECK_KEYFILE_RLST_IND_STRU;

/*****************************************************************************
 结构名    : SI_PIH_QUERY_CARDSTATUS_REQ_STRU
 结构说明  : 卡应用状态查询结构体
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    SI_PIH_CMD_REQ_TYPE_ENUM_UINT32     ulMsgName;
}SI_PIH_QUERY_CARDSTATUS_REQ_STRU;

/*****************************************************************************
 结构名    : SI_PIH_CARDAPP_STATUS_STRU
 结构说明  : 卡应用状态信息
*****************************************************************************/
typedef struct
{
    USIMM_CARDAPP_SERVIC_ENUM_UINT32    enCardAppService;
    VOS_UINT32                          ulIsTestCard;                    /* VOS_TRUE为测试卡，VOS_FALSE为非测试卡 */
}SI_PIH_CARDAPP_STATUS_STRU;

/*****************************************************************************
 结构名    : SI_PIH_QUERY_CARDSTATUS_CNF_STRU
 结构说明  : 卡应用状态回复结构体
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    SI_PIH_CMD_CNF_TYPE_ENUM_UINT32     ulMsgName;
    USIMM_PHYCARD_TYPE_ENUM_UINT32      enPhyCardType;  /*物理卡状态*/
    SI_PIH_CARDAPP_STATUS_STRU          stUsimSimInfo;  /*GUL SIM状态*/
    SI_PIH_CARDAPP_STATUS_STRU          stCsimUimInfo;  /*CDMA SIM状态*/
    SI_PIH_CARDAPP_STATUS_STRU          stIsimInfo;     /*ISIM状态*/
    USIMM_CARDSTATUS_ADDINFO_STRU       stAddInfo;      /*卡状态有效时候才能使用里面的信息*/
}SI_PIH_QUERY_CARDSTATUS_CNF_STRU;

/*外部接口*/
#if (FEATURE_ON == FEATURE_VSIM)

typedef struct
{
    VOS_MSG_HEADER
    SI_PIH_CMD_CNF_TYPE_ENUM_UINT32       enMsgName;       /* 消息名 */
    VOS_UINT8                             ucLength;        /* VSIM APN内容长度 */
    VOS_UINT8                             aucRev[3];
    VOS_UINT8                             aucVsimApn[SI_PIH_VSIMAPN_MAX];  /* VSIM APN内容 */
}SI_PIH_VSIMAPN_IND_STRU;

#ifdef CONFIG_TZDRIVER
typedef struct
{
    unsigned int    time_type;  /*Timer Type*/
    unsigned int    time_id;    /*Timer ID*/
    unsigned int    rev1;
    unsigned int    rev2;
}TEEC_TIMER_PROPERTY_STRU;

extern int TC_NS_RegisterServiceCallbackFunc(char *uuid, void *func, void *private_data);

extern VOS_VOID SI_PIH_TEETimeOutCB (
    TEEC_TIMER_PROPERTY_STRU            *pstTimerData
);

#endif/*CONFIG_TZDRIVER*/
#endif/*(FEATURE_ON == FEATURE_VSIM)*/

/*****************************************************************************
  6 函数声明
*****************************************************************************/
#if (OSA_CPU_NRCPU != VOS_OSA_CPU)
#if  ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT))) || (defined(__PC_UT__))
extern VOS_UINT32 SI_PIH_GetReceiverPid(
    MN_CLIENT_ID_T                      ClientId,
    VOS_UINT32                          *pulReceiverPid);

extern VOS_UINT32 SI_PIH_FdnEnable (
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    VOS_UINT8                           *pPIN2);

extern VOS_UINT32 SI_PIH_FdnDisable(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    VOS_UINT8                           *pPIN2);

extern VOS_UINT32  SI_PIH_FdnBdnQuery(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_QUERY_TYPE_ENUM_UINT32       enQueryType);

extern VOS_UINT32 SI_PIH_GenericAccessReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_CSIM_COMMAND_STRU            *pstData);

extern VOS_UINT32 SI_PIH_IsdbAccessReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_ISDB_ACCESS_COMMAND_STRU    *pstData);

extern VOS_UINT32 SI_PIH_CchoSetReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_CCHO_COMMAND_STRU           *pstCchoCmd);

extern VOS_UINT32 SI_PIH_CchpSetReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_CCHP_COMMAND_STRU           *pstCchopCmd);

extern VOS_UINT32 SI_PIH_CchcSetReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    VOS_UINT32                          ulSessionID);

extern VOS_UINT32 SI_PIH_HvSstQuery(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

extern VOS_UINT32 SI_PIH_HvSstSet (
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_HVSST_SET_STRU              *pstHvSStSet);

extern VOS_UINT32 SI_PIH_CglaSetReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_CGLA_COMMAND_STRU           *pstData);

extern VOS_UINT32 SI_PIH_GetCardATRReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

extern VOS_UINT32 SI_PIH_CrsmSetReq(
    MN_CLIENT_ID_T                          ClientId,
    MN_OPERATION_ID_T                       OpId,
    SI_PIH_CRSM_STRU                       *pstCrsmPara);

extern VOS_UINT32 SI_PIH_CrlaSetReq(
    MN_CLIENT_ID_T                          ClientId,
    MN_OPERATION_ID_T                       OpId,
    SI_PIH_CRLA_STRU                       *pstCrlaPara);

extern VOS_UINT32 SI_PIH_CardSessionQuery(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

extern VOS_UINT32 SI_PIH_CimiSetReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId
);

extern VOS_UINT32 SI_PIH_CCimiSetReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId
);

extern VOS_UINT32 SI_PIH_SciCfgSet (
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_CARD_SLOT_ENUM_UINT32        enCard0Slot,
    SI_PIH_CARD_SLOT_ENUM_UINT32        enCard1Slot,
    SI_PIH_CARD_SLOT_ENUM_UINT32        enCard2Slot
);

extern VOS_UINT32 SI_PIH_SciCfgQuery (
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

extern VOS_UINT32 SI_PIH_BwtSet (
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    VOS_UINT16                          usProtectTime);

extern VOS_UINT32 SI_PIH_HvCheckCardQuery(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId
);

extern VOS_UINT32 SI_PIH_UiccAuthReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_UICCAUTH_STRU                *pstData);

extern VOS_UINT32 SI_PIH_AccessUICCFileReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_ACCESSFILE_STRU              *pstData);

extern VOS_UINT32 SI_PIH_CardTypeQuery(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

extern VOS_UINT32 SI_PIH_CardTypeExQuery(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

#if (FEATURE_ON == FEATURE_PHONE_SC)
extern VOS_UINT32 SI_PIH_GetSilentPinInfoReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    VOS_UINT8                          *pucPin);

extern VOS_UINT32 SI_PIH_SetSilentPinReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_CRYPTO_PIN_STRU             *pstCryptoPin);
#endif
extern VOS_UINT32 SI_PIH_CardVoltageQuery(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

extern VOS_UINT32 SI_PIH_PrivateCglaSetReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_CGLA_COMMAND_STRU           *pstData);

VOS_UINT32 SI_PIH_GetSecIccVsimVer(VOS_VOID);

VOS_UINT32 SI_PIH_EsimSwitchSet (
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_PIH_ESIMSWITCH_SET_STRU         *pstEsimSwitchSet);

VOS_UINT32 SI_PIH_EsimSwitchQuery(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);
#endif /*#if  ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT)))*/

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (defined(DMT)))

extern VOS_UINT32 WuepsPIHPidInit(enum VOS_INIT_PHASE_DEFINE InitPhrase);

extern VOS_UINT32 PIH_RegUsimCardStatusIndMsg(
    MODEM_ID_ENUM_UINT16                enModemId,
    VOS_UINT32                          ulRegPID
);

extern VOS_UINT32 PIH_DeregUsimCardStatusIndMsg(
    MODEM_ID_ENUM_UINT16                enModemId,
    VOS_UINT32                          ulRegPID
);

extern VOS_UINT32 PIH_RegCardRefreshIndMsg(
    MODEM_ID_ENUM_UINT16                enModemId,
    VOS_UINT32                          ulRegPID
);

extern VOS_UINT32 PIH_DeregCardRefreshIndMsg(
    MODEM_ID_ENUM_UINT16                enModemId,
    VOS_UINT32                          ulRegPID
);
#endif /*#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (defined(DMT)))*/
#endif /*#if (OSA_CPU_NRCPU != VOS_OSA_CPU)*/

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of SiAppPih.h */

