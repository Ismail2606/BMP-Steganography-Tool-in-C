#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include<stdarg.h>
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

OperationType check_operation_type(char *arg)
{
    if (strcmp(arg, "-e") == 0)
        return e_encode;
    else if (strcmp(arg, "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}

Status  read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname = argv[3];
    
    if (!strstr(encInfo->src_image_fname, ".bmp"))
    {
        printf("Error: Source file is not a .bmp file\n");
        return e_failure;
    }

    if (!strstr(encInfo->secret_fname, ".txt"))
    {
        printf("Error: Secret file is not a .txt file\n");
        return e_failure;
    }
    
    if(argv[4]!= NULL)
    {
        encInfo->stego_image_fname = argv[4];
        if (!strstr(encInfo->stego_image_fname, ".bmp"))
        {
            printf("Error: Stego image file is not a .bmp file\n");
            return e_failure;
        }
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }

    strcpy(encInfo->extn_secret_file ,strchr(encInfo->secret_fname, '.'));

    printf("All files are validated\n");
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }
    // No failure return e_success
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) != e_success) 
        return e_failure;
    else
        printf("Files are opened successfully\n");

    printf("Enter magic string: ");
    scanf(" %[^\n]",encInfo->magic_string);
    if(check_capacity(encInfo)!= e_success)
        return e_failure;
    else
        printf("check capacity done successfully\n");
 
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) != e_success)
    {
        return e_failure;
    }
    else
        printf("Copy header file successfully\n");

    if(encode_magic_string_size(strlen(encInfo->magic_string), encInfo)!= e_success)
        return e_failure;
    else
        printf("Encode magic string size done successfully\n");

    if(encode_magic_string(encInfo->magic_string, encInfo)!= e_success)
        return e_failure;
    else
        printf("Encode magic string done successfully\n");

    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo)!= e_success)
        return e_failure;
    else
        printf("Encode extn size done successfully\n");

    if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo)!= e_success)
        return e_failure;
    else
        printf("Encode extn done successfully\n");

    if(encode_secret_file_size(encInfo->size_secret_file, encInfo)!= e_success)
        return e_failure;
    else
        printf("Encode secret file size done successfully\n");

    if(encode_secret_file_data(encInfo)!= e_success)
        return e_failure;
    else
        printf("Encode secret file data done successfully\n");

    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)!= e_success)
        return e_failure;
    else
        printf("Remaining data copied successfully\n");

    return e_success;
}

Status  check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // Calculate total bits needed:
    int required_capacity =
        strlen(encInfo->magic_string) * 8 +           // Magic string
        32 +                                 // Magic string size
        32 +                                 // Extension size
        strlen(encInfo->extn_secret_file) * 8 + // Extension characters
        32 +                                 // Secret file size
        encInfo->size_secret_file * 8;       // Secret file data

    // Check if image has enough capacity
    if (required_capacity > encInfo->image_capacity)
    {
        printf("ERROR: Insufficient image capacity (%d needed, %u available)\n",
               required_capacity, encInfo->image_capacity);
        return e_failure;
    }

    return e_success;
}

uint get_file_size(FILE *fptr)
{
    long curr_pos = ftell(fptr);           // Save current position
    fseek(fptr, 0, SEEK_END);              // Move to end
    long file_size = ftell(fptr);          // Get size
    fseek(fptr, curr_pos, SEEK_SET);       // Restore pointer
    return (uint)file_size;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);

    // Create buffer of 54 bytes (BMP header)
    char header[54];

    //Read 54 bytes from source
    if (fread(header, 54, 1, fptr_src_image) != 1)
    {
        printf("ERROR: Failed to read BMP header\n");
        return e_failure;
    }

    //Write 54 bytes to destination
    if (fwrite(header, 54, 1, fptr_dest_image) != 1)
    {
        printf("ERROR: Failed to write BMP header to stego image\n");
        return e_failure;
    }
    printf("Copy bmp header successfully\n");
    return e_success;
}
Status encode_magic_string_size(int MG_size, EncodeInfo *encInfo)
{
    char buffer[32];

    // Read 32 bytes from source image file
    if (fread(buffer, 32, 1, encInfo->fptr_src_image) != 1)
    {
        printf("Error: Unable to read 32 bytes for encoding magic string size.\n");
        return e_failure;
    }

    if (encode_size_to_lsb(MG_size, buffer) == e_failure)
    {
        printf("Error: Failed to encode magic string size.\n");
        return e_failure;
    }

    // Write encoded buffer to destination image file
    if (fwrite(buffer, 32, 1, encInfo->fptr_stego_image) != 1)
    {
        printf("Error: Failed to write encoded buffer to stego image.\n");
        return e_failure;
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];

    // Loop through each character in the magic string
    for (int i = 0; magic_string[i] != '\0'; i++)
    {
        // Read 8 bytes from source image
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1)
        {
            printf("Error: Unable to read 8 bytes from source image.\n");
            return e_failure;
        }

        // Encode one character of the magic string into the 8 bytes
        if (encode_byte_to_lsb(magic_string[i], buffer) == e_failure)
        {
            printf("Error: Failed to encode magic string character.\n");
            return e_failure;
        }

        // Write encoded bytes to stego image
        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1)
        {
            printf("Error: Unable to write encoded bytes to stego image.\n");
            return e_failure;
        }
    }
    return e_success;
}

Status encode_secret_file_extn_size(long ext_size, EncodeInfo *encInfo)
{
    char buffer[32];

    // Read 32 bytes from source image
    if (fread(buffer, 32, 1, encInfo->fptr_src_image) != 1)
    {
        printf("Error: Failed to read 32 bytes from source image for extension size.\n");
        return e_failure;
    }

    if (encode_size_to_lsb((int)ext_size, buffer) == e_failure)
    {
        printf("Error: Failed to encode file extn size.\n");
        return e_failure;
    }
    
    // Encode 4 bytes (32 bits) into buffer
    if (fwrite(buffer, 32, 1, encInfo->fptr_stego_image) != 1)
    {
        printf("Error: Failed to encode extension size to image.\n");
        return e_failure;
    }
    // printf("Encoding extension size: %ld\n", ext_size);
    return e_success;
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];

    for (int i = 0; file_extn[i] != '\0'; i++)
    {
        // Read 8 bytes from source image
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1)
        {
            printf("Error: Failed to read 8 bytes from source image for extension character.\n");
            return e_failure;
        }

        // Encode one character from the file extension
        if (encode_byte_to_lsb(file_extn[i], buffer) != e_success)
        {
            printf("Error: Failed to encode file extension character.\n");
            return e_failure;
        }

        // Write 8 modified bytes to stego image
        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1)
        {
            printf("Error: Failed to write encoded extension bytes to stego image.\n");
            return e_failure;
        }
    }
    return e_success;
}
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    // Read 32 bytes from source image
    if (fread(buffer, 32, 1, encInfo->fptr_src_image) != 1)
    {
        printf("Error: Failed to read 32 bytes from source image.\n");
        return e_failure;
    }

    // Encode the secret file size (as 4-byte int, 32 bits)
    if (encode_size_to_lsb((int)file_size, buffer) == e_failure)
    {
        printf("Error: Failed to encode secret file size.\n");
        return e_failure;
    }

    // Write 32 bytes to stego image
    if (fwrite(buffer, 32, 1, encInfo->fptr_stego_image) != 1)
    {
        printf("Error: Failed to write 32 bytes to stego image.\n");
        return e_failure;
    }
    return e_success;
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[8];
    char ch;

    while(fread(&ch, 1, 1,encInfo->fptr_secret)==1)
    {
        // Read 8 bytes from source image
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1)
        {
            printf("Error: Failed to read 8 bytes from source image for secret data.\n");
            return e_failure;
        }

        if (encode_byte_to_lsb(ch, buffer) != e_success)
        {
            printf("Error: Failed to encode secret data character.\n");
            return e_failure;
        }

        // Write 8 modified bytes to stego image
        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1)
        {
            printf("Error: Failed to write encoded secret data bytes to stego image.\n");
            return e_failure;
        }
    }
    return e_success;
}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = (data >> (7 - i)) & 1;  // MSB first
        image_buffer[i] = (image_buffer[i] & 0xFE) | bit;
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *image_buffer)
{
    for (int i = 31; i >= 0; i--)
    {
        int bit = (size >> i) & 1;
        image_buffer[31 - i] = (image_buffer[31 - i] & 0xFE) | bit;
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch,1,1,fptr_src)>0)
    {
        fwrite(&ch,1,1,fptr_dest);
    }
    return e_success;
}

