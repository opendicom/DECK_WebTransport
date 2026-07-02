#!/usr/bin/env bash

# stdin | uidb64 | stdout
# Created by jacquesfauquex on 2026-05-01.

# converts uid into its reduced b64 squeeze
# =========================================
# - each character is transformed in half byte
# - the resulting stream is then transformed in a
# special base64, friendly to urls
# (which does not contain the reserved chars / or =)

b64=( - 0 1 2 3 4 5 6 7 8 9 A B C D E F G H I J K L M N O P Q R S T U V W X Y Z _ a b c d e f g h i j k l m n o p q r s t u v w x y z )
function hb2b64 {
local -n hb=$1 # use nameref for indirection

# padding with .. in case of hb%3 != 0
hb+=(0x0A)
hb+=(0x0A)
#echo "${hb[@]}"
# 3 half bytes transformed into 2 base64 chars
let "hbCount=(( ${#hb[@]} / 3 ) *3)"
for (( j=0; j<$hbCount; j+=3 ))
do
# "aaaa bb""bb cccc"
echo -n "${b64[$(( (${hb[$j]} << 2) + (${hb[$j+1]} >> 2) ))]}""${b64[$((( (${hb[$j+1]} & 0x03) << 4) + ${hb[$j+2]} ))]}"
done
hb=()
}


# main

#hb=half byte (0-15) 10 digits, 3 number attributes .+-, and 3 hl7 v2 separators |^~
declare -A ascii2halfbyte=( [0]=0x00 [1]=0x01 [2]=0x02 [3]=0x03 [4]=0x04 [5]=0x05 [6]=0x06 [7]=0x07 [8]=0x08 [9]=0x09 [.]=0x0A [+]=0x0B [-]=0x0C [|]=0x0D [^]=0x0E [~]=0x0F )
while read line; do
for word in $line; do
wordLength=${#word}
halfbytes=() # partial result (before base64)
for (( i=0; i<${#word}; i++ )); do
curascii=${word:$i:1}
halfbytes+=(${ascii2halfbyte[$curascii]})
done
[[ ${#halfbytes[@]} -gt 0 ]] && hb2b64 halfbytes
done #word
done #line
exit 0