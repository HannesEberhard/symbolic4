
#include <time.h>
#include "../src/symbolic4.h"

int main(int argc, const char * argv[]) {
    
    uint8_t verbose;
    clock_t clock_reference = 0;
    char buffer[50];
    uint8_t status = 0;
    
    uint8_t i;
    
    if (argc != 4) {
        printf("invalid number of arguments\n");
        return 1;
    }
    
    verbose = (uint8_t) argv[1];
    
    if (verbose) {
        printf("%-20s ?= %-20s | ", argv[2], argv[3]);
        clock_reference = clock();
    }
    
    status = symbolic4(buffer, (char*) argv[2]);
    
    if (verbose) {
        
        clock_reference = clock() - clock_reference;
        printf("%10f ms | ", ((float) (1000 * clock_reference)) / CLOCKS_PER_SEC);
        
        if (status == RETS_SUCCESS && strcmp(buffer, argv[3]) == 0) {
            printf("success");
        } else if (status == RETS_SUCCESS) {
            printf("FAILURE/unequal: %s", buffer);
        } else if (status == RETS_ERROR) {
            printf("FAILURE/error: %s", buffer);
        } else {
            printf("UNEXPECTED RESULT");
        }
        
        printf("\n");
        
    }
    
    return 0;
    
}
