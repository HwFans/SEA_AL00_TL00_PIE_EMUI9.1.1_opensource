
#ifndef __EAGLEYE_H__
#define __EAGLEYE_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#ifdef __EAGLEYE__
extern void Coverity_Tainted_Set(void* pkt);
#define COVERITY_TAINTED_SET(pkt) { \
    Coverity_Tainted_Set(pkt); \
}
#else
#define COVERITY_TAINTED_SET(pkt)
#endif

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

/*****************************************************************************
  4 ��Ϣͷ����
*****************************************************************************/

/*****************************************************************************
  5 ��Ϣ����
*****************************************************************************/

/*****************************************************************************
  6 STRUCT����
*****************************************************************************/

/*****************************************************************************
  7 UNION����
*****************************************************************************/

/*****************************************************************************
  8 OTHERS����
*****************************************************************************/

/*****************************************************************************
  9 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  10 ��������
*****************************************************************************/

#endif /* end eagleye */



