#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct partition
{
	u8 drive; /* drive number FD=0, HD=0x80, etc. */

	u8 head;	 /* starting head */
	u8 sector;   /* starting sector */
	u8 cylinder; /* starting cylinder */

	u8 sys_type; /* partition type: NTFS, LINUX, etc. */

	u8 end_head;	 /* end head */
	u8 end_sector;   /* end sector */
	u8 end_cylinder; /* end cylinder */

	u32 start_sector; /* starting sector counting from 0 */
	u32 nr_sectors;   /* number of of sectors in partition */
};

int fd;
int sector_one_start = 0, estart = 0, dev = 1, endsector = 0;
char buf[512];
void read_partitions(struct partition *p);

int main(int argc, const char *argv[])
{
	struct partition *p;
	puts("Device \t Boot Start\t\tEnd\t    Sectors\t  Id ");
	fd = open(argv[1], O_RDONLY);
	lseek(fd, (long)0, 0);
	read(fd, buf, 512);
	p = (struct partition *)(&buf[0x1be]);
	read_partitions(p);
	close(fd);
	return 0;
} // end main

void read_partitions(struct partition *p)
{
	while (p->sys_type != 0 && p->drive == 0)
	{
		printf("dev%d \t", dev);
		printf(" %8d \t", p->start_sector);
		printf(" %8d\t", ((int)p->start_sector) + ((int)p->nr_sectors) - 1);
		printf(" %8d\t", p->nr_sectors);
		printf(" %2x \n", p->sys_type);

		if (p->sys_type == 5)
		{
			estart += p->start_sector;
			int offset = 0;
			struct partition *pEx = p;
			lseek(fd, estart * 512, SEEK_SET);
			while (pEx->sys_type)
			{
				dev++;
				read(fd, buf, 512);
				pEx = (struct partition *)&buf[0x1BE];
				int exstart = pEx->start_sector + offset + estart;
				printf("dev%d \t", dev);
				printf(" %8d \t", exstart);
				printf(" %8d\t", exstart + ((int)pEx->nr_sectors) - 1);
				printf(" %8d\t", pEx->nr_sectors);
				printf(" %2x \n", pEx->sys_type);
				pEx++;
				lseek(fd, (estart + pEx->start_sector) * 512, SEEK_SET);
				offset = pEx->start_sector;
			}
			if (sector_one_start == 0)
				sector_one_start = p->start_sector;
			read(fd, buf, 512);
		}
		p++;
	}
}
