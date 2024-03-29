/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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




/*****************************************************************************
  1 Include HeadFile
*****************************************************************************/
#include "ImmInterface.h"
#include "NVIM_Interface.h"
#include "PsNdis.h"
#include "LUPQueue.h"
#include "Ipv4DhcpServer.h"
#include "PsCommonDef.h"
#include "msp_at.h"
#include "msp_diag.h"
#include "vos.h"
#include "IpNdServer.h"
#include "nv_stru_gucnas.h"
#include "acore_nv_stru_gucnas.h"
#include "NdisDrv.h"
#include "PsIfaceGlobalDef.h"
#include "mdrv.h"
#include "ps_tag.h"
#define THIS_MODU ps_ndis
/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
/*lint -e767*/
#define    THIS_FILE_ID          PS_FILE_ID_PSNDIS_C
/*lint +e767*/

/*****************************************************************************
  2 Declare the Global Variable
*****************************************************************************/
#define C2A_QUE_SIZE                512        /*暂定512*/
#define USB_DATAREQ_QUE_SIZE        512        /*暂定512*/

#define NDIS_PERIOD_ARP_TMRNAME     1
#define NDIS_ARP_REQ_TMRNAME        2

#define NDIS_ARP_FRAME_REV_OFFSET   (((VOS_UINT64)(&(((ETH_ARP_FRAME_STRU*)0)->aucRev[0]))) & 0xFFFFFFFF)



/*****************************************************************************
  3 function
*****************************************************************************/
/*来自ADS的数据，存放队列*/
LUP_QUEUE_STRU  *g_pstC2ACoreQue        = VOS_NULL_PTR;
VOS_VOID        *p_aC2AQueBuf[C2A_QUE_SIZE];

/*来自USB数据的存放队列*/
LUP_QUEUE_STRU  *g_pstUsbDataReqQue     = VOS_NULL_PTR;
VOS_VOID        *p_aUsbQueBuf[USB_DATAREQ_QUE_SIZE];

/*arp请求中间部分固定的值*/
VOS_UINT8       g_aucArpReqFixVal[ETH_ARP_FIXED_MSG_LEN] = {0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x01 };
/*arp响应中间部分固定的值*/
VOS_UINT8       g_aucArpRspFixVal[ETH_ARP_FIXED_MSG_LEN] = {0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x02 };
/*广播地址，全1*/
VOS_UINT8       g_aucBroadCastAddr[ETH_MAC_ADDR_LEN]   = {0xff,0xff,0xff,0xff,0xff,0xff };

VOS_UINT8       g_aucInvalidAddr[IPV4_ADDR_LEN] = {0};

/*ARP周期*/
VOS_UINT32      g_ulPeriodicArpCyc      = 3000;   /*周期性ARP发送周期*/


/*统计信息*/
NDIS_STAT_INFO_STRU        g_stNdisStatStru = {0};

NDIS_ENTITY_STRU           g_astNdisEntity[NAS_NDIS_MAX_ITEM] = {{0}};
NDIS_ENTITY_STRU          *g_pstNdisEntity = g_astNdisEntity;

VOS_UINT32                 g_ulNvMtu = 1500;              /*IPV6 MTU默认取值*/

VOS_UINT32 g_ulNdisLomSwitch = 0;
SPE_MAC_ETHER_HEADER_STRU g_stSpeMacHeader = {{0x58,0x02,0x03,0x04,0x05,0x06},{0x00,0x11,0x09,0x64,0x01,0x01},0x00000000};     /*mac地址初始化为固定值*/

/*****************************************************************************
  3 Function
*****************************************************************************/
/*声明*/
VOS_UINT32 Ndis_DlSpeSendNcm(NDIS_ENTITY_STRU *pstNdisEntity, ADS_PKT_TYPE_ENUM_UINT8 ucPktType, IMM_ZC_STRU *pstImmZc);

extern VOS_UINT32 DIPC_Pid_InitFunc( enum VOS_INIT_PHASE_DEFINE ip );
extern VOS_UINT32 DIPC_AtMsgProc( const MsgBlock *pMsgBlock );

extern VOS_UINT32 MUX_Pid_InitFunc( enum VOS_INIT_PHASE_DEFINE ip );
extern VOS_UINT32 MUX_AtMsgProc( const MsgBlock *pMsgBlock );

/* ****************************************************************************
 函 数 名  : Ndis_GetMacAddr
 功能描述  : 获取MAC地址
 输入参数  : None
 输出参数  : None
 返 回 值  : VOS_UINT8*
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年10月17日
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT8* Ndis_GetMacAddr(VOS_VOID)
{
    /*LTE协议栈MAC地址*/
    static VOS_UINT8 g_ucMacAddressPstable[] =
    {
        0x4c, 0x54, 0x99, 0x45, 0xe5, 0xd5
    };

    return g_ucMacAddressPstable;
}


/*****************************************************************************
 函 数 名  : Ndis_SndMsgToAt
 功能描述  : 发送Cnf消息到AT
 输入参数  :
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年3月16日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 Ndis_SndMsgToAt(const VOS_UINT8 *pucBuf,VOS_UINT16 usMsgLen,VOS_UINT32 ulMsgId)
{
    MsgBlock                     *pstMsgBlock   = VOS_NULL_PTR;
    MSG_HEADER_STRU              *pstMsgHeader  = VOS_NULL_PTR;


    /*lint -e516 -esym(516,*)*/
    pstMsgBlock = (MsgBlock*)PS_ALLOC_MSG(NDIS_TASK_PID, usMsgLen - VOS_MSG_HEAD_LENGTH);
    /*lint -e516 +esym(516,*)*/


    if (VOS_NULL_PTR == pstMsgBlock)
    {
        return PS_FAIL;
    }

    pstMsgHeader = (MSG_HEADER_STRU *)(VOS_VOID*)pstMsgBlock;

    NDIS_MEM_CPY_S(pstMsgBlock->aucValue, usMsgLen - VOS_MSG_HEAD_LENGTH, (pucBuf + VOS_MSG_HEAD_LENGTH), usMsgLen - VOS_MSG_HEAD_LENGTH);

    pstMsgHeader->ulSenderPid   = NDIS_TASK_PID;
    pstMsgHeader->ulReceiverPid = APP_AT_PID;
    pstMsgHeader->ulMsgName     = ulMsgId;


    if(VOS_OK != PS_SEND_MSG(NDIS_TASK_PID, pstMsgBlock))
    {
        /*异常打印*/
        return PS_FAIL;
    }

    return PS_SUCC;
}



/* ****************************************************************************
 函 数 名  : Ndis_DlAdsDataRcv
 功能描述  : NDIS_ADS下行数据接收回调函数
 输入参数  : VOS_VOID *pBuf
             VOS_UINT32 ulLen
 输出参数  : 无
 返 回 值  : VOS_INT
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年12月9日
    修改内容   : 新生成函数

  2.日    期   : 2013年1月16日
    修改内容   : DSDA特性开发，入参修改为扩展承载ID

*****************************************************************************/
VOS_INT Ndis_DlAdsDataRcv(VOS_UINT8 ucExRabId, IMM_ZC_STRU *pData, ADS_PKT_TYPE_ENUM_UINT8 enPktType, VOS_UINT32 ulExParam)
{
    if (VOS_NULL_PTR == pData)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlAdsDataRcv, recv NULL PTR!");
        return PS_FAIL;
    }

    if ((PS_SUCC != Ndis_ChkRabIdValid(ucExRabId))
             || (ADS_PKT_TYPE_BUTT <= enPktType))
    {
        /*lint -e522*/
        IMM_ZcFree(pData);
        /*lint +e522*/
        NDIS_ERROR_LOG2(NDIS_TASK_PID, "Ndis_DlAdsDataRcv, recv RabId or PktType fail!", ucExRabId, enPktType);
        NDIS_STAT_DL_DISCARD_ADSPKT(1);
        return PS_FAIL;
    }

    /*增加从ADS接收到的数据包个数统计*/
    NDIS_STAT_DL_RECV_ADSPKT_SUCC(1);

    Ndis_LomTraceRcvDlData();
    if (PS_SUCC != Ndis_DlSendNcm(ucExRabId, enPktType, pData))
    {
        /*lint -e522*/
        IMM_ZcFree(pData);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlAdsDataRcv, Ndis_DlSendNcm fail!");
        return PS_FAIL;
    }

    return PS_SUCC;
}

/* ****************************************************************************
 函 数 名  : Ndis_DlAdsDataRcvV2
 功能描述  : NDIS_ADS下行数据接收回调函数
 输入参数  : VOS_VOID *pBuf
             VOS_UINT32 ulLen
 输出参数  : 无
 返 回 值  : VOS_INT
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年08月16日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_INT Ndis_DlAdsDataRcvV2(unsigned long ulUserData, IMM_ZC_STRU *pData)
{
    ADS_PKT_TYPE_ENUM_UINT8         enPktType;

    if (VOS_NULL_PTR == pData)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlAdsDataRcvV2, recv NULL PTR!");
        return PS_FAIL;
    }

    if ((PS_SUCC != Ndis_ChkRabIdValid((VOS_UINT8)ulUserData)))
    {
        /*lint -e522*/
        IMM_ZcFree(pData);
        /*lint +e522*/
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_DlAdsDataRcvV2, recv RabId or PktType fail!", ulUserData);
        NDIS_STAT_DL_DISCARD_ADSPKT(1);
        return PS_FAIL;
    }

    /*增加从ADS接收到的数据包个数统计*/
    NDIS_STAT_DL_RECV_ADSPKT_SUCC(1);

    Ndis_LomTraceRcvDlData();
    if (IP_PAYLOAD == IMM_ZcGetProtocol(pData))
    {
        enPktType = ADS_PKT_TYPE_IPV4;
    }
    else
    {
        enPktType = ADS_PKT_TYPE_IPV6;
    }
    if (PS_SUCC != Ndis_DlSendNcm((VOS_UINT8)ulUserData, enPktType, pData))
    {
        /*lint -e522*/
        IMM_ZcFree(pData);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlAdsDataRcvV2, Ndis_DlSendNcm fail!");
        return PS_FAIL;
    }

    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : AppNdis_UsbReadCb
 功能描述  : App核间USB通道
 输入参数  : VOS_VOID *pBuf
             VOS_UINT32 ulLen
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年1月31日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AppNdis_UsbReadCb(UDI_HANDLE ulhandle, VOS_VOID *pPktNode)
{
    IMM_ZC_STRU            *pstImmZc = (IMM_ZC_STRU*)pPktNode;    /*目前ImmZc和sk_buff完全一致，直接强转*/

    VOS_UINT8                   ucExRabId;
    VOS_UINT16                  usFrameType;
    ETHFRM_IPV4_PKT_STRU       *pstIpPacket;
    VOS_UINT8                  *pucData;
    VOS_UINT32                  ulDataLen;

    if (VOS_NULL_PTR == pstImmZc)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "AppNdis_UsbReadCb read NULL PTR!");
        return PS_FAIL;
    }

    pucData = IMM_ZcGetDataPtr(pstImmZc);
    if (VOS_NULL_PTR == pucData)
    {
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, IMM_ZcGetDataPtr fail!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return PS_FAIL;
    }

    /*长度异常判断*/
    ulDataLen = IMM_ZcGetUsedLen(pstImmZc);
    if (ulDataLen < ETH_MAC_HEADER_LEN)
    {
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ulDataLen less than ETH_MAC_HEADER_LEN!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return PS_FAIL;
    }

    pstIpPacket = (ETHFRM_IPV4_PKT_STRU  *)(VOS_VOID*)pucData;
    usFrameType = pstIpPacket->usFrameType;

    /*这里获取的是扩展RabId*/
    ucExRabId = Ndis_FindRabIdByHandle(ulhandle, usFrameType);

    if (PS_SUCC != Ndis_ChkRabIdValid(ucExRabId))
    {
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, Ndis_ChkRabIdValid fail!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return PS_FAIL;
    }

    NDIS_STAT_UL_RECV_USBPKT_SUCC(1);

    Ndis_LomTraceRcvUlData();
    Ndis_UlNcmFrmProc(ucExRabId, pstImmZc);

    return PS_SUCC;
}
/*****************************************************************************
 函 数 名  : AppNdis_SpeReadCb
 功能描述  : App核间SPE通道
 输入参数  : VOS_VOID *pBuf
             VOS_UINT32 ulLen
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月31日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AppNdis_SpeReadCb(VOS_INT32 lSpePort, VOS_VOID *pPktNode)
{
    IMM_ZC_STRU            *pstImmZc = (IMM_ZC_STRU*)pPktNode;    /*目前ImmZc和sk_buff完全一致，直接强转*/

    VOS_UINT16                  usFrameType;
    ETHFRM_IPV4_PKT_STRU       *pstIpPacket;
    VOS_UINT8                  *pucData;
    VOS_UINT8                   ucExRabId;
    VOS_UINT32                  ulDataLen;

    if (VOS_NULL_PTR == pstImmZc)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "AppNdis_UsbReadCb read NULL PTR!");
        return PS_FAIL;
    }

    pucData = IMM_ZcGetDataPtr(pstImmZc);
    if (VOS_NULL_PTR == pucData)
    {
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, IMM_ZcGetDataPtr fail!");
        return PS_FAIL;
    }

    /*长度异常判断*/
    ulDataLen = IMM_ZcGetUsedLen(pstImmZc);
    if (ulDataLen < ETH_MAC_HEADER_LEN)
    {
         /*lint -e522*/
         IMM_ZcFree(pstImmZc);
         /*lint +e522*/
         NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ulDataLen less than ETH_MAC_HEADER_LEN!");
         return PS_FAIL;
    }

    pstIpPacket = (ETHFRM_IPV4_PKT_STRU  *)(VOS_VOID*)pucData;
    usFrameType = pstIpPacket->usFrameType;

    /*这里获取的是扩展RabId*/
    ucExRabId = Ndis_FindRabIdBySpePort(lSpePort, usFrameType);

    if (PS_SUCC != Ndis_ChkRabIdValid(ucExRabId))
    {
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, Ndis_ChkRabIdValid fail!");
        return PS_FAIL;
    }

    Ndis_LomTraceRcvUlData();
    Ndis_UlNcmFrmProc(ucExRabId, pstImmZc);

    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : Ndis_NvItemInit
 功能描述  : A核NDIS读取NV项的初始化函数
 输入参数  : VOS_VOID
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年4月24日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 Ndis_NvItemInit(VOS_VOID)
{
    VOS_UINT32     ulRtn;
    VOS_UINT32     ulDhcpLeaseHour;
    VOS_UINT32     ulIpv6Mtu;
    NDIS_NV_DHCP_LEASE_HOUR_STRU        stNdisDhcpLeaseHour;
    TAF_NDIS_NV_IPV6_ROUTER_MTU_STRU    stNdisIPv6Mtu;

    /*
        DHCP Lease Time, 设定范围为[1, 8784]小时
        Vodafone    24小时
        其他        72小时

        时间经过DHCP Lease Time一半时，PC会主动发起续租，
        如果DHCP租约超期，则从PC通过NDIS通道发往单板的数据会出现目的不可达错误
        目前测试，当DHCP Lease Time小于等于4S时，对于数传影响较大，所以定义最小租约为1小时
        目前没有遇到超过8天的DHCP Lease Time，暂定上限为8784小时(366天)
        */

    /* 从NV读取流控配置信息 */
    /*lint -e718*/
    /*lint -e732*/
    /*lint -e746*/
    ulRtn = Ndis_NvimItem_Read(en_NV_Item_NDIS_DHCP_DEF_LEASE_TIME,\
                                    &stNdisDhcpLeaseHour,\
                                    sizeof(NDIS_NV_DHCP_LEASE_HOUR_STRU));
    if (PS_SUCC != ulRtn)
    {
        PS_PRINTF_ERR("Ndis_NvItemInit, Fail to read NV DHCP_LEASE_TIME: %d\n", ulRtn);
        stNdisDhcpLeaseHour.ulDhcpLeaseHour = TTF_NDIS_DHCP_DEFAULT_LEASE_HOUR;
    }

    /* NV值合法性判断 */
    ulDhcpLeaseHour = stNdisDhcpLeaseHour.ulDhcpLeaseHour;
    if((0 < ulDhcpLeaseHour) && (ulDhcpLeaseHour <= TTF_NDIS_DHCP_MAX_LEASE_HOUR))
    {
        /* hour -> second */
        g_ulLeaseTime = ulDhcpLeaseHour * 3600;
    }

    /* 从NV读取IPV6 MTU信息 */
    ulRtn = Ndis_NvimItem_Read(en_NV_Item_IPV6_ROUTER_MTU,\
                                    &stNdisIPv6Mtu,\
                                    sizeof(TAF_NDIS_NV_IPV6_ROUTER_MTU_STRU));
    /*lint +e746*/
    /*lint +e732*/
    /*lint +e718*/

    if (PS_SUCC != ulRtn)
    {
        PS_PRINTF_ERR( "Ndis_NvItemInit, Fail to read NV IPV6_MTU: %d\n!", ulRtn);
        stNdisIPv6Mtu.ulIpv6RouterMtu = TTF_NDIS_IPV6_MTU_DEFAULT;
    }

    /* NV值合法性判断 */
    ulIpv6Mtu = stNdisIPv6Mtu.ulIpv6RouterMtu;
    if (0 == ulIpv6Mtu)
    {
        g_ulNvMtu = TTF_NDIS_IPV6_MTU_DEFAULT;
    }
    else
    {
        g_ulNvMtu = ulIpv6Mtu;
    }

    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : Ndis_Init
 功能描述  : APP核NDIS功能的初始化函数
 输入参数  : VOS_VOID
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年2月10日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 Ndis_Init( VOS_VOID )
{
    VOS_UINT32     ulLoop;
    VOS_UINT8     *pucMacAddr = VOS_NULL_PTR;
    VOS_UINT16     usPayLoad;
    NDIS_ARP_PERIOD_TIMER_STRU    *pstArpPeriodTimer = VOS_NULL_PTR;

    /*lint -e746*/
    pucMacAddr = (VOS_UINT8 *)Ndis_GetMacAddr();                                 /*获得单板MAC地址*/
    /*lint -e746*/

    if (VOS_NULL_PTR == pucMacAddr)
    {
        PS_PRINTF_ERR("Ndis_Init, Ndis_GetMacAddr Fail!\n");
        return PS_FAIL;
    }

    usPayLoad = IP_PAYLOAD;
    for(ulLoop=0; ulLoop<NAS_NDIS_MAX_ITEM; ulLoop++)
    {
        g_astNdisEntity[ulLoop].ucRabType  =  NDIS_RAB_NULL;
        g_astNdisEntity[ulLoop].ulHandle   =  NDIS_INVALID_HANDLE;
        g_astNdisEntity[ulLoop].ucRabId    =  NDIS_INVALID_RABID;
        g_astNdisEntity[ulLoop].enUsed     =  PS_FALSE;
        g_astNdisEntity[ulLoop].lSpePort   =  NDIS_INVALID_SPEPORT;
        g_astNdisEntity[ulLoop].ulSpeIpfFlag = PS_FALSE;
        NDIS_MEM_CPY_S(g_astNdisEntity[ulLoop].stIpV4Info.aucMacFrmHdr+ETH_MAC_ADDR_LEN,ETH_MAC_ADDR_LEN, pucMacAddr,ETH_MAC_ADDR_LEN);
        NDIS_MEM_CPY_S(g_astNdisEntity[ulLoop].stIpV4Info.aucMacFrmHdr+(2*ETH_MAC_ADDR_LEN),2, (VOS_UINT8*)(&usPayLoad),2);

        /*周期性ARP定时器初始化*/
        pstArpPeriodTimer = &(g_astNdisEntity[ulLoop].stIpV4Info.stArpPeriodTimer);
        pstArpPeriodTimer->hTm          = VOS_NULL_PTR;
        pstArpPeriodTimer->ulName       = NDIS_PERIOD_ARP_TMRNAME;
        pstArpPeriodTimer->ulTimerValue = g_ulPeriodicArpCyc;
    }

    if (PS_SUCC != Ndis_NvItemInit())             /*NV项初始化*/
    {
        PS_PRINTF_ERR("Ndis_Init, Ndis_NvItemInit Fail!\n");
        return PS_FAIL;
    }

    return PS_SUCC;
}
/*****************************************************************************
 函 数 名  : Ndis_DlSendNcm
 功能描述  : 下行方向的NCM数据的发送
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年2月11日
    修改内容   : 新生成函数
     2.日    期   : 2015年2月11日
    修改内容   : SPE
*****************************************************************************/
VOS_UINT32 Ndis_DlSendNcm(VOS_UINT8 ucExRabId, ADS_PKT_TYPE_ENUM_UINT8 ucPktType, IMM_ZC_STRU *pstImmZc)
{
    VOS_UINT32                     ulResult;
    NDIS_ENTITY_STRU              *pstNdisEntity;

        /*使用ExRabId获取NDIS实体*/
    pstNdisEntity = NDIS_GetEntityByRabId(ucExRabId);
    if(IP_NULL_PTR == pstNdisEntity)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlSendNcm, NDIS_GetEntityByRabId fail!");
        return PS_FAIL;
    }

    ulResult = Ndis_DlUsbSendNcm(ucExRabId, ucPktType, pstImmZc);
    return ulResult;
}


VOS_UINT32 Ndis_DlUsbSendNcm(VOS_UINT8 ucExRabId, ADS_PKT_TYPE_ENUM_UINT8 ucPktType, IMM_ZC_STRU *pstImmZc)
{
    VOS_UINT8                      ucIndex;
    VOS_UINT8                     *pucAddData = VOS_NULL_PTR;
    UDI_HANDLE                     ulHandle;
    VOS_UINT32                     ulSize;
    NDIS_ENTITY_STRU              *pstNdisEntity;
    VOS_UINT8                      enTeAddrState;
    VOS_UINT16                     usApp = 0;
    VOS_UINT16                     usTmpApp = 0;

    /*使用ExRabId获取NDIS实体*/
    pstNdisEntity = NDIS_GetEntityByRabId(ucExRabId);
    if(IP_NULL_PTR == pstNdisEntity)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlSendNcm, NDIS_GetEntityByRabId fail!");
        return PS_FAIL;
    }

    ulHandle      = pstNdisEntity->ulHandle;

    /*填充MAC帧头，调用ImmZc接口将MAC帧头填入ImmZc中*/
    if ((ADS_PKT_TYPE_IPV4 == ucPktType)     /*包类型枚举*/
           && (NDIS_ENTITY_IPV4 == (pstNdisEntity->ucRabType & NDIS_ENTITY_IPV4)))
    {
        pucAddData = pstNdisEntity->stIpV4Info.aucMacFrmHdr;
    }
    else if ((ADS_PKT_TYPE_IPV6 == ucPktType)
           && (NDIS_ENTITY_IPV6 == (pstNdisEntity->ucRabType & NDIS_ENTITY_IPV6)))
    {
        ucIndex = (VOS_UINT8)IP_NDSERVER_GET_ADDR_INFO_INDEX(ucExRabId);
        if (ucIndex >= IP_NDSERVER_ADDRINFO_MAX_NUM)
        {
            NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlSendNcm, IP_NDSERVER_GET_ADDR_INFO_INDEX fail!");
            return PS_FAIL;
        }

        pucAddData = NdSer_GetMacFrm(ucIndex, &enTeAddrState);
        if (VOS_NULL_PTR == pucAddData)
        {
            NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlSendNcm, NdSer_GetMacFrm fail!");
            NDIS_STAT_DL_GET_IPV6MAC_FAIL(1);
            return PS_FAIL;
        }

        if (IP_NDSERVER_TE_ADDR_REACHABLE != enTeAddrState)
        {
            /*将ucExRabId和数据包类型放入ImmZc的私有数据域中*/
            usTmpApp = (ucPktType & 0xFF);
            usApp    = ((VOS_UINT16)(usTmpApp << 8)) | (ucExRabId);
            IMM_ZcSetUserApp(pstImmZc, usApp);

            NdSer_MacAddrInvalidProc(pstImmZc, ucIndex);
            return PS_SUCC;
        }
    }
    else   /*数据包类型与承载支持类型不一致*/
    {
        NDIS_ERROR_LOG2(NDIS_TASK_PID, "Ndis_DlSendNcm, Rab is different from PktType!", pstNdisEntity->ucRabType, ucPktType);
        NDIS_STAT_DL_PKT_DIFF_RAB_NUM(1);
        return PS_FAIL;
    }

    if (VOS_OK != IMM_ZcAddMacHead(pstImmZc, pucAddData))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlSendNcm, IMM_ZcAddMacHead fail!");
        NDIS_STAT_DL_ADDMACFRM_FAIL(1);
        return PS_FAIL;
    }

    ulSize = IMM_ZcGetUsedLen(pstImmZc);         /*加上以太网帧头的长度*/

    /*lint -e718*/
    if (0 != NDIS_UDI_WRITE(ulHandle, pstImmZc, ulSize))
    {
        NDIS_STAT_DL_SEND_USBPKT_FAIL(1);
        return PS_FAIL;
    }
    /*lint +e718*/

    NDIS_STAT_DL_SEND_USBPKT_SUCC(1);
    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : Ndis_ProcTmrMsg
 功能描述  : 处理TmerMsg
 输入参数  : MsgBlock *pRcvMsg
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

  1.日    期   : 2012年4月28日
    修改内容   : 新生成函数
*****************************************************************************/
VOS_VOID Ndis_ProcARPTimerExp(VOS_VOID)
{
    VOS_UINT32                   ulLoop;
    NDIS_ENTITY_STRU            *pstNdisEntity      = VOS_NULL_PTR;
    NDIS_IPV4_INFO_STRU         *pstIpV4Info        = VOS_NULL_PTR;
    NDIS_ARP_PERIOD_TIMER_STRU  *pstArpPeriodTimer  = VOS_NULL_PTR;

    for(ulLoop=0; ulLoop<NAS_NDIS_MAX_ITEM; ulLoop++)
    {
        pstNdisEntity = &g_astNdisEntity[ulLoop];
        if (NDIS_ENTITY_IPV4 == (pstNdisEntity->ucRabType & NDIS_ENTITY_IPV4))
        {
            pstIpV4Info = &(pstNdisEntity->stIpV4Info);
            pstArpPeriodTimer = &(pstIpV4Info->stArpPeriodTimer);

            if (PS_TRUE == pstIpV4Info->ulArpInitFlg)
            {
                Ndis_StopARPTimer(pstArpPeriodTimer);
            }
            else
            {
                (VOS_VOID)Ndis_SendRequestArp(&(pstNdisEntity->stIpV4Info), pstNdisEntity->ucRabId);
            }
        }
    }

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_ProcTmrMsg
 功能描述  : 处理TmerMsg
 输入参数  : MsgBlock *pRcvMsg
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史  :
  1.日    期   : 2012年4月28日
    修改内容   : 新生成函数
*****************************************************************************/
VOS_VOID Ndis_ProcTmrMsg(const REL_TIMER_MSG *pRcvMsg)
{

    if ( sizeof(REL_TIMER_MSG) - VOS_MSG_HEAD_LENGTH > pRcvMsg->ulLength )
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_ProcTmrMsg, input msg length less than struc", pRcvMsg->ulLength);
        return;
    }

    switch(pRcvMsg->ulName)
    {
        case NDIS_PERIOD_ARP_TMRNAME:
            Ndis_ProcARPTimerExp();
            break;
        default:
            NDIS_INFO_LOG1(NDIS_TASK_PID, "Ndis_ProcTmrMsg, Recv other Timer", pRcvMsg->ulName);
            break;
    }

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_DHCPPkt_Proc
 功能描述  : DHCP处理
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年2月11日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_DHCPPkt_Proc(VOS_VOID *pRcvMsg)
{
    ADS_NDIS_DATA_IND_STRU  *pstAdsNdisMsg  = (ADS_NDIS_DATA_IND_STRU*)pRcvMsg;
    VOS_UINT8               *pucData        = VOS_NULL_PTR;
    NDIS_ENTITY_STRU        *pstNdisEntity  = VOS_NULL_PTR;
    VOS_UINT8                ucExRabId;
    VOS_UINT32               ulPktMemLen;

    pucData  = IMM_ZcGetDataPtr(pstAdsNdisMsg->pstData);
    if (VOS_NULL_PTR == pucData)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DHCPPkt_Proc, IMM_ZcGetDataPtr fail!");
        return;
    }
    ulPktMemLen = IMM_ZcGetUsedLen(pstAdsNdisMsg->pstData);

    ucExRabId = NDIS_FORM_EXBID(pstAdsNdisMsg->enModemId, pstAdsNdisMsg->ucRabId);
    if (PS_SUCC != Ndis_ChkRabIdValid(ucExRabId))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DHCPPkt_Proc, Ndis_ChkRabIdValid fail!");
        return;
    }

    pstNdisEntity = NDIS_GetEntityByRabId(ucExRabId);
    if(VOS_NULL_PTR == pstNdisEntity)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DHCPPkt_Proc, NDIS_GetEntityByRabId fail!");
        return;
    }

    if (NDIS_ENTITY_IPV4 != (pstNdisEntity->ucRabType & NDIS_ENTITY_IPV4))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DHCPPkt_Proc, Rab not support IPV4!");
        return;
    }

    /*DHCP处理*/
    NDIS_STAT_UL_RECV_DHCPPKT(1);
    IPV4_DHCP_ProcDhcpPkt(pucData, ucExRabId,ulPktMemLen);

    return;
}
/*****************************************************************************
 函 数 名  : Ndis_FindRabIdBySpePort
 功能描述  : 根据SpePort查找RabId
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年12月9日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT8 Ndis_FindRabIdBySpePort(VOS_INT32 lPort, VOS_UINT16 usFrameType)
{
    VOS_UINT32                      ulLoop;
    VOS_UINT8                       ucTmpRabType;
    NDIS_ENTITY_STRU               *pstNdisEntity = VOS_NULL_PTR;

    switch(usFrameType)
    {
        case ARP_PAYLOAD:
            ucTmpRabType = NDIS_ENTITY_IPV4;    /*ARP包也经过SPE*/
            break;
        case IP_PAYLOAD:
            ucTmpRabType = NDIS_ENTITY_IPV4;
            break;
        case IPV6_PAYLOAD:
            ucTmpRabType = NDIS_ENTITY_IPV6;
            break;
        default:
            NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_FindRabIdByHandle, FrameType Error!", usFrameType);
            return NDIS_INVALID_RABID;
    }
    for(ulLoop=0; ulLoop<NAS_NDIS_MAX_ITEM; ulLoop++)
    {
         pstNdisEntity = &g_astNdisEntity[ulLoop];

         if ((lPort == pstNdisEntity->lSpePort)
                && (ucTmpRabType == (pstNdisEntity->ucRabType & ucTmpRabType)))   /*数据包类型与承载类型一致*/
         {
             return pstNdisEntity->ucRabId;
         }
    }

    return NDIS_INVALID_RABID;

}

/*****************************************************************************
 函 数 名  : Ndis_FindRabIdByHandle
 功能描述  : 根据Handle查找RabId
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年12月9日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT8 Ndis_FindRabIdByHandle(UDI_HANDLE ulhandle, VOS_UINT16 usFrameType)
{
    VOS_UINT32                      ulLoop;
    VOS_UINT8                       ucTmpRabType;
    NDIS_ENTITY_STRU               *pstNdisEntity = VOS_NULL_PTR;

    if ((ARP_PAYLOAD == usFrameType) || (IP_PAYLOAD == usFrameType))
    {
        ucTmpRabType = NDIS_ENTITY_IPV4;
    }
    else if (IPV6_PAYLOAD == usFrameType)
    {
        ucTmpRabType = NDIS_ENTITY_IPV6;
    }
    else
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_FindRabIdByHandle, FrameType Error!", usFrameType);
        return NDIS_INVALID_RABID;
    }

    for(ulLoop=0; ulLoop<NAS_NDIS_MAX_ITEM; ulLoop++)
    {
         pstNdisEntity = &g_astNdisEntity[ulLoop];

         if ((ulhandle == pstNdisEntity->ulHandle)
              && (ucTmpRabType == (pstNdisEntity->ucRabType & ucTmpRabType)))   /*数据包类型与承载类型一致*/
         {
             return pstNdisEntity->ucRabId;
         }
    }

    return NDIS_INVALID_RABID;

}

/*****************************************************************************
 函 数 名  : Ndis_UlNcmFrmProc
 功能描述  : 对上行NCM帧的处理
 输入参数  : UDI_HANDLE ulhandle, IMM_ZC_STRU *pstImmZc
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年2月11日
    修改内容   : 新生成函数

  2.日    期   : 2013年1月16日
    修改内容   : DSDA开发

*****************************************************************************/
VOS_VOID Ndis_UlNcmFrmProc(VOS_UINT8 ucExRabId, IMM_ZC_STRU *pstImmZc)
{
    VOS_UINT16                  usFrameType;
    ETHFRM_IPV4_PKT_STRU       *pstIpPacket = VOS_NULL_PTR;
    ETH_IPFIXHDR_STRU          *pstIpFixHdr = VOS_NULL_PTR;
    VOS_UINT8                  *pucData = VOS_NULL_PTR;
    VOS_UINT32                  ulDataLen;
    VOS_UINT32                  ulIpLen;
    VOS_UINT32                  ulIpv4Flag = 0;
    VOS_UINT32                  ulCacheLen;

    pucData = IMM_ZcGetDataPtr(pstImmZc);
    if (VOS_NULL_PTR == pucData)
    {
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, IMM_ZcGetDataPtr fail!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return;
    }

    pstIpPacket = (ETHFRM_IPV4_PKT_STRU  *)(VOS_VOID*)pucData;
    usFrameType = pstIpPacket->usFrameType;

    /*ARP处理*/
    if(ARP_PAYLOAD == usFrameType)
    {
        /*长度异常判断*/
        ulDataLen = IMM_ZcGetUsedLen(pstImmZc);
        /*lint -e413*/
        if (ulDataLen < NDIS_ARP_FRAME_REV_OFFSET)
        {
            /*lint -e522*/
            IMM_ZcFree(pstImmZc);
            /*lint +e522*/
            NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ulDataLen less than NDIS_ARP_FRAME_REV_OFFSET!");
            NDIS_STAT_UL_DISCARD_USBPKT(1);
            return;
        }

        /*ARP处理函数入参中增加RabId，后续以RabId作为Ndis实体遍历索引*/
        (VOS_VOID)Ndis_ProcArpMsg((ETH_ARP_FRAME_STRU*)(VOS_VOID*)pstIpPacket, ucExRabId);/*lint !e527*/

        /*处理完ARP后调用Imm_ZcFree释放ImmZc*/
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/

        return;
    }
     /*IPV6超长包处理*/
    if (IPV6_PAYLOAD == usFrameType)
    {
        ulDataLen = IMM_ZcGetUsedLen(pstImmZc);

        if ((ulDataLen - ETH_MAC_HEADER_LEN) > g_ulNvMtu)
        {
            NDIS_SPE_MEM_UNMAP(pstImmZc, ulDataLen);

            IP_NDSERVER_ProcTooBigPkt(ucExRabId,
                                      ((VOS_UINT8*)pstIpPacket + ETH_MAC_HEADER_LEN),
                                      g_ulNvMtu);

            NDIS_SPE_MEM_MAP(pstImmZc, ulDataLen);

            /*调用Imm_ZcFree释放ImmZc*/
            /*lint -e522*/
            IMM_ZcFree(pstImmZc);
            /*lint +e522*/

            return;
        }
    }

    if (IP_PAYLOAD == usFrameType)
    {
        ulIpv4Flag = 1;
        ulDataLen  = IMM_ZcGetUsedLen(pstImmZc);
        ulCacheLen = (ulDataLen < NDIS_SPE_CACHE_HDR_SIZE) ?
                        ulDataLen : NDIS_SPE_CACHE_HDR_SIZE;
    }

    /*经MAC层过滤后剩余的IP包发送，去掉MAC帧头后递交ADS*/
    if (VOS_OK != IMM_ZcRemoveMacHead(pstImmZc))
    {
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, IMM_ZcRemoveMacHead fail!");
        return;
    }

    /*检查IPV4包长度和实际Skb长度，如果不一致，则修改Skb长度为实际IP包长度*/
    if (1 == ulIpv4Flag)
    {
        pucData = IMM_ZcGetDataPtr(pstImmZc);
        if (VOS_NULL_PTR == pucData)
        {
            /*lint -e522*/
            IMM_ZcFree(pstImmZc);
            /*lint +e522*/
            return;
        }

        NDIS_SPE_MEM_UNMAP(pstImmZc, ulCacheLen);

        /*长度异常判断*/
        /*lint -e644*/
        if (ulDataLen < sizeof(ETH_IPFIXHDR_STRU))
        {
            /*lint -e522*/
            IMM_ZcFree(pstImmZc);
            /*lint +e522*/
            NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ulDataLen less than size of ETH_IPFIXHDR_STRU!");
            NDIS_STAT_UL_DISCARD_USBPKT(1);
            return;
        }
        /*lint +e644*/
        pstIpFixHdr = (ETH_IPFIXHDR_STRU *)((VOS_VOID*)pucData);
        ulIpLen = IP_NTOHS(pstIpFixHdr->usTotalLen);
        if (ulIpLen < pstImmZc->len)
        {
            pstImmZc->tail -= (pstImmZc->len - ulIpLen);
            pstImmZc->len  = ulIpLen;
        }

        NDIS_SPE_MEM_MAP(pstImmZc, ulCacheLen);
    }

    /*NR版本上暂不编译该部分，待NR Ndis迭代开始后再调整该部分*/
    if (VOS_OK != ADS_UL_SendPacket(pstImmZc, ucExRabId))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ADS_UL_SendPacket fail!");
        return;
    }

    NDIS_STAT_UL_SEND_ADSPKT(1);

    return;
}

/*****************************************************************************
 函 数 名  : APP_Ndis_DLPid_InitFunc
 功能描述  : APP NDIS下行PID初始化函数
 输入参数  : enum VOS_INIT_PHASE_DEFINE ePhase
 输出参数  : 无
 返 回 值  : extern VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年2月15日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32  APP_Ndis_Pid_InitFunc( enum VOS_INIT_PHASE_DEFINE ePhase)
{
    VOS_UINT32  ulResult = PS_FAIL;

    switch( ePhase )
    {
        case   VOS_IP_LOAD_CONFIG:

            ulResult = Ndis_Init();

            if (PS_SUCC != ulResult)
            {
                PS_PRINTF_ERR("APP_Ndis_Pid_InitFunc, Ndis_Init fail!\n");
                return VOS_ERR;
            }
            break;
        case VOS_IP_FARMALLOC:
        case VOS_IP_INITIAL:
        case VOS_IP_ENROLLMENT:
        case VOS_IP_LOAD_DATA:
        case VOS_IP_FETCH_DATA:
        case VOS_IP_STARTUP:
        case VOS_IP_RIVAL:
        case VOS_IP_KICKOFF:
        case VOS_IP_STANDBY:
        case VOS_IP_BROADCAST_STATE:
        case VOS_IP_RESTART:
        case VOS_IP_BUTT:
            break;
        default:
            break;
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : Ndis_SendMacFrm
 功能描述  : 发送以太网帧接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年2月14日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 Ndis_SendMacFrm(const VOS_UINT8  *pucBuf, VOS_UINT32 ulLen, VOS_UINT8 ucExRabId)
{
    IMM_ZC_STRU      *pstImmZc = VOS_NULL_PTR;
    VOS_INT32         lRtn;
    UDI_HANDLE        ulHandle;
    VOS_UINT8        *ucPdata = VOS_NULL_PTR;
    NDIS_ENTITY_STRU *pstNdisEntity = VOS_NULL_PTR;

    if (VOS_NULL_PTR == pucBuf)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_SendMacFrm, pucBuf is NULL!");
        return PS_FAIL;
    }

    pstImmZc = IMM_ZcStaticAlloc(ulLen);
    if (VOS_NULL_PTR == pstImmZc)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_SendMacFrm, IMM_ZcStaticAlloc fail!");
        return PS_FAIL;
    }

    ucPdata = (VOS_UINT8*)IMM_ZcPut(pstImmZc, ulLen);
    if (VOS_NULL_PTR == ucPdata)
    {
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_SendMacFrm, IMM_ZcPut fail!");
        return PS_FAIL;
    }

    NDIS_MEM_CPY_S(ucPdata,ulLen,pucBuf,ulLen);

    pstNdisEntity = NDIS_GetEntityByRabId(ucExRabId);
    if (VOS_NULL_PTR == pstNdisEntity)
    {
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_SendMacFrm, NDIS_GetEntityByRabId fail!");
        return PS_FAIL;
    }

    ulHandle      = pstNdisEntity->ulHandle;

    /*数据发送*/
    lRtn = NDIS_UDI_WRITE(ulHandle, pstImmZc, ulLen);

    if (0 != lRtn)
    {
        /*lint -e522*/
        IMM_ZcFree(pstImmZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_SendMacFrm, udi_write fail!");
        NDIS_STAT_DL_SEND_ARPDHCPPKT_FAIL(1);
        return PS_FAIL;
    }
    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : Ndis_ProcReqArp
 功能描述  : 处理ARP Request 帧
 输入参数  :

 输出参数  : 无
 返 回 值  : 成功返回PS_SUCC;
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年2月11日
    修改内容   : 新生成函数
  2.日    期   : 2013年1月22日
  修改内容   : DSDA

*****************************************************************************/

VOS_UINT32 Ndis_ProcReqArp(ETH_ARP_FRAME_STRU *pstReqArp, VOS_UINT8 ucRabId)
{
    VOS_UINT32                  ulTgtIpAddr  = pstReqArp->unTargetIP.ulIpAddr;
    NDIS_ENTITY_STRU           *pstNdisEntity = VOS_NULL_PTR;
    NDIS_IPV4_INFO_STRU        *pstArpV4Info  = VOS_NULL_PTR;

    pstNdisEntity = NDIS_GetEntityByRabId(ucRabId);
    if(VOS_NULL_PTR == pstNdisEntity)
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_ProcReqArp,  NDIS_GetEntityByRabId Error!", ucRabId);
        NDIS_STAT_PROC_ARP_FAIL(1);
        return PS_FAIL;
    }

    pstArpV4Info  = &pstNdisEntity->stIpV4Info;

    if ((0 != pstReqArp->unSenderIP.ulIpAddr)     /*兼容MAC OS 免费ARP类型,其Sender IP为0*/
           && (pstArpV4Info->unUeIpInfo.ulIpAddr != pstReqArp->unSenderIP.ulIpAddr))
    {
        /*源UE IP与网侧配置不符，这种情况不处理*/
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_ProcReqArp,  SenderIP Error!", pstReqArp->unSenderIP.ulIpAddr);
        NDIS_STAT_PROC_ARP_FAIL(1);
        return PS_SUCC;
    }

    /*更新PC MAC地址*/
    NDIS_MEM_CPY_S(pstArpV4Info->aucUeMacAddr,ETH_MAC_ADDR_LEN,pstReqArp->aucSenderAddr,ETH_MAC_ADDR_LEN);
    NDIS_MEM_CPY_S(pstArpV4Info->aucMacFrmHdr,ETH_MAC_HEADER_LEN,pstReqArp->aucSenderAddr,ETH_MAC_ADDR_LEN);

    pstArpV4Info->ulArpInitFlg  = PS_TRUE;

    NDIS_MEM_CPY_S(g_stSpeMacHeader.aucSrcAddr,ETH_MAC_ADDR_LEN,(pstArpV4Info->aucMacFrmHdr + ETH_MAC_ADDR_LEN),ETH_MAC_ADDR_LEN);
    NDIS_MEM_CPY_S(g_stSpeMacHeader.aucDstAddr,ETH_MAC_ADDR_LEN,pstReqArp->aucSrcAddr,ETH_MAC_ADDR_LEN);


    /*免费ARP不回复响应*/
    if ((pstReqArp->unTargetIP.ulIpAddr == pstReqArp->unSenderIP.ulIpAddr)
        || (0 == pstReqArp->unSenderIP.ulIpAddr))
    {
        return PS_SUCC;
    }

    /*发送响应*/
    NDIS_MEM_CPY_S(pstReqArp->aucDstAddr,ETH_MAC_ADDR_LEN,pstReqArp->aucSrcAddr,ETH_MAC_ADDR_LEN);
    NDIS_MEM_CPY_S(pstReqArp->aucTargetAddr,ETH_MAC_ADDR_LEN,pstReqArp->aucSrcAddr,ETH_MAC_ADDR_LEN);
    pstReqArp->unTargetIP.ulIpAddr = pstReqArp->unSenderIP.ulIpAddr;

    NDIS_MEM_CPY_S(pstReqArp->aucSrcAddr,ETH_MAC_ADDR_LEN, (pstArpV4Info->aucMacFrmHdr + ETH_MAC_ADDR_LEN), ETH_MAC_ADDR_LEN);
    NDIS_MEM_CPY_S(pstReqArp->aucSenderAddr,ETH_MAC_ADDR_LEN, (pstArpV4Info->aucMacFrmHdr + ETH_MAC_ADDR_LEN), ETH_MAC_ADDR_LEN);

    pstReqArp->unSenderIP.ulIpAddr = ulTgtIpAddr;

    /*opcode*/
    pstReqArp->usOpCode = ETH_ARP_RSP_TYPE;

    /*发送ARP Reply*/
    NDIS_STAT_DL_SEND_ARP_REPLY(1);
    (VOS_VOID)Ndis_SendMacFrm((VOS_UINT8*)pstReqArp,sizeof(ETH_ARP_FRAME_STRU),ucRabId);

    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : Ndis_ProcReplyArp
 功能描述  : 处理ARP Reply 帧,更新PC的MAC地址
 输入参数  :

 输出参数  : 无
 返 回 值  : 成功返回PS_SUCC;
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2009年12月31日
    修改内容   : 新生成函数
  2.日    期   : 2013年1月22日
    修改内容   : DSDA
*****************************************************************************/
VOS_UINT32 Ndis_ProcReplyArp(const ETH_ARP_FRAME_STRU *pstRspArp, VOS_UINT8 ucRabId)
{
    NDIS_IPV4_INFO_STRU         *pstArpV4Info = VOS_NULL_PTR;
    VOS_UINT32                   ulTargetIP   = pstRspArp->unTargetIP.ulIpAddr;
    NDIS_ENTITY_STRU            *pstNdisEntity;

    pstNdisEntity = NDIS_GetEntityByRabId(ucRabId);
    if(VOS_NULL_PTR == pstNdisEntity)
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_ProcReplyArp,  NDIS_GetEntityByRabId Error!", ucRabId);
        return PS_FAIL;
    }

    pstArpV4Info  = &pstNdisEntity->stIpV4Info;

    if (ulTargetIP == pstNdisEntity->stIpV4Info.unGwIpInfo.ulIpAddr)
    {
        /*更新PC MAC地址*/
        NDIS_MEM_CPY_S(pstArpV4Info->aucUeMacAddr,ETH_MAC_ADDR_LEN,pstRspArp->aucSenderAddr,ETH_MAC_ADDR_LEN);
        NDIS_MEM_CPY_S(pstArpV4Info->aucMacFrmHdr,ETH_MAC_ADDR_LEN,pstRspArp->aucSenderAddr,ETH_MAC_ADDR_LEN);
        pstArpV4Info->ulArpInitFlg  = PS_TRUE;

        pstArpV4Info->ulArpRepFlg = PS_TRUE;

        return PS_SUCC;
    }

    return PS_FAIL;
}

/*****************************************************************************
 函 数 名  : Ndis_Ipv4PdnInfoCfg
 功能描述  : IPV4 PDN信息配置
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年3月15日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID  Ndis_Ipv4PdnInfoCfg(const AT_NDIS_IPV4_PDN_INFO_STRU *pstNasNdisInfo,
                                           NDIS_ENTITY_STRU  *pstNdisEntity)
{
    NDIS_IPV4_INFO_STRU         *pstIpV4Info    = &(pstNdisEntity->stIpV4Info);

    pstIpV4Info->ulArpInitFlg = PS_FALSE;
    pstIpV4Info->ulArpRepFlg  = PS_FALSE;

    NDIS_MEM_CPY_S(pstIpV4Info->unUeIpInfo.aucIPAddr,
               IPV4_ADDR_LEN,
               pstNasNdisInfo->stPDNAddrInfo.aucIpV4Addr,
               IPV4_ADDR_LEN);

    NDIS_MEM_CPY_S(pstIpV4Info->unGwIpInfo.aucIPAddr,
               IPV4_ADDR_LEN,
               pstNasNdisInfo->stGateWayAddrInfo.aucIpV4Addr,
               IPV4_ADDR_LEN);

    NDIS_MEM_CPY_S(pstIpV4Info->unNmIpInfo.aucIPAddr,
               IPV4_ADDR_LEN,
               pstNasNdisInfo->stSubnetMask.aucIpV4Addr,
               IPV4_ADDR_LEN);

    /*使能则配置DNS*/
    if (PS_TRUE == pstNasNdisInfo->bitOpDnsPrim)
    {
        NDIS_MEM_CPY_S(pstIpV4Info->unPrimDnsAddr.aucIPAddr,
                   IPV4_ADDR_LEN,
                   pstNasNdisInfo->stDnsPrimAddrInfo.aucIpV4Addr,
                   IPV4_ADDR_LEN);
    }
    else
    {
        pstIpV4Info->unPrimDnsAddr.ulIpAddr = 0;
    }

    /*使能则配置辅DNS*/
    if (PS_TRUE == pstNasNdisInfo->bitOpDnsSec)
    {
        NDIS_MEM_CPY_S(pstIpV4Info->unSecDnsAddr.aucIPAddr,
                   IPV4_ADDR_LEN,
                   pstNasNdisInfo->stDnsSecAddrInfo.aucIpV4Addr,
                   IPV4_ADDR_LEN);
    }
    else
    {
        pstIpV4Info->unSecDnsAddr.ulIpAddr = 0;
    }

    /*使能则配置主WINS*/
    if (PS_TRUE == pstNasNdisInfo->bitOpWinsPrim)
    {
        NDIS_MEM_CPY_S(pstIpV4Info->unPrimWinsAddr.aucIPAddr,
                   IPV4_ADDR_LEN,
                   pstNasNdisInfo->stWinsPrimAddrInfo.aucIpV4Addr,
                   IPV4_ADDR_LEN);
    }
    else
    {
        pstIpV4Info->unPrimWinsAddr.ulIpAddr = 0;
    }

    /*使能则配置辅WINS*/
    if (PS_TRUE == pstNasNdisInfo->bitOpWinsSec)
    {
        NDIS_MEM_CPY_S(pstIpV4Info->unSecWinsAddr.aucIPAddr,
                   IPV4_ADDR_LEN,
                   pstNasNdisInfo->stWinsSecAddrInfo.aucIpV4Addr,
                   IPV4_ADDR_LEN);
    }
    else
    {
        pstIpV4Info->unSecWinsAddr.ulIpAddr = 0;
    }

    /*PCSCF暂时不操作,待需求描述*/

    pstIpV4Info->ulIpAssignStatus = IPV4_DHCP_ADDR_STATUS_FREE;

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_StartARPTimer
 功能描述  : NDIS启动周期性ARP定时器
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年4月19日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 Ndis_StartARPTimer(NDIS_ENTITY_STRU *pstNdisEntity)
{
    VOS_UINT32                      ulRtn;
    NDIS_ARP_PERIOD_TIMER_STRU     *pstArpPeriodTimer;

    /*入参指针判断*/
    if (VOS_NULL_PTR == pstNdisEntity)
    {
        return PS_FAIL;
    }

    /*检查是否需要启动ARP定时器*/
    if ((NDIS_ENTITY_IPV4 != (pstNdisEntity->ucRabType & NDIS_ENTITY_IPV4))
            || (PS_TRUE == pstNdisEntity->stIpV4Info.ulArpInitFlg))
    {
        return PS_SUCC;
    }

    pstArpPeriodTimer = &(pstNdisEntity->stIpV4Info.stArpPeriodTimer);

    /*如果还在运行，则停掉*/
    if (VOS_NULL_PTR != pstArpPeriodTimer->hTm)
    {
        Ndis_StopARPTimer(pstArpPeriodTimer);
    }

    ulRtn = VOS_StartRelTimer(&(pstArpPeriodTimer->hTm),PS_PID_APP_NDIS,pstArpPeriodTimer->ulTimerValue,
                               pstArpPeriodTimer->ulName,0, VOS_RELTIMER_LOOP, VOS_TIMER_PRECISION_0);
    if (VOS_OK != ulRtn)
    {
        pstArpPeriodTimer->hTm = VOS_NULL_PTR;
        return PS_FAIL;
    }

    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : Ndis_StopARPTimer
 功能描述  : NDIS停止周期性ARP定时器
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年4月19日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_StopARPTimer(NDIS_ARP_PERIOD_TIMER_STRU *pstArpPeriodTimer)
{
    if (VOS_NULL_PTR != pstArpPeriodTimer->hTm)
    {
        if (VOS_OK != VOS_StopRelTimer(&(pstArpPeriodTimer->hTm)))
        {
            NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_StopARPTimer, VOS_StopRelTimer fail!");
        }
        pstArpPeriodTimer->hTm = VOS_NULL_PTR;
    }

    return;
}

/*****************************************************************************
 函 数 名  : ads_iface_register_rx_handler
 功能描述  : 桩函数
 输入参数  :

 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年08月21日
    修改内容   : 新生成函数

*****************************************************************************/
int ads_iface_register_rx_handler(VOS_UINT8 iface_id,
                                       struct ads_iface_rx_handler_s *rx_handler)
{
    return PS_SUCC;
}
/*****************************************************************************
 函 数 名  : Ndis_ChkRabIdValid
 功能描述  : Ndis检查ExRabId取值是否在合法范围内
 输入参数  :

 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年12月7日
    修改内容   : 新生成函数

  2.日    期   : 2013年1月15日
    修改内容   : DSDA特性开发:对ModemID和RabId均做检查

*****************************************************************************/
VOS_UINT32 Ndis_ChkRabIdValid(VOS_UINT8 ucExRabId)
{
    VOS_UINT16 usModemId;
    VOS_UINT8  ucRabId;

    usModemId = NDIS_GET_MODEMID_FROM_EXBID(ucExRabId);
    if (usModemId >= MODEM_ID_BUTT)
    {
        return PS_FAIL;
    }

    ucRabId = NDIS_GET_BID_FROM_EXBID(ucExRabId);
    if ((ucRabId < MIN_VAL_EPSID) || (ucRabId > MAX_VAL_EPSID))
    {
        return PS_FAIL;
    }

    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : NDIS_GetEntityByRabId
 功能描述  : 根据ExRabId查找NDIS实体
 输入参数  : VOS_UINT8 ucExRabId
 输出参数  : 无
 返 回 值  : NDIS_ENTITY_STRU*
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年1月15日
    修改内容   : 新生成函数

*****************************************************************************/
NDIS_ENTITY_STRU* NDIS_GetEntityByRabId(VOS_UINT8 ucExRabId)
{
    VOS_UINT16 i = 0;

    /* 查询是否已存在相应Entity */
    do{
        if((PS_TRUE == g_astNdisEntity[i].enUsed) && (ucExRabId == g_astNdisEntity[i].ucRabId))
        {
            /*找到相应实体*/
            return &g_astNdisEntity[i];
        }

    }while((++i) < NAS_NDIS_MAX_ITEM);

    return VOS_NULL_PTR;
}

/*****************************************************************************
 函 数 名  : NDIS_AllocEntity
 功能描述  : 分配一个空闲的NDIS实体
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NDIS_ENTITY_STRU*
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年1月15日
    修改内容   : 新生成函数

*****************************************************************************/
NDIS_ENTITY_STRU* NDIS_AllocEntity(VOS_VOID)
{
    VOS_UINT16 i = 0;

    /* 返回第一个空闲的实体*/
    do{
        if(PS_FALSE == g_astNdisEntity[i].enUsed)
        {
            /*找到空闲实体*/
            return &g_astNdisEntity[i];
        }

    }while((++i) < NAS_NDIS_MAX_ITEM);

    return VOS_NULL_PTR;
}

/*****************************************************************************
 函 数 名  : Ndis_CheckIpv4PdnInfo
 功能描述  : PDN IPV4地址信息检查
 输入参数  : AT_NDIS_IPV4_PDN_INFO_STRU
 输出参数  :
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年12月11日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 Ndis_CheckIpv4PdnInfo(const AT_NDIS_IPV4_PDN_INFO_STRU *pstIpv4PdnInfo)
{
    if (PS_FALSE == pstIpv4PdnInfo->bitOpPdnAddr)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_CheckIpv4PdnInfo,  pstIpv4PdnInfo->bitOpPdnAddr is false!");
        return PS_FAIL;
    }

    /*PDN地址和网关地址如果为全0，则也失败*/
    if (0 == VOS_MemCmp(pstIpv4PdnInfo->stPDNAddrInfo.aucIpV4Addr, g_aucInvalidAddr, IPV4_ADDR_LEN))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_CheckIpv4PdnInfo,  stPDNAddrInfo all zero!");
        return PS_FAIL;
    }

    if (0 == VOS_MemCmp(pstIpv4PdnInfo->stGateWayAddrInfo.aucIpV4Addr, g_aucInvalidAddr, IPV4_ADDR_LEN))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_CheckIpv4PdnInfo,  stGateWayAddrInfo all zero!");
        return PS_FAIL;
    }

    return PS_SUCC;
}


VOS_UINT32  Ndis_PdnV4PdnCfg( const AT_NDIS_PDNINFO_CFG_REQ_STRU *pstNasNdisInfo,
                                     NDIS_ENTITY_STRU  *pstNdisEntity)
{
    /*IPV4地址检查*/
    if (PS_FALSE == pstNasNdisInfo->bitOpIpv4PdnInfo)  /*原语指示IPV4信息无效 */
    {
        NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_PdnV4PdnCfg,  bitOpIpv4PdnInfo is false!");
        return PS_FAIL;
    }

    if (PS_SUCC != Ndis_CheckIpv4PdnInfo(&(pstNasNdisInfo->stIpv4PdnInfo)))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnV4PdnCfg,  Ndis_CheckIpv4PdnInfo fail!");
        return PS_FAIL;
    }

    Ndis_Ipv4PdnInfoCfg(&(pstNasNdisInfo->stIpv4PdnInfo),pstNdisEntity);

    /*更新NDIS实体承载属性*/
    pstNdisEntity->ucRabType |= NDIS_ENTITY_IPV4;

    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : Ndis_PdnV6PdnCfg
 功能描述  : IPV6 PDN信息配置
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年12月23日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32  Ndis_PdnV6PdnCfg(const AT_NDIS_PDNINFO_CFG_REQ_STRU *pstNasNdisInfo,
                                     NDIS_ENTITY_STRU  *pstNdisEntity)
{
    VOS_UINT8                       ucExRabId;

    /*IPV6地址检查*/
    if (PS_FALSE == pstNasNdisInfo->bitOpIpv6PdnInfo)  /*原语指示IPV6信息无效*/
    {
        NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_PdnV6PdnCfg,  bitOpIpv6PdnInfo is false!");
        return PS_FAIL;
    }

    ucExRabId = NDIS_FORM_EXBID(pstNasNdisInfo->enModemId, pstNasNdisInfo->ucRabId);

    if (PS_SUCC != NdSer_CheckIpv6PdnInfo(&(pstNasNdisInfo->stIpv6PdnInfo)))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnV6PdnCfg,  NdSer_CheckIpv6PdnInfo fail!");
        return PS_FAIL;
    }

    /*调ND SERVER API  配置IPV6地址信息给ND SERVER*/
    NdSer_Ipv6PdnInfoCfg(ucExRabId, &(pstNasNdisInfo->stIpv6PdnInfo));

    /*更新NDIS实体属性*/
    pstNdisEntity->ucRabType |= NDIS_ENTITY_IPV6;

    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : Ndis_AtCnfResultProc
 功能描述  : NDIS向AT返回的配置确认结果处理
 输入参数  :

 输出参数  : 无
 返 回 值  : 成功返回PS_SUCC;
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年4月25日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT8 Ndis_AtCnfResultProc(const AT_NDIS_PDNINFO_CFG_REQ_STRU *pstNasNdisInfo, VOS_UINT32 ulV4Ret, VOS_UINT32 ulV6Ret)
{
    VOS_UINT8  enResult;

    /*根据配置结果向AT返回配置CNF原语*/
    if ((PS_TRUE == pstNasNdisInfo->bitOpIpv4PdnInfo) &&(PS_TRUE == pstNasNdisInfo->bitOpIpv6PdnInfo))
    {
        if ((PS_SUCC == ulV4Ret) &&(PS_SUCC == ulV6Ret))        /*IPV4和IPV6配置都成功*/
        {
            enResult = AT_NDIS_PDNCFG_CNF_SUCC;
        }
        else if (PS_SUCC == ulV4Ret)                             /*只有IPV4配置成功*/
        {
            enResult = AT_NDIS_PDNCFG_CNF_IPV4ONLY_SUCC;
        }
        else                                                     /*只有IPV6配置成功*/
        {
            enResult = AT_NDIS_PDNCFG_CNF_IPV6ONLY_SUCC;
        }
    }
    else if (PS_TRUE == pstNasNdisInfo->bitOpIpv4PdnInfo)  /*只配置了IPV4*/
    {
        enResult = AT_NDIS_PDNCFG_CNF_IPV4ONLY_SUCC;
    }
    else                                                   /*只配置了IPV6*/
    {
        enResult = AT_NDIS_PDNCFG_CNF_IPV6ONLY_SUCC;
    }

    return enResult;
}
/*****************************************************************************
 函 数 名  : Ndis_PdnInfoCfgProc
 功能描述  : PDN地址信息参数配置
 输入参数  :

 输出参数  : 无
 返 回 值  : 成功返回PS_SUCC;
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年12月31日
    修改内容   : 新生成函数

  2.日    期   : 2013年1月15日
    修改内容   : DSDA特性，主要增加对接口消息中ModemId的处理

*****************************************************************************/
VOS_VOID Ndis_PdnInfoCfgProc(const AT_NDIS_PDNINFO_CFG_REQ_STRU *pstNasNdisInfo)
{
    VOS_UINT8                       ucExRabId;
    UDI_HANDLE                      ulHandle;
    NDIS_ENTITY_STRU               *pstNdisEntity = VOS_NULL_PTR;
    AT_NDIS_PDNINFO_CFG_CNF_STRU    stCfgCnf;
    VOS_UINT32                      ulV4Ret;
    VOS_UINT32                      ulV6Ret;
    VOS_INT32                       lSpePort;
    VOS_UINT32                      ulSpeIpfFlag;

    NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc entered!");

    /*长度异常检查*/
    if ((sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH) > pstNasNdisInfo->ulLength)
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc: input msg length less than struc", pstNasNdisInfo->ulMsgId);
        return;
    }

    /*生成扩展的RabId*/
    ucExRabId  = NDIS_FORM_EXBID(pstNasNdisInfo->enModemId, pstNasNdisInfo->ucRabId);
    ulHandle = pstNasNdisInfo->ulHandle;
    lSpePort = pstNasNdisInfo->lSpePort;
    ulSpeIpfFlag = pstNasNdisInfo->ulIpfFlag;

    stCfgCnf.enResult  = AT_NDIS_PDNCFG_CNF_FAIL;
    stCfgCnf.ucRabId   = pstNasNdisInfo->ucRabId;
    stCfgCnf.enModemId = pstNasNdisInfo->enModemId;

    /*ExRabId参数范围有效性检查。若检查失败，则直接向AT回复配置失败*/
    if (PS_SUCC != Ndis_ChkRabIdValid(ucExRabId))
    {
        (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8*)&stCfgCnf,sizeof(AT_NDIS_PDNINFO_CFG_CNF_STRU),ID_AT_NDIS_PDNINFO_CFG_CNF);
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc,  Ndis_ChkRabIdValid fail!");
        return;
    }

    /*如果根据ExRabId查找不到NDIS实体，则分配一个空闲的NDIS实体*/
    pstNdisEntity = NDIS_GetEntityByRabId(ucExRabId);
    if(VOS_NULL_PTR == pstNdisEntity)
    {
        /*如果分配不到空闲的NDIS实体，则返回*/
        pstNdisEntity = NDIS_AllocEntity();
        if(VOS_NULL_PTR == pstNdisEntity)
        {
            /*向AT回复PDN配置失败*/
            (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8*)&stCfgCnf,sizeof(AT_NDIS_PDNINFO_CFG_CNF_STRU),ID_AT_NDIS_PDNINFO_CFG_CNF);
            NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc,  NDIS_AllocEntity failed!");
            return;
        }

        /*该承载之前没有对应的NDIS实体，故填无效值*/
        pstNdisEntity->ucRabType= NDIS_RAB_NULL;
        pstNdisEntity->ulHandle = NDIS_INVALID_HANDLE;
        pstNdisEntity->lSpePort = NDIS_INVALID_SPEPORT;
        pstNdisEntity->ulSpeIpfFlag= PS_FALSE;
    }

    ulV4Ret = Ndis_PdnV4PdnCfg(pstNasNdisInfo,pstNdisEntity);
    ulV6Ret = Ndis_PdnV6PdnCfg(pstNasNdisInfo,pstNdisEntity);

    if ((PS_FAIL == ulV6Ret) && (PS_FAIL == ulV4Ret))   /*如果IPV4和IPV6配置指示信息都无效，也认为配置失败*/
    {
        /*向AT回复PDN配置失败*/
        (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8*)&stCfgCnf,sizeof(AT_NDIS_PDNINFO_CFG_CNF_STRU),ID_AT_NDIS_PDNINFO_CFG_CNF);
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc,  Ipv4 and Ipv6 Cfg all fail!");
        return;
    }

    pstNdisEntity->enUsed = PS_TRUE;      /*设置该NDIS实体为使用状态*/
    pstNdisEntity->ucRabId  = ucExRabId;  /*将扩展RabId存到对应NDIS实体中*/
    pstNdisEntity->ulHandle = ulHandle;   /*保存Handle到NDIS实体中*/
    pstNdisEntity->lSpePort = lSpePort;   /*保存SPE Port到NDIS实体中*/
    pstNdisEntity->ulSpeIpfFlag = ulSpeIpfFlag;

    stCfgCnf.enResult  = Ndis_AtCnfResultProc(pstNasNdisInfo, ulV4Ret, ulV6Ret);
    stCfgCnf.ucRabType = pstNdisEntity->ucRabType;

    /*启动周期发送ARP的定时器*/
    if (PS_SUCC != Ndis_StartARPTimer(pstNdisEntity))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_ConfigArpInfo StartTmr Failed!");
        return;
    }

    pstNdisEntity->lSpePort = NDIS_INVALID_SPEPORT;

    /*向ADS注册下行回调:只注册一次*/
    if (VOS_OK != (ADS_DL_RegDlDataCallback(ucExRabId, Ndis_DlAdsDataRcv, 0)))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc, ADS_DL_RegDlDataCallback fail!");
        return;
    }

    /*lint -e718*/
    if (VOS_OK != NDIS_UDI_IOCTL (pstNdisEntity->ulHandle, NCM_IOCTL_REG_UPLINK_RX_FUNC, AppNdis_UsbReadCb))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc, regist AppNdis_UsbReadCb fail!");
        return;
    }
    /*lint +e718*/

    (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8*)&stCfgCnf,sizeof(AT_NDIS_PDNINFO_CFG_CNF_STRU),ID_AT_NDIS_PDNINFO_CFG_CNF);

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_PdnRel
 功能描述  : PDN释放
 输入参数  :

 输出参数  : 无
 返 回 值  : 成功返回PS_SUCC;
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年12月7日
    修改内容   : 用户面融合修改

  1.日    期   : 2013年1月15日
    修改内容   : DSDA特性开发:

*****************************************************************************/
VOS_VOID Ndis_PdnRel(const AT_NDIS_PDNINFO_REL_REQ_STRU *pstNasNdisRel)
{
    VOS_UINT8                      ucExRabId;
    NDIS_ENTITY_STRU              *pstNdisEntity     = VOS_NULL_PTR;
    NDIS_ARP_PERIOD_TIMER_STRU    *pstArpPeriodTimer = VOS_NULL_PTR;
    AT_NDIS_PDNINFO_REL_CNF_STRU   stRelCnf;

    NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_PdnRel entered!");

    /*长度异常检查*/
    if ((sizeof(AT_NDIS_PDNINFO_REL_REQ_STRU) - VOS_MSG_HEAD_LENGTH) > pstNasNdisRel->ulLength)
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_PdnRel: input msg length less than struc", pstNasNdisRel->ulMsgId);
        return;
    }

    stRelCnf.enResult  = AT_NDIS_FAIL;
    stRelCnf.ucRabId   = pstNasNdisRel->ucRabId;
    stRelCnf.enModemId = pstNasNdisRel->enModemId;

    ucExRabId = NDIS_FORM_EXBID(pstNasNdisRel->enModemId, pstNasNdisRel->ucRabId);
    if (PS_FAIL == Ndis_ChkRabIdValid(ucExRabId))
    {
        (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8*)&stRelCnf,sizeof(AT_NDIS_PDNINFO_REL_CNF_STRU),ID_AT_NDIS_PDNINFO_REL_CNF);
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnRel,  Ndis_ChkRabIdValid fail!");
        return;
    }

    pstNdisEntity = NDIS_GetEntityByRabId(ucExRabId);
    if(VOS_NULL_PTR == pstNdisEntity)
    {
        (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8*)&stRelCnf,sizeof(AT_NDIS_PDNINFO_REL_CNF_STRU),ID_AT_NDIS_PDNINFO_REL_CNF);
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnRel,  NDIS_GetEntityByRabId failed!");
        return;
    }

    pstArpPeriodTimer = &(pstNdisEntity->stIpV4Info.stArpPeriodTimer);

    stRelCnf.ucRabType = pstNdisEntity->ucRabType;

    /*如果周期性ARP定时器还在运行，则停掉*/
    Ndis_StopARPTimer(pstArpPeriodTimer);

    /*调用ND SERVER API 释放该RabId对应ND SERVER实体*/
    if (NDIS_ENTITY_IPV6 == (pstNdisEntity->ucRabType & NDIS_ENTITY_IPV6))
    {
        NdSer_Ipv6PdnRel(ucExRabId);
    }

    /*更新该RabId对应NDIS实体为空*/
    pstNdisEntity->ucRabType = NDIS_RAB_NULL;
    pstNdisEntity->ucRabId   = NDIS_INVALID_RABID;
    pstNdisEntity->ulHandle  = NDIS_INVALID_HANDLE;
    pstNdisEntity->enUsed    = PS_FALSE;
    pstNdisEntity->lSpePort = NDIS_INVALID_SPEPORT;
    pstNdisEntity->ulSpeIpfFlag = PS_FALSE;

    /*NDIS向AT回复释放确认原语*/
    stRelCnf.enResult  = AT_NDIS_SUCC;
    (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8*)&stRelCnf,sizeof(AT_NDIS_PDNINFO_REL_CNF_STRU),ID_AT_NDIS_PDNINFO_REL_CNF);

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_ATIfaceUpCfgTransToPdnInfoCfg
 功能描述  : IFACE UP CFG消息内容转成PND INFO CFG消息内容
 输入参数  :

 输出参数  : 无
 返 回 值  : 成功返回PS_SUCC;
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年08月16日
    修改内容   : 新增

*****************************************************************************/

VOS_VOID Ndis_ATIfaceUpCfgTransToPdnInfoCfg(const	AT_NDIS_IFACE_UP_CONFIG_IND_STRU *pstNdisIFaceInfo,
                                            AT_NDIS_PDNINFO_CFG_REQ_STRU             *pstNdisPdnInfo)
{
    NDIS_MEM_SET_S((VOS_UINT8*)pstNdisPdnInfo,
                    sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU),
                    0,
                    sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU));

    pstNdisPdnInfo->ulMsgId             = pstNdisIFaceInfo->ulMsgId;
    pstNdisPdnInfo->bitOpIpv4PdnInfo    = pstNdisIFaceInfo->bitOpIpv4PdnInfo;
    pstNdisPdnInfo->bitOpIpv6PdnInfo    = pstNdisIFaceInfo->bitOpIpv6PdnInfo;
    pstNdisPdnInfo->enModemId           = NDIS_GET_MODEMID_FROM_EXBID(pstNdisIFaceInfo->ucIfaceId);/*将Iface ID转成ModemId+RabId*/
    pstNdisPdnInfo->ucRabId             = NDIS_GET_BID_FROM_EXBID(pstNdisIFaceInfo->ucIfaceId);    /*将Iface ID转成ModemId+RabId*/
    pstNdisPdnInfo->ulHandle            = pstNdisIFaceInfo->ulHandle;
    NDIS_MEM_CPY_S(&pstNdisPdnInfo->stIpv4PdnInfo,
                   sizeof(AT_NDIS_IPV4_PDN_INFO_STRU),
                   &pstNdisIFaceInfo->stIpv4PdnInfo,
                   sizeof(AT_NDIS_IPV4_PDN_INFO_STRU));
    NDIS_MEM_CPY_S(&pstNdisPdnInfo->stIpv6PdnInfo,
                   sizeof(AT_NDIS_IPV6_PDN_INFO_STRU),
                   &pstNdisIFaceInfo->stIpv6PdnInfo,
                   sizeof(AT_NDIS_IPV6_PDN_INFO_STRU));

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_IfaceUpCfgProc
 功能描述  : IFACE UP消息处理
 输入参数  :

 输出参数  : 无
 返 回 值  : 成功返回PS_SUCC;
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年08月16日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_IfaceUpCfgProc(const AT_NDIS_IFACE_UP_CONFIG_IND_STRU *pstIfacInfo)
{
    VOS_UINT8                       ucExRabId;
    UDI_HANDLE                      ulHandle;
    NDIS_ENTITY_STRU               *pstNdisEntity = VOS_NULL_PTR;
    VOS_UINT32                      ulV4Ret;
    VOS_UINT32                      ulV6Ret;
    AT_NDIS_PDNINFO_CFG_REQ_STRU    stPdnInfo;
    struct ads_iface_rx_handler_s   stIfaceRxHandle;

    NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc entered!");

    /*长度异常检查*/
    if ((sizeof(AT_NDIS_IFACE_UP_CONFIG_IND_STRU) - VOS_MSG_HEAD_LENGTH) > pstIfacInfo->ulLength)
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc: input msg length less than struc", pstIfacInfo->ulMsgId);
        return;
    }

    /*生成扩展的RabId*/
    ucExRabId  = pstIfacInfo->ucIfaceId;
    ulHandle   = pstIfacInfo->ulHandle;

    /*ExRabId参数范围有效性检查。若检查失败，则直接向AT回复配置失败*/
    if (PS_SUCC != Ndis_ChkRabIdValid(ucExRabId))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc,  Ndis_ChkRabIdValid fail!");
        return;
    }

    /*如果根据ExRabId查找不到NDIS实体，则分配一个空闲的NDIS实体*/
    pstNdisEntity = NDIS_GetEntityByRabId(ucExRabId);
    if(VOS_NULL_PTR == pstNdisEntity)
    {
        /*如果分配不到空闲的NDIS实体，则返回*/
        pstNdisEntity = NDIS_AllocEntity();
        if(VOS_NULL_PTR == pstNdisEntity)
        {
            NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc,  NDIS_AllocEntity failed!");
            return;
        }

        /*该承载之前没有对应的NDIS实体，故填无效值*/
        pstNdisEntity->ucRabType= NDIS_RAB_NULL;
        pstNdisEntity->ulHandle = NDIS_INVALID_HANDLE;
        pstNdisEntity->lSpePort = NDIS_INVALID_SPEPORT;
        pstNdisEntity->ulSpeIpfFlag= PS_FALSE;
    }

    Ndis_ATIfaceUpCfgTransToPdnInfoCfg(pstIfacInfo, &stPdnInfo);
    ulV4Ret = Ndis_PdnV4PdnCfg(&stPdnInfo,pstNdisEntity);
    ulV6Ret = Ndis_PdnV6PdnCfg(&stPdnInfo,pstNdisEntity);

    if ((PS_FAIL == ulV6Ret) && (PS_FAIL == ulV4Ret))   /*如果IPV4和IPV6配置指示信息都无效，也认为配置失败*/
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc,  Ipv4 and Ipv6 Cfg all fail!");
        return;
    }

    pstNdisEntity->enUsed = PS_TRUE;      /*设置该NDIS实体为使用状态*/
    pstNdisEntity->ucRabId  = ucExRabId;  /*将扩展RabId存到对应NDIS实体中*/
    pstNdisEntity->ulHandle = ulHandle;   /*保存Handle到NDIS实体中*/
    pstNdisEntity->lSpePort = 0;          /*保存SPE Port到NDIS实体中*/
    pstNdisEntity->ulSpeIpfFlag = 0;

    /*启动周期发送ARP的定时器*/
    if (PS_SUCC != Ndis_StartARPTimer(pstNdisEntity))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc StartTmr Failed!");
        return;
    }

    pstNdisEntity->lSpePort = NDIS_INVALID_SPEPORT;

    stIfaceRxHandle.user_data       = ucExRabId;
    stIfaceRxHandle.rx_func         = Ndis_DlAdsDataRcvV2;
    stIfaceRxHandle.rx_cmplt_func   = VOS_NULL_PTR;

    /*向ADS注册下行回调*/
    if (VOS_OK != ads_iface_register_rx_handler(ucExRabId, &stIfaceRxHandle))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc, ADS_DL_RegDlDataCallback fail!");
        return;
    }

    /*lint -e718*/
    if (VOS_OK != NDIS_UDI_IOCTL (pstNdisEntity->ulHandle, NCM_IOCTL_REG_UPLINK_RX_FUNC, AppNdis_UsbReadCb))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc, regist AppNdis_UsbReadCb fail!");
        return;
    }
    /*lint +e718*/

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_IfaceDownCfgProc
 功能描述  : IFACE DOWN消息处理
 输入参数  :

 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年08月16日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_IfaceDownCfgProc(const AT_NDIS_IFACE_DOWN_CONFIG_IND_STRU *pstIfacInfo)

{
    VOS_UINT8                      ucExRabId;
    NDIS_ENTITY_STRU              *pstNdisEntity        = VOS_NULL_PTR;
    NDIS_ARP_PERIOD_TIMER_STRU    *pstArpPeriodTimer    = VOS_NULL_PTR;

    NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_IfaceDownCfgProc entered!");

    /*长度异常检查*/
    if ((sizeof(AT_NDIS_IFACE_DOWN_CONFIG_IND_STRU) - VOS_MSG_HEAD_LENGTH) > pstIfacInfo->ulLength)
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_IfaceDownCfgProc: input msg length less than struc", pstIfacInfo->ulMsgId);
        return;
    }

    ucExRabId = pstIfacInfo->ucIfaceId;
    if (PS_SUCC != Ndis_ChkRabIdValid(ucExRabId))
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "pstIfacInfo,  Ndis_ChkRabIdValid fail!");
        return;
    }

    pstNdisEntity = NDIS_GetEntityByRabId(ucExRabId);
    if(VOS_NULL_PTR == pstNdisEntity)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "pstIfacInfo,  NDIS_GetEntityByRabId failed!");
        return;
    }

    pstArpPeriodTimer = &(pstNdisEntity->stIpV4Info.stArpPeriodTimer);

    /*如果周期性ARP定时器还在运行，则停掉*/
    Ndis_StopARPTimer(pstArpPeriodTimer);

    /*调用ND SERVER API 释放该RabId对应ND SERVER实体*/
    if (NDIS_ENTITY_IPV6 == (pstNdisEntity->ucRabType & NDIS_ENTITY_IPV6))
    {
        NdSer_Ipv6PdnRel(ucExRabId);
    }

    /*更新该RabId对应NDIS实体为空*/
    pstNdisEntity->ucRabType = NDIS_RAB_NULL;
    pstNdisEntity->ucRabId   = NDIS_INVALID_RABID;
    pstNdisEntity->ulHandle  = NDIS_INVALID_HANDLE;
    pstNdisEntity->enUsed    = PS_FALSE;
    pstNdisEntity->lSpePort = NDIS_INVALID_SPEPORT;
    pstNdisEntity->ulSpeIpfFlag = PS_FALSE;

    return;
}

/*NDIS ARP PROC Begin*/
/*****************************************************************************
 函 数 名  : Ndis_SendRequestArp
 功能描述  : 发送ARP Request 帧到Ethenet上
 输入参数  :

 输出参数  : 无
 返 回 值  : 成功返回PS_SUCC;
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2009年12月31日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32  Ndis_SendRequestArp(NDIS_IPV4_INFO_STRU  *pstArpInfoItem, VOS_UINT8 ucExRabId)
{
    ETH_ARP_FRAME_STRU  stArpReq;

    /*之前一次发送的Req尚未受到Reply反馈*/
    if (PS_FALSE == pstArpInfoItem->ulArpRepFlg)
    {
        NDIS_STAT_ARPREPLY_NOTRECV(1);
        /*做一次告警日志*/
    }

    NDIS_MEM_SET_S((VOS_UINT8*)&stArpReq,sizeof(ETH_ARP_FRAME_STRU), 0, sizeof(ETH_ARP_FRAME_STRU));

    /*组ARP Request*/
    NDIS_MEM_CPY_S(stArpReq.aucDstAddr,ETH_MAC_ADDR_LEN,g_aucBroadCastAddr,ETH_MAC_ADDR_LEN);
    NDIS_MEM_CPY_S(stArpReq.aucSrcAddr,ETH_MAC_ADDR_LEN,pstArpInfoItem->aucMacFrmHdr+ETH_MAC_ADDR_LEN, ETH_MAC_ADDR_LEN);
    stArpReq.usFrameType = ARP_PAYLOAD;

    /*请求的固定部分*/
    NDIS_MEM_CPY_S(((VOS_UINT8*)&stArpReq + ETH_MAC_HEADER_LEN),ETH_ARP_FIXED_MSG_LEN,g_aucArpReqFixVal, ETH_ARP_FIXED_MSG_LEN);

    /*Payload部分的MAC地址设置*/
    NDIS_MEM_SET_S(stArpReq.aucTargetAddr,ETH_MAC_ADDR_LEN,0,ETH_MAC_ADDR_LEN);
    NDIS_MEM_CPY_S(stArpReq.aucSenderAddr,ETH_MAC_ADDR_LEN,pstArpInfoItem->aucMacFrmHdr+ETH_MAC_ADDR_LEN,ETH_MAC_ADDR_LEN);

    /*单板IP*/
    stArpReq.unSenderIP.ulIpAddr = pstArpInfoItem->unGwIpInfo.ulIpAddr;
    stArpReq.unTargetIP.ulIpAddr = pstArpInfoItem->unUeIpInfo.ulIpAddr;

    if (PS_SUCC != Ndis_SendMacFrm((VOS_UINT8*)&stArpReq,sizeof(ETH_ARP_FRAME_STRU),ucExRabId))
    {
        pstArpInfoItem->ulArpRepFlg = PS_TRUE;
        NDIS_STAT_DL_SEND_ARP_REQUEST_FAIL(1);
        return PS_FAIL;
    }

    NDIS_STAT_DL_SEND_ARP_REQUEST_SUCC(1);

    pstArpInfoItem->ulArpRepFlg = PS_FALSE;

    return PS_SUCC;
}

/*****************************************************************************
 函 数 名  : Ndis_ProcArpMsg
 功能描述  : 处理底软发送上来的ARP帧
 输入参数  :

 输出参数  : 无
 返 回 值  : 成功返回PS_SUCC;
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2009年12月31日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 Ndis_ProcArpMsg(ETH_ARP_FRAME_STRU* pstArpMsg, VOS_UINT8 ucRabId)
{
    /*Arp Request*/
    if(0 == VOS_MemCmp((VOS_UINT8*)pstArpMsg + ETH_MAC_HEADER_LEN,g_aucArpReqFixVal, ETH_ARP_FIXED_MSG_LEN))
    {
        NDIS_STAT_UL_RECV_ARP_REQUEST(1);
        return Ndis_ProcReqArp(pstArpMsg, ucRabId);
    }
    /*ARP Reply*/
    else if(0 == VOS_MemCmp((VOS_UINT8*)pstArpMsg + ETH_MAC_HEADER_LEN,g_aucArpRspFixVal,ETH_ARP_FIXED_MSG_LEN))
    {
        NDIS_STAT_DL_RECV_ARP_REPLY(1);
        return Ndis_ProcReplyArp(pstArpMsg, ucRabId);
    }
    else
    {
        NDIS_STAT_PROC_ARP_FAIL(1);
        return PS_FAIL;
    }
}

/*****************************************************************************
 函 数 名  : Ndis_AtMsgProc
 功能描述  :
 输入参数  : const MsgBlock *pMsgBlock
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年3月16日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_AtMsgProc( const MsgBlock *pMsgBlock )
{
    AT_NDIS_MSG_ID_ENUM_UINT32      ulMsgId;

    /*begin: 鹰眼插桩*/
    COVERITY_TAINTED_SET(pMsgBlock->aucValue);
    /*end: 鹰眼插桩*/

    /*长度异常保护*/
    if (sizeof(MSG_HEADER_STRU) - VOS_MSG_HEAD_LENGTH > pMsgBlock->ulLength )
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_AtMsgProc: input msg length less than struc MSG_HEADER_STRU", pMsgBlock->ulLength);
        return;
    }

    ulMsgId  = ((MSG_HEADER_STRU *)(VOS_VOID*)pMsgBlock)->ulMsgName;

    switch (ulMsgId)
    {
        case ID_AT_NDIS_PDNINFO_CFG_REQ :/*根据消息的不同处理AT不同的请求*/
            Ndis_PdnInfoCfgProc((AT_NDIS_PDNINFO_CFG_REQ_STRU *)(VOS_VOID*)pMsgBlock);
            break;

        case ID_AT_NDIS_PDNINFO_REL_REQ :
            Ndis_PdnRel((AT_NDIS_PDNINFO_REL_REQ_STRU *)(VOS_VOID*)pMsgBlock);
            break;

        case ID_AT_NDIS_IFACE_UP_CONFIG_IND:
            Ndis_IfaceUpCfgProc((AT_NDIS_IFACE_UP_CONFIG_IND_STRU *)(VOS_VOID*)pMsgBlock);
            break;

        case ID_AT_NDIS_IFACE_DOWN_CONFIG_IND:
            Ndis_IfaceDownCfgProc((AT_NDIS_IFACE_DOWN_CONFIG_IND_STRU *)(VOS_VOID*)pMsgBlock);
            break;

        default:
            NDIS_WARNING_LOG(NDIS_TASK_PID, "Ndis_AtMsgProc:Error Msg!");
            break;
    }

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_AdsMsgProc
 功能描述  : NDIS接收ADS消息处理函数
 输入参数  : MsgBlock* pMsgBlock
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年12月15日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_AdsMsgProc(const MsgBlock* pMsgBlock )
{
    ADS_NDIS_DATA_IND_STRU  *pstAdsNdisMsg  = (ADS_NDIS_DATA_IND_STRU*)(VOS_VOID*)pMsgBlock;

    if ( sizeof(ADS_NDIS_DATA_IND_STRU) - VOS_MSG_HEAD_LENGTH > pMsgBlock->ulLength )
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_AdsMsgProc, input msg length less than struc", pMsgBlock->ulLength);
        return;
    }
    if (VOS_NULL_PTR == pstAdsNdisMsg->pstData)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_AdsMsgProc recv NULL PTR!");
        return;
    }

    if (ID_ADS_NDIS_DATA_IND != pstAdsNdisMsg->ulMsgId)
    {
        /*lint -e522*/
        IMM_ZcFree(pstAdsNdisMsg->pstData);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_AdsMsgProc, MsgId error!");
        return;
    }

    switch (pstAdsNdisMsg->enIpPacketType)
    {
        case ADS_NDIS_IP_PACKET_TYPE_DHCPV4:                                     /*DHCP包*/
             Ndis_DHCPPkt_Proc(pstAdsNdisMsg);
             break;
        case ADS_NDIS_IP_PACKET_TYPE_DHCPV6:                                     /*DHCPV6包*/
             NdSer_DhcpV6PktProc(pstAdsNdisMsg);
             break;
        case ADS_NDIS_IP_PACKET_TYPE_ICMPV6:                                     /*ND和ECHO REQUEST包*/
             NdSer_NdAndEchoPktProc(pstAdsNdisMsg);
             break;

        default:
             NDIS_WARNING_LOG1(NDIS_TASK_PID, "Ndis_AdsMsgProc:Other Msg!", pstAdsNdisMsg->enIpPacketType);
             break;
    }

     /*处理完成后释放ImmZc*/
     /*lint -e522*/
     IMM_ZcFree(pstAdsNdisMsg->pstData);
     /*lint +e522*/

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_AdsV2MsgTransToV1Msg
 功能描述  : ADS V2消息内容转成V1格式
 输入参数  :
 输出参数  :
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年08月16日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_AdsV2MsgTransToV1Msg(ADS_NDIS_DATA_IND_V2_STRU    *pstV2Msg,
                                              ADS_NDIS_DATA_IND_STRU       *pstV1Msg)
{
    NDIS_MEM_SET_S(pstV1Msg,
                   sizeof(ADS_NDIS_DATA_IND_STRU),
                   0,
                   sizeof(ADS_NDIS_DATA_IND_STRU));

    pstV1Msg->ulMsgId           = ID_ADS_NDIS_DATA_IND;
    pstV1Msg->enModemId         = NDIS_GET_MODEMID_FROM_EXBID(pstV2Msg->ucIfaceId);
    pstV1Msg->ucRabId           = NDIS_GET_BID_FROM_EXBID(pstV2Msg->ucIfaceId);
    pstV1Msg->enIpPacketType    = pstV2Msg->enIpPacketType;
    pstV1Msg->pstData           = pstV2Msg->pstData;

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_AdsMsgProcV2
 功能描述  : NDIS接收ADS消息处理函数
 输入参数  : MsgBlock* pMsgBlock
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年12月15日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_AdsMsgProcV2(const MsgBlock* pMsgBlock )
{
    ADS_NDIS_DATA_IND_V2_STRU  *pstAdsNdisMsg  = (ADS_NDIS_DATA_IND_V2_STRU*)(VOS_VOID*)pMsgBlock;
    ADS_NDIS_DATA_IND_STRU      stAdsNdisV1Msg;

    if ( sizeof(ADS_NDIS_DATA_IND_V2_STRU) - VOS_MSG_HEAD_LENGTH > pMsgBlock->ulLength )
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_AdsMsgProcV2, input msg length less than struc", pMsgBlock->ulLength);
        return;
    }
    if (VOS_NULL_PTR == pstAdsNdisMsg->pstData)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_AdsMsgProcV2 recv NULL PTR!");
        return;
    }

    Ndis_AdsV2MsgTransToV1Msg(pstAdsNdisMsg, &stAdsNdisV1Msg);

    switch (stAdsNdisV1Msg.enIpPacketType)
    {
        case ADS_NDIS_IP_PACKET_TYPE_DHCPV4:                                     /*DHCP包*/
             Ndis_DHCPPkt_Proc(&stAdsNdisV1Msg);
             break;
        case ADS_NDIS_IP_PACKET_TYPE_DHCPV6:                                     /*DHCPV6包*/
             NdSer_DhcpV6PktProc(&stAdsNdisV1Msg);
             break;
        case ADS_NDIS_IP_PACKET_TYPE_ICMPV6:                                     /*ND和ECHO REQUEST包*/
             NdSer_NdAndEchoPktProc(&stAdsNdisV1Msg);
             break;

        default:
             NDIS_WARNING_LOG1(NDIS_TASK_PID, "Ndis_AdsMsgProcV2:Other Msg!", stAdsNdisV1Msg.enIpPacketType);
             break;
    }

     /*处理完成后释放ImmZc*/
     /*lint -e522*/
     IMM_ZcFree(pstAdsNdisMsg->pstData);
     /*lint +e522*/

    return;
}


/*****************************************************************************
 函 数 名  : Ndis_AdsMsgDispatch
 功能描述  : NDIS接收ADS消息处理函数
 输入参数  : MsgBlock* pMsgBlock
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年08月16日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_AdsMsgDispatch(const MsgBlock* pMsgBlock )
{
    AT_NDIS_MSG_ID_ENUM_UINT32      ulMsgId;

    /*begin: 鹰眼插桩*/
    COVERITY_TAINTED_SET(pMsgBlock->aucValue);
    /*end: 鹰眼插桩*/

    /*长度异常保护*/
    if (sizeof(MSG_HEADER_STRU) - VOS_MSG_HEAD_LENGTH > pMsgBlock->ulLength )
    {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_AdsMsgDispatch: input msg length less than struc MSG_HEADER_STRU", pMsgBlock->ulLength);
        return;
    }

    ulMsgId  = ((MSG_HEADER_STRU *)(VOS_VOID*)pMsgBlock)->ulMsgName;

    switch (ulMsgId)
    {
        case ID_ADS_NDIS_DATA_IND :/*根据消息的不同处理AT不同的请求*/
            Ndis_AdsMsgProc(pMsgBlock);
            break;

        case ID_ADS_NDIS_DATA_IND_V2 :
            Ndis_AdsMsgProcV2(pMsgBlock);
            break;

        default:
            NDIS_WARNING_LOG(NDIS_TASK_PID, "Ndis_AdsMsgDispatch:Error Msg!");
            break;
    }

    return;
}

/*****************************************************************************
 函 数 名  : APP_Ndis_PidMsgProc
 功能描述  : NDIS接收各模块消息处理函数
 输入参数  : MsgBlock* pMsgBlock
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年2月15日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID APP_Ndis_PidMsgProc(const MsgBlock* pMsgBlock )
{
    if (VOS_NULL_PTR == pMsgBlock)
    {
        PS_PRINTF_INFO("Error:APP_Ndis_DLPidMsgProc Parameter pRcvMsg is NULL!");
        return ;
    }

    switch (pMsgBlock->ulSenderPid)
    {
        case DOPRA_PID_TIMER:
            /*lint -e826*/
            Ndis_ProcTmrMsg((REL_TIMER_MSG *)pMsgBlock);
            /*lint +e826*/
            break;

        case APP_AT_PID:
            Ndis_AtMsgProc(pMsgBlock);
            break;

        case ACPU_PID_ADS_UL:          /*ADS通过OSA消息发送DHCP和ND SERVER包给NDIS模块*/
            Ndis_AdsMsgDispatch(pMsgBlock);
            break;

        default:
            NDIS_WARNING_LOG(NDIS_TASK_PID,"Warning:APP_Ndis_PidMsgProc Recv not expected msg!");
            break;
    }

    return ;
}

/*lint -e40*/
/*****************************************************************************
 函 数 名  : APP_NDIS_FidInit
 功能描述  : NDIS的FID初始化函数
 输入参数  : enum VOS_INIT_PHASE_DEFINE enPhase
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2008年9月17日
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 APP_NDIS_FidInit(enum VOS_INIT_PHASE_DEFINE enPhase)
{
    VOS_UINT32  ulResult = PS_FAIL;

    switch (enPhase)
    {
        case   VOS_IP_LOAD_CONFIG:

            /*注册NDIS PID*/
            ulResult = VOS_RegisterPIDInfo(NDIS_TASK_PID,
                                           (Init_Fun_Type)APP_Ndis_Pid_InitFunc,
                                           (Msg_Fun_Type)APP_Ndis_PidMsgProc);
            if (VOS_OK != ulResult)
            {
                PS_PRINTF_ERR("APP_NDIS_FidInit, register NDIS PID fail!\n");
                return VOS_ERR;
            }

            /*注册ND SERVER PID*/
            ulResult = VOS_RegisterPIDInfo(NDIS_NDSERVER_PID,
                                                       (Init_Fun_Type)APP_NdServer_Pid_InitFunc,
                                                       (Msg_Fun_Type)APP_NdServer_PidMsgProc);
            if (VOS_OK != ulResult)
            {
                PS_PRINTF_ERR("APP_NDIS_FidInit, register NDSERVER PID fail!\n");
                return VOS_ERR;
            }

            ulResult = VOS_RegisterMsgTaskPrio(NDIS_TASK_FID, VOS_PRIORITY_P4);
            if( VOS_OK != ulResult )
            {
                PS_PRINTF_ERR("APP_NDIS_FidInit, register priority fail!\n");
                return VOS_ERR;
            }
            break;
        case   VOS_IP_FARMALLOC:
        case   VOS_IP_INITIAL:
        case   VOS_IP_ENROLLMENT:
        case   VOS_IP_LOAD_DATA:
        case   VOS_IP_FETCH_DATA:
        case   VOS_IP_STARTUP:
        case   VOS_IP_RIVAL:
        case   VOS_IP_KICKOFF:
        case   VOS_IP_STANDBY:
        case   VOS_IP_BROADCAST_STATE:
        case   VOS_IP_RESTART:
        case   VOS_IP_BUTT:
            break;
        default:
            break;
    }

    return PS_SUCC;
}
/*lint +e40*/

/*****************************************************************************
 Function Name  : Ndis_MsgHook
 Discription    :
 Input          : None
 Output         : None
 Return         : None

 History:

*****************************************************************************/
VOS_UINT32 Ndis_MsgHook (VOS_UINT8 *pucData,VOS_UINT32 ulLength,
     AT_NDIS_MSG_ID_ENUM_UINT32 enMsgId)
{
/* OM融合二阶段,HOOK接口变更，入参为标准OSA消息 */

    DIAG_TraceReport((VOS_VOID *)pucData);
    return VOS_OK;

}
VOS_UINT32 g_ulGUNdisOMSwitch = PS_FALSE;
VOS_VOID GU_NDIS_OM_SWITCH_ON(VOS_VOID)
{
    g_ulGUNdisOMSwitch = PS_TRUE;
    return;
}

VOS_VOID GU_NDIS_OM_SWITCH_OFF(VOS_VOID)
{
    g_ulGUNdisOMSwitch = PS_FALSE;
    return;
}

/*======================================统计信息==============================*/
/*****************************************************************************
 函 数 名  : Ndis_ShowAppDataInfo
 功能描述  : 显示收发的业务数据信息
 输入参数  : VOS_VOID
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2009年6月16日
    修改内容   : 新生成函数
修改历史      :
  2.日    期   : 2013年1月22日
    修改内容   : DSDA

*****************************************************************************/
VOS_VOID Ndis_ShowStat(VOS_VOID)
{
    PS_PRINTF_ERR("uplink discard packets num           %d\n", g_stNdisStatStru.ulDicardUsbFrmNum);
    PS_PRINTF_ERR("uplink packets recevied from USB     %d\n", g_stNdisStatStru.ulRecvUsbPktSuccNum);
    PS_PRINTF_ERR("uplink packets sent to ADS           %d\n", g_stNdisStatStru.ulSendPktToAdsSucNum);
    PS_PRINTF_ERR("downlink discarded ADS packets       %d\n", g_stNdisStatStru.ulDicardAdsPktNum);
    PS_PRINTF_ERR("downlink received ADS succ           %d\n", g_stNdisStatStru.ulRecvAdsPktSuccNum);
    PS_PRINTF_ERR("downlink get IPV6 MAC fail           %d\n", g_stNdisStatStru.ulGetIpv6MacFailNum);
    PS_PRINTF_ERR("downlink packet diff with Rab        %d\n", g_stNdisStatStru.ulDlPktDiffRabNum);
    PS_PRINTF_ERR("downlink add MAC head fail           %d\n", g_stNdisStatStru.ulAddMacHdrFailNum);
    PS_PRINTF_ERR("downlink send packet fail            %d\n", g_stNdisStatStru.ulDlSendPktFailNum);
    PS_PRINTF_ERR("downlink send packet succ            %d\n", g_stNdisStatStru.ulDlSendPktSuccNum);

    PS_PRINTF_ERR("\nrecv DHCP                          %d\n", g_stNdisStatStru.ulRecvDhcpPktNum);
    PS_PRINTF_ERR("recv ARP Request                     %d\n", g_stNdisStatStru.ulRecvArpReq);
    PS_PRINTF_ERR("recv ARP Reply                       %d\n", g_stNdisStatStru.ulRecvArpReply);
    PS_PRINTF_ERR("proc error ARP packets               %d\n", g_stNdisStatStru.ulProcArpError);
    PS_PRINTF_ERR("send ARP Req succ                    %d\n", g_stNdisStatStru.ulSendArpReqSucc);
    PS_PRINTF_ERR("send ARP Req fail                    %d\n", g_stNdisStatStru.ulSendArpReqFail);
    PS_PRINTF_ERR("send ARP Req No Reply                %d\n", g_stNdisStatStru.ulArpReplyNotRecv);
    PS_PRINTF_ERR("send ARP Reply                       %d\n", g_stNdisStatStru.ulSendArpReply);
    PS_PRINTF_ERR("send ARP or DHCP or ND fail          %d\n", g_stNdisStatStru.ulSendArpDhcpNDFailNum);

    return;
}

/*****************************************************************************
 函 数 名  : Ndis_PrintIpAddr
 功能描述  : 打印IP地址信息
 输入参数  : VOS_VOID
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2009年6月16日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_PrintIpv4Addr(const VOS_UINT8 *pIpaddr)
{
    if ((pIpaddr[0] == 0) && (pIpaddr[1] == 0) && (pIpaddr[2] == 0) && (pIpaddr[3] == 0) )
    {
         PS_PRINTF_ERR("                   addr not config\n");
         return;
    }


    return;
}

/*****************************************************************************
 函 数 名  : Ndis_ShowValidEntity
 功能描述  : 显示有效的实体信息
 输入参数  : VOS_VOID
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年3月16日
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID Ndis_ShowValidEntity(VOS_UINT16 usModemId, VOS_UINT8 ucRabId)
{
    NDIS_ENTITY_STRU     *pstEntity;
    VOS_UINT8            ucExRabId;

    ucExRabId = NDIS_FORM_EXBID(usModemId, ucRabId);
    pstEntity  =  NDIS_GetEntityByRabId(ucExRabId);
    if(VOS_NULL_PTR == pstEntity)
    {
        PS_PRINTF_ERR("             no right NDIS entity    \n");
        return;
    }

    PS_PRINTF_ERR("                 ModemID:  %d\n", NDIS_GET_MODEMID_FROM_EXBID(pstEntity->ucRabId));
    PS_PRINTF_ERR("                 EPS bearer ID:  %d\n", NDIS_GET_BID_FROM_EXBID(pstEntity->ucRabId));
    PS_PRINTF_ERR("             ARP got flag:  %d\n", pstEntity->stIpV4Info.ulArpInitFlg);
    PS_PRINTF_ERR(" ARP recv reply flag:  %d\n", pstEntity->stIpV4Info.ulArpRepFlg);


    PS_PRINTF_ERR("\n======================================================\n");

}

/*****************************************************************************
 函 数 名  : Ndis_ShowAllEntity
 功能描述  : 显示所有的实体信息
 输入参数  : VOS_VOID
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年4月18日
    修改内容   : 新生成函数

  2.日    期   : 2013年1月16日
    修改内容   : DSDA

*****************************************************************************/
VOS_VOID Ndis_ShowAllEntity(VOS_VOID)
{
    VOS_UINT32            ulLoop;
    NDIS_ENTITY_STRU     *pstEntity = VOS_NULL_PTR;

    for(ulLoop=0; ulLoop<NAS_NDIS_MAX_ITEM; ulLoop++)
    {
        pstEntity  =  &g_pstNdisEntity[ulLoop];
        if (PS_FALSE == pstEntity->enUsed)
        {
            PS_PRINTF_ERR("                 ModemID:  %d\n", NDIS_GET_MODEMID_FROM_EXBID(pstEntity->ucRabId));
            PS_PRINTF_ERR("                 EPS bearer ID %d inactive\n", NDIS_GET_BID_FROM_EXBID(pstEntity->ucRabId));
            continue;
        }

        PS_PRINTF_ERR("                 ModemID:  %d\n", NDIS_GET_MODEMID_FROM_EXBID(pstEntity->ucRabId));
        PS_PRINTF_ERR("                 EPS bearer ID:  %d\n", NDIS_GET_BID_FROM_EXBID(pstEntity->ucRabId));
        PS_PRINTF_ERR("             ARP got flag:  %d\n", pstEntity->stIpV4Info.ulArpInitFlg);
        PS_PRINTF_ERR(" ARP recv reply flag:  %d\n", pstEntity->stIpV4Info.ulArpRepFlg);


        PS_PRINTF_ERR("\n======================================================\n");
    }
}



/*****************************************************************************
 Function Name  : NDIS_OpenLatency
 Discription    : UserPlane Latency
 Input          : None
 Output         : None
 Return         : None

 History:
*****************************************************************************/
VOS_VOID NDIS_OpenLatency(VOS_VOID)
{
    g_ulNdisLomSwitch = 1;
    return;
}

/*****************************************************************************
 Function Name  : NDIS_CloseLatency
 Discription    : UserPlane Latency
 Input          : None
 Output         : None
 Return         : None

 History:
*****************************************************************************/
VOS_VOID NDIS_CloseLatency(VOS_VOID)
{
    g_ulNdisLomSwitch = 0;
    return;
}

/*****************************************************************************
 Function Name  : Ndis_LomTraceRcvUlData
 Discription    : UserPlane Latency
 Input          : None
 Output         : None
 Return         : None

 History:
*****************************************************************************/
VOS_VOID Ndis_LomTraceRcvUlData(VOS_VOID)
{
    if (1 == g_ulNdisLomSwitch)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "User plane latency trace: NDIS rcv UL data");
    }
    return;
}

/*****************************************************************************
 Function Name  : Ndis_LomTraceRcvDlData
 Discription    : UserPlane Latency
 Input          : None
 Output         : None
 Return         : None

 History:
*****************************************************************************/
VOS_VOID Ndis_LomTraceRcvDlData(VOS_VOID)
{
    if (1 == g_ulNdisLomSwitch)
    {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "User plane latency trace: NDIS rcv DL data");
    }
    return;
}

