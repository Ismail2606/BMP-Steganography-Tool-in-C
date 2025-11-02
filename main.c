#include "encode.h"
#include "types.h"
#include "decode.h"

int main(int argc,char * argv[]){
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    uint img_size;
    if(check_operation_type(argv[1]) == e_encode){
        if(read_and_validate_encode_args(argv, &encInfo) == e_failure){
            printf("Error: Validation of encode arguments failed\n");
            return e_failure;
        }
        if(do_encoding(&encInfo)==e_failure){
            printf("Error: Encoding failed\n");
            return e_failure;
        }  
        else
            printf("\n---Encoding done successfully---\n\n");
    }
    else if(check_operation_type(argv[1]) == e_decode){
        if(read_and_validate_decode_args(argv, &decInfo) == e_failure){
            printf("Error: Validation of decode arguments failed\n");
            return e_failure;
        }
        if(do_decoding(&decInfo,&encInfo)==e_failure){
            printf("Error: Decoding failed\n");
            return e_failure;
        }
        else
            printf("\n---Decoding done successfully---\n\n");
    } 
    return 0;
}
