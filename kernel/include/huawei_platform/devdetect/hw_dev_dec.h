/*
 * hw_dev_dec.h
 *
 * Device detect type header file.
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef KERNEL_INCLUDE_DEV_DEC_H
#define KERNEL_INCLUDE_DEV_DEC_H

/* hw device list */
enum hw_device_type {
	DEV_I2C_START,
	DEV_I2C_TOUCH_PANEL = DEV_I2C_START,        /* id: 0 order number: 1 */
	DEV_I2C_COMPASS,                                /* 1 */
	DEV_I2C_G_SENSOR,                               /* 2 */
	DEV_I2C_CAMERA_MAIN,                            /* 3 */
	DEV_I2C_CAMERA_SLAVE,                           /* 4 */
	DEV_I2C_KEYPAD,                                 /* 5 */
	DEV_I2C_APS,                                    /* 6 */
	DEV_I2C_GYROSCOPE,                              /* 7 */
	DEV_I2C_NFC,                                    /* 8 */
	DEV_I2C_DC_DC,                                  /* 9 */
	DEV_I2C_SPEAKER,                                /* 10 */
	DEV_I2C_OFN,                                    /* 11 */
	DEV_I2C_TPS,                                    /* 12 */
	DEV_I2C_L_SENSOR,                               /* 13 */
	DEV_I2C_CHARGER,                                /* 14 */
	DEV_I2C_BATTERY,                                /* 15 */
	DEV_I2C_NCT,                                    /* 16 */
	DEV_I2C_MHL,                                    /* 17 */
	DEV_I2C_AUDIENCE,                               /* 18 */
	DEV_I2C_IRDA,                                   /* 19 */
	DEV_I2C_CS,                                     /* 20 */
	DEV_I2C_USB_SWITCH,                             /* 21 */
	DEV_I2C_PMU_DCDC,                               /* 22 */
	DEV_SPI_FPGA,                                   /* 23 */
	DEV_I2C_CPU_CHIP,                               /* 24 */
	DEV_I2C_AIRPRESS,                               /* 25 */
	DEV_I2C_HANDPRESS,                              /* 26 */
	DEV_I2C_FFLASH,                                 /* 27 */
	DEV_I2C_VIBRATOR_LRA,                           /* 28 */
	DEV_I2C_TYPEC,                                  /* 29 */
	DEV_I2C_ANC_MAX14744,                           /* 30 */
	DEV_I2C_LASER,                                  /* 31 */
	DEV_I2C_CAMERA_PMIC,                            /* 32 */
	DEV_I2C_LOADSWITCH,                             /* 33 */
	DEV_I2C_BUCKBOOST,                              /* 34 */
	DEV_I2C_COUL,                                   /* 35 */
	DEV_I2C_DTV,                                    /* 36 */
	DEV_I2C_AP_COLOR_SENSOR,                        /* 37 */
	DEV_I2C_SWITCHCAP,                              /* 38 */
	DEV_I2C_STEP_HALL,                              /* 39 */
	DEV_I2C_BUCK,                                   /* 40 */
	DEV_I2C_MAX,                                    /* 41 */
	DEV_CONNECTIVITY_START,
	DEV_CONNECTIVITY_WIFI = DEV_CONNECTIVITY_START, /* 42 */
	DEV_CONNECTIVITY_BT,                            /* 43 */
	DEV_CONNECTIVITY_FM,                            /* 44 */
	DEV_CONNECTIVITY_GPS,                           /* 45 */
	DEV_CONNECTIVITY_GPU,                           /* 46 */
	DEV_CONNECTIVITY_MAX,                           /* 47 */
};

/* set a device flag as true */
int set_hw_dev_flag(int dev_id);

#endif
