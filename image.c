#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <i86.h>
#include <bios.h>

#define NSECTORS (1)

int get_chs(int drive, int *cyl, int *heads, int *sectors)
{
    union REGS regs;
    struct SREGS sregs;

    regs.h.ah = 0x08;
    regs.h.dl = drive;
    regs.w.di = 0;
    sregs.es = 0;

    int86x(0x13, &regs, &regs, &sregs);
    *heads = regs.h.dh;
    *sectors = regs.h.cl & ((1 << 6) - 1);
    *cyl = regs.h.ch;
    *cyl |= (regs.w.cx & (128|64)) << 2;

    return regs.h.ah;
}

int main(int argc, char **argv)
{
    int drive_num;
    char *endptr;
    FILE* fp;
    int cyl, heads, sectors;
    int rc;
    int i,j,k;
    char *buf = malloc(1 << 10);

    if (!buf) {
        perror("malloc");
    }

    if (argc < 3) {
        printf("Usage: %s <drive number in hex, 80 for first hdd> <output file>\n");
        exit(2);
    }
    drive_num = strtol(argv[1], &endptr, 16);
    if (endptr == argv[1]) {
        printf("Invalid drive number: %s\n", argv[1]);
        exit(2);
    }

    fp = fopen(argv[2], "wb");
    if (fp == NULL) {
        perror("open");
        exit(2);
    }

    rc = get_chs(drive_num, &cyl, &heads, &sectors);

    printf("SRW Cylinders: %d Heads: %d Sectors: %d\n", cyl+1, heads+1, sectors);

    if (rc) {
        printf("Read Drive Paramters failed, status: %d\n", rc);
        exit(2);
    }

    for (i=0; i<=cyl; i++) {
        for (j=0; j<=heads; j++) {
            printf("Cyl %d Head %d\n", i,j);
            for (k=1; k<=sectors; k += NSECTORS) {
                int tries = 0;
                struct _ibm_diskinfo_t d;
                unsigned short status;

                do {
                    d.drive = drive_num;
                    d.head = j;
                    d.track = i;
                    d.sector = k;
                    d.nsectors = NSECTORS;
                    d.buffer = buf;

                    status = _bios_disk(_DISK_READ, &d) >> 8;
                } while (status && ++tries < 5);
                if (status) {
                    printf("Error %d at %d,%d,%d\n", status, i, j, k);
                    memset(buf, 0, 512);
                }
                fwrite(buf, 512, NSECTORS, fp);
            }
        }
    }

    fclose(fp);
    free(buf);

    return 0;
}
