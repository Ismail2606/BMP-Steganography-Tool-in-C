#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types
#include "encode.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    uint magic_size;
    uint extn_size;
    uint data_size;
    char magic_string[50];
    char extn[10];
    char data[100];

} DecodeInfo;

/* Decoding function prototype */
/* Read and validate Encode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the encoding */
Status do_decoding(DecodeInfo *decInfo, EncodeInfo *encInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);
 
Status skip_header(FILE *fptr_stego_image);
/*get magic string size*/
Status decode_magic_string_size(DecodeInfo *decInfo);

/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo);
/*encode int size */
/*encode secret file extn size*/
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Encode secret file extenstion */
Status decode_secret_file_extn( DecodeInfo *decInfo);

/* Encode secret file size */
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo);

/* Encode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Encode a byte into LSB of image data array */
char decode_byte_from_lsb(char *image_buffer);

/*Encode a size into lsb*/
int decode_size_from_lsb(char *image_buffer);

#endif
