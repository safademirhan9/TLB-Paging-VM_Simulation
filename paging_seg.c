// Safa Demirhan
// 171101070

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BELLEKSIZE 100
#define TLBSIZE 50

struct TLB
{
    int segment_no;
    int page_no;
    int timestamp;
};

struct Bellek
{
    int segment_no;
    int page_no;
    int timestamp;
};

/*GLOBAL VARIABLES*/
int invalidRef = 0;
int tlb_miss, tlb_hit = 0;
int mem_miss, mem_hit = 0;
int delay_ms = 0;
int segment, page;
int page_arr[3];

/* PROTOTYPES */
void read_file(char*, int*);                            /*      $size komutu ciktisi dosyadan okunur segmentler alinir      */
void set_bellek(struct Bellek *bellek);                 /*      En basta bellege her segment'in ilk page'i getirilir        */
void set_tlb(struct TLB *tlb);                          /*      TLB'nin segment ve page'lerini -1 ile doldur, bostur demek  */
int isInBellek(int, int, struct Bellek *bellek);        /*      Verilen segment/page bellekte mevcut mu kontrolu            */
int isInTLB(int, int, struct TLB *tlb);                 /*      Verilen segment/page TLB'de mevcut mu kontrolu              */
void checkInvalidReference(int*, int, int);             /*      Reference String'de kural disi erisim var mi kontrolu       */
void update_bellek(int, int, struct Bellek *bellek);    /*      VM'den page getirilir bos yer yoksa LRU uygulanir           */
void update_tlb(int, int, struct TLB *tlb);             /*      Bellekten page getirilir bos yer yoksa FIFO uygulanir       */
void count_bellek(struct Bellek *bellek);               /*      Bellek icerisindeki her page'in timestamp'i 1 artar         */
void count_tlb(struct TLB *tlb);                        /*      TLB icerisindeki her page'in timestamp'i 1 artar            */

int main(int argc, char *argv[]) {
    
    char buf[32];
    char* filename = argv[1];

    sprintf(buf, "size %s > size.txt", filename);
    system(buf);

    int segments_arr[3];
    
    read_file("size.txt", segments_arr);

    //int page_arr[3]; // GLOBAL VARIABLE
    //printf("Page count for each segment:\n");
    for(int i = -1; i<3; i++) {
        //if(i == -1) {printf("text\tdata\tbss\n");}
        //else {
            //printf("%d\t", (segments_arr[i]/1024)+1);
            page_arr[i] = (segments_arr[i]/1024)+1;
        //}
    }

    int physical_frame_arr[3];
    for(int i = 0; i<3; i++) {
        if(page_arr[i] % 2 == 1) {physical_frame_arr[i] = (page_arr[i]+1)/2;}
        else physical_frame_arr[i] = page_arr[i]/2;
    }

    /* PRINT AREA */
    printf("\n");
    printf("Segmentler kac byte:\n");
    for(int i = -1; i<3; i++) {
        if(i == -1) {printf("text\tdata\tbss\n");}
        else printf("%d\t", (segments_arr[i]));
    }
    printf("\n");
    printf("Kac page var:\n");
    for(int i = 0; i<3; i++) {printf("%d\t", page_arr[i]);}
    printf("\n");
    printf("Kac frame var:\n");
    for(int i = 0; i<3; i++) {printf("%d\t", physical_frame_arr[i]);}
    printf("\n\n");
    /* END OF PRINT AREA */


    /* Reference string dosyasini program olustursin istiyorsaniz burayi calistirin */
    // FILE *fp;
    // fp = fopen(argv[2], "w+");
    // size_t n = sizeof(page_arr)/sizeof(page_arr[0]);
    // for(int i = 0; i<3; i++) {
    //     for(int j = 0; j<page_arr[i]; j++) {
    //         fprintf(fp, "%d %d\n", i,j);
    //     }
    // }
    // fclose(fp);

    struct TLB tlb[TLBSIZE];
    set_tlb(tlb);
    struct Bellek bellek[BELLEKSIZE];
    set_bellek(bellek);

    FILE *file;
    char buf2[100];
    file = fopen(argv[2], "r");
    while(fgets(buf2, 99, file) != NULL){
        sscanf( buf2, "%d %d", &segment, &page);
        /* Reference String Erisimleri Sirasiyla Bu Sekilde */
        if(isInTLB(segment, page, tlb)) {
            sleep(0.001);
            delay_ms += 1;
            count_tlb(tlb);
            tlb_hit++;
            continue;
        }
        if(isInBellek(segment, page, bellek)) {
            sleep(0.01);
            delay_ms += 10;
            count_bellek(bellek);
            update_tlb(segment, page, tlb);
            tlb_miss++;
            mem_hit++;
            continue;
        }
        // PAGE FAULT, SANAL BELLEKTEN GETIR
        else{ 
            sleep(0.1);
            delay_ms += 100;
            update_bellek(segment, page, bellek);
            update_tlb(segment, page, tlb);
            tlb_miss++;
            mem_miss++;
        } 
    }

    double tlb_miss_ratio = tlb_miss/(double)(tlb_hit+tlb_miss);
    double mem_miss_ratio = mem_miss/(double)(mem_hit+mem_miss);
    printf("1. TLB miss sayisi ve orani:\t%d\t-\t%c%f\n", tlb_miss, '%',tlb_miss_ratio*100);
    printf("2. Page fault sayisi ve orani:\t%d\t-\t%c%f\n", mem_miss, '%',mem_miss_ratio*100);
    printf("3. Toplam olusan delay suresi:\t%dms (~%ds)\n", delay_ms, delay_ms/1000);
    printf("4. Invalid reference sayisi:\t%d\n", invalidRef);
}

void read_file(char* filename, int* nums) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int i,j = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) {exit(EXIT_FAILURE);}

    while ((read = getline(&line, &len, fp)) != -1) {
        if(i == 1) {
            const char s[2] = "\t";
            char *token;
            token = strtok(line, s);
            while( token != NULL ) {
                int num = atoi(token);
                nums[j++] = num;
                token = strtok(NULL, s);
            }
        }
        i++;
    }
    fclose(fp);
    if (line) {free(line);}
}

void set_bellek(struct Bellek *bellek) {
    for(int i = 0; i<3; i++) {
        bellek[i].segment_no = i;
        bellek[i].page_no = 0;
        bellek[i].timestamp = 0;
    }
    // Bos oldugu anlamina gelir
    for(int i = 3; i<BELLEKSIZE; i++) {
        bellek[i].segment_no = -1;
        bellek[i].page_no = -1;
    }
}

void set_tlb(struct TLB *tlb) {
    // Bos oldugu anlamina gelir
    for(int i = 0; i<TLBSIZE; i++) {
        tlb[i].segment_no = -1;
        tlb[i].page_no = -1;
    }
}

int isInBellek(int segment, int page, struct Bellek *bellek) {
    int true = 0; // seg no bulursa artar page no'ya o zaman bakmaya baslar
    int foundSeg = -1; // seg no indexini alir, page no uyusuyor mu bakar
    for(int i = 0; i<3; i++) {
        if(segment == i) {true++; foundSeg = i; break;}
    }
    // Found in memory, also reset timestamp
    if(true == 1){
        for(int i = 0; i<BELLEKSIZE; i++) {
            //if(page == bellek[i].page_no && segment == bellek[i].segment_no) {true++; break;}
            if(page == bellek[foundSeg].page_no && segment == bellek[i].segment_no) {
                true++;
                bellek[foundSeg].timestamp = 0;
                break;
            }
        }
    }
    // check invalid reference, not needed, already done in TLB 
    //if(segment > 2) {invalidRef++;}
    //else if(page > page_arr[segment]) {invalidRef++;}
    if(true == 2) {return 1;}
    else return 0;
}

int isInTLB(int segment, int page, struct TLB *tlb) {
    // check invalid reference
    if(segment > 2) {invalidRef++;}
    else if(page > page_arr[segment]) {invalidRef++;}

    // iterate over TLB
    for(int i = 0; i<TLBSIZE; i++) {
        if(segment == tlb[i].segment_no) {
            if(page == tlb[i].page_no) {
                //TLB HIT
                return 1;
            }
        }
    }
    return 0;
}

void checkInvalidReference(int* page_arr, int segment, int page) {
    if(segment > 2) {invalidRef++; return;}
    else if(page > page_arr[segment]) {invalidRef++;}
}

void update_bellek(int segment, int page, struct Bellek *bellek) {
    // Is there any free space in memory?
    int index = 0;
    for(int i = 0; i<BELLEKSIZE; i++) {
        if(bellek[i].segment_no == 3) {
            index = i;
            break;
        }
    }
    // There was an empty space
    if(index != 0) {
        bellek[index].segment_no = segment;
        bellek[index].page_no = page;
        bellek[index].timestamp = 0;
    }
    // Update policy: LRU, add target page 
    else{
        int timestamp = 0;
        for(int i = 0; i<BELLEKSIZE; i++) {
            if(bellek[i].timestamp > timestamp) {
                timestamp = bellek[i].timestamp;
            }
        }
        // Spot max timestamps' index
        int index = 0;
        for(int i = 0; i<TLBSIZE; i++) {
            if(bellek[i].timestamp == timestamp) {index = i;}
        }
        // Set target page in index
        bellek[index].segment_no = segment;
        bellek[index].page_no = page;
        bellek[index].timestamp = 0;
    }
}
void update_tlb(int segment, int page, struct TLB *tlb) {
    // Is there any free space in tlb?
    int index = 0;
    for(int i = 0; i<TLBSIZE; i++) {
        if(tlb[i].segment_no == 3) {
            index = i;
            break;
        }
    }
    // There was an empty space
    if(index != 0) {
        tlb[index].segment_no = segment;
        tlb[index].page_no = page;
        tlb[index].timestamp = 0;
    }
    // Update policy: FIFO, add target page 
    else{
        int timestamp = 0;
        for(int i = 0; i<TLBSIZE; i++) {
            if(tlb[i].timestamp > timestamp) {
                timestamp = tlb[i].timestamp;
            }
        }
        // Spot max timestamps' index
        int index = 0;
        for(int i = 0; i<TLBSIZE; i++) {
            if(tlb[i].timestamp == timestamp) {index = i;}
        }
        // Set target page in index
        tlb[index].segment_no = segment;
        tlb[index].page_no = page;
        tlb[index].timestamp = 0;
    }
}

void count_bellek(struct Bellek *bellek) {
    for(int i = 0; i<BELLEKSIZE; i++) {
        if(bellek[i].segment_no != -1 && bellek[i].page_no != -1)
        bellek[i].timestamp++;
    }
}
void count_tlb(struct TLB *tlb) {
    for(int i = 0; i<TLBSIZE; i++) {
        if(tlb[i].segment_no != -1 && tlb[i].page_no != -1)
        tlb[i].timestamp++;
    }
}

