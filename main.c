
#include "src/symbolic4.h"

int main(int argc, const char * argv[]) {
    
    char buffer[500];
    char query[150];
    
    if (argc == 1) {
        while (true) {
            printf("Query:\n");
            scanf("%s", query);
            if (symbolic4(buffer, query) == RETS_SUCCESS) {
                printf("%s\n\n", buffer);
            } else {
                printf("ERROR\n\n");
            }
        }
    } else if (argc == 2) {
        if (symbolic4(buffer, (char*) argv[1]) == RETS_SUCCESS) {
            printf("%s", buffer);
            return 0;
        } else {
            printf("ERROR");
            return 1;
        }
    }
    
    return 0;
    
}
