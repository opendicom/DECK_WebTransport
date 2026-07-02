#include "opj_includes.h"


#pragma mark message handlers

OPJ_BOOL OPJ_CALLCONV opj_set_info_handler(opj_codec_t * p_codec,
        opj_msg_callback p_callback,
        void * p_user_data)
{
    opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
    if (! l_codec) {
        return OPJ_FALSE;
    }

    l_codec->m_event_mgr.info_handler = p_callback;
    l_codec->m_event_mgr.m_info_data = p_user_data;

    return OPJ_TRUE;
}

OPJ_BOOL OPJ_CALLCONV opj_set_warning_handler(opj_codec_t * p_codec,
        opj_msg_callback p_callback,
        void * p_user_data)
{
    opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
    if (! l_codec) {
        return OPJ_FALSE;
    }

    l_codec->m_event_mgr.warning_handler = p_callback;
    l_codec->m_event_mgr.m_warning_data = p_user_data;

    return OPJ_TRUE;
}

OPJ_BOOL OPJ_CALLCONV opj_set_error_handler(opj_codec_t * p_codec,
        opj_msg_callback p_callback,
        void * p_user_data)
{
    opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
    if (! l_codec) {
        return OPJ_FALSE;
    }

    l_codec->m_event_mgr.error_handler = p_callback;
    l_codec->m_event_mgr.m_error_data = p_user_data;

    return OPJ_TRUE;
}

#pragma mark - from file

static OPJ_SIZE_T opj_read_from_file(void * p_buffer, OPJ_SIZE_T p_nb_bytes,
                                     void * p_user_data)
{
    FILE* p_file = (FILE*)p_user_data;
    OPJ_SIZE_T l_nb_read = fread(p_buffer, 1, p_nb_bytes, (FILE*)p_file);
    return l_nb_read ? l_nb_read : (OPJ_SIZE_T) - 1;
}

static OPJ_UINT64 opj_get_data_length_from_file(void * p_user_data)
{
    FILE* p_file = (FILE*)p_user_data;
    OPJ_OFF_T file_length = 0;

    OPJ_FSEEK(p_file, 0, SEEK_END);
    file_length = (OPJ_OFF_T)OPJ_FTELL(p_file);
    OPJ_FSEEK(p_file, 0, SEEK_SET);

    return (OPJ_UINT64)file_length;
}

static OPJ_SIZE_T opj_write_from_file(void * p_buffer, OPJ_SIZE_T p_nb_bytes,
                                      void * p_user_data)
{
    FILE* p_file = (FILE*)p_user_data;
    return fwrite(p_buffer, 1, p_nb_bytes, p_file);
}

static OPJ_OFF_T opj_skip_from_file(OPJ_OFF_T p_nb_bytes, void * p_user_data)
{
    FILE* p_file = (FILE*)p_user_data;
    if (OPJ_FSEEK(p_file, p_nb_bytes, SEEK_CUR)) {
        return -1;
    }

    return p_nb_bytes;
}

static OPJ_BOOL opj_seek_from_file(OPJ_OFF_T p_nb_bytes, void * p_user_data)
{
    FILE* p_file = (FILE*)p_user_data;
    if (OPJ_FSEEK(p_file, p_nb_bytes, SEEK_SET)) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static void opj_close_from_file(void* p_user_data)
{
    FILE* p_file = (FILE*)p_user_data;
    fclose(p_file);
}



#pragma mark COMPRESSION FUNCTIONS*/

//case OPJ_CODEC_J2K only
opj_codec_t* OPJ_CALLCONV opj_create_compress(OPJ_CODEC_FORMAT p_format)
{
//l_codec
    opj_codec_private_t *l_codec = 00;
    l_codec = (opj_codec_private_t*)calloc(1, sizeof(opj_codec_private_t));
   
//l_codec->m_codec_data.m_compression
    l_codec->m_codec_data.m_compression.opj_encode =
      (OPJ_BOOL(*)(void *, struct opj_stream_private *, struct opj_event_mgr *))                                   opj_j2k_encode;
   
    l_codec->m_codec_data.m_compression.opj_end_compress =
      (OPJ_BOOL(*)(void *, struct opj_stream_private *, struct opj_event_mgr *))                                    opj_j2k_end_compress;
   
    l_codec->m_codec_data.m_compression.opj_start_compress =
      (OPJ_BOOL(*)(void *, struct opj_stream_private *, struct opj_image *, struct opj_event_mgr *))                opj_j2k_start_compress;
   
    l_codec->m_codec_data.m_compression.opj_write_tile =
      (OPJ_BOOL(*)(void *, OPJ_UINT32, OPJ_BYTE*, OPJ_UINT32, struct opj_stream_private *, struct opj_event_mgr *)) opj_j2k_write_tile;

    l_codec->m_codec_data.m_compression.opj_destroy = (void (*)( void *))                                           opj_j2k_destroy;
   
    l_codec->m_codec_data.m_compression.opj_setup_encoder =
      (OPJ_BOOL(*)(void *, opj_cparameters_t *, struct opj_image *, struct opj_event_mgr *))                        opj_j2k_setup_encoder;
   
//threads
    l_codec->opj_set_threads = (OPJ_BOOL(*)(void * p_codec, OPJ_UINT32 num_threads)) opj_j2k_set_threads;
//create codec
   l_codec->m_codec = opj_j2k_create_compress();
    if (! l_codec->m_codec) { free(l_codec); return 00;}

//event manager
    opj_set_default_event_handler(&(l_codec->m_event_mgr));
   
    return (opj_codec_t*) l_codec;
}

void OPJ_CALLCONV opj_set_default_encoder_parameters(opj_cparameters_t
        *parameters)
{
    if (parameters) {
        memset(parameters, 0, sizeof(opj_cparameters_t));
        /* default coding parameters */
        parameters->rsiz = OPJ_PROFILE_NONE;
        parameters->numresolution = OPJ_COMP_PARAM_DEFAULT_NUMRESOLUTION;
        parameters->cblockw_init = OPJ_COMP_PARAM_DEFAULT_CBLOCKW;
        parameters->cblockh_init = OPJ_COMP_PARAM_DEFAULT_CBLOCKH;
        parameters->prog_order = OPJ_COMP_PARAM_DEFAULT_PROG_ORDER;
        parameters->roi_compno = -1;        /* no ROI */
        parameters->subsampling_dx = 1;
        parameters->subsampling_dy = 1;
        parameters->tp_on = 0;
        parameters->decod_format = -1;
        parameters->cod_format = -1;
        parameters->tcp_rates[0] = 0;
        parameters->tcp_numlayers = 0;
        parameters->cp_disto_alloc = 0;
        parameters->cp_fixed_alloc = 0;
        parameters->cp_fixed_quality = 0;
        parameters->jpip_on = OPJ_FALSE;
    }
}

OPJ_BOOL OPJ_CALLCONV opj_setup_encoder(opj_codec_t *p_codec,
                                        opj_cparameters_t *parameters,
                                        opj_image_t *p_image)
{
    if (p_codec && parameters && p_image) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;

    return l_codec->m_codec_data.m_compression.opj_setup_encoder(l_codec->m_codec,
                    parameters,
                    p_image,
                    &(l_codec->m_event_mgr));
    }

    return OPJ_FALSE;
}


OPJ_BOOL OPJ_CALLCONV opj_start_compress(
 opj_codec_t *p_codec,
 opj_image_t *p_image,
 opj_stream_t *p_stream)
{
   opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;
   
   opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
   return l_codec->m_codec_data.m_compression.opj_start_compress(
           l_codec->m_codec,
           l_stream,
           p_image,
           &(l_codec->m_event_mgr)
          );
}

OPJ_BOOL OPJ_CALLCONV opj_encode(opj_codec_t *p_info, opj_stream_t *p_stream)
{
    if (p_info && p_stream) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_info;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        return l_codec->m_codec_data.m_compression.opj_encode(l_codec->m_codec,
                    l_stream,
                    &(l_codec->m_event_mgr));
    }

    return OPJ_FALSE;

}

OPJ_BOOL OPJ_CALLCONV opj_end_compress(opj_codec_t *p_codec,
                                       opj_stream_t *p_stream)
{
    if (p_codec && p_stream) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        return l_codec->m_codec_data.m_compression.opj_end_compress(l_codec->m_codec,
                    l_stream,
                    &(l_codec->m_event_mgr));
        
    }
    return OPJ_FALSE;

}


OPJ_BOOL OPJ_CALLCONV opj_set_MCT(opj_cparameters_t *parameters,
                                  OPJ_FLOAT32 * pEncodingMatrix,
                                  OPJ_INT32 * p_dc_shift, OPJ_UINT32 pNbComp)
{
    OPJ_UINT32 l_matrix_size = pNbComp * pNbComp * (OPJ_UINT32)sizeof(OPJ_FLOAT32);
    OPJ_UINT32 l_dc_shift_size = pNbComp * (OPJ_UINT32)sizeof(OPJ_INT32);
    OPJ_UINT32 l_mct_total_size = l_matrix_size + l_dc_shift_size;

    /* add MCT capability */
    if (OPJ_IS_PART2(parameters->rsiz)) {
        parameters->rsiz |= OPJ_EXTENSION_MCT;
    } else {
        parameters->rsiz = ((OPJ_PROFILE_PART2) | (OPJ_EXTENSION_MCT));
    }
    parameters->irreversible = 1;

    /* use array based MCT */
    parameters->tcp_mct = 2;
    parameters->mct_data = malloc(l_mct_total_size);
    if (! parameters->mct_data) {
        return OPJ_FALSE;
    }

    memcpy(parameters->mct_data, pEncodingMatrix, l_matrix_size);
    memcpy(((OPJ_BYTE *) parameters->mct_data) +  l_matrix_size, p_dc_shift,
           l_dc_shift_size);

    return OPJ_TRUE;
}

OPJ_BOOL OPJ_CALLCONV opj_write_tile(opj_codec_t *p_codec,
                                     OPJ_UINT32 p_tile_index,
                                     OPJ_BYTE * p_data,
                                     OPJ_UINT32 p_data_size,
                                     opj_stream_t *p_stream)
{
    if (p_codec && p_stream && p_data) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        return l_codec->m_codec_data.m_compression.opj_write_tile(l_codec->m_codec,
                p_tile_index,
                p_data,
                p_data_size,
                l_stream,
                &(l_codec->m_event_mgr));
    }

    return OPJ_FALSE;
}

#pragma mark - codec

void OPJ_CALLCONV opj_destroy_codec(opj_codec_t *p_codec)
{
    if (p_codec) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;

        l_codec->m_codec_data.m_compression.opj_destroy(l_codec->m_codec);

        l_codec->m_codec = 00;
        free(l_codec);
    }
}

void OPJ_CALLCONV opj_dump_codec(opj_codec_t *p_codec,
                                 OPJ_INT32 info_flag,
                                 FILE* output_stream)
{
    if (p_codec) {
        opj_codec_private_t* l_codec = (opj_codec_private_t*) p_codec;

        l_codec->opj_dump_codec(l_codec->m_codec, info_flag, output_stream);
        return;
    }

    /* TODO return error */
    /* fprintf(stderr, "[ERROR] Input parameter of the dump_codec function are incorrect.\n"); */
    return;
}

opj_codestream_info_v2_t* OPJ_CALLCONV opj_get_cstr_info(opj_codec_t *p_codec)
{
    if (p_codec) {
        opj_codec_private_t* l_codec = (opj_codec_private_t*) p_codec;

        return l_codec->opj_get_codec_info(l_codec->m_codec);
    }

    return NULL;
}

void OPJ_CALLCONV opj_destroy_cstr_info(opj_codestream_info_v2_t **cstr_info)
{
    if (cstr_info) {

        if ((*cstr_info)->m_default_tile_info.tccp_info) {
            free((*cstr_info)->m_default_tile_info.tccp_info);
        }

        if ((*cstr_info)->tile_info) {
            /* FIXME not used for the moment*/
        }

        free((*cstr_info));
        (*cstr_info) = NULL;
    }
}

opj_codestream_index_t * OPJ_CALLCONV opj_get_cstr_index(opj_codec_t *p_codec)
{
    if (p_codec) {
        opj_codec_private_t* l_codec = (opj_codec_private_t*) p_codec;

        return l_codec->opj_get_codec_index(l_codec->m_codec);
    }

    return NULL;
}

void OPJ_CALLCONV opj_destroy_cstr_index(opj_codestream_index_t **p_cstr_index)
{
    if (*p_cstr_index) {
        j2k_destroy_cstr_index(*p_cstr_index);
        (*p_cstr_index) = NULL;
    }
}

#pragma mark - stream

opj_stream_t* OPJ_CALLCONV opj_stream_create_file_stream(
 const char *fname,
 OPJ_SIZE_T p_size,
 OPJ_BOOL p_is_read_stream
)
{
    opj_stream_t* l_stream = 00;
    FILE *p_file;
    const char *mode;

    if (! fname) {
        return NULL;
    }

    if (p_is_read_stream) {
        mode = "rb";
    } else {
        mode = "wb";
    }

    p_file = fopen(fname, mode);

    if (! p_file) {
        return NULL;
    }

    l_stream = opj_stream_create(p_size, p_is_read_stream);
    if (! l_stream) {
        fclose(p_file);
        return NULL;
    }

    opj_stream_set_user_data(l_stream, p_file, opj_close_from_file);
    opj_stream_set_user_data_length(l_stream,opj_get_data_length_from_file(p_file));
    opj_stream_set_read_function(l_stream, opj_read_from_file);
    opj_stream_set_write_function(l_stream,(opj_stream_write_fn) opj_write_from_file);
    opj_stream_set_skip_function(l_stream, opj_skip_from_file);
    opj_stream_set_seek_function(l_stream, opj_seek_from_file);

    return l_stream;
}

opj_stream_t* OPJ_CALLCONV opj_stream_create_default_file_stream(
 const char *fname,
 OPJ_BOOL p_is_read_stream
)
{
    return opj_stream_create_file_stream(fname, OPJ_J2K_STREAM_CHUNK_SIZE, p_is_read_stream);//0x100000 /** 1 mega by default */ ??? 100K
}

#pragma mark - image data buffer

void* OPJ_CALLCONV opj_image_data_alloc(OPJ_SIZE_T size)
{
    void* ret = opj_aligned_malloc(size);
    /* printf("opj_image_data_alloc %p\n", ret); */
    return ret;
}

void OPJ_CALLCONV opj_image_data_free(void* ptr)
{
    /* printf("opj_image_data_free %p\n", ptr); */
    opj_aligned_free(ptr);
}
