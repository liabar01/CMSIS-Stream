/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        xxx.c
 * Description:  
 *
 * $Date:        15 February 2023
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2023 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
* 
 */
#ifndef cSTREAM_PLATFORM_H
#define cSTREAM_PLATFORM_H


#include "stream_const.h"

#define U(x) ((uint32_t)(x))
#define U8(x) ((uint8_t)(x))

//enum platform_al_services       
#define PLATFORM_PROC_ID            1u   /* who am i ? */
#define PLATFORM_MP_GRAPH_SHARED    2u   /* need to declare the graph area as "sharable" in S = MPU_RASR[18] */
#define PLATFORM_MP_BOOT_SYNCHRO    3u   /* collission of access to the graph at boot time */
#define PLATFORM_MP_BOOT_WAIT       4u   /* wait master processor copies the graph */
#define PLATFORM_MP_BOOT_DONE       5u   /* to confirm the graph was copied in RAM */
#define PLATFORM_MP_RESET_WAIT      6u   /* wait the graph is initialized */
#define PLATFORM_MP_RESET_DONE      7u   /* tell the reset sequence was executed for that Stream instance */
#define PLATFORM_IO_SET_STREAM_ALL  8u   /* launch all the graph interfaces */
#define PLATFORM_IO_SET_STREAM      9u   /* share &platform_io(), buffer address, *selection of setting EXTENSION/option, data format */
#define PLATFORM_IO_DATA           10u   /* "data exchanges */ 
#define PLATFORM_IO_STOP_STREAM    11u   /*  */
#define PLATFORM_IO_ACK            12u   /* interface callback to arm_stream_io */
#define PLATFORM_BACKUP_MEM        13u   /* share the list of memory areas to backup periodically */
#define PLATFORM_RESTORE_MEM       14u   /* restore memory areas before warm boot */
#define PLATFORM_EXEC_TIME         15u   /* time counter since last call */
#define PLATFORM_ERROR             16u   /* error to report to the application */
#define PLATFORM_TIME_SET          17u
#define PLATFORM_TIMER_SET         18u
#define PLATFORM_TIME_READ         19u
#define PLATFORM_OFFSETS           20u


//enum error_codes 
#define ERROR_MEMORY_ALLOCATION     1u



////enum io_buffer_allocation => duplicates with the information "IOCOMMAND_IOFMT" 
//#define ALLOC_FROM_APPLICATION      0u
//#define ALLOC_FROM_IO               1u
////enum io_master_follower
//#define IO_IS_MASTER                0u
//#define IO_IS_FOLLOWER              1u

///*---- Platform Sensor configuration : io_domain ----*/
//#define     UNUSED_IOMEM_MSB        31u 
//#define     UNUSED_IOMEM_LSB        28u /* 4   */
//#define  NOTISOCHRONOUS_IOMEM_MSB   27u /*    used to size the FIFO:  FRAMESIZE_FMT0 x SAMPLING_FMT1 */
//#define  NOTISOCHRONOUS_IOMEM_LSB   27u /* 1  1:bursty asynchronous : frame size and FS give the peak data-rate */
//#define            ARCH_IOMEM_MSB   26u 
//#define            ARCH_IOMEM_LSB   23u /* 4  arch bit-field usable 0=any for the list of platform_manifest[arch] */
//#define            PROC_IOMEM_MSB   22u 
//#define            PROC_IOMEM_LSB   15u /* 8  locks this IO to proc (among 8), 0=any from this architecture */
//#define   MEM_SPACE_OFF_IOMEM_MSB   14u  
//#define   MEM_SPACE_OFF_IOMEM_LSB   12u /* 3  64bits offset selection : 0/internal/external/io = idx_memory_base_offset */
//#define  BUFFER_FROM_HW_IOMEM_MSB   11u /*    "io_buffer_allocation"  TO REMOVE */
//#define  BUFFER_FROM_HW_IOMEM_LSB   11u /* 1  Buffer is provided by the driver = 1, data pointer is provided by the application at set-up = 0 */
//#define MASTER_FOLLOWER_IOMEM_MSB   10u /*    "io_master_follower" */
//#define MASTER_FOLLOWER_IOMEM_LSB   10u /* 1  Master=0 Follower=1  */
//#define PLATFORM_IO_IDX_IOMEM_MSB    9u /*    16 io_index of the same domain (16 GPIO, 16 ADC, ..) */  
//#define PLATFORM_IO_IDX_IOMEM_LSB    6u /* 4  platform_io_idx from domains below */
//#define          DOMAIN_IOMEM_MSB    5u
//#define          DOMAIN_IOMEM_LSB    0u /* 6  64 physical domains = enum  stream_io_domain */
//
//#define PACK_IOMEMDOMAIN(ARCH,PROC,OFFSET,HWBUFFER,MASTER,IOIDX,DOMAIN) \
//    (((ARCH)<<23)|((PROC)<<15)|((OFFSET)<<12)|((HWBUFFER)<<11)|((MASTER)<<10)|((IOIDX)<<6)|((DOMAIN)<<0))



/*
    STREAM SERVICES
*/

enum stream_command 
{
    /* the 4 LSB are used for the command type selection */
    STREAM_COMMAND_SELECT = 0x0000000F,
#define STREAM_COMMAND_MASK 0x0000FFF0

    /* Commands LSB4 = 0 ------------------------------------------------------------------- */
    STREAM_CONTROL = 0x00000010,           

 
    //STREAM_SET_BUFFER_NOTIFICATION,

    STREAM_DEBUG_TRACE, STREAM_DEBUG_TRACE_1B, STREAM_DEBUG_TRACE_STRING,
    STREAM_DEBUG_TRACE_STAMPS,
    STREAM_DEBUG_ARC_CALLBACK,

    STREAM_SAVE_HOT_PARAMETER, STREAM_DATA_FETCH, STREAM_CHECK_FETCH_COMPLETION,
    /* dummy access to pre-load the cache when the algorithm is working somewhere else 
        see draw_horiz_band() in FFMPEG */
    /* Invalidate = flush  => clear valid bit to allow $line to be reused (no WB in memory)
       Clean = force dirty D-$ line to be written to main memory (coherence between D-$ & mem) */

    STREAM_READ_TIME, STREAM_READ_TIME_FROM_START, STREAM_TIME_DIFFERENCE, 
    STREAM_TIME_CONVERSION,   /* linux time format conversion, ..  */

    //STREAM_LOW_POWER,     /* interface to low-power platform settings, "wake-me in 24h with deep-sleep in-between" */
    //STREAM_URGENT_TASKS,  /* from 1ms SYSTICK : call platform_io "get/put next" data stream */
    //STREAM_PLL_SETTING,   /* tell the scheduler the processor's PLL was changed */


    /* STREAM_STANDARD_LIBRARIES LSB4 = 1 ------------------------------------------------------------------- */
    STREAM_STANDARD_LIBRARIES = 0x00000011,           

    /* stdio.h */
    STREAM_FEOF, STREAM_FERROR, STREAM_FFLUSH, STREAM_FGETC, STREAM_FGETS, STREAM_FOPEN, STREAM_FPRINTF, STREAM_FPUTC,
    STREAM_FPUTS, STREAM_FREAD, STREAM_FSCANF, STREAM_FWRITE,

    /* stdlib.h */
    STREAM_ABS, STREAM_ATOF, STREAM_ATOI, STREAM_ATOL, STREAM_ATOLL, STREAM_CALLOC, STREAM_FREE, STREAM_LABS,
    STREAM_LDIV, STREAM_LLABS, STREAM_LLDIV, STREAM_MALLOC, STREAM_RAND, STREAM_REALLOC, STREAM_SRAND, STREAM_STRTOD,
    STREAM_STRTOF, STREAM_STRTOL, STREAM_STRTOLD, STREAM_STRTOLL, STREAM_STRTOUL, STREAM_STRTOULL,

    /* string.h */
    STREAM_MEMCHR, STREAM_MEMCMP, STREAM_MEMCPY, STREAM_MEMMOVE, STREAM_MEMSET, STREAM_STRCHR, STREAM_STRLEN,
    STREAM_STRNCAT, STREAM_STRNCMP, STREAM_STRNCPY, STREAM_STRSTR, STREAM_STRTOK,

    /* STREAM_MATH_LIBRARIES LSB4 = 2 ------------------------------------------------------------------- */
    STREAM_MATH_LIBRARIES = 0x00000012,           

    /* math.h */
    STREAM_SIN_FP16,  STREAM_COS_FP16,   STREAM_SIN_FP32,  STREAM_COS_FP32,   STREAM_SIN_FP64,  STREAM_COS_FP64, 
    STREAM_ASIN_FP16, STREAM_ACOS_FP16,  STREAM_ASIN_FP32, STREAM_ACOS_FP32,  STREAM_ASIN_FP64, STREAM_ACOS_FP64, 
    STREAM_TAN_FP16,                   STREAM_TAN_FP32,                   STREAM_TAN_FP64,  
    STREAM_ATAN_FP16, STREAM_ATAN2_FP16, STREAM_ATAN_FP32, STREAM_ATAN2_FP32, STREAM_ATAN_FP64, STREAM_ATAN2_FP64,  
    STREAM_LOG10_FP16,STREAM_LOG2_FP16,  STREAM_LOG10_FP32,STREAM_LOG2_FP32,  STREAM_LOG10_FP64,STREAM_LOG2_FP64, 
    STREAM_POW_FP16,  STREAM_SQRT_FP16,  STREAM_POW_FP32,  STREAM_SQRT_FP32,  STREAM_POW_FP64,  STREAM_SQRT_FP64, 

    /* CMSIS-DSP/ML */
    STREAM_SET_ACCURACY, STREAM_SET_MEMORY_CONSTRAINT, STREAM_FREE_COMPUTE_INSTANCE, 

    //STREAM_CONJ,                          /* Conjugate */
    STREAM_RFFT, STREAM_CFFT, STREAM_CIFFT,   /* FFT - there is a state and associated instance common for fft and ifft */

    //STREAM_RADD, STREAM_CADD,
    //STREAM_RSUB, STREAM_CSUB,
    //STREAM_RMUL, STREAM_CMUL,               /* element-wise real/complex vector multiply, with conjugate */
    //STREAM_RDIV, STREAM_CDIV,     
    //STREAM_RABS,                          /* absolute values of real vectors, norm-1/2/Inf of complex numbers */
    //STREAM_MAX,
    //STREAM_CNORM,                         
    //STREAM_RMAX,                          /* max between two vectors */

    //STREAM_WINDOWS,                       /* windowing */
    //STREAM_BIQUAD_CASCADE,                /* filters - there a state and associated instance */
    //STREAM_FIR,
    //STREAM_MATRIX_MULT,
    //STREAM_2D_FILTER, 
    //STREAM_ASRC, 
    //STREAM_2D_DECIMATE, 

    //STREAM_APPLY_GAIN, 
    //STREAM_MATH_TABLE, 
    //STREAM_RATE_CONVERTER, 
    //STREAM_MIXER, 


    /* STREAM_MULTIMEDIA_LIBRARIES LSB4 = 3 ------------------------------------------------------------------- */
    STREAM_MULTIMEDIA_LIBRARIES = 0x00000013,           

    /* audio Codecs */
    STREAM_ALAW_ENC, STREAM_ALAW_DEC, STREAM_MULAW_ENC, STREAM_MULAW_DEC, STREAM_IMADPCM_ENC, STREAM_IMADPCM_DEC,
    STREAM_LPC_ENC, STREAM_LPC_DEC,

    /* image                   */
    STREAM_JPEG_ENC, STREAM_JPEG_DEC, STREAM_PNG_ENC, STREAM_PNG_DEC,

    /* STREAM_APPLICATION_LIBRARIES LSB4 = 4 ------------------------------------------------------------------- */
    STREAM_APPLICATION_LIBRARIES = 0x00000014,

    STREAM_RAWDATA_FORMAT_CONVERSION,
    STREAM_RFC8428_CONVERTER,

    STREAM_SECURITY_LIBRARY, 
    STREAM_TEA,
};
#endif /* #ifndef cSTREAM_PLATFORM_H */
/*
 * -----------------------------------------------------------------------
 */
