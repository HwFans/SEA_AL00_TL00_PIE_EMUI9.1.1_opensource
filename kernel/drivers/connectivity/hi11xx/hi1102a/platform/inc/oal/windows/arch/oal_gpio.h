

#ifndef __OAL_WINDOWS_GPIO_H__
#define __OAL_WINDOWS_GPIO_H__

/*****************************************************************************
  2 �궨��
*****************************************************************************/

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/

/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/

/*****************************************************************************
  7 STRUCT����
*****************************************************************************/

/*****************************************************************************
  8 UNION����
*****************************************************************************/

/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/*****************************************************************************
  10 ��������
*****************************************************************************/
/*
 * �� �� ��  : oal_config_gpio_pin
 * ��������  : ��������GPIO�ܽŵ�״̬
 */
OAL_STATIC OAL_INLINE oal_int32 oal_config_gpio_pin(oal_uint32 ulGPIOBaseAddr, oal_uint32 ulGPIOPinBits,
                                                    oal_uint8 ucInOut, oal_uint8 ucGPIOType)
{
    return 0;
}

/*
 * �� �� ��  : oal_set_gpio_level
 * ��������  : ���ߡ����Ͷ�ӦGPIO�ܽŵĵ�ƽ����Ҫ��֤��Ӧ��GPIOΪ��ͨGPIO����
 *             ���ж�
 * �������  : UINT32 ulGpioBit, UINT8 ucLevel
 * �������  : ��
 * �� �� ֵ  : INT32
 */
OAL_STATIC OAL_INLINE oal_int32 oal_set_gpio_level(oal_uint32 ulGPIOBaseAddr, oal_uint32 ulGpioBit, oal_uint8 ucLevel)
{
    return 0;
}

/*
 * �� �� ��  : oal_set_gpio_trigger_type
 * ��������  : ����GPIO�жϵĴ�����ʽ
 * �������  : UINT32 ulGpioBit,UINT8 ucTrigTypeLevelEdge, UINT8 ucTrigTypeHighLow
 * �������  : ��
 * �� �� ֵ  : INT32
 */
OAL_STATIC OAL_INLINE oal_int32 oal_set_gpio_trigger_type(oal_uint32 ulGPIOBaseAddr, oal_uint32 ulGpioBit,
                                                          oal_uint8 ucTrigTypeLevelEdge, oal_uint8 ucTrigTypeHighLow)
{
    return 0;
}

/*
 * �� �� ��  : oal_get_gpio_level
 * ��������  : ��ȡָ��GPIO�ܽŵĵ�ǰ��ƽ
 */
OAL_STATIC OAL_INLINE oal_int32 oal_get_gpio_level(oal_uint32 ulGPIOBaseAddr, oal_uint32 ulGpioBit, oal_uint8 *ucLevel)
{
    return 0;
}

/*
 * �� �� ��  : oal_clear_gpio_int
 * ��������  : ���ָ���ж�
 */
OAL_STATIC OAL_INLINE oal_int32 oal_clear_gpio_int(oal_uint32 ulGPIOBaseAddr, oal_uint32 ulGPIOBit)
{
    return 0;
}

/*
 * �� �� ��  : oal_debounce_gpio_int
 * ��������  : ����ָ��GPIO�ܽŵĶ���
 */
OAL_STATIC OAL_INLINE oal_int32 oal_debounce_gpio_int(oal_uint32 ulGPIOBaseAddr, oal_uint32 ulGPIOBit,
                                                      oal_uint8 ucDebounceType)
{
    return 0;
}

#endif /* end of oal_ext_if.h */