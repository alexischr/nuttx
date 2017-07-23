/****************************************************************************
 * include/nuttx/drivers/smps.h
 *
 *   Copyright (C) 2017 Gregory Nutt. All rights reserved.
 *   Author: Mateusz Szafoni <raiden00@railab.me>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_NUTTX_DRIVERS_SMPS_H
#define __INCLUDE_NUTTX_DRIVERS_SMPS_H

/*
 * The SMPS (switched-mode power supply) driver is split into two parts:
 *
 * 1) An "upper half", generic driver that provides the common SMPS interface
 *    to application level code, and
 * 2) A "lower half", platform-specific driver that implements the low-level
 *    functionality eg.:
 *      - timer controls to implement the PWM signals,
 *      - analog peripherals configuration such as ADC, DAC and comparators,
 *      - control algorithm for SMPS action (eg. PID loop)
 *
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <nuttx/fs/ioctl.h>

#ifdef CONFIG_SMPS

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* REVISIT: move to fs/ioctl.h ? */

/* General ioctl definitions ************************************************/

#define _SMPSBASE              (0x2700) /* SMPS ioctl commands */

/* SMPS IOCTLs **************************************************************/

#define _SMPSIOCVALID(c)       (_IOC_TYPE(c)==_SMPS_BASE)
#define _SMPSIOC(nr)           _IOC(_SMPSBASE,nr)

/* Ioctl Commands ************************************************************/

#define SMPSIOC_START          _SMPSIOC(1)
#define SMPSIOC_STOP           _SMPSIOC(2)
#define SMPSIOC_SET_MODE       _SMPSIOC(3)
#define SMPSIOC_SET_LIMITS     _SMPSIOC(4)
#define SMPSIOC_GET_STATE     _SMPSIOC(5)
#define SMPSIOC_GET_FAULT      _SMPSIOC(6)
#define SMPSIOC_SET_FAULT      _SMPSIOC(7)
#define SMPSIOC_CLEAN_FAULT    _SMPSIOC(8)
#define SMPSIOC_SET_PARAMS     _SMPSIOC(9)

#ifdef CONFIG_DEBUG_SMPS_ERROR
#  define smpserr(format, ...)     _err(format, ##__VA_ARGS__)
#else
#  define smpserr(x...)
#endif

#ifdef CONFIG_DEBUG_SMPS_WARN
#  define smpswarn(format, ...)   _warn(format, ##__VA_ARGS__)
#else
#  define smpswarn(x...)
#endif

#ifdef CONFIG_DEBUG_SMPS_INFO
#  define smpsinfo(format, ...)   _info(format, ##__VA_ARGS__)
#else
#  define smpsinfo(x...)
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* SMPS operation modes */

enum smps_opmode_e
{
  SMPS_OPMODE_INIT       = 0,        /* Initial mode */
  SMPS_OPMODE_CC         = 1,        /* Constant current mode */
  SMPS_OPMODE_CV         = 2,        /* Constant voltage mode */
  SMPS_OPMODE_CP         = 3         /* Constant power mode*/
};

/* SMPS state */

enum smps_state_e
{
  SMPS_STATE_INIT         = 0,       /* Initial state */
  SMPS_STATE_IDLE         = 1,       /* IDLE state */
  SMPS_STATE_RUN          = 2,       /* Run state */
  SMPS_STATE_FAULT        = 3,       /* Fault state */
  SMPS_STATE_CRITICAL     = 4        /* Critical Fault state */
};

/* SMPS fault type */

enum smps_fault_e
{
  SMPS_FAULT_OVERCURRENT  = (1 << 0),  /* Over-current Fault */
  SMPS_FAULT_OVERVOLTAGE  = (1 << 1),  /* Over-voltage Fault */
  SMPS_FAULT_OVERPOWER    = (1 << 2),  /* Over-power Fault */
  SMPS_FAULT_OVERTEMP     = (1 << 3),  /* Over-temperature Fault */
  SMPS_FAULT_INVAL_PARAM  = (1 << 4),  /* Invalid parameter */
  SMPS_FAULT_OTHER        = (1 << 5)   /* Other */
};

/* SMPS operation flags */

enum smps_opflags_s
{
  SMPS_FLAG_NOLOAD        = (1 << 0),  /* No load */
  SMPS_FLAG_SHARED_LOAD   = (1 << 1),  /* Shared Load mode */
  SMPS_FLAG_BURST_MODE    = (1 << 2)   /* Burst mode */
};

/* This structure contain feedback data from converter */

struct smps_feedback_s
{
#ifdef CONFIG_SMPS_HAVE_OUTPUT_VOLTAGE
  float v_out;                       /* Output Voltage */
#endif
#ifdef CONFIG_SMPS_HAVE_INPUT_CURRENT
  float i_in;                        /* Input Current */
#endif
#ifdef CONFIG_SMPS_HAVE_INPUT_VOLTAGE
  float v_in;                        /* Input Voltage */
#endif
#ifdef CONFIG_SMPS_HAVE_OUTPUT_CURRENT
  float i_out;                       /* Output Current */
#endif
#ifdef CONFIG_SMPS_HAVE_INPUT_POWER
  float p_in;                        /* Input Power */
#endif
#ifdef CONFIG_SMPS_HAVE_OUTPUT_POWER
  float p_out;                       /* Output Power */
#endif
#ifdef CONFIG_SMPS_HAVE_EFFICIENCY
  float eff;                         /* Converter Efficiency */
#endif
};

/* This structure describes converter state */

struct smps_state_s
{
  uint8_t                state;      /* SMPS state  */
  uint8_t                fault;      /* SMPS faults state */
  struct smps_feedback_s fb;         /* Feedback from SMPS */
};

/* SMPS absolute limits. Exceeding this limits should cause critical error */

struct smps_limits_s
{
  bool  lock;                          /*  */
  float v_in;                         /* Maximum input voltage */
  float v_out;                        /* Maximum output voltage */
  float i_in;                         /* Maximum input current */
  float i_out;                        /* Maximum output current */
  float p_in;                         /* Maximum input power */
  float p_out;                        /* Maximum output power */
};

/* SMPS parameters */

struct smps_params_s
{
  bool  lock;
  float v_out;                        /*  */
  float i_out;                        /*  */
  float p_out;                        /*  */
};

/* SMPS private data strucutre  */

struct smps_s
{
  uint8_t                    opmode;  /* SMPS operation mode */
  uint8_t                    opflags; /* SMPS operation flags */
  const struct smps_limits_s limits;  /* SMPS absolute limits */
  struct smps_params_s       param;   /* SMPS settings */
  struct smps_state_s        state;   /* SMPS state */
};

/* SMPS operations used to call from the upper-half, generic SMPS driver
 * into lower-half, platform-specific logic.
 */

struct smps_dev_s;
struct smps_ops_s
{
  /* Configure SMPS */

  CODE int (*setup)(FAR struct smps_dev_s *dev);

  /* Disable converter action */

  CODE int (*shutdown)(FAR struct smps_dev_s *dev);

  /* Stop SMPS action */

  CODE int (*stop)(FAR struct smps_dev_s *dev);

  /* Start SMPS action */

  CODE int (*start)(FAR struct smps_dev_s *dev);

  /* Set SMPS parameters */

  CODE int (*params_set)(FAR struct smps_dev_s *dev,
                         FAR struct smps_params_s *param);

  /* Set SMPS operation mode */

  CODE int (*mode_set)(FAR struct smps_dev_s *dev, uint8_t mode);

  /* Set SMPS limts */

  CODE int (*limits_set)(FAR struct smps_dev_s *dev,
                         FAR struct smps_limits_s *limits);

  /* Set SMPS fault */

  CODE int (*fault_set)(FAR struct smps_dev_s *dev, uint8_t fault);

  /* Get SMPS state  */

  CODE int (*state_get)(FAR struct smps_dev_s *dev,
                         FAR struct smps_state_s *state);

  /* Get current fault state */

  CODE int (*fault_get)(FAR struct smps_dev_s *dev, FAR uint8_t *fault);

  /* Clean fault state */

  CODE int (*fault_clean)(FAR struct smps_dev_s *dev, uint8_t fault);

  /* Lower-half logic may support platform-specific ioctl commands */

  CODE int (*ioctl)(FAR struct smps_dev_s *dev, int cmd, unsigned long arg);
};

/* SMPS device structure used by the driver. The caller of smps_register
 * must allocate and initialize this structure. The calling logic need
 * provide 'ops', 'priv' and 'lower' elements.
 */

struct smps_dev_s
{
  /* Fields managed by common upper half SMPS logic */

  uint8_t                     ocount;   /* The number of times the device
                                         * has been opened
                                         */
  sem_t                       closesem; /* Locks out new opens while close
                                         * is in progress
                                         */

  /* Fields provided by lower half SMPS logic */

  FAR const struct smps_ops_s *ops;     /* Arch-specific operations */
  FAR void                    *priv;    /* Reference to SMPS private data */
  FAR void                    *lower;   /* Reference to lower level drivers */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Name: smps_register
 ****************************************************************************/

int smps_register(FAR const char *path, FAR struct smps_dev_s *dev,
                  FAR void *lower);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* CONFIG_SMPS */
#endif /* __INCLUDE_NUTTX_DRIVERS_SMPS_H */
