//
// Created by utku on 17.04.2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "dma.h"

float time_diff(struct timeval *start, struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}

int main() {
    struct timeval start;
    struct timeval end;
    void *p1;
    void *p2;
    void *p3;
    void *p4;
    int ret;
    ret = dma_init(20);
    if (ret != 0) {
        printf ("something was wrong\n");
        exit(1);
    }

    dma_print_bitmap();

    p1 = dma_alloc(512);
    dma_print_bitmap();
    p2 = dma_alloc(1024);
    dma_print_bitmap();
    p3 = dma_alloc(2048);
    dma_print_bitmap();
    p4 = dma_alloc(4096);
    dma_print_bitmap();
    gettimeofday(&start, NULL);
    dma_free(p3);
    gettimeofday(&end, NULL);



    dma_print_bitmap();

    printf("%p\n",p1);
    printf("%p\n",p2);
    printf("%p\n",p3);
    printf("%p\n",p4);
    printf("%d\n", dma_give_intfrag());
    dma_print_blocks();
    printf("time spent: %0.8f sec\n", time_diff(&start, &end));

    return 0;
}

