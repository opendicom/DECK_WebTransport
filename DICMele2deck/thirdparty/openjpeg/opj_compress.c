#include "opj_compress.h"
#include "opj_includes.h"

extern char *DICMbuf;
bool opj_cfho(
          u16  photocode,
          u16  spp,
          u16  rows,
          u16  cols,
          u16  stored,
          u64  dkvs,//source idx
          u64  dkvc,//compressed idx
          u64 *dkvf,//fast idx
          u64 *dkvh,//high idx
          u64 *dkvo,//original idx
          u64 *dkvz//end idx
)
{
#pragma mark parameters coded
   opj_cparameters_t parameters;
   //memset(parameters, 0, sizeof(opj_cparameters_t));
   parameters.rsiz             = 0 ;//PJ_PROFILE_NONE no profile, conform to 15444-1O (Lossless)
   parameters.numresolution    = 6 ;//OPJ_COMP_PARAM_DEFAULT_NUMRESOLUTION
   parameters.cblockw_init     = 64;//OPJ_COMP_PARAM_DEFAULT_CBLOCKW 4,8,16,32,64,128,256,512,1024
   parameters.cblockh_init     = 64;//OPJ_COMP_PARAM_DEFAULT_CBLOCKH 4,8,16,32,64,128,256,512,1024
   parameters.prog_order       = 0 ;// OPJ_LRCP layer-resolution-component-precinct order
   parameters.roi_compno       = -1;//no ROI
   parameters.subsampling_dx   = 1 ;//No sub-sampling in x direction
   parameters.subsampling_dy   = 1 ;//No sub-sampling in y direction
   parameters.tp_on            = 0 ;//Tile part generation
   parameters.cod_format       = 0 ;//¿-1? output file format 0: J2K, 1: JP2, 2: JPT
   parameters.tcp_rates[0]     = 0 ;//[100];//rates of layers (Should be decreasing).1 can be used as last value to indicate the last layer is lossless
   parameters.tcp_numlayers    = 0 ;//number of layers
   parameters.cp_disto_alloc   = 0 ;//allocation by rate/distortion
   parameters.cp_fixed_alloc   = 0 ;//allocation by fixed layer
   parameters.cp_fixed_quality = 0 ;//allocation by fixed quality (PSNR)
   parameters.jpip_on          = 0 ;//OPJ_FALSE

   
#pragma mark - parameters image
   
   parameters.tcp_mct = (spp >= 3) ? 1 : 0;

#pragma mark - opj_image_create
   opj_image_t *image = NULL;//bitmap data
   image = (opj_image_t*) calloc(1, sizeof(opj_image_t));
   image->x0 = 0;
   image->y0 = 0;
   image->x1 = cols - 1;
   image->y1 = rows - 1;
   
   image->numcomps = (OPJ_UINT32)spp;
   image->comps = (opj_image_comp_t*)calloc(image->numcomps, sizeof(opj_image_comp_t));
   size_t compsize=(dkvc-dkvs)/spp;
   for (OPJ_UINT32 compno = 0; compno < spp; compno++)
   {
       image->comps[compno].w    = (OPJ_UINT32)cols;
       image->comps[compno].h    = (OPJ_UINT32)rows;
       image->comps[compno].prec = (OPJ_UINT32)stored;
       //image->comps[compno].sgnd = (OPJ_UINT32)pixrep;// 0=unsigned 1=signed
       image->comps[compno].data =(OPJ_INT32*)(DICMbuf+dkvs+(compsize*compno));
   }


   /** color space: sRGB, Greyscale or YUV */
#pragma mark TODO create equivalence
   image->color_space = photocode;//2 OPJ_CLRSPC_GRAY = 2; 3 OPJ_CLRSPC_SYCC (YUV); 1 OPJ_CLRSPC_SRGB; 4 OPJ_CLRSPC_EYCC (e-YCC); 5 OPJ_CLRSPC_CMYK; 0 OPJ_CLRSPC_UNSPECIFIED; -1 OPJ_CLRSPC_UNKNOWN = -1;
   image->icc_profile_buf=NULL;
   image->icc_profile_len=0;
   /** 'restricted' ICC profile */
   //OPJ_BYTE *icc_profile_buf;
   /** size of ICC profile */
   //OPJ_UINT32 icc_profile_len;

   

#pragma mark - encode
//Get a decoder handle for parameters.cod_format J2K_CFMT:JPEG-2000 codestream
//format_defs.h defines J2K_CFMT 0

    
#pragma mark openjpeg.opj_create_compress(OPJ_CODEC_J2K) [start]
   //opj_codec_t* l_codec = 00;//void* (pointer only), structured privately
   //l_codec = opj_create_compress(OPJ_CODEC_J2K);
   //opj_codec_t* OPJ_CALLCONV opj_create_compress(OPJ_CODEC_FORMAT p_format){
   //l_codec
 opj_codec_private_t *l_codec = 00;//pointer
 l_codec = (opj_codec_private_t*)calloc(1, sizeof(opj_codec_private_t));//l_codec buffer
 
//l_codec->m_codec_data.m_compression (methods to be used)
 l_codec->m_codec_data.m_compression.opj_encode =
   (OPJ_BOOL(*)(void *, struct opj_stream_private *, struct opj_event_mgr *)) opj_j2k_encode;
 l_codec->m_codec_data.m_compression.opj_end_compress =
   (OPJ_BOOL(*)(void *, struct opj_stream_private *, struct opj_event_mgr *))opj_j2k_end_compress;
 l_codec->m_codec_data.m_compression.opj_start_compress =
   (OPJ_BOOL(*)(void *, struct opj_stream_private *, struct opj_image *, struct opj_event_mgr *)) opj_j2k_start_compress;
 l_codec->m_codec_data.m_compression.opj_write_tile =
   (OPJ_BOOL(*)(void *, OPJ_UINT32, OPJ_BYTE*, OPJ_UINT32, struct opj_stream_private *, struct opj_event_mgr *)) opj_j2k_write_tile;
 l_codec->m_codec_data.m_compression.opj_destroy = (void (*)( void *)) opj_j2k_destroy;
 l_codec->m_codec_data.m_compression.opj_setup_encoder =
   (OPJ_BOOL(*)(void *, opj_cparameters_t *, struct opj_image *, struct opj_event_mgr *)) opj_j2k_setup_encoder;

//threads
 l_codec->opj_set_threads = (OPJ_BOOL(*)(void * p_codec, OPJ_UINT32 num_threads)) opj_j2k_set_threads;

   
   
//create m_codec j2k.opj_j2k_create_compress()
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
 
   

 //thread pool
 int num_threads=1;//JF opj_get_num_cpus();
 l_j2k->m_tp = opj_thread_pool_create(num_threads);
 
 

   
   if (!l_j2k->m_tp) l_j2k->m_tp = opj_thread_pool_create(0);
   if (!l_j2k->m_tp) {opj_j2k_destroy(l_j2k);return NULL;}

   
   
   
   
   
   
 if (!l_j2k->m_tp) l_j2k->m_tp = opj_thread_pool_create(0);
 if (!l_j2k->m_tp) {opj_j2k_destroy(l_j2k);return NULL;}

 l_codec->m_codec = l_j2k;
 if (! l_codec->m_codec) { free(l_codec); return 00;}

//event manager
 opj_set_default_event_handler(&(l_codec->m_event_mgr));

#pragma mark openjpeg.opj_create_compress(OPJ_CODEC_J2K) [end]

      
#pragma mark (1) openjpeg.opj_setup_encoder(l_codec, &parameters, image)) [start]
   //l_codec->m_codec_data.m_compression
 if (!opj_j2k_setup_encoder(l_codec->m_codec,&parameters,image,&(l_codec->m_event_mgr)))
 {
    fprintf(stderr, "failed to encode image: opj_setup_encoder\n");
    opj_j2k_destroy(l_j2k);//was opj_destroy_codec(l_codec);
      opj_image_destroy(image);
    if (parameters.cp_comment)    free(parameters.cp_comment);
    if (parameters.cp_matrice)    free(parameters.cp_matrice);
    return false;
 }
#pragma mark (1) openjpeg.opj_setup_encoder(l_codec, &parameters, image)) [stop]
   

#pragma mark stdout
   opj_stream_t *l_stream = 00;//type void *
//JF overwrite of stream methods to keep the data in a buffer
   l_stream = opj_stream_create_default_file_stream(parameters.outfile, OPJ_FALSE);//OPJ_FALSE = write stream
   if (! l_stream) {
      fprintf(stderr, "cannot create %s\n", parameters.outfile);
      opj_j2k_destroy(l_j2k);//was opj_destroy_codec(l_codec);
      opj_image_destroy(image);
      if (parameters.cp_comment)    free(parameters.cp_comment);
      if (parameters.cp_matrice)    free(parameters.cp_matrice);
      return false;
   }

#pragma mark (2) (3) (4) encode
//JF l_stream referred to in each operation
   
   if (!opj_j2k_start_compress(
                           l_codec->m_codec,
                           l_stream,
                           image,
                           &(l_codec->m_event_mgr)
                          ))
   {
      fprintf(stderr, "cannot compress %s\n", parameters.outfile);
      opj_j2k_destroy(l_j2k);//was opj_destroy_codec(l_codec);
      opj_image_destroy(image);
      if (parameters.cp_comment)    free(parameters.cp_comment);
      if (parameters.cp_matrice)    free(parameters.cp_matrice);
      return false;

   }
   if (!opj_j2k_encode(l_codec,      l_stream))
   {
      fprintf(stderr, "cannot encode %s\n", parameters.outfile);
      opj_j2k_destroy(l_j2k);//was opj_destroy_codec(l_codec);
      opj_image_destroy(image);
      if (parameters.cp_comment)    free(parameters.cp_comment);
      if (parameters.cp_matrice)    free(parameters.cp_matrice);
      return false;

   }
   if (!opj_end_compress(l_codec,      l_stream))
   {
      fprintf(stderr, "cannot end compress %s\n", parameters.outfile);
      opj_j2k_destroy(l_j2k);//was opj_destroy_codec(l_codec);
      opj_image_destroy(image);
      if (parameters.cp_comment)    free(parameters.cp_comment);
      if (parameters.cp_matrice)    free(parameters.cp_matrice);
      return false;
   }
   
#pragma mark - (5) destroy
   opj_stream_destroy(l_stream);
   opj_destroy_codec(l_codec);//free remaining compression structures
   opj_image_destroy(image);//free image data

   
#pragma mark free
fin:
    if (parameters.cp_comment)    free(parameters.cp_comment);
    if (parameters.cp_matrice)    free(parameters.cp_matrice);
#pragma mark TODO freeing
    /*if (raw_cp.rawComps)          free(raw_cp.rawComps);
    if (img_fol.imgdirpath)       free(img_fol.imgdirpath);
    if (dirptr) {
        if (dirptr->filename_buf) free(dirptr->filename_buf);
        if (dirptr->filename)     free(dirptr->filename);
        free(dirptr);
    }
     */
    return true;
}
