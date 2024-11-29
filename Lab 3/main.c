#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PAGE_SIZE 256
#define NUM_PAGES 256
#define NUM_FRAMES 256
#define TLB_SIZE 16

typedef struct {
    int page_number;
    int frame_number;
} TLBEntry;

int page_table[NUM_PAGES];
bool frame_used[NUM_FRAMES];
TLBEntry tlb[TLB_SIZE];
int tlb_index = 0;

int page_faults = 0;
int tlb_hits = 0;
int tlb_misses = 0;
int address_count = 0;

unsigned char physical_memory[NUM_FRAMES][PAGE_SIZE];

int search_tlb(int page_number);
void update_tlb(int page_number, int frame_number);
int get_free_frame();
int translate_address(int virtual_address);
void load_page_to_frame(int page_number, int frame_number);

int main(int argc, char *argv[]) {
    memset(page_table, -1, sizeof(page_table));
    memset(frame_used, false, sizeof(frame_used));

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) {
        perror("Error opening file");
        return 1;
    }

    int virtual_address;
    while (fscanf(input_file, "%d", &virtual_address) != EOF) {
        address_count++;
        translate_address(virtual_address);
    }

    fclose(input_file);

    printf("Number of addresses: %d\n", address_count);
    printf("Number of page faults: %d\n", page_faults);
    printf("Number of TLB hits: %d\n", tlb_hits);
    printf("Number of TLB misses: %d\n", tlb_misses);

    return 0;
}

int search_tlb(int page_number) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].page_number == page_number) {
            return tlb[i].frame_number;
        }
    }
    return -1;
}

void update_tlb(int page_number, int frame_number) {
    tlb[tlb_index].page_number = page_number;
    tlb[tlb_index].frame_number = frame_number;
    tlb_index = (tlb_index + 1) % TLB_SIZE;
}

int get_free_frame() {
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (!frame_used[i]) {
            frame_used[i] = true;
            return i;
        }
    }
    return -1;
}

void load_page_to_frame(int page_number, int frame_number) {
    for (int i = 0; i < PAGE_SIZE; i++) {
        physical_memory[frame_number][i] = page_number;
    }
}

int translate_address(int virtual_address) {
    int page_number = (virtual_address / PAGE_SIZE) % NUM_PAGES;
    int offset = virtual_address % PAGE_SIZE;

    int frame_number = search_tlb(page_number);
    if (frame_number != -1) {
        tlb_hits++;
    } else {
        tlb_misses++;
        frame_number = page_table[page_number];

        // Page fault
        if (frame_number == -1) {
            page_faults++;
            frame_number = get_free_frame();

            if (frame_number == -1) {
                fprintf(stderr, "Error: No free frames available\n");
                exit(EXIT_FAILURE);
            }

            page_table[page_number] = frame_number;
            load_page_to_frame(page_number, frame_number);
        }

        update_tlb(page_number, frame_number);
    }

    int physical_address = frame_number * PAGE_SIZE + offset;
    unsigned char value = physical_memory[frame_number][offset];
    printf("Virtual Address: %d, Physical Address: %d, Value: %d\n", virtual_address, physical_address, value);
    return physical_address;
}
