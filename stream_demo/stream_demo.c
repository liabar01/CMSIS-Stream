/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_demo.c
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

#include "stream_const.h"

#include "stream_types.h"      /* all non preprocessor directives */
#include "platform_const.h"

/*----------------------------------------------------------*/

intPtr_t pack2linaddr_int(uint32_t data, intPtr_t *offsets)
{
    intPtr_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    uint32_t tmp0;
    tmp0 = EXTRACT_FIELD(data, BASEIDX_ARCW0);      tmp1 = (intPtr_t)tmp0;
    tmp0 = EXTRACT_FIELD(data, BASESHIFT_ARCW0);    tmp2 = (intPtr_t)tmp0;
    tmp0 = EXTRACT_FIELD(data, DATAOFF_ARCW0);      tmp3 = (intPtr_t)tmp0;

    tmp4 = tmp1 << (tmp2 << U(2));
    tmp5 = *(offsets + tmp3);
    tmp6 = tmp4 + tmp5;

    return tmp6;
}

intPtr_t * pack2linaddr_ptr(uint32_t data, intPtr_t *offsets)
{
    return convert_int_to_voidp (pack2linaddr_int(data, offsets));
}

/*---------------------------------------------------------------------------
 * Application execution
 *---------------------------------------------------------------------------*/
void stream_demo_init(uint8_t stream_instance, uint8_t total_nb_stream_instance,
            const uint32_t *graph, 
            uint32_t graph_size,
            stream_parameters_t *parameters
            )
{
    uint32_t nio;
    uint32_t iio; 
    uint32_t fw_idx;
    struct stream_local_instance *pinst;
    uint32_t *pio;
    uint32_t stream_format_io;
    struct platform_control_stream set_stream;
    uint32_t io_mask;
    uint32_t iarc; 
    uint32_t *all_arcs; 
    uint32_t *graph_src;
    uint32_t *graph_dst; 
    uint32_t graph_len_in_ram; 
    uint32_t graph_bytes_in_ram;
    uint8_t procID;
    uint8_t archID;
    uint8_t copy_to_do;
    intPtr_t *long_offset;

    /* 
        start copying the graph in RAM 
        initialize the current Stream instance (->node_entry_points, ->graph)
        initialize the IO boundaries of the graph
    */

    /* read the processors' long_offset table */
    platform_al (PLATFORM_OFFSETS,&long_offset,0,0);

    /* read the graph memory mapping configuration (copy in RAM, partial copy, no copy) */
    parameters->graph = graph;

    graph_src = graph;
    graph_dst = graph;

    switch (RD(graph,COPY_CONF_GRAPH0))
    {
    case COPY_CONF_GRAPH0_COPY_ALL_IN_RAM:
        graph_bytes_in_ram = graph_size;
        break;

    case COPY_CONF_GRAPH0_COPY_PARTIALLY:
        { void *x = MEMCPY((char *)graph_dst, (char *)graph, graph_size); }             
        break;

    case COPY_CONF_GRAPH0_ALREADY_IN_RAM:
    default:
        graph_bytes_in_ram = 0;
        break;
    }

    { void *x = MEMCPY((char *)graph_dst, (char *)graph_src, graph_bytes_in_ram); }             


    parameters->graph = GRAPH_ADDR_RAM(graph,long_offset);
    parameters->instance_control = stream_instance;    

    //TEST_BIT(ZZ,SELECT_BASE_ARCW0);

    /* read the instance long_offset table */
    pinst = (struct stream_local_instance *)GRAPH_STREAM_INST_ADDR_FLASH(graph);
    pinst = &(pinst[stream_instance]);
    platform_al (PLATFORM_OFFSETS,long_offset,0,0);

    /* if I am the master processor (archID=1 + procID=0), copy the graph, or wait */
    platform_al (PLATFORM_PROC_ID, &procID, &archID,0);
    if ((archID == 1U) && (procID == 0U))
    {
        platform_al (PLATFORM_MP_BOOT_SYNCHRO, &stream_instance, &copy_to_do, 0);
        if (0 != copy_to_do)
        {
            /* 
                check if only a portion of the graph is copied in RAM 

            */
            if (0 != GRAPH_PARTIALLY_COPIED_IN_RAM(graph))
            {   
                /* the second word gives the target address of the graph 
                which is the split section or the full graph */
                graph_bytes_in_ram = graph_size - (graph[0]<<2);
                graph_src = GRAPH_STREAM_INST_ADDR_FLASH(graph);
                graph_dst = (uint32_t *)GRAPH_STREAM_INST_ADDR_RAM_p(graph,long_offset);
            }
            else
            {
                /* copy from Flash to RAM, "graph" points to the RAM address */
                graph_bytes_in_ram = graph_size;
                graph_src = graph;
                graph_dst = (uint32_t *)GRAPH_ADDR_RAM(graph,long_offset);
            }
            
            { void *x = MEMCPY((char *)graph_dst, (char *)graph_src, graph_bytes_in_ram); }

            /* this graph memory area is shareable */
            platform_al (PLATFORM_MP_GRAPH_SHARED, (uint8_t *)graph,(uint8_t *)&(graph[graph_bytes_in_ram]),0);
            platform_al (PLATFORM_MP_BOOT_DONE,0,0,0);
        }
    }

    /* I wait the graph is copied in RAM */
    {   uint8_t wait; 
        do {
            platform_al (PLATFORM_MP_BOOT_WAIT, &wait, 0,0);
        } while (0u != wait);
    }



    /*-------------------------------------------*/

    /* local stream instance initialization */
    pinst = (struct stream_local_instance *)GRAPH_STREAM_INST_ADDR_RAM(graph, long_offset);
    pinst = &(pinst[stream_instance]);
    pinst->node_entry_points = node_entry_point_table;
    //pinst->platform_al = platform_al;
    pinst->graph = graph;
    pinst->whoami_ports = PACKWHOAMI(stream_instance,procID,archID,0x003); // scan 2 io ports

    /* 
        initialization of all the SWC 
    */
    arm_stream(STREAM_RESET, parameters,0,0); 

    platform_al (PLATFORM_MP_RESET_DONE, &stream_instance,0,0);

    /* wait all the process have initialized the graph */
    {   uint8_t wait; 
        do {
            platform_al (PLATFORM_MP_RESET_WAIT, &wait, &total_nb_stream_instance, 0);
        } while (wait);
    }

    /* 
        initialization of the graph IO ports 
    */     
    io_mask = EXTRACT_FIELD(pinst->whoami_ports, BOUNDARY_PARCH);
    pio = GRAPH_IO_CONFIG_ADDR_FLASH(graph);
    nio = GRAPH_NB_IO(graph);
    all_arcs = GRAPH_ARC_LIST_ADDR_RAM(graph,long_offset);


    for (iio = 0; iio < nio; iio++)
    {
        stream_format_io = *pio++;

        /* does this port is managed by the Stream instance ? */
        if (0 == (io_mask & (1U << iio))) 
            continue; 

        fw_idx = EXTRACT_FIELD(stream_format_io, FW_IO_IDX_IOFMT);
        set_stream.fw_idx = fw_idx;

        /* default value settings */
        set_stream.domain_settings[0] = 0;
        set_stream.domain_settings[1] = 0;

        /* the graph compiler uses the platform manifest to know the need for each IO 
           to receive from Stream an allocated memory buffers, and its arc descriptor.
           When the processing is in-place, this arc descriptor is not associated to a 
           buffer.
           */
        if (0 == TEST_BIT(stream_format_io, BUFFER_HW_IOFMT_LSB))
        {
            iarc = SIZEOF_ARCDESC_W32 * RD(stream_format_io, IOARCID_IOFMT);
            set_stream.buffer.address = 
                (uint8_t *)pack2linaddr_ptr(all_arcs[iarc + BUF_PTR_ARCW0],long_offset);
            set_stream.buffer.size = 
                (uint32_t)RD(all_arcs[iarc + BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);
        }
        /* the Stream instance holds the memory offsets */
        set_stream.stream_instance = pinst;
        platform_al (PLATFORM_IO_SET_STREAM, (uint8_t *)&set_stream, 0, 0);
    }

    /* release the stack before graph execution */
}

/*--------------------------------------------------------------------------- */
 