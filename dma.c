#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include "dma.h"

#define SetBit(A,k)     ( A[(k/8)] |= (1 << (k%8)) )
#define ClearBit(A,k)   ( A[(k/8)] &= ~(1 << (k%8)) )
#define TestBit(A,k)    ( A[(k/8)] & (1 << (k%8)) )

struct heap {
    pthread_mutex_t heapLock;
    char * memory;
    int size;
    int bitmapSize;
    int bits;
    int intFrag;
}heap;

int dma_init(int m) {
    heap.bitmapSize = pow(2,m-6);
    heap.bits = 8*heap.bitmapSize;
    heap.size = pow(2,m);
    heap.intFrag = 0;
    heap.memory = mmap(NULL, heap.size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if(heap.memory == MAP_FAILED) {
        return -1;
    }
    printf("%d\n", sizeof(*heap.memory));
    ClearBit(heap.memory,0);
    SetBit(heap.memory,1);
    for(int i = 2; i <= heap.bitmapSize/8; i++) {
        ClearBit(heap.memory,i);
    }
    for(int j = heap.bitmapSize/8+1; j < heap.bitmapSize/8+2; j++) {
        SetBit(heap.memory,j);
    }
    for(int j = heap.bitmapSize/8+2; j < heap.bitmapSize/8+32; j++) {
        ClearBit(heap.memory,j);
    }
    for (int i = heap.bitmapSize/8+32; i < heap.bits; ++i) {
        SetBit(heap.memory,i);
    }
    return 0;
}

void *dma_alloc(int size) {
    pthread_mutex_lock(&heap.heapLock);
    if(size % 16 != 0) {
        heap.intFrag += 16 - (size%16);
        size += 16 - (size%16);
    }
    for (int i = 0; i < heap.bits; i += 2) {
        if(TestBit(heap.memory,i) && i < (heap.bits - size/8)) {
            int x = 1;
            for (int j = i; j < i + size/8; j+=2) {
                if(!TestBit(heap.memory,j)) {
                    x = 0;
                }
            }
            if(x == 1) {
                ClearBit(heap.memory,i);
                for (int j = i+2; j < i+size/8; ++j) {
                    ClearBit(heap.memory,j);
                }
                void *temp = heap.memory + i*8;
                pthread_mutex_unlock(&heap.heapLock);
                return temp;
            }
        }
    }
    return NULL;
}

void dma_free(void *p) {
    pthread_mutex_lock(&heap.heapLock);
    int index;
    index = p - (void *)heap.memory;
    int bitIndex = index / 8;
    SetBit(heap.memory,bitIndex);
    bitIndex += 2;
    while(!TestBit(heap.memory,(bitIndex + 1))) {
        SetBit(heap.memory,bitIndex);
        SetBit(heap.memory,(bitIndex+1));
        bitIndex += 2;
    }
    pthread_mutex_unlock(&heap.heapLock);
}

void dma_print_bitmap(){
    pthread_mutex_lock(&heap.heapLock);
    int print_count = 0;
    for (int i = 0; i < heap.bits; ++i) {
        if(TestBit(heap.memory,i)) {
            printf("1");
        }
        else {
            printf("0");
        }
        print_count = print_count + 1;
        if(print_count % 8 == 0 && i != 0)  {
            printf(" ");
        }
        if(print_count % 64 == 0 && i != 0){
            printf("\n");
        }
    }
    printf("%d\n",print_count);
    pthread_mutex_unlock(&heap.heapLock);
}



void dma_print_blocks() {
    pthread_mutex_lock(&heap.heapLock);
    int size;
    for (int i = 0; i < heap.bits; ) {
        if(TestBit(heap.memory,i)) {
            printf("F, %.16p, ",heap.memory+i*8);
            size = 2;
            i += 2;
            while(TestBit(heap.memory,i)) {
                size += 2;
                i += 2;
            }
            printf("0x%x, (%d)\n", size*8, size*8);
        } else {
            printf("A, %.16p, ",heap.memory+i*8);
            size = 2;
            i += 2;
            while(!TestBit(heap.memory,(i+1))) {
                size += 2;
                i += 2;
            }
            printf("0x%x, (%d)\n", size*8, size*8);
        }
    }
    pthread_mutex_unlock(&heap.heapLock);
}

int dma_give_intfrag() {
    pthread_mutex_lock(&heap.heapLock);
    int frag = heap.intFrag;
    pthread_mutex_unlock(&heap.heapLock);
    return frag;
}

