#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <i86.h>
#include <bios.h>

#define NSECTORS (1)

int get_chs(int drive, int *cyl, int *heads, int *sectors)
{
    union REGS regs;

    regs.h.ah = 0x08;
    regs.h.dl = drive;
    regs.w.di = 0;

    int86(0x13, &regs, &regs);
    *heads = regs.h.dh;
    *sectors = regs.h.cl & ((1 << 6) - 1);
    *cyl = regs.h.ch;
    *cyl |= (regs.w.cx & (128|64)) << 2;

    return regs.h.ah;
}

int read_sector(int drive, int cyl, int head, int sector, char* buf)
{
    union REGS regs;
    int ret;

    regs.h.ah = 0x02;
    regs.h.al = 1; // Sectors to read
    regs.w.bx = (unsigned short)buf;
    regs.w.cx = ((cyl & 0xff) << 8) | ((cyl & 0x30) >> 2) | sector;
    regs.h.dh = head;
    regs.h.dl = drive;

    int86(0x13, &regs, &regs);

    ret = regs.h.ah;
    if (regs.x.cflag) {
        ret |= 0x100;
    }

    return ret;
}

int get_status_last_op(int drive)
{
    union REGS regs;
    int ret;

    regs.h.ah = 0x01;
    regs.h.dl = drive;

    int86(0x13, &regs, &regs);

    ret = regs.h.ah;
    if (regs.x.cflag) {
        ret |= 0x100;
    }
    return ret;
}

int main(int argc, char **argv)
{
    int drive_num;
    char *endptr;
    FILE* fp;
    int cyl, heads, sectors;
    int rc;
    int i,j,k;
    char *buf = malloc(512);

    if (!buf) {
        perror("malloc");
    }
    memset(buf, 0, 512);

    if (argc < 3) {
        printf("Usage: %s <drive number in hex, 80 for first hdd> <output file>\n", argv[0]);
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
                unsigned short status;

                do {
                    status = read_sector(drive_num, i, j, k, buf);
                } while (status && ++tries < 5);
                if (1) {
                    printf("get status = 0x%x\n", status);
                }
                fwrite(buf, 512, NSECTORS, fp);
            }
        }
    }

    fflush(fp); // Just in case fclose is broken
    fclose(fp);
    free(buf);

    return 0;
}
