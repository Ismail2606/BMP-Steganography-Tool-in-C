#include <stdio.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include<stdarg.h>
#include "common.h"
#include<stdlib.h>
#include "encode.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    decInfo->stego_image_fname = argv[2];
    
    if (!strstr(decInfo->stego_image_fname, ".bmp"))
    {
        printf("Error: Stego file is not a .bmp file\n");
        return e_failure;
    }
    return e_success;
}
Status do_decoding(DecodeInfo *decInfo, EncodeInfo *encInfo)
{
    if (open_decode_files(decInfo) != e_success) 
        return e_failure;
    else
        printf("Files are opened successfully\n");

    if(skip_header(decInfo->fptr_stego_image)!=e_success)
        return e_failure;
    else
        printf("BMP header skipped successfully\n");

    if(decode_magic_string_size(decInfo)!= e_success)
        return e_failure;
    else
        printf("Decode magic string size done successfully\n");

    if (decode_magic_string(decInfo) != e_success)
        return e_failure;
    else
        printf("Decode magic string done successfully\n");

    char user_magic_string[50];
    printf("Enter expected magic string to validate: ");
    scanf(" %[^\n]", user_magic_string);

    if(strcmp(decInfo->magic_string, user_magic_string) != 0)
    {
        printf("Error: Magic string does not match\n");
        return e_failure;
    }
    else
        printf("Magic string validated successfully\n");

    if (decode_secret_file_extn_size(decInfo) != e_success)
        return e_failure;
    else
        printf("Decode extn size done successfully\n");
    
    if (decode_secret_file_extn( decInfo) != e_success)
        return e_failure;
    else
        printf("Decode extn done successfully\n");

    long file_size;
    if (decode_secret_file_size(&file_size,decInfo) != e_success)
        return e_failure;
    printf("Secret file size: %ld\n", file_size);

    if (decode_secret_file_data(decInfo) != e_success)
        return e_failure;
    printf("Secret data decoded and written to .txt file\n");

    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    
    if (decInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
    	return e_failure;
    }
    return e_success;
}
Status skip_header(FILE *fptr_stego_image)
{
    // printf("Position before decode extn size: %ld\n", ftell(decInfo->fptr_stego_image));

    if(fseek(fptr_stego_image, 54, SEEK_SET) != 0)
    {
        printf("Error: Unable to skip header.\n");
        return e_failure;
    }
    return e_success;
}
Status decode_magic_string_size(DecodeInfo *decInfo)
{   
    // printf("Position before decode magic string size: %ld\n", ftell(decInfo->fptr_stego_image));
    char buffer[32];
    fread(buffer,32,1,decInfo->fptr_stego_image);
    decInfo->magic_size = decode_size_from_lsb(buffer);
    if(decInfo->magic_size == -1)
    {
        printf("Error: Unable to decode size to lsb.\n");
        return e_failure;
    }
    return e_success;
}
int decode_size_from_lsb(char *image_buffer)
{
    int size = 0;
    for (int i = 0; i < 32; i++)
    {
        int bit = image_buffer[i] & 1; 
        size = (size << 1) | bit;
    }
    printf("\nDecoded size: %d\n", size);
    return size;
}
Status decode_magic_string(DecodeInfo *decInfo)
{
    char buffer[8];

    for (int i = 0; i < decInfo->magic_size; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            printf("Error: Unable to read 8 bytes from stego image for magic character %d\n", i);
            return e_failure;
        }

        decInfo->magic_string[i] = decode_byte_from_lsb(buffer);
    }

    decInfo->magic_string[decInfo->magic_size] = '\0';

    return e_success;
}

char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;
    for (int i = 0; i < 8; i++)
    {
        int bit = image_buffer[i] & 1;
        data = (data << 1) | bit; // MSB first
    }
    return data;
}

Status decode_secret_file_extn_size( DecodeInfo *decInfo)
{
    // printf("Position before decode extn size: %ld\n", ftell(decInfo->fptr_stego_image));
    char buffer[32];
    if (fread(buffer, 32, 1, decInfo->fptr_stego_image) != 1)
    {
        printf("Error: Unable to read 32 bytes for extension size.\n");
        return e_failure;
    }
    decInfo->extn_size = decode_size_from_lsb(buffer);
    return e_success;
}
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];
    char file_extn[decInfo->extn_size + 1];
    // printf("extn size: %u\n",decInfo->extn_size );
    for (int i = 0; i < decInfo->extn_size; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            printf("Error: Could not read 8 bytes for extension char\n");
            return e_failure;
        }
        file_extn[i] = decode_byte_from_lsb(buffer);
    }
    file_extn[decInfo->extn_size] = '\0';
    strcpy(decInfo->extn, file_extn);
    return e_success;
}
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo)
{
    char buffer[32];

    if (fread(buffer, 32, 1, decInfo->fptr_stego_image) != 1)
    {
        printf("Error: Could not read 32 bytes for secret file size\n");
        return e_failure;
    }

    int size = decode_size_from_lsb(buffer);
    decInfo->data_size = size;
    *file_size = size;
    
    return e_success;
}
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;
    FILE *fptr_output = fopen("output.txt", "w");

    if (fptr_output == NULL)
    {
        printf("Error: Unable to open output.txt for writing\n");
        return e_failure;
    }

    for (int i = 0; i < decInfo->data_size; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            printf("Error: Failed to read 8 bytes for data byte\n");
            fclose(fptr_output);
            return e_failure;
        }
        ch = decode_byte_from_lsb(buffer);
        fputc(ch, fptr_output);
    }

    fclose(fptr_output);
    return e_success;
}

