#include <stdio.h>
#include "../include/server.h"

/**
 * the main function of our application server
 */
int main(){
    printf("%d", startserver(3000));
    return 0;
}
