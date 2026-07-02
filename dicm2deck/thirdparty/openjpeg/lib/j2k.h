#ifndef OPJ_J2K_H
#define OPJ_J2K_H

#include "opj_includes.h"

//JPEG-2000 Codestream Writer (J2K)
//The functions in J2K.C have for goal to write the several parts of the codestream: markers and data.

#define J2K_CP_CSTY_PRT 0x01
#define J2K_CP_CSTY_SOP 0x02
#define J2K_CP_CSTY_EPH 0x04
#define J2K_CCP_CSTY_PRT 0x01
#define J2K_CCP_CBLKSTY_LAZY 0x01     /**< Selective arithmetic coding bypass */
#define J2K_CCP_CBLKSTY_RESET 0x02    /**< Reset context probabilities on coding pass boundaries */
#define J2K_CCP_CBLKSTY_TERMALL 0x04  /**< Termination on each coding pass */
#define J2K_CCP_CBLKSTY_VSC 0x08      /**< Vertically stripe causal context */
#define J2K_CCP_CBLKSTY_PTERM 0x10    /**< Predictable termination */
#define J2K_CCP_CBLKSTY_SEGSYM 0x20   /**< Segmentation symbols are used */
#define J2K_CCP_CBLKSTY_HT 0x40       /**< (high throughput) HT codeblocks */
#define J2K_CCP_CBLKSTY_HTMIXED 0x80  /**< MIXED mode HT codeblocks */
#define J2K_CCP_QNTSTY_NOQNT 0
#define J2K_CCP_QNTSTY_SIQNT 1
#define J2K_CCP_QNTSTY_SEQNT 2

/* ----------------------------------------------------------------------- */

#define J2K_MS_SOC 0xff4f   /**< SOC marker value */
#define J2K_MS_SOT 0xff90   /**< SOT marker value */
#define J2K_MS_SOD 0xff93   /**< SOD marker value */
#define J2K_MS_EOC 0xffd9   /**< EOC marker value */
#define J2K_MS_CAP 0xff50   /**< CAP marker value */
#define J2K_MS_SIZ 0xff51   /**< SIZ marker value */
#define J2K_MS_COD 0xff52   /**< COD marker value */
#define J2K_MS_COC 0xff53   /**< COC marker value */
#define J2K_MS_CPF 0xff59   /**< CPF marker value */
#define J2K_MS_RGN 0xff5e   /**< RGN marker value */
#define J2K_MS_QCD 0xff5c   /**< QCD marker value */
#define J2K_MS_QCC 0xff5d   /**< QCC marker value */
#define J2K_MS_POC 0xff5f   /**< POC marker value */
#define J2K_MS_TLM 0xff55   /**< TLM marker value */
#define J2K_MS_PLM 0xff57   /**< PLM marker value */
#define J2K_MS_PLT 0xff58   /**< PLT marker value */
#define J2K_MS_PPM 0xff60   /**< PPM marker value */
#define J2K_MS_PPT 0xff61   /**< PPT marker value */
#define J2K_MS_SOP 0xff91   /**< SOP marker value */
#define J2K_MS_EPH 0xff92   /**< EPH marker value */
#define J2K_MS_CRG 0xff63   /**< CRG marker value */
#define J2K_MS_COM 0xff64   /**< COM marker value */
#define J2K_MS_CBD 0xff78   /**< CBD marker value */
#define J2K_MS_MCC 0xff75   /**< MCC marker value */
#define J2K_MS_MCT 0xff74   /**< MCT marker value */
#define J2K_MS_MCO 0xff77   /**< MCO marker value */

#define J2K_MS_UNK 0        /**< UNKNOWN marker value */

/* UniPG>> */
#ifdef USE_JPWL
#define J2K_MS_EPC 0xff68   /**< EPC marker value (Part 11: JPEG 2000 for Wireless) */
#define J2K_MS_EPB 0xff66   /**< EPB marker value (Part 11: JPEG 2000 for Wireless) */
#define J2K_MS_ESD 0xff67   /**< ESD marker value (Part 11: JPEG 2000 for Wireless) */
#define J2K_MS_RED 0xff69   /**< RED marker value (Part 11: JPEG 2000 for Wireless) */
#endif /* USE_JPWL */
#ifdef USE_JPSEC
#define J2K_MS_SEC 0xff65    /**< SEC marker value (Part 8: Secure JPEG 2000) */
#define J2K_MS_INSEC 0xff94  /**< INSEC marker value (Part 8: Secure JPEG 2000) */
#endif /* USE_JPSEC */
/* <<UniPG */

#define J2K_MAX_POCS    32      /**< Maximum number of POCs */

#define J2K_TCD_MATRIX_MAX_LAYER_COUNT 10
#define J2K_TCD_MATRIX_MAX_RESOLUTION_COUNT 10


#pragma mark - enum
//status of the decoding process when decoding the main header.
typedef enum J2K_STATUS {
    J2K_STATE_NONE  =  0x0000, /**< a SOC marker is expected */
    J2K_STATE_MHSOC  = 0x0001, /**< a SOC marker is expected */
    J2K_STATE_MHSIZ  = 0x0002, /**< a SIZ marker is expected */
    J2K_STATE_MH     = 0x0004, /**< the decoding process is in the main header */
    J2K_STATE_TPHSOT = 0x0008, /**< the decoding process is in a tile part header and expects a SOT marker */
    J2K_STATE_TPH    = 0x0010, /**< the decoding process is in a tile part header */
    J2K_STATE_MT     = 0x0020, /**< the EOC marker has just been read */
    J2K_STATE_NEOC   = 0x0040, /**< the decoding process must not expect a EOC marker because the codestream is truncated */
    J2K_STATE_DATA   = 0x0080, /**< a tile header has been successfully read and codestream is expected */

    J2K_STATE_EOC    = 0x0100, /**< the decoding process has encountered the EOC marker */
    J2K_STATE_ERR    = 0x8000  /**< the decoding process has encountered an error (FIXME warning V1 = 0x0080)*/
} J2K_STATUS;

//Type of elements storing in the MCT data
typedef enum MCT_ELEMENT_TYPE {
    MCT_TYPE_INT16 = 0,     /** MCT data is stored as signed shorts*/
    MCT_TYPE_INT32 = 1,     /** MCT data is stored as signed integers*/
    MCT_TYPE_FLOAT = 2,     /** MCT data is stored as floats*/
    MCT_TYPE_DOUBLE = 3     /** MCT data is stored as doubles*/
} J2K_MCT_ELEMENT_TYPE;

typedef enum MCT_ARRAY_TYPE {
    MCT_TYPE_DEPENDENCY = 0,
    MCT_TYPE_DECORRELATION = 1,
    MCT_TYPE_OFFSET = 2
} J2K_MCT_ARRAY_TYPE;

typedef enum T2_MODE {
    THRESH_CALC = 0,    /** Function called in Rate allocation process*/
    FINAL_PASS = 1      /** Function called in Tier 2 process*/
} J2K_T2_MODE;


#pragma mark - unspecific structs

typedef struct opj_ppx_struct {
    OPJ_BYTE*   m_data; /* m_data == NULL => Zppx not read yet */
    OPJ_UINT32  m_data_size;
} opj_ppx;

//Quantization stepsize
typedef struct opj_stepsize {
    /** exponent */
    OPJ_INT32 expn;
    /** mantissa */
    OPJ_INT32 mant;
} opj_stepsize_t;

//Entry of a TLM marker segment
typedef struct opj_j2k_tlm_tile_part_info {
    /** Tile index of the tile part. Ttlmi field */
    OPJ_UINT16 m_tile_index;
    /** Length in bytes, from the beginning of the SOT marker to the end of
     * the bit stream data for that tile-part. Ptlmi field */
    OPJ_UINT32 m_length;
} opj_j2k_tlm_tile_part_info_t;

typedef struct opj_tgt_node {
    struct opj_tgt_node *parent;
    OPJ_INT32 value;
    OPJ_INT32 low;
    OPJ_UINT32 known;
} opj_tgt_node_t;
typedef struct opj_tgt_tree {
    OPJ_UINT32  numleafsh;
    OPJ_UINT32  numleafsv;
    OPJ_UINT32 numnodes;
    opj_tgt_node_t *nodes;
    OPJ_UINT32  nodes_size;     /* maximum size taken by nodes */
} opj_tgt_tree_t;


#pragma mark - parameters structs

//Information got from the concatenation of TLM marker semgnets
typedef struct opj_j2k_tlm_info {
    /** Number of entries in m_tile_part_infos. */
    OPJ_UINT32 m_entries_count;
    /** Array of m_entries_count values. */
    opj_j2k_tlm_tile_part_info_t* m_tile_part_infos;

    OPJ_BOOL m_is_invalid;
} opj_j2k_tlm_info_t;


typedef struct opj_mct_data {
    J2K_MCT_ELEMENT_TYPE m_element_type;
    J2K_MCT_ARRAY_TYPE   m_array_type;
    OPJ_UINT32           m_index;
    OPJ_BYTE *           m_data;
    OPJ_UINT32           m_data_size;
} opj_mct_data_t;

typedef struct opj_simple_mcc_decorrelation_data {
    OPJ_UINT32           m_index;
    OPJ_UINT32           m_nb_comps;
    opj_mct_data_t *     m_decorrelation_array;
    opj_mct_data_t *     m_offset_array;
    OPJ_BITFIELD         m_is_irreversible : 1;
} opj_simple_mcc_decorrelation_data_t;

//Tile-component coding parameters
typedef struct opj_tccp {
    /** coding style */
    OPJ_UINT32 csty;
    /** number of resolutions */
    OPJ_UINT32 numresolutions;
    /** code-blocks width */
    OPJ_UINT32 cblkw;
    /** code-blocks height */
    OPJ_UINT32 cblkh;
    /** code-block coding style */
    OPJ_UINT32 cblksty;
    /** discrete wavelet transform identifier */
    OPJ_UINT32 qmfbid;
    /** quantisation style */
    OPJ_UINT32 qntsty;
    /** stepsizes used for quantization */
    opj_stepsize_t stepsizes[OPJ_J2K_MAXBANDS];
    /** number of guard bits */
    OPJ_UINT32 numgbits;
    /** Region Of Interest shift */
    OPJ_INT32 roishift;
    /** precinct width */
    OPJ_UINT32 prcw[OPJ_J2K_MAXRLVLS];
    /** precinct height */
    OPJ_UINT32 prch[OPJ_J2K_MAXRLVLS];
    /** the dc_level_shift **/
    OPJ_INT32 m_dc_level_shift;
} opj_tccp_t;

/**
Tile coding parameters :
this structure is used to store coding/decoding parameters common to all
tiles (information like COD, COC in main header)
*/
typedef struct opj_tcp {
    /** coding style */
    OPJ_UINT32 csty;
    /** progression order */
    OPJ_PROG_ORDER prg;
    /** number of layers */
    OPJ_UINT32 numlayers;
    OPJ_UINT32 num_layers_to_decode;
    /** multi-component transform identifier */
    OPJ_UINT32 mct;
    /** rates of layers */
    OPJ_FLOAT32 rates[100];
    /** number of progression order changes */
    OPJ_UINT32 numpocs;
    /** progression order changes */
    opj_poc_t pocs[J2K_MAX_POCS];

    /** number of ppt markers (reserved size) */
    OPJ_UINT32 ppt_markers_count;
    /** ppt markers data (table indexed by Zppt) */
    opj_ppx* ppt_markers;

    /** packet header store there for future use in t2_decode_packet */
    OPJ_BYTE *ppt_data;
    /** used to keep a track of the allocated memory */
    OPJ_BYTE *ppt_buffer;
    /** Number of bytes stored inside ppt_data*/
    OPJ_UINT32 ppt_data_size;
    /** size of ppt_data*/
    OPJ_UINT32 ppt_len;
    /** PSNR values */
    OPJ_FLOAT32 distoratio[100];
    /** tile-component coding parameters */
    opj_tccp_t *tccps;
    /** current tile part number or -1 if first time into this tile */
    OPJ_INT32  m_current_tile_part_number;
    /** number of tile parts for the tile. */
    OPJ_UINT32 m_nb_tile_parts;
    /** data for the tile */
    OPJ_BYTE *      m_data;
    /** size of data */
    OPJ_UINT32      m_data_size;
    /** encoding norms */
    OPJ_FLOAT64 *   mct_norms;
    /** the mct decoding matrix */
    OPJ_FLOAT32 *   m_mct_decoding_matrix;
    /** the mct coding matrix */
    OPJ_FLOAT32 *   m_mct_coding_matrix;
    /** mct records */
    opj_mct_data_t * m_mct_records;
    /** the number of mct records. */
    OPJ_UINT32 m_nb_mct_records;
    /** the max number of mct records. */
    OPJ_UINT32 m_nb_max_mct_records;
    /** mcc records */
    opj_simple_mcc_decorrelation_data_t * m_mcc_records;
    /** the number of mct records. */
    OPJ_UINT32 m_nb_mcc_records;
    /** the max number of mct records. */
    OPJ_UINT32 m_nb_max_mcc_records;


    /***** FLAGS *******/
    /** If cod == 1 --> there was a COD marker for the present tile */
    OPJ_BITFIELD cod : 1;
    /** If ppt == 1 --> there was a PPT marker for the present tile */
    OPJ_BITFIELD ppt : 1;
    /** indicates if a POC marker has been used O:NO, 1:YES */
    OPJ_BITFIELD POC : 1;
} opj_tcp_t;

typedef enum {
    RATE_DISTORTION_RATIO = 0,    /** allocation by rate/distortion */
    FIXED_DISTORTION_RATIO = 1,   /** allocation by fixed distortion ratio (PSNR) (fixed quality) */
    FIXED_LAYER = 2,              /** allocation by fixed layer (number of passes per layer / resolution / subband) */
} J2K_QUALITY_LAYER_ALLOCATION_STRATEGY;
typedef struct opj_encoding_param {
    /** Maximum rate for each component. If == 0, component size limitation is not considered */
    //OPJ_UINT32 m_max_comp_size;
    /** Position of tile part flag in progression order*/
    OPJ_INT32 m_tp_pos;
    /** fixed layer */
    OPJ_INT32 *m_matrice;
    /** Flag determining tile part generation*/
    OPJ_BYTE m_tp_flag;
    /** Quality layer allocation strategy */
    J2K_QUALITY_LAYER_ALLOCATION_STRATEGY m_quality_layer_alloc_strategy;
    /** Enabling Tile part generation*/
    OPJ_BITFIELD m_tp_on : 1;
} opj_encoding_param_t;
typedef struct opj_decoding_param {
    /** if != 0, then original dimension divided by 2^(reduce); if == 0 or not used, image is decoded to the full resolution */
    OPJ_UINT32 m_reduce;
    /** if != 0, then only the first "layer" layers are decoded; if == 0 or not used, all the quality layers are decoded */
    OPJ_UINT32 m_layer;
} opj_decoding_param_t;
typedef struct opj_cp {
    /** Size of the image in bits*/
    /*int img_size;*/
    /** Rsiz*/
    OPJ_UINT16 rsiz;
    /** XTOsiz */
    OPJ_UINT32 tx0; /* MSD see norm */
    /** YTOsiz */
    OPJ_UINT32 ty0; /* MSD see norm */
    /** XTsiz */
    OPJ_UINT32 tdx;
    /** YTsiz */
    OPJ_UINT32 tdy;
    /** comment */
    OPJ_CHAR *comment;
    /** number of tiles in width */
    OPJ_UINT32 tw;
    /** number of tiles in height */
    OPJ_UINT32 th;

    /** number of ppm markers (reserved size) */
    OPJ_UINT32 ppm_markers_count;
    /** ppm markers data (table indexed by Zppm) */
    opj_ppx* ppm_markers;

    /** packet header store there for future use in t2_decode_packet */
    OPJ_BYTE *ppm_data;
    /** size of the ppm_data*/
    OPJ_UINT32 ppm_len;
    /** size of the ppm_data*/
    OPJ_UINT32 ppm_data_read;

    OPJ_BYTE *ppm_data_current;

    /** packet header storage original buffer */
    OPJ_BYTE *ppm_buffer;
    /** pointer remaining on the first byte of the first header if ppm is used */
    OPJ_BYTE *ppm_data_first;
    /** Number of bytes actually stored inside the ppm_data */
    OPJ_UINT32 ppm_data_size;
    /** use in case of multiple marker PPM (number of info already store) */
    OPJ_INT32 ppm_store;
    /** use in case of multiple marker PPM (case on non-finished previous info) */
    OPJ_INT32 ppm_previous;

    /** tile coding parameters */
    opj_tcp_t *tcps;

    union {
        opj_decoding_param_t m_dec;
        opj_encoding_param_t m_enc;
    }
    m_specific_param;

    /** OPJ_TRUE if entire bit stream must be decoded, OPJ_FALSE if partial bitstream decoding allowed */
    OPJ_BOOL strict;
    /******** FLAGS *********/
    /** if ppm == 1 --> there was a PPM marker*/
    OPJ_BITFIELD ppm : 1;
    /** tells if the parameter is a coding or decoding one */
    OPJ_BITFIELD m_is_decoder : 1;
    /** whether different bit depth or sign per component is allowed. Decoder only for ow */
    OPJ_BITFIELD allow_different_bit_depth_sign : 1;
    /* <<UniPG */
} opj_cp_t;
typedef struct opj_j2k_dec {
    /** locate in which part of the codestream the decoder is (main header, tile header, end) */
    OPJ_UINT32 m_state;
    /**
     * store decoding parameters common to all tiles (information like COD, COC in main header)
     */
    opj_tcp_t *m_default_tcp;
    OPJ_BYTE  *m_header_data;
    OPJ_UINT32 m_header_data_size;
    /** to tell the tile part length */
    OPJ_UINT32 m_sot_length;
    /** Only tiles index in the correct range will be decoded.*/
    OPJ_UINT32 m_start_tile_x;
    OPJ_UINT32 m_start_tile_y;
    OPJ_UINT32 m_end_tile_x;
    OPJ_UINT32 m_end_tile_y;

    /** Index of the tile to decode (used in get_tile) */
    OPJ_INT32 m_tile_ind_to_dec;
    /** Position of the last SOT marker read */
    OPJ_OFF_T m_last_sot_read_pos;

    /**
     * Indicate that the current tile-part is assume as the last tile part of the codestream.
     * It is useful in the case of PSot is equal to zero. The sot length will be compute in the
     * SOD reader function. FIXME NOT USED for the moment
     */
    OPJ_BOOL   m_last_tile_part;

    OPJ_UINT32   m_numcomps_to_decode;
    OPJ_UINT32  *m_comps_indices_to_decode;

    opj_j2k_tlm_info_t m_tlm;

    /** Below if used when there's TLM information available and we use
     * opj_set_decoded_area() to a subset of all tiles.
     */
    /* Current index in m_intersecting_tile_parts_offset[] to seek to */
    OPJ_UINT32  m_idx_intersecting_tile_parts;
    /* Number of elements of m_intersecting_tile_parts_offset[] */
    OPJ_UINT32  m_num_intersecting_tile_parts;
    /* Start offset of contributing tile parts */
    OPJ_OFF_T*  m_intersecting_tile_parts_offset;

    /** to tell that a tile can be decoded. */
    OPJ_BITFIELD m_can_decode : 1;
    OPJ_BITFIELD m_discard_tiles : 1;
    OPJ_BITFIELD m_skip_data : 1;
    /** TNsot correction : see issue 254 **/
    OPJ_BITFIELD m_nb_tile_parts_correction_checked : 1;
    OPJ_BITFIELD m_nb_tile_parts_correction : 1;

} opj_j2k_dec_t;
typedef struct opj_j2k_enc {
    /** Tile part number, regardless of poc, for each new poc, tp is reset to 1*/
    OPJ_UINT32 m_current_poc_tile_part_number; /* tp_num */

    /** Tile part number currently coding, taking into account POC. m_current_tile_part_number holds the total number of tile parts while encoding the last tile part.*/
    OPJ_UINT32 m_current_tile_part_number; /*cur_tp_num */

    /* whether to generate TLM markers */
    OPJ_BOOL   m_TLM;

    /* whether the Ttlmi field in a TLM marker is a byte (otherwise a uint16) */
    OPJ_BOOL   m_Ttlmi_is_byte;

    /**
    locate the start position of the TLM marker
    after encoding the tilepart, a jump (in j2k_write_sod) is done to the TLM marker to store the value of its length.
    */
    OPJ_OFF_T m_tlm_start;
    /**
     * Stores the sizes of the tlm.
     */
    OPJ_BYTE * m_tlm_sot_offsets_buffer;
    /**
     * The current offset of the tlm buffer.
     */
    OPJ_BYTE * m_tlm_sot_offsets_current;

    /** Total num of tile parts in whole image = num tiles* num tileparts in each tile*/
    /** used in TLMmarker*/
    OPJ_UINT32 m_total_tile_parts;   /* totnum_tp */

    OPJ_BYTE * m_encoded_tile_data;
    OPJ_UINT32 m_encoded_tile_size;

    OPJ_BYTE * m_header_tile_data;
    OPJ_UINT32 m_header_tile_data_size;

    /* whether to generate PLT markers */
    OPJ_BOOL   m_PLT;

    /* reserved bytes in m_encoded_tile_size for PLT markers */
    OPJ_UINT32 m_reserved_bytes_for_PLT;

    /** Number of components */
    OPJ_UINT32 m_nb_comps;

} opj_j2k_enc_t;
//JF???? struct opj_tcd;
typedef struct opj_j2k {
    /* J2K codestream is decoded*/
    OPJ_BOOL m_is_decoder;

    /* FIXME DOC*/
    union {
        opj_j2k_dec_t m_decoder;
        opj_j2k_enc_t m_encoder;
    }
    m_specific_param;

    /** pointer to the internal/private encoded / decoded image */
    opj_image_t* m_private_image;

    /* pointer to the output image (decoded)*/
    opj_image_t* m_output_image;

    /** Coding parameters */
    opj_cp_t m_cp;

    /** the list of procedures to exec **/
    opj_procedure_list_t *  m_procedure_list;

    /** the list of validation procedures to follow to make sure the code is valid **/
    opj_procedure_list_t *  m_validation_list;

    /** helper used to write the index file */
    opj_codestream_index_t *cstr_index;

    /** number of the tile currently concern by coding/decoding */
    OPJ_UINT32 m_current_tile_number;

    /** the current tile coder/decoder **/
    struct opj_tcd *    m_tcd;

    /** Thread pool */
    opj_thread_pool_t* m_tp;

    /** Image width coming from JP2 IHDR box. 0 from a pure codestream */
    OPJ_UINT32 ihdr_w;

    /** Image height coming from JP2 IHDR box. 0 from a pure codestream */
    OPJ_UINT32 ihdr_h;

} opj_j2k_t;







#pragma mark -

OPJ_BOOL opj_j2k_set_threads(opj_j2k_t *j2k, OPJ_UINT32 num_threads);

/**
 * Creates a J2K compression structure
 *
 * @return Returns a handle to a J2K compressor if successful, returns NULL otherwise
*/
opj_j2k_t* opj_j2k_create_compress(void);


OPJ_BOOL opj_j2k_setup_encoder(opj_j2k_t *p_j2k,
                               opj_cparameters_t *parameters,
                               opj_image_t *image,
                               opj_event_mgr_t * p_manager);

//Converts an enum type progression order to string type
const char *opj_j2k_convert_progression_order(OPJ_PROG_ORDER prg_order);

void opj_j2k_destroy(opj_j2k_t *p_j2k);
void j2k_destroy_cstr_index(opj_codestream_index_t *p_cstr_ind);

/**
 * Writes a tile.
 * @param   p_j2k       the jpeg2000 codec.
 * @param p_tile_index FIXME DOC
 * @param p_data FIXME DOC
 * @param p_data_size FIXME DOC
 * @param   p_stream            the stream to write data to.
 * @param   p_manager   the user event manager.
 */
OPJ_BOOL opj_j2k_write_tile(opj_j2k_t * p_j2k,
                            OPJ_UINT32 p_tile_index,
                            OPJ_BYTE * p_data,
                            OPJ_UINT32 p_data_size,
                            opj_stream_private_t *p_stream,
                            opj_event_mgr_t * p_manager);

/**
 * Encodes an image into a JPEG-2000 codestream
 */
OPJ_BOOL opj_j2k_encode(opj_j2k_t * p_j2k,
                        opj_stream_private_t *cio);

/**
 * Starts a compression scheme, i.e. validates the codec parameters, writes the header.
 *
 * @param   p_j2k       the jpeg2000 codec.
 * @param   p_stream            the stream object.
 * @param   p_image FIXME DOC
 * @param   p_manager   the user event manager.
 *
 * @return true if the codec is valid.
 */
OPJ_BOOL opj_j2k_start_compress(opj_j2k_t *p_j2k,
                                opj_stream_private_t *p_stream,
                                opj_image_t * p_image,
                                opj_event_mgr_t * p_manager);

/**
 * Ends the compression procedures and possibiliy add data to be read after the
 * codestream.
 */
OPJ_BOOL opj_j2k_end_compress(opj_j2k_t *p_j2k,
                              opj_stream_private_t *cio,
                              opj_event_mgr_t * p_manager);

OPJ_BOOL opj_j2k_setup_mct_encoding(opj_tcp_t * p_tcp, opj_image_t * p_image);

#pragma mark - procedures execution
/** @defgroup J2K J2K - JPEG-2000 codestream reader/writer */

/**
 * Executes the given procedures on the given codec.
 *
 * @param       p_procedure_list        the list of procedures to execute
 *
 * @return      true                            if all the procedures were successfully executed.
 */

static OPJ_BOOL opj_j2k_exec(
 opj_j2k_t            * p_j2k,
 opj_procedure_list_t * p_procedure_list,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_build_encoder(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_mct_validation(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_copy_default_tcp_and_create_tcd(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_create_tcd(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);

//read?
static OPJ_BOOL opj_j2k_update_rates(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_decode_tiles(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_get_end_header(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_init_info(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);

//read
static OPJ_BOOL opj_j2k_read_header_procedure(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_read_soc(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_read_sod(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);


//write
static OPJ_BOOL opj_j2k_post_write_tile(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream
);
static OPJ_BOOL opj_j2k_write_soc(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_siz(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_com(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_cod(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_qcd(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_poc(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_updated_tlm(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_tlm(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_eoc(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
//Writes the CBD-MCT-MCC-MCO markers (Multi components transform)
static OPJ_BOOL opj_j2k_write_mct_data_group(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_mco(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_cbd(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_all_coc(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_all_qcc(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_regions(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_write_epc(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);

//end
static OPJ_BOOL opj_j2k_destroy_header_memory(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_j2k_end_encoding(
 opj_j2k_t            * p_j2k,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);


#pragma mark -
















/**
 * Reads the lookup table containing all the marker, status and action,
 * and returns the handler associated with the marker value.
 * @param       p_id            Marker value to look up
 *
 * @return      the handler associated with the id.
*/
static const struct opj_dec_memory_marker_handler * opj_j2k_get_marker_handler(
    OPJ_UINT32 p_id);

/**
 * Destroys a tile coding parameter structure.
 *
 * @param       p_tcp           the tile coding parameter to destroy.
 */
static void opj_j2k_tcp_destroy(opj_tcp_t *p_tcp);

/**
 * Destroys the data inside a tile coding parameter structure.
 *
 * @param       p_tcp           the tile coding parameter which contain data to destroy.
 */
static void opj_j2k_tcp_data_destroy(opj_tcp_t *p_tcp);

/**
 * Destroys a coding parameter structure.
 *
 * @param       p_cp            the coding parameter to destroy.
 */
static void opj_j2k_cp_destroy(opj_cp_t *p_cp);

/**
 * Compare 2 a SPCod/ SPCoc elements, i.e. the coding style of a given component of a tile.
 *
 * @param       p_j2k            J2K codec.
 * @param       p_tile_no        Tile number
 * @param       p_first_comp_no  The 1st component number to compare.
 * @param       p_second_comp_no The 1st component number to compare.
 *
 * @return OPJ_TRUE if SPCdod are equals.
 */
static OPJ_BOOL opj_j2k_compare_SPCod_SPCoc(
 opj_j2k_t  * p_j2k,
 OPJ_UINT32   p_tile_no,
 OPJ_UINT32   p_first_comp_no,
 OPJ_UINT32   p_second_comp_no
);

/**
 * Writes a SPCod or SPCoc element, i.e. the coding style of a given component of a tile.
 *
 * @param       p_tile_no       FIXME DOC
 * @param       p_comp_no       the component number to output.
 * @param       p_data          FIXME DOC
 * @param       p_header_size   FIXME DOC
 *
 * @return FIXME DOC
*/
static OPJ_BOOL opj_j2k_write_SPCod_SPCoc(
 opj_j2k_t       * p_j2k,
 OPJ_UINT32        p_tile_no,
 OPJ_UINT32        p_comp_no,
 OPJ_BYTE        * p_data,
 OPJ_UINT32      * p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Gets the size taken by writing a SPCod or SPCoc for the given tile and component.
 *
 * @param       p_tile_no               the tile index.
 * @param       p_comp_no               the component being outputted.
 *
 * @return      the number of bytes taken by the SPCod element.
 */
static OPJ_UINT32 opj_j2k_get_SPCod_SPCoc_size(
 opj_j2k_t  * p_j2k,
 OPJ_UINT32   p_tile_no,
 OPJ_UINT32   p_comp_no
);

/**
 * Reads a SPCod or SPCoc element, i.e. the coding style of a given component of a tile.
 * @param       compno          FIXME DOC
 * @param       p_header_data   the data contained in the COM box.
 * @param       p_header_size   the size of the data contained in the COM marker.
*/
static OPJ_BOOL opj_j2k_read_SPCod_SPCoc(
 opj_j2k_t       * p_j2k,
 OPJ_UINT32        compno,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32      * p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Gets the size taken by writing SQcd or SQcc element, i.e. the quantization values of a band in the QCD or QCC.
 *
 * @param       p_tile_no               the tile index.
 * @param       p_comp_no               the component being outputted.
 *
 * @return      the number of bytes taken by the SPCod element.
 */
static OPJ_UINT32 opj_j2k_get_SQcd_SQcc_size(
 opj_j2k_t  * p_j2k,
 OPJ_UINT32   p_tile_no,
 OPJ_UINT32   p_comp_no
);

/**
 * Compares 2 SQcd or SQcc element, i.e. the quantization values of a band in the QCD or QCC.
 *
 * @param       p_tile_no               the tile to output.
 * @param       p_first_comp_no         the first component number to compare.
 * @param       p_second_comp_no        the second component number to compare.
 *
 * @return OPJ_TRUE if equals.
 */
static OPJ_BOOL opj_j2k_compare_SQcd_SQcc(
 opj_j2k_t  * p_j2k,
 OPJ_UINT32   p_tile_no,
 OPJ_UINT32   p_first_comp_no,
 OPJ_UINT32   p_second_comp_no
);


/**
 * Writes a SQcd or SQcc element, i.e. the quantization values of a band in the QCD or QCC.
 *
 * @param       p_tile_no               the tile to output.
 * @param       p_comp_no               the component number to output.
 * @param       p_header_size   pointer to the size of the data buffer, it is changed by the function.
 *
*/
static OPJ_BOOL opj_j2k_write_SQcd_SQcc(
 opj_j2k_t       * p_j2k,
 OPJ_UINT32        p_tile_no,
 OPJ_UINT32        p_comp_no,
 OPJ_BYTE        * p_data,
 OPJ_UINT32      * p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Updates the Tile Length Marker.
 */
static void opj_j2k_update_tlm(opj_j2k_t * p_j2k, OPJ_UINT32 p_tile_part_size);

/**
 * Reads a SQcd or SQcc element, i.e. the quantization values of a band in the QCD or QCC.
 *
 * @param       compno          the component number to output.
 * @param       p_header_data   the data buffer.
 * @param       p_header_size   pointer to the size of the data buffer, it is changed by the function.
 *
*/
static OPJ_BOOL opj_j2k_read_SQcd_SQcc(
 opj_j2k_t       * p_j2k,
 OPJ_UINT32        compno,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32      * p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Copies the tile component parameters of all the component from the first tile component.
 *
 * @param               p_j2k           the J2k codec.
 */
static void opj_j2k_copy_tile_component_parameters(opj_j2k_t *p_j2k);

/**
 * Copies the tile quantization parameters of all the component from the first tile component.
 *
 * @param               p_j2k           the J2k codec.
 */
static void opj_j2k_copy_tile_quantization_parameters(opj_j2k_t *p_j2k);





#pragma mark -tcd

typedef struct opj_tcd_pass {
    OPJ_UINT32 rate;
    OPJ_FLOAT64 distortiondec;
    OPJ_UINT32 len;
    OPJ_BITFIELD term : 1;
} opj_tcd_pass_t;

typedef struct opj_tcd_layer {
    OPJ_UINT32 numpasses;       /* Number of passes in the layer */
    OPJ_UINT32 len;             /* len of information */
    OPJ_FLOAT64 disto;          /* add for index (Cfr. Marcela) */
    OPJ_BYTE *data;             /* data */
} opj_tcd_layer_t;

typedef struct opj_tcd_cblk_enc {
    OPJ_BYTE* data;               /* Data */
    opj_tcd_layer_t* layers;      /* layer information */
    opj_tcd_pass_t* passes;       /* information about the passes */
    OPJ_INT32 x0, y0, x1,
              y1;     /* dimension of the code-blocks : left upper corner (x0, y0) right low corner (x1,y1) */
    OPJ_UINT32 numbps;
    OPJ_UINT32 numlenbits;
    OPJ_UINT32 data_size;         /* Size of allocated data buffer */
    OPJ_UINT32
    numpasses;         /* number of pass already done for the code-blocks */
    OPJ_UINT32 numpassesinlayers; /* number of passes in the layer */
    OPJ_UINT32 totalpasses;       /* total number of passes */
} opj_tcd_cblk_enc_t;

/** Chunk of codestream data that is part of a code block */
typedef struct opj_tcd_seg_data_chunk {
    /* Point to tilepart buffer. We don't make a copy !
       So the tilepart buffer must be kept alive
       as long as we need to decode the codeblocks */
    OPJ_BYTE * data;
    OPJ_UINT32 len;                 /* Usable length of data */
} opj_tcd_seg_data_chunk_t;

/** Segment of a code-block.
 * A segment represent a number of consecutive coding passes, without termination
 * of MQC or RAW between them. */
typedef struct opj_tcd_seg {
    OPJ_UINT32 len;      /* Size of data related to this segment */
    /* Number of passes decoded. Including those that we skip */
    OPJ_UINT32 numpasses;
    /* Number of passes actually to be decoded. To be used for code-block decoding */
    OPJ_UINT32 real_num_passes;
    /* Maximum number of passes for this segment */
    OPJ_UINT32 maxpasses;
    /* Number of new passes for current packed. Transitory value */
    OPJ_UINT32 numnewpasses;
    /* Codestream length for this segment for current packed. Transitory value */
    OPJ_UINT32 newlen;
} opj_tcd_seg_t;

/** Code-block for decoding */
typedef struct opj_tcd_cblk_dec {
    opj_tcd_seg_t* segs;            /* segments information */
    opj_tcd_seg_data_chunk_t* chunks; /* Array of chunks */
    /* position of the code-blocks : left upper corner (x0, y0) right low corner (x1,y1) */
    OPJ_INT32 x0, y0, x1, y1;
    /* Mb is The maximum number of bit-planes available for the representation of
       coefficients in any sub-band, b, as defined in Equation (E-2). See
       Section B.10.5 of the standard */
    OPJ_UINT32 Mb;  /* currently used only to check if HT decoding is correct */
    /* numbps is Mb - P as defined in Section B.10.5 of the standard */
    OPJ_UINT32 numbps;
    /* number of bits for len, for the current packet. Transitory value */
    OPJ_UINT32 numlenbits;
    /* number of pass added to the code-blocks, for the current packet. Transitory value */
    OPJ_UINT32 numnewpasses;
    /* number of segments, including those of packet we skip */
    OPJ_UINT32 numsegs;
    /* number of segments, to be used for code block decoding */
    OPJ_UINT32 real_num_segs;
    OPJ_UINT32 m_current_max_segs;  /* allocated number of segs[] items */
    OPJ_UINT32 numchunks;           /* Number of valid chunks items */
    OPJ_UINT32 numchunksalloc;      /* Number of chunks item allocated */
    /* Decoded code-block. Only used for subtile decoding. Otherwise tilec->data is directly updated */
    OPJ_INT32* decoded_data;
    OPJ_BOOL corrupted; /* whether the code block data is corrupted */
} opj_tcd_cblk_dec_t;

/** Precinct structure */
typedef struct opj_tcd_precinct {
    /* dimension of the precinct : left upper corner (x0, y0) right low corner (x1,y1) */
    OPJ_INT32 x0, y0, x1, y1;
    OPJ_UINT32 cw, ch;              /* number of code-blocks, in width and height */
    union {                         /* code-blocks information */
        opj_tcd_cblk_enc_t* enc;
        opj_tcd_cblk_dec_t* dec;
        void*               blocks;
    } cblks;
    OPJ_UINT32 block_size;          /* size taken by cblks (in bytes) */
    opj_tgt_tree_t *incltree;       /* inclusion tree */
    opj_tgt_tree_t *imsbtree;       /* IMSB tree */
} opj_tcd_precinct_t;

/** Sub-band structure */
typedef struct opj_tcd_band {
    /* dimension of the subband : left upper corner (x0, y0) right low corner (x1,y1) */
    OPJ_INT32 x0, y0, x1, y1;
    /* band number: for lowest resolution level (0=LL), otherwise (1=HL, 2=LH, 3=HH) */
    OPJ_UINT32 bandno;
    /* precinct information */
    opj_tcd_precinct_t *precincts;
    /* size of data taken by precincts */
    OPJ_UINT32 precincts_data_size;
    OPJ_INT32 numbps;
    OPJ_FLOAT32 stepsize;
} opj_tcd_band_t;

/** Tile-component resolution structure */
typedef struct opj_tcd_resolution {
    /* dimension of the resolution level : left upper corner (x0, y0) right low corner (x1,y1) */
    OPJ_INT32 x0, y0, x1, y1;
    /* number of precincts, in width and height, for this resolution level */
    OPJ_UINT32 pw, ph;
    /* number of sub-bands for the resolution level (1 for lowest resolution level, 3 otherwise) */
    OPJ_UINT32 numbands;
    /* subband information */
    opj_tcd_band_t bands[3];

    /* dimension of the resolution limited to window of interest. Only valid if tcd->whole_tile_decoding is set */
    OPJ_UINT32 win_x0;
    OPJ_UINT32 win_y0;
    OPJ_UINT32 win_x1;
    OPJ_UINT32 win_y1;
} opj_tcd_resolution_t;

/** Tile-component structure */
typedef struct opj_tcd_tilecomp {
    /* dimension of component : left upper corner (x0, y0) right low corner (x1,y1) */
    OPJ_INT32 x0, y0, x1, y1;
    /* component number */
    OPJ_UINT32 compno;
    /* number of resolutions level */
    OPJ_UINT32 numresolutions;
    /* number of resolutions level to decode (at max)*/
    OPJ_UINT32 minimum_num_resolutions;
    /* resolutions information */
    opj_tcd_resolution_t *resolutions;
    /* size of data for resolutions (in bytes) */
    OPJ_UINT32 resolutions_size;

    /* data of the component. For decoding, only valid if tcd->whole_tile_decoding is set (so exclusive of data_win member) */
    OPJ_INT32 *data;
    /* if true, then need to free after usage, otherwise do not free */
    OPJ_BOOL  ownsData;
    /* we may either need to allocate this amount of data, or re-use image data and ignore this value */
    size_t data_size_needed;
    /* size of the data of the component */
    size_t data_size;

    /** data of the component limited to window of interest. Only valid for decoding and if tcd->whole_tile_decoding is NOT set (so exclusive of data member) */
    OPJ_INT32 *data_win;
    /* dimension of the component limited to window of interest. Only valid for decoding and  if tcd->whole_tile_decoding is NOT set */
    OPJ_UINT32 win_x0;
    OPJ_UINT32 win_y0;
    OPJ_UINT32 win_x1;
    OPJ_UINT32 win_y1;

    /* number of pixels */
    OPJ_SIZE_T numpix;
} opj_tcd_tilecomp_t;

typedef struct opj_tcd_tile {
    /* dimension of the tile : left upper corner (x0, y0) right low corner (x1,y1) */
    OPJ_INT32 x0, y0, x1, y1;
    OPJ_UINT32 numcomps;            /* number of components in tile */
    opj_tcd_tilecomp_t *comps;  /* Components information */
    OPJ_SIZE_T numpix;               /* number of pixels */
    OPJ_FLOAT64 distotile;          /* distortion of the tile */
    OPJ_FLOAT64 distolayer[100];    /* distortion per layer */
    OPJ_UINT32 packno;              /* packet number */
} opj_tcd_tile_t;

typedef struct opj_tcd_image {
    opj_tcd_tile_t *tiles;      /* Tiles information */
} opj_tcd_image_t;

typedef struct opj_tcd {
    /** Position of the tilepart flag in Progression order*/
    OPJ_INT32 tp_pos;
    /** Tile part number*/
    OPJ_UINT32 tp_num;
    /** Current tile part number*/
    OPJ_UINT32 cur_tp_num;
    /** Total number of tileparts of the current tile*/
    OPJ_UINT32 cur_totnum_tp;
    /** Current Packet iterator number */
    OPJ_UINT32 cur_pino;
    /** info on each image tile */
    opj_tcd_image_t *tcd_image;
    /** image header */
    opj_image_t *image;
    /** coding parameters */
    opj_cp_t *cp;
    /** coding/decoding parameters common to all tiles */
    opj_tcp_t *tcp;
    /** current encoded/decoded tile */
    OPJ_UINT32 tcd_tileno;
    /** tell if the tcd is a decoder. */
    OPJ_BITFIELD m_is_decoder : 1;
    /** Thread pool */
    opj_thread_pool_t* thread_pool;
    /** Coordinates of the window of interest, in grid reference space */
    OPJ_UINT32 win_x0;
    OPJ_UINT32 win_y0;
    OPJ_UINT32 win_x1;
    OPJ_UINT32 win_y1;
    /** Only valid for decoding. Whether the whole tile is decoded, or just the region in win_x0/win_y0/win_x1/win_y1 */
    OPJ_BOOL   whole_tile_decoding;
    /* Array of size image->numcomps indicating if a component must be decoded. NULL if all components must be decoded */
    OPJ_BOOL* used_component;
} opj_tcd_t;

//Structure to hold information needed to generate some markers.
//Used by encoder.
typedef struct opj_tcd_marker_info {
    /** In: Whether information to generate PLT markers in needed */
    OPJ_BOOL    need_PLT;

    /** OUT: Number of elements in p_packet_size[] array */
    OPJ_UINT32  packet_count;

    /** OUT: Array of size packet_count, such that p_packet_size[i] is
     *       the size in bytes of the ith packet */
    OPJ_UINT32* p_packet_size;
} opj_tcd_marker_info_t;


opj_tcd_t* opj_tcd_create(OPJ_BOOL p_is_decoder);
void opj_tcd_destroy(opj_tcd_t *tcd);

opj_tcd_marker_info_t* opj_tcd_marker_info_create(OPJ_BOOL need_PLT);
void opj_tcd_marker_info_destroy(opj_tcd_marker_info_t *p_tcd_marker_info);


/**
 * Initialize the tile coder and may reuse some memory.
 * @param   p_tcd       TCD handle.
 * @param   p_image     raw image.
 * @param   p_cp        coding parameters.
 * @param   p_tp        thread pool
 *
 * @return true if the encoding values could be set (false otherwise).
*/
OPJ_BOOL opj_tcd_init(opj_tcd_t *p_tcd,
                      opj_image_t * p_image,
                      opj_cp_t * p_cp,
                      opj_thread_pool_t* p_tp);

/**
 * Allocates memory for decoding a specific tile.
 *
 * @param   p_tcd       the tile decoder.
 * @param   p_tile_no   the index of the tile received in sequence. This not necessarily lead to the
 * tile at index p_tile_no.
 * @param p_manager the event manager.
 *
 * @return  true if the remaining data is sufficient.
 */
OPJ_BOOL opj_tcd_init_decode_tile(opj_tcd_t *p_tcd, OPJ_UINT32 p_tile_no,
                                  opj_event_mgr_t* p_manager);

/**
 * Gets the maximum tile size that will be taken by the tile once decoded.
 */
OPJ_UINT32 opj_tcd_get_decoded_tile_size(opj_tcd_t *p_tcd,
        OPJ_BOOL take_into_account_partial_decoding);

/**
 * Encodes a tile from the raw image into the given buffer.
 * @param   p_tcd           Tile Coder handle
 * @param   p_tile_no       Index of the tile to encode.
 * @param   p_dest          Destination buffer
 * @param   p_data_written  pointer to an int that is incremented by the number of bytes really written on p_dest
 * @param   p_len           Maximum length of the destination buffer
 * @param   p_cstr_info     Codestream information structure
 * @param   p_marker_info   Marker information structure
 * @return  true if the coding is successful.
*/
OPJ_BOOL opj_tcd_encode_tile(opj_tcd_t *p_tcd,
                             OPJ_UINT32 p_tile_no,
                             OPJ_BYTE *p_dest,
                             OPJ_UINT32 * p_data_written,
                             OPJ_UINT32 p_len,
                             struct opj_codestream_info *p_cstr_info,
                             opj_tcd_marker_info_t* p_marker_info);


/**
Decode a tile from a buffer into a raw image
@param tcd TCD handle
@param win_x0 Upper left x of region to decode (in grid coordinates)
@param win_y0 Upper left y of region to decode (in grid coordinates)
@param win_x1 Lower right x of region to decode (in grid coordinates)
@param win_y1 Lower right y of region to decode (in grid coordinates)
@param numcomps_to_decode  Size of the comps_indices array, or 0 if decoding all components.
@param comps_indices   Array of numcomps values representing the indices
                       of the components to decode (relative to the
                       codestream, starting at 0). Or NULL if decoding all components.
@param src Source buffer
@param len Length of source buffer
@param tileno Number that identifies one of the tiles to be decoded
@param cstr_info  FIXME DOC
@param manager the event manager.
*/
OPJ_BOOL opj_tcd_decode_tile(opj_tcd_t *tcd,
                             OPJ_UINT32 win_x0,
                             OPJ_UINT32 win_y0,
                             OPJ_UINT32 win_x1,
                             OPJ_UINT32 win_y1,
                             OPJ_UINT32 numcomps_to_decode,
                             const OPJ_UINT32 *comps_indices,
                             OPJ_BYTE *src,
                             OPJ_UINT32 len,
                             OPJ_UINT32 tileno,
                             opj_codestream_index_t *cstr_info,
                             opj_event_mgr_t *manager);


/**
 * Copies tile data from the system onto the given memory block.
 */
OPJ_BOOL opj_tcd_update_tile_data(opj_tcd_t *p_tcd,
                                  OPJ_BYTE * p_dest,
                                  OPJ_UINT32 p_dest_length);

/**
 * Get the size in bytes of the input buffer provided before encoded.
 * This must be the size provided to the p_src_length argument of
 * opj_tcd_copy_tile_data()
 */
OPJ_SIZE_T opj_tcd_get_encoder_input_buffer_size(opj_tcd_t *p_tcd);

/**
 * Initialize the tile coder and may reuse some memory.
 *
 * @param   p_tcd       TCD handle.
 * @param   p_tile_no   current tile index to encode.
 * @param p_manager the event manager.
 *
 * @return true if the encoding values could be set (false otherwise).
*/
OPJ_BOOL opj_tcd_init_encode_tile(opj_tcd_t *p_tcd,
                                  OPJ_UINT32 p_tile_no, opj_event_mgr_t* p_manager);

/**
 * Copies tile data from the given memory block onto the system.
 *
 * p_src_length must be equal to opj_tcd_get_encoder_input_buffer_size()
 */
OPJ_BOOL opj_tcd_copy_tile_data(opj_tcd_t *p_tcd,
                                OPJ_BYTE * p_src,
                                OPJ_SIZE_T p_src_length);

/**
 * Allocates tile component data
 *
 *
 */
OPJ_BOOL opj_alloc_tile_component_data(opj_tcd_tilecomp_t *l_tilec);

/** Returns whether a sub-band is empty (i.e. whether it has a null area)
 * @param band Sub-band handle.
 * @return OPJ_TRUE whether the sub-band is empty.
 */
OPJ_BOOL opj_tcd_is_band_empty(opj_tcd_band_t* band);

/** Reinitialize a segment */
void opj_tcd_reinit_segment(opj_tcd_seg_t* seg);


/** Returns whether a sub-band region contributes to the area of interest
 * tcd->win_x0,tcd->win_y0,tcd->win_x1,tcd->win_y1.
 *
 * @param tcd    TCD handle.
 * @param compno Component number
 * @param resno  Resolution number
 * @param bandno Band number (*not* band index, ie 0, 1, 2 or 3)
 * @param x0     Upper left x in subband coordinates
 * @param y0     Upper left y in subband coordinates
 * @param x1     Lower right x in subband coordinates
 * @param y1     Lower right y in subband coordinates
 * @return OPJ_TRUE whether the sub-band region contributes to the area of
 *                  interest.
 */
OPJ_BOOL opj_tcd_is_subband_area_of_interest(opj_tcd_t *tcd,
        OPJ_UINT32 compno,
        OPJ_UINT32 resno,
        OPJ_UINT32 bandno,
        OPJ_UINT32 x0,
        OPJ_UINT32 y0,
        OPJ_UINT32 x1,
        OPJ_UINT32 y1);

#pragma mark -tgt

//Implementation of a tag-tree coder (TGT)
//used by some function in T2.C.




/**
Create a tag-tree
@param numleafsh Width of the array of leafs of the tree
@param numleafsv Height of the array of leafs of the tree
@param p_manager the event manager
@return Returns a new tag-tree if successful, returns NULL otherwise
*/
opj_tgt_tree_t *opj_tgt_create(OPJ_UINT32 numleafsh, OPJ_UINT32 numleafsv,opj_event_mgr_t *p_manager);
/**
 * Reinitialises a tag-tree from an existing one.
 *
 * @param   p_tree              the tree to reinitialize.
 * @param   p_num_leafs_h       the width of the array of leafs of the tree
 * @param   p_num_leafs_v       the height of the array of leafs of the tree
 * @param p_manager       the event manager
 * @return  a new tag-tree if successful, NULL otherwise
*/
opj_tgt_tree_t *opj_tgt_init(opj_tgt_tree_t * p_tree,OPJ_UINT32  p_num_leafs_h,OPJ_UINT32  p_num_leafs_v, opj_event_mgr_t *p_manager);
void opj_tgt_destroy(opj_tgt_tree_t *tree);
//Reset a tag-tree (set all leaves to 0)
void opj_tgt_reset(opj_tgt_tree_t *tree);
void opj_tgt_setvalue(opj_tgt_tree_t *tree,OPJ_UINT32 leafno,OPJ_INT32 value);
/**
Encode the value of a leaf of the tag-tree up to a given threshold
@param bio Pointer to a BIO handle
@param tree Tag-tree to modify
@param leafno Number that identifies the leaf to encode
@param threshold Threshold to use when encoding value of the leaf
*/
void opj_tgt_encode(opj_bio_t *bio,
                    opj_tgt_tree_t *tree,
                    OPJ_UINT32 leafno,
                    OPJ_INT32 threshold);
/**
Decode the value of a leaf of the tag-tree up to a given threshold
@param bio Pointer to a BIO handle
@param tree Tag-tree to decode
@param leafno Number that identifies the leaf to decode
@param threshold Threshold to use when decoding value of the leaf
@return Returns 1 if the node's value < threshold, returns 0 otherwise
*/
//JF OPJ_UINT32 opj_tgt_decode(opj_bio_t *bio,opj_tgt_tree_t *tree,OPJ_UINT32 leafno,OPJ_INT32 threshold);


#pragma mark - markers

static OPJ_BOOL opj_j2k_pre_write_tile(
 opj_j2k_t            * p_j2k,
 OPJ_UINT32             p_tile_index,
 opj_stream_private_t * p_stream
);

static OPJ_BOOL opj_j2k_update_image_data(
 opj_tcd_t   * p_tcd,
 opj_image_t * p_output_image
);


/**
 * Sets up the procedures to do on writing header.
 * Developers wanting to extend the library can add their own writing procedures.
 */

static OPJ_BOOL opj_j2k_write_first_tile_part(
 opj_j2k_t            * p_j2k,
 OPJ_BYTE             * p_data,
 OPJ_UINT32           * p_data_written,
 OPJ_UINT32             total_data_size,
 opj_stream_private_t * p_stream,
 struct opj_event_mgr * p_manager);

static OPJ_BOOL opj_j2k_write_all_tile_parts(
 opj_j2k_t            * p_j2k,
 OPJ_BYTE             * p_data,
 OPJ_UINT32           * p_data_written,
 OPJ_UINT32             total_data_size,
 opj_stream_private_t * p_stream
);

/**
 * Reads a SIZ marker (image and tile size)
 * @param       p_header_data   the data contained in the SIZ box.
 * @param       p_header_size   the size of the data contained in the SIZ marker.
*/
static OPJ_BOOL opj_j2k_read_siz(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);


/**
 * Reads a COM marker (comments)
 * @param       p_header_data   the data contained in the COM box.
 * @param       p_header_size   the size of the data contained in the COM marker.
*/
static OPJ_BOOL opj_j2k_read_com(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);


/**
 * Reads a COD marker (Coding style defaults)
 * @param       p_header_data   the data contained in the COD box.
 * @param       p_header_size   the size of the data contained in the COD marker.
*/
static OPJ_BOOL opj_j2k_read_cod(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Compares 2 COC markers (Coding style component)
 *
 * @param       p_first_comp_no  the index of the first component to compare.
 * @param       p_second_comp_no the index of the second component to compare.
 *
 * @return      OPJ_TRUE if equals
 */
static OPJ_BOOL opj_j2k_compare_coc(
 opj_j2k_t  * p_j2k,
 OPJ_UINT32   p_first_comp_no,
 OPJ_UINT32   p_second_comp_no
);

/**
 * Writes the COC marker (Coding style component)
 *
 * @param       p_comp_no   the index of the component to output.
*/
static OPJ_BOOL opj_j2k_write_coc(
 opj_j2k_t            * p_j2k,
 OPJ_UINT32             p_comp_no,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);

/**
 * Writes the COC marker (Coding style component)
 *
 * @param       p_comp_no               the index of the component to output.
 * @param       p_data          FIXME DOC
 * @param       p_data_written  FIXME DOC
*/
static void opj_j2k_write_coc_in_memory(
 opj_j2k_t       * p_j2k,
 OPJ_UINT32        p_comp_no,
 OPJ_BYTE        * p_data,
 OPJ_UINT32      * p_data_written,
 opj_event_mgr_t * p_manager
);

/**
 * Gets the maximum size taken by a coc.
 *
 * @param       p_j2k   the jpeg2000 codec to use.
 */
static OPJ_UINT32 opj_j2k_get_max_coc_size(opj_j2k_t *p_j2k);

/**
 * Reads a COC marker (Coding Style Component)
 * @param       p_header_data   the data contained in the COC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the COC marker.
 * @param       p_manager               the user event manager.
*/
static OPJ_BOOL opj_j2k_read_coc(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Reads a QCD marker (Quantization defaults)
 * @param       p_header_data   the data contained in the QCD box.
 * @param       p_header_size   the size of the data contained in the QCD marker.
*/
static OPJ_BOOL opj_j2k_read_qcd(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Compare QCC markers (quantization component)
 *
 * @param       p_first_comp_no       the index of the first component to compare.
 * @param       p_second_comp_no      the index of the second component to compare.
 *
 * @return OPJ_TRUE if equals.
 */
static OPJ_BOOL opj_j2k_compare_qcc(
 opj_j2k_t  * p_j2k,
 OPJ_UINT32   p_first_comp_no,
 OPJ_UINT32   p_second_comp_no
);

/**
 * Writes the QCC marker (quantization component)
 *
 * @param       p_comp_no       the index of the component to output.
*/
static OPJ_BOOL opj_j2k_write_qcc(
 opj_j2k_t            * p_j2k,
 OPJ_UINT32             p_comp_no,
 opj_stream_private_t * p_stream,
 opj_event_mgr_t      * p_manager
);

/**
 * Writes the QCC marker (quantization component)
 *
 * @param       p_comp_no       the index of the component to output.
 * @param       p_data_written  the stream to write data to.
*/
static void opj_j2k_write_qcc_in_memory(
 opj_j2k_t       * p_j2k,
 OPJ_UINT32        p_comp_no,
 OPJ_BYTE        * p_data,
 OPJ_UINT32      * p_data_written,
 opj_event_mgr_t * p_manager
);

/**
 * Gets the maximum size taken by a qcc.
 */
static OPJ_UINT32 opj_j2k_get_max_qcc_size(opj_j2k_t *p_j2k);

/**
 * Reads a QCC marker (Quantization component)
 * @param       p_header_data   the data contained in the QCC box.
 * @param       p_header_size   the size of the data contained in the QCC marker.
*/
static OPJ_BOOL opj_j2k_read_qcc(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);


/**
 * Writes the POC marker (Progression Order Change)
 *
 * @param       data         FIXME DOC
 * @param       data_written the stream to write data to.
 */
static void opj_j2k_write_poc_in_memory(
 opj_j2k_t       * j2k,
 OPJ_BYTE        * data,
 OPJ_UINT32      * data_written
);

/**
 * Gets the maximum size taken by the writing of a POC.
 */
static OPJ_UINT32 opj_j2k_get_max_poc_size(opj_j2k_t *p_j2k);

/**
 * Reads a POC marker (Progression Order Change)
 *
 * @param       p_header_data   the data contained in the POC box.
 * @param       p_header_size   the size of the data contained in the POC marker.
*/
static OPJ_BOOL opj_j2k_read_poc(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Gets the maximum size taken by the toc headers of all the tile parts of any given tile.
 */
static OPJ_UINT32 opj_j2k_get_max_toc_size(opj_j2k_t *p_j2k);

/**
 * Gets the maximum size taken by the headers of the SOT.
 *
 * @param       p_j2k   the jpeg2000 codec to use.
 */
static OPJ_UINT32 opj_j2k_get_specific_header_sizes(opj_j2k_t *p_j2k);

/**
 * Reads a CRG marker (Component registration)
 *
 * @param       p_header_data   the data contained in the TLM box.
 * @param       p_header_size   the size of the data contained in the TLM marker.
*/
static OPJ_BOOL opj_j2k_read_crg(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);
/**
 * Reads a TLM marker (Tile Length Marker)
 *
 * @param       p_header_data   the data contained in the TLM box.
 * @param       p_header_size   the size of the data contained in the TLM marker.
*/
static OPJ_BOOL opj_j2k_read_tlm(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);


/**
 * Reads a PLM marker (Packet length, main header marker)
 *
 * @param       p_header_data   the data contained in the TLM box.
 * @param       p_header_size   the size of the data contained in the TLM marker.
 */
static OPJ_BOOL opj_j2k_read_plm(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Reads a PLT marker (Packet length, tile-part header)
 *
 * @param       p_header_data   the data contained in the PLT box.
 * @param       p_header_size   the size of the data contained in the PLT marker.
*/
static OPJ_BOOL opj_j2k_read_plt(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Reads a PPM marker (Packed headers, main header)
 *
 * @param       p_header_data   the data contained in the POC box.
 * @param       p_header_size   the size of the data contained in the POC marker.
 */

static OPJ_BOOL opj_j2k_read_ppm(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Merges all PPM markers read (Packed headers, main header)
 *
 * @param       p_cp      main coding parameters.
  */
static OPJ_BOOL opj_j2k_merge_ppm(opj_cp_t *p_cp, opj_event_mgr_t * p_manager);

/**
 * Reads a PPT marker (Packed packet headers, tile-part header)
 *
 * @param       p_header_data   the data contained in the PPT box.
 * @param       p_header_size   the size of the data contained in the PPT marker.
*/
static OPJ_BOOL opj_j2k_read_ppt(
 opj_j2k_t *p_j2k,
 OPJ_BYTE * p_header_data,
 OPJ_UINT32 p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Merges all PPT markers read (Packed headers, tile-part header)
 *
 * @param       p_tcp   the tile.
 */
static OPJ_BOOL opj_j2k_merge_ppt(
 opj_tcp_t       * p_tcp,
 opj_event_mgr_t * p_manager
);


/**
 * Writes the SOT marker (Start of tile-part)
 *
 * @param       j2k             J2K codec.
 * @param       data            Output buffer
 * @param       total_data_size Output buffer size
 * @param       data_written    Number of bytes written into stream
 * @param       stream          the stream to write data to.
 * @param       manager         the user event manager.
*/
static OPJ_BOOL opj_j2k_write_sot(
 opj_j2k_t *j2k,
 OPJ_BYTE * data,
 OPJ_UINT32 total_data_size,
 OPJ_UINT32 * data_written,
 const opj_stream_private_t *stream
);

/**
 * Reads values from a SOT marker (Start of tile-part)
 *
 * the j2k decoder state is not affected. No side effects, no checks except for p_header_size.
 *
 * @param       p_header_data   the data contained in the SOT marker.
 * @param       p_header_size   the size of the data contained in the SOT marker.
 * @param       p_tile_no       Isot.
 * @param       p_tot_len       Psot.
 * @param       p_current_part  TPsot.
 * @param       p_num_parts     TNsot.
 */
static OPJ_BOOL opj_j2k_get_sot_values(
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 OPJ_UINT32      * p_tile_no,
 OPJ_UINT32      * p_tot_len,
 OPJ_UINT32      * p_current_part,
 OPJ_UINT32      * p_num_parts,
 opj_event_mgr_t * p_manager
);

/**
 * Reads a SOT marker (Start of tile-part)
 *
 * @param       p_header_data   the data contained in the SOT marker.
 * @param       p_header_size   the size of the data contained in the PPT marker.
*/
static OPJ_BOOL opj_j2k_read_sot(
 opj_j2k_t       * p_j2k,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);

/**
 * Writes the SOD marker (Start of data)
 *
 * This also writes optional PLT markers (before SOD)
*/
static OPJ_BOOL opj_j2k_write_sod(
 opj_j2k_t                  * p_j2k,
 opj_tcd_t                  * p_tile_coder,
 OPJ_BYTE                   * p_data,
 OPJ_UINT32                 * p_data_written,
 OPJ_UINT32                   total_data_size,
 const opj_stream_private_t * p_stream
);


/*static void jp2_write_url(opj_cio_t *cio, char *Idx_file);*/
/*
//Reads a IHDR box - Image Header box
//@param   p_image_header_data         pointer to actual data (already read from file)
//@param   p_image_header_size         the size of the image header
static OPJ_BOOL opj_jp2_read_ihdr(
 opj_jp2_t       * jp2,
 OPJ_BYTE        * p_image_header_data,
 OPJ_UINT32        p_image_header_size,
 opj_event_mgr_t * p_manager
);
//@param p_nb_bytes_written    pointer to store the nb of bytes written by the function.
static OPJ_BYTE * opj_jp2_write_ihdr(
 opj_jp2_t  * jp2,
 OPJ_UINT32 * p_nb_bytes_written);
//@param   p_nb_bytes_written      pointer to store the nb of bytes written by the function.
static OPJ_BYTE * opj_jp2_write_bpcc(
 opj_jp2_t  * jp2,
 OPJ_UINT32 * p_nb_bytes_written
);
//@param   p_bpc_header_data           pointer to actual data (already read from file)
//@param   p_bpc_header_size           the size of the bpc header
static OPJ_BOOL opj_jp2_read_bpcc(
 opj_jp2_t       * jp2,
 OPJ_BYTE        * p_bpc_header_data,
 OPJ_UINT32        p_bpc_header_size,
 opj_event_mgr_t * p_manager
);
static OPJ_BOOL opj_jp2_read_cdef(
 opj_jp2_t       * jp2,
 OPJ_BYTE        * p_cdef_header_data,
 OPJ_UINT32        p_cdef_header_size,
 opj_event_mgr_t * p_manager
);
//!!! p_manager added by jf
static void opj_jp2_apply_cdef(
 opj_image_t     * image,
 opj_jp2_color_t * color,
 opj_event_mgr_t *p_manager
);
//@param p_nb_bytes_written    pointer to store the nb of bytes written by the function.
static OPJ_BYTE * opj_jp2_write_cdef(
 opj_jp2_t  * jp2,
 OPJ_UINT32 * p_nb_bytes_written
);
//@param p_nb_bytes_written    pointer to store the nb of bytes written by the function.
static OPJ_BYTE * opj_jp2_write_colr(
 opj_jp2_t  * jp2,
 OPJ_UINT32 * p_nb_bytes_written
);
//@param   cio         the stream to write data to.
//@param   jp2         the jpeg2000 file codec.
static OPJ_BOOL opj_jp2_write_ftyp(
 opj_jp2_t            * jp2,
 opj_stream_private_t * cio,
 opj_event_mgr_t      * p_manager
);
//@param   p_header_data   the data contained in the FTYP box.
//@param   p_header_size   the size of the data contained in the FTYP box.
static OPJ_BOOL opj_jp2_read_ftyp(
 opj_jp2_t       * jp2,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);
static OPJ_BOOL opj_jp2_skip_jp2c(
 opj_jp2_t            * jp2,
 opj_stream_private_t * stream,
 opj_event_mgr_t      * p_manager
);
//@param   p_header_data   the data contained in the file header box.
//@param   p_header_size   the size of the data contained in the file header box.
static OPJ_BOOL opj_jp2_read_jp2h(
 opj_jp2_t       * jp2,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);
static OPJ_BOOL opj_jp2_write_jp2h(
 opj_jp2_t            * jp2,
 opj_stream_private_t * stream,
 opj_event_mgr_t      * p_manager
);
//@param   cio         the stream to write data to.
static OPJ_BOOL opj_jp2_write_jp2c(
 opj_jp2_t            * jp2,
 opj_stream_private_t * cio,
 opj_event_mgr_t      * p_manager
);
//@param   p_header_data   the data contained in the signature box.
//@param   p_header_size   the size of the data contained in the signature box.
static OPJ_BOOL opj_jp2_read_jp(
 opj_jp2_t       * jp2,
 OPJ_BYTE        * p_header_data,
 OPJ_UINT32        p_header_size,
 opj_event_mgr_t * p_manager
);
//@param cio the stream to write data to.
static OPJ_BOOL opj_jp2_write_jp(
 opj_jp2_t            * jp2,
 opj_stream_private_t * cio,
 opj_event_mgr_t      * p_manager
);
//Apply collected palette data
//@param image Image.
//@param color Collector for profile, cdef and pclr data.
static OPJ_BOOL opj_jp2_apply_pclr(
 opj_image_t     * image,
 opj_jp2_color_t * color,
 opj_event_mgr_t * p_manager
);
static void opj_jp2_free_pclr(opj_jp2_color_t *color);
//Collect palette data
static OPJ_BOOL opj_jp2_read_pclr(
 opj_jp2_t       * jp2,
 OPJ_BYTE        * p_pclr_header_data,
 OPJ_UINT32        p_pclr_header_size,
 opj_event_mgr_t * p_manager
);
//Collect component mapping data
//@param jp2                 JP2 handle
static OPJ_BOOL opj_jp2_read_cmap(
 opj_jp2_t       * jp2,
 OPJ_BYTE        * p_cmap_header_data,
 OPJ_UINT32        p_cmap_header_size,
 opj_event_mgr_t * p_manager
);
//@param   p_colr_header_data          pointer to actual data (already read from file)
//@param   p_colr_header_size          the size of the color header
static OPJ_BOOL opj_jp2_read_colr(
 opj_jp2_t       * jp2,
 OPJ_BYTE        * p_colr_header_data,
 OPJ_UINT32        p_colr_header_size,
 opj_event_mgr_t * p_manager
);
//Sets up the procedures to do on writing header after the codestream.
static OPJ_BOOL opj_jp2_setup_end_header_writing(
 opj_jp2_t       * jp2,
 opj_event_mgr_t * p_manager
);
//Sets up the procedures to do on reading header after the codestream.
static OPJ_BOOL opj_jp2_setup_end_header_reading(
 opj_jp2_t       * jp2,
 opj_event_mgr_t * p_manager
);
static OPJ_BOOL opj_jp2_read_header_procedure(
 opj_jp2_t            * jp2,
 opj_stream_private_t * stream,
 opj_event_mgr_t      * p_manager
);
static OPJ_BOOL opj_jp2_exec(
 opj_jp2_t            * jp2,
 opj_procedure_list_t * p_procedure_list,
 opj_stream_private_t * stream,
 opj_event_mgr_t      * p_manager
);
//@param   cio                     the input stream to read data from.
//@param   box                     the box structure to fill.
//@param   p_number_bytes_read     pointer to an int that will store the number of bytes read from the stream (shoul usually be 2).
static OPJ_BOOL opj_jp2_read_boxhdr(
 opj_jp2_box_t        * box,
 OPJ_UINT32           * p_number_bytes_read,
 opj_stream_private_t * cio,
 opj_event_mgr_t      * p_manager
);
//Sets up the validation ,i.e. adds the procedures to launch to make sure the codec parameters
//are valid. Developers wanting to extend the library can add their own validation procedures.
static OPJ_BOOL opj_jp2_setup_encoding_validation(
 opj_jp2_t       * jp2,
 opj_event_mgr_t * p_manager
);
static OPJ_BOOL opj_jp2_setup_header_writing(
 opj_jp2_t       * jp2,
 opj_event_mgr_t * p_manager
);
static OPJ_BOOL opj_jp2_default_validation(
 opj_jp2_t            * jp2,
 opj_stream_private_t * cio,
 opj_event_mgr_t      * p_manager
);
//Finds the image execution function related to the given box id.
//@param   p_id    the id of the handler to fetch.
//@return  the given handler or NULL if it could not be found.
static const opj_jp2_header_handler_t * opj_jp2_img_find_handler(OPJ_UINT32 p_id);
static const opj_jp2_header_handler_t * opj_jp2_find_handler(OPJ_UINT32 p_id);
//param   box                     the box structure to fill.
//@param   p_number_bytes_read     pointer to an int that will store the number of bytes read from the stream (shoul usually be 2).
//@param   p_box_max_size          the maximum number of bytes in the box.
static OPJ_BOOL opj_jp2_read_boxhdr_char(
 opj_jp2_box_t   * box,
 OPJ_BYTE        * p_data,
 OPJ_UINT32      * p_number_bytes_read,
 OPJ_UINT32        p_box_max_size,
 opj_event_mgr_t * p_manager
);
//Sets up the validation ,i.e. adds the procedures to launch to make sure the codec parameters are valid.
static OPJ_BOOL opj_jp2_setup_decoding_validation(
 opj_jp2_t       * jp2,
 opj_event_mgr_t * p_manager
);
static OPJ_BOOL opj_jp2_setup_header_reading(
 opj_jp2_t *jp2,
 opj_event_mgr_t * p_manager
);
*/

#endif /* OPJ_J2K_H */
