//
//  opj_compress.h
//  dicm2dckv
//
//  Created by jacquesfauquex on 2025-01-29.
//

#ifndef opj_compress_h
#define opj_compress_h
#include "dckvtypes.h"

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
);

#endif /* opj_compress_h */


/* compression parameters */
//opj_set_default_encoder_parameters(&parameters);
/*
Set encoding parameters to default values, that means :
<ul>
    <li>Lossless
    <li>1 tile
<li>Size of precinct : 2^15 x 2^15 (means 1 precinct)
<li>Size of code-block : 64 x 64
    <li>Number of resolutions: 6
<li>No SOP marker in the codestream
<li>No EPH marker in the codestream
    <li>No sub-sampling in x or y direction
<li>No mode switch activated
    <li>Progression order: LRCP
<li>No index file
<li>No ROI upshifted
<li>No offset of the origin of the image
<li>No offset of the origin of the tiles
<li>Reversible DWT 5-3
</ul>
@param parameters Compression parameters
*/
