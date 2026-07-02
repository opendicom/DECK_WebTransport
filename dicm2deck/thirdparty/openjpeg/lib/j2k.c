#include "opj_includes.h"
#include "dckvtypes.h"


#pragma mark - const and struct

typedef struct j2k_prog_order {
    OPJ_PROG_ORDER enum_prog;
    char str_prog[5];
} j2k_prog_order_t;



static const j2k_prog_order_t j2k_prog_order_list[] = {
    {OPJ_CPRL, "CPRL"},
    {OPJ_LRCP, "LRCP"},
    {OPJ_PCRL, "PCRL"},
    {OPJ_RLCP, "RLCP"},
    {OPJ_RPCL, "RPCL"},
    {(OPJ_PROG_ORDER) - 1, ""}
};

/**
 * FIXME DOC
 */
static const OPJ_UINT32 MCT_ELEMENT_SIZE [] = {
    2,
    4,
    4,
    8
};

static void  opj_j2k_read_int16_to_float(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);
static void  opj_j2k_read_int32_to_float(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);
static void  opj_j2k_read_float32_to_float(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);
static void  opj_j2k_read_float64_to_float(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);

static void  opj_j2k_read_int16_to_int32(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);
static void  opj_j2k_read_int32_to_int32(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);
static void  opj_j2k_read_float32_to_int32(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);
static void  opj_j2k_read_float64_to_int32(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);

static void  opj_j2k_write_float_to_int16(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);
static void  opj_j2k_write_float_to_int32(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);
static void  opj_j2k_write_float_to_float(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);
static void  opj_j2k_write_float_to_float64(const void * p_src_data,
        void * p_dest_data, OPJ_UINT32 p_nb_elem);

typedef void (* opj_j2k_mct_function)(const void * p_src_data,
                                      void * p_dest_data, OPJ_UINT32 p_nb_elem);

static const opj_j2k_mct_function j2k_mct_read_functions_to_float [] = {
    opj_j2k_read_int16_to_float,
    opj_j2k_read_int32_to_float,
    opj_j2k_read_float32_to_float,
    opj_j2k_read_float64_to_float
};

static const opj_j2k_mct_function j2k_mct_read_functions_to_int32 [] = {
    opj_j2k_read_int16_to_int32,
    opj_j2k_read_int32_to_int32,
    opj_j2k_read_float32_to_int32,
    opj_j2k_read_float64_to_int32
};

static const opj_j2k_mct_function j2k_mct_write_functions_from_float [] = {
    opj_j2k_write_float_to_int16,
    opj_j2k_write_float_to_int32,
    opj_j2k_write_float_to_float,
    opj_j2k_write_float_to_float64
};

typedef struct opj_dec_memory_marker_handler {
    /** marker value */
    OPJ_UINT32 id;
    /** value of the state when the marker can appear */
    OPJ_UINT32 states;
    /** action linked to the marker */
    OPJ_BOOL(*handler)(opj_j2k_t *p_j2k,
                       OPJ_BYTE * p_header_data,
                       OPJ_UINT32 p_header_size,
                       opj_event_mgr_t * p_manager);
}
opj_dec_memory_marker_handler_t;



#pragma mark - generic read/write copies (const void * p_src_data,void * p_dest_data,OPJ_UINT32 p_nb_elem)

static void opj_j2k_update_tlm(opj_j2k_t * p_j2k, OPJ_UINT32 p_tile_part_size)
{
    if (p_j2k->m_specific_param.m_encoder.m_Ttlmi_is_byte)
    {
        opj_write_bytes(p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current,p_j2k->m_current_tile_number, 1);
        p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current += 1;
    }
    else
    {
        opj_write_bytes(p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current,p_j2k->m_current_tile_number, 2);
        p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current += 2;
    }
    opj_write_bytes(p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current,p_tile_part_size, 4);/* PSOT */
    p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current += 4;
}

static void  opj_j2k_read_int16_to_float(const void * p_src_data,void * p_dest_data,OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_src_data = (OPJ_BYTE *) p_src_data;
    OPJ_FLOAT32 * l_dest_data = (OPJ_FLOAT32 *) p_dest_data;
    OPJ_UINT32 i;
    OPJ_UINT32 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        opj_read_bytes(l_src_data, &l_temp, 2);
        l_src_data += sizeof(OPJ_INT16);
        *(l_dest_data++) = (OPJ_FLOAT32) l_temp;
    }
}

static void  opj_j2k_read_int32_to_float(const void * p_src_data,void * p_dest_data,OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_src_data = (OPJ_BYTE *) p_src_data;
    OPJ_FLOAT32 * l_dest_data = (OPJ_FLOAT32 *) p_dest_data;
    OPJ_UINT32 i;
    OPJ_UINT32 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        opj_read_bytes(l_src_data, &l_temp, 4);
        l_src_data += sizeof(OPJ_INT32);
        *(l_dest_data++) = (OPJ_FLOAT32) l_temp;
    }
}

static void  opj_j2k_read_float32_to_float(const void * p_src_data,void * p_dest_data,OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_src_data = (OPJ_BYTE *) p_src_data;
    OPJ_FLOAT32 * l_dest_data = (OPJ_FLOAT32 *) p_dest_data;
    OPJ_UINT32 i;
    OPJ_FLOAT32 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        opj_read_float(l_src_data, &l_temp);
        l_src_data += sizeof(OPJ_FLOAT32);
        *(l_dest_data++) = l_temp;
    }
}

static void  opj_j2k_read_float64_to_float(const void * p_src_data,void * p_dest_data,OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_src_data = (OPJ_BYTE *) p_src_data;
    OPJ_FLOAT32 * l_dest_data = (OPJ_FLOAT32 *) p_dest_data;
    OPJ_UINT32 i;
    OPJ_FLOAT64 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        opj_read_double(l_src_data, &l_temp);
        l_src_data += sizeof(OPJ_FLOAT64);
        *(l_dest_data++) = (OPJ_FLOAT32) l_temp;
    }
}

static void  opj_j2k_read_int16_to_int32(const void * p_src_data,void * p_dest_data,OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_src_data = (OPJ_BYTE *) p_src_data;
    OPJ_INT32 * l_dest_data = (OPJ_INT32 *) p_dest_data;
    OPJ_UINT32 i;
    OPJ_UINT32 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        opj_read_bytes(l_src_data, &l_temp, 2);
        l_src_data += sizeof(OPJ_INT16);
        *(l_dest_data++) = (OPJ_INT32) l_temp;
    }
}

static void  opj_j2k_read_int32_to_int32(const void * p_src_data,void * p_dest_data, OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_src_data = (OPJ_BYTE *) p_src_data;
    OPJ_INT32 * l_dest_data = (OPJ_INT32 *) p_dest_data;
    OPJ_UINT32 i;
    OPJ_UINT32 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        opj_read_bytes(l_src_data, &l_temp, 4);
        l_src_data += sizeof(OPJ_INT32);
        *(l_dest_data++) = (OPJ_INT32) l_temp;
    }
}

static void  opj_j2k_read_float32_to_int32(const void * p_src_data,void * p_dest_data, OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_src_data = (OPJ_BYTE *) p_src_data;
    OPJ_INT32 * l_dest_data = (OPJ_INT32 *) p_dest_data;
    OPJ_UINT32 i;
    OPJ_FLOAT32 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        opj_read_float(l_src_data, &l_temp);
        l_src_data += sizeof(OPJ_FLOAT32);
        *(l_dest_data++) = (OPJ_INT32) l_temp;
    }
}

static void  opj_j2k_read_float64_to_int32(const void * p_src_data,void * p_dest_data, OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_src_data = (OPJ_BYTE *) p_src_data;
    OPJ_INT32 * l_dest_data = (OPJ_INT32 *) p_dest_data;
    OPJ_UINT32 i;
    OPJ_FLOAT64 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        opj_read_double(l_src_data, &l_temp);
        l_src_data += sizeof(OPJ_FLOAT64);
        *(l_dest_data++) = (OPJ_INT32) l_temp;
    }
}

static void  opj_j2k_write_float_to_int16(const void * p_src_data,void * p_dest_data, OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_dest_data = (OPJ_BYTE *) p_dest_data;
    OPJ_FLOAT32 * l_src_data = (OPJ_FLOAT32 *) p_src_data;
    OPJ_UINT32 i;
    OPJ_UINT32 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        l_temp = (OPJ_UINT32) * (l_src_data++);
        opj_write_bytes(l_dest_data, l_temp, sizeof(OPJ_INT16));
        l_dest_data += sizeof(OPJ_INT16);
    }
}

static void opj_j2k_write_float_to_int32(const void * p_src_data,void * p_dest_data, OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_dest_data = (OPJ_BYTE *) p_dest_data;
    OPJ_FLOAT32 * l_src_data = (OPJ_FLOAT32 *) p_src_data;
    OPJ_UINT32 i;
    OPJ_UINT32 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        l_temp = (OPJ_UINT32) * (l_src_data++);
        opj_write_bytes(l_dest_data, l_temp, sizeof(OPJ_INT32));
        l_dest_data += sizeof(OPJ_INT32);
    }
}

static void  opj_j2k_write_float_to_float(const void * p_src_data,void * p_dest_data, OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_dest_data = (OPJ_BYTE *) p_dest_data;
    OPJ_FLOAT32 * l_src_data = (OPJ_FLOAT32 *) p_src_data;
    OPJ_UINT32 i;
    OPJ_FLOAT32 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        l_temp = (OPJ_FLOAT32) * (l_src_data++);
        opj_write_float(l_dest_data, l_temp);
        l_dest_data += sizeof(OPJ_FLOAT32);
    }
}

static void  opj_j2k_write_float_to_float64(const void * p_src_data,void * p_dest_data, OPJ_UINT32 p_nb_elem)
{
    OPJ_BYTE * l_dest_data = (OPJ_BYTE *) p_dest_data;
    OPJ_FLOAT32 * l_src_data = (OPJ_FLOAT32 *) p_src_data;
    OPJ_UINT32 i;
    OPJ_FLOAT64 l_temp;

    for (i = 0; i < p_nb_elem; ++i) {
        l_temp = (OPJ_FLOAT64) * (l_src_data++);
        opj_write_double(l_dest_data, l_temp);
        l_dest_data += sizeof(OPJ_FLOAT64);
    }
}


#pragma mark -
const char *opj_j2k_convert_progression_order(OPJ_PROG_ORDER prg_order)
{
    const j2k_prog_order_t *po;
    for (po = j2k_prog_order_list; po->enum_prog != -1; po++) {
        if (po->enum_prog == prg_order) {
            return po->str_prog;
        }
    }
    return po->str_prog;
}

static OPJ_BOOL opj_j2k_check_poc_val(const opj_poc_t *p_pocs,
                                      OPJ_UINT32 tileno,
                                      OPJ_UINT32 p_nb_pocs,
                                      OPJ_UINT32 p_nb_resolutions,
                                      OPJ_UINT32 p_num_comps,
                                      OPJ_UINT32 p_num_layers,
                                      opj_event_mgr_t * p_manager)
{
    OPJ_UINT32* packet_array;
    OPJ_UINT32 index, resno, compno, layno;
    OPJ_UINT32 i;
    OPJ_UINT32 step_c = 1;
    OPJ_UINT32 step_r = p_num_comps * step_c;
    OPJ_UINT32 step_l = p_nb_resolutions * step_r;
    OPJ_BOOL loss = OPJ_FALSE;

    assert(p_nb_pocs > 0);

    packet_array = (OPJ_UINT32*) calloc((size_t)step_l * p_num_layers,
                                            sizeof(OPJ_UINT32));
    if (packet_array == 00) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Not enough memory for checking the poc values.\n");
        return OPJ_FALSE;
    }

    /* iterate through all the pocs that match our tile of interest. */
    for (i = 0; i < p_nb_pocs; ++i) {
        const opj_poc_t *poc = &p_pocs[i];
        if (tileno + 1 == poc->tile) {
            index = step_r * poc->resno0;

            /* take each resolution for each poc */
            for (resno = poc->resno0 ;
                    resno < opj_uint_min(poc->resno1, p_nb_resolutions); ++resno) {
                OPJ_UINT32 res_index = index + poc->compno0 * step_c;

                /* take each comp of each resolution for each poc */
                for (compno = poc->compno0 ;
                        compno < opj_uint_min(poc->compno1, p_num_comps); ++compno) {
                    /* The layer index always starts at zero for every progression. */
                    const OPJ_UINT32 layno0 = 0;
                    OPJ_UINT32 comp_index = res_index + layno0 * step_l;

                    /* and finally take each layer of each res of ... */
                    for (layno = layno0; layno < opj_uint_min(poc->layno1, p_num_layers);
                            ++layno) {
                        packet_array[comp_index] = 1;
                        comp_index += step_l;
                    }

                    res_index += step_c;
                }

                index += step_r;
            }
        }
    }

    index = 0;
    for (layno = 0; layno < p_num_layers ; ++layno) {
        for (resno = 0; resno < p_nb_resolutions; ++resno) {
            for (compno = 0; compno < p_num_comps; ++compno) {
                loss |= (packet_array[index] != 1);
#ifdef DEBUG_VERBOSE
                if (packet_array[index] != 1) {
                    fprintf(stderr,
                            "Missing packet in POC: layno=%d resno=%d compno=%d\n",
                            layno, resno, compno);
                }
#endif
                index += step_c;
            }
        }
    }

    if (loss) {
        opj_event_msg(p_manager, EVT_ERROR, "Missing packets possible loss of data\n");
    }

    free(packet_array);

    return !loss;
}


static OPJ_UINT32 opj_j2k_get_num_tp(opj_cp_t *cp, OPJ_UINT32 pino, OPJ_UINT32 tileno)
{
    const OPJ_CHAR *prog = 00;
    OPJ_INT32 i;
    OPJ_UINT32 tpnum = 1;
    opj_tcp_t *tcp = 00;
    opj_poc_t * l_current_poc = 00;

    /*  preconditions */
    assert(tileno < (cp->tw * cp->th));
    assert(pino < (cp->tcps[tileno].numpocs + 1));

    /* get the given tile coding parameter */
    tcp = &cp->tcps[tileno];
    assert(tcp != 00);

    l_current_poc = &(tcp->pocs[pino]);
    assert(l_current_poc != 0);

    /* get the progression order as a character string */
    prog = opj_j2k_convert_progression_order(tcp->prg);
    assert(strlen(prog) > 0);

    if (cp->m_specific_param.m_enc.m_tp_on == 1) {
        for (i = 0; i < 4; ++i) {
            switch (prog[i]) {
            /* component wise */
            case 'C':
                tpnum *= l_current_poc->compE;
                break;
            /* resolution wise */
            case 'R':
                tpnum *= l_current_poc->resE;
                break;
            /* precinct wise */
            case 'P':
                tpnum *= l_current_poc->prcE;
                break;
            /* layer wise */
            case 'L':
                tpnum *= l_current_poc->layE;
                break;
            }
            /* would we split here ? */
            if (cp->m_specific_param.m_enc.m_tp_flag == prog[i]) {
                cp->m_specific_param.m_enc.m_tp_pos = i;
                break;
            }
        }
    } else {
        tpnum = 1;
    }

    return tpnum;
}

static OPJ_BOOL opj_j2k_calculate_tp(opj_j2k_t *p_j2k,
                                     opj_cp_t *cp,
                                     OPJ_UINT32 * p_nb_tiles,
                                     opj_image_t *image,
                                     opj_event_mgr_t * p_manager
                                    )
{
    OPJ_UINT32 pino, tileno;
    OPJ_UINT32 l_nb_tiles;
    opj_tcp_t *tcp;

    /* preconditions */
    assert(p_nb_tiles != 00);
    assert(cp != 00);
    assert(image != 00);
    assert(p_j2k != 00);
    assert(p_manager != 00);

    OPJ_UNUSED(p_j2k);
    OPJ_UNUSED(p_manager);

    l_nb_tiles = cp->tw * cp->th;
    * p_nb_tiles = 0;
    tcp = cp->tcps;
    for (tileno = 0; tileno < l_nb_tiles; ++tileno) {
      OPJ_UINT32 cur_totnum_tp = 0;

      opj_pi_update_encoding_parameters(image, cp, tileno);

      for (pino = 0; pino <= tcp->numpocs; ++pino) {
          OPJ_UINT32 tp_num = opj_j2k_get_num_tp(cp, pino, tileno);

          *p_nb_tiles = *p_nb_tiles + tp_num;

          cur_totnum_tp += tp_num;
      }
      tcp->m_nb_tile_parts = cur_totnum_tp;

      ++tcp;
    }
    return OPJ_TRUE;
}

static OPJ_UINT32 opj_j2k_get_specific_header_sizes(opj_j2k_t *p_j2k)
{
    OPJ_UINT32 l_nb_bytes = 0;
    OPJ_UINT32 l_nb_comps;
    OPJ_UINT32 l_coc_bytes, l_qcc_bytes;

    l_nb_comps = p_j2k->m_private_image->numcomps - 1;
    l_nb_bytes += opj_j2k_get_max_toc_size(p_j2k);

    if (!(OPJ_IS_CINEMA(p_j2k->m_cp.rsiz))) {
        l_coc_bytes = opj_j2k_get_max_coc_size(p_j2k);
        l_nb_bytes += l_nb_comps * l_coc_bytes;

        l_qcc_bytes = opj_j2k_get_max_qcc_size(p_j2k);
        l_nb_bytes += l_nb_comps * l_qcc_bytes;
    }

    l_nb_bytes += opj_j2k_get_max_poc_size(p_j2k);

    if (p_j2k->m_specific_param.m_encoder.m_PLT) {
        /* Reserve space for PLT markers */

        OPJ_UINT32 i;
        const opj_cp_t * l_cp = &(p_j2k->m_cp);
        OPJ_UINT32 l_max_packet_count = 0;
        for (i = 0; i < l_cp->th * l_cp->tw; ++i) {
            l_max_packet_count = opj_uint_max(l_max_packet_count,
                                              opj_get_encoding_packet_count(p_j2k->m_private_image, l_cp, i));
        }
        /* Minimum 6 bytes per PLT marker, and at a minimum (taking a pessimistic */
        /* estimate of 4 bytes for a packet size), one can write */
        /* (65536-6) / 4 = 16382 paquet sizes per PLT marker */
        p_j2k->m_specific_param.m_encoder.m_reserved_bytes_for_PLT =
            6 * opj_uint_ceildiv(l_max_packet_count, 16382);
        /* Maximum 5 bytes per packet to encode a full UINT32 */
        p_j2k->m_specific_param.m_encoder.m_reserved_bytes_for_PLT +=
            l_nb_bytes += 5 * l_max_packet_count;
        p_j2k->m_specific_param.m_encoder.m_reserved_bytes_for_PLT += 1;
        l_nb_bytes += p_j2k->m_specific_param.m_encoder.m_reserved_bytes_for_PLT;
    }

    /*** DEVELOPER CORNER, Add room for your headers ***/

    return l_nb_bytes;
}

static OPJ_BOOL opj_j2k_compare_coc(opj_j2k_t *p_j2k,OPJ_UINT32 p_first_comp_no, OPJ_UINT32 p_second_comp_no)
{
    opj_cp_t *l_cp = NULL;
    opj_tcp_t *l_tcp = NULL;

    /* preconditions */
    assert(p_j2k != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = &l_cp->tcps[p_j2k->m_current_tile_number];

    if (l_tcp->tccps[p_first_comp_no].csty != l_tcp->tccps[p_second_comp_no].csty) {
        return OPJ_FALSE;
    }


    return opj_j2k_compare_SPCod_SPCoc(p_j2k, p_j2k->m_current_tile_number,
                                       p_first_comp_no, p_second_comp_no);
}

static OPJ_UINT32 opj_j2k_get_max_coc_size(opj_j2k_t *p_j2k)
{
    OPJ_UINT32 i, j;
    OPJ_UINT32 l_nb_comp;
    OPJ_UINT32 l_nb_tiles;
    OPJ_UINT32 l_max = 0;

    /* preconditions */

    l_nb_tiles = p_j2k->m_cp.tw * p_j2k->m_cp.th ;
    l_nb_comp = p_j2k->m_private_image->numcomps;

    for (i = 0; i < l_nb_tiles; ++i) {
        for (j = 0; j < l_nb_comp; ++j) {
            l_max = opj_uint_max(l_max, opj_j2k_get_SPCod_SPCoc_size(p_j2k, i, j));
        }
    }

    return 6 + l_max;
}


static OPJ_BOOL opj_j2k_compare_qcc(opj_j2k_t *p_j2k,OPJ_UINT32 p_first_comp_no, OPJ_UINT32 p_second_comp_no)
{
    return opj_j2k_compare_SQcd_SQcc(p_j2k, p_j2k->m_current_tile_number,p_first_comp_no, p_second_comp_no);
}

static OPJ_UINT32 opj_j2k_get_max_qcc_size(opj_j2k_t *p_j2k)
{
    return opj_j2k_get_max_coc_size(p_j2k);
}

static OPJ_UINT32 opj_j2k_get_max_poc_size(opj_j2k_t *p_j2k)
{
    opj_tcp_t * l_tcp = 00;
    OPJ_UINT32 l_nb_tiles = 0;
    OPJ_UINT32 l_max_poc = 0;
    OPJ_UINT32 i;

    l_tcp = p_j2k->m_cp.tcps;
    l_nb_tiles = p_j2k->m_cp.th * p_j2k->m_cp.tw;

    for (i = 0; i < l_nb_tiles; ++i) {
        l_max_poc = opj_uint_max(l_max_poc, l_tcp->numpocs);
        ++l_tcp;
    }

    ++l_max_poc;

    return 4 + 9 * l_max_poc;
}

static OPJ_UINT32 opj_j2k_get_max_toc_size(opj_j2k_t *p_j2k)
{
    OPJ_UINT32 i;
    OPJ_UINT32 l_nb_tiles;
    OPJ_UINT32 l_max = 0;
    opj_tcp_t * l_tcp = 00;

    l_tcp = p_j2k->m_cp.tcps;
    l_nb_tiles = p_j2k->m_cp.tw * p_j2k->m_cp.th ;

    for (i = 0; i < l_nb_tiles; ++i) {
        l_max = opj_uint_max(l_max, l_tcp->m_nb_tile_parts);

        ++l_tcp;
    }

    return 12 * l_max;
}

static OPJ_BOOL opj_j2k_merge_ppm(opj_cp_t *p_cp, opj_event_mgr_t * p_manager)
{
   /**
    * Merges all PPM markers read (Packed headers, main header)
    *
    * @param       p_cp      main coding parameters.
    * @param       p_manager the user event manager.
    */
    OPJ_UINT32 i, l_ppm_data_size, l_N_ppm_remaining;

    if (p_cp->ppm == 0U) {
        return OPJ_TRUE;
    }

    l_ppm_data_size = 0U;
    l_N_ppm_remaining = 0U;
    for (i = 0U; i < p_cp->ppm_markers_count; ++i) {
        if (p_cp->ppm_markers[i].m_data !=
                NULL) { /* standard doesn't seem to require contiguous Zppm */
            OPJ_UINT32 l_N_ppm;
            OPJ_UINT32 l_data_size = p_cp->ppm_markers[i].m_data_size;
            const OPJ_BYTE* l_data = p_cp->ppm_markers[i].m_data;

            if (l_N_ppm_remaining >= l_data_size) {
                l_N_ppm_remaining -= l_data_size;
                l_data_size = 0U;
            } else {
                l_data += l_N_ppm_remaining;
                l_data_size -= l_N_ppm_remaining;
                l_N_ppm_remaining = 0U;
            }

            if (l_data_size > 0U) {
                do {
                    /* read Nppm */
                    if (l_data_size < 4U) {
                        /* clean up to be done on l_cp destruction */
                        opj_event_msg(p_manager, EVT_ERROR, "Not enough bytes to read Nppm\n");
                        return OPJ_FALSE;
                    }
                    opj_read_bytes(l_data, &l_N_ppm, 4);
                    l_data += 4;
                    l_data_size -= 4;

                    if (l_ppm_data_size > UINT_MAX - l_N_ppm) {
                        opj_event_msg(p_manager, EVT_ERROR, "Too large value for Nppm\n");
                        return OPJ_FALSE;
                    }
                    l_ppm_data_size += l_N_ppm;
                    if (l_data_size >= l_N_ppm) {
                        l_data_size -= l_N_ppm;
                        l_data += l_N_ppm;
                    } else {
                        l_N_ppm_remaining = l_N_ppm - l_data_size;
                        l_data_size = 0U;
                    }
                } while (l_data_size > 0U);
            }
        }
    }

    if (l_N_ppm_remaining != 0U) {
        /* clean up to be done on l_cp destruction */
        opj_event_msg(p_manager, EVT_ERROR, "Corrupted PPM markers\n");
        return OPJ_FALSE;
    }

    p_cp->ppm_buffer = (OPJ_BYTE *) malloc(l_ppm_data_size);
    if (p_cp->ppm_buffer == 00) {
        opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to read PPM marker\n");
        return OPJ_FALSE;
    }
    p_cp->ppm_len = l_ppm_data_size;
    l_ppm_data_size = 0U;
    l_N_ppm_remaining = 0U;
    for (i = 0U; i < p_cp->ppm_markers_count; ++i) {
        if (p_cp->ppm_markers[i].m_data !=
                NULL) { /* standard doesn't seem to require contiguous Zppm */
            OPJ_UINT32 l_N_ppm;
            OPJ_UINT32 l_data_size = p_cp->ppm_markers[i].m_data_size;
            const OPJ_BYTE* l_data = p_cp->ppm_markers[i].m_data;

            if (l_N_ppm_remaining >= l_data_size) {
                memcpy(p_cp->ppm_buffer + l_ppm_data_size, l_data, l_data_size);
                l_ppm_data_size += l_data_size;
                l_N_ppm_remaining -= l_data_size;
                l_data_size = 0U;
            } else {
                memcpy(p_cp->ppm_buffer + l_ppm_data_size, l_data, l_N_ppm_remaining);
                l_ppm_data_size += l_N_ppm_remaining;
                l_data += l_N_ppm_remaining;
                l_data_size -= l_N_ppm_remaining;
                l_N_ppm_remaining = 0U;
            }

            if (l_data_size > 0U) {
                do {
                    /* read Nppm */
                    if (l_data_size < 4U) {
                        /* clean up to be done on l_cp destruction */
                        opj_event_msg(p_manager, EVT_ERROR, "Not enough bytes to read Nppm\n");
                        return OPJ_FALSE;
                    }
                    opj_read_bytes(l_data, &l_N_ppm, 4);
                    l_data += 4;
                    l_data_size -= 4;

                    if (l_data_size >= l_N_ppm) {
                        memcpy(p_cp->ppm_buffer + l_ppm_data_size, l_data, l_N_ppm);
                        l_ppm_data_size += l_N_ppm;
                        l_data_size -= l_N_ppm;
                        l_data += l_N_ppm;
                    } else {
                        memcpy(p_cp->ppm_buffer + l_ppm_data_size, l_data, l_data_size);
                        l_ppm_data_size += l_data_size;
                        l_N_ppm_remaining = l_N_ppm - l_data_size;
                        l_data_size = 0U;
                    }
                } while (l_data_size > 0U);
            }
            free(p_cp->ppm_markers[i].m_data);
            p_cp->ppm_markers[i].m_data = NULL;
            p_cp->ppm_markers[i].m_data_size = 0U;
        }
    }

    p_cp->ppm_data = p_cp->ppm_buffer;
    p_cp->ppm_data_size = p_cp->ppm_len;

    p_cp->ppm_markers_count = 0U;
    free(p_cp->ppm_markers);
    p_cp->ppm_markers = NULL;

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_merge_ppt(opj_tcp_t *p_tcp, opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 i, l_ppt_data_size;
   /**
    * Merges all PPT markers read (Packed packet headers, tile-part header)
    *
    * @param       p_tcp   the tile.
    * @param       p_manager               the user event manager.
    */

    if (p_tcp->ppt_buffer != NULL) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "opj_j2k_merge_ppt() has already been called\n");
        return OPJ_FALSE;
    }

    if (p_tcp->ppt == 0U) {
        return OPJ_TRUE;
    }

    l_ppt_data_size = 0U;
    for (i = 0U; i < p_tcp->ppt_markers_count; ++i) {
        l_ppt_data_size +=
            p_tcp->ppt_markers[i].m_data_size; /* can't overflow, max 256 markers of max 65536 bytes */
    }

    p_tcp->ppt_buffer = (OPJ_BYTE *) malloc(l_ppt_data_size);
    if (p_tcp->ppt_buffer == 00) {
        opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to read PPT marker\n");
        return OPJ_FALSE;
    }
    p_tcp->ppt_len = l_ppt_data_size;
    l_ppt_data_size = 0U;
    for (i = 0U; i < p_tcp->ppt_markers_count; ++i) {
        if (p_tcp->ppt_markers[i].m_data !=
                NULL) { /* standard doesn't seem to require contiguous Zppt */
            memcpy(p_tcp->ppt_buffer + l_ppt_data_size, p_tcp->ppt_markers[i].m_data,
                   p_tcp->ppt_markers[i].m_data_size);
            l_ppt_data_size +=
                p_tcp->ppt_markers[i].m_data_size; /* can't overflow, max 256 markers of max 65536 bytes */

            free(p_tcp->ppt_markers[i].m_data);
            p_tcp->ppt_markers[i].m_data = NULL;
            p_tcp->ppt_markers[i].m_data_size = 0U;
        }
    }

    p_tcp->ppt_markers_count = 0U;
    free(p_tcp->ppt_markers);
    p_tcp->ppt_markers = NULL;

    p_tcp->ppt_data = p_tcp->ppt_buffer;
    p_tcp->ppt_data_size = p_tcp->ppt_len;
    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_get_sot_values(OPJ_BYTE *  p_header_data,OPJ_UINT32  p_header_size,OPJ_UINT32* p_tile_no,OPJ_UINT32* p_tot_len,OPJ_UINT32* p_current_part,OPJ_UINT32* p_num_parts,opj_event_mgr_t * p_manager)
{
    /* Size of this marker is fixed = 12 (we have already read marker and its size)*/
    if (p_header_size != 8) {
        opj_event_msg(p_manager, EVT_ERROR, "Error reading SOT marker\n");
        return OPJ_FALSE;
    }

    opj_read_bytes(p_header_data, p_tile_no, 2);    /* Isot */
    p_header_data += 2;
    opj_read_bytes(p_header_data, p_tot_len, 4);    /* Psot */
    p_header_data += 4;
    opj_read_bytes(p_header_data, p_current_part, 1); /* TPsot */
    ++p_header_data;
    opj_read_bytes(p_header_data, p_num_parts, 1);  /* TNsot */
    ++p_header_data;
    return OPJ_TRUE;
}

static OPJ_FLOAT32 opj_j2k_get_tp_stride(opj_tcp_t * p_tcp)
{
    return (OPJ_FLOAT32)((p_tcp->m_nb_tile_parts - 1) * 14);
}

static OPJ_FLOAT32 opj_j2k_get_default_stride(opj_tcp_t * p_tcp)
{
    (void)p_tcp;
    return 0;
}




static OPJ_BOOL opj_j2k_add_mct(opj_tcp_t * p_tcp, opj_image_t * p_image,OPJ_UINT32 p_index)
{
    OPJ_UINT32 i;
    opj_simple_mcc_decorrelation_data_t * l_mcc_record;
    opj_mct_data_t * l_deco_array, * l_offset_array;
    OPJ_UINT32 l_data_size, l_mct_size, l_offset_size;
    OPJ_UINT32 l_nb_elem;
    OPJ_UINT32 * l_offset_data, * l_current_offset_data;
    opj_tccp_t * l_tccp;

    /* preconditions */
    assert(p_tcp != 00);

    l_mcc_record = p_tcp->m_mcc_records;

    for (i = 0; i < p_tcp->m_nb_mcc_records; ++i) {
        if (l_mcc_record->m_index == p_index) {
            break;
        }
    }

    if (i == p_tcp->m_nb_mcc_records) {
        /** element discarded **/
        return OPJ_TRUE;
    }

    if (l_mcc_record->m_nb_comps != p_image->numcomps) {
        /** do not support number of comps != image */
        return OPJ_TRUE;
    }

    l_deco_array = l_mcc_record->m_decorrelation_array;

    if (l_deco_array) {
        l_data_size = MCT_ELEMENT_SIZE[l_deco_array->m_element_type] * p_image->numcomps
                      * p_image->numcomps;
        if (l_deco_array->m_data_size != l_data_size) {
            return OPJ_FALSE;
        }

        l_nb_elem = p_image->numcomps * p_image->numcomps;
        l_mct_size = l_nb_elem * (OPJ_UINT32)sizeof(OPJ_FLOAT32);
        p_tcp->m_mct_decoding_matrix = (OPJ_FLOAT32*)malloc(l_mct_size);

        if (! p_tcp->m_mct_decoding_matrix) {
            return OPJ_FALSE;
        }

        j2k_mct_read_functions_to_float[l_deco_array->m_element_type](
            l_deco_array->m_data, p_tcp->m_mct_decoding_matrix, l_nb_elem);
    }

    l_offset_array = l_mcc_record->m_offset_array;

    if (l_offset_array) {
        l_data_size = MCT_ELEMENT_SIZE[l_offset_array->m_element_type] *
                      p_image->numcomps;
        if (l_offset_array->m_data_size != l_data_size) {
            return OPJ_FALSE;
        }

        l_nb_elem = p_image->numcomps;
        l_offset_size = l_nb_elem * (OPJ_UINT32)sizeof(OPJ_UINT32);
        l_offset_data = (OPJ_UINT32*)malloc(l_offset_size);

        if (! l_offset_data) {
            return OPJ_FALSE;
        }

        j2k_mct_read_functions_to_int32[l_offset_array->m_element_type](
            l_offset_array->m_data, l_offset_data, l_nb_elem);

        l_tccp = p_tcp->tccps;
        l_current_offset_data = l_offset_data;

        for (i = 0; i < p_image->numcomps; ++i) {
            l_tccp->m_dc_level_shift = (OPJ_INT32) * (l_current_offset_data++);
            ++l_tccp;
        }

        free(l_offset_data);
    }

    return OPJ_TRUE;
}

#pragma mark - write memory

static void opj_j2k_write_coc_in_memory(opj_j2k_t *p_j2k,OPJ_UINT32 p_comp_no,OPJ_BYTE * p_data,OPJ_UINT32 * p_data_written,opj_event_mgr_t * p_manager)
{
    opj_cp_t *l_cp = 00;
    opj_tcp_t *l_tcp = 00;
    OPJ_UINT32 l_coc_size, l_remaining_size;
    OPJ_BYTE * l_current_data = 00;
    opj_image_t *l_image = 00;
    OPJ_UINT32 l_comp_room;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = &l_cp->tcps[p_j2k->m_current_tile_number];
    l_image = p_j2k->m_private_image;
    l_comp_room = (l_image->numcomps <= 256) ? 1 : 2;

    l_coc_size = 5 + l_comp_room + opj_j2k_get_SPCod_SPCoc_size(p_j2k,
                 p_j2k->m_current_tile_number, p_comp_no);
    l_remaining_size = l_coc_size;

    l_current_data = p_data;

    opj_write_bytes(l_current_data, J2K_MS_COC,
                    2);                         /* COC */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_coc_size - 2,
                    2);                     /* L_COC */
    l_current_data += 2;

    opj_write_bytes(l_current_data, p_comp_no, l_comp_room);        /* Ccoc */
    l_current_data += l_comp_room;

    opj_write_bytes(l_current_data, l_tcp->tccps[p_comp_no].csty,
                    1);               /* Scoc */
    ++l_current_data;

    l_remaining_size -= (5 + l_comp_room);
    opj_j2k_write_SPCod_SPCoc(p_j2k, p_j2k->m_current_tile_number, 0,
                              l_current_data, &l_remaining_size, p_manager);
    * p_data_written = l_coc_size;
}

static void opj_j2k_write_qcc_in_memory(opj_j2k_t *p_j2k,OPJ_UINT32 p_comp_no,OPJ_BYTE * p_data,OPJ_UINT32 * p_data_written,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 l_qcc_size, l_remaining_size;
    OPJ_BYTE * l_current_data = 00;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);

    l_qcc_size = 6 + opj_j2k_get_SQcd_SQcc_size(p_j2k, p_j2k->m_current_tile_number,
                 p_comp_no);
    l_remaining_size = l_qcc_size;

    l_current_data = p_data;

    opj_write_bytes(l_current_data, J2K_MS_QCC, 2);         /* QCC */
    l_current_data += 2;

    if (p_j2k->m_private_image->numcomps <= 256) {
        --l_qcc_size;

        opj_write_bytes(l_current_data, l_qcc_size - 2, 2);     /* L_QCC */
        l_current_data += 2;

        opj_write_bytes(l_current_data, p_comp_no, 1);  /* Cqcc */
        ++l_current_data;

        /* in the case only one byte is sufficient the last byte allocated is useless -> still do -6 for available */
        l_remaining_size -= 6;
    } else {
        opj_write_bytes(l_current_data, l_qcc_size - 2, 2);     /* L_QCC */
        l_current_data += 2;

        opj_write_bytes(l_current_data, p_comp_no, 2);  /* Cqcc */
        l_current_data += 2;

        l_remaining_size -= 6;
    }

    opj_j2k_write_SQcd_SQcc(p_j2k, p_j2k->m_current_tile_number, p_comp_no,
                            l_current_data, &l_remaining_size, p_manager);

    *p_data_written = l_qcc_size;
}

static OPJ_BOOL opj_j2k_write_plt_in_memory(opj_j2k_t *p_j2k,opj_tcd_marker_info_t* marker_info,OPJ_BYTE * p_data,OPJ_UINT32 * p_data_written)
{
   /**
    * Write one or more PLT markers in the provided buffer
    */
    OPJ_BYTE Zplt = 0;
    OPJ_UINT16 Lplt;
    OPJ_BYTE* p_data_start = p_data;
    OPJ_BYTE* p_data_Lplt = p_data + 2;
    OPJ_UINT32 i;

    OPJ_UNUSED(p_j2k);

    opj_write_bytes(p_data, J2K_MS_PLT, 2);
    p_data += 2;

    /* Reserve space for Lplt */
    p_data += 2;

    opj_write_bytes(p_data, Zplt, 1);
    p_data += 1;

    Lplt = 3;

    for (i = 0; i < marker_info->packet_count; i++) {
        OPJ_BYTE var_bytes[5];
        OPJ_UINT8 var_bytes_size = 0;
        OPJ_UINT32 packet_size = marker_info->p_packet_size[i];

        /* Packet size written in variable-length way, starting with LSB */
        var_bytes[var_bytes_size] = (OPJ_BYTE)(packet_size & 0x7f);
        var_bytes_size ++;
        packet_size >>= 7;
        while (packet_size > 0) {
            var_bytes[var_bytes_size] = (OPJ_BYTE)((packet_size & 0x7f) | 0x80);
            var_bytes_size ++;
            packet_size >>= 7;
        }

        /* Check if that can fit in the current PLT marker. If not, finish */
        /* current one, and start a new one */
        if (Lplt + var_bytes_size > 65535) {
            if (Zplt == 255) {
                E("%s","More than 255 PLT markers would be needed for current tile-part !\n");
                return OPJ_FALSE;
            }

            /* Patch Lplt */
            opj_write_bytes(p_data_Lplt, Lplt, 2);

            /* Start new segment */
            opj_write_bytes(p_data, J2K_MS_PLT, 2);
            p_data += 2;

            /* Reserve space for Lplt */
            p_data_Lplt = p_data;
            p_data += 2;

            Zplt ++;
            opj_write_bytes(p_data, Zplt, 1);
            p_data += 1;

            Lplt = 3;
        }

        Lplt = (OPJ_UINT16)(Lplt + var_bytes_size);

        /* Serialize variable-length packet size, starting with MSB */
        for (; var_bytes_size > 0; --var_bytes_size) {
            opj_write_bytes(p_data, var_bytes[var_bytes_size - 1], 1);
            p_data += 1;
        }
    }

    *p_data_written = (OPJ_UINT32)(p_data - p_data_start);

    /* Patch Lplt */
    opj_write_bytes(p_data_Lplt, Lplt, 2);

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_sod(opj_j2k_t *p_j2k,opj_tcd_t * p_tile_coder,OPJ_BYTE * p_data,OPJ_UINT32 * p_data_written,OPJ_UINT32 total_data_size,const opj_stream_private_t *p_stream)
{
    opj_codestream_info_t *l_cstr_info = 00;
    OPJ_UINT32 l_remaining_data;
    opj_tcd_marker_info_t* marker_info = NULL;


    OPJ_UNUSED(p_stream);

    if (total_data_size < 4) {
        E("%s","Not enough bytes in output buffer to write SOD marker\n");
        return OPJ_FALSE;
    }

    opj_write_bytes(p_data, J2K_MS_SOD,2);

    /* make room for the EOF marker */
    l_remaining_data =  total_data_size - 4;

    /* update tile coder */
    p_tile_coder->tp_num =
        p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number ;
    p_tile_coder->cur_tp_num =
        p_j2k->m_specific_param.m_encoder.m_current_tile_part_number;

    /* INDEX >> */
    /* TODO mergeV2: check this part which use cstr_info */
    /*l_cstr_info = p_j2k->cstr_info;
    if (l_cstr_info) {
            if (!p_j2k->m_specific_param.m_encoder.m_current_tile_part_number ) {
                    //TODO cstr_info->tile[p_j2k->m_current_tile_number].end_header = p_stream_tell(p_stream) + p_j2k->pos_correction - 1;
                    l_cstr_info->tile[p_j2k->m_current_tile_number].tileno = p_j2k->m_current_tile_number;
            }
            else {*/
    /*
    TODO
    if
            (cstr_info->tile[p_j2k->m_current_tile_number].packet[cstr_info->packno - 1].end_pos < p_stream_tell(p_stream))
    {
            cstr_info->tile[p_j2k->m_current_tile_number].packet[cstr_info->packno].start_pos = p_stream_tell(p_stream);
    }*/
    /*}*/
    /* UniPG>> */
#ifdef USE_JPWL
    /* update markers struct */
    /*OPJ_BOOL res = j2k_add_marker(p_j2k->cstr_info, J2K_MS_SOD, p_j2k->sod_start, 2);
    */
    assert(0 && "TODO");
#endif /* USE_JPWL */
    /* <<UniPG */
    /*}*/
    /* << INDEX */

    if (p_j2k->m_specific_param.m_encoder.m_current_tile_part_number == 0) {
        p_tile_coder->tcd_image->tiles->packno = 0;
#ifdef deadcode
        if (l_cstr_info) {
            l_cstr_info->packno = 0;
        }
#endif
    }

    *p_data_written = 0;

    if (p_j2k->m_specific_param.m_encoder.m_PLT) {
        marker_info = opj_tcd_marker_info_create(
                          p_j2k->m_specific_param.m_encoder.m_PLT);
        if (marker_info == NULL) {
            E("%s","Cannot encode tile: opj_tcd_marker_info_create() failed\n");
            return OPJ_FALSE;
        }
    }

    if (l_remaining_data <
            p_j2k->m_specific_param.m_encoder.m_reserved_bytes_for_PLT) {
       E("%s","Not enough bytes in output buffer to write SOD marker\n");
        opj_tcd_marker_info_destroy(marker_info);
        return OPJ_FALSE;
    }
    l_remaining_data -= p_j2k->m_specific_param.m_encoder.m_reserved_bytes_for_PLT;

    if (! opj_tcd_encode_tile(p_tile_coder, p_j2k->m_current_tile_number,
                              p_data + 2,
                              p_data_written, l_remaining_data, l_cstr_info,
                              marker_info)) {
       E("%s","Cannot encode tile\n");
        opj_tcd_marker_info_destroy(marker_info);
        return OPJ_FALSE;
    }

    /* For SOD */
    *p_data_written += 2;

    if (p_j2k->m_specific_param.m_encoder.m_PLT) {
        OPJ_UINT32 l_data_written_PLT = 0;
        OPJ_BYTE* p_PLT_buffer = (OPJ_BYTE*)malloc(
                                     p_j2k->m_specific_param.m_encoder.m_reserved_bytes_for_PLT);
        if (!p_PLT_buffer) {
           E("%s","Cannot allocate memory\n");
            opj_tcd_marker_info_destroy(marker_info);
            return OPJ_FALSE;
        }
        if (!opj_j2k_write_plt_in_memory(p_j2k,
                                         marker_info,
                                         p_PLT_buffer,
                                         &l_data_written_PLT)) {
            opj_tcd_marker_info_destroy(marker_info);
            free(p_PLT_buffer);
            return OPJ_FALSE;
        }

        assert(l_data_written_PLT <=
               p_j2k->m_specific_param.m_encoder.m_reserved_bytes_for_PLT);

        /* Move PLT marker(s) before SOD */
        memmove(p_data + l_data_written_PLT, p_data, *p_data_written);
        memcpy(p_data, p_PLT_buffer, l_data_written_PLT);
        free(p_PLT_buffer);
        *p_data_written += l_data_written_PLT;
    }

    opj_tcd_marker_info_destroy(marker_info);

    return OPJ_TRUE;
}


#pragma mark -




static OPJ_BOOL opj_j2k_write_epc(opj_j2k_t *p_j2k,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    opj_codestream_index_t * l_cstr_index = 00;
    OPJ_UNUSED(p_manager);

    l_cstr_index = p_j2k->cstr_index;
    if (l_cstr_index) {
        l_cstr_index->codestream_size = (OPJ_UINT64)opj_stream_tell(p_stream);
        /* UniPG>> */
        /* The following adjustment is done to adjust the codestream size */
        /* if SOD is not at 0 in the buffer. Useful in case of JP2, where */
        /* the first bunch of bytes is not in the codestream              */
        l_cstr_index->codestream_size -= (OPJ_UINT64)l_cstr_index->main_head_start;
        /* <<UniPG */
    }

#ifdef USE_JPWL
    /* preparation of JPWL marker segments */
#if 0
    if (cp->epc_on) {

        /* encode according to JPWL */
        jpwl_encode(p_j2k, p_stream, image);

    }
#endif
    assert(0 && "TODO");
#endif /* USE_JPWL */

    return OPJ_TRUE;
}





#pragma mark - J2K / JPT decoder interface
OPJ_BOOL opj_j2k_set_threads(opj_j2k_t *j2k, OPJ_UINT32 num_threads)
{
    /* Currently we pass the thread-pool to the tcd, so we cannot re-set it */
    /* afterwards */
    if (opj_has_thread_support() && j2k->m_tcd == NULL) {
        opj_thread_pool_destroy(j2k->m_tp);
        j2k->m_tp = NULL;
        if (num_threads <= (OPJ_UINT32)INT_MAX) {
            j2k->m_tp = opj_thread_pool_create((int)num_threads);
        }
        if (j2k->m_tp == NULL) {
            j2k->m_tp = opj_thread_pool_create(0);
            return OPJ_FALSE;
        }
        return OPJ_TRUE;
    }
    return OPJ_FALSE;
}

static int opj_j2k_get_default_thread_count(void)
{
    const char* num_threads_str = getenv("OPJ_NUM_THREADS");
    int num_cpus;
    int num_threads;

    if (num_threads_str == NULL || !opj_has_thread_support()) {
        return 0;
    }
    num_cpus = opj_get_num_cpus();
    if (strcmp(num_threads_str, "ALL_CPUS") == 0) {
        return num_cpus;
    }
    if (num_cpus == 0) {
        num_cpus = 32;
    }
    num_threads = atoi(num_threads_str);
    if (num_threads < 0) {
        num_threads = 0;
    } else if (num_threads > 2 * num_cpus) {
        num_threads = 2 * num_cpus;
    }
    return num_threads;
}


#pragma mark - J2K encoder interface


opj_j2k_t* opj_j2k_create_compress(void)
{
    opj_j2k_t *l_j2k = (opj_j2k_t*) calloc(1, sizeof(opj_j2k_t));
    if (!l_j2k) return NULL;


    l_j2k->m_is_decoder = 0;
    l_j2k->m_cp.m_is_decoder = 0;

    l_j2k->m_specific_param.m_encoder.m_header_tile_data = (OPJ_BYTE *) malloc(OPJ_J2K_DEFAULT_HEADER_SIZE);//1000
    if (! l_j2k->m_specific_param.m_encoder.m_header_tile_data) {opj_j2k_destroy(l_j2k);return NULL;}
    l_j2k->m_specific_param.m_encoder.m_header_tile_data_size = OPJ_J2K_DEFAULT_HEADER_SIZE;

    //validation list
    l_j2k->m_validation_list = opj_procedure_list_create();
    if (! l_j2k->m_validation_list) { opj_j2k_destroy(l_j2k); return NULL;}

    //execution list
    l_j2k->m_procedure_list = opj_procedure_list_create();
    if (! l_j2k->m_procedure_list) { opj_j2k_destroy(l_j2k); return NULL;}

   
   //thread count
   
   
    //thread pool
    l_j2k->m_tp = opj_thread_pool_create(opj_j2k_get_default_thread_count());
    if (!l_j2k->m_tp) l_j2k->m_tp = opj_thread_pool_create(0);
    if (!l_j2k->m_tp) {opj_j2k_destroy(l_j2k);return NULL;}

    return l_j2k;
}

static int opj_j2k_initialise_4K_poc(opj_poc_t *POC, int numres)
{
    POC[0].tile  = 1;
    POC[0].resno0  = 0;
    POC[0].compno0 = 0;
    POC[0].layno1  = 1;
    POC[0].resno1  = (OPJ_UINT32)(numres - 1);
    POC[0].compno1 = 3;
    POC[0].prg1 = OPJ_CPRL;
    POC[1].tile  = 1;
    POC[1].resno0  = (OPJ_UINT32)(numres - 1);
    POC[1].compno0 = 0;
    POC[1].layno1  = 1;
    POC[1].resno1  = (OPJ_UINT32)numres;
    POC[1].compno1 = 3;
    POC[1].prg1 = OPJ_CPRL;
    return 2;
}

static void opj_j2k_set_cinema_parameters(opj_cparameters_t *parameters,opj_image_t *image, opj_event_mgr_t *p_manager)
{
    /* Configure cinema parameters */
    int i;

    /* No tiling */
    parameters->tile_size_on = OPJ_FALSE;
    parameters->cp_tdx = 1;
    parameters->cp_tdy = 1;

    /* One tile part for each component */
    parameters->tp_flag = 'C';
    parameters->tp_on = 1;

    /* Tile and Image shall be at (0,0) */
    parameters->cp_tx0 = 0;
    parameters->cp_ty0 = 0;
    parameters->image_offset_x0 = 0;
    parameters->image_offset_y0 = 0;

    /* Codeblock size= 32*32 */
    parameters->cblockw_init = 32;
    parameters->cblockh_init = 32;

    /* Codeblock style: no mode switch enabled */
    parameters->mode = 0;

    /* No ROI */
    parameters->roi_compno = -1;

    /* No subsampling */
    parameters->subsampling_dx = 1;
    parameters->subsampling_dy = 1;

    /* 9-7 transform */
    parameters->irreversible = 1;

    /* Number of layers */
    if (parameters->tcp_numlayers > 1) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "JPEG 2000 Profile-3 and 4 (2k/4k dc profile) requires:\n"
                      "1 single quality layer"
                      "-> Number of layers forced to 1 (rather than %d)\n"
                      "-> Rate of the last layer (%3.1f) will be used",
                      parameters->tcp_numlayers,
                      parameters->tcp_rates[parameters->tcp_numlayers - 1]);
        parameters->tcp_rates[0] = parameters->tcp_rates[parameters->tcp_numlayers - 1];
        parameters->tcp_numlayers = 1;
    }

    /* Resolution levels */
    switch (parameters->rsiz) {
    case OPJ_PROFILE_CINEMA_2K:
        if (parameters->numresolution > 6) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "JPEG 2000 Profile-3 (2k dc profile) requires:\n"
                          "Number of decomposition levels <= 5\n"
                          "-> Number of decomposition levels forced to 5 (rather than %d)\n",
                          parameters->numresolution + 1);
            parameters->numresolution = 6;
        }
        break;
    case OPJ_PROFILE_CINEMA_4K:
        if (parameters->numresolution < 2) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "JPEG 2000 Profile-4 (4k dc profile) requires:\n"
                          "Number of decomposition levels >= 1 && <= 6\n"
                          "-> Number of decomposition levels forced to 1 (rather than %d)\n",
                          parameters->numresolution + 1);
            parameters->numresolution = 1;
        } else if (parameters->numresolution > 7) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "JPEG 2000 Profile-4 (4k dc profile) requires:\n"
                          "Number of decomposition levels >= 1 && <= 6\n"
                          "-> Number of decomposition levels forced to 6 (rather than %d)\n",
                          parameters->numresolution + 1);
            parameters->numresolution = 7;
        }
        break;
    default :
        break;
    }

    /* Precincts */
    parameters->csty |= J2K_CP_CSTY_PRT;
    if (parameters->numresolution == 1) {
        parameters->res_spec = 1;
        parameters->prcw_init[0] = 128;
        parameters->prch_init[0] = 128;
    } else {
        parameters->res_spec = parameters->numresolution - 1;
        for (i = 0; i < parameters->res_spec; i++) {
            parameters->prcw_init[i] = 256;
            parameters->prch_init[i] = 256;
        }
    }

    /* The progression order shall be CPRL */
    parameters->prog_order = OPJ_CPRL;

    /* Progression order changes for 4K, disallowed for 2K */
    if (parameters->rsiz == OPJ_PROFILE_CINEMA_4K) {
        parameters->numpocs = (OPJ_UINT32)opj_j2k_initialise_4K_poc(parameters->POC,
                              parameters->numresolution);
    } else {
        parameters->numpocs = 0;
    }

    /* Limited bit-rate */
    parameters->cp_disto_alloc = 1;
    if (parameters->max_cs_size <= 0) {
        /* No rate has been introduced, 24 fps is assumed */
        parameters->max_cs_size = OPJ_CINEMA_24_CS;
        opj_event_msg(p_manager, EVT_WARNING,
                      "JPEG 2000 Profile-3 and 4 (2k/4k dc profile) requires:\n"
                      "Maximum 1302083 compressed bytes @ 24fps\n"
                      "As no rate has been given, this limit will be used.\n");
    } else if (parameters->max_cs_size > OPJ_CINEMA_24_CS) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "JPEG 2000 Profile-3 and 4 (2k/4k dc profile) requires:\n"
                      "Maximum 1302083 compressed bytes @ 24fps\n"
                      "-> Specified rate exceeds this limit. Rate will be forced to 1302083 bytes.\n");
        parameters->max_cs_size = OPJ_CINEMA_24_CS;
    }


    parameters->tcp_rates[0] = (OPJ_FLOAT32)(image->numcomps * image->comps[0].w *
                               image->comps[0].h * image->comps[0].prec) /
                               (OPJ_FLOAT32)(((OPJ_UINT32)parameters->max_cs_size) * 8 * image->comps[0].dx *
                                       image->comps[0].dy);

}

static OPJ_BOOL opj_j2k_is_cinema_compliant(opj_image_t *image, OPJ_UINT16 rsiz,opj_event_mgr_t *p_manager)
{
    OPJ_UINT32 i;

    /* Number of components */
    if (image->numcomps != 3) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "JPEG 2000 Profile-3 (2k dc profile) requires:\n"
                      "3 components"
                      "-> Number of components of input image (%d) is not compliant\n"
                      "-> Non-profile-3 codestream will be generated\n",
                      image->numcomps);
        return OPJ_FALSE;
    }

    /* Bitdepth */
    for (i = 0; i < image->numcomps; i++) {
        if ((image->comps[i].prec != 12) | (image->comps[i].sgnd)) {
            char signed_str[] = "signed";
            char unsigned_str[] = "unsigned";
            char *tmp_str = image->comps[i].sgnd ? signed_str : unsigned_str;
            opj_event_msg(p_manager, EVT_WARNING,
                          "JPEG 2000 Profile-3 (2k dc profile) requires:\n"
                          "Precision of each component shall be 12 bits unsigned"
                          "-> At least component %d of input image (%d bits, %s) is not compliant\n"
                          "-> Non-profile-3 codestream will be generated\n",
                          i, image->comps[i].prec, tmp_str);
            return OPJ_FALSE;
        }
    }

    /* Image size */
    switch (rsiz) {
    case OPJ_PROFILE_CINEMA_2K:
        if (((image->comps[0].w > 2048) | (image->comps[0].h > 1080))) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "JPEG 2000 Profile-3 (2k dc profile) requires:\n"
                          "width <= 2048 and height <= 1080\n"
                          "-> Input image size %d x %d is not compliant\n"
                          "-> Non-profile-3 codestream will be generated\n",
                          image->comps[0].w, image->comps[0].h);
            return OPJ_FALSE;
        }
        break;
    case OPJ_PROFILE_CINEMA_4K:
        if (((image->comps[0].w > 4096) | (image->comps[0].h > 2160))) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "JPEG 2000 Profile-4 (4k dc profile) requires:\n"
                          "width <= 4096 and height <= 2160\n"
                          "-> Image size %d x %d is not compliant\n"
                          "-> Non-profile-4 codestream will be generated\n",
                          image->comps[0].w, image->comps[0].h);
            return OPJ_FALSE;
        }
        break;
    default :
        break;
    }

    return OPJ_TRUE;
}

static int opj_j2k_get_imf_max_NL(opj_cparameters_t *parameters,opj_image_t *image)
{
    /* Decomposition levels */
    const OPJ_UINT16 rsiz = parameters->rsiz;
    const OPJ_UINT16 profile = OPJ_GET_IMF_PROFILE(rsiz);
    const OPJ_UINT32 XTsiz = parameters->tile_size_on ? (OPJ_UINT32)
                             parameters->cp_tdx : image->x1;
    switch (profile) {
    case OPJ_PROFILE_IMF_2K:
        return 5;
    case OPJ_PROFILE_IMF_4K:
        return 6;
    case OPJ_PROFILE_IMF_8K:
        return 7;
    case OPJ_PROFILE_IMF_2K_R: {
        if (XTsiz >= 2048) {
            return 5;
        } else if (XTsiz >= 1024) {
            return 4;
        }
        break;
    }
    case OPJ_PROFILE_IMF_4K_R: {
        if (XTsiz >= 4096) {
            return 6;
        } else if (XTsiz >= 2048) {
            return 5;
        } else if (XTsiz >= 1024) {
            return 4;
        }
        break;
    }
    case OPJ_PROFILE_IMF_8K_R: {
        if (XTsiz >= 8192) {
            return 7;
        } else if (XTsiz >= 4096) {
            return 6;
        } else if (XTsiz >= 2048) {
            return 5;
        } else if (XTsiz >= 1024) {
            return 4;
        }
        break;
    }
    default:
        break;
    }
    return -1;
}

static void opj_j2k_set_imf_parameters(opj_cparameters_t *parameters,opj_image_t *image, opj_event_mgr_t *p_manager)
{
    const OPJ_UINT16 rsiz = parameters->rsiz;
    const OPJ_UINT16 profile = OPJ_GET_IMF_PROFILE(rsiz);

    OPJ_UNUSED(p_manager);

    /* Override defaults set by opj_set_default_encoder_parameters */
    if (parameters->cblockw_init == OPJ_COMP_PARAM_DEFAULT_CBLOCKW &&
            parameters->cblockh_init == OPJ_COMP_PARAM_DEFAULT_CBLOCKH) {
        parameters->cblockw_init = 32;
        parameters->cblockh_init = 32;
    }

    /* One tile part for each component */
    parameters->tp_flag = 'C';
    parameters->tp_on = 1;

    if (parameters->prog_order == OPJ_COMP_PARAM_DEFAULT_PROG_ORDER) {
        parameters->prog_order = OPJ_CPRL;
    }

    if (profile == OPJ_PROFILE_IMF_2K ||
            profile == OPJ_PROFILE_IMF_4K ||
            profile == OPJ_PROFILE_IMF_8K) {
        /* 9-7 transform */
        parameters->irreversible = 1;
    }

    /* Adjust the number of resolutions if set to its defaults */
    if (parameters->numresolution == OPJ_COMP_PARAM_DEFAULT_NUMRESOLUTION &&
            image->x0 == 0 &&
            image->y0 == 0) {
        const int max_NL = opj_j2k_get_imf_max_NL(parameters, image);
        if (max_NL >= 0 && parameters->numresolution > max_NL) {
            parameters->numresolution = max_NL + 1;
        }

        /* Note: below is generic logic */
        if (!parameters->tile_size_on) {
            while (parameters->numresolution > 0) {
                if (image->x1 < (1U << ((OPJ_UINT32)parameters->numresolution - 1U))) {
                    parameters->numresolution --;
                    continue;
                }
                if (image->y1 < (1U << ((OPJ_UINT32)parameters->numresolution - 1U))) {
                    parameters->numresolution --;
                    continue;
                }
                break;
            }
        }
    }

    /* Set defaults precincts */
    if (parameters->csty == 0) {
        parameters->csty |= J2K_CP_CSTY_PRT;
        if (parameters->numresolution == 1) {
            parameters->res_spec = 1;
            parameters->prcw_init[0] = 128;
            parameters->prch_init[0] = 128;
        } else {
            int i;
            parameters->res_spec = parameters->numresolution - 1;
            for (i = 0; i < parameters->res_spec; i++) {
                parameters->prcw_init[i] = 256;
                parameters->prch_init[i] = 256;
            }
        }
    }
}

/* Table A.53 from JPEG2000 standard */
static const OPJ_UINT16 tabMaxSubLevelFromMainLevel[] = {
    15, /* unspecified */
    1,
    1,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9
};

static OPJ_BOOL opj_j2k_is_imf_compliant(opj_cparameters_t *parameters,opj_image_t *image,opj_event_mgr_t *p_manager)
{
    OPJ_UINT32 i;
    const OPJ_UINT16 rsiz = parameters->rsiz;
    const OPJ_UINT16 profile = OPJ_GET_IMF_PROFILE(rsiz);
    const OPJ_UINT16 mainlevel = OPJ_GET_IMF_MAINLEVEL(rsiz);
    const OPJ_UINT16 sublevel = OPJ_GET_IMF_SUBLEVEL(rsiz);
    const int NL = parameters->numresolution - 1;
    const OPJ_UINT32 XTsiz = parameters->tile_size_on ? (OPJ_UINT32)
                             parameters->cp_tdx : image->x1;
    OPJ_BOOL ret = OPJ_TRUE;

    /* Validate mainlevel */
    if (mainlevel > OPJ_IMF_MAINLEVEL_MAX) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "IMF profile require mainlevel <= 11.\n"
                      "-> %d is thus not compliant\n"
                      "-> Non-IMF codestream will be generated\n",
                      mainlevel);
        ret = OPJ_FALSE;
    } else {
        /* Validate sublevel */
        assert(sizeof(tabMaxSubLevelFromMainLevel) ==
               (OPJ_IMF_MAINLEVEL_MAX + 1) * sizeof(tabMaxSubLevelFromMainLevel[0]));
        if (sublevel > tabMaxSubLevelFromMainLevel[mainlevel]) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF profile require sublevel <= %d for mainlevel = %d.\n"
                          "-> %d is thus not compliant\n"
                          "-> Non-IMF codestream will be generated\n",
                          tabMaxSubLevelFromMainLevel[mainlevel],
                          mainlevel,
                          sublevel);
            ret = OPJ_FALSE;
        }
    }

    /* Number of components */
    if (image->numcomps > 3) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "IMF profiles require at most 3 components.\n"
                      "-> Number of components of input image (%d) is not compliant\n"
                      "-> Non-IMF codestream will be generated\n",
                      image->numcomps);
        ret = OPJ_FALSE;
    }

    if (image->x0 != 0 || image->y0 != 0) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "IMF profiles require image origin to be at 0,0.\n"
                      "-> %d,%d is not compliant\n"
                      "-> Non-IMF codestream will be generated\n",
                      image->x0, image->y0 != 0);
        ret = OPJ_FALSE;
    }

    if (parameters->cp_tx0 != 0 || parameters->cp_ty0 != 0) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "IMF profiles require tile origin to be at 0,0.\n"
                      "-> %d,%d is not compliant\n"
                      "-> Non-IMF codestream will be generated\n",
                      parameters->cp_tx0, parameters->cp_ty0);
        ret = OPJ_FALSE;
    }

    if (parameters->tile_size_on) {
        if (profile == OPJ_PROFILE_IMF_2K ||
                profile == OPJ_PROFILE_IMF_4K ||
                profile == OPJ_PROFILE_IMF_8K) {
            if ((OPJ_UINT32)parameters->cp_tdx < image->x1 ||
                    (OPJ_UINT32)parameters->cp_tdy < image->y1) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 2K/4K/8K single tile profiles require tile to be greater or equal to image size.\n"
                              "-> %d,%d is lesser than %d,%d\n"
                              "-> Non-IMF codestream will be generated\n",
                              parameters->cp_tdx,
                              parameters->cp_tdy,
                              image->x1,
                              image->y1);
                ret = OPJ_FALSE;
            }
        } else {
            if ((OPJ_UINT32)parameters->cp_tdx >= image->x1 &&
                    (OPJ_UINT32)parameters->cp_tdy >= image->y1) {
                /* ok */
            } else if (parameters->cp_tdx == 1024 &&
                       parameters->cp_tdy == 1024) {
                /* ok */
            } else if (parameters->cp_tdx == 2048 &&
                       parameters->cp_tdy == 2048 &&
                       (profile == OPJ_PROFILE_IMF_4K ||
                        profile == OPJ_PROFILE_IMF_8K)) {
                /* ok */
            } else if (parameters->cp_tdx == 4096 &&
                       parameters->cp_tdy == 4096 &&
                       profile == OPJ_PROFILE_IMF_8K) {
                /* ok */
            } else {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 2K_R/4K_R/8K_R single/multiple tile profiles "
                              "require tile to be greater or equal to image size,\n"
                              "or to be (1024,1024), or (2048,2048) for 4K_R/8K_R "
                              "or (4096,4096) for 8K_R.\n"
                              "-> %d,%d is non conformant\n"
                              "-> Non-IMF codestream will be generated\n",
                              parameters->cp_tdx,
                              parameters->cp_tdy);
                ret = OPJ_FALSE;
            }
        }
    }

    /* Bitdepth */
    for (i = 0; i < image->numcomps; i++) {
        if (!(image->comps[i].prec >= 8 && image->comps[i].prec <= 16) ||
                (image->comps[i].sgnd)) {
            char signed_str[] = "signed";
            char unsigned_str[] = "unsigned";
            char *tmp_str = image->comps[i].sgnd ? signed_str : unsigned_str;
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF profiles require precision of each component to b in [8-16] bits unsigned"
                          "-> At least component %d of input image (%d bits, %s) is not compliant\n"
                          "-> Non-IMF codestream will be generated\n",
                          i, image->comps[i].prec, tmp_str);
            ret = OPJ_FALSE;
        }
    }

    /* Sub-sampling */
    for (i = 0; i < image->numcomps; i++) {
        if (i == 0 && image->comps[i].dx != 1) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF profiles require XRSiz1 == 1. Here it is set to %d.\n"
                          "-> Non-IMF codestream will be generated\n",
                          image->comps[i].dx);
            ret = OPJ_FALSE;
        }
        if (i == 1 && image->comps[i].dx != 1 && image->comps[i].dx != 2) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF profiles require XRSiz2 == 1 or 2. Here it is set to %d.\n"
                          "-> Non-IMF codestream will be generated\n",
                          image->comps[i].dx);
            ret = OPJ_FALSE;
        }
        if (i > 1 && image->comps[i].dx != image->comps[i - 1].dx) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF profiles require XRSiz%d to be the same as XRSiz2. "
                          "Here it is set to %d instead of %d.\n"
                          "-> Non-IMF codestream will be generated\n",
                          i + 1, image->comps[i].dx, image->comps[i - 1].dx);
            ret = OPJ_FALSE;
        }
        if (image->comps[i].dy != 1) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF profiles require YRsiz == 1. "
                          "Here it is set to %d for component %d.\n"
                          "-> Non-IMF codestream will be generated\n",
                          image->comps[i].dy, i);
            ret = OPJ_FALSE;
        }
    }

    /* Image size */
    switch (profile) {
    case OPJ_PROFILE_IMF_2K:
    case OPJ_PROFILE_IMF_2K_R:
        if (((image->comps[0].w > 2048) | (image->comps[0].h > 1556))) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF 2K/2K_R profile require:\n"
                          "width <= 2048 and height <= 1556\n"
                          "-> Input image size %d x %d is not compliant\n"
                          "-> Non-IMF codestream will be generated\n",
                          image->comps[0].w, image->comps[0].h);
            ret = OPJ_FALSE;
        }
        break;
    case OPJ_PROFILE_IMF_4K:
    case OPJ_PROFILE_IMF_4K_R:
        if (((image->comps[0].w > 4096) | (image->comps[0].h > 3112))) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF 4K/4K_R profile require:\n"
                          "width <= 4096 and height <= 3112\n"
                          "-> Input image size %d x %d is not compliant\n"
                          "-> Non-IMF codestream will be generated\n",
                          image->comps[0].w, image->comps[0].h);
            ret = OPJ_FALSE;
        }
        break;
    case OPJ_PROFILE_IMF_8K:
    case OPJ_PROFILE_IMF_8K_R:
        if (((image->comps[0].w > 8192) | (image->comps[0].h > 6224))) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF 8K/8K_R profile require:\n"
                          "width <= 8192 and height <= 6224\n"
                          "-> Input image size %d x %d is not compliant\n"
                          "-> Non-IMF codestream will be generated\n",
                          image->comps[0].w, image->comps[0].h);
            ret = OPJ_FALSE;
        }
        break;
    default :
        assert(0);
        return OPJ_FALSE;
    }

    if (parameters->roi_compno != -1) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "IMF profile forbid RGN / region of interest marker.\n"
                      "-> Compression parameters specify a ROI\n"
                      "-> Non-IMF codestream will be generated\n");
        ret = OPJ_FALSE;
    }

    if (parameters->cblockw_init != 32 || parameters->cblockh_init != 32) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "IMF profile require code block size to be 32x32.\n"
                      "-> Compression parameters set it to %dx%d.\n"
                      "-> Non-IMF codestream will be generated\n",
                      parameters->cblockw_init,
                      parameters->cblockh_init);
        ret = OPJ_FALSE;
    }

    if (parameters->prog_order != OPJ_CPRL) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "IMF profile require progression order to be CPRL.\n"
                      "-> Compression parameters set it to %d.\n"
                      "-> Non-IMF codestream will be generated\n",
                      parameters->prog_order);
        ret = OPJ_FALSE;
    }

    if (parameters->numpocs != 0) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "IMF profile forbid POC markers.\n"
                      "-> Compression parameters set %d POC.\n"
                      "-> Non-IMF codestream will be generated\n",
                      parameters->numpocs);
        ret = OPJ_FALSE;
    }

    /* Codeblock style: no mode switch enabled */
    if (parameters->mode != 0) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "IMF profile forbid mode switch in code block style.\n"
                      "-> Compression parameters set code block style to %d.\n"
                      "-> Non-IMF codestream will be generated\n",
                      parameters->mode);
        ret = OPJ_FALSE;
    }

    if (profile == OPJ_PROFILE_IMF_2K ||
            profile == OPJ_PROFILE_IMF_4K ||
            profile == OPJ_PROFILE_IMF_8K) {
        /* Expect 9-7 transform */
        if (parameters->irreversible != 1) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF 2K/4K/8K profiles require 9-7 Irreversible Transform.\n"
                          "-> Compression parameters set it to reversible.\n"
                          "-> Non-IMF codestream will be generated\n");
            ret = OPJ_FALSE;
        }
    } else {
        /* Expect 5-3 transform */
        if (parameters->irreversible != 0) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF 2K/4K/8K profiles require 5-3 reversible Transform.\n"
                          "-> Compression parameters set it to irreversible.\n"
                          "-> Non-IMF codestream will be generated\n");
            ret = OPJ_FALSE;
        }
    }

    /* Number of layers */
    if (parameters->tcp_numlayers != 1) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "IMF 2K/4K/8K profiles require 1 single quality layer.\n"
                      "-> Number of layers is %d.\n"
                      "-> Non-IMF codestream will be generated\n",
                      parameters->tcp_numlayers);
        ret = OPJ_FALSE;
    }

    /* Decomposition levels */
    switch (profile) {
    case OPJ_PROFILE_IMF_2K:
        if (!(NL >= 1 && NL <= 5)) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF 2K profile requires 1 <= NL <= 5:\n"
                          "-> Number of decomposition levels is %d.\n"
                          "-> Non-IMF codestream will be generated\n",
                          NL);
            ret = OPJ_FALSE;
        }
        break;
    case OPJ_PROFILE_IMF_4K:
        if (!(NL >= 1 && NL <= 6)) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF 4K profile requires 1 <= NL <= 6:\n"
                          "-> Number of decomposition levels is %d.\n"
                          "-> Non-IMF codestream will be generated\n",
                          NL);
            ret = OPJ_FALSE;
        }
        break;
    case OPJ_PROFILE_IMF_8K:
        if (!(NL >= 1 && NL <= 7)) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF 8K profile requires 1 <= NL <= 7:\n"
                          "-> Number of decomposition levels is %d.\n"
                          "-> Non-IMF codestream will be generated\n",
                          NL);
            ret = OPJ_FALSE;
        }
        break;
    case OPJ_PROFILE_IMF_2K_R: {
        if (XTsiz >= 2048) {
            if (!(NL >= 1 && NL <= 5)) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 2K_R profile requires 1 <= NL <= 5 for XTsiz >= 2048:\n"
                              "-> Number of decomposition levels is %d.\n"
                              "-> Non-IMF codestream will be generated\n",
                              NL);
                ret = OPJ_FALSE;
            }
        } else if (XTsiz >= 1024) {
            if (!(NL >= 1 && NL <= 4)) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 2K_R profile requires 1 <= NL <= 4 for XTsiz in [1024,2048[:\n"
                              "-> Number of decomposition levels is %d.\n"
                              "-> Non-IMF codestream will be generated\n",
                              NL);
                ret = OPJ_FALSE;
            }
        }
        break;
    }
    case OPJ_PROFILE_IMF_4K_R: {
        if (XTsiz >= 4096) {
            if (!(NL >= 1 && NL <= 6)) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 4K_R profile requires 1 <= NL <= 6 for XTsiz >= 4096:\n"
                              "-> Number of decomposition levels is %d.\n"
                              "-> Non-IMF codestream will be generated\n",
                              NL);
                ret = OPJ_FALSE;
            }
        } else if (XTsiz >= 2048) {
            if (!(NL >= 1 && NL <= 5)) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 4K_R profile requires 1 <= NL <= 5 for XTsiz in [2048,4096[:\n"
                              "-> Number of decomposition levels is %d.\n"
                              "-> Non-IMF codestream will be generated\n",
                              NL);
                ret = OPJ_FALSE;
            }
        } else if (XTsiz >= 1024) {
            if (!(NL >= 1 && NL <= 4)) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 4K_R profile requires 1 <= NL <= 4 for XTsiz in [1024,2048[:\n"
                              "-> Number of decomposition levels is %d.\n"
                              "-> Non-IMF codestream will be generated\n",
                              NL);
                ret = OPJ_FALSE;
            }
        }
        break;
    }
    case OPJ_PROFILE_IMF_8K_R: {
        if (XTsiz >= 8192) {
            if (!(NL >= 1 && NL <= 7)) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 4K_R profile requires 1 <= NL <= 7 for XTsiz >= 8192:\n"
                              "-> Number of decomposition levels is %d.\n"
                              "-> Non-IMF codestream will be generated\n",
                              NL);
                ret = OPJ_FALSE;
            }
        } else if (XTsiz >= 4096) {
            if (!(NL >= 1 && NL <= 6)) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 4K_R profile requires 1 <= NL <= 6 for XTsiz in [4096,8192[:\n"
                              "-> Number of decomposition levels is %d.\n"
                              "-> Non-IMF codestream will be generated\n",
                              NL);
                ret = OPJ_FALSE;
            }
        } else if (XTsiz >= 2048) {
            if (!(NL >= 1 && NL <= 5)) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 4K_R profile requires 1 <= NL <= 5 for XTsiz in [2048,4096[:\n"
                              "-> Number of decomposition levels is %d.\n"
                              "-> Non-IMF codestream will be generated\n",
                              NL);
                ret = OPJ_FALSE;
            }
        } else if (XTsiz >= 1024) {
            if (!(NL >= 1 && NL <= 4)) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF 4K_R profile requires 1 <= NL <= 4 for XTsiz in [1024,2048[:\n"
                              "-> Number of decomposition levels is %d.\n"
                              "-> Non-IMF codestream will be generated\n",
                              NL);
                ret = OPJ_FALSE;
            }
        }
        break;
    }
    default:
        break;
    }

    if (parameters->numresolution == 1) {
        if (parameters->res_spec != 1 ||
                parameters->prcw_init[0] != 128 ||
                parameters->prch_init[0] != 128) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "IMF profiles require PPx = PPy = 7 for NLLL band, else 8.\n"
                          "-> Supplied values are different from that.\n"
                          "-> Non-IMF codestream will be generated\n");
            ret = OPJ_FALSE;
        }
    } else {
        int i;
        for (i = 0; i < parameters->res_spec; i++) {
            if (parameters->prcw_init[i] != 256 ||
                    parameters->prch_init[i] != 256) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "IMF profiles require PPx = PPy = 7 for NLLL band, else 8.\n"
                              "-> Supplied values are different from that.\n"
                              "-> Non-IMF codestream will be generated\n");
                ret = OPJ_FALSE;
            }
        }
    }

    return ret;
}

static OPJ_BOOL opj_j2k_add_mhmarker(opj_codestream_index_t *cstr_index,OPJ_UINT32 type, OPJ_OFF_T pos, OPJ_UINT32 len)
{
    assert(cstr_index != 00);

    /* expand the list? */
    if ((cstr_index->marknum + 1) > cstr_index->maxmarknum) {
        opj_marker_info_t *new_marker;
        cstr_index->maxmarknum = (OPJ_UINT32)(100 + (OPJ_FLOAT32)
                                              cstr_index->maxmarknum);
        new_marker = (opj_marker_info_t *) realloc(cstr_index->marker,
                     cstr_index->maxmarknum * sizeof(opj_marker_info_t));
        if (! new_marker) {
            free(cstr_index->marker);
            cstr_index->marker = NULL;
            cstr_index->maxmarknum = 0;
            cstr_index->marknum = 0;
            /* opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to add mh marker\n"); */
            return OPJ_FALSE;
        }
        cstr_index->marker = new_marker;
    }

    /* add the marker */
    cstr_index->marker[cstr_index->marknum].type = (OPJ_UINT16)type;
    cstr_index->marker[cstr_index->marknum].pos = (OPJ_INT32)pos;
    cstr_index->marker[cstr_index->marknum].len = (OPJ_INT32)len;
    cstr_index->marknum++;
    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_add_tlmarker(OPJ_UINT32 tileno,opj_codestream_index_t *cstr_index, OPJ_UINT32 type, OPJ_OFF_T pos,OPJ_UINT32 len)
{
    /* expand the list? */
    if ((cstr_index->tile_index[tileno].marknum + 1) >
            cstr_index->tile_index[tileno].maxmarknum) {
        opj_marker_info_t *new_marker;
        cstr_index->tile_index[tileno].maxmarknum = (OPJ_UINT32)(100 +
                (OPJ_FLOAT32) cstr_index->tile_index[tileno].maxmarknum);
        new_marker = (opj_marker_info_t *) realloc(
                         cstr_index->tile_index[tileno].marker,
                         cstr_index->tile_index[tileno].maxmarknum * sizeof(opj_marker_info_t));
        if (! new_marker) {
            free(cstr_index->tile_index[tileno].marker);
            cstr_index->tile_index[tileno].marker = NULL;
            cstr_index->tile_index[tileno].maxmarknum = 0;
            cstr_index->tile_index[tileno].marknum = 0;
            /* opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to add tl marker\n"); */
            return OPJ_FALSE;
        }
        cstr_index->tile_index[tileno].marker = new_marker;
    }

    /* add the marker */
    cstr_index->tile_index[tileno].marker[cstr_index->tile_index[tileno].marknum].type
        = (OPJ_UINT16)type;
    cstr_index->tile_index[tileno].marker[cstr_index->tile_index[tileno].marknum].pos
        = (OPJ_INT32)pos;
    cstr_index->tile_index[tileno].marker[cstr_index->tile_index[tileno].marknum].len
        = (OPJ_INT32)len;
    cstr_index->tile_index[tileno].marknum++;

    if (type == J2K_MS_SOT) {
        OPJ_UINT32 l_current_tile_part = cstr_index->tile_index[tileno].current_tpsno;

        if (cstr_index->tile_index[tileno].tp_index &&
                l_current_tile_part < cstr_index->tile_index[tileno].nb_tps) {
            cstr_index->tile_index[tileno].tp_index[l_current_tile_part].start_pos = pos;
        }

    }
    return OPJ_TRUE;
}

#pragma mark - read decompress

OPJ_BOOL opj_j2k_end_decompress(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    (void)p_j2k;
    (void)p_stream;
    (void)p_manager;
    return OPJ_TRUE;
}

OPJ_BOOL opj_j2k_read_header(opj_stream_private_t *p_stream,opj_j2k_t* p_j2k,opj_image_t** p_image,opj_event_mgr_t* p_manager)
{
    /* create an empty image header */
    p_j2k->m_private_image = opj_image_create0();
    if (! p_j2k->m_private_image) {
        return OPJ_FALSE;
    }


    /* validation of the parameters codec */
    if (! opj_j2k_exec(p_j2k, p_j2k->m_validation_list, p_stream, p_manager)) {
        opj_image_destroy(p_j2k->m_private_image);
        p_j2k->m_private_image = NULL;
        return OPJ_FALSE;
    }

    /* customization of the encoding */
    if  (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)opj_j2k_read_header_procedure, p_manager))
    {
        opj_image_destroy(p_j2k->m_private_image);
        p_j2k->m_private_image = NULL;
        return OPJ_FALSE;
    }

    /* read header */
    if (! opj_j2k_exec(p_j2k, p_j2k->m_procedure_list, p_stream, p_manager)) {
        opj_image_destroy(p_j2k->m_private_image);
        p_j2k->m_private_image = NULL;
        return OPJ_FALSE;
    }

    *p_image = opj_image_create0();
    if (!(*p_image)) {
        return OPJ_FALSE;
    }

    /* Copy codestream image information to the output image */
    opj_copy_image_header(p_j2k->m_private_image, *p_image);

    return OPJ_TRUE;
}

#pragma mark -

OPJ_BOOL opj_j2k_setup_mct_encoding(opj_tcp_t * p_tcp, opj_image_t * p_image)
{
    OPJ_UINT32 i;
    OPJ_UINT32 l_indix = 1;
    opj_mct_data_t * l_mct_deco_data = 00, * l_mct_offset_data = 00;
    opj_simple_mcc_decorrelation_data_t * l_mcc_data;
    OPJ_UINT32 l_mct_size, l_nb_elem;
    OPJ_FLOAT32 * l_data, * l_current_data;
    opj_tccp_t * l_tccp;

    /* preconditions */
    assert(p_tcp != 00);

    if (p_tcp->mct != 2) {
        return OPJ_TRUE;
    }

    if (p_tcp->m_mct_decoding_matrix) {
        if (p_tcp->m_nb_mct_records == p_tcp->m_nb_max_mct_records) {
            opj_mct_data_t *new_mct_records;
            p_tcp->m_nb_max_mct_records += OPJ_J2K_MCT_DEFAULT_NB_RECORDS;

            new_mct_records = (opj_mct_data_t *) realloc(p_tcp->m_mct_records,
                              p_tcp->m_nb_max_mct_records * sizeof(opj_mct_data_t));
            if (! new_mct_records) {
                free(p_tcp->m_mct_records);
                p_tcp->m_mct_records = NULL;
                p_tcp->m_nb_max_mct_records = 0;
                p_tcp->m_nb_mct_records = 0;
                /* opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to setup mct encoding\n"); */
                return OPJ_FALSE;
            }
            p_tcp->m_mct_records = new_mct_records;
            l_mct_deco_data = p_tcp->m_mct_records + p_tcp->m_nb_mct_records;

            memset(l_mct_deco_data, 0,
                   (p_tcp->m_nb_max_mct_records - p_tcp->m_nb_mct_records) * sizeof(
                       opj_mct_data_t));
        }
        l_mct_deco_data = p_tcp->m_mct_records + p_tcp->m_nb_mct_records;

        if (l_mct_deco_data->m_data) {
            free(l_mct_deco_data->m_data);
            l_mct_deco_data->m_data = 00;
        }

        l_mct_deco_data->m_index = l_indix++;
        l_mct_deco_data->m_array_type = MCT_TYPE_DECORRELATION;
        l_mct_deco_data->m_element_type = MCT_TYPE_FLOAT;
        l_nb_elem = p_image->numcomps * p_image->numcomps;
        l_mct_size = l_nb_elem * MCT_ELEMENT_SIZE[l_mct_deco_data->m_element_type];
        l_mct_deco_data->m_data = (OPJ_BYTE*)malloc(l_mct_size);

        if (! l_mct_deco_data->m_data) {
            return OPJ_FALSE;
        }

        j2k_mct_write_functions_from_float[l_mct_deco_data->m_element_type](
            p_tcp->m_mct_decoding_matrix, l_mct_deco_data->m_data, l_nb_elem);

        l_mct_deco_data->m_data_size = l_mct_size;
        ++p_tcp->m_nb_mct_records;
    }

    if (p_tcp->m_nb_mct_records == p_tcp->m_nb_max_mct_records) {
        opj_mct_data_t *new_mct_records;
        p_tcp->m_nb_max_mct_records += OPJ_J2K_MCT_DEFAULT_NB_RECORDS;
        new_mct_records = (opj_mct_data_t *) realloc(p_tcp->m_mct_records,
                          p_tcp->m_nb_max_mct_records * sizeof(opj_mct_data_t));
        if (! new_mct_records) {
            free(p_tcp->m_mct_records);
            p_tcp->m_mct_records = NULL;
            p_tcp->m_nb_max_mct_records = 0;
            p_tcp->m_nb_mct_records = 0;
            /* opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to setup mct encoding\n"); */
            return OPJ_FALSE;
        }
        p_tcp->m_mct_records = new_mct_records;
        l_mct_offset_data = p_tcp->m_mct_records + p_tcp->m_nb_mct_records;

        memset(l_mct_offset_data, 0,
               (p_tcp->m_nb_max_mct_records - p_tcp->m_nb_mct_records) * sizeof(
                   opj_mct_data_t));

        if (l_mct_deco_data) {
            l_mct_deco_data = l_mct_offset_data - 1;
        }
    }

    l_mct_offset_data = p_tcp->m_mct_records + p_tcp->m_nb_mct_records;

    if (l_mct_offset_data->m_data) {
        free(l_mct_offset_data->m_data);
        l_mct_offset_data->m_data = 00;
    }

    l_mct_offset_data->m_index = l_indix++;
    l_mct_offset_data->m_array_type = MCT_TYPE_OFFSET;
    l_mct_offset_data->m_element_type = MCT_TYPE_FLOAT;
    l_nb_elem = p_image->numcomps;
    l_mct_size = l_nb_elem * MCT_ELEMENT_SIZE[l_mct_offset_data->m_element_type];
    l_mct_offset_data->m_data = (OPJ_BYTE*)malloc(l_mct_size);

    if (! l_mct_offset_data->m_data) {
        return OPJ_FALSE;
    }

    l_data = (OPJ_FLOAT32*)malloc(l_nb_elem * sizeof(OPJ_FLOAT32));
    if (! l_data) {
        free(l_mct_offset_data->m_data);
        l_mct_offset_data->m_data = 00;
        return OPJ_FALSE;
    }

    l_tccp = p_tcp->tccps;
    l_current_data = l_data;

    for (i = 0; i < l_nb_elem; ++i) {
        *(l_current_data++) = (OPJ_FLOAT32)(l_tccp->m_dc_level_shift);
        ++l_tccp;
    }

    j2k_mct_write_functions_from_float[l_mct_offset_data->m_element_type](l_data,
            l_mct_offset_data->m_data, l_nb_elem);

    free(l_data);

    l_mct_offset_data->m_data_size = l_mct_size;

    ++p_tcp->m_nb_mct_records;

    if (p_tcp->m_nb_mcc_records == p_tcp->m_nb_max_mcc_records) {
        opj_simple_mcc_decorrelation_data_t *new_mcc_records;
        p_tcp->m_nb_max_mcc_records += OPJ_J2K_MCT_DEFAULT_NB_RECORDS;
        new_mcc_records = (opj_simple_mcc_decorrelation_data_t *) realloc(
                              p_tcp->m_mcc_records, p_tcp->m_nb_max_mcc_records * sizeof(
                                  opj_simple_mcc_decorrelation_data_t));
        if (! new_mcc_records) {
            free(p_tcp->m_mcc_records);
            p_tcp->m_mcc_records = NULL;
            p_tcp->m_nb_max_mcc_records = 0;
            p_tcp->m_nb_mcc_records = 0;
            /* opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to setup mct encoding\n"); */
            return OPJ_FALSE;
        }
        p_tcp->m_mcc_records = new_mcc_records;
        l_mcc_data = p_tcp->m_mcc_records + p_tcp->m_nb_mcc_records;
        memset(l_mcc_data, 0, (p_tcp->m_nb_max_mcc_records - p_tcp->m_nb_mcc_records) *
               sizeof(opj_simple_mcc_decorrelation_data_t));

    }

    l_mcc_data = p_tcp->m_mcc_records + p_tcp->m_nb_mcc_records;
    l_mcc_data->m_decorrelation_array = l_mct_deco_data;
    l_mcc_data->m_is_irreversible = 1;
    l_mcc_data->m_nb_comps = p_image->numcomps;
    l_mcc_data->m_index = l_indix++;
    l_mcc_data->m_offset_array = l_mct_offset_data;
    ++p_tcp->m_nb_mcc_records;

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_build_decoder(opj_j2k_t * p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    /* add here initialization of cp
       copy paste of setup_decoder */
    (void)p_j2k;
    (void)p_stream;
    (void)p_manager;
    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_build_encoder(opj_j2k_t * p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    /* add here initialization of cp
       copy paste of setup_encoder */
    (void)p_j2k;
    (void)p_stream;
    (void)p_manager;
    return OPJ_TRUE;
}


static OPJ_BOOL opj_j2k_decoding_validation(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_BOOL l_is_valid = OPJ_TRUE;
    OPJ_UNUSED(p_stream);
    OPJ_UNUSED(p_manager);

    /* STATE checking */
    /* make sure the state is at 0 */
#ifdef TODO_MSD
    l_is_valid &= (p_j2k->m_specific_param.m_decoder.m_state == J2K_DEC_STATE_NONE);
#endif
    l_is_valid &= (p_j2k->m_specific_param.m_decoder.m_state == 0x0000);

    /* POINTER validation */
    /* make sure a p_j2k codec is present */
    /* make sure a procedure list is present */
    l_is_valid &= (p_j2k->m_procedure_list != 00);
    /* make sure a validation list is present */
    l_is_valid &= (p_j2k->m_validation_list != 00);

    /* PARAMETER VALIDATION */
    return l_is_valid;
}

/** Fill p_j2k->cstr_index->tp_index[].start_pos/end_pos fields from TLM marker segments */
static void opj_j2k_build_tp_index_from_tlm(opj_j2k_t* p_j2k,opj_event_mgr_t * p_manager)
{
    opj_j2k_tlm_info_t* l_tlm;
    OPJ_UINT32 i;
    OPJ_OFF_T l_cur_offset;

    assert(p_j2k->cstr_index->main_head_end > 0);
    assert(p_j2k->cstr_index->nb_of_tiles > 0);
    assert(p_j2k->cstr_index->tile_index != NULL);

    l_tlm = &(p_j2k->m_specific_param.m_decoder.m_tlm);

    if (l_tlm->m_entries_count == 0) {
        l_tlm->m_is_invalid = OPJ_TRUE;
        return;
    }

    if (l_tlm->m_is_invalid) {
        return;
    }

    /* Initial pass to count the number of tile-parts per tile */
    for (i = 0; i < l_tlm->m_entries_count; ++i) {
        OPJ_UINT32 l_tile_index_no = l_tlm->m_tile_part_infos[i].m_tile_index;
        assert(l_tile_index_no < p_j2k->cstr_index->nb_of_tiles);
        p_j2k->cstr_index->tile_index[l_tile_index_no].tileno = l_tile_index_no;
        ++p_j2k->cstr_index->tile_index[l_tile_index_no].current_nb_tps;
    }

    /* Now check that all tiles have at least one tile-part */
    for (i = 0; i < p_j2k->cstr_index->nb_of_tiles; ++i) {
        if (p_j2k->cstr_index->tile_index[i].current_nb_tps == 0) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "opj_j2k_build_tp_index_from_tlm(): tile %d has no "
                          "registered tile-part in TLM marker segments.\n", i);
            goto error;
        }
    }

    /* Final pass to fill p_j2k->cstr_index */
    l_cur_offset = p_j2k->cstr_index->main_head_end;
    for (i = 0; i < l_tlm->m_entries_count; ++i) {
        OPJ_UINT32 l_tile_index_no = l_tlm->m_tile_part_infos[i].m_tile_index;
        opj_tile_index_t* l_tile_index = &
                                         (p_j2k->cstr_index->tile_index[l_tile_index_no]);
        if (!l_tile_index->tp_index) {
            l_tile_index->tp_index = (opj_tp_index_t *) calloc(
                                         l_tile_index->current_nb_tps, sizeof(opj_tp_index_t));
            if (! l_tile_index->tp_index) {
                opj_event_msg(p_manager, EVT_ERROR,
                              "opj_j2k_build_tp_index_from_tlm(): tile index allocation failed\n");
                goto error;
            }
        }

        assert(l_tile_index->nb_tps < l_tile_index->current_nb_tps);
        l_tile_index->tp_index[l_tile_index->nb_tps].start_pos = l_cur_offset;
        /* We don't know how to set the tp_index[].end_header field, but this is not really needed */
        /* If there would be no markers between SOT and SOD, that would be : */
        /* l_tile_index->tp_index[l_tile_index->nb_tps].end_header = l_cur_offset + 12; */
        l_tile_index->tp_index[l_tile_index->nb_tps].end_pos = l_cur_offset +
                l_tlm->m_tile_part_infos[i].m_length;
        ++l_tile_index->nb_tps;

        l_cur_offset += l_tlm->m_tile_part_infos[i].m_length;
    }

    return;

error:
    l_tlm->m_is_invalid = OPJ_TRUE;
    for (i = 0; i < l_tlm->m_entries_count; ++i) {
        OPJ_UINT32 l_tile_index = l_tlm->m_tile_part_infos[i].m_tile_index;
        p_j2k->cstr_index->tile_index[l_tile_index].current_nb_tps = 0;
        free(p_j2k->cstr_index->tile_index[l_tile_index].tp_index);
        p_j2k->cstr_index->tile_index[l_tile_index].tp_index = NULL;
    }
}

static OPJ_BOOL opj_j2k_read_header_procedure(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 l_current_marker;
    OPJ_UINT32 l_marker_size;
    const opj_dec_memory_marker_handler_t * l_marker_handler = 00;
    OPJ_BOOL l_has_siz = 0;
    OPJ_BOOL l_has_cod = 0;
    OPJ_BOOL l_has_qcd = 0;

    /* preconditions */
    assert(p_stream != 00);
    assert(p_j2k != 00);
    assert(p_manager != 00);

    /*  We enter in the main header */
    p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_MHSOC;

    /* Try to read the SOC marker, the codestream must begin with SOC marker */
    if (! opj_j2k_read_soc(p_j2k, p_stream, p_manager)) {
        opj_event_msg(p_manager, EVT_ERROR, "Expected a SOC marker \n");
        return OPJ_FALSE;
    }

    /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer */
    if (opj_stream_read_data(p_stream,
                             p_j2k->m_specific_param.m_decoder.m_header_data, 2, p_manager) != 2) {
        opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
        return OPJ_FALSE;
    }

    /* Read 2 bytes as the new marker ID */
    opj_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,
                   &l_current_marker, 2);

    /* Try to read until the SOT is detected */
    while (l_current_marker != J2K_MS_SOT) {

        /* Check if the current marker ID is valid */
        if (l_current_marker < 0xff00) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "A marker ID was expected (0xff--) instead of %.8x\n", l_current_marker);
            return OPJ_FALSE;
        }

        /* Get the marker handler from the marker ID */
        l_marker_handler = opj_j2k_get_marker_handler(l_current_marker);

        /* Manage case where marker is unknown */
        if (l_marker_handler->id == J2K_MS_UNK) {
            if (! opj_j2k_read_unk(p_j2k, p_stream, &l_current_marker, p_manager)) {
                opj_event_msg(p_manager, EVT_ERROR,
                              "Unknown marker has been detected and generated error.\n");
                return OPJ_FALSE;
            }

            if (l_current_marker == J2K_MS_SOT) {
                break;    /* SOT marker is detected main header is completely read */
            } else { /* Get the marker handler from the marker ID */
                l_marker_handler = opj_j2k_get_marker_handler(l_current_marker);
            }
        }

        if (l_marker_handler->id == J2K_MS_SIZ) {
            /* Mark required SIZ marker as found */
            l_has_siz = 1;
        }
        if (l_marker_handler->id == J2K_MS_COD) {
            /* Mark required COD marker as found */
            l_has_cod = 1;
        }
        if (l_marker_handler->id == J2K_MS_QCD) {
            /* Mark required QCD marker as found */
            l_has_qcd = 1;
        }

        /* Check if the marker is known and if it is the right place to find it */
        if (!(p_j2k->m_specific_param.m_decoder.m_state & l_marker_handler->states)) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Marker is not compliant with its position\n");
            return OPJ_FALSE;
        }

        /* Try to read 2 bytes (the marker size) from stream and copy them into the buffer */
        if (opj_stream_read_data(p_stream,
                                 p_j2k->m_specific_param.m_decoder.m_header_data, 2, p_manager) != 2) {
            opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
            return OPJ_FALSE;
        }

        /* read 2 bytes as the marker size */
        opj_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data, &l_marker_size,
                       2);
        if (l_marker_size < 2) {
            opj_event_msg(p_manager, EVT_ERROR, "Invalid marker size\n");
            return OPJ_FALSE;
        }
        l_marker_size -= 2; /* Subtract the size of the marker ID already read */

        /* Check if the marker size is compatible with the header data size */
        if (l_marker_size > p_j2k->m_specific_param.m_decoder.m_header_data_size) {
            OPJ_BYTE *new_header_data = (OPJ_BYTE *) realloc(
                                            p_j2k->m_specific_param.m_decoder.m_header_data, l_marker_size);
            if (! new_header_data) {
                free(p_j2k->m_specific_param.m_decoder.m_header_data);
                p_j2k->m_specific_param.m_decoder.m_header_data = NULL;
                p_j2k->m_specific_param.m_decoder.m_header_data_size = 0;
                opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to read header\n");
                return OPJ_FALSE;
            }
            p_j2k->m_specific_param.m_decoder.m_header_data = new_header_data;
            p_j2k->m_specific_param.m_decoder.m_header_data_size = l_marker_size;
        }

        /* Try to read the rest of the marker segment from stream and copy them into the buffer */
        if (opj_stream_read_data(p_stream,
                                 p_j2k->m_specific_param.m_decoder.m_header_data, l_marker_size,
                                 p_manager) != l_marker_size) {
            opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
            return OPJ_FALSE;
        }

        /* Read the marker segment with the correct marker handler */
        if (!(*(l_marker_handler->handler))(p_j2k,
                                            p_j2k->m_specific_param.m_decoder.m_header_data, l_marker_size, p_manager)) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Marker handler function failed to read the marker segment\n");
            return OPJ_FALSE;
        }

        /* Add the marker to the codestream index*/
        if (OPJ_FALSE == opj_j2k_add_mhmarker(
                    p_j2k->cstr_index,
                    l_marker_handler->id,
                    (OPJ_UINT32) opj_stream_tell(p_stream) - l_marker_size - 4,
                    l_marker_size + 4)) {
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to add mh marker\n");
            return OPJ_FALSE;
        }

        /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer */
        if (opj_stream_read_data(p_stream,
                                 p_j2k->m_specific_param.m_decoder.m_header_data, 2, p_manager) != 2) {
            opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
            return OPJ_FALSE;
        }

        /* read 2 bytes as the new marker ID */
        opj_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,
                       &l_current_marker, 2);
    }

    if (l_has_siz == 0) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "required SIZ marker not found in main header\n");
        return OPJ_FALSE;
    }
    if (l_has_cod == 0) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "required COD marker not found in main header\n");
        return OPJ_FALSE;
    }
    if (l_has_qcd == 0) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "required QCD marker not found in main header\n");
        return OPJ_FALSE;
    }

    if (! opj_j2k_merge_ppm(&(p_j2k->m_cp), p_manager)) {
        opj_event_msg(p_manager, EVT_ERROR, "Failed to merge PPM data\n");
        return OPJ_FALSE;
    }

    opj_event_msg(p_manager, EVT_INFO, "Main header has been correctly decoded.\n");

    /* Position of the last element if the main header */
    p_j2k->cstr_index->main_head_end = (OPJ_UINT32) opj_stream_tell(p_stream) - 2;

    /* Build tile-part index from TLM information */
    opj_j2k_build_tp_index_from_tlm(p_j2k, p_manager);

    /* Next step: read a tile-part header */
    p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_TPHSOT;

    return OPJ_TRUE;
}


/* FIXME DOC*/
static OPJ_BOOL opj_j2k_copy_default_tcp_and_create_tcd(opj_j2k_t * p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    opj_tcp_t * l_tcp = 00;
    opj_tcp_t * l_default_tcp = 00;
    OPJ_UINT32 l_nb_tiles;
    OPJ_UINT32 i, j;
    opj_tccp_t *l_current_tccp = 00;
    OPJ_UINT32 l_tccp_size;
    OPJ_UINT32 l_mct_size;
    opj_image_t * l_image;
    OPJ_UINT32 l_mcc_records_size, l_mct_records_size;
    opj_mct_data_t * l_src_mct_rec, *l_dest_mct_rec;
    opj_simple_mcc_decorrelation_data_t * l_src_mcc_rec, *l_dest_mcc_rec;
    OPJ_UINT32 l_offset;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_stream != 00);
    assert(p_manager != 00);

    OPJ_UNUSED(p_stream);

    l_image = p_j2k->m_private_image;
    l_nb_tiles = p_j2k->m_cp.th * p_j2k->m_cp.tw;
    l_tcp = p_j2k->m_cp.tcps;
    l_tccp_size = l_image->numcomps * (OPJ_UINT32)sizeof(opj_tccp_t);
    l_default_tcp = p_j2k->m_specific_param.m_decoder.m_default_tcp;
    l_mct_size = l_image->numcomps * l_image->numcomps * (OPJ_UINT32)sizeof(
                     OPJ_FLOAT32);

    /* For each tile */
    for (i = 0; i < l_nb_tiles; ++i) {
        /* keep the tile-compo coding parameters pointer of the current tile coding parameters*/
        l_current_tccp = l_tcp->tccps;
        /*Copy default coding parameters into the current tile coding parameters*/
        memcpy(l_tcp, l_default_tcp, sizeof(opj_tcp_t));
        /* Initialize some values of the current tile coding parameters*/
        l_tcp->cod = 0;
        l_tcp->ppt = 0;
        l_tcp->ppt_data = 00;
        l_tcp->m_current_tile_part_number = -1;
        /* Remove memory not owned by this tile in case of early error return. */
        l_tcp->m_mct_decoding_matrix = 00;
        l_tcp->m_nb_max_mct_records = 0;
        l_tcp->m_mct_records = 00;
        l_tcp->m_nb_max_mcc_records = 0;
        l_tcp->m_mcc_records = 00;
        /* Reconnect the tile-compo coding parameters pointer to the current tile coding parameters*/
        l_tcp->tccps = l_current_tccp;

        /* Get the mct_decoding_matrix of the dflt_tile_cp and copy them into the current tile cp*/
        if (l_default_tcp->m_mct_decoding_matrix) {
            l_tcp->m_mct_decoding_matrix = (OPJ_FLOAT32*)malloc(l_mct_size);
            if (! l_tcp->m_mct_decoding_matrix) {
                return OPJ_FALSE;
            }
            memcpy(l_tcp->m_mct_decoding_matrix, l_default_tcp->m_mct_decoding_matrix,
                   l_mct_size);
        }

        /* Get the mct_record of the dflt_tile_cp and copy them into the current tile cp*/
        l_mct_records_size = l_default_tcp->m_nb_max_mct_records * (OPJ_UINT32)sizeof(
                                 opj_mct_data_t);
        l_tcp->m_mct_records = (opj_mct_data_t*)malloc(l_mct_records_size);
        if (! l_tcp->m_mct_records) {
            return OPJ_FALSE;
        }
        memcpy(l_tcp->m_mct_records, l_default_tcp->m_mct_records, l_mct_records_size);

        /* Copy the mct record data from dflt_tile_cp to the current tile*/
        l_src_mct_rec = l_default_tcp->m_mct_records;
        l_dest_mct_rec = l_tcp->m_mct_records;

        for (j = 0; j < l_default_tcp->m_nb_mct_records; ++j) {

            if (l_src_mct_rec->m_data) {

                l_dest_mct_rec->m_data = (OPJ_BYTE*) malloc(l_src_mct_rec->m_data_size);
                if (! l_dest_mct_rec->m_data) {
                    return OPJ_FALSE;
                }
                memcpy(l_dest_mct_rec->m_data, l_src_mct_rec->m_data,
                       l_src_mct_rec->m_data_size);
            }

            ++l_src_mct_rec;
            ++l_dest_mct_rec;
            /* Update with each pass to free exactly what has been allocated on early return. */
            l_tcp->m_nb_max_mct_records += 1;
        }

        /* Get the mcc_record of the dflt_tile_cp and copy them into the current tile cp*/
        l_mcc_records_size = l_default_tcp->m_nb_max_mcc_records * (OPJ_UINT32)sizeof(
                                 opj_simple_mcc_decorrelation_data_t);
        l_tcp->m_mcc_records = (opj_simple_mcc_decorrelation_data_t*) malloc(
                                   l_mcc_records_size);
        if (! l_tcp->m_mcc_records) {
            return OPJ_FALSE;
        }
        memcpy(l_tcp->m_mcc_records, l_default_tcp->m_mcc_records, l_mcc_records_size);
        l_tcp->m_nb_max_mcc_records = l_default_tcp->m_nb_max_mcc_records;

        /* Copy the mcc record data from dflt_tile_cp to the current tile*/
        l_src_mcc_rec = l_default_tcp->m_mcc_records;
        l_dest_mcc_rec = l_tcp->m_mcc_records;

        for (j = 0; j < l_default_tcp->m_nb_max_mcc_records; ++j) {

            if (l_src_mcc_rec->m_decorrelation_array) {
                l_offset = (OPJ_UINT32)(l_src_mcc_rec->m_decorrelation_array -
                                        l_default_tcp->m_mct_records);
                l_dest_mcc_rec->m_decorrelation_array = l_tcp->m_mct_records + l_offset;
            }

            if (l_src_mcc_rec->m_offset_array) {
                l_offset = (OPJ_UINT32)(l_src_mcc_rec->m_offset_array -
                                        l_default_tcp->m_mct_records);
                l_dest_mcc_rec->m_offset_array = l_tcp->m_mct_records + l_offset;
            }

            ++l_src_mcc_rec;
            ++l_dest_mcc_rec;
        }

        /* Copy all the dflt_tile_compo_cp to the current tile cp */
        memcpy(l_current_tccp, l_default_tcp->tccps, l_tccp_size);

        /* Move to next tile cp*/
        ++l_tcp;
    }

    /* Create the current tile decoder*/
    p_j2k->m_tcd = opj_tcd_create(OPJ_TRUE);
    if (! p_j2k->m_tcd) {
        return OPJ_FALSE;
    }

    if (!opj_tcd_init(p_j2k->m_tcd, l_image, &(p_j2k->m_cp), p_j2k->m_tp)) {
        opj_tcd_destroy(p_j2k->m_tcd);
        p_j2k->m_tcd = 00;
        opj_event_msg(p_manager, EVT_ERROR, "Cannot decode tile, memory error\n");
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static const opj_dec_memory_marker_handler_t * opj_j2k_get_marker_handler(OPJ_UINT32 p_id)
{
    const opj_dec_memory_marker_handler_t *e;
    for (e = j2k_memory_marker_handler_tab; e->id != 0; ++e) {
        if (e->id == p_id) {
            break; /* we find a handler corresponding to the marker ID*/
        }
    }
    return e;
}

void opj_j2k_destroy(opj_j2k_t *p_j2k)
{
    if (p_j2k == 00) {
        return;
    }

    if (p_j2k->m_is_decoder) {

        if (p_j2k->m_specific_param.m_decoder.m_default_tcp != 00) {
            opj_j2k_tcp_destroy(p_j2k->m_specific_param.m_decoder.m_default_tcp);
            free(p_j2k->m_specific_param.m_decoder.m_default_tcp);
            p_j2k->m_specific_param.m_decoder.m_default_tcp = 00;
        }

        if (p_j2k->m_specific_param.m_decoder.m_header_data != 00) {
            free(p_j2k->m_specific_param.m_decoder.m_header_data);
            p_j2k->m_specific_param.m_decoder.m_header_data = 00;
            p_j2k->m_specific_param.m_decoder.m_header_data_size = 0;
        }

        free(p_j2k->m_specific_param.m_decoder.m_comps_indices_to_decode);
        p_j2k->m_specific_param.m_decoder.m_comps_indices_to_decode = 00;
        p_j2k->m_specific_param.m_decoder.m_numcomps_to_decode = 0;

        free(p_j2k->m_specific_param.m_decoder.m_tlm.m_tile_part_infos);
        p_j2k->m_specific_param.m_decoder.m_tlm.m_tile_part_infos = NULL;

        free(p_j2k->m_specific_param.m_decoder.m_intersecting_tile_parts_offset);
        p_j2k->m_specific_param.m_decoder.m_intersecting_tile_parts_offset = NULL;

    } else {

        if (p_j2k->m_specific_param.m_encoder.m_encoded_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_encoded_tile_data);
            p_j2k->m_specific_param.m_encoder.m_encoded_tile_data = 00;
        }

        if (p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer) {
            free(p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer);
            p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer = 00;
            p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current = 00;
        }

        if (p_j2k->m_specific_param.m_encoder.m_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = 00;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
        }
    }

    opj_tcd_destroy(p_j2k->m_tcd);

    opj_j2k_cp_destroy(&(p_j2k->m_cp));
    memset(&(p_j2k->m_cp), 0, sizeof(opj_cp_t));

    opj_procedure_list_destroy(p_j2k->m_procedure_list);
    p_j2k->m_procedure_list = 00;

    opj_procedure_list_destroy(p_j2k->m_validation_list);
    p_j2k->m_procedure_list = 00;

    j2k_destroy_cstr_index(p_j2k->cstr_index);
    p_j2k->cstr_index = NULL;

    opj_image_destroy(p_j2k->m_private_image);
    p_j2k->m_private_image = NULL;

    opj_image_destroy(p_j2k->m_output_image);
    p_j2k->m_output_image = NULL;

    opj_thread_pool_destroy(p_j2k->m_tp);
    p_j2k->m_tp = NULL;

    free(p_j2k);
}

void j2k_destroy_cstr_index(opj_codestream_index_t *p_cstr_ind)
{
    if (p_cstr_ind) {

        if (p_cstr_ind->marker) {
            free(p_cstr_ind->marker);
            p_cstr_ind->marker = NULL;
        }

        if (p_cstr_ind->tile_index) {
            OPJ_UINT32 it_tile = 0;

            for (it_tile = 0; it_tile < p_cstr_ind->nb_of_tiles; it_tile++) {

                if (p_cstr_ind->tile_index[it_tile].packet_index) {
                    free(p_cstr_ind->tile_index[it_tile].packet_index);
                    p_cstr_ind->tile_index[it_tile].packet_index = NULL;
                }

                if (p_cstr_ind->tile_index[it_tile].tp_index) {
                    free(p_cstr_ind->tile_index[it_tile].tp_index);
                    p_cstr_ind->tile_index[it_tile].tp_index = NULL;
                }

                if (p_cstr_ind->tile_index[it_tile].marker) {
                    free(p_cstr_ind->tile_index[it_tile].marker);
                    p_cstr_ind->tile_index[it_tile].marker = NULL;

                }
            }

            free(p_cstr_ind->tile_index);
            p_cstr_ind->tile_index = NULL;
        }

        free(p_cstr_ind);
    }
}

static void opj_j2k_tcp_destroy(opj_tcp_t *p_tcp)
{
    if (p_tcp == 00) {
        return;
    }

    if (p_tcp->ppt_markers != 00) {
        OPJ_UINT32 i;
        for (i = 0U; i < p_tcp->ppt_markers_count; ++i) {
            if (p_tcp->ppt_markers[i].m_data != NULL) {
                free(p_tcp->ppt_markers[i].m_data);
            }
        }
        p_tcp->ppt_markers_count = 0U;
        free(p_tcp->ppt_markers);
        p_tcp->ppt_markers = NULL;
    }

    if (p_tcp->ppt_buffer != 00) {
        free(p_tcp->ppt_buffer);
        p_tcp->ppt_buffer = 00;
    }

    if (p_tcp->tccps != 00) {
        free(p_tcp->tccps);
        p_tcp->tccps = 00;
    }

    if (p_tcp->m_mct_coding_matrix != 00) {
        free(p_tcp->m_mct_coding_matrix);
        p_tcp->m_mct_coding_matrix = 00;
    }

    if (p_tcp->m_mct_decoding_matrix != 00) {
        free(p_tcp->m_mct_decoding_matrix);
        p_tcp->m_mct_decoding_matrix = 00;
    }

    if (p_tcp->m_mcc_records) {
        free(p_tcp->m_mcc_records);
        p_tcp->m_mcc_records = 00;
        p_tcp->m_nb_max_mcc_records = 0;
        p_tcp->m_nb_mcc_records = 0;
    }

    if (p_tcp->m_mct_records) {
        opj_mct_data_t * l_mct_data = p_tcp->m_mct_records;
        OPJ_UINT32 i;

        for (i = 0; i < p_tcp->m_nb_mct_records; ++i) {
            if (l_mct_data->m_data) {
                free(l_mct_data->m_data);
                l_mct_data->m_data = 00;
            }

            ++l_mct_data;
        }

        free(p_tcp->m_mct_records);
        p_tcp->m_mct_records = 00;
    }

    if (p_tcp->mct_norms != 00) {
        free(p_tcp->mct_norms);
        p_tcp->mct_norms = 00;
    }

    opj_j2k_tcp_data_destroy(p_tcp);

}

static void opj_j2k_tcp_data_destroy(opj_tcp_t *p_tcp)
{
    if (p_tcp->m_data) {
        free(p_tcp->m_data);
        p_tcp->m_data = NULL;
        p_tcp->m_data_size = 0;
    }
}

static void opj_j2k_cp_destroy(opj_cp_t *p_cp)
{
    OPJ_UINT32 l_nb_tiles;
    opj_tcp_t * l_current_tile = 00;

    if (p_cp == 00) {
        return;
    }
    if (p_cp->tcps != 00) {
        OPJ_UINT32 i;
        l_current_tile = p_cp->tcps;
        l_nb_tiles = p_cp->th * p_cp->tw;

        for (i = 0U; i < l_nb_tiles; ++i) {
            opj_j2k_tcp_destroy(l_current_tile);
            ++l_current_tile;
        }
        free(p_cp->tcps);
        p_cp->tcps = 00;
    }
    if (p_cp->ppm_markers != 00) {
        OPJ_UINT32 i;
        for (i = 0U; i < p_cp->ppm_markers_count; ++i) {
            if (p_cp->ppm_markers[i].m_data != NULL) {
                free(p_cp->ppm_markers[i].m_data);
            }
        }
        p_cp->ppm_markers_count = 0U;
        free(p_cp->ppm_markers);
        p_cp->ppm_markers = NULL;
    }
    free(p_cp->ppm_buffer);
    p_cp->ppm_buffer = 00;
    p_cp->ppm_data =
        NULL; /* ppm_data belongs to the allocated buffer pointed by ppm_buffer */
    free(p_cp->comment);
    p_cp->comment = 00;
    if (! p_cp->m_is_decoder) {
        free(p_cp->m_specific_param.m_enc.m_matrice);
        p_cp->m_specific_param.m_enc.m_matrice = 00;
    }
}

static OPJ_BOOL opj_j2k_need_nb_tile_parts_correction(opj_stream_private_t *p_stream, OPJ_UINT32 tile_no, OPJ_BOOL* p_correction_needed,opj_event_mgr_t * p_manager)
{
    OPJ_BYTE   l_header_data[10];
    OPJ_OFF_T  l_stream_pos_backup;
    OPJ_UINT32 l_current_marker;
    OPJ_UINT32 l_marker_size;
    OPJ_UINT32 l_tile_no, l_tot_len, l_current_part, l_num_parts;

    /* initialize to no correction needed */
    *p_correction_needed = OPJ_FALSE;

    if (!opj_stream_has_seek(p_stream)) {
        /* We can't do much in this case, seek is needed */
        return OPJ_TRUE;
    }

    l_stream_pos_backup = opj_stream_tell(p_stream);
    if (l_stream_pos_backup == -1) {
        /* let's do nothing */
        return OPJ_TRUE;
    }

    for (;;) {
        /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer */
        if (opj_stream_read_data(p_stream, l_header_data, 2, p_manager) != 2) {
            /* assume all is OK */
            if (! opj_stream_seek(p_stream, l_stream_pos_backup, p_manager)) {
                return OPJ_FALSE;
            }
            return OPJ_TRUE;
        }

        /* Read 2 bytes from buffer as the new marker ID */
        opj_read_bytes(l_header_data, &l_current_marker, 2);

        if (l_current_marker != J2K_MS_SOT) {
            /* assume all is OK */
            if (! opj_stream_seek(p_stream, l_stream_pos_backup, p_manager)) {
                return OPJ_FALSE;
            }
            return OPJ_TRUE;
        }

        /* Try to read 2 bytes (the marker size) from stream and copy them into the buffer */
        if (opj_stream_read_data(p_stream, l_header_data, 2, p_manager) != 2) {
            opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
            return OPJ_FALSE;
        }

        /* Read 2 bytes from the buffer as the marker size */
        opj_read_bytes(l_header_data, &l_marker_size, 2);

        /* Check marker size for SOT Marker */
        if (l_marker_size != 10) {
            opj_event_msg(p_manager, EVT_ERROR, "Inconsistent marker size\n");
            return OPJ_FALSE;
        }
        l_marker_size -= 2;

        if (opj_stream_read_data(p_stream, l_header_data, l_marker_size,
                                 p_manager) != l_marker_size) {
            opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
            return OPJ_FALSE;
        }

        if (! opj_j2k_get_sot_values(l_header_data, l_marker_size, &l_tile_no,
                                     &l_tot_len, &l_current_part, &l_num_parts, p_manager)) {
            return OPJ_FALSE;
        }

        if (l_tile_no == tile_no) {
            /* we found what we were looking for */
            break;
        }

        if (l_tot_len < 14U) {
            /* last SOT until EOC or invalid Psot value */
            /* assume all is OK */
            if (! opj_stream_seek(p_stream, l_stream_pos_backup, p_manager)) {
                return OPJ_FALSE;
            }
            return OPJ_TRUE;
        }
        l_tot_len -= 12U;
        /* look for next SOT marker */
        if (opj_stream_skip(p_stream, (OPJ_OFF_T)(l_tot_len),
                            p_manager) != (OPJ_OFF_T)(l_tot_len)) {
            /* assume all is OK */
            if (! opj_stream_seek(p_stream, l_stream_pos_backup, p_manager)) {
                return OPJ_FALSE;
            }
            return OPJ_TRUE;
        }
    }

    /* check for correction */
    if (l_current_part == l_num_parts) {
        *p_correction_needed = OPJ_TRUE;
    }

    if (! opj_stream_seek(p_stream, l_stream_pos_backup, p_manager)) {
        return OPJ_FALSE;
    }
    return OPJ_TRUE;
}

OPJ_BOOL opj_j2k_read_tile_header(opj_j2k_t * p_j2k,
                                  OPJ_UINT32 * p_tile_index,
                                  OPJ_UINT32 * p_data_size,
                                  OPJ_INT32 * p_tile_x0, OPJ_INT32 * p_tile_y0,
                                  OPJ_INT32 * p_tile_x1, OPJ_INT32 * p_tile_y1,
                                  OPJ_UINT32 * p_nb_comps,
                                  OPJ_BOOL * p_go_on,
                                  opj_stream_private_t *p_stream,
                                  opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 l_current_marker = J2K_MS_SOT;
    OPJ_UINT32 l_marker_size;
    const opj_dec_memory_marker_handler_t * l_marker_handler = 00;
    opj_tcp_t * l_tcp = NULL;
    const OPJ_UINT32 l_nb_tiles = p_j2k->m_cp.tw * p_j2k->m_cp.th;

    /* Reach the End Of Codestream ?*/
    if (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_EOC) {
        l_current_marker = J2K_MS_EOC;
    }
    /* We need to encounter a SOT marker (a new tile-part header) */
    else if (p_j2k->m_specific_param.m_decoder.m_state != J2K_STATE_TPHSOT) {
        return OPJ_FALSE;
    }

    /* Read into the codestream until reach the EOC or ! can_decode ??? FIXME */
    while ((!p_j2k->m_specific_param.m_decoder.m_can_decode) &&
            (l_current_marker != J2K_MS_EOC)) {

        if (p_j2k->m_specific_param.m_decoder.m_num_intersecting_tile_parts > 0 &&
                p_j2k->m_specific_param.m_decoder.m_idx_intersecting_tile_parts <
                p_j2k->m_specific_param.m_decoder.m_num_intersecting_tile_parts) {
            OPJ_OFF_T next_tp_sot_pos;

            next_tp_sot_pos =
                p_j2k->m_specific_param.m_decoder.m_intersecting_tile_parts_offset[p_j2k->m_specific_param.m_decoder.m_idx_intersecting_tile_parts];
            ++p_j2k->m_specific_param.m_decoder.m_idx_intersecting_tile_parts;
            if (!(opj_stream_read_seek(p_stream,
                                       next_tp_sot_pos,
                                       p_manager))) {
                opj_event_msg(p_manager, EVT_ERROR, "Problem with seek function\n");
                return OPJ_FALSE;
            }

            /* Try to read 2 bytes (the marker ID) from stream and copy them into the buffer */
            if (opj_stream_read_data(p_stream,
                                     p_j2k->m_specific_param.m_decoder.m_header_data, 2, p_manager) != 2) {
                opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                return OPJ_FALSE;
            }

            /* Read 2 bytes from the buffer as the marker ID */
            opj_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,
                           &l_current_marker,
                           2);

            if (l_current_marker != J2K_MS_SOT) {
                opj_event_msg(p_manager, EVT_ERROR, "Did not get expected SOT marker\n");
                return OPJ_FALSE;
            }
        }

        /* Try to read until the Start Of Data is detected */
        while (l_current_marker != J2K_MS_SOD) {

            if (opj_stream_get_number_byte_left(p_stream) == 0) {
                p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_NEOC;
                break;
            }

            /* Try to read 2 bytes (the marker size) from stream and copy them into the buffer */
            if (opj_stream_read_data(p_stream,
                                     p_j2k->m_specific_param.m_decoder.m_header_data, 2, p_manager) != 2) {
                opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                return OPJ_FALSE;
            }

            /* Read 2 bytes from the buffer as the marker size */
            opj_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data, &l_marker_size,
                           2);

            /* Check marker size (does not include marker ID but includes marker size) */
            if (l_marker_size < 2) {
                opj_event_msg(p_manager, EVT_ERROR, "Inconsistent marker size\n");
                return OPJ_FALSE;
            }

            /* cf. https://code.google.com/p/openjpeg/issues/detail?id=226 */
            if (l_current_marker == 0x8080 &&
                    opj_stream_get_number_byte_left(p_stream) == 0) {
                p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_NEOC;
                break;
            }

            /* Why this condition? FIXME */
            if ((p_j2k->m_specific_param.m_decoder.m_state & J2K_STATE_TPH) &&
                    p_j2k->m_specific_param.m_decoder.m_sot_length != 0) {
                if (p_j2k->m_specific_param.m_decoder.m_sot_length < l_marker_size + 2) {
                    opj_event_msg(p_manager, EVT_ERROR,
                                  "Sot length is less than marker size + marker ID\n");
                    return OPJ_FALSE;
                }
                p_j2k->m_specific_param.m_decoder.m_sot_length -= (l_marker_size + 2);
            }
            l_marker_size -= 2; /* Subtract the size of the marker ID already read */

            /* Get the marker handler from the marker ID */
            l_marker_handler = opj_j2k_get_marker_handler(l_current_marker);

            /* Check if the marker is known and if it is the right place to find it */
            if (!(p_j2k->m_specific_param.m_decoder.m_state & l_marker_handler->states)) {
                opj_event_msg(p_manager, EVT_ERROR,
                              "Marker is not compliant with its position\n");
                return OPJ_FALSE;
            }
            /* FIXME manage case of unknown marker as in the main header ? */

            /* Check if the marker size is compatible with the header data size */
            if (l_marker_size > p_j2k->m_specific_param.m_decoder.m_header_data_size) {
                OPJ_BYTE *new_header_data = NULL;
                /* If we are here, this means we consider this marker as known & we will read it */
                /* Check enough bytes left in stream before allocation */
                if ((OPJ_OFF_T)l_marker_size >  opj_stream_get_number_byte_left(p_stream)) {
                    opj_event_msg(p_manager, EVT_ERROR,
                                  "Marker size inconsistent with stream length\n");
                    return OPJ_FALSE;
                }
                new_header_data = (OPJ_BYTE *) realloc(
                                      p_j2k->m_specific_param.m_decoder.m_header_data, l_marker_size);
                if (! new_header_data) {
                    free(p_j2k->m_specific_param.m_decoder.m_header_data);
                    p_j2k->m_specific_param.m_decoder.m_header_data = NULL;
                    p_j2k->m_specific_param.m_decoder.m_header_data_size = 0;
                    opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to read header\n");
                    return OPJ_FALSE;
                }
                p_j2k->m_specific_param.m_decoder.m_header_data = new_header_data;
                p_j2k->m_specific_param.m_decoder.m_header_data_size = l_marker_size;
            }

            /* Try to read the rest of the marker segment from stream and copy them into the buffer */
            if (opj_stream_read_data(p_stream,
                                     p_j2k->m_specific_param.m_decoder.m_header_data, l_marker_size,
                                     p_manager) != l_marker_size) {
                opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                return OPJ_FALSE;
            }

            if (!l_marker_handler->handler) {
                /* See issue #175 */
                opj_event_msg(p_manager, EVT_ERROR, "Not sure how that happened.\n");
                return OPJ_FALSE;
            }
            /* Read the marker segment with the correct marker handler */
            if (!(*(l_marker_handler->handler))(p_j2k,
                                                p_j2k->m_specific_param.m_decoder.m_header_data, l_marker_size, p_manager)) {
                opj_event_msg(p_manager, EVT_ERROR,
                              "Fail to read the current marker segment (%#x)\n", l_current_marker);
                return OPJ_FALSE;
            }

            /* Add the marker to the codestream index*/
            if (OPJ_FALSE == opj_j2k_add_tlmarker(p_j2k->m_current_tile_number,
                                                  p_j2k->cstr_index,
                                                  l_marker_handler->id,
                                                  (OPJ_UINT32) opj_stream_tell(p_stream) - l_marker_size - 4,
                                                  l_marker_size + 4)) {
                opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to add tl marker\n");
                return OPJ_FALSE;
            }

            /* Keep the position of the last SOT marker read */
            if (l_marker_handler->id == J2K_MS_SOT) {
                OPJ_UINT32 sot_pos = (OPJ_UINT32) opj_stream_tell(p_stream) - l_marker_size - 4
                                     ;
                if (sot_pos > p_j2k->m_specific_param.m_decoder.m_last_sot_read_pos) {
                    p_j2k->m_specific_param.m_decoder.m_last_sot_read_pos = sot_pos;
                }
            }

            if (p_j2k->m_specific_param.m_decoder.m_skip_data) {
                /* Skip the rest of the tile part header*/
                if (opj_stream_skip(p_stream, p_j2k->m_specific_param.m_decoder.m_sot_length,
                                    p_manager) != p_j2k->m_specific_param.m_decoder.m_sot_length) {
                    opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                    return OPJ_FALSE;
                }
                l_current_marker = J2K_MS_SOD; /* Normally we reached a SOD */
            } else {
                /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer*/
                if (opj_stream_read_data(p_stream,
                                         p_j2k->m_specific_param.m_decoder.m_header_data, 2, p_manager) != 2) {
                    opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                    return OPJ_FALSE;
                }
                /* Read 2 bytes from the buffer as the new marker ID */
                opj_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,
                               &l_current_marker, 2);
            }
        }
        if (opj_stream_get_number_byte_left(p_stream) == 0
                && p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_NEOC) {
            break;
        }

        /* If we didn't skip data before, we need to read the SOD marker*/
        if (! p_j2k->m_specific_param.m_decoder.m_skip_data) {
            /* Try to read the SOD marker and skip data ? FIXME */
            if (! opj_j2k_read_sod(p_j2k, p_stream, p_manager)) {
                return OPJ_FALSE;
            }

            /* Check if we can use the TLM index to access the next tile-part */
            if (!p_j2k->m_specific_param.m_decoder.m_can_decode &&
                    p_j2k->m_specific_param.m_decoder.m_tile_ind_to_dec >= 0 &&
                    p_j2k->m_current_tile_number == (OPJ_UINT32)
                    p_j2k->m_specific_param.m_decoder.m_tile_ind_to_dec &&
                    !p_j2k->m_specific_param.m_decoder.m_tlm.m_is_invalid &&
                    opj_stream_has_seek(p_stream)) {
                l_tcp = p_j2k->m_cp.tcps + p_j2k->m_current_tile_number;
                if (l_tcp->m_nb_tile_parts ==
                        p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].nb_tps &&
                        (OPJ_UINT32)l_tcp->m_current_tile_part_number + 1 < l_tcp->m_nb_tile_parts) {
                    const OPJ_OFF_T next_tp_sot_pos = p_j2k->cstr_index->tile_index[
                                                          p_j2k->m_current_tile_number].tp_index[l_tcp->m_current_tile_part_number +
                                                                  1].start_pos;

                    if (next_tp_sot_pos != opj_stream_tell(p_stream)) {
#if 0
                        opj_event_msg(p_manager, EVT_INFO,
                                      "opj_j2k_read_tile_header(tile=%u): seek to tile part %u at %" PRId64 "\n",
                                      p_j2k->m_current_tile_number,
                                      l_tcp->m_current_tile_part_number + 1,
                                      next_tp_sot_pos);
#endif

                        if (!(opj_stream_read_seek(p_stream,
                                                   next_tp_sot_pos,
                                                   p_manager))) {
                            opj_event_msg(p_manager, EVT_ERROR, "Problem with seek function\n");
                            return OPJ_FALSE;
                        }
                    }

                    /* Try to read 2 bytes (the marker ID) from stream and copy them into the buffer */
                    if (opj_stream_read_data(p_stream,
                                             p_j2k->m_specific_param.m_decoder.m_header_data, 2, p_manager) != 2) {
                        opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                        return OPJ_FALSE;
                    }

                    /* Read 2 bytes from the buffer as the marker ID */
                    opj_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,
                                   &l_current_marker,
                                   2);

                    if (l_current_marker != J2K_MS_SOT) {
                        opj_event_msg(p_manager, EVT_ERROR, "Did not get expected SOT marker\n");
                        return OPJ_FALSE;
                    }

                    continue;
                }
            }

            if (p_j2k->m_specific_param.m_decoder.m_can_decode &&
                    !p_j2k->m_specific_param.m_decoder.m_nb_tile_parts_correction_checked) {
                /* Issue 254 */
                OPJ_BOOL l_correction_needed = OPJ_FALSE;

                p_j2k->m_specific_param.m_decoder.m_nb_tile_parts_correction_checked = 1;
                if (p_j2k->m_cp.tcps[p_j2k->m_current_tile_number].m_nb_tile_parts == 1) {
                    /* Skip opj_j2k_need_nb_tile_parts_correction() if there is
                     * only a single tile part declared. The
                     * opj_j2k_need_nb_tile_parts_correction() hack was needed
                     * for files with 5 declared tileparts (where they were
                     * actually 6).
                     * Doing it systematically hurts performance when reading
                     * Sentinel2 L1C JPEG2000 files as explained in
                     * https://lists.osgeo.org/pipermail/gdal-dev/2024-November/059805.html
                     */
                } else if (!opj_j2k_need_nb_tile_parts_correction(p_stream,
                           p_j2k->m_current_tile_number, &l_correction_needed, p_manager)) {
                    opj_event_msg(p_manager, EVT_ERROR,
                                  "opj_j2k_apply_nb_tile_parts_correction error\n");
                    return OPJ_FALSE;
                }
                if (l_correction_needed) {
                    OPJ_UINT32 l_tile_no;

                    p_j2k->m_specific_param.m_decoder.m_can_decode = 0;
                    p_j2k->m_specific_param.m_decoder.m_nb_tile_parts_correction = 1;
                    /* correct tiles */
                    for (l_tile_no = 0U; l_tile_no < l_nb_tiles; ++l_tile_no) {
                        if (p_j2k->m_cp.tcps[l_tile_no].m_nb_tile_parts != 0U) {
                            p_j2k->m_cp.tcps[l_tile_no].m_nb_tile_parts += 1;
                        }
                    }
                    opj_event_msg(p_manager, EVT_WARNING,
                                  "Non conformant codestream TPsot==TNsot.\n");
                }
            }
        } else {
            /* Indicate we will try to read a new tile-part header*/
            p_j2k->m_specific_param.m_decoder.m_skip_data = 0;
            p_j2k->m_specific_param.m_decoder.m_can_decode = 0;
            p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_TPHSOT;
        }

        if (! p_j2k->m_specific_param.m_decoder.m_can_decode) {
            /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer */
            if (opj_stream_read_data(p_stream,
                                     p_j2k->m_specific_param.m_decoder.m_header_data, 2, p_manager) != 2) {

                /* Deal with likely non conformant SPOT6 files, where the last */
                /* row of tiles have TPsot == 0 and TNsot == 0, and missing EOC, */
                /* but no other tile-parts were found. */
                if (p_j2k->m_current_tile_number + 1 == l_nb_tiles) {
                    OPJ_UINT32 l_tile_no;
                    for (l_tile_no = 0U; l_tile_no < l_nb_tiles; ++l_tile_no) {
                        if (p_j2k->m_cp.tcps[l_tile_no].m_current_tile_part_number == 0 &&
                                p_j2k->m_cp.tcps[l_tile_no].m_nb_tile_parts == 0) {
                            break;
                        }
                    }
                    if (l_tile_no < l_nb_tiles) {
                        opj_event_msg(p_manager, EVT_INFO,
                                      "Tile %u has TPsot == 0 and TNsot == 0, "
                                      "but no other tile-parts were found. "
                                      "EOC is also missing.\n",
                                      l_tile_no);
                        p_j2k->m_current_tile_number = l_tile_no;
                        l_current_marker = J2K_MS_EOC;
                        p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_EOC;
                        break;
                    }
                }

                opj_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                return OPJ_FALSE;
            }

            /* Read 2 bytes from buffer as the new marker ID */
            opj_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,
                           &l_current_marker, 2);
        }
    }

    /* Current marker is the EOC marker ?*/
    if (l_current_marker == J2K_MS_EOC) {
        if (p_j2k->m_specific_param.m_decoder.m_state != J2K_STATE_EOC) {
            p_j2k->m_current_tile_number = 0;
            p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_EOC;
        }
    }

    /* Deal with tiles that have a single tile-part with TPsot == 0 and TNsot == 0 */
    if (! p_j2k->m_specific_param.m_decoder.m_can_decode) {
        l_tcp = p_j2k->m_cp.tcps + p_j2k->m_current_tile_number;

        while ((p_j2k->m_current_tile_number < l_nb_tiles) && (l_tcp->m_data == 00)) {
            ++p_j2k->m_current_tile_number;
            ++l_tcp;
        }

        if (p_j2k->m_current_tile_number == l_nb_tiles) {
            *p_go_on = OPJ_FALSE;
            return OPJ_TRUE;
        }
    }

    if (! opj_j2k_merge_ppt(p_j2k->m_cp.tcps + p_j2k->m_current_tile_number,
                            p_manager)) {
        opj_event_msg(p_manager, EVT_ERROR, "Failed to merge PPT data\n");
        return OPJ_FALSE;
    }
    /*FIXME ???*/
    if (! opj_tcd_init_decode_tile(p_j2k->m_tcd, p_j2k->m_current_tile_number,
                                   p_manager)) {
        opj_event_msg(p_manager, EVT_ERROR, "Cannot decode tile, memory error\n");
        return OPJ_FALSE;
    }

    opj_event_msg(p_manager, EVT_INFO, "Header of tile %d / %d has been read.\n",
                  p_j2k->m_current_tile_number + 1, (p_j2k->m_cp.th * p_j2k->m_cp.tw));

    *p_tile_index = p_j2k->m_current_tile_number;
    *p_go_on = OPJ_TRUE;
    if (p_data_size) {
        /* For internal use in j2k.c, we don't need this */
        /* This is just needed for folks using the opj_read_tile_header() / opj_decode_tile_data() combo */
        *p_data_size = opj_tcd_get_decoded_tile_size(p_j2k->m_tcd, OPJ_FALSE);
        if (*p_data_size == UINT_MAX) {
            return OPJ_FALSE;
        }
    }
    *p_tile_x0 = p_j2k->m_tcd->tcd_image->tiles->x0;
    *p_tile_y0 = p_j2k->m_tcd->tcd_image->tiles->y0;
    *p_tile_x1 = p_j2k->m_tcd->tcd_image->tiles->x1;
    *p_tile_y1 = p_j2k->m_tcd->tcd_image->tiles->y1;
    *p_nb_comps = p_j2k->m_tcd->tcd_image->tiles->numcomps;

    p_j2k->m_specific_param.m_decoder.m_state |= J2K_STATE_DATA;

    return OPJ_TRUE;
}

OPJ_BOOL opj_j2k_decode_tile(opj_j2k_t * p_j2k,
                             OPJ_UINT32 p_tile_index,
                             OPJ_BYTE * p_data,
                             OPJ_UINT32 p_data_size,
                             opj_stream_private_t *p_stream,
                             opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 l_current_marker;
    OPJ_BYTE l_data [2];
    opj_tcp_t * l_tcp;
    opj_image_t* l_image_for_bounds;

    /* preconditions */
    assert(p_stream != 00);
    assert(p_j2k != 00);
    assert(p_manager != 00);

    if (!(p_j2k->m_specific_param.m_decoder.m_state & J2K_STATE_DATA)
            || (p_tile_index != p_j2k->m_current_tile_number)) {
        return OPJ_FALSE;
    }

    l_tcp = &(p_j2k->m_cp.tcps[p_tile_index]);
    if (! l_tcp->m_data) {
        opj_j2k_tcp_destroy(l_tcp);
        return OPJ_FALSE;
    }

    /* When using the opj_read_tile_header / opj_decode_tile_data API */
    /* such as in test_tile_decoder, m_output_image is NULL, so fall back */
    /* to the full image dimension. This is a bit surprising that */
    /* opj_set_decode_area() is only used to determine intersecting tiles, */
    /* but full tile decoding is done */
    l_image_for_bounds = p_j2k->m_output_image ? p_j2k->m_output_image :
                         p_j2k->m_private_image;
    if (! opj_tcd_decode_tile(p_j2k->m_tcd,
                              l_image_for_bounds->x0,
                              l_image_for_bounds->y0,
                              l_image_for_bounds->x1,
                              l_image_for_bounds->y1,
                              p_j2k->m_specific_param.m_decoder.m_numcomps_to_decode,
                              p_j2k->m_specific_param.m_decoder.m_comps_indices_to_decode,
                              l_tcp->m_data,
                              l_tcp->m_data_size,
                              p_tile_index,
                              p_j2k->cstr_index, p_manager)) {
        opj_j2k_tcp_destroy(l_tcp);
        p_j2k->m_specific_param.m_decoder.m_state |= J2K_STATE_ERR;
        opj_event_msg(p_manager, EVT_ERROR, "Failed to decode.\n");
        return OPJ_FALSE;
    }

    /* p_data can be set to NULL when the call will take care of using */
    /* itself the TCD data. This is typically the case for whole single */
    /* tile decoding optimization. */
    if (p_data != NULL) {
        if (! opj_tcd_update_tile_data(p_j2k->m_tcd, p_data, p_data_size)) {
            return OPJ_FALSE;
        }

        /* To avoid to destroy the tcp which can be useful when we try to decode a tile decoded before (cf j2k_random_tile_access)
        * we destroy just the data which will be re-read in read_tile_header*/
        /*opj_j2k_tcp_destroy(l_tcp);
        p_j2k->m_tcd->tcp = 0;*/
        opj_j2k_tcp_data_destroy(l_tcp);
    }

    p_j2k->m_specific_param.m_decoder.m_can_decode = 0;
    p_j2k->m_specific_param.m_decoder.m_state &= (~(OPJ_UINT32)J2K_STATE_DATA);

    if (opj_stream_get_number_byte_left(p_stream) == 0
            && p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_NEOC) {
        return OPJ_TRUE;
    }

    if (p_j2k->m_specific_param.m_decoder.m_state != J2K_STATE_EOC) {
        if (opj_stream_read_data(p_stream, l_data, 2, p_manager) != 2) {
            opj_event_msg(p_manager, p_j2k->m_cp.strict ? EVT_ERROR : EVT_WARNING,
                          "Stream too short\n");
            return p_j2k->m_cp.strict ? OPJ_FALSE : OPJ_TRUE;
        }
        opj_read_bytes(l_data, &l_current_marker, 2);

        if (l_current_marker == J2K_MS_EOC) {
            p_j2k->m_current_tile_number = 0;
            p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_EOC;
        } else if (l_current_marker != J2K_MS_SOT) {
            if (opj_stream_get_number_byte_left(p_stream) == 0) {
                p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_NEOC;
                opj_event_msg(p_manager, EVT_WARNING, "Stream does not end with EOC\n");
                return OPJ_TRUE;
            }
            opj_event_msg(p_manager, EVT_ERROR, "Stream too short, expected SOT\n");
            return OPJ_FALSE;
        }
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_update_image_data(opj_tcd_t * p_tcd,opj_image_t* p_output_image)
{
    OPJ_UINT32 i, j;
    OPJ_UINT32 l_width_src, l_height_src;
    OPJ_UINT32 l_width_dest, l_height_dest;
    OPJ_INT32 l_offset_x0_src, l_offset_y0_src, l_offset_x1_src, l_offset_y1_src;
    OPJ_SIZE_T l_start_offset_src;
    OPJ_UINT32 l_start_x_dest, l_start_y_dest;
    OPJ_UINT32 l_x0_dest, l_y0_dest, l_x1_dest, l_y1_dest;
    OPJ_SIZE_T l_start_offset_dest;

    opj_image_comp_t * l_img_comp_src = 00;
    opj_image_comp_t * l_img_comp_dest = 00;

    opj_tcd_tilecomp_t * l_tilec = 00;
    opj_image_t * l_image_src = 00;
    OPJ_INT32 * l_dest_ptr;

    l_tilec = p_tcd->tcd_image->tiles->comps;
    l_image_src = p_tcd->image;
    l_img_comp_src = l_image_src->comps;

    l_img_comp_dest = p_output_image->comps;

    for (i = 0; i < l_image_src->numcomps;
            i++, ++l_img_comp_dest, ++l_img_comp_src,  ++l_tilec) {
        OPJ_INT32 res_x0, res_x1, res_y0, res_y1;
        OPJ_UINT32 src_data_stride;
        const OPJ_INT32* p_src_data;

        /* Copy info from decoded comp image to output image */
        l_img_comp_dest->resno_decoded = l_img_comp_src->resno_decoded;

        if (p_tcd->whole_tile_decoding) {
            opj_tcd_resolution_t* l_res = l_tilec->resolutions +
                                          l_img_comp_src->resno_decoded;
            res_x0 = l_res->x0;
            res_y0 = l_res->y0;
            res_x1 = l_res->x1;
            res_y1 = l_res->y1;
            src_data_stride = (OPJ_UINT32)(
                                  l_tilec->resolutions[l_tilec->minimum_num_resolutions - 1].x1 -
                                  l_tilec->resolutions[l_tilec->minimum_num_resolutions - 1].x0);
            p_src_data = l_tilec->data;
        } else {
            opj_tcd_resolution_t* l_res = l_tilec->resolutions +
                                          l_img_comp_src->resno_decoded;
            res_x0 = (OPJ_INT32)l_res->win_x0;
            res_y0 = (OPJ_INT32)l_res->win_y0;
            res_x1 = (OPJ_INT32)l_res->win_x1;
            res_y1 = (OPJ_INT32)l_res->win_y1;
            src_data_stride = l_res->win_x1 - l_res->win_x0;
            p_src_data = l_tilec->data_win;
        }

        if (p_src_data == NULL) {
            /* Happens for partial component decoding */
            continue;
        }

        l_width_src = (OPJ_UINT32)(res_x1 - res_x0);
        l_height_src = (OPJ_UINT32)(res_y1 - res_y0);


        /* Current tile component size*/
        /*if (i == 0) {
        fprintf(stdout, "SRC: l_res_x0=%d, l_res_x1=%d, l_res_y0=%d, l_res_y1=%d\n",
                        res_x0, res_x1, res_y0, res_y1);
        }*/


        /* Border of the current output component*/
        l_x0_dest = opj_uint_ceildivpow2(l_img_comp_dest->x0, l_img_comp_dest->factor);
        l_y0_dest = opj_uint_ceildivpow2(l_img_comp_dest->y0, l_img_comp_dest->factor);
        l_x1_dest = l_x0_dest +
                    l_img_comp_dest->w; /* can't overflow given that image->x1 is uint32 */
        l_y1_dest = l_y0_dest + l_img_comp_dest->h;

        /*if (i == 0) {
        fprintf(stdout, "DEST: l_x0_dest=%d, l_x1_dest=%d, l_y0_dest=%d, l_y1_dest=%d (%d)\n",
                        l_x0_dest, l_x1_dest, l_y0_dest, l_y1_dest, l_img_comp_dest->factor );
        }*/

        /*-----*/
        /* Compute the area (l_offset_x0_src, l_offset_y0_src, l_offset_x1_src, l_offset_y1_src)
         * of the input buffer (decoded tile component) which will be move
         * in the output buffer. Compute the area of the output buffer (l_start_x_dest,
         * l_start_y_dest, l_width_dest, l_height_dest)  which will be modified
         * by this input area.
         * */
        assert(res_x0 >= 0);
        assert(res_x1 >= 0);
        if (l_x0_dest < (OPJ_UINT32)res_x0) {
            l_start_x_dest = (OPJ_UINT32)res_x0 - l_x0_dest;
            l_offset_x0_src = 0;

            if (l_x1_dest >= (OPJ_UINT32)res_x1) {
                l_width_dest = l_width_src;
                l_offset_x1_src = 0;
            } else {
                l_width_dest = l_x1_dest - (OPJ_UINT32)res_x0 ;
                l_offset_x1_src = (OPJ_INT32)(l_width_src - l_width_dest);
            }
        } else {
            l_start_x_dest = 0U;
            l_offset_x0_src = (OPJ_INT32)l_x0_dest - res_x0;

            if (l_x1_dest >= (OPJ_UINT32)res_x1) {
                l_width_dest = l_width_src - (OPJ_UINT32)l_offset_x0_src;
                l_offset_x1_src = 0;
            } else {
                l_width_dest = l_img_comp_dest->w ;
                l_offset_x1_src = res_x1 - (OPJ_INT32)l_x1_dest;
            }
        }

        if (l_y0_dest < (OPJ_UINT32)res_y0) {
            l_start_y_dest = (OPJ_UINT32)res_y0 - l_y0_dest;
            l_offset_y0_src = 0;

            if (l_y1_dest >= (OPJ_UINT32)res_y1) {
                l_height_dest = l_height_src;
                l_offset_y1_src = 0;
            } else {
                l_height_dest = l_y1_dest - (OPJ_UINT32)res_y0 ;
                l_offset_y1_src = (OPJ_INT32)(l_height_src - l_height_dest);
            }
        } else {
            l_start_y_dest = 0U;
            l_offset_y0_src = (OPJ_INT32)l_y0_dest - res_y0;

            if (l_y1_dest >= (OPJ_UINT32)res_y1) {
                l_height_dest = l_height_src - (OPJ_UINT32)l_offset_y0_src;
                l_offset_y1_src = 0;
            } else {
                l_height_dest = l_img_comp_dest->h ;
                l_offset_y1_src = res_y1 - (OPJ_INT32)l_y1_dest;
            }
        }

        if ((l_offset_x0_src < 0) || (l_offset_y0_src < 0) || (l_offset_x1_src < 0) ||
                (l_offset_y1_src < 0)) {
            return OPJ_FALSE;
        }
        /* testcase 2977.pdf.asan.67.2198 */
        if ((OPJ_INT32)l_width_dest < 0 || (OPJ_INT32)l_height_dest < 0) {
            return OPJ_FALSE;
        }
        /*-----*/

        /* Compute the input buffer offset */
        l_start_offset_src = (OPJ_SIZE_T)l_offset_x0_src + (OPJ_SIZE_T)l_offset_y0_src
                             * (OPJ_SIZE_T)src_data_stride;

        /* Compute the output buffer offset */
        l_start_offset_dest = (OPJ_SIZE_T)l_start_x_dest + (OPJ_SIZE_T)l_start_y_dest
                              * (OPJ_SIZE_T)l_img_comp_dest->w;

        /* Allocate output component buffer if necessary */
        if (l_img_comp_dest->data == NULL &&
                l_start_offset_src == 0 && l_start_offset_dest == 0 &&
                src_data_stride == l_img_comp_dest->w &&
                l_width_dest == l_img_comp_dest->w &&
                l_height_dest == l_img_comp_dest->h) {
            /* If the final image matches the tile buffer, then borrow it */
            /* directly to save a copy */
            if (p_tcd->whole_tile_decoding) {
                l_img_comp_dest->data = l_tilec->data;
                l_tilec->data = NULL;
            } else {
                l_img_comp_dest->data = l_tilec->data_win;
                l_tilec->data_win = NULL;
            }
            continue;
        } else if (l_img_comp_dest->data == NULL) {
            OPJ_SIZE_T l_width = l_img_comp_dest->w;
            OPJ_SIZE_T l_height = l_img_comp_dest->h;

            if ((l_height == 0U) || (l_width > (SIZE_MAX / l_height)) ||
                    l_width * l_height > SIZE_MAX / sizeof(OPJ_INT32)) {
                /* would overflow */
                return OPJ_FALSE;
            }
            l_img_comp_dest->data = (OPJ_INT32*) opj_image_data_alloc(l_width * l_height *
                                    sizeof(OPJ_INT32));
            if (! l_img_comp_dest->data) {
                return OPJ_FALSE;
            }

            if (l_img_comp_dest->w != l_width_dest ||
                    l_img_comp_dest->h != l_height_dest) {
                memset(l_img_comp_dest->data, 0,
                       (OPJ_SIZE_T)l_img_comp_dest->w * l_img_comp_dest->h * sizeof(OPJ_INT32));
            }
        }

        /* Move the output buffer to the first place where we will write*/
        l_dest_ptr = l_img_comp_dest->data + l_start_offset_dest;

        {
            const OPJ_INT32 * l_src_ptr = p_src_data;
            l_src_ptr += l_start_offset_src;

            for (j = 0; j < l_height_dest; ++j) {
                memcpy(l_dest_ptr, l_src_ptr, l_width_dest * sizeof(OPJ_INT32));
                l_dest_ptr += l_img_comp_dest->w;
                l_src_ptr += src_data_stride;
            }
        }


    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_update_image_dimensions(opj_image_t* p_image,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 it_comp;
    OPJ_INT32 l_comp_x1, l_comp_y1;
    opj_image_comp_t* l_img_comp = NULL;

    l_img_comp = p_image->comps;
    for (it_comp = 0; it_comp < p_image->numcomps; ++it_comp) {
        OPJ_INT32 l_h, l_w;
        if (p_image->x0 > (OPJ_UINT32)INT_MAX ||
                p_image->y0 > (OPJ_UINT32)INT_MAX ||
                p_image->x1 > (OPJ_UINT32)INT_MAX ||
                p_image->y1 > (OPJ_UINT32)INT_MAX) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Image coordinates above INT_MAX are not supported\n");
            return OPJ_FALSE;
        }

        l_img_comp->x0 = opj_uint_ceildiv(p_image->x0, l_img_comp->dx);
        l_img_comp->y0 = opj_uint_ceildiv(p_image->y0, l_img_comp->dy);
        l_comp_x1 = opj_int_ceildiv((OPJ_INT32)p_image->x1, (OPJ_INT32)l_img_comp->dx);
        l_comp_y1 = opj_int_ceildiv((OPJ_INT32)p_image->y1, (OPJ_INT32)l_img_comp->dy);

        l_w = opj_int_ceildivpow2(l_comp_x1, (OPJ_INT32)l_img_comp->factor)
              - opj_int_ceildivpow2((OPJ_INT32)l_img_comp->x0, (OPJ_INT32)l_img_comp->factor);
        if (l_w < 0) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Size x of the decoded component image is incorrect (comp[%d].w=%d).\n",
                          it_comp, l_w);
            return OPJ_FALSE;
        }
        l_img_comp->w = (OPJ_UINT32)l_w;

        l_h = opj_int_ceildivpow2(l_comp_y1, (OPJ_INT32)l_img_comp->factor)
              - opj_int_ceildivpow2((OPJ_INT32)l_img_comp->y0, (OPJ_INT32)l_img_comp->factor);
        if (l_h < 0) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Size y of the decoded component image is incorrect (comp[%d].h=%d).\n",
                          it_comp, l_h);
            return OPJ_FALSE;
        }
        l_img_comp->h = (OPJ_UINT32)l_h;

        l_img_comp++;
    }

    return OPJ_TRUE;
}

OPJ_BOOL opj_j2k_set_decoded_components(opj_j2k_t *p_j2k,OPJ_UINT32 numcomps,const OPJ_UINT32* comps_indices,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 i;
    OPJ_BOOL* already_mapped;

    if (p_j2k->m_private_image == NULL) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "opj_read_header() should be called before "
                      "opj_set_decoded_components().\n");
        return OPJ_FALSE;
    }

    already_mapped = (OPJ_BOOL*) calloc(sizeof(OPJ_BOOL),
                                            p_j2k->m_private_image->numcomps);
    if (already_mapped == NULL) {
        return OPJ_FALSE;
    }

    for (i = 0; i < numcomps; i++) {
        if (comps_indices[i] >= p_j2k->m_private_image->numcomps) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Invalid component index: %u\n",
                          comps_indices[i]);
            free(already_mapped);
            return OPJ_FALSE;
        }
        if (already_mapped[comps_indices[i]]) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Component index %u used several times\n",
                          comps_indices[i]);
            free(already_mapped);
            return OPJ_FALSE;
        }
        already_mapped[comps_indices[i]] = OPJ_TRUE;
    }
    free(already_mapped);

    free(p_j2k->m_specific_param.m_decoder.m_comps_indices_to_decode);
    if (numcomps) {
        p_j2k->m_specific_param.m_decoder.m_comps_indices_to_decode =
            (OPJ_UINT32*) malloc(numcomps * sizeof(OPJ_UINT32));
        if (p_j2k->m_specific_param.m_decoder.m_comps_indices_to_decode == NULL) {
            p_j2k->m_specific_param.m_decoder.m_numcomps_to_decode = 0;
            return OPJ_FALSE;
        }
        memcpy(p_j2k->m_specific_param.m_decoder.m_comps_indices_to_decode,
               comps_indices,
               numcomps * sizeof(OPJ_UINT32));
    } else {
        p_j2k->m_specific_param.m_decoder.m_comps_indices_to_decode = NULL;
    }
    p_j2k->m_specific_param.m_decoder.m_numcomps_to_decode = numcomps;

    return OPJ_TRUE;
}


OPJ_BOOL opj_j2k_set_decode_area(opj_j2k_t *p_j2k,opj_image_t* p_image,OPJ_INT32 p_start_x, OPJ_INT32 p_start_y,OPJ_INT32 p_end_x, OPJ_INT32 p_end_y,opj_event_mgr_t * p_manager)
{
    opj_cp_t * l_cp = &(p_j2k->m_cp);
    opj_image_t * l_image = p_j2k->m_private_image;
    OPJ_BOOL ret;
    OPJ_UINT32 it_comp;

    if (p_j2k->m_cp.tw == 1 && p_j2k->m_cp.th == 1 &&
            p_j2k->m_cp.tcps[0].m_data != NULL) {
        /* In the case of a single-tiled image whose codestream we have already */
        /* ingested, go on */
    }
    /* Check if we are read the main header */
    else if (p_j2k->m_specific_param.m_decoder.m_state != J2K_STATE_TPHSOT) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Need to decode the main header before begin to decode the remaining codestream.\n");
        return OPJ_FALSE;
    }

    /* Update the comps[].factor member of the output image with the one */
    /* of m_reduce */
    for (it_comp = 0; it_comp < p_image->numcomps; ++it_comp) {
        p_image->comps[it_comp].factor = p_j2k->m_cp.m_specific_param.m_dec.m_reduce;
    }

    if (!p_start_x && !p_start_y && !p_end_x && !p_end_y) {
        opj_event_msg(p_manager, EVT_INFO,
                      "No decoded area parameters, set the decoded area to the whole image\n");

        p_j2k->m_specific_param.m_decoder.m_start_tile_x = 0;
        p_j2k->m_specific_param.m_decoder.m_start_tile_y = 0;
        p_j2k->m_specific_param.m_decoder.m_end_tile_x = l_cp->tw;
        p_j2k->m_specific_param.m_decoder.m_end_tile_y = l_cp->th;

        p_image->x0 = l_image->x0;
        p_image->y0 = l_image->y0;
        p_image->x1 = l_image->x1;
        p_image->y1 = l_image->y1;

        return opj_j2k_update_image_dimensions(p_image, p_manager);
    }

    /* ----- */
    /* Check if the positions provided by the user are correct */

    /* Left */
    if (p_start_x < 0) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Left position of the decoded area (region_x0=%d) should be >= 0.\n",
                      p_start_x);
        return OPJ_FALSE;
    } else if ((OPJ_UINT32)p_start_x > l_image->x1) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Left position of the decoded area (region_x0=%d) is outside the image area (Xsiz=%d).\n",
                      p_start_x, l_image->x1);
        return OPJ_FALSE;
    } else if ((OPJ_UINT32)p_start_x < l_image->x0) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "Left position of the decoded area (region_x0=%d) is outside the image area (XOsiz=%d).\n",
                      p_start_x, l_image->x0);
        p_j2k->m_specific_param.m_decoder.m_start_tile_x = 0;
        p_image->x0 = l_image->x0;
    } else {
        p_j2k->m_specific_param.m_decoder.m_start_tile_x = ((OPJ_UINT32)p_start_x -
                l_cp->tx0) / l_cp->tdx;
        p_image->x0 = (OPJ_UINT32)p_start_x;
    }

    /* Up */
    if (p_start_y < 0) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Up position of the decoded area (region_y0=%d) should be >= 0.\n",
                      p_start_y);
        return OPJ_FALSE;
    } else if ((OPJ_UINT32)p_start_y > l_image->y1) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Up position of the decoded area (region_y0=%d) is outside the image area (Ysiz=%d).\n",
                      p_start_y, l_image->y1);
        return OPJ_FALSE;
    } else if ((OPJ_UINT32)p_start_y < l_image->y0) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "Up position of the decoded area (region_y0=%d) is outside the image area (YOsiz=%d).\n",
                      p_start_y, l_image->y0);
        p_j2k->m_specific_param.m_decoder.m_start_tile_y = 0;
        p_image->y0 = l_image->y0;
    } else {
        p_j2k->m_specific_param.m_decoder.m_start_tile_y = ((OPJ_UINT32)p_start_y -
                l_cp->ty0) / l_cp->tdy;
        p_image->y0 = (OPJ_UINT32)p_start_y;
    }

    /* Right */
    if (p_end_x <= 0) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Right position of the decoded area (region_x1=%d) should be > 0.\n",
                      p_end_x);
        return OPJ_FALSE;
    } else if ((OPJ_UINT32)p_end_x < l_image->x0) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Right position of the decoded area (region_x1=%d) is outside the image area (XOsiz=%d).\n",
                      p_end_x, l_image->x0);
        return OPJ_FALSE;
    } else if ((OPJ_UINT32)p_end_x > l_image->x1) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "Right position of the decoded area (region_x1=%d) is outside the image area (Xsiz=%d).\n",
                      p_end_x, l_image->x1);
        p_j2k->m_specific_param.m_decoder.m_end_tile_x = l_cp->tw;
        p_image->x1 = l_image->x1;
    } else {
        p_j2k->m_specific_param.m_decoder.m_end_tile_x = opj_uint_ceildiv((
                    OPJ_UINT32)p_end_x - l_cp->tx0, l_cp->tdx);
        p_image->x1 = (OPJ_UINT32)p_end_x;
    }

    /* Bottom */
    if (p_end_y <= 0) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Bottom position of the decoded area (region_y1=%d) should be > 0.\n",
                      p_end_y);
        return OPJ_FALSE;
    } else if ((OPJ_UINT32)p_end_y < l_image->y0) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Bottom position of the decoded area (region_y1=%d) is outside the image area (YOsiz=%d).\n",
                      p_end_y, l_image->y0);
        return OPJ_FALSE;
    }
    if ((OPJ_UINT32)p_end_y > l_image->y1) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "Bottom position of the decoded area (region_y1=%d) is outside the image area (Ysiz=%d).\n",
                      p_end_y, l_image->y1);
        p_j2k->m_specific_param.m_decoder.m_end_tile_y = l_cp->th;
        p_image->y1 = l_image->y1;
    } else {
        p_j2k->m_specific_param.m_decoder.m_end_tile_y = opj_uint_ceildiv((
                    OPJ_UINT32)p_end_y - l_cp->ty0, l_cp->tdy);
        p_image->y1 = (OPJ_UINT32)p_end_y;
    }
    /* ----- */

    p_j2k->m_specific_param.m_decoder.m_discard_tiles = 1;

    ret = opj_j2k_update_image_dimensions(p_image, p_manager);

    if (ret) {
        opj_event_msg(p_manager, EVT_INFO, "Setting decoding area to %d,%d,%d,%d\n",
                      p_image->x0, p_image->y0, p_image->x1, p_image->y1);
    }

    return ret;
}

static opj_codestream_index_t* opj_j2k_create_cstr_index(void)
{
    opj_codestream_index_t* cstr_index = (opj_codestream_index_t*)
                                         calloc(1, sizeof(opj_codestream_index_t));
    if (!cstr_index) {
        return NULL;
    }

    cstr_index->maxmarknum = 100;
    cstr_index->marknum = 0;
    cstr_index->marker = (opj_marker_info_t*)
                         calloc(cstr_index->maxmarknum, sizeof(opj_marker_info_t));
    if (!cstr_index-> marker) {
        free(cstr_index);
        return NULL;
    }

    cstr_index->tile_index = NULL;

    return cstr_index;
}

static OPJ_UINT32 opj_j2k_get_SPCod_SPCoc_size(opj_j2k_t *p_j2k,OPJ_UINT32 p_tile_no,OPJ_UINT32 p_comp_no)
{
    opj_cp_t *l_cp = 00;
    opj_tcp_t *l_tcp = 00;
    opj_tccp_t *l_tccp = 00;

    /* preconditions */
    assert(p_j2k != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = &l_cp->tcps[p_tile_no];
    l_tccp = &l_tcp->tccps[p_comp_no];

    /* preconditions again */
    assert(p_tile_no < (l_cp->tw * l_cp->th));
    assert(p_comp_no < p_j2k->m_private_image->numcomps);

    if (l_tccp->csty & J2K_CCP_CSTY_PRT) {
        return 5 + l_tccp->numresolutions;
    } else {
        return 5;
    }
}

static OPJ_BOOL opj_j2k_compare_SPCod_SPCoc(opj_j2k_t *p_j2k,OPJ_UINT32 p_tile_no, OPJ_UINT32 p_first_comp_no, OPJ_UINT32 p_second_comp_no)
{
    OPJ_UINT32 i;
    opj_cp_t *l_cp = NULL;
    opj_tcp_t *l_tcp = NULL;
    opj_tccp_t *l_tccp0 = NULL;
    opj_tccp_t *l_tccp1 = NULL;

    /* preconditions */
    assert(p_j2k != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = &l_cp->tcps[p_tile_no];
    l_tccp0 = &l_tcp->tccps[p_first_comp_no];
    l_tccp1 = &l_tcp->tccps[p_second_comp_no];

    if (l_tccp0->numresolutions != l_tccp1->numresolutions) {
        return OPJ_FALSE;
    }
    if (l_tccp0->cblkw != l_tccp1->cblkw) {
        return OPJ_FALSE;
    }
    if (l_tccp0->cblkh != l_tccp1->cblkh) {
        return OPJ_FALSE;
    }
    if (l_tccp0->cblksty != l_tccp1->cblksty) {
        return OPJ_FALSE;
    }
    if (l_tccp0->qmfbid != l_tccp1->qmfbid) {
        return OPJ_FALSE;
    }
    if ((l_tccp0->csty & J2K_CCP_CSTY_PRT) != (l_tccp1->csty & J2K_CCP_CSTY_PRT)) {
        return OPJ_FALSE;
    }

    for (i = 0U; i < l_tccp0->numresolutions; ++i) {
        if (l_tccp0->prcw[i] != l_tccp1->prcw[i]) {
            return OPJ_FALSE;
        }
        if (l_tccp0->prch[i] != l_tccp1->prch[i]) {
            return OPJ_FALSE;
        }
    }
    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_SPCod_SPCoc(opj_j2k_t *p_j2k,OPJ_UINT32 p_tile_no,OPJ_UINT32 p_comp_no,OPJ_BYTE * p_data,OPJ_UINT32 * p_header_size,struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 i;
    opj_cp_t *l_cp = 00;
    opj_tcp_t *l_tcp = 00;
    opj_tccp_t *l_tccp = 00;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_header_size != 00);
    assert(p_manager != 00);
    assert(p_data != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = &l_cp->tcps[p_tile_no];
    l_tccp = &l_tcp->tccps[p_comp_no];

    /* preconditions again */
    assert(p_tile_no < (l_cp->tw * l_cp->th));
    assert(p_comp_no < (p_j2k->m_private_image->numcomps));

    if (*p_header_size < 5) {
        opj_event_msg(p_manager, EVT_ERROR, "Error writing SPCod SPCoc element\n");
        return OPJ_FALSE;
    }

    opj_write_bytes(p_data, l_tccp->numresolutions - 1, 1); /* SPcoc (D) */
    ++p_data;

    opj_write_bytes(p_data, l_tccp->cblkw - 2, 1);                  /* SPcoc (E) */
    ++p_data;

    opj_write_bytes(p_data, l_tccp->cblkh - 2, 1);                  /* SPcoc (F) */
    ++p_data;

    opj_write_bytes(p_data, l_tccp->cblksty,
                    1);                            /* SPcoc (G) */
    ++p_data;

    opj_write_bytes(p_data, l_tccp->qmfbid,
                    1);                             /* SPcoc (H) */
    ++p_data;

    *p_header_size = *p_header_size - 5;

    if (l_tccp->csty & J2K_CCP_CSTY_PRT) {

        if (*p_header_size < l_tccp->numresolutions) {
            opj_event_msg(p_manager, EVT_ERROR, "Error writing SPCod SPCoc element\n");
            return OPJ_FALSE;
        }

        for (i = 0; i < l_tccp->numresolutions; ++i) {
            opj_write_bytes(p_data, l_tccp->prcw[i] + (l_tccp->prch[i] << 4),
                            1);   /* SPcoc (I_i) */
            ++p_data;
        }

        *p_header_size = *p_header_size - l_tccp->numresolutions;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_read_SPCod_SPCoc(opj_j2k_t *p_j2k,OPJ_UINT32 compno,OPJ_BYTE * p_header_data,OPJ_UINT32 * p_header_size,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 i, l_tmp;
    opj_cp_t *l_cp = NULL;
    opj_tcp_t *l_tcp = NULL;
    opj_tccp_t *l_tccp = NULL;
    OPJ_BYTE * l_current_ptr = NULL;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_header_data != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH) ?
            &l_cp->tcps[p_j2k->m_current_tile_number] :
            p_j2k->m_specific_param.m_decoder.m_default_tcp;

    /* precondition again */
    assert(compno < p_j2k->m_private_image->numcomps);

    l_tccp = &l_tcp->tccps[compno];
    l_current_ptr = p_header_data;

    /* make sure room is sufficient */
    if (*p_header_size < 5) {
        opj_event_msg(p_manager, EVT_ERROR, "Error reading SPCod SPCoc element\n");
        return OPJ_FALSE;
    }

    /* SPcod (D) / SPcoc (A) */
    opj_read_bytes(l_current_ptr, &l_tccp->numresolutions, 1);
    ++l_tccp->numresolutions;  /* tccp->numresolutions = read() + 1 */
    if (l_tccp->numresolutions > OPJ_J2K_MAXRLVLS) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Invalid value for numresolutions : %d, max value is set in openjpeg.h at %d\n",
                      l_tccp->numresolutions, OPJ_J2K_MAXRLVLS);
        return OPJ_FALSE;
    }
    ++l_current_ptr;

    /* If user wants to remove more resolutions than the codestream contains, return error */
    if (l_cp->m_specific_param.m_dec.m_reduce >= l_tccp->numresolutions) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Error decoding component %d.\nThe number of resolutions "
                      "to remove (%d) is greater or equal than the number "
                      "of resolutions of this component (%d)\nModify the cp_reduce parameter.\n\n",
                      compno, l_cp->m_specific_param.m_dec.m_reduce, l_tccp->numresolutions);
        p_j2k->m_specific_param.m_decoder.m_state |=
            0x8000;/* FIXME J2K_DEC_STATE_ERR;*/
        return OPJ_FALSE;
    }

    /* SPcod (E) / SPcoc (B) */
    opj_read_bytes(l_current_ptr, &l_tccp->cblkw, 1);
    ++l_current_ptr;
    l_tccp->cblkw += 2;

    /* SPcod (F) / SPcoc (C) */
    opj_read_bytes(l_current_ptr, &l_tccp->cblkh, 1);
    ++l_current_ptr;
    l_tccp->cblkh += 2;

    if ((l_tccp->cblkw > 10) || (l_tccp->cblkh > 10) ||
            ((l_tccp->cblkw + l_tccp->cblkh) > 12)) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Error reading SPCod SPCoc element, Invalid cblkw/cblkh combination\n");
        return OPJ_FALSE;
    }

    /* SPcod (G) / SPcoc (D) */
    opj_read_bytes(l_current_ptr, &l_tccp->cblksty, 1);
    ++l_current_ptr;
    if ((l_tccp->cblksty & J2K_CCP_CBLKSTY_HTMIXED) != 0) {
        /* We do not support HT mixed mode yet.  For conformance, it should be supported.*/
        opj_event_msg(p_manager, EVT_ERROR,
                      "Error reading SPCod SPCoc element. Unsupported Mixed HT code-block style found\n");
        return OPJ_FALSE;
    }

    /* SPcod (H) / SPcoc (E) */
    opj_read_bytes(l_current_ptr, &l_tccp->qmfbid, 1);
    ++l_current_ptr;

    if (l_tccp->qmfbid > 1) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Error reading SPCod SPCoc element, Invalid transformation found\n");
        return OPJ_FALSE;
    }

    *p_header_size = *p_header_size - 5;

    /* use custom precinct size ? */
    if (l_tccp->csty & J2K_CCP_CSTY_PRT) {
        if (*p_header_size < l_tccp->numresolutions) {
            opj_event_msg(p_manager, EVT_ERROR, "Error reading SPCod SPCoc element\n");
            return OPJ_FALSE;
        }

        /* SPcod (I_i) / SPcoc (F_i) */
        for (i = 0; i < l_tccp->numresolutions; ++i) {
            opj_read_bytes(l_current_ptr, &l_tmp, 1);
            ++l_current_ptr;
            /* Precinct exponent 0 is only allowed for lowest resolution level (Table A.21) */
            if ((i != 0) && (((l_tmp & 0xf) == 0) || ((l_tmp >> 4) == 0))) {
                opj_event_msg(p_manager, EVT_ERROR, "Invalid precinct size\n");
                return OPJ_FALSE;
            }
            l_tccp->prcw[i] = l_tmp & 0xf;
            l_tccp->prch[i] = l_tmp >> 4;
        }

        *p_header_size = *p_header_size - l_tccp->numresolutions;
    } else {
        /* set default size for the precinct width and height */
        for (i = 0; i < l_tccp->numresolutions; ++i) {
            l_tccp->prcw[i] = 15;
            l_tccp->prch[i] = 15;
        }
    }

#ifdef WIP_REMOVE_MSD
    /* INDEX >> */
    if (p_j2k->cstr_info && compno == 0) {
        OPJ_UINT32 l_data_size = l_tccp->numresolutions * sizeof(OPJ_UINT32);

        p_j2k->cstr_info->tile[p_j2k->m_current_tile_number].tccp_info[compno].cblkh =
            l_tccp->cblkh;
        p_j2k->cstr_info->tile[p_j2k->m_current_tile_number].tccp_info[compno].cblkw =
            l_tccp->cblkw;
        p_j2k->cstr_info->tile[p_j2k->m_current_tile_number].tccp_info[compno].numresolutions
            = l_tccp->numresolutions;
        p_j2k->cstr_info->tile[p_j2k->m_current_tile_number].tccp_info[compno].cblksty =
            l_tccp->cblksty;
        p_j2k->cstr_info->tile[p_j2k->m_current_tile_number].tccp_info[compno].qmfbid =
            l_tccp->qmfbid;

        memcpy(p_j2k->cstr_info->tile[p_j2k->m_current_tile_number].pdx, l_tccp->prcw,
               l_data_size);
        memcpy(p_j2k->cstr_info->tile[p_j2k->m_current_tile_number].pdy, l_tccp->prch,
               l_data_size);
    }
    /* << INDEX */
#endif

    return OPJ_TRUE;
}

static void opj_j2k_copy_tile_component_parameters(opj_j2k_t *p_j2k)
{
    /* loop */
    OPJ_UINT32 i;
    opj_cp_t *l_cp = NULL;
    opj_tcp_t *l_tcp = NULL;
    opj_tccp_t *l_ref_tccp = NULL, *l_copied_tccp = NULL;
    OPJ_UINT32 l_prc_size;

    /* preconditions */
    assert(p_j2k != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH)
            ?
            &l_cp->tcps[p_j2k->m_current_tile_number] :
            p_j2k->m_specific_param.m_decoder.m_default_tcp;

    l_ref_tccp = &l_tcp->tccps[0];
    l_copied_tccp = l_ref_tccp + 1;
    l_prc_size = l_ref_tccp->numresolutions * (OPJ_UINT32)sizeof(OPJ_UINT32);

    for (i = 1; i < p_j2k->m_private_image->numcomps; ++i) {
        l_copied_tccp->numresolutions = l_ref_tccp->numresolutions;
        l_copied_tccp->cblkw = l_ref_tccp->cblkw;
        l_copied_tccp->cblkh = l_ref_tccp->cblkh;
        l_copied_tccp->cblksty = l_ref_tccp->cblksty;
        l_copied_tccp->qmfbid = l_ref_tccp->qmfbid;
        memcpy(l_copied_tccp->prcw, l_ref_tccp->prcw, l_prc_size);
        memcpy(l_copied_tccp->prch, l_ref_tccp->prch, l_prc_size);
        ++l_copied_tccp;
    }
}

static OPJ_UINT32 opj_j2k_get_SQcd_SQcc_size(opj_j2k_t *p_j2k,OPJ_UINT32 p_tile_no,OPJ_UINT32 p_comp_no)
{
    OPJ_UINT32 l_num_bands;

    opj_cp_t *l_cp = 00;
    opj_tcp_t *l_tcp = 00;
    opj_tccp_t *l_tccp = 00;

    /* preconditions */
    assert(p_j2k != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = &l_cp->tcps[p_tile_no];
    l_tccp = &l_tcp->tccps[p_comp_no];

    /* preconditions again */
    assert(p_tile_no < l_cp->tw * l_cp->th);
    assert(p_comp_no < p_j2k->m_private_image->numcomps);

    l_num_bands = (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) ? 1 :
                  (l_tccp->numresolutions * 3 - 2);

    if (l_tccp->qntsty == J2K_CCP_QNTSTY_NOQNT)  {
        return 1 + l_num_bands;
    } else {
        return 1 + 2 * l_num_bands;
    }
}

static OPJ_BOOL opj_j2k_compare_SQcd_SQcc(opj_j2k_t *p_j2k,OPJ_UINT32 p_tile_no, OPJ_UINT32 p_first_comp_no, OPJ_UINT32 p_second_comp_no)
{
    opj_cp_t *l_cp = NULL;
    opj_tcp_t *l_tcp = NULL;
    opj_tccp_t *l_tccp0 = NULL;
    opj_tccp_t *l_tccp1 = NULL;
    OPJ_UINT32 l_band_no, l_num_bands;

    /* preconditions */
    assert(p_j2k != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = &l_cp->tcps[p_tile_no];
    l_tccp0 = &l_tcp->tccps[p_first_comp_no];
    l_tccp1 = &l_tcp->tccps[p_second_comp_no];

    if (l_tccp0->qntsty != l_tccp1->qntsty) {
        return OPJ_FALSE;
    }
    if (l_tccp0->numgbits != l_tccp1->numgbits) {
        return OPJ_FALSE;
    }
    if (l_tccp0->qntsty == J2K_CCP_QNTSTY_SIQNT) {
        l_num_bands = 1U;
    } else {
        l_num_bands = l_tccp0->numresolutions * 3U - 2U;
        if (l_num_bands != (l_tccp1->numresolutions * 3U - 2U)) {
            return OPJ_FALSE;
        }
    }

    for (l_band_no = 0; l_band_no < l_num_bands; ++l_band_no) {
        if (l_tccp0->stepsizes[l_band_no].expn != l_tccp1->stepsizes[l_band_no].expn) {
            return OPJ_FALSE;
        }
    }
    if (l_tccp0->qntsty != J2K_CCP_QNTSTY_NOQNT) {
        for (l_band_no = 0; l_band_no < l_num_bands; ++l_band_no) {
            if (l_tccp0->stepsizes[l_band_no].mant != l_tccp1->stepsizes[l_band_no].mant) {
                return OPJ_FALSE;
            }
        }
    }
    return OPJ_TRUE;
}


static OPJ_BOOL opj_j2k_write_SQcd_SQcc(opj_j2k_t *p_j2k,OPJ_UINT32 p_tile_no,OPJ_UINT32 p_comp_no,OPJ_BYTE * p_data,OPJ_UINT32 * p_header_size,struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 l_header_size;
    OPJ_UINT32 l_band_no, l_num_bands;
    OPJ_UINT32 l_expn, l_mant;

    opj_cp_t *l_cp = 00;
    opj_tcp_t *l_tcp = 00;
    opj_tccp_t *l_tccp = 00;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_header_size != 00);
    assert(p_manager != 00);
    assert(p_data != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = &l_cp->tcps[p_tile_no];
    l_tccp = &l_tcp->tccps[p_comp_no];

    /* preconditions again */
    assert(p_tile_no < l_cp->tw * l_cp->th);
    assert(p_comp_no < p_j2k->m_private_image->numcomps);

    l_num_bands = (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) ? 1 :
                  (l_tccp->numresolutions * 3 - 2);

    if (l_tccp->qntsty == J2K_CCP_QNTSTY_NOQNT)  {
        l_header_size = 1 + l_num_bands;

        if (*p_header_size < l_header_size) {
            opj_event_msg(p_manager, EVT_ERROR, "Error writing SQcd SQcc element\n");
            return OPJ_FALSE;
        }

        opj_write_bytes(p_data, l_tccp->qntsty + (l_tccp->numgbits << 5),
                        1);   /* Sqcx */
        ++p_data;

        for (l_band_no = 0; l_band_no < l_num_bands; ++l_band_no) {
            l_expn = (OPJ_UINT32)l_tccp->stepsizes[l_band_no].expn;
            opj_write_bytes(p_data, l_expn << 3, 1);        /* SPqcx_i */
            ++p_data;
        }
    } else {
        l_header_size = 1 + 2 * l_num_bands;

        if (*p_header_size < l_header_size) {
            opj_event_msg(p_manager, EVT_ERROR, "Error writing SQcd SQcc element\n");
            return OPJ_FALSE;
        }

        opj_write_bytes(p_data, l_tccp->qntsty + (l_tccp->numgbits << 5),
                        1);   /* Sqcx */
        ++p_data;

        for (l_band_no = 0; l_band_no < l_num_bands; ++l_band_no) {
            l_expn = (OPJ_UINT32)l_tccp->stepsizes[l_band_no].expn;
            l_mant = (OPJ_UINT32)l_tccp->stepsizes[l_band_no].mant;

            opj_write_bytes(p_data, (l_expn << 11) + l_mant, 2);    /* SPqcx_i */
            p_data += 2;
        }
    }

    *p_header_size = *p_header_size - l_header_size;

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_read_SQcd_SQcc(opj_j2k_t *p_j2k,OPJ_UINT32 p_comp_no,OPJ_BYTE* p_header_data,OPJ_UINT32 * p_header_size,opj_event_mgr_t * p_manager)
{
    /* loop*/
    OPJ_UINT32 l_band_no;
    opj_cp_t *l_cp = 00;
    opj_tcp_t *l_tcp = 00;
    opj_tccp_t *l_tccp = 00;
    OPJ_BYTE * l_current_ptr = 00;
    OPJ_UINT32 l_tmp, l_num_band;

    /* preconditions*/
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_header_data != 00);

    l_cp = &(p_j2k->m_cp);
    /* come from tile part header or main header ?*/
    l_tcp = (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH)
            ?
            &l_cp->tcps[p_j2k->m_current_tile_number] :
            p_j2k->m_specific_param.m_decoder.m_default_tcp;

    /* precondition again*/
    assert(p_comp_no <  p_j2k->m_private_image->numcomps);

    l_tccp = &l_tcp->tccps[p_comp_no];
    l_current_ptr = p_header_data;

    if (*p_header_size < 1) {
        opj_event_msg(p_manager, EVT_ERROR, "Error reading SQcd or SQcc element\n");
        return OPJ_FALSE;
    }
    *p_header_size -= 1;

    opj_read_bytes(l_current_ptr, &l_tmp, 1);                       /* Sqcx */
    ++l_current_ptr;

    l_tccp->qntsty = l_tmp & 0x1f;
    l_tccp->numgbits = l_tmp >> 5;
    if (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) {
        l_num_band = 1;
    } else {
        l_num_band = (l_tccp->qntsty == J2K_CCP_QNTSTY_NOQNT) ?
                     (*p_header_size) :
                     (*p_header_size) / 2;

        if (l_num_band > OPJ_J2K_MAXBANDS) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "While reading CCP_QNTSTY element inside QCD or QCC marker segment, "
                          "number of subbands (%d) is greater to OPJ_J2K_MAXBANDS (%d). So we limit the number of elements stored to "
                          "OPJ_J2K_MAXBANDS (%d) and skip the rest. \n", l_num_band, OPJ_J2K_MAXBANDS,
                          OPJ_J2K_MAXBANDS);
            /*return OPJ_FALSE;*/
        }
    }

#ifdef USE_JPWL
    if (l_cp->correct) {

        /* if JPWL is on, we check whether there are too many subbands */
        if (/*(l_num_band < 0) ||*/ (l_num_band >= OPJ_J2K_MAXBANDS)) {
            opj_event_msg(p_manager, JPWL_ASSUME ? EVT_WARNING : EVT_ERROR,
                          "JPWL: bad number of subbands in Sqcx (%d)\n",
                          l_num_band);
            if (!JPWL_ASSUME) {
                opj_event_msg(p_manager, EVT_ERROR, "JPWL: giving up\n");
                return OPJ_FALSE;
            }
            /* we try to correct */
            l_num_band = 1;
            opj_event_msg(p_manager, EVT_WARNING, "- trying to adjust them\n"
                          "- setting number of bands to %d => HYPOTHESIS!!!\n",
                          l_num_band);
        };

    };
#endif /* USE_JPWL */

    if (l_tccp->qntsty == J2K_CCP_QNTSTY_NOQNT) {
        for (l_band_no = 0; l_band_no < l_num_band; l_band_no++) {
            opj_read_bytes(l_current_ptr, &l_tmp, 1);                       /* SPqcx_i */
            ++l_current_ptr;
            if (l_band_no < OPJ_J2K_MAXBANDS) {
                l_tccp->stepsizes[l_band_no].expn = (OPJ_INT32)(l_tmp >> 3);
                l_tccp->stepsizes[l_band_no].mant = 0;
            }
        }

        if (*p_header_size < l_num_band) {
            return OPJ_FALSE;
        }
        *p_header_size = *p_header_size - l_num_band;
    } else {
        for (l_band_no = 0; l_band_no < l_num_band; l_band_no++) {
            opj_read_bytes(l_current_ptr, &l_tmp, 2);                       /* SPqcx_i */
            l_current_ptr += 2;
            if (l_band_no < OPJ_J2K_MAXBANDS) {
                l_tccp->stepsizes[l_band_no].expn = (OPJ_INT32)(l_tmp >> 11);
                l_tccp->stepsizes[l_band_no].mant = l_tmp & 0x7ff;
            }
        }

        if (*p_header_size < 2 * l_num_band) {
            return OPJ_FALSE;
        }
        *p_header_size = *p_header_size - 2 * l_num_band;
    }

    /* Add Antonin : if scalar_derived -> compute other stepsizes */
    if (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) {
        for (l_band_no = 1; l_band_no < OPJ_J2K_MAXBANDS; l_band_no++) {
            l_tccp->stepsizes[l_band_no].expn =
                ((OPJ_INT32)(l_tccp->stepsizes[0].expn) - (OPJ_INT32)((l_band_no - 1) / 3) > 0)
                ?
                (OPJ_INT32)(l_tccp->stepsizes[0].expn) - (OPJ_INT32)((l_band_no - 1) / 3) : 0;
            l_tccp->stepsizes[l_band_no].mant = l_tccp->stepsizes[0].mant;
        }
    }

    return OPJ_TRUE;
}

static void opj_j2k_copy_tile_quantization_parameters(opj_j2k_t *p_j2k)
{
    OPJ_UINT32 i;
    opj_cp_t *l_cp = NULL;
    opj_tcp_t *l_tcp = NULL;
    opj_tccp_t *l_ref_tccp = NULL;
    opj_tccp_t *l_copied_tccp = NULL;
    OPJ_UINT32 l_size;

    l_cp = &(p_j2k->m_cp);
    l_tcp = p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH ?
            &l_cp->tcps[p_j2k->m_current_tile_number] :
            p_j2k->m_specific_param.m_decoder.m_default_tcp;

    l_ref_tccp = &l_tcp->tccps[0];
    l_copied_tccp = l_ref_tccp + 1;
    l_size = OPJ_J2K_MAXBANDS * sizeof(opj_stepsize_t);

    for (i = 1; i < p_j2k->m_private_image->numcomps; ++i) {
        l_copied_tccp->qntsty = l_ref_tccp->qntsty;
        l_copied_tccp->numgbits = l_ref_tccp->numgbits;
        memcpy(l_copied_tccp->stepsizes, l_ref_tccp->stepsizes, l_size);
        ++l_copied_tccp;
    }
}

static void opj_j2k_dump_tile_info(opj_tcp_t * l_default_tile,OPJ_INT32 numcomps, FILE* out_stream)
{
    if (l_default_tile) {
        OPJ_INT32 compno;

        fprintf(out_stream, "\t default tile {\n");
        fprintf(out_stream, "\t\t csty=%#x\n", l_default_tile->csty);
        fprintf(out_stream, "\t\t prg=%#x\n", l_default_tile->prg);
        fprintf(out_stream, "\t\t numlayers=%d\n", l_default_tile->numlayers);
        fprintf(out_stream, "\t\t mct=%x\n", l_default_tile->mct);

        for (compno = 0; compno < numcomps; compno++) {
            opj_tccp_t *l_tccp = &(l_default_tile->tccps[compno]);
            OPJ_UINT32 resno;
            OPJ_INT32 bandno, numbands;

            /* coding style*/
            fprintf(out_stream, "\t\t comp %d {\n", compno);
            fprintf(out_stream, "\t\t\t csty=%#x\n", l_tccp->csty);
            fprintf(out_stream, "\t\t\t numresolutions=%d\n", l_tccp->numresolutions);
            fprintf(out_stream, "\t\t\t cblkw=2^%d\n", l_tccp->cblkw);
            fprintf(out_stream, "\t\t\t cblkh=2^%d\n", l_tccp->cblkh);
            fprintf(out_stream, "\t\t\t cblksty=%#x\n", l_tccp->cblksty);
            fprintf(out_stream, "\t\t\t qmfbid=%d\n", l_tccp->qmfbid);

            fprintf(out_stream, "\t\t\t preccintsize (w,h)=");
            for (resno = 0; resno < l_tccp->numresolutions; resno++) {
                fprintf(out_stream, "(%d,%d) ", l_tccp->prcw[resno], l_tccp->prch[resno]);
            }
            fprintf(out_stream, "\n");

            /* quantization style*/
            fprintf(out_stream, "\t\t\t qntsty=%d\n", l_tccp->qntsty);
            fprintf(out_stream, "\t\t\t numgbits=%d\n", l_tccp->numgbits);
            fprintf(out_stream, "\t\t\t stepsizes (m,e)=");
            numbands = (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) ? 1 :
                       (OPJ_INT32)l_tccp->numresolutions * 3 - 2;
            for (bandno = 0; bandno < numbands; bandno++) {
                fprintf(out_stream, "(%d,%d) ", l_tccp->stepsizes[bandno].mant,
                        l_tccp->stepsizes[bandno].expn);
            }
            fprintf(out_stream, "\n");

            /* RGN value*/
            fprintf(out_stream, "\t\t\t roishift=%d\n", l_tccp->roishift);

            fprintf(out_stream, "\t\t }\n");
        } /*end of component of default tile*/
        fprintf(out_stream, "\t }\n"); /*end of default tile*/
    }
}

static void opj_j2k_dump_MH_index(opj_j2k_t* p_j2k, FILE* out_stream)
{
    opj_codestream_index_t* cstr_index = p_j2k->cstr_index;
    OPJ_UINT32 it_marker, it_tile, it_tile_part;

    fprintf(out_stream, "Codestream index from main header: {\n");

    fprintf(out_stream, "\t Main header start position=%" PRIi64 "\n"
            "\t Main header end position=%" PRIi64 "\n",
            cstr_index->main_head_start, cstr_index->main_head_end);

    fprintf(out_stream, "\t Marker list: {\n");

    if (cstr_index->marker) {
        for (it_marker = 0; it_marker < cstr_index->marknum ; it_marker++) {
            fprintf(out_stream, "\t\t type=%#x, pos=%" PRIi64 ", len=%d\n",
                    cstr_index->marker[it_marker].type,
                    cstr_index->marker[it_marker].pos,
                    cstr_index->marker[it_marker].len);
        }
    }

    fprintf(out_stream, "\t }\n");

    if (cstr_index->tile_index) {

        /* Simple test to avoid to write empty information*/
        OPJ_UINT32 l_acc_nb_of_tile_part = 0;
        for (it_tile = 0; it_tile < cstr_index->nb_of_tiles ; it_tile++) {
            l_acc_nb_of_tile_part += cstr_index->tile_index[it_tile].nb_tps;

            /* To avoid regenerating expected opj_dump results from the test */
            /* suite when there is a TLM marker present */
            if (cstr_index->tile_index[it_tile].nb_tps &&
                    cstr_index->tile_index[it_tile].tp_index &&
                    cstr_index->tile_index[it_tile].tp_index[0].start_pos > 0 &&
                    cstr_index->tile_index[it_tile].tp_index[0].end_header == 0 &&
                    getenv("OJP_DO_NOT_DISPLAY_TILE_INDEX_IF_TLM") != NULL) {
                l_acc_nb_of_tile_part = 0;
                break;
            }
        }

        if (l_acc_nb_of_tile_part) {
            fprintf(out_stream, "\t Tile index: {\n");

            for (it_tile = 0; it_tile < cstr_index->nb_of_tiles ; it_tile++) {
                OPJ_UINT32 nb_of_tile_part = cstr_index->tile_index[it_tile].nb_tps;

                fprintf(out_stream, "\t\t nb of tile-part in tile [%d]=%d\n", it_tile,
                        nb_of_tile_part);

                if (cstr_index->tile_index[it_tile].tp_index) {
                    for (it_tile_part = 0; it_tile_part < nb_of_tile_part; it_tile_part++) {
                        fprintf(out_stream, "\t\t\t tile-part[%d]: star_pos=%" PRIi64 ", end_header=%"
                                PRIi64 ", end_pos=%" PRIi64 ".\n",
                                it_tile_part,
                                cstr_index->tile_index[it_tile].tp_index[it_tile_part].start_pos,
                                cstr_index->tile_index[it_tile].tp_index[it_tile_part].end_header,
                                cstr_index->tile_index[it_tile].tp_index[it_tile_part].end_pos);
                    }
                }

                if (cstr_index->tile_index[it_tile].marker) {
                    for (it_marker = 0; it_marker < cstr_index->tile_index[it_tile].marknum ;
                            it_marker++) {
                        fprintf(out_stream, "\t\t type=%#x, pos=%" PRIi64 ", len=%d\n",
                                cstr_index->tile_index[it_tile].marker[it_marker].type,
                                cstr_index->tile_index[it_tile].marker[it_marker].pos,
                                cstr_index->tile_index[it_tile].marker[it_marker].len);
                    }
                }
            }
            fprintf(out_stream, "\t }\n");
        }
    }

    fprintf(out_stream, "}\n");

}

static void opj_j2k_dump_MH_info(opj_j2k_t* p_j2k, FILE* out_stream)
{

    fprintf(out_stream, "Codestream info from main header: {\n");

    fprintf(out_stream, "\t tx0=%" PRIu32 ", ty0=%" PRIu32 "\n", p_j2k->m_cp.tx0,
            p_j2k->m_cp.ty0);
    fprintf(out_stream, "\t tdx=%" PRIu32 ", tdy=%" PRIu32 "\n", p_j2k->m_cp.tdx,
            p_j2k->m_cp.tdy);
    fprintf(out_stream, "\t tw=%" PRIu32 ", th=%" PRIu32 "\n", p_j2k->m_cp.tw,
            p_j2k->m_cp.th);
    opj_j2k_dump_tile_info(p_j2k->m_specific_param.m_decoder.m_default_tcp,
                           (OPJ_INT32)p_j2k->m_private_image->numcomps, out_stream);
    fprintf(out_stream, "}\n");
}

opj_codestream_info_v2_t* j2k_get_cstr_info(opj_j2k_t* p_j2k)
{
    OPJ_UINT32 compno;
    OPJ_UINT32 numcomps = p_j2k->m_private_image->numcomps;
    opj_tcp_t *l_default_tile;
    opj_codestream_info_v2_t* cstr_info = (opj_codestream_info_v2_t*) calloc(1,
                                          sizeof(opj_codestream_info_v2_t));
    if (!cstr_info) {
        return NULL;
    }

    cstr_info->nbcomps = p_j2k->m_private_image->numcomps;

    cstr_info->tx0 = p_j2k->m_cp.tx0;
    cstr_info->ty0 = p_j2k->m_cp.ty0;
    cstr_info->tdx = p_j2k->m_cp.tdx;
    cstr_info->tdy = p_j2k->m_cp.tdy;
    cstr_info->tw = p_j2k->m_cp.tw;
    cstr_info->th = p_j2k->m_cp.th;

    cstr_info->tile_info = NULL; /* Not fill from the main header*/

    l_default_tile = p_j2k->m_specific_param.m_decoder.m_default_tcp;

    cstr_info->m_default_tile_info.csty = l_default_tile->csty;
    cstr_info->m_default_tile_info.prg = l_default_tile->prg;
    cstr_info->m_default_tile_info.numlayers = l_default_tile->numlayers;
    cstr_info->m_default_tile_info.mct = l_default_tile->mct;

    cstr_info->m_default_tile_info.tccp_info = (opj_tccp_info_t*) calloc(
                cstr_info->nbcomps, sizeof(opj_tccp_info_t));
    if (!cstr_info->m_default_tile_info.tccp_info) {
        opj_destroy_cstr_info(&cstr_info);
        return NULL;
    }

    for (compno = 0; compno < numcomps; compno++) {
        opj_tccp_t *l_tccp = &(l_default_tile->tccps[compno]);
        opj_tccp_info_t *l_tccp_info = &
                                       (cstr_info->m_default_tile_info.tccp_info[compno]);
        OPJ_INT32 bandno, numbands;

        /* coding style*/
        l_tccp_info->csty = l_tccp->csty;
        l_tccp_info->numresolutions = l_tccp->numresolutions;
        l_tccp_info->cblkw = l_tccp->cblkw;
        l_tccp_info->cblkh = l_tccp->cblkh;
        l_tccp_info->cblksty = l_tccp->cblksty;
        l_tccp_info->qmfbid = l_tccp->qmfbid;
        if (l_tccp->numresolutions < OPJ_J2K_MAXRLVLS) {
            memcpy(l_tccp_info->prch, l_tccp->prch, l_tccp->numresolutions);
            memcpy(l_tccp_info->prcw, l_tccp->prcw, l_tccp->numresolutions);
        }

        /* quantization style*/
        l_tccp_info->qntsty = l_tccp->qntsty;
        l_tccp_info->numgbits = l_tccp->numgbits;

        numbands = (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) ? 1 :
                   (OPJ_INT32)l_tccp->numresolutions * 3 - 2;
        if (numbands < OPJ_J2K_MAXBANDS) {
            for (bandno = 0; bandno < numbands; bandno++) {
                l_tccp_info->stepsizes_mant[bandno] = (OPJ_UINT32)
                                                      l_tccp->stepsizes[bandno].mant;
                l_tccp_info->stepsizes_expn[bandno] = (OPJ_UINT32)
                                                      l_tccp->stepsizes[bandno].expn;
            }
        }

        /* RGN value*/
        l_tccp_info->roishift = l_tccp->roishift;
    }

    return cstr_info;
}

opj_codestream_index_t* j2k_get_cstr_index(opj_j2k_t* p_j2k)
{
    opj_codestream_index_t* l_cstr_index = (opj_codestream_index_t*)
                                           calloc(1, sizeof(opj_codestream_index_t));
    if (!l_cstr_index) {
        return NULL;
    }

    l_cstr_index->main_head_start = p_j2k->cstr_index->main_head_start;
    l_cstr_index->main_head_end = p_j2k->cstr_index->main_head_end;
    l_cstr_index->codestream_size = p_j2k->cstr_index->codestream_size;

    l_cstr_index->marknum = p_j2k->cstr_index->marknum;
    l_cstr_index->marker = (opj_marker_info_t*)malloc(l_cstr_index->marknum *
                           sizeof(opj_marker_info_t));
    if (!l_cstr_index->marker) {
        free(l_cstr_index);
        return NULL;
    }

    if (p_j2k->cstr_index->marker) {
        memcpy(l_cstr_index->marker, p_j2k->cstr_index->marker,
               l_cstr_index->marknum * sizeof(opj_marker_info_t));
    } else {
        free(l_cstr_index->marker);
        l_cstr_index->marker = NULL;
    }

    l_cstr_index->nb_of_tiles = p_j2k->cstr_index->nb_of_tiles;
    l_cstr_index->tile_index = (opj_tile_index_t*)calloc(
                                   l_cstr_index->nb_of_tiles, sizeof(opj_tile_index_t));
    if (!l_cstr_index->tile_index) {
        free(l_cstr_index->marker);
        free(l_cstr_index);
        return NULL;
    }

    if (!p_j2k->cstr_index->tile_index) {
        free(l_cstr_index->tile_index);
        l_cstr_index->tile_index = NULL;
    } else {
        OPJ_UINT32 it_tile = 0;
        for (it_tile = 0; it_tile < l_cstr_index->nb_of_tiles; it_tile++) {

            /* Tile Marker*/
            l_cstr_index->tile_index[it_tile].marknum =
                p_j2k->cstr_index->tile_index[it_tile].marknum;

            l_cstr_index->tile_index[it_tile].marker =
                (opj_marker_info_t*)malloc(l_cstr_index->tile_index[it_tile].marknum *
                                               sizeof(opj_marker_info_t));

            if (!l_cstr_index->tile_index[it_tile].marker) {
                OPJ_UINT32 it_tile_free;

                for (it_tile_free = 0; it_tile_free < it_tile; it_tile_free++) {
                    free(l_cstr_index->tile_index[it_tile_free].marker);
                }

                free(l_cstr_index->tile_index);
                free(l_cstr_index->marker);
                free(l_cstr_index);
                return NULL;
            }

            if (p_j2k->cstr_index->tile_index[it_tile].marker)
                memcpy(l_cstr_index->tile_index[it_tile].marker,
                       p_j2k->cstr_index->tile_index[it_tile].marker,
                       l_cstr_index->tile_index[it_tile].marknum * sizeof(opj_marker_info_t));
            else {
                free(l_cstr_index->tile_index[it_tile].marker);
                l_cstr_index->tile_index[it_tile].marker = NULL;
            }

            /* Tile part index*/
            l_cstr_index->tile_index[it_tile].nb_tps =
                p_j2k->cstr_index->tile_index[it_tile].nb_tps;

            l_cstr_index->tile_index[it_tile].tp_index =
                (opj_tp_index_t*)malloc(l_cstr_index->tile_index[it_tile].nb_tps * sizeof(
                                                opj_tp_index_t));

            if (!l_cstr_index->tile_index[it_tile].tp_index) {
                OPJ_UINT32 it_tile_free;

                for (it_tile_free = 0; it_tile_free < it_tile; it_tile_free++) {
                    free(l_cstr_index->tile_index[it_tile_free].marker);
                    free(l_cstr_index->tile_index[it_tile_free].tp_index);
                }

                free(l_cstr_index->tile_index);
                free(l_cstr_index->marker);
                free(l_cstr_index);
                return NULL;
            }

            if (p_j2k->cstr_index->tile_index[it_tile].tp_index) {
                memcpy(l_cstr_index->tile_index[it_tile].tp_index,
                       p_j2k->cstr_index->tile_index[it_tile].tp_index,
                       l_cstr_index->tile_index[it_tile].nb_tps * sizeof(opj_tp_index_t));
            } else {
                free(l_cstr_index->tile_index[it_tile].tp_index);
                l_cstr_index->tile_index[it_tile].tp_index = NULL;
            }

            /* Packet index (NOT USED)*/
            l_cstr_index->tile_index[it_tile].nb_packet = 0;
            l_cstr_index->tile_index[it_tile].packet_index = NULL;

        }
    }

    return l_cstr_index;
}

static OPJ_BOOL opj_j2k_allocate_tile_element_cstr_index(opj_j2k_t *p_j2k)
{
    OPJ_UINT32 it_tile = 0;

    p_j2k->cstr_index->nb_of_tiles = p_j2k->m_cp.tw * p_j2k->m_cp.th;
    p_j2k->cstr_index->tile_index = (opj_tile_index_t*)calloc(
                                        p_j2k->cstr_index->nb_of_tiles, sizeof(opj_tile_index_t));
    if (!p_j2k->cstr_index->tile_index) {
        return OPJ_FALSE;
    }

    for (it_tile = 0; it_tile < p_j2k->cstr_index->nb_of_tiles; it_tile++) {
        p_j2k->cstr_index->tile_index[it_tile].maxmarknum = 100;
        p_j2k->cstr_index->tile_index[it_tile].marknum = 0;
        p_j2k->cstr_index->tile_index[it_tile].marker = (opj_marker_info_t*)
                calloc(p_j2k->cstr_index->tile_index[it_tile].maxmarknum,
                           sizeof(opj_marker_info_t));
        if (!p_j2k->cstr_index->tile_index[it_tile].marker) {
            return OPJ_FALSE;
        }
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_are_all_used_components_decoded(opj_j2k_t *p_j2k,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 compno;
    OPJ_BOOL decoded_all_used_components = OPJ_TRUE;

    if (p_j2k->m_specific_param.m_decoder.m_numcomps_to_decode) {
        for (compno = 0;
                compno < p_j2k->m_specific_param.m_decoder.m_numcomps_to_decode; compno++) {
            OPJ_UINT32 dec_compno =
                p_j2k->m_specific_param.m_decoder.m_comps_indices_to_decode[compno];
            if (p_j2k->m_output_image->comps[dec_compno].data == NULL) {
                opj_event_msg(p_manager, EVT_WARNING, "Failed to decode component %d\n",
                              dec_compno);
                decoded_all_used_components = OPJ_FALSE;
            }
        }
    } else {
        for (compno = 0; compno < p_j2k->m_output_image->numcomps; compno++) {
            if (p_j2k->m_output_image->comps[compno].data == NULL) {
                opj_event_msg(p_manager, EVT_WARNING, "Failed to decode component %d\n",
                              compno);
                decoded_all_used_components = OPJ_FALSE;
            }
        }
    }

    if (decoded_all_used_components == OPJ_FALSE) {
        opj_event_msg(p_manager, EVT_ERROR, "Failed to decode all used components\n");
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static int CompareOffT(const void* a, const void* b)
{
    const OPJ_OFF_T offA = *(const OPJ_OFF_T*)a;
    const OPJ_OFF_T offB = *(const OPJ_OFF_T*)b;
    if (offA < offB) {
        return -1;
    }
    if (offA == offB) {
        return 0;
    }
    return 1;
}





static OPJ_BOOL opj_j2k_write_first_tile_part(opj_j2k_t *p_j2k,
        OPJ_BYTE * p_data,
        OPJ_UINT32 * p_data_written,
        OPJ_UINT32 total_data_size,
        opj_stream_private_t *p_stream,
        struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 l_nb_bytes_written = 0;
    OPJ_UINT32 l_current_nb_bytes_written;
    OPJ_BYTE * l_begin_data = 00;

    opj_tcd_t * l_tcd = 00;
    opj_cp_t * l_cp = 00;

    l_tcd = p_j2k->m_tcd;
    l_cp = &(p_j2k->m_cp);

    l_tcd->cur_pino = 0;

    /*Get number of tile parts*/
    p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number = 0;

    /* INDEX >> */
    /* << INDEX */

    l_current_nb_bytes_written = 0;
    l_begin_data = p_data;
    if (! opj_j2k_write_sot(p_j2k, p_data, total_data_size,
                            &l_current_nb_bytes_written, p_stream)) {
        return OPJ_FALSE;
    }

    l_nb_bytes_written += l_current_nb_bytes_written;
    p_data += l_current_nb_bytes_written;
    total_data_size -= l_current_nb_bytes_written;

    if (!OPJ_IS_CINEMA(l_cp->rsiz)) {
#if 0
        for (compno = 1; compno < p_j2k->m_private_image->numcomps; compno++) {
            l_current_nb_bytes_written = 0;
            opj_j2k_write_coc_in_memory(p_j2k, compno, p_data, &l_current_nb_bytes_written,
                                        p_manager);
            l_nb_bytes_written += l_current_nb_bytes_written;
            p_data += l_current_nb_bytes_written;
            total_data_size -= l_current_nb_bytes_written;

            l_current_nb_bytes_written = 0;
            opj_j2k_write_qcc_in_memory(p_j2k, compno, p_data, &l_current_nb_bytes_written,
                                        p_manager);
            l_nb_bytes_written += l_current_nb_bytes_written;
            p_data += l_current_nb_bytes_written;
            total_data_size -= l_current_nb_bytes_written;
        }
#endif
        if (l_cp->tcps[p_j2k->m_current_tile_number].POC) {
            l_current_nb_bytes_written = 0;
            opj_j2k_write_poc_in_memory(p_j2k, p_data, &l_current_nb_bytes_written);
            l_nb_bytes_written += l_current_nb_bytes_written;
            p_data += l_current_nb_bytes_written;
            total_data_size -= l_current_nb_bytes_written;
        }
    }

    l_current_nb_bytes_written = 0;
    if (! opj_j2k_write_sod(p_j2k, l_tcd, p_data, &l_current_nb_bytes_written,
                            total_data_size, p_stream)) {
        return OPJ_FALSE;
    }

    l_nb_bytes_written += l_current_nb_bytes_written;
    * p_data_written = l_nb_bytes_written;

    /* Writing Psot in SOT marker */
    opj_write_bytes(l_begin_data + 6, l_nb_bytes_written,
                    4);                                 /* PSOT */

    if (p_j2k->m_specific_param.m_encoder.m_TLM) {
        opj_j2k_update_tlm(p_j2k, l_nb_bytes_written);
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_all_tile_parts(opj_j2k_t *p_j2k,
        OPJ_BYTE * p_data,
        OPJ_UINT32 * p_data_written,
        OPJ_UINT32 total_data_size,
        opj_stream_private_t *p_stream)
{
    OPJ_UINT32 tilepartno = 0;
    OPJ_UINT32 l_nb_bytes_written = 0;
    OPJ_UINT32 l_current_nb_bytes_written;
    OPJ_UINT32 l_part_tile_size;
    OPJ_UINT32 tot_num_tp;
    OPJ_UINT32 pino;

    OPJ_BYTE * l_begin_data;
    opj_tcp_t *l_tcp = 00;
    opj_tcd_t * l_tcd = 00;
    opj_cp_t * l_cp = 00;

    l_tcd = p_j2k->m_tcd;
    l_cp = &(p_j2k->m_cp);
    l_tcp = l_cp->tcps + p_j2k->m_current_tile_number;

    /*Get number of tile parts*/
    tot_num_tp = opj_j2k_get_num_tp(l_cp, 0, p_j2k->m_current_tile_number);

    /* start writing remaining tile parts */
    ++p_j2k->m_specific_param.m_encoder.m_current_tile_part_number;
    for (tilepartno = 1; tilepartno < tot_num_tp ; ++tilepartno) {
        p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number = tilepartno;
        l_current_nb_bytes_written = 0;
        l_part_tile_size = 0;
        l_begin_data = p_data;

        if (! opj_j2k_write_sot(p_j2k, p_data,
                                total_data_size,
                                &l_current_nb_bytes_written,
                                p_stream)) {
            return OPJ_FALSE;
        }

        l_nb_bytes_written += l_current_nb_bytes_written;
        p_data += l_current_nb_bytes_written;
        total_data_size -= l_current_nb_bytes_written;
        l_part_tile_size += l_current_nb_bytes_written;

        l_current_nb_bytes_written = 0;
        if (! opj_j2k_write_sod(p_j2k, l_tcd, p_data, &l_current_nb_bytes_written,
                                total_data_size, p_stream)) {
            return OPJ_FALSE;
        }

        p_data += l_current_nb_bytes_written;
        l_nb_bytes_written += l_current_nb_bytes_written;
        total_data_size -= l_current_nb_bytes_written;
        l_part_tile_size += l_current_nb_bytes_written;

        /* Writing Psot in SOT marker */
        opj_write_bytes(l_begin_data + 6, l_part_tile_size,
                        4);                                   /* PSOT */

        if (p_j2k->m_specific_param.m_encoder.m_TLM) {
            opj_j2k_update_tlm(p_j2k, l_part_tile_size);
        }

        ++p_j2k->m_specific_param.m_encoder.m_current_tile_part_number;
    }

    for (pino = 1; pino <= l_tcp->numpocs; ++pino) {
        l_tcd->cur_pino = pino;

        /*Get number of tile parts*/
        tot_num_tp = opj_j2k_get_num_tp(l_cp, pino, p_j2k->m_current_tile_number);
        for (tilepartno = 0; tilepartno < tot_num_tp ; ++tilepartno) {
            p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number = tilepartno;
            l_current_nb_bytes_written = 0;
            l_part_tile_size = 0;
            l_begin_data = p_data;

            if (! opj_j2k_write_sot(p_j2k, p_data,
                                    total_data_size,
                                    &l_current_nb_bytes_written, p_stream)) {
                return OPJ_FALSE;
            }

            l_nb_bytes_written += l_current_nb_bytes_written;
            p_data += l_current_nb_bytes_written;
            total_data_size -= l_current_nb_bytes_written;
            l_part_tile_size += l_current_nb_bytes_written;

            l_current_nb_bytes_written = 0;

            if (! opj_j2k_write_sod(p_j2k, l_tcd, p_data, &l_current_nb_bytes_written,
                                    total_data_size, p_stream)) {
                return OPJ_FALSE;
            }

            l_nb_bytes_written += l_current_nb_bytes_written;
            p_data += l_current_nb_bytes_written;
            total_data_size -= l_current_nb_bytes_written;
            l_part_tile_size += l_current_nb_bytes_written;

            /* Writing Psot in SOT marker */
            opj_write_bytes(l_begin_data + 6, l_part_tile_size,
                            4);                                   /* PSOT */

            if (p_j2k->m_specific_param.m_encoder.m_TLM) {
                opj_j2k_update_tlm(p_j2k, l_part_tile_size);
            }

            ++p_j2k->m_specific_param.m_encoder.m_current_tile_part_number;
        }
    }

    *p_data_written = l_nb_bytes_written;

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_end_encoding(opj_j2k_t *p_j2k,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    OPJ_UNUSED(p_stream);
    OPJ_UNUSED(p_manager);

    opj_tcd_destroy(p_j2k->m_tcd);
    p_j2k->m_tcd = 00;

    if (p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer) {
        free(p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer);
        p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer = 0;
        p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current = 0;
    }

    if (p_j2k->m_specific_param.m_encoder.m_encoded_tile_data) {
        free(p_j2k->m_specific_param.m_encoder.m_encoded_tile_data);
        p_j2k->m_specific_param.m_encoder.m_encoded_tile_data = 0;
    }

    p_j2k->m_specific_param.m_encoder.m_encoded_tile_size = 0;

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_destroy_header_memory(opj_j2k_t * p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
   /**
    * Destroys the memory associated with the decoding of headers.
    */

    OPJ_UNUSED(p_stream);
    OPJ_UNUSED(p_manager);

    if (p_j2k->m_specific_param.m_encoder.m_header_tile_data) {
        free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = 0;
    }

    p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;

    return OPJ_TRUE;
}



OPJ_BOOL opj_j2k_write_tile(opj_j2k_t * p_j2k,
                            OPJ_UINT32 p_tile_index,
                            OPJ_BYTE * p_data,
                            OPJ_UINT32 p_data_size,
                            opj_stream_private_t *p_stream,
                            opj_event_mgr_t * p_manager)
{
    if (! opj_j2k_pre_write_tile(p_j2k, p_tile_index, p_stream)) {
        E("Error while opj_j2k_pre_write_tile with tile index = %d\n", p_tile_index);
        return OPJ_FALSE;
    } else {
        OPJ_UINT32 j;
        /* Allocate data */
        for (j = 0; j < p_j2k->m_tcd->image->numcomps; ++j) {
            opj_tcd_tilecomp_t* l_tilec = p_j2k->m_tcd->tcd_image->tiles->comps + j;

            if (! opj_alloc_tile_component_data(l_tilec)) {
                opj_event_msg(p_manager, EVT_ERROR, "Error allocating tile component data.");
                return OPJ_FALSE;
            }
        }

        /* now copy data into the tile component */
        if (! opj_tcd_copy_tile_data(p_j2k->m_tcd, p_data, p_data_size)) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Size mismatch between tile data and sent data.");
            return OPJ_FALSE;
        }
        if (! opj_j2k_post_write_tile(p_j2k, p_stream)) {
            E("Error while opj_j2k_post_write_tile with tile index = %d\n", p_tile_index);
            return OPJ_FALSE;
        }
    }

    return OPJ_TRUE;
}


#pragma mark - third level methods


static OPJ_BOOL opj_j2k_write_sot(
   opj_j2k_t *j2k,
   OPJ_BYTE * data,
   OPJ_UINT32 total_data_size,
   OPJ_UINT32 * data_written,
   const opj_stream_private_t *stream
   )
{
   //A.4.2 SOT Start of tile marker 0xff90 12 bytes
   //??? OPJ_UNUSED(stream);//const opj_stream_private_t -> void

   *data++=0xFF;//SOT 2 bytes
   *data++=0x90;
   *data++=0x00;//Lsot 2 bytes (marker length)
   *data++=0x0A;
   *data++=j2k->m_current_tile_number+1;// m_current_tile_number is 4 bytes LE / Isot 2 bytes BE tile number
   *data++=j2k->m_current_tile_number;
    data += 4;//0x00000000 Psot 4 bytes (segment length) Only the last tile-part in the codestream may contain a 0 for Psot. If the Psot is 0, this tile-part is assumed to contain all data until the EOC marker.
   *data++=j2k->m_specific_param.m_encoder.m_current_tile_part_number;//4 bytes LE -> TPsot 1 byte
   *data++=j2k->m_cp.tcps[j2k->m_current_tile_number].m_nb_tile_parts;//4 bytes LE -> TNsot 1 byte
   *data_written = 12;
   return OPJ_TRUE;
}

static void opj_j2k_write_poc_in_memory(
   opj_j2k_t       * j2k,
   OPJ_BYTE        * data,
   OPJ_UINT32      * data_written
)
{
   OPJ_BYTE * l_current_data = data;
   // ??? OPJ_UNUSED(manager);
    OPJ_UINT32 i;
    OPJ_UINT32 l_nb_comp;
    OPJ_UINT32 l_nb_poc;
    OPJ_UINT32 l_poc_size;
    opj_image_t *l_image = 00;
    opj_tcp_t *l_tcp = 00;
    opj_tccp_t *l_tccp = 00;
    opj_poc_t *l_current_poc = 00;
    OPJ_UINT32 l_poc_room;

    

    l_tcp = &j2k->m_cp.tcps[j2k->m_current_tile_number];
    l_tccp = &l_tcp->tccps[0];
    l_image = j2k->m_private_image;
    l_nb_comp = l_image->numcomps;
    l_nb_poc = 1 + l_tcp->numpocs;

    if (l_nb_comp <= 256) {
        l_poc_room = 1;
    } else {
        l_poc_room = 2;
    }

    l_poc_size = 4 + (5 + 2 * l_poc_room) * l_nb_poc;


    opj_write_bytes(data, J2K_MS_POC,2);                                   /* POC  */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_poc_size - 2,
                    2);                                 /* Lpoc */
    l_current_data += 2;

    l_current_poc =  l_tcp->pocs;
    for (i = 0; i < l_nb_poc; ++i) {
        opj_write_bytes(l_current_data, l_current_poc->resno0,
                        1);                                /* RSpoc_i */
        ++l_current_data;

        opj_write_bytes(l_current_data, l_current_poc->compno0,
                        l_poc_room);              /* CSpoc_i */
        l_current_data += l_poc_room;

        opj_write_bytes(l_current_data, l_current_poc->layno1,
                        2);                                /* LYEpoc_i */
        l_current_data += 2;

        opj_write_bytes(l_current_data, l_current_poc->resno1,
                        1);                                /* REpoc_i */
        ++l_current_data;

        opj_write_bytes(l_current_data, l_current_poc->compno1,
                        l_poc_room);              /* CEpoc_i */
        l_current_data += l_poc_room;

        opj_write_bytes(l_current_data, (OPJ_UINT32)l_current_poc->prg,
                        1);   /* Ppoc_i */
        ++l_current_data;

        /* change the value of the max layer according to the actual number of layers in the file, components and resolutions*/
        l_current_poc->layno1 = (OPJ_UINT32)opj_int_min((OPJ_INT32)
                                l_current_poc->layno1, (OPJ_INT32)l_tcp->numlayers);
        l_current_poc->resno1 = (OPJ_UINT32)opj_int_min((OPJ_INT32)
                                l_current_poc->resno1, (OPJ_INT32)l_tccp->numresolutions);
        l_current_poc->compno1 = (OPJ_UINT32)opj_int_min((OPJ_INT32)
                                 l_current_poc->compno1, (OPJ_INT32)l_nb_comp);

        ++l_current_poc;
    }

    *data_written = l_poc_size;
}
#pragma mark - second level methods

#pragma mark call to tcd class
static OPJ_BOOL opj_j2k_pre_write_tile(opj_j2k_t * p_j2k,OPJ_UINT32 p_tile_index,opj_stream_private_t *p_stream)
{
    (void)p_stream;
    if (p_tile_index != p_j2k->m_current_tile_number) {
        opj_event_msg(p_manager, EVT_ERROR, "The given tile index does not match.");
        return OPJ_FALSE;
    }

    opj_event_msg(p_manager, EVT_INFO, "tile number %d / %d\n", p_j2k->m_current_tile_number + 1, p_j2k->m_cp.tw * p_j2k->m_cp.th);

    p_j2k->m_specific_param.m_encoder.m_current_tile_part_number = 0;
    p_j2k->m_tcd->cur_totnum_tp = p_j2k->m_cp.tcps[p_tile_index].m_nb_tile_parts;
    p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number = 0;

    /* initialisation before tile encoding  */
    if (! opj_tcd_init_encode_tile(p_j2k->m_tcd, p_j2k->m_current_tile_number,p_manager)) return OPJ_FALSE;
    return OPJ_TRUE;
}

static void opj_j2k_get_tile_data(opj_tcd_t * p_tcd, OPJ_BYTE * p_data)
{
    OPJ_UINT32 i, j, k = 0;

    for (i = 0; i < p_tcd->image->numcomps; ++i) {
        opj_image_t * l_image =  p_tcd->image;
        OPJ_INT32 * l_src_ptr;
        opj_tcd_tilecomp_t * l_tilec = p_tcd->tcd_image->tiles->comps + i;
        opj_image_comp_t * l_img_comp = l_image->comps + i;
        OPJ_UINT32 l_size_comp;

       OPJ_UINT32 l_remaining;
       l_size_comp = l_img_comp->prec >> 3; /* (/8) */
       l_remaining = l_img_comp->prec & 7;  /* (%8) */
       if (l_remaining) {
           l_size_comp += 1;
       }

       if (l_size_comp == 3) {
           l_size_comp = 4;
       }

       OPJ_UINT32 l_width  = (OPJ_UINT32)(l_tilec->x1 - l_tilec->x0);
       OPJ_UINT32 l_height = (OPJ_UINT32)(l_tilec->y1 - l_tilec->y0);
       OPJ_UINT32 l_offset_x = opj_uint_ceildiv(l_image->x0, l_img_comp->dx);
       OPJ_UINT32 l_offset_y = opj_uint_ceildiv(l_image->y0, l_img_comp->dy);
       OPJ_UINT32 l_image_width = opj_uint_ceildiv(l_image->x1 - l_image->x0, l_img_comp->dx);
       OPJ_UINT32 l_stride = l_image_width - l_width;
       OPJ_UINT32 l_tile_offset = ((OPJ_UINT32)l_tilec->x0 - l_offset_x) + ((OPJ_UINT32)l_tilec->y0 - l_offset_y) * l_image_width;

        l_src_ptr = l_img_comp->data + l_tile_offset;

        switch (l_size_comp) {
        case 1: {
            OPJ_CHAR * l_dest_ptr = (OPJ_CHAR*) p_data;
            if (l_img_comp->sgnd) {
                for (j = 0; j < l_height; ++j) {
                    for (k = 0; k < l_width; ++k) {
                        *(l_dest_ptr) = (OPJ_CHAR)(*l_src_ptr);
                        ++l_dest_ptr;
                        ++l_src_ptr;
                    }
                    l_src_ptr += l_stride;
                }
            } else {
                for (j = 0; j < l_height; ++j) {
                    for (k = 0; k < l_width; ++k) {
                        *(l_dest_ptr) = (OPJ_CHAR)((*l_src_ptr) & 0xff);
                        ++l_dest_ptr;
                        ++l_src_ptr;
                    }
                    l_src_ptr += l_stride;
                }
            }

            p_data = (OPJ_BYTE*) l_dest_ptr;
        }
        break;
        case 2: {
            OPJ_INT16 * l_dest_ptr = (OPJ_INT16 *) p_data;
            if (l_img_comp->sgnd) {
                for (j = 0; j < l_height; ++j) {
                    for (k = 0; k < l_width; ++k) {
                        *(l_dest_ptr++) = (OPJ_INT16)(*(l_src_ptr++));
                    }
                    l_src_ptr += l_stride;
                }
            } else {
                for (j = 0; j < l_height; ++j) {
                    for (k = 0; k < l_width; ++k) {
                        *(l_dest_ptr++) = (OPJ_INT16)((*(l_src_ptr++)) & 0xffff);
                    }
                    l_src_ptr += l_stride;
                }
            }

            p_data = (OPJ_BYTE*) l_dest_ptr;
        }
        break;
        case 4: {
            OPJ_INT32 * l_dest_ptr = (OPJ_INT32 *) p_data;
            for (j = 0; j < l_height; ++j) {
                for (k = 0; k < l_width; ++k) {
                    *(l_dest_ptr++) = *(l_src_ptr++);
                }
                l_src_ptr += l_stride;
            }

            p_data = (OPJ_BYTE*) l_dest_ptr;
        }
        break;
        }
    }
}


#pragma mark - write stream





static OPJ_BOOL opj_j2k_write_coc(opj_j2k_t *p_j2k,OPJ_UINT32 p_comp_no,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 l_coc_size, l_remaining_size;
    OPJ_UINT32 l_comp_room;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    l_comp_room = (p_j2k->m_private_image->numcomps <= 256) ? 1 : 2;

    l_coc_size = 5 + l_comp_room + opj_j2k_get_SPCod_SPCoc_size(p_j2k,
                 p_j2k->m_current_tile_number, p_comp_no);

    if (l_coc_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
        OPJ_BYTE *new_header_tile_data;
        /*p_j2k->m_specific_param.m_encoder.m_header_tile_data
                = (OPJ_BYTE*)realloc(
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data,
                        l_coc_size);*/

        new_header_tile_data = (OPJ_BYTE *) realloc(
                                   p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_coc_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to write COC marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_coc_size;
    }

    opj_j2k_write_coc_in_memory(p_j2k, p_comp_no,
                                p_j2k->m_specific_param.m_encoder.m_header_tile_data, &l_remaining_size,
                                p_manager);

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_coc_size) != l_coc_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}


static OPJ_BOOL opj_j2k_write_qcc(opj_j2k_t *p_j2k,OPJ_UINT32 p_comp_no,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 l_qcc_size, l_remaining_size;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    l_qcc_size = 5 + opj_j2k_get_SQcd_SQcc_size(p_j2k, p_j2k->m_current_tile_number,
                 p_comp_no);
    l_qcc_size += p_j2k->m_private_image->numcomps <= 256 ? 0 : 1;
    l_remaining_size = l_qcc_size;

    if (l_qcc_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_qcc_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to write QCC marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_qcc_size;
    }

    opj_j2k_write_qcc_in_memory(p_j2k, p_comp_no,
                                p_j2k->m_specific_param.m_encoder.m_header_tile_data, &l_remaining_size,
                                p_manager);

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_qcc_size) != l_qcc_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}



static OPJ_BOOL opj_j2k_write_rgn(opj_j2k_t *p_j2k,OPJ_UINT32 p_tile_no,OPJ_UINT32 p_comp_no,OPJ_UINT32 nb_comps,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_BYTE * l_current_data = 00;
    OPJ_UINT32 l_rgn_size;
    opj_cp_t *l_cp = 00;
    opj_tcp_t *l_tcp = 00;
    opj_tccp_t *l_tccp = 00;
    OPJ_UINT32 l_comp_room;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = &l_cp->tcps[p_tile_no];
    l_tccp = &l_tcp->tccps[p_comp_no];

    if (nb_comps <= 256) {
        l_comp_room = 1;
    } else {
        l_comp_room = 2;
    }

    l_rgn_size = 6 + l_comp_room;

    l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

    opj_write_bytes(l_current_data, J2K_MS_RGN,
                    2);                                   /* RGN  */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_rgn_size - 2,
                    2);                                 /* Lrgn */
    l_current_data += 2;

    opj_write_bytes(l_current_data, p_comp_no,
                    l_comp_room);                          /* Crgn */
    l_current_data += l_comp_room;

    opj_write_bytes(l_current_data, 0,
                    1);                                           /* Srgn */
    ++l_current_data;

    opj_write_bytes(l_current_data, (OPJ_UINT32)l_tccp->roishift,
                    1);                            /* SPrgn */
    ++l_current_data;

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_rgn_size) != l_rgn_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_eoc(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    opj_write_bytes(p_j2k->m_specific_param.m_encoder.m_header_tile_data,
                    J2K_MS_EOC, 2);                                    /* EOC */
    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, 2) != 2) {
        return OPJ_FALSE;
    }

    if (! opj_stream_flush(p_stream)) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_mct_record(opj_j2k_t *p_j2k,opj_mct_data_t * p_mct_record,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 l_mct_size;
    OPJ_BYTE * l_current_data = 00;
    OPJ_UINT32 l_tmp;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    l_mct_size = 10 + p_mct_record->m_data_size;

    if (l_mct_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_mct_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to write MCT marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_mct_size;
    }

    l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

    opj_write_bytes(l_current_data, J2K_MS_MCT,
                    2);                                   /* MCT */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_mct_size - 2,
                    2);                                 /* Lmct */
    l_current_data += 2;

    opj_write_bytes(l_current_data, 0,
                    2);                                                    /* Zmct */
    l_current_data += 2;

    /* only one marker atm */
    l_tmp = (p_mct_record->m_index & 0xff) | (p_mct_record->m_array_type << 8) |
            (p_mct_record->m_element_type << 10);

    opj_write_bytes(l_current_data, l_tmp, 2);
    l_current_data += 2;

    opj_write_bytes(l_current_data, 0,
                    2);                                                    /* Ymct */
    l_current_data += 2;

    memcpy(l_current_data, p_mct_record->m_data, p_mct_record->m_data_size);

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_mct_size) != l_mct_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_mcc_record(opj_j2k_t *p_j2k,struct opj_simple_mcc_decorrelation_data * p_mcc_record,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 i;
    OPJ_UINT32 l_mcc_size;
    OPJ_BYTE * l_current_data = 00;
    OPJ_UINT32 l_nb_bytes_for_comp;
    OPJ_UINT32 l_mask;
    OPJ_UINT32 l_tmcc;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    if (p_mcc_record->m_nb_comps > 255) {
        l_nb_bytes_for_comp = 2;
        l_mask = 0x8000;
    } else {
        l_nb_bytes_for_comp = 1;
        l_mask = 0;
    }

    l_mcc_size = p_mcc_record->m_nb_comps * 2 * l_nb_bytes_for_comp + 19;
    if (l_mcc_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_mcc_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to write MCC marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_mcc_size;
    }

    l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

    opj_write_bytes(l_current_data, J2K_MS_MCC,
                    2);                                   /* MCC */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_mcc_size - 2,
                    2);                                 /* Lmcc */
    l_current_data += 2;

    /* first marker */
    opj_write_bytes(l_current_data, 0,
                    2);                                  /* Zmcc */
    l_current_data += 2;

    opj_write_bytes(l_current_data, p_mcc_record->m_index,
                    1);                                        /* Imcc -> no need for other values, take the first */
    ++l_current_data;

    /* only one marker atm */
    opj_write_bytes(l_current_data, 0,
                    2);                                  /* Ymcc */
    l_current_data += 2;

    opj_write_bytes(l_current_data, 1,
                    2);                                  /* Qmcc -> number of collections -> 1 */
    l_current_data += 2;

    opj_write_bytes(l_current_data, 0x1,
                    1);                                /* Xmcci type of component transformation -> array based decorrelation */
    ++l_current_data;

    opj_write_bytes(l_current_data, p_mcc_record->m_nb_comps | l_mask,
                    2);  /* Nmcci number of input components involved and size for each component offset = 8 bits */
    l_current_data += 2;

    for (i = 0; i < p_mcc_record->m_nb_comps; ++i) {
        opj_write_bytes(l_current_data, i,
                        l_nb_bytes_for_comp);                          /* Cmccij Component offset*/
        l_current_data += l_nb_bytes_for_comp;
    }

    opj_write_bytes(l_current_data, p_mcc_record->m_nb_comps | l_mask,
                    2);  /* Mmcci number of output components involved and size for each component offset = 8 bits */
    l_current_data += 2;

    for (i = 0; i < p_mcc_record->m_nb_comps; ++i) {
        opj_write_bytes(l_current_data, i,
                        l_nb_bytes_for_comp);                          /* Wmccij Component offset*/
        l_current_data += l_nb_bytes_for_comp;
    }

    l_tmcc = ((!p_mcc_record->m_is_irreversible) & 1U) << 16;

    if (p_mcc_record->m_decorrelation_array) {
        l_tmcc |= p_mcc_record->m_decorrelation_array->m_index;
    }

    if (p_mcc_record->m_offset_array) {
        l_tmcc |= ((p_mcc_record->m_offset_array->m_index) << 8);
    }

    opj_write_bytes(l_current_data, l_tmcc,
                    3);     /* Tmcci : use MCT defined as number 1 and irreversible array based. */
    l_current_data += 3;

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_mcc_size) != l_mcc_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_mco(opj_j2k_t *p_j2k,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    OPJ_BYTE * l_current_data = 00;
    OPJ_UINT32 l_mco_size;
    opj_tcp_t * l_tcp = 00;
    opj_simple_mcc_decorrelation_data_t * l_mcc_record;
    OPJ_UINT32 i;

    l_tcp = &(p_j2k->m_cp.tcps[p_j2k->m_current_tile_number]);

    l_mco_size = 5 + l_tcp->m_nb_mcc_records;
    if (l_mco_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {

        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_mco_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to write MCO marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_mco_size;
    }
    l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;


    opj_write_bytes(l_current_data, J2K_MS_MCO, 2);                 /* MCO */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_mco_size - 2, 2);             /* Lmco */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_tcp->m_nb_mcc_records,
                    1);    /* Nmco : only one transform stage*/
    ++l_current_data;

    l_mcc_record = l_tcp->m_mcc_records;
    for (i = 0; i < l_tcp->m_nb_mcc_records; ++i) {
        opj_write_bytes(l_current_data, l_mcc_record->m_index,
                        1); /* Imco -> use the mcc indicated by 1*/
        ++l_current_data;
        ++l_mcc_record;
    }

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_mco_size) != l_mco_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_cbd(opj_j2k_t *p_j2k,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 i;
    OPJ_UINT32 l_cbd_size;
    OPJ_BYTE * l_current_data = 00;
    opj_image_t *l_image = 00;
    opj_image_comp_t * l_comp = 00;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    l_image = p_j2k->m_private_image;
    l_cbd_size = 6 + p_j2k->m_private_image->numcomps;

    if (l_cbd_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_cbd_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to write CBD marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_cbd_size;
    }

    l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

    opj_write_bytes(l_current_data, J2K_MS_CBD, 2);                 /* CBD */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_cbd_size - 2, 2);             /* L_CBD */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_image->numcomps, 2);          /* Ncbd */
    l_current_data += 2;

    l_comp = l_image->comps;

    for (i = 0; i < l_image->numcomps; ++i) {
        opj_write_bytes(l_current_data, (l_comp->sgnd << 7) | (l_comp->prec - 1),
                        1);           /* Component bit depth */
        ++l_current_data;

        ++l_comp;
    }

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_cbd_size) != l_cbd_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_updated_tlm(opj_j2k_t *p_j2k,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 l_tlm_size;
    OPJ_OFF_T l_tlm_position, l_current_position;
    OPJ_UINT32 size_per_tile_part;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    size_per_tile_part = p_j2k->m_specific_param.m_encoder.m_Ttlmi_is_byte ? 5 : 6;
    l_tlm_size = size_per_tile_part *
                 p_j2k->m_specific_param.m_encoder.m_total_tile_parts;
    l_tlm_position = 6 + p_j2k->m_specific_param.m_encoder.m_tlm_start;
    l_current_position = opj_stream_tell(p_stream);

    if (! opj_stream_seek(p_stream, l_tlm_position, p_manager)) {
        return OPJ_FALSE;
    }

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer, l_tlm_size) != l_tlm_size) {
        return OPJ_FALSE;
    }

    if (! opj_stream_seek(p_stream, l_current_position, p_manager)) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}


static OPJ_BOOL opj_j2k_post_write_tile(
                                        opj_j2k_t * p_j2k,
                                        opj_stream_private_t *p_stream
                                        )
{
   unsigned char *encoded_tile_data=p_j2k->m_specific_param.m_encoder.m_encoded_tile_data;//2
   unsigned char *begin_data       =p_j2k->m_specific_param.m_encoder.m_encoded_tile_data;//2
   uint32_t total_bytes_written    =0;
   uint32_t bytes_written          =0;
   uint32_t encoded_tile_size      = p_j2k->m_specific_param.m_encoder.m_encoded_tile_size;
   uint32_t bytes_available        = p_j2k->m_specific_param.m_encoder.m_encoded_tile_size;

//start opj_j2k_write_first_tile_part(p_j2k, l_current_data, &l_nb_bytes_written,l_available_data, p_stream, p_manager)

   opj_tcd_t * l_tcd = p_j2k->m_tcd;
   l_tcd->cur_pino   = 0;
   opj_cp_t  * l_cp  = &(p_j2k->m_cp);
   p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number = 0;

if (! opj_j2k_write_sot(
          p_j2k,
          encoded_tile_data,
          encoded_tile_size,
          &bytes_written,
          p_stream
   )) return OPJ_FALSE;
   total_bytes_written += bytes_written;
   encoded_tile_data   += bytes_written;
   encoded_tile_size   -= bytes_written;

   if (!OPJ_IS_CINEMA(l_cp->rsiz)) {
       if (l_cp->tcps[p_j2k->m_current_tile_number].POC) {
           bytes_written = 0;
opj_j2k_write_poc_in_memory(p_j2k,encoded_tile_data,&bytes_written);
           total_bytes_written += bytes_written;
           encoded_tile_data += bytes_written;
           encoded_tile_size -= bytes_written;
       }
   }

   bytes_written = 0;
   if (! opj_j2k_write_sod(
                           p_j2k,
                           l_tcd,
                           encoded_tile_data,
                           &bytes_written,
                           encoded_tile_size,
                           p_stream
                           )
       ) {
       return OPJ_FALSE;
   }

   total_bytes_written += bytes_written;
#pragma mark verify
//JF ????   * p_data_written = total_bytes_written;

   /* Writing Psot in SOT marker */
   opj_write_bytes(begin_data + 6, total_bytes_written,4);

   if (p_j2k->m_specific_param.m_encoder.m_TLM) {
       opj_j2k_update_tlm(p_j2k, total_bytes_written);
   }
//end opj_j2k_write_first_tile_part
   
   
   
   
    encoded_tile_data += total_bytes_written;
    bytes_available -= total_bytes_written;

   total_bytes_written = 0;
if (! opj_j2k_write_all_tile_parts(p_j2k, encoded_tile_data, &total_bytes_written,
                                       bytes_available, p_stream)) {
        return OPJ_FALSE;
    }

    bytes_available -= total_bytes_written;
   total_bytes_written = encoded_tile_size - bytes_available;

#pragma mark opj_stream_write_data
if (opj_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_encoded_tile_data,total_bytes_written) != total_bytes_written) return OPJ_FALSE;

   ++p_j2k->m_current_tile_number;
   return OPJ_TRUE;
}


#pragma mark - procedures execution

static OPJ_BOOL opj_j2k_exec(
 opj_j2k_t            * p_j2k,
 opj_procedure_list_t * p_procedure_list,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t * p_manager
)
{
    //signature fo the methods (procedures): (OPJ_BOOL(**)(opj_j2k_t *, opj_stream_private_t *,opj_event_mgr_t *))
    //¿¿¿initiation zero response of procedure ??? ¿¿¿ Does it do something ???
    OPJ_BOOL(** l_procedure)(opj_j2k_t *, opj_stream_private_t *,opj_event_mgr_t *) = 00;
    OPJ_BOOL l_result = OPJ_TRUE;
    OPJ_UINT32 l_nb_proc, i;

    /* preconditions*/
    assert(p_procedure_list != 00);
    assert(p_j2k != 00);
    assert(p_stream != 00);
    assert(p_manager != 00);

    l_nb_proc = opj_procedure_list_get_nb_procedures(p_procedure_list);
    l_procedure = (OPJ_BOOL(**)(opj_j2k_t *, opj_stream_private_t *,opj_event_mgr_t *)) opj_procedure_list_get_first_procedure(p_procedure_list);

    for (i = 0; i < l_nb_proc; ++i) {
        l_result = l_result && ((*l_procedure)(p_j2k, p_stream, p_manager));//executes l_procedure
        ++l_procedure;
    }

    /* and clear the procedure list at the end.*/
    opj_procedure_list_clear(p_procedure_list);
    return l_result;
}


#pragma mark - start_compress p_j2k->m_procedure_list


static OPJ_BOOL opj_j2k_write_soc(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_BYTE * l_start_stream = 00;
    l_start_stream = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

    /* write SOC identifier */
    opj_write_bytes(l_start_stream, J2K_MS_SOC, 2);//dst,src,siz   0xff4f

    if (opj_stream_write_data(p_stream, l_start_stream, 2) != 2) {
        return OPJ_FALSE;
    }
    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_siz(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 i;
    OPJ_UINT32 l_size_len;
    OPJ_BYTE * l_current_ptr;
    opj_image_t * l_image = 00;
    opj_cp_t *cp = 00;
    opj_image_comp_t * l_img_comp = 00;

    /* preconditions */
    assert(p_stream != 00);
    assert(p_j2k != 00);
    assert(p_manager != 00);

    l_image = p_j2k->m_private_image;
    cp = &(p_j2k->m_cp);
    l_size_len = 40 + 3 * l_image->numcomps;
    l_img_comp = l_image->comps;

    if (l_size_len > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {

        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_size_len);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory for the SIZ marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_size_len;
    }

    l_current_ptr = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

    /* write SOC identifier */
    opj_write_bytes(l_current_ptr, J2K_MS_SIZ, 2);  /* SIZ */
    l_current_ptr += 2;

    opj_write_bytes(l_current_ptr, l_size_len - 2, 2); /* L_SIZ */
    l_current_ptr += 2;

    opj_write_bytes(l_current_ptr, cp->rsiz, 2);    /* Rsiz (capabilities) */
    l_current_ptr += 2;

    opj_write_bytes(l_current_ptr, l_image->x1, 4); /* Xsiz */
    l_current_ptr += 4;

    opj_write_bytes(l_current_ptr, l_image->y1, 4); /* Ysiz */
    l_current_ptr += 4;

    opj_write_bytes(l_current_ptr, l_image->x0, 4); /* X0siz */
    l_current_ptr += 4;

    opj_write_bytes(l_current_ptr, l_image->y0, 4); /* Y0siz */
    l_current_ptr += 4;

    opj_write_bytes(l_current_ptr, cp->tdx, 4);             /* XTsiz */
    l_current_ptr += 4;

    opj_write_bytes(l_current_ptr, cp->tdy, 4);             /* YTsiz */
    l_current_ptr += 4;

    opj_write_bytes(l_current_ptr, cp->tx0, 4);             /* XT0siz */
    l_current_ptr += 4;

    opj_write_bytes(l_current_ptr, cp->ty0, 4);             /* YT0siz */
    l_current_ptr += 4;

    opj_write_bytes(l_current_ptr, l_image->numcomps, 2);   /* Csiz */
    l_current_ptr += 2;

    for (i = 0; i < l_image->numcomps; ++i) {
        /* TODO here with MCT ? */
        opj_write_bytes(l_current_ptr, l_img_comp->prec - 1 + (l_img_comp->sgnd << 7),
                        1);      /* Ssiz_i */
        ++l_current_ptr;

        opj_write_bytes(l_current_ptr, l_img_comp->dx, 1);      /* XRsiz_i */
        ++l_current_ptr;

        opj_write_bytes(l_current_ptr, l_img_comp->dy, 1);      /* YRsiz_i */
        ++l_current_ptr;

        ++l_img_comp;
    }

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_size_len) != l_size_len) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_cod(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    opj_cp_t *l_cp = 00;
    opj_tcp_t *l_tcp = 00;
    OPJ_UINT32 l_code_size, l_remaining_size;
    OPJ_BYTE * l_current_data = 00;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    l_cp = &(p_j2k->m_cp);
    l_tcp = &l_cp->tcps[p_j2k->m_current_tile_number];
    l_code_size = 9 + opj_j2k_get_SPCod_SPCoc_size(p_j2k,
                  p_j2k->m_current_tile_number, 0);
    l_remaining_size = l_code_size;

    if (l_code_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_code_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to write COD marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_code_size;
    }

    l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

    opj_write_bytes(l_current_data, J2K_MS_COD, 2);           /* COD */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_code_size - 2, 2);      /* L_COD */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_tcp->csty, 1);          /* Scod */
    ++l_current_data;

    opj_write_bytes(l_current_data, (OPJ_UINT32)l_tcp->prg, 1); /* SGcod (A) */
    ++l_current_data;

    opj_write_bytes(l_current_data, l_tcp->numlayers, 2);     /* SGcod (B) */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_tcp->mct, 1);           /* SGcod (C) */
    ++l_current_data;

    l_remaining_size -= 9;

    if (! opj_j2k_write_SPCod_SPCoc(p_j2k, p_j2k->m_current_tile_number, 0,
                                    l_current_data, &l_remaining_size, p_manager)) {
        opj_event_msg(p_manager, EVT_ERROR, "Error writing COD marker\n");
        return OPJ_FALSE;
    }

    if (l_remaining_size != 0) {
        opj_event_msg(p_manager, EVT_ERROR, "Error writing COD marker\n");
        return OPJ_FALSE;
    }

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_code_size) != l_code_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_qcd(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 l_qcd_size, l_remaining_size;
    OPJ_BYTE * l_current_data = 00;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    l_qcd_size = 4 + opj_j2k_get_SQcd_SQcc_size(p_j2k, p_j2k->m_current_tile_number,
                 0);
    l_remaining_size = l_qcd_size;

    if (l_qcd_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_qcd_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to write QCD marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_qcd_size;
    }

    l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

    opj_write_bytes(l_current_data, J2K_MS_QCD, 2);         /* QCD */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_qcd_size - 2, 2);     /* L_QCD */
    l_current_data += 2;

    l_remaining_size -= 4;

    if (! opj_j2k_write_SQcd_SQcc(p_j2k, p_j2k->m_current_tile_number, 0,
                                  l_current_data, &l_remaining_size, p_manager)) {
        opj_event_msg(p_manager, EVT_ERROR, "Error writing QCD marker\n");
        return OPJ_FALSE;
    }

    if (l_remaining_size != 0) {
        opj_event_msg(p_manager, EVT_ERROR, "Error writing QCD marker\n");
        return OPJ_FALSE;
    }

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_qcd_size) != l_qcd_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_all_coc(opj_j2k_t *p_j2k,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 compno;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    for (compno = 1; compno < p_j2k->m_private_image->numcomps; ++compno) {
        /* cod is first component of first tile */
        if (! opj_j2k_compare_coc(p_j2k, 0, compno)) {
            if (! opj_j2k_write_coc(p_j2k, compno, p_stream, p_manager)) {
                return OPJ_FALSE;
            }
        }
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_all_qcc(opj_j2k_t *p_j2k,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 compno;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    for (compno = 1; compno < p_j2k->m_private_image->numcomps; ++compno) {
        /* qcd is first component of first tile */
        if (! opj_j2k_compare_qcc(p_j2k, 0, compno)) {
            if (! opj_j2k_write_qcc(p_j2k, compno, p_stream, p_manager)) {
                return OPJ_FALSE;
            }
        }
    }
    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_tlm(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_BYTE * l_current_data = 00;
    OPJ_UINT32 l_tlm_size;
    OPJ_UINT32 size_per_tile_part;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    /* 10921 = (65535 - header_size) / size_per_tile_part where */
    /* header_size = 4 and size_per_tile_part = 6 */
    if (p_j2k->m_specific_param.m_encoder.m_total_tile_parts > 10921) {
        /* We could do more but it would require writing several TLM markers */
        opj_event_msg(p_manager, EVT_ERROR,
                      "A maximum of 10921 tile-parts are supported currently "
                      "when writing TLM marker\n");
        return OPJ_FALSE;
    }

    if (p_j2k->m_specific_param.m_encoder.m_total_tile_parts <= 255) {
        size_per_tile_part = 5;
        p_j2k->m_specific_param.m_encoder.m_Ttlmi_is_byte = OPJ_TRUE;
    } else {
        size_per_tile_part = 6;
        p_j2k->m_specific_param.m_encoder.m_Ttlmi_is_byte = OPJ_FALSE;
    }

    l_tlm_size = 2 + 4 + (size_per_tile_part *
                          p_j2k->m_specific_param.m_encoder.m_total_tile_parts);

    if (l_tlm_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_tlm_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to write TLM marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_tlm_size;
    }
    memset(p_j2k->m_specific_param.m_encoder.m_header_tile_data, 0, l_tlm_size);

    l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

    /* change the way data is written to avoid seeking if possible */
    /* TODO */
    p_j2k->m_specific_param.m_encoder.m_tlm_start = opj_stream_tell(p_stream);

    opj_write_bytes(l_current_data, J2K_MS_TLM,
                    2);                                   /* TLM */
    l_current_data += 2;

    opj_write_bytes(l_current_data, l_tlm_size - 2,
                    2);                                 /* Lpoc */
    l_current_data += 2;

    opj_write_bytes(l_current_data, 0,
                    1);                                                    /* Ztlm=0*/
    ++l_current_data;

    /* Stlm 0x50= ST=1(8bits-255 tiles max),SP=1(Ptlm=32bits) */
    /* Stlm 0x60= ST=2(16bits-65535 tiles max),SP=1(Ptlm=32bits) */
    opj_write_bytes(l_current_data,
                    size_per_tile_part == 5 ? 0x50 : 0x60,
                    1);
    ++l_current_data;

    /* do nothing on the size_per_tile_part * l_j2k->m_specific_param.m_encoder.m_total_tile_parts remaining data */
    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_tlm_size) != l_tlm_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_poc(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 l_nb_comp;
    OPJ_UINT32 l_nb_poc;
    OPJ_UINT32 l_poc_size;
    OPJ_UINT32 l_written_size = 0;
    opj_tcp_t *l_tcp = 00;
    OPJ_UINT32 l_poc_room;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    l_tcp = &p_j2k->m_cp.tcps[p_j2k->m_current_tile_number];
    l_nb_comp = p_j2k->m_private_image->numcomps;
    l_nb_poc = 1 + l_tcp->numpocs;

    if (l_nb_comp <= 256) {
        l_poc_room = 1;
    } else {
        l_poc_room = 2;
    }
    l_poc_size = 4 + (5 + 2 * l_poc_room) * l_nb_poc;

    if (l_poc_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_poc_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to write POC marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_poc_size;
    }

    opj_j2k_write_poc_in_memory(p_j2k,
                                p_j2k->m_specific_param.m_encoder.m_header_tile_data, &l_written_size);

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_poc_size) != l_poc_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_regions(opj_j2k_t *p_j2k,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 compno;
    const opj_tccp_t *l_tccp = 00;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    l_tccp = p_j2k->m_cp.tcps->tccps;

    for (compno = 0; compno < p_j2k->m_private_image->numcomps; ++compno)  {
        if (l_tccp->roishift) {

            if (! opj_j2k_write_rgn(p_j2k, 0, compno, p_j2k->m_private_image->numcomps,
                                    p_stream, p_manager)) {
                return OPJ_FALSE;
            }
        }

        ++l_tccp;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_com(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 l_comment_size;
    OPJ_UINT32 l_total_com_size;
    const OPJ_CHAR *l_comment;
    OPJ_BYTE * l_current_ptr = 00;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_stream != 00);
    assert(p_manager != 00);

    l_comment = p_j2k->m_cp.comment;
    l_comment_size = (OPJ_UINT32)strlen(l_comment);
    l_total_com_size = l_comment_size + 6;

    if (l_total_com_size >
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
        OPJ_BYTE *new_header_tile_data = (OPJ_BYTE *) realloc(
                                             p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_total_com_size);
        if (! new_header_tile_data) {
            free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
            p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
            p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
            opj_event_msg(p_manager, EVT_ERROR,
                          "Not enough memory to write the COM marker\n");
            return OPJ_FALSE;
        }
        p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_total_com_size;
    }

    l_current_ptr = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

    opj_write_bytes(l_current_ptr, J2K_MS_COM, 2);  /* COM */
    l_current_ptr += 2;

    opj_write_bytes(l_current_ptr, l_total_com_size - 2, 2);        /* L_COM */
    l_current_ptr += 2;

    opj_write_bytes(l_current_ptr, 1,
                    2);   /* General use (IS 8859-15:1999 (Latin) values) */
    l_current_ptr += 2;

    memcpy(l_current_ptr, l_comment, l_comment_size);

    if (opj_stream_write_data(p_stream,
                              p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_total_com_size) != l_total_com_size) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_write_mct_data_group(opj_j2k_t *p_j2k,struct opj_stream_private *p_stream,struct opj_event_mgr * p_manager)
{
    OPJ_UINT32 i;
    opj_simple_mcc_decorrelation_data_t * l_mcc_record;
    opj_mct_data_t * l_mct_record;
    opj_tcp_t * l_tcp;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_stream != 00);
    assert(p_manager != 00);

    if (! opj_j2k_write_cbd(p_j2k, p_stream, p_manager)) {
        return OPJ_FALSE;
    }

    l_tcp = &(p_j2k->m_cp.tcps[p_j2k->m_current_tile_number]);
    l_mct_record = l_tcp->m_mct_records;

    for (i = 0; i < l_tcp->m_nb_mct_records; ++i) {

        if (! opj_j2k_write_mct_record(p_j2k, l_mct_record, p_stream, p_manager)) {
            return OPJ_FALSE;
        }

        ++l_mct_record;
    }

    l_mcc_record = l_tcp->m_mcc_records;

    for (i = 0; i < l_tcp->m_nb_mcc_records; ++i) {

        if (! opj_j2k_write_mcc_record(p_j2k, l_mcc_record, p_stream, p_manager)) {
            return OPJ_FALSE;
        }

        ++l_mcc_record;
    }

    if (! opj_j2k_write_mco(p_j2k, p_stream, p_manager)) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_create_tcd(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
   /**
    * Creates a tile-coder encoder.
    *
    * @param       p_stream                the stream to write data to.
    * @param       p_j2k                   J2K codec.
    * @param       p_manager               the user event manager.
   */

    OPJ_UNUSED(p_stream);

    p_j2k->m_tcd = opj_tcd_create(OPJ_FALSE);

    if (! p_j2k->m_tcd) {
        opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to create Tile Coder\n");
        return OPJ_FALSE;
    }

    if (!opj_tcd_init(p_j2k->m_tcd, p_j2k->m_private_image, &p_j2k->m_cp,
                      p_j2k->m_tp)) {
        opj_tcd_destroy(p_j2k->m_tcd);
        p_j2k->m_tcd = 00;
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_j2k_update_rates(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
    opj_cp_t * l_cp = 00;
    opj_image_t * l_image = 00;
    opj_tcp_t * l_tcp = 00;
    opj_image_comp_t * l_img_comp = 00;

    OPJ_UINT32 i, j, k;
    OPJ_INT32 l_x0, l_y0, l_x1, l_y1;
    OPJ_FLOAT32 * l_rates = 0;
    OPJ_FLOAT32 l_sot_remove;
    OPJ_UINT32 l_bits_empty, l_size_pixel;
    OPJ_UINT64 l_tile_size = 0;
    OPJ_UINT32 l_last_res;
    OPJ_FLOAT32(* l_tp_stride_func)(opj_tcp_t *) = 00;

    /* preconditions */
    assert(p_j2k != 00);
    assert(p_manager != 00);
    assert(p_stream != 00);

    OPJ_UNUSED(p_manager);

    l_cp = &(p_j2k->m_cp);
    l_image = p_j2k->m_private_image;
    l_tcp = l_cp->tcps;

    l_bits_empty = 8 * l_image->comps->dx * l_image->comps->dy;
    l_size_pixel = l_image->numcomps * l_image->comps->prec;
    l_sot_remove = (OPJ_FLOAT32) opj_stream_tell(p_stream) / (OPJ_FLOAT32)(
                       l_cp->th * l_cp->tw);

    if (l_cp->m_specific_param.m_enc.m_tp_on) {
        l_tp_stride_func = opj_j2k_get_tp_stride;
    } else {
        l_tp_stride_func = opj_j2k_get_default_stride;
    }

    for (i = 0; i < l_cp->th; ++i) {
        for (j = 0; j < l_cp->tw; ++j) {
            OPJ_FLOAT32 l_offset = (OPJ_FLOAT32)(*l_tp_stride_func)(l_tcp) /
                                   (OPJ_FLOAT32)l_tcp->numlayers;

            /* 4 borders of the tile rescale on the image if necessary */
            l_x0 = opj_int_max((OPJ_INT32)(l_cp->tx0 + j * l_cp->tdx),
                               (OPJ_INT32)l_image->x0);
            l_y0 = opj_int_max((OPJ_INT32)(l_cp->ty0 + i * l_cp->tdy),
                               (OPJ_INT32)l_image->y0);
            l_x1 = opj_int_min((OPJ_INT32)(l_cp->tx0 + (j + 1) * l_cp->tdx),
                               (OPJ_INT32)l_image->x1);
            l_y1 = opj_int_min((OPJ_INT32)(l_cp->ty0 + (i + 1) * l_cp->tdy),
                               (OPJ_INT32)l_image->y1);

            l_rates = l_tcp->rates;

            /* Modification of the RATE >> */
            for (k = 0; k < l_tcp->numlayers; ++k) {
                if (*l_rates > 0.0f) {
                    *l_rates = (OPJ_FLOAT32)(((OPJ_FLOAT64)l_size_pixel * (OPJ_UINT32)(
                                                  l_x1 - l_x0) *
                                              (OPJ_UINT32)(l_y1 - l_y0))
                                             / ((*l_rates) * (OPJ_FLOAT32)l_bits_empty))
                               -
                               l_offset;
                }

                ++l_rates;
            }

            ++l_tcp;

        }
    }

    l_tcp = l_cp->tcps;

    for (i = 0; i < l_cp->th; ++i) {
        for (j = 0; j < l_cp->tw; ++j) {
            l_rates = l_tcp->rates;

            if (*l_rates > 0.0f) {
                *l_rates -= l_sot_remove;

                if (*l_rates < 30.0f) {
                    *l_rates = 30.0f;
                }
            }

            ++l_rates;

            l_last_res = l_tcp->numlayers - 1;

            for (k = 1; k < l_last_res; ++k) {

                if (*l_rates > 0.0f) {
                    *l_rates -= l_sot_remove;

                    if (*l_rates < * (l_rates - 1) + 10.0f) {
                        *l_rates  = (*(l_rates - 1)) + 20.0f;
                    }
                }

                ++l_rates;
            }

            if (*l_rates > 0.0f) {
                *l_rates -= (l_sot_remove + 2.f);

                if (*l_rates < * (l_rates - 1) + 10.0f) {
                    *l_rates  = (*(l_rates - 1)) + 20.0f;
                }
            }

            ++l_tcp;
        }
    }

    l_img_comp = l_image->comps;
    l_tile_size = 0;

    for (i = 0; i < l_image->numcomps; ++i) {
        l_tile_size += (OPJ_UINT64)opj_uint_ceildiv(l_cp->tdx, l_img_comp->dx)
                       *
                       opj_uint_ceildiv(l_cp->tdy, l_img_comp->dy)
                       *
                       l_img_comp->prec;

        ++l_img_comp;
    }

    /* TODO: where does this magic value come from ? */
    /* This used to be 1.3 / 8, but with random data and very small code */
    /* block sizes, this is not enough. For example with */
    /* bin/test_tile_encoder 1 256 256 32 32 8 0 reversible_with_precinct.j2k 4 4 3 0 0 1 16 16 */
    /* TODO revise this to take into account the overhead linked to the */
    /* number of packets and number of code blocks in packets */
    l_tile_size = (OPJ_UINT64)((double)l_tile_size * 1.4 / 8);

    /* Arbitrary amount to make the following work: */
    /* bin/test_tile_encoder 1 256 256 17 16 8 0 reversible_no_precinct.j2k 4 4 3 0 0 1 */
    l_tile_size += 500;

    l_tile_size += opj_j2k_get_specific_header_sizes(p_j2k);

    if (l_tile_size > UINT_MAX) {
        l_tile_size = UINT_MAX;
    }

    p_j2k->m_specific_param.m_encoder.m_encoded_tile_size = (OPJ_UINT32)l_tile_size;
    p_j2k->m_specific_param.m_encoder.m_encoded_tile_data =
        (OPJ_BYTE *) malloc(p_j2k->m_specific_param.m_encoder.m_encoded_tile_size);
    if (p_j2k->m_specific_param.m_encoder.m_encoded_tile_data == 00) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Not enough memory to allocate m_encoded_tile_data. %u MB required\n",
                      (OPJ_UINT32)(l_tile_size / 1024 / 1024));
        return OPJ_FALSE;
    }

    if (p_j2k->m_specific_param.m_encoder.m_TLM) {
        p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer =
            (OPJ_BYTE *) malloc(6 *
                                    p_j2k->m_specific_param.m_encoder.m_total_tile_parts);
        if (! p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer) {
            return OPJ_FALSE;
        }

        p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current =
            p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer;
    }

    return OPJ_TRUE;
}


#pragma mark - setup and encode first level methods

#pragma mark (1)
OPJ_BOOL opj_j2k_setup_encoder(
 opj_j2k_t *p_j2k,
 opj_cparameters_t *parameters,
 opj_image_t *image,
 opj_event_mgr_t * p_manager
)
{
    OPJ_UINT32 i;
    OPJ_UINT32 j;
    OPJ_UINT32 tileno;
    OPJ_UINT32 numpocs_tile;
    opj_cp_t *cp = 00;
    OPJ_UINT32 cblkw;
    OPJ_UINT32 cblkh;

    if (parameters->cblockw_init < 4 || parameters->cblockw_init > 1024) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Invalid value for cblockw_init: %d not a power of 2 in range [4,1024]\n",
                      parameters->cblockw_init);
        return OPJ_FALSE;
    }
    if (parameters->cblockh_init < 4 || parameters->cblockh_init > 1024) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Invalid value for cblockh_init: %d not a power of 2 not in range [4,1024]\n",
                      parameters->cblockh_init);
        return OPJ_FALSE;
    }
    if (parameters->cblockw_init * parameters->cblockh_init > 4096) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Invalid value for cblockw_init * cblockh_init: should be <= 4096\n");
        return OPJ_FALSE;
    }
    cblkw = (OPJ_UINT32)opj_int_floorlog2(parameters->cblockw_init);
    cblkh = (OPJ_UINT32)opj_int_floorlog2(parameters->cblockh_init);
    if (parameters->cblockw_init != (1 << cblkw)) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Invalid value for cblockw_init: %d not a power of 2 in range [4,1024]\n",
                      parameters->cblockw_init);
        return OPJ_FALSE;
    }
    if (parameters->cblockh_init != (1 << cblkh)) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Invalid value for cblockw_init: %d not a power of 2 in range [4,1024]\n",
                      parameters->cblockh_init);
        return OPJ_FALSE;
    }

    if (parameters->cp_fixed_alloc) {
        if (parameters->cp_matrice == NULL) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "cp_fixed_alloc set, but cp_matrice missing\n");
            return OPJ_FALSE;
        }

        if (parameters->tcp_numlayers > J2K_TCD_MATRIX_MAX_LAYER_COUNT) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "tcp_numlayers when cp_fixed_alloc set should not exceed %d\n",
                          J2K_TCD_MATRIX_MAX_LAYER_COUNT);
            return OPJ_FALSE;
        }
        if (parameters->numresolution > J2K_TCD_MATRIX_MAX_RESOLUTION_COUNT) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "numresolution when cp_fixed_alloc set should not exceed %d\n",
                          J2K_TCD_MATRIX_MAX_RESOLUTION_COUNT);
            return OPJ_FALSE;
        }
    }

    p_j2k->m_specific_param.m_encoder.m_nb_comps = image->numcomps;

    /* keep a link to cp so that we can destroy it later in j2k_destroy_compress */
    cp = &(p_j2k->m_cp);

    /* set default values for cp */
    cp->tw = 1;
    cp->th = 1;

    /* If no explicit layers are provided, use lossless settings */
    if (parameters->tcp_numlayers == 0) {
        parameters->tcp_numlayers = 1;
        parameters->cp_disto_alloc = 1;
        parameters->tcp_rates[0] = 0;
    }

    if (parameters->cp_disto_alloc) {
        /* Emit warnings if tcp_rates are not decreasing */
        for (i = 1; i < (OPJ_UINT32) parameters->tcp_numlayers; i++) {
            OPJ_FLOAT32 rate_i_corr = parameters->tcp_rates[i];
            OPJ_FLOAT32 rate_i_m_1_corr = parameters->tcp_rates[i - 1];
            if (rate_i_corr <= 1.0) {
                rate_i_corr = 1.0;
            }
            if (rate_i_m_1_corr <= 1.0) {
                rate_i_m_1_corr = 1.0;
            }
            if (rate_i_corr >= rate_i_m_1_corr) {
                if (rate_i_corr != parameters->tcp_rates[i] &&
                        rate_i_m_1_corr != parameters->tcp_rates[i - 1]) {
                    opj_event_msg(p_manager, EVT_WARNING,
                                  "tcp_rates[%d]=%f (corrected as %f) should be strictly lesser "
                                  "than tcp_rates[%d]=%f (corrected as %f)\n",
                                  i, parameters->tcp_rates[i], rate_i_corr,
                                  i - 1, parameters->tcp_rates[i - 1], rate_i_m_1_corr);
                } else if (rate_i_corr != parameters->tcp_rates[i]) {
                    opj_event_msg(p_manager, EVT_WARNING,
                                  "tcp_rates[%d]=%f (corrected as %f) should be strictly lesser "
                                  "than tcp_rates[%d]=%f\n",
                                  i, parameters->tcp_rates[i], rate_i_corr,
                                  i - 1, parameters->tcp_rates[i - 1]);
                } else if (rate_i_m_1_corr != parameters->tcp_rates[i - 1]) {
                    opj_event_msg(p_manager, EVT_WARNING,
                                  "tcp_rates[%d]=%f should be strictly lesser "
                                  "than tcp_rates[%d]=%f (corrected as %f)\n",
                                  i, parameters->tcp_rates[i],
                                  i - 1, parameters->tcp_rates[i - 1], rate_i_m_1_corr);
                } else {
                    opj_event_msg(p_manager, EVT_WARNING,
                                  "tcp_rates[%d]=%f should be strictly lesser "
                                  "than tcp_rates[%d]=%f\n",
                                  i, parameters->tcp_rates[i],
                                  i - 1, parameters->tcp_rates[i - 1]);
                }
            }
        }
    } else if (parameters->cp_fixed_quality) {
        /* Emit warnings if tcp_distoratio are not increasing */
        for (i = 1; i < (OPJ_UINT32) parameters->tcp_numlayers; i++) {
            if (parameters->tcp_distoratio[i] < parameters->tcp_distoratio[i - 1] &&
                    !(i == (OPJ_UINT32)parameters->tcp_numlayers - 1 &&
                      parameters->tcp_distoratio[i] == 0)) {
                opj_event_msg(p_manager, EVT_WARNING,
                              "tcp_distoratio[%d]=%f should be strictly greater "
                              "than tcp_distoratio[%d]=%f\n",
                              i, parameters->tcp_distoratio[i], i - 1,
                              parameters->tcp_distoratio[i - 1]);
            }
        }
    }

    /* see if max_codestream_size does limit input rate */
    if (parameters->max_cs_size <= 0) {
        if (parameters->tcp_rates[parameters->tcp_numlayers - 1] > 0) {
            OPJ_FLOAT32 temp_size;
            temp_size = (OPJ_FLOAT32)(((double)image->numcomps * image->comps[0].w *
                                       image->comps[0].h * image->comps[0].prec) /
                                      ((double)parameters->tcp_rates[parameters->tcp_numlayers - 1] * 8 *
                                       image->comps[0].dx * image->comps[0].dy));
            if (temp_size > (OPJ_FLOAT32)INT_MAX) {
                parameters->max_cs_size = INT_MAX;
            } else {
                parameters->max_cs_size = (int) floor(temp_size);
            }
        } else {
            parameters->max_cs_size = 0;
        }
    } else {
        OPJ_FLOAT32 temp_rate;
        OPJ_BOOL cap = OPJ_FALSE;

        if (OPJ_IS_IMF(parameters->rsiz) && parameters->max_cs_size > 0 &&
                parameters->tcp_numlayers == 1 && parameters->tcp_rates[0] == 0) {
            parameters->tcp_rates[0] = (OPJ_FLOAT32)(image->numcomps * image->comps[0].w *
                                       image->comps[0].h * image->comps[0].prec) /
                                       (OPJ_FLOAT32)(((OPJ_UINT32)parameters->max_cs_size) * 8 * image->comps[0].dx *
                                               image->comps[0].dy);
        }

        temp_rate = (OPJ_FLOAT32)(((double)image->numcomps * image->comps[0].w *
                                   image->comps[0].h * image->comps[0].prec) /
                                  (((double)parameters->max_cs_size) * 8 * image->comps[0].dx *
                                   image->comps[0].dy));
        for (i = 0; i < (OPJ_UINT32) parameters->tcp_numlayers; i++) {
            if (parameters->tcp_rates[i] < temp_rate) {
                parameters->tcp_rates[i] = temp_rate;
                cap = OPJ_TRUE;
            }
        }
        if (cap) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "The desired maximum codestream size has limited\n"
                          "at least one of the desired quality layers\n");
        }
    }

    if (OPJ_IS_CINEMA(parameters->rsiz) || OPJ_IS_IMF(parameters->rsiz)) {
        p_j2k->m_specific_param.m_encoder.m_TLM = OPJ_TRUE;
    }

    /* Manage profiles and applications and set RSIZ */
    /* set cinema parameters if required */
    if (OPJ_IS_CINEMA(parameters->rsiz)) {
        if ((parameters->rsiz == OPJ_PROFILE_CINEMA_S2K)
                || (parameters->rsiz == OPJ_PROFILE_CINEMA_S4K)) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "JPEG 2000 Scalable Digital Cinema profiles not yet supported\n");
            parameters->rsiz = OPJ_PROFILE_NONE;
        } else {
            opj_j2k_set_cinema_parameters(parameters, image, p_manager);
            if (!opj_j2k_is_cinema_compliant(image, parameters->rsiz, p_manager)) {
                parameters->rsiz = OPJ_PROFILE_NONE;
            }
        }
    } else if (OPJ_IS_STORAGE(parameters->rsiz)) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "JPEG 2000 Long Term Storage profile not yet supported\n");
        parameters->rsiz = OPJ_PROFILE_NONE;
    } else if (OPJ_IS_BROADCAST(parameters->rsiz)) {
        opj_event_msg(p_manager, EVT_WARNING,
                      "JPEG 2000 Broadcast profiles not yet supported\n");
        parameters->rsiz = OPJ_PROFILE_NONE;
    } else if (OPJ_IS_IMF(parameters->rsiz)) {
        opj_j2k_set_imf_parameters(parameters, image, p_manager);
        if (!opj_j2k_is_imf_compliant(parameters, image, p_manager)) {
            parameters->rsiz = OPJ_PROFILE_NONE;
        }
    } else if (OPJ_IS_PART2(parameters->rsiz)) {
        if (parameters->rsiz == ((OPJ_PROFILE_PART2) | (OPJ_EXTENSION_NONE))) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "JPEG 2000 Part-2 profile defined\n"
                          "but no Part-2 extension enabled.\n"
                          "Profile set to NONE.\n");
            parameters->rsiz = OPJ_PROFILE_NONE;
        } else if (parameters->rsiz != ((OPJ_PROFILE_PART2) | (OPJ_EXTENSION_MCT))) {
            opj_event_msg(p_manager, EVT_WARNING,
                          "Unsupported Part-2 extension enabled\n"
                          "Profile set to NONE.\n");
            parameters->rsiz = OPJ_PROFILE_NONE;
        }
    }

    /*
    copy user encoding parameters
    */
    cp->rsiz = parameters->rsiz;
    if (parameters->cp_fixed_alloc) {
        cp->m_specific_param.m_enc.m_quality_layer_alloc_strategy = FIXED_LAYER;
    } else if (parameters->cp_fixed_quality) {
        cp->m_specific_param.m_enc.m_quality_layer_alloc_strategy =
            FIXED_DISTORTION_RATIO;
    } else {
        cp->m_specific_param.m_enc.m_quality_layer_alloc_strategy =
            RATE_DISTORTION_RATIO;
    }

    if (parameters->cp_fixed_alloc) {
        size_t array_size = (size_t)parameters->tcp_numlayers *
                            (size_t)parameters->numresolution * 3 * sizeof(OPJ_INT32);
        cp->m_specific_param.m_enc.m_matrice = (OPJ_INT32 *) malloc(array_size);
        if (!cp->m_specific_param.m_enc.m_matrice) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Not enough memory to allocate copy of user encoding parameters matrix \n");
            return OPJ_FALSE;
        }
        memcpy(cp->m_specific_param.m_enc.m_matrice, parameters->cp_matrice,
               array_size);
    }

    /* tiles */
    cp->tdx = (OPJ_UINT32)parameters->cp_tdx;
    cp->tdy = (OPJ_UINT32)parameters->cp_tdy;

    /* tile offset */
    cp->tx0 = (OPJ_UINT32)parameters->cp_tx0;
    cp->ty0 = (OPJ_UINT32)parameters->cp_ty0;

    /* comment string */
    if (parameters->cp_comment) {
        cp->comment = (char*)malloc(strlen(parameters->cp_comment) + 1U);
        if (!cp->comment) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Not enough memory to allocate copy of comment string\n");
            return OPJ_FALSE;
        }
        strcpy(cp->comment, parameters->cp_comment);
    } else {
        /* Create default comment for codestream */
        const char comment[] = "Created by OpenJPEG version ";
        const size_t clen = strlen(comment);
        const char *version = opj_version();

        /* UniPG>> */
#ifdef USE_JPWL
        const size_t cp_comment_buf_size = clen + strlen(version) + 11;
        cp->comment = (char*)malloc(cp_comment_buf_size);
        if (!cp->comment) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Not enough memory to allocate comment string\n");
            return OPJ_FALSE;
        }
        snprintf(cp->comment, cp_comment_buf_size, "%s%s with JPWL",
                 comment, version);
#else
        const size_t cp_comment_buf_size = clen + strlen(version) + 1;
        cp->comment = (char*)malloc(cp_comment_buf_size);
        if (!cp->comment) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Not enough memory to allocate comment string\n");
            return OPJ_FALSE;
        }
        snprintf(cp->comment, cp_comment_buf_size, "%s%s", comment, version);
#endif
        /* <<UniPG */
    }

    /*
    calculate other encoding parameters
    */

    if (parameters->tile_size_on) {
        if (cp->tdx == 0) {
            opj_event_msg(p_manager, EVT_ERROR, "Invalid tile width\n");
            return OPJ_FALSE;
        }
        if (cp->tdy == 0) {
            opj_event_msg(p_manager, EVT_ERROR, "Invalid tile height\n");
            return OPJ_FALSE;
        }
        cp->tw = opj_uint_ceildiv(image->x1 - cp->tx0, cp->tdx);
        cp->th = opj_uint_ceildiv(image->y1 - cp->ty0, cp->tdy);
        /* Check that the number of tiles is valid */
        if (cp->tw > 65535 / cp->th) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Invalid number of tiles : %u x %u (maximum fixed by jpeg2000 norm is 65535 tiles)\n",
                          cp->tw, cp->th);
            return OPJ_FALSE;
        }
    } else {
        cp->tdx = image->x1 - cp->tx0;
        cp->tdy = image->y1 - cp->ty0;
    }

    if (parameters->tp_on) {
        cp->m_specific_param.m_enc.m_tp_flag = (OPJ_BYTE)parameters->tp_flag;
        cp->m_specific_param.m_enc.m_tp_on = 1;
    }

#ifdef USE_JPWL
    /*
    calculate JPWL encoding parameters
    */

    if (parameters->jpwl_epc_on) {
        OPJ_INT32 i;

        /* set JPWL on */
        cp->epc_on = OPJ_TRUE;
        cp->info_on = OPJ_FALSE; /* no informative technique */

        /* set EPB on */
        if ((parameters->jpwl_hprot_MH > 0) || (parameters->jpwl_hprot_TPH[0] > 0)) {
            cp->epb_on = OPJ_TRUE;

            cp->hprot_MH = parameters->jpwl_hprot_MH;
            for (i = 0; i < JPWL_MAX_NO_TILESPECS; i++) {
                cp->hprot_TPH_tileno[i] = parameters->jpwl_hprot_TPH_tileno[i];
                cp->hprot_TPH[i] = parameters->jpwl_hprot_TPH[i];
            }
            /* if tile specs are not specified, copy MH specs */
            if (cp->hprot_TPH[0] == -1) {
                cp->hprot_TPH_tileno[0] = 0;
                cp->hprot_TPH[0] = parameters->jpwl_hprot_MH;
            }
            for (i = 0; i < JPWL_MAX_NO_PACKSPECS; i++) {
                cp->pprot_tileno[i] = parameters->jpwl_pprot_tileno[i];
                cp->pprot_packno[i] = parameters->jpwl_pprot_packno[i];
                cp->pprot[i] = parameters->jpwl_pprot[i];
            }
        }

        /* set ESD writing */
        if ((parameters->jpwl_sens_size == 1) || (parameters->jpwl_sens_size == 2)) {
            cp->esd_on = OPJ_TRUE;

            cp->sens_size = parameters->jpwl_sens_size;
            cp->sens_addr = parameters->jpwl_sens_addr;
            cp->sens_range = parameters->jpwl_sens_range;

            cp->sens_MH = parameters->jpwl_sens_MH;
            for (i = 0; i < JPWL_MAX_NO_TILESPECS; i++) {
                cp->sens_TPH_tileno[i] = parameters->jpwl_sens_TPH_tileno[i];
                cp->sens_TPH[i] = parameters->jpwl_sens_TPH[i];
            }
        }

        /* always set RED writing to false: we are at the encoder */
        cp->red_on = OPJ_FALSE;

    } else {
        cp->epc_on = OPJ_FALSE;
    }
#endif /* USE_JPWL */

    /* initialize the multiple tiles */
    /* ---------------------------- */
    cp->tcps = (opj_tcp_t*) calloc(cp->tw * cp->th, sizeof(opj_tcp_t));
    if (!cp->tcps) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Not enough memory to allocate tile coding parameters\n");
        return OPJ_FALSE;
    }

    for (tileno = 0; tileno < cp->tw * cp->th; tileno++) {
        opj_tcp_t *tcp = &cp->tcps[tileno];
        const OPJ_BOOL fixed_distoratio =
            cp->m_specific_param.m_enc.m_quality_layer_alloc_strategy ==
            FIXED_DISTORTION_RATIO;
        tcp->numlayers = (OPJ_UINT32)parameters->tcp_numlayers;

        for (j = 0; j < tcp->numlayers; j++) {
            if (OPJ_IS_CINEMA(cp->rsiz) || OPJ_IS_IMF(cp->rsiz)) {
                if (fixed_distoratio) {
                    tcp->distoratio[j] = parameters->tcp_distoratio[j];
                }
                tcp->rates[j] = parameters->tcp_rates[j];
            } else {
                if (fixed_distoratio) {
                    tcp->distoratio[j] = parameters->tcp_distoratio[j];
                } else {
                    tcp->rates[j] = parameters->tcp_rates[j];
                }
            }
            if (!fixed_distoratio &&
                    tcp->rates[j] <= 1.0) {
                tcp->rates[j] = 0.0;    /* force lossless */
            }
        }

        tcp->csty = (OPJ_UINT32)parameters->csty;
        tcp->prg = parameters->prog_order;
        tcp->mct = (OPJ_UINT32)parameters->tcp_mct;

        numpocs_tile = 0;
        tcp->POC = 0;

        if (parameters->numpocs) {
            /* initialisation of POC */
            for (i = 0; i < parameters->numpocs; i++) {
                if (tileno + 1 == parameters->POC[i].tile)  {
                    opj_poc_t *tcp_poc = &tcp->pocs[numpocs_tile];

                    if (parameters->POC[numpocs_tile].compno0 >= image->numcomps) {
                        opj_event_msg(p_manager, EVT_ERROR,
                                      "Invalid compno0 for POC %d\n", i);
                        return OPJ_FALSE;
                    }

                    tcp_poc->resno0         = parameters->POC[numpocs_tile].resno0;
                    tcp_poc->compno0        = parameters->POC[numpocs_tile].compno0;
                    tcp_poc->layno1         = parameters->POC[numpocs_tile].layno1;
                    tcp_poc->resno1         = parameters->POC[numpocs_tile].resno1;
                    tcp_poc->compno1        = opj_uint_min(parameters->POC[numpocs_tile].compno1,
                                                           image->numcomps);
                    tcp_poc->prg1           = parameters->POC[numpocs_tile].prg1;
                    tcp_poc->tile           = parameters->POC[numpocs_tile].tile;

                    numpocs_tile++;
                }
            }

            if (numpocs_tile) {

                /* TODO MSD use the return value*/
                opj_j2k_check_poc_val(parameters->POC, tileno, parameters->numpocs,
                                      (OPJ_UINT32)parameters->numresolution, image->numcomps,
                                      (OPJ_UINT32)parameters->tcp_numlayers, p_manager);

                tcp->POC = 1;
                tcp->numpocs = numpocs_tile - 1 ;
            }
        } else {
            tcp->numpocs = 0;
        }

        tcp->tccps = (opj_tccp_t*) calloc(image->numcomps, sizeof(opj_tccp_t));
        if (!tcp->tccps) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Not enough memory to allocate tile component coding parameters\n");
            return OPJ_FALSE;
        }
        if (parameters->mct_data) {

            OPJ_UINT32 lMctSize = image->numcomps * image->numcomps * (OPJ_UINT32)sizeof(
                                      OPJ_FLOAT32);
            OPJ_FLOAT32 * lTmpBuf = (OPJ_FLOAT32*)malloc(lMctSize);
            OPJ_INT32 * l_dc_shift = (OPJ_INT32 *)((OPJ_BYTE *) parameters->mct_data +
                                                   lMctSize);

            if (!lTmpBuf) {
                opj_event_msg(p_manager, EVT_ERROR,
                              "Not enough memory to allocate temp buffer\n");
                return OPJ_FALSE;
            }

            tcp->mct = 2;
            tcp->m_mct_coding_matrix = (OPJ_FLOAT32*)malloc(lMctSize);
            if (! tcp->m_mct_coding_matrix) {
                free(lTmpBuf);
                lTmpBuf = NULL;
                opj_event_msg(p_manager, EVT_ERROR,
                              "Not enough memory to allocate encoder MCT coding matrix \n");
                return OPJ_FALSE;
            }
            memcpy(tcp->m_mct_coding_matrix, parameters->mct_data, lMctSize);
            memcpy(lTmpBuf, parameters->mct_data, lMctSize);

            tcp->m_mct_decoding_matrix = (OPJ_FLOAT32*)malloc(lMctSize);
            if (! tcp->m_mct_decoding_matrix) {
                free(lTmpBuf);
                lTmpBuf = NULL;
                opj_event_msg(p_manager, EVT_ERROR,
                              "Not enough memory to allocate encoder MCT decoding matrix \n");
                return OPJ_FALSE;
            }
            if (opj_matrix_inversion_f(lTmpBuf, (tcp->m_mct_decoding_matrix),
                                       image->numcomps) == OPJ_FALSE) {
                free(lTmpBuf);
                lTmpBuf = NULL;
                opj_event_msg(p_manager, EVT_ERROR,
                              "Failed to inverse encoder MCT decoding matrix \n");
                return OPJ_FALSE;
            }

            tcp->mct_norms = (OPJ_FLOAT64*)
                             malloc(image->numcomps * sizeof(OPJ_FLOAT64));
            if (! tcp->mct_norms) {
                free(lTmpBuf);
                lTmpBuf = NULL;
                opj_event_msg(p_manager, EVT_ERROR,
                              "Not enough memory to allocate encoder MCT norms \n");
                return OPJ_FALSE;
            }
            opj_calculate_norms(tcp->mct_norms, image->numcomps,
                                tcp->m_mct_decoding_matrix);
            free(lTmpBuf);

            for (i = 0; i < image->numcomps; i++) {
                opj_tccp_t *tccp = &tcp->tccps[i];
                tccp->m_dc_level_shift = l_dc_shift[i];
            }

            if (opj_j2k_setup_mct_encoding(tcp, image) == OPJ_FALSE) {
                /* free will be handled by opj_j2k_destroy */
                opj_event_msg(p_manager, EVT_ERROR, "Failed to setup j2k mct encoding\n");
                return OPJ_FALSE;
            }
        } else {
            if (tcp->mct == 1 && image->numcomps >= 3) { /* RGB->YCC MCT is enabled */
                if ((image->comps[0].dx != image->comps[1].dx) ||
                        (image->comps[0].dx != image->comps[2].dx) ||
                        (image->comps[0].dy != image->comps[1].dy) ||
                        (image->comps[0].dy != image->comps[2].dy)) {
                    opj_event_msg(p_manager, EVT_WARNING,
                                  "Cannot perform MCT on components with different sizes. Disabling MCT.\n");
                    tcp->mct = 0;
                }
            }
            for (i = 0; i < image->numcomps; i++) {
                opj_tccp_t *tccp = &tcp->tccps[i];
                opj_image_comp_t * l_comp = &(image->comps[i]);

                if (! l_comp->sgnd) {
                    tccp->m_dc_level_shift = 1 << (l_comp->prec - 1);
                }
            }
        }

        for (i = 0; i < image->numcomps; i++) {
            opj_tccp_t *tccp = &tcp->tccps[i];

            tccp->csty = parameters->csty &
                         0x01;   /* 0 => one precinct || 1 => custom precinct  */
            tccp->numresolutions = (OPJ_UINT32)parameters->numresolution;
            tccp->cblkw = (OPJ_UINT32)opj_int_floorlog2(parameters->cblockw_init);
            tccp->cblkh = (OPJ_UINT32)opj_int_floorlog2(parameters->cblockh_init);
            tccp->cblksty = (OPJ_UINT32)parameters->mode;
            tccp->qmfbid = parameters->irreversible ? 0 : 1;
            tccp->qntsty = parameters->irreversible ? J2K_CCP_QNTSTY_SEQNT :
                           J2K_CCP_QNTSTY_NOQNT;

            if (OPJ_IS_CINEMA(parameters->rsiz) &&
                    parameters->rsiz == OPJ_PROFILE_CINEMA_2K) {
                /* From https://github.com/uclouvain/openjpeg/issues/1340 */
                tccp->numgbits = 1;
            } else {
                tccp->numgbits = 2;
            }

            if ((OPJ_INT32)i == parameters->roi_compno) {
                tccp->roishift = parameters->roi_shift;
            } else {
                tccp->roishift = 0;
            }

            if (parameters->csty & J2K_CCP_CSTY_PRT) {
                OPJ_INT32 p = 0, it_res;
                assert(tccp->numresolutions > 0);
                for (it_res = (OPJ_INT32)tccp->numresolutions - 1; it_res >= 0; it_res--) {
                    if (p < parameters->res_spec) {

                        if (parameters->prcw_init[p] < 1) {
                            tccp->prcw[it_res] = 1;
                        } else {
                            tccp->prcw[it_res] = (OPJ_UINT32)opj_int_floorlog2(parameters->prcw_init[p]);
                        }

                        if (parameters->prch_init[p] < 1) {
                            tccp->prch[it_res] = 1;
                        } else {
                            tccp->prch[it_res] = (OPJ_UINT32)opj_int_floorlog2(parameters->prch_init[p]);
                        }

                    } else {
                        OPJ_INT32 res_spec = parameters->res_spec;
                        OPJ_INT32 size_prcw = 0;
                        OPJ_INT32 size_prch = 0;

                        assert(res_spec > 0); /* issue 189 */
                        size_prcw = parameters->prcw_init[res_spec - 1] >> (p - (res_spec - 1));
                        size_prch = parameters->prch_init[res_spec - 1] >> (p - (res_spec - 1));


                        if (size_prcw < 1) {
                            tccp->prcw[it_res] = 1;
                        } else {
                            tccp->prcw[it_res] = (OPJ_UINT32)opj_int_floorlog2(size_prcw);
                        }

                        if (size_prch < 1) {
                            tccp->prch[it_res] = 1;
                        } else {
                            tccp->prch[it_res] = (OPJ_UINT32)opj_int_floorlog2(size_prch);
                        }
                    }
                    p++;
                    /*printf("\nsize precinct for level %d : %d,%d\n", it_res,tccp->prcw[it_res], tccp->prch[it_res]); */
                }       /*end for*/
            } else {
                for (j = 0; j < tccp->numresolutions; j++) {
                    tccp->prcw[j] = 15;
                    tccp->prch[j] = 15;
                }
            }

            opj_dwt_calc_explicit_stepsizes(tccp, image->comps[i].prec);
        }
    }

    if (parameters->mct_data) {
        free(parameters->mct_data);
        parameters->mct_data = 00;
    }
    return OPJ_TRUE;
}


#pragma mark (2)
OPJ_BOOL opj_j2k_start_compress(
 opj_j2k_t *p_j2k,
 opj_stream_private_t *p_stream,
 opj_image_t * p_image,
 opj_event_mgr_t * p_manager
)
{
   //copy opj_2k_t to newly created in memory opj_stream_private_t
   p_j2k->m_private_image = (opj_image_t*)calloc(1, sizeof(opj_image_t));
   //better create private directly !!!!
   
#pragma mark image.opj_copy_image_header [start]
   //[source]p_image,
   //[dest]p_j2k->m_private_image)
   
   p_j2k->m_private_image->x0 = p_image->x0;
   p_j2k->m_private_image->y0 = p_image->y0;
   p_j2k->m_private_image->x1 = p_image->x1;
   p_j2k->m_private_image->y1 = p_image->y1;

//free image_dest->comps[x]->data
   OPJ_UINT32 compno;//iterator
   if (p_j2k->m_private_image->comps) {
       for (compno = 0; compno < p_j2k->m_private_image->numcomps; compno++) {
           opj_image_comp_t *image_comp = &(p_j2k->m_private_image->comps[compno]);
           if (image_comp->data) opj_aligned_free(image_comp->data);
       }
       free(p_j2k->m_private_image->comps);
       p_j2k->m_private_image->comps = NULL;
   }
//copy comps
   p_j2k->m_private_image->numcomps = p_image->numcomps;
   p_j2k->m_private_image->comps = (opj_image_comp_t*) malloc(p_j2k->m_private_image->numcomps * sizeof(opj_image_comp_t));
   for (compno = 0; compno < p_j2k->m_private_image->numcomps; compno++) {
       memcpy(&(p_j2k->m_private_image->comps[compno]),&(p_image->comps[compno]),sizeof(opj_image_comp_t));
       p_j2k->m_private_image->comps[compno].data = NULL;
   }
   if (p_image->comps) {
       OPJ_UINT32 it_comp;
       for (it_comp = 0 ; it_comp < p_image->numcomps; it_comp++) {
           if (p_image->comps[it_comp].data) {
               p_j2k->m_private_image->comps[it_comp].data = p_image->comps[it_comp].data;
               p_image->comps[it_comp].data = NULL;
           }
       }
   }
// copy colorspace
   p_j2k->m_private_image->color_space = p_image->color_space;
   p_j2k->m_private_image->icc_profile_len = p_image->icc_profile_len;
   if (p_j2k->m_private_image->icc_profile_len) {
       p_j2k->m_private_image->icc_profile_buf = (OPJ_BYTE*)malloc(p_j2k->m_private_image->icc_profile_len);
       memcpy(p_j2k->m_private_image->icc_profile_buf,p_image->icc_profile_buf,p_image->icc_profile_len);
   } else {
       p_j2k->m_private_image->icc_profile_buf = NULL;
   }
#pragma mark image.opj_copy_image_header [end]
   
/*
#pragma mark j2k.opj_j2k_setup_encoding_validation [start]
if (! opj_procedure_list_add_procedure(p_j2k->m_validation_list,(opj_procedure)   opj_j2k_build_encoder,        p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_validation_list,(opj_procedure)   opj_j2k_encoding_validation,  p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_validation_list,(opj_procedure)   opj_j2k_mct_validation,       p_manager)) return OPJ_FALSE;
#pragma mark j2k.opj_j2k_setup_encoding_validation [end] + exec
if (! opj_j2k_exec(p_j2k, p_j2k->m_validation_list, p_stream, p_manager)) return OPJ_FALSE;
*/
   
   //if (!      opj_j2k_build_encoder(p_j2k, p_stream, p_manager)) return OPJ_FALSE; placeholder for futur development

#pragma mark opj_j2k_encoding_validation [start]
   if (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_NONE)return OPJ_FALSE;;//state is at 0
   if (p_j2k->m_procedure_list != 00)return OPJ_FALSE;;//p_j2k codec present
   if (p_j2k->m_validation_list != 00)return OPJ_FALSE;;//validation list present
   if (p_j2k->m_cp.tcps->tccps->numresolutions > 32) return OPJ_FALSE;//ISO 15444-1:2004 states between 1 & 33 (0 -> 32)
   //Number of resolutions is too high in comparison to the size of tiles
   if ((p_j2k->m_cp.tdx) < (OPJ_UINT32)(1 << (p_j2k->m_cp.tcps->tccps->numresolutions - 1U))) return OPJ_FALSE;
   if ((p_j2k->m_cp.tdy) < (OPJ_UINT32)(1 << (p_j2k->m_cp.tcps->tccps->numresolutions - 1U))) OPJ_FALSE;
#pragma mark opj_j2k_encoding_validation [end]

#pragma mark opj_j2k_mct_validation [start]
   if ((p_j2k->m_cp.rsiz & 0x8200) == 0x8200) {
       OPJ_UINT32 l_nb_tiles = p_j2k->m_cp.th * p_j2k->m_cp.tw;
       opj_tcp_t * l_tcp = p_j2k->m_cp.tcps;

       for (OPJ_UINT32 i = 0; i < l_nb_tiles; ++i) {
           if (l_tcp->mct == 2) {
               opj_tccp_t * l_tccp = l_tcp->tccps;
               if (l_tcp->m_mct_coding_matrix != 00) return OPJ_FALSE;

               for (OPJ_UINT32 j = 0; j < p_j2k->m_private_image->numcomps; ++j) {
                  if (!(l_tccp->qmfbid & 1)) return OPJ_FALSE;
                  ++l_tccp;
               }
           }
           ++l_tcp;
       }
   }
#pragma mark opj_j2k_mct_validation [end]

/*
#pragma mark opj_j2k_setup_header_writing [start]
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_init_info,            p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_soc,            p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_siz,            p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_cod,            p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_qcd,            p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_all_coc,        p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_all_qcc,        p_manager)) return OPJ_FALSE;

   if (p_j2k->m_specific_param.m_encoder.m_TLM) {
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_tlm,            p_manager)) return OPJ_FALSE;
       if (p_j2k->m_cp.rsiz == OPJ_PROFILE_CINEMA_4K) {
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_poc,            p_manager)) return OPJ_FALSE;
       }
   }

if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_regions,        p_manager)) return OPJ_FALSE;

   if (p_j2k->m_cp.comment != 00)  {
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_com,            p_manager)) return OPJ_FALSE;
   }

   //DEVELOPER CORNER, insert your custom procedures
   if ((p_j2k->m_cp.rsiz & (OPJ_PROFILE_PART2 | OPJ_EXTENSION_MCT)) == (OPJ_PROFILE_PART2 | OPJ_EXTENSION_MCT)) {
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_write_mct_data_group, p_manager)) return OPJ_FALSE;
   }

   if (p_j2k->cstr_index) {
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_get_end_header,       p_manager)) return OPJ_FALSE;
   }

   if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_create_tcd,           p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)    opj_j2k_update_rates,         p_manager)) return OPJ_FALSE;
   return OPJ_TRUE;
#pragma mark opj_j2k_setup_header_writing [end] + exec
if (! opj_j2k_exec(p_j2k, p_j2k->m_procedure_list, p_stream, p_manager)) return OPJ_FALSE;
*/
   
   opj_j2k_calculate_tp(p_j2k, &(p_j2k->m_cp),&p_j2k->m_specific_param.m_encoder.m_total_tile_parts, p_j2k->m_private_image,p_manager);
//   if opj_j2k_calculate_tp(p_j2k, &(p_j2k->m_cp),&p_j2k->m_specific_param.m_encoder.m_total_tile_parts, p_j2k->m_private_image,p_manager) return OPJ_FALSE;//opj_j2k_init_info
   
   if (!opj_j2k_write_soc(    p_j2k, p_stream, p_manager)) return OPJ_FALSE;
   if (!opj_j2k_write_siz(    p_j2k, p_stream, p_manager)) return OPJ_FALSE;
   if (!opj_j2k_write_cod(    p_j2k, p_stream, p_manager)) return OPJ_FALSE;
   if (!opj_j2k_write_qcd(    p_j2k, p_stream, p_manager)) return OPJ_FALSE;
   if (!opj_j2k_write_all_coc(p_j2k, p_stream, p_manager)) return OPJ_FALSE;
   if (!opj_j2k_write_all_qcc(p_j2k, p_stream, p_manager)) return OPJ_FALSE;
   
   if (p_j2k->m_specific_param.m_encoder.m_TLM) {
       if (!opj_j2k_write_tlm(    p_j2k, p_stream, p_manager)) return OPJ_FALSE;
       if (p_j2k->m_cp.rsiz == OPJ_PROFILE_CINEMA_4K) {
           if (!opj_j2k_write_poc(    p_j2k, p_stream, p_manager)) return OPJ_FALSE;
      }
   }
   
   if (!opj_j2k_write_regions(p_j2k, p_stream, p_manager)) return OPJ_FALSE;
   
   if (p_j2k->m_cp.comment != 00)  {
       if (!opj_j2k_write_com(p_j2k, p_stream, p_manager)) return OPJ_FALSE;
   }

   if ((p_j2k->m_cp.rsiz & (OPJ_PROFILE_PART2 | OPJ_EXTENSION_MCT)) == (OPJ_PROFILE_PART2 | OPJ_EXTENSION_MCT)) {
       if (!opj_j2k_write_mct_data_group(p_j2k, p_stream, p_manager)) return OPJ_FALSE;
   }

   if (p_j2k->cstr_index) p_j2k->cstr_index->main_head_end = opj_stream_tell(p_stream);//opj_j2k_get_end_header
   
   if (!opj_j2k_create_tcd(  p_j2k, p_stream, p_manager)) return OPJ_FALSE;
   if (!opj_j2k_update_rates(p_j2k, p_stream, p_manager)) return OPJ_FALSE;

   
   
    return OPJ_TRUE;
}

#pragma mark (3)
OPJ_BOOL opj_j2k_encode(opj_j2k_t * p_j2k,opj_stream_private_t *p_stream)
{
    OPJ_UINT32 i, j;
    OPJ_UINT32 l_nb_tiles;
    OPJ_SIZE_T l_max_tile_size = 0, l_current_tile_size;
    OPJ_BYTE * l_current_data = 00;
    OPJ_BOOL l_reuse_data = OPJ_FALSE;
    opj_tcd_t* p_tcd = 00;

    p_tcd = p_j2k->m_tcd;

    l_nb_tiles = p_j2k->m_cp.th * p_j2k->m_cp.tw;
    if (l_nb_tiles == 1) {
        l_reuse_data = OPJ_TRUE;
#ifdef __SSE__
        for (j = 0; j < p_j2k->m_tcd->image->numcomps; ++j) {
            opj_image_comp_t * l_img_comp = p_tcd->image->comps + j;
            if (((size_t)l_img_comp->data & 0xFU) !=
                    0U) { /* tile data shall be aligned on 16 bytes */
                l_reuse_data = OPJ_FALSE;
            }
        }
#endif
    }
    for (i = 0; i < l_nb_tiles; ++i) {
        if (! opj_j2k_pre_write_tile(p_j2k, i, p_stream)) {
            if (l_current_data) {
                free(l_current_data);
            }
            return OPJ_FALSE;
        }

/* if we only have one tile, then simply set tile component data equal to image component data */
/* otherwise, allocate the data */
        for (j = 0; j < p_j2k->m_tcd->image->numcomps; ++j) {
            opj_tcd_tilecomp_t* l_tilec = p_tcd->tcd_image->tiles->comps + j;
            if (l_reuse_data) {
                opj_image_comp_t * l_img_comp = p_tcd->image->comps + j;
                l_tilec->data  =  l_img_comp->data;
                l_tilec->ownsData = OPJ_FALSE;
            } else {
                if (! opj_alloc_tile_component_data(l_tilec)) {
                    E("%s","Error allocating tile component data.");
                    if (l_current_data) free(l_current_data);
                    return OPJ_FALSE;
                }
            }
        }
        l_current_tile_size = opj_tcd_get_encoder_input_buffer_size(p_j2k->m_tcd);
        if (!l_reuse_data) {
            if (l_current_tile_size > l_max_tile_size) {
                OPJ_BYTE *l_new_current_data = (OPJ_BYTE *) realloc(l_current_data,l_current_tile_size);
                if (! l_new_current_data) {
                    if (l_current_data) free(l_current_data);
                    E("%s","Not enough memory to encode all tiles\n");
                    return OPJ_FALSE;
                }
                l_current_data = l_new_current_data;
                l_max_tile_size = l_current_tile_size;
            }

            /* copy image data (32 bit) to l_current_data as contiguous, all-component, zero offset buffer */
            /* 32 bit components @ 8 bit precision get converted to 8 bit */
            /* 32 bit components @ 16 bit precision get converted to 16 bit */
            opj_j2k_get_tile_data(p_j2k->m_tcd, l_current_data);

            /* now copy this data into the tile component */
if (! opj_tcd_copy_tile_data(p_j2k->m_tcd, l_current_data,l_current_tile_size)) {
                E("%s","Size mismatch between tile data and sent data.");
                free(l_current_data);
                return OPJ_FALSE;
            }
        }

if (! opj_j2k_post_write_tile(p_j2k, p_stream)) {
            if (l_current_data) free(l_current_data);
            return OPJ_FALSE;
        }
    }

    if (l_current_data) {
        free(l_current_data);
    }
    return OPJ_TRUE;
}

#pragma mark (4)
OPJ_BOOL opj_j2k_end_compress(opj_j2k_t *p_j2k,opj_stream_private_t *p_stream,opj_event_mgr_t * p_manager)
{
   /**
    * Sets up the validation ,i.e. adds the procedures to launch to make sure the codec parameters
    * are valid. Developers wanting to extend the library can add their own validation procedures.
    */
//funcion_list.c defines a 10 procedures list (expandable)
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)opj_j2k_write_eoc, p_manager))return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)opj_j2k_write_updated_tlm, p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)opj_j2k_write_epc, p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)opj_j2k_end_encoding, p_manager)) return OPJ_FALSE;
if (! opj_procedure_list_add_procedure(p_j2k->m_procedure_list,(opj_procedure)opj_j2k_destroy_header_memory, p_manager)) return OPJ_FALSE;

if (! opj_j2k_exec(p_j2k, p_j2k->m_procedure_list, p_stream, p_manager)) return OPJ_FALSE;
    
return OPJ_TRUE;
}
