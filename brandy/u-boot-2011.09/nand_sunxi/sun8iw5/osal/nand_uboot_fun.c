/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <malloc.h>
#include "../nand_interface/nand_bsp.h"

#define  OOB_BUF_SIZE                   64
#define NAND_BOOT0_BLK_START    0
#define NAND_BOOT0_BLK_CNT		4
#define NAND_UBOOT_BLK_START    (NAND_BOOT0_BLK_START+NAND_BOOT0_BLK_CNT)
#define NAND_UBOOT_BLK_CNT		30
#define NAND_BOOT0_PAGE_CNT_PER_COPY     64

static char nand_para_store[256];
static int  flash_scaned;
static struct _nand_info* g_nand_info = NULL;
static int nand_partition_num;
int  Nand_erase_flag = 0;
int Uboot_Start_Block = 0;
int Uboot_Next_Block = 0;
int Logic_Start_Block = 0;

int  mbr_burned_flag;
PARTITION_MBR nand_mbr = {0};

extern int NAND_Print(const char * str, ...);
extern int NAND_set_boot_mode(__u32 boot);
extern __u32 NAND_GetNandCapacityLevel(void);
extern void set_capacity_level(struct _nand_info*nand_info,unsigned short capacity_level);
__s32 NAND_ReadPhyArch(void);

extern __u32 NAND_GetBlankBlockFlag(void);
extern int Physic_Info_Add_To_Uboot_Tail(__u32 *buf_dst, __u32 uboot_size);
extern __s32 Clean_Physic_Info(void);

extern __s32 GetUbootStartBlock(void);
extern __s32 GetUbootNextBlock(void);
extern __s32 GetLogicStartBlock(void);

int __NAND_UpdatePhyArch(void)
{
	printf("call null __NAND_UpdatePhyArch()!!!\n");
    return 0;
}
int NAND_UpdatePhyArch(void)
	__attribute__((weak, alias("__NAND_UpdatePhyArch")));


//int __erase_last_phy_partition(struct _nand_info*nand_info)
//{
//	printf("call null __erase_last_phy_partition()!!!\n");
//    return 0;
//}
//int erase_last_phy_partition(struct _nand_info*nand_info)
//	__attribute__((weak, alias("__erase_last_phy_partition")));


int msg(const char * str, ...)
{
    NAND_Print(str);

    return 0;
}

int NAND_PhyInit(void)
{
	struct _nand_info* nand_phy_info;

	NAND_Print("NB1 : enter phy init\n");

    ClearNandStruct();

	nand_phy_info = NandHwInit();
	if (nand_phy_info == NULL)
	{
		printf("NB1 : nand phy init fail\n");
		return -1;
	}

	Uboot_Start_Block = GetUbootStartBlock();
	Uboot_Next_Block = GetUbootNextBlock();
	Logic_Start_Block = GetLogicStartBlock();
	NAND_Print("Uboot_Start_Block %d Uboot_Next_Block %d Logic_Start_Block %d\n",Uboot_Start_Block,Uboot_Next_Block,Logic_Start_Block);

	NAND_Print("NB1 : nand phy init ok\n");

	return 0;

}

int NAND_PhyExit(void)
{
	NAND_Print("NB1 : enter phy Exit\n");	
	NandHwExit();

	return 0;
}


int NAND_LogicWrite(uint nSectNum, uint nSectorCnt, void * pBuf)
{
    return nftl_write(nSectNum,nSectorCnt,pBuf);
}


int NAND_LogicRead(uint nSectNum, uint nSectorCnt, void * pBuf)
{
	return nftl_read(nSectNum,nSectorCnt,pBuf);
}

#if 0
int NAND_LogicInit(int boot_mode)
{
	__s32  result;
	__s32 ret;
	struct _nand_info* nand_info;
	char* mbr;

	printf("NB1 : enter NAND_LogicInit\n");

    ClearNandStruct();

	nand_info = NandHwInit();
	if (nand_info == NULL)
	{
		printf("NB1 : nand phy init fail\n");
		return ret;
	}

	if((!boot_mode)&&(nand_mbr.PartCount!= 0)&&(mbr_burned_flag ==0))
	{
		result = nand_info_init(nand_info, 0, 8, (uchar *)&nand_mbr);
		mbr_burned_flag = 1;
	}
	else
	{
		result = nand_info_init(nand_info, 0, 8, NULL);
	}

	if(result < 0)
	{
		printf("NB1 : nand_info_init fail\n");
		return -5;
	}

	result = nftl_build_all(nand_info);
	if(result < 0)
	{
		printf("NB1 : nftl_build_all fail\n");
		return -5;
	}

   	 printf("NB1 : NAND_LogicInit ok\n");
    return result;
}
#else
int NAND_LogicInit(int boot_mode)
{
	__s32  result =0;
	__s32 ret = -1;
	__s32 i, nftl_num;
	__s32 capacity_level;
	struct _nand_info* nand_info;
	//char* mbr;

	printf("NB1 : enter NAND_LogicInit\n");

    ClearNandStruct();

	nand_info = NandHwInit();

	capacity_level = NAND_GetNandCapacityLevel();
	set_capacity_level(nand_info,capacity_level);

	g_nand_info = nand_info;
	if (nand_info == NULL)
	{
		printf("NB1 : nand phy init fail\n");
		return ret;
	}

	if((!boot_mode)&&(nand_mbr.PartCount!= 0)&&(mbr_burned_flag ==0))
	{
		printf("burn nand partition table! mbr tbl: 0x%x, part_count:%d\n", (__u32)(&nand_mbr), nand_mbr.PartCount);
		result = nand_info_init(nand_info, 0, 8, (uchar *)&nand_mbr);
		mbr_burned_flag = 1;
	}
	else
	{
		printf("not burn nand partition table!\n");
		result = nand_info_init(nand_info, 0, 8, NULL);
	}

	if(result != 0)
	{
		printf("NB1 : nand_info_init fail\n");
		return -5;
	}

	if(boot_mode)
	{
		nftl_num = get_phy_partition_num(nand_info);
		printf("NB1 : nftl num: %d \n", nftl_num);
		if((nftl_num<1)||(nftl_num>5))
		{
			printf("NB1 : nftl num: %d error \n", nftl_num);
			return -1;
		}

        nand_partition_num = 0;
		if(nftl_num > 1)
		{
			for(i=0; i<nftl_num-1; i++)
			{
			    nand_partition_num++;
				printf(" init nftl: %d \n", i);
				result = nftl_build_one(nand_info, i);
			}
		}
		else if(nftl_num == 1)
		{
			nand_partition_num++;
			printf(" init nftl: %d \n", 0);
			result = nftl_build_one(nand_info, 0);

		}
	}
	else
	{
		result = nftl_build_all(nand_info);
		nand_partition_num = get_phy_partition_num(nand_info);
	}

	if(result != 0)
	{
		printf("NB1 : nftl_build_all fail\n");
		return -5;
	}

   	 printf("NB1 : NAND_LogicInit ok, result = 0x%x \n",result);
	return result;
}
#endif

int NAND_LogicExit(void)
{
	printf("NB1 : NAND_LogicExit\n");
	nftl_flush_write_cache();
	NandHwExit();
	g_nand_info = NULL;
	return 0;
}

int NAND_build_all_partition(void)
{
	int result,i;
	int nftl_num;

	if(g_nand_info == NULL)
	{
		printf("NAND_build_all_partition fail 1\n");
		return -1;
	}

	nftl_num = get_phy_partition_num(g_nand_info);
	if(nftl_num == nand_partition_num)
	{
		return 0;
	}

	if((nand_partition_num >= nftl_num) || (nand_partition_num == 0))
	{
		printf("NAND_build_all_partition fail 2 %d\n",nand_partition_num);
		return -1;
	}

	for(i=nand_partition_num; i<nftl_num; i++)
	{
		printf(" init nftl: %d \n", i);
		result = nftl_build_one(g_nand_info, i);
		if(result != 0)
		{
			printf("NAND_build_all_partition fail 3 %d %d\n",result,i);
			return -1;
		}
	}
	return 0;
}

uint max_badblk(uint v0, uint v1)
{
	uint val;

	if(v0 > v1)
		val = v0;
	else
		val = v1;

	return val;
}

int cal_real_chip(uint global_bank, uint chip_connect_info)
{
	uint chip;
	uint i,cnt;

	cnt = 0;
	chip = global_bank;

	for (i = 0; i < 8; i++ )
	{
		if (chip_connect_info & (1 << i))
			{
				cnt++;
				if (cnt == (chip+1))
					{
						chip = i;
						return chip;
					}
			}
	}

	return -1;
}

int get_bad_blk_marked_page(uint badflag, uint page_per_blk,
		int *page_index)
{
	page_index[0] = 0;

	switch(badflag & 0x03)
	{
		case 0x00:
			/**
			 * the bad block flag is in the first page,
			 * same as the logical information, just read 1 page is ok
			 */
			break;

		case 0x01:
			/**
			 * the bad block flag is in the first page or the second page,
			 * need read the first page and the second page
			 */
			page_index[1] = 1;
			break;

		case 0x02:
			/**
			 * the bad block flag is in the last page,
			 * need read the first page and the last page
			 */
			page_index[1] = page_per_blk - 1;
			break;

		case 0x03:
			/**
			 * the bad block flag is in the last 2 page,
			 * so, need read the first page, the last page and the last-1 page
			 */
			page_index[1] = page_per_blk - 1;
			page_index[2] = page_per_blk - 2;
			break;

		default:
			return -1;
	}
	return 0;
}

int mark_bad_block( uint chip_num, uint blk_num)
{
    boot_flash_info_t info;
	struct boot_physical_param  para;
	unsigned char   oob_buf[OOB_BUF_SIZE];
	unsigned char*  page_buf;
	int  page_index[4] = {0xEE, 0xEE, 0xEE, 0xEE};
	uint  i;
	int  mark_err_flag = -1;


	if( NAND_GetFlashInfo( &info ))
	{
		printf("get flash info failed.\n");
		return -1;
	}

	//cal nand parameters
	//page_buf = (unsigned char*)(MARK_BAD_BLK_BUF_ADR);
    page_buf = (unsigned char*)malloc(32 * 1024);
    if(!page_buf)
    {
        printf("malloc memory for page buf fail\n");
        return -1;
    }

	get_bad_blk_marked_page(info.pagewithbadflag,
			info.blocksize / info.pagesize, page_index);

	for(i =0; i<4; i++)
	{
		oob_buf[0] = 0x0;
		oob_buf[1] = 0x1;
		oob_buf[2] = 0x2;
		oob_buf[3] = 0x3;
		oob_buf[4] = 0x89;
		oob_buf[5] = 0xab;
		oob_buf[6] = 0xcd;
		oob_buf[7] = 0xef;

		para.chip = chip_num;
		para.block = blk_num;
		para.page = page_index[i];
		para.mainbuf = page_buf;
		para.oobbuf = oob_buf;

		if(para.page == 0xEE)
		    continue;

        PHY_SimpleWrite( &para );
        PHY_SimpleRead( &para );

		if(oob_buf[0] !=0xff)
			mark_err_flag = 0;
	}

	free(page_buf);

	return mark_err_flag;
}

int NAND_VersionGet(unsigned char *version)
{
    __u32 nand_version;

	nand_version = NAND_GetNandVersion();

    version[0] = 0xff;     //bad block flag
    version[1] = 0x00;     //reserved, set to 0x00
    version[2] = (nand_version>>16);     //nand driver version 0, current vresion is 0x02
    version[3] = (nand_version>>24);     //nand driver version 1, current vresion is 0x00

    return 0;
}


int NAND_VersionCheck(void)
{
	struct boot_physical_param boot0_readop_temp;
	struct boot_physical_param *boot0_readop = NULL;
	uint block_index;
	uint cnt1;
	int version_match_flag = -1;
	//uint chip_type;
	int i;
	unsigned char  oob_buf[32];
	uint* main_data;
	unsigned char  nand_version[4];

    /********************************************************************************
    *   nand_version[2] = 0xFF;          //the sequnece mode version <
    *   nand_version[2] = 0x01;          //the first interleave mode version, care ecc
    *                                      2010-06-05
    *   nand_version[2] = 0x02;          //the current version, don't care ecc
    *                                      2010-07-13
    *   NOTE:  need update the nand version in update_boot0 at the same time
    ********************************************************************************/
    NAND_VersionGet(nand_version);

    printf("check nand version start.\n");
	printf("Current nand driver version is %x %x %x %x \n", nand_version[0], nand_version[1], nand_version[2], nand_version[3]);

    boot0_readop = &boot0_readop_temp;

	//init boot0_readop
	boot0_readop->block = 0x0;
	boot0_readop->chip = 0;
	boot0_readop->mainbuf = (void*)malloc(32 * 1024);
    if(!boot0_readop->mainbuf)
    {
        printf("malloc memory for boot0 read operation fail\n");
        return -1;
    }

	boot0_readop->oobbuf = oob_buf;
	boot0_readop->page = 0;
	boot0_readop->sectorbitmap = 0;

    main_data = (uint*)malloc(32 * 1024);
    if(!main_data)
    {
        printf("malloc memory for main data fail\n");
        return -1;
    }

    //scan boot1 area blocks
	for(block_index=Uboot_Start_Block;block_index<Uboot_Next_Block;block_index++)
	{

		boot0_readop->block = block_index;
		boot0_readop->page = 0;
		cnt1 = 0;

        //printf("%s %d mainbuf: 0x%x\n", __FILE__, __LINE__, (__u32)boot0_readop->mainbuf);
        PHY_SimpleRead(boot0_readop);

        //check the current block is a bad block
		if(oob_buf[0] != 0xFF)
	    {
			printf("block %u is bad block.\n",block_index);
			continue;
	    }


		//check the current block is a all 0xFF block
		for(i=0; i<256; i++)
		{
			if(*(main_data+i) == 0xffffffff)
				cnt1++;
		}

		if(cnt1 == 256)
	    {
			printf("block %u is cleared block.\n",block_index);
			continue;
	    }

	    if((oob_buf[1] == 0x00) || (oob_buf[1] == 0xFF))
	    {
	       printf("Media version is valid in block %u, version info is %x %x %x %x \n", block_index, oob_buf[0], oob_buf[1], oob_buf[2], oob_buf[3]);
	       if(oob_buf[2] == nand_version[2])
	       {
	            printf("nand driver version match ok in block %u.\n",block_index);
    		    version_match_flag = 0;
    		    break;
	       }
	       else
	       {
	            printf("nand driver version match fail in block %u.\n",block_index);
    		    version_match_flag = 1;
    		    break;
	       }

	    }
	    else
	    {
	        printf("Media version is invalid in block %uversion info is %x %x %x %x \n", block_index, oob_buf[0], oob_buf[1], oob_buf[2], oob_buf[3]);
	    }

	}

    if(block_index >= Uboot_Next_Block)
    {
         printf("can't find valid version info in boot blocks. \n");
         version_match_flag = -1;
    }

    free(main_data);
	return version_match_flag;
}


int  NAND_EraseBootBlocks(void)
{

    struct boot_physical_param  para;
	int  i;
	int  ret;

	printf("Ready to erase boot blocks.\n");

	for( i = 0;  i < Uboot_Start_Block;  i++ )
	{
		para.chip = 0;
		para.block = i;
		ret = PHY_SimpleErase( &para ) ;
		if(ret)
		    printf("erasing block %u failed.\n", i );
	}

    printf("has cleared the boot blocks.\n");

    return 0;

}


int  NAND_EraseChip(void)
{
	struct boot_physical_param  para_read;
	int  i,j,k;
	int  ret;
	//int  cnt0, cnt1;
	//int  cnt_oob;
	int  mark_err_flag;
	uint  bad_block_flag;
	uint  chip_cnt, chip_connect, page_size, page_per_block, blk_cnt_per_chip;
	uint block_cnt_of_die, die_skip_flag, start_blk;
	int  page_index[4];
	uint  chip;
	unsigned char   oob_buf_read[OOB_BUF_SIZE];
	unsigned char*  page_buf_read;
	int  error_flag = 0;
	__u32 read_retry_type = 0, read_retry_mode;
	boot_flash_info_t info;

	if (NAND_GetFlashInfo(&info)) {
		printf("get flash info failed.\n");
		return -1;
	}

	read_retry_type = NAND_GetReadRetryType();
	read_retry_mode = (read_retry_type>>16)&0xff;

    page_buf_read = (unsigned char*)malloc(64 * 1024);
    if(!page_buf_read)
    {
        printf("malloc memory for page read fail\n");
        return -1;
    }
	printf("Ready to erase chip.\n");

	page_size = NAND_GetPageSize();
	page_per_block = NAND_GetPageCntPerBlk();
	blk_cnt_per_chip = NAND_GetBlkCntPerChip();
	printf("page_size=%d, page_per_block=%d, blk_cnt_per_chip=%d\n", page_size, page_per_block, blk_cnt_per_chip);
	chip_cnt = NAND_GetChipCnt();
	chip_connect = NAND_GetChipConnect();
    printf("chip_cnt = %x, chip_connect = %x\n",chip_cnt,chip_connect);
    block_cnt_of_die = NAND_GetBlkCntOfDie();
    die_skip_flag = NAND_GetDieSkipFlag();

	get_bad_blk_marked_page(info.pagewithbadflag, page_per_block, page_index);

	for( i = 0;  i < chip_cnt;  i++ )
	{
	    //select chip
		chip = cal_real_chip( i, chip_connect );
        printf("erase chip %u \n", chip);
		if(i==0)
		{
			//start_blk = Uboot_Next_Block;
			start_blk = Logic_Start_Block;
		}
		else
			start_blk = 0;

        //scan for bad blocks, only erase good block, all 0x00 blocks is defined bad blocks
		for( j = start_blk;  j < blk_cnt_per_chip;  j++ )
		{


			if(!die_skip_flag)
			    para_read.block = j;
			else
			    para_read.block = j%block_cnt_of_die + 2*block_cnt_of_die*(j/block_cnt_of_die);

			if((j&0xff) == 0)
				printf("erase chip %u, block %u\n",chip, para_read.block);

			para_read.chip = chip;
			para_read.mainbuf = page_buf_read;
			para_read.oobbuf = oob_buf_read;

			if(((read_retry_mode == 0x32)||(read_retry_mode == 0x33))&&((NAND_GetBlankBlockFlag()) == 1))
			{
				bad_block_flag = 0;
				//1y nm new flash:force erase
			}
			else
			{
				bad_block_flag = 0;

				for(k = 0; k<4; k++)
				{
					//cnt_oob =0;
					//cnt0 =0;
					//cnt1 =0;
					para_read.page = page_index[k];
					if( para_read.page== 0xEE)
					    break;

					ret = PHY_SimpleRead_2CH(& para_read );

					//check the current block is a all 0x00 block
					if(oob_buf_read[0] != 0xff)
					{
						bad_block_flag = 1;
						printf("find a bad block %u, page %d, oob %x\n",
								para_read.block, page_index[i], oob_buf_read[0]);
						break;
					}

				}
			}

			if(bad_block_flag)
				continue;


			ret = PHY_SimpleErase_2CH( &para_read );
			if( ret != 0 )
	    		{
	    		    printf("erasing block %u failed.\n", para_read.block );
			    	#if 1
			    	mark_err_flag = mark_bad_block( i, para_read.block );
	    		    if( mark_err_flag!= 0 )
	    		        {
        					error_flag++;
        					printf("error in marking bad block flag in chip %u, block %u, mark error flag %u.\n", i, para_read.block, mark_err_flag);
        				}
        			#endif
	    		}

    		}
	}
	Nand_erase_flag = 1;

	printf("has cleared the chip.\n");
	if(error_flag)
		printf("the nand is Bad.\n");
	else
		printf("the nand is OK.\n");

    free(page_buf_read);

	return 0;

}


int  NAND_EraseChip_force(void)
{
	struct boot_physical_param  para_read;
	int  i,j,k;
	int  ret;
	int  mark_err_flag;
	uint  chip_cnt, chip_connect, page_size, page_per_block, blk_cnt_per_chip;
	uint block_cnt_of_die, die_skip_flag, start_blk;
	int  page_index[4] = {0xEE, 0xEE, 0xEE, 0xEE};
	uint  chip;
	unsigned char   oob_buf_read[OOB_BUF_SIZE];
	unsigned char*  page_buf_read;
	int  error_flag = 0;
	boot_flash_info_t info;

	if (NAND_GetFlashInfo(&info)) {
		printf("get flash info failed.\n");
		return -1;
	}

    page_buf_read = (unsigned char*)malloc(64 * 1024);
    if(!page_buf_read)
    {
        printf("malloc memory for page read fail\n");
        return -1;
    }
	printf("Ready to force erase chip.\n");

	page_size = NAND_GetPageSize();
	page_per_block = NAND_GetPageCntPerBlk();
	blk_cnt_per_chip = NAND_GetBlkCntPerChip();
	printf("page_size=%d, page_per_block=%d, blk_cnt_per_chip=%d\n", page_size, page_per_block, blk_cnt_per_chip);
	chip_cnt = NAND_GetChipCnt();
	chip_connect = NAND_GetChipConnect();
    printf("chip_cnt = %x, chip_connect = %x\n",chip_cnt,chip_connect);
    block_cnt_of_die = NAND_GetBlkCntOfDie();
    die_skip_flag = NAND_GetDieSkipFlag();

	get_bad_blk_marked_page(info.pagewithbadflag, page_per_block, page_index);

	for( i = 0;  i < chip_cnt;  i++ )
	{
	    //select chip
		chip = cal_real_chip( i, chip_connect );
        printf("erase chip %u \n", chip);
		if(i==0)
		{
			//start_blk = Uboot_Next_Block;
			start_blk = Logic_Start_Block;
		}
		else
			start_blk = 0;

        //scan for bad blocks, only erase good block, all 0x00 blocks is defined bad blocks
		for( j = start_blk;  j < blk_cnt_per_chip;  j++ )
		{


			if(!die_skip_flag)
			    para_read.block = j;
			else
			    para_read.block = j%block_cnt_of_die + 2*block_cnt_of_die*(j/block_cnt_of_die);

			if((j&0xff) == 0)
				printf("erase chip %u, block %u\n",chip, para_read.block);

			para_read.chip = chip;
			para_read.mainbuf = page_buf_read;
			para_read.oobbuf = oob_buf_read;


			for(k = 0; k<4; k++)
			{
				//cnt_oob =0;
				//cnt0 =0;
				//cnt1 =0;
				para_read.page = page_index[k];
				if( para_read.page== 0xEE)
				    break;

				ret = PHY_SimpleRead_2CH(& para_read );

				//check the current block is a all 0x00 block
				if(oob_buf_read[0] == 0x0)
				{
					printf("find a bad block %u, page %d, oob %x\n",
							para_read.block, page_index[i], oob_buf_read[0]);
					break;
				}

			}

			ret = PHY_SimpleErase_2CH( &para_read );
			if( ret != 0 )
	    		{
	    		    printf("erasing block %u failed.\n", para_read.block );
			    	#if 1
			    	mark_err_flag = mark_bad_block( i, para_read.block );
	    		    if( mark_err_flag!= 0 )
	    		        {
        					error_flag++;
        					printf("error in marking bad block flag in chip %u, block %u, mark error flag %u.\n", i, para_read.block, mark_err_flag);
        				}
        			#endif
	    		}

    		}
	}

	printf("has cleared the chip.\n");
	if(error_flag)
		printf("the nand is Bad.\n");
	else
		printf("the nand is OK.\n");

    free(page_buf_read);

	return 0;
}


int NAND_BadBlockScan(void)
{
	int  i, j, k;
	uint  page_per_block, block_cnt_per_chip, chip_cnt, chip_connect_mode;
	int  page_index[4] = {0xEE, 0xEE, 0xEE, 0xEE};
	uint  bad_block_cnt[8];
	uint  bad_block_num = 0;
	uint  good_block_num = 0;
	int  good_block_ratio = -1, default_good_block_ratio =-1;
	uint  chip;
	unsigned char   oob_buf[OOB_BUF_SIZE];
	unsigned char*  page_buf;
	struct boot_physical_param  para;
	boot_flash_info_t info;

	if( NAND_GetFlashInfo( &info ))
	{
		printf("get flash info failed.\n");
		return -1;
	}

	for(i=0; i<8; i++)
	    bad_block_cnt[i] = 0;

	printf("Ready to scan bad blocks.\n");

	//cal nand parameters
	//page_buf = (unsigned char*)(BAD_BLK_SCAN_BUF_ADR);
    page_buf = (unsigned char*)malloc(32 * 1024);
    if(!page_buf)
    {
        printf("malloc memory for page buf fail\n");
        return -1;
    }

	page_per_block = NAND_GetPageCntPerBlk();
	block_cnt_per_chip = NAND_GetBlkCntPerChip();
	chip_cnt = NAND_GetChipCnt();
	chip_connect_mode = NAND_GetChipConnect();
	default_good_block_ratio = NAND_GetValidBlkRatio();

	get_bad_blk_marked_page(info.pagewithbadflag, page_per_block, page_index);

    //scan bad blocks
	for( i = 0;  i < chip_cnt;  i++ ){

		chip = cal_real_chip( i, chip_connect_mode );
		printf("scan CE %u\n", chip);
		bad_block_cnt[chip] = 0;

		for( j = 0;  j < block_cnt_per_chip;  j++ )
		{
			para.chip = chip;
			para.block = j;
			para.mainbuf = page_buf;
			para.oobbuf = oob_buf;

			for(k = 0; k<4; k++)
			{
				// read pages for check
				para.page = page_index[k];
				if(para.page == 0xEE)
				    continue;
				PHY_SimpleRead_2CH(&para );

				// find bad blocks
				if(oob_buf[0] != 0xff)
				{
				    printf("find defined bad block in chip %u, block %u.\n", i, j);
					bad_block_cnt[chip]++;
                    break;
				}
			}
		}
	}

	// cal bad block num
	if(chip_cnt == 0x1)        //for one CE
    	{
    	    if(chip_connect_mode == 0x1)
    	        {
    	            bad_block_num = bad_block_cnt[0]<<1;
    	        }
    		else
    		    {
    		        printf("chip connect parameter %x error \n", chip_connect_mode);
    		        free(page_buf);

        			return -1;
    		    }
    	}
	else if(chip_cnt == 2)     //for two CE
    	{
    		if(chip_connect_mode == 0x3)
    		    {
    			    bad_block_num = (bad_block_cnt[0] + bad_block_cnt[1])<<1;
    		    }
    		else if(chip_connect_mode == 0x5)
        		{
        			bad_block_num = (bad_block_cnt[0] + bad_block_cnt[2])<<1;
        		}
    		else if(chip_connect_mode == 0x81)
        		{
        			bad_block_num = (bad_block_cnt[0] + bad_block_cnt[7])<<1;
        		}
    		else
        		{
        			printf("chip connect parameter %x error \n", chip_connect_mode);
        			free(page_buf);

        			return -1;
        		}
    	}
	else if(chip_cnt == 4)     //for four CE
	    {
    		if(chip_connect_mode == 0xf)
    		    {
    			    bad_block_num = max_badblk((bad_block_cnt[0] + bad_block_cnt[2]),(bad_block_cnt[1] + bad_block_cnt[3]))<<1;
    		    }
    		else if(chip_connect_mode == 0x55)
    		    {
    			    bad_block_num = max_badblk((bad_block_cnt[0] + bad_block_cnt[2]),(bad_block_cnt[4] + bad_block_cnt[6]))<<1;
    		    }
    		else
    		    {
    			    printf("chip connect parameter %x error \n",chip_connect_mode);
    			    free(page_buf);

    			    return -1;
    		    }

	    }
	else if(chip_cnt == 8)     //for eight CE
	    {
    		if(chip_connect_mode == 0xff)
    		    {
        			bad_block_num = max_badblk((bad_block_cnt[0] + bad_block_cnt[2]),(bad_block_cnt[1] + bad_block_cnt[3]));
        			bad_block_num = 2*max_badblk(bad_block_num, max_badblk((bad_block_cnt[4] + bad_block_cnt[6]),(bad_block_cnt[5] + bad_block_cnt[7])));

        		}
    		else
    		    {
        			printf("chip connect parameter %x error \n",chip_connect_mode);
        			free(page_buf);

        			return -1;
        		}
	    }
	else
	    {
	        printf("chip cnt parameter %x error \n",chip_connect_mode);
	        free(page_buf);

        	return -1;
	    }


	//cal good block num required per 1024 blocks
	good_block_num = (1024*(block_cnt_per_chip - bad_block_num))/block_cnt_per_chip -50;
    for(i=0; i<chip_cnt; i++)
    {
		chip = cal_real_chip( i, chip_connect_mode );
		printf(" %d bad blocks in CE %u \n", bad_block_cnt[chip], chip);
	}
	printf("cal bad block num is %u \n", bad_block_num);
	printf("cal good block num is %u \n", good_block_num);

    //cal good block ratio
	for(i=0; i<5; i++)
	{
		if(good_block_num >= (default_good_block_ratio - 32*i))
	    {
			good_block_ratio = (default_good_block_ratio - 32*i);
			printf("good block ratio is %u \n",good_block_ratio);
			break;
		}
	}
    free(page_buf);

 	return good_block_ratio;
}


int NAND_UbootInit(int boot_mode)
{
	int ret = 0;
	//int enable_bad_block_scan_flag = 0;
	//uint good_block_ratio=0;

	printf("NAND_UbootInit start\n");
		
	NAND_set_boot_mode(boot_mode);
	
    /* logic init */
	ret |= NAND_LogicInit(boot_mode);
	if(!boot_mode)
	{
		if(!flash_scaned)
		{
			NAND_GetParam((boot_nand_para_t *)nand_para_store);
			flash_scaned = 1;
		}
	}

	debug("NAND_UbootInit end: 0x%x\n", ret);

	return ret;

}


int NAND_UbootExit(void)
{
	int ret = 0;

	debug("NAND_UbootExit \n");

	ret = NAND_LogicExit();

	return ret;
}

int NAND_UbootProbe(void)
{
	int ret = 0;

	printf("NAND_UbootProbe start\n");

    /* logic init */
	ret = NAND_PhyInit();
	NAND_PhyExit();

	printf("NAND_UbootProbe end: 0x%x\n", ret);

	return ret;

}



uint NAND_UbootRead(uint start, uint sectors, void *buffer)
{
	return NAND_LogicRead(start, sectors, buffer);
}

uint NAND_UbootWrite(uint start, uint sectors, void *buffer)
{
	return NAND_LogicWrite(start, sectors, buffer);
}

int NAND_Uboot_Erase(int erase_flag)
{
	int version_match_flag;
	int nand_erased = 0;
	__u32 read_retry_type = 0, read_retry_mode;
	__s32 ret;

	printf("erase_flag = %d\n", erase_flag);
	ret = NAND_PhyInit();
	if(ret == -1)
	{
		return -1;
	}

	read_retry_type = NAND_GetReadRetryType();
	read_retry_mode = (read_retry_type>>16)&0xff;

	if(erase_flag)
	{
		printf("erase by flag %d\n", erase_flag);
		Clean_Physic_Info();
		NAND_EraseBootBlocks();
		NAND_EraseChip();
		NAND_UpdatePhyArch();
		nand_erased = 1;
	}
	else
	{
		version_match_flag = NAND_VersionCheck();
		printf("nand version = %x\n", version_match_flag);
		NAND_EraseBootBlocks();
		if((read_retry_mode == 0x32)||(read_retry_mode == 0x33))
		{
			if((Nand_erase_flag == 0)&&((NAND_GetBlankBlockFlag()) == 1))
				NAND_EraseChip();
		}
		if(NAND_ReadPhyArch()==2)//no para
			NAND_UpdatePhyArch();
	}

	NAND_PhyExit();

	return nand_erased;
}

int NAND_Uboot_Force_Erase(void)
{
	printf("force erase\n");
	if(NAND_PhyInit())
	{
		printf("phy init fail\n");
		return -1;
	}

	NAND_EraseChip_force();

	NAND_PhyExit();

	return 0;
}

__s32  burn_boot0_1k_mode( __u32 read_retry_type, __u32 Boot0_buf )
{
    __u32 i, j, k;
//    __u32 length;
	__u32 pages_per_block;
	__u32 copies_per_block;
    __u8  oob_buf[32];
    struct boot_physical_param  para;

    printf("burn boot0 normal mode!\n");

    for(i=0;i<32;i++)
        oob_buf[i] = 0xff;

    NAND_GetVersion(oob_buf);
	if((oob_buf[0]!=0xff)||(oob_buf[1]!= 0x00))
	{
		printf("get flash driver version error!");
		goto error;
	}

	/* 检查 page count */
	pages_per_block = NAND_GetPageCntPerBlk();
	if(pages_per_block%64)
	{
		printf("get page cnt per block error %x!", pages_per_block);
		goto error;
	}

	/* cal copy cnt per bock */
	copies_per_block = pages_per_block / NAND_BOOT0_PAGE_CNT_PER_COPY;

	/* burn boot0 copys */
    for( i = NAND_BOOT0_BLK_START;  i < Uboot_Start_Block;  i++ )
    {
        printf("boot0 %x \n", i);

		/* 擦除块 */
		para.chip  = 0;
		para.block = i;
		if( PHY_SimpleErase( &para ) <0 )
		{
		    printf("Fail in erasing block %d.\n", i );
    		//continue;
    	}

        /* 在块中烧写boot0备份 */
        for( j = 0;  j < copies_per_block;  j++ )
       	{

			for( k = 0;  k < NAND_BOOT0_PAGE_CNT_PER_COPY;  k++ )
			{
				para.chip  = 0;
				para.block = i;
				para.page = j * NAND_BOOT0_PAGE_CNT_PER_COPY + k;
				para.mainbuf = (void *) (Boot0_buf + k * 1024);
				para.oobbuf = oob_buf;
				if( PHY_SimpleWrite_1K( &para ) <0)
				{
					printf("Warning. Fail in writing page %d in block %d.\n", j * NAND_BOOT0_PAGE_CNT_PER_COPY + k, i );
       			}
       		}
       	}
    }
	return 0;

error:
    return -1;
}

__s32  burn_boot0_lsb_mode(__u32 read_retry_type, __u32 Boot0_buf )
{
    __u32 i,k;
    __u8  oob_buf[32];
    __u32 page_size;
    struct boot_physical_param  para;
	void * buf;

    printf("burn boot0 lsb mode!\n");

    for(i=0;i<32;i++)
        oob_buf[i] = 0xff;

	/* get nand driver version */
    NAND_GetVersion(oob_buf);
	if((oob_buf[0]!=0xff)||(oob_buf[1]!= 0x00))
	{
		printf("get flash driver version error!");
		goto error;
	}

	/* lsb enable */
	printf("lsb enalbe \n");
	printf("read retry mode: 0x%x\n", read_retry_type);
	if( NFC_LSBInit(read_retry_type) )
	{
	    printf("lsb init failed.\n");
		goto error;
	}
	NFC_LSBEnable(0, read_retry_type);



	/* 检查 page count */
	page_size = NAND_GetPageSize();
	{
		if(page_size %1024)
		{
			printf("get flash page size error!");
			goto error;
		}
	}
	/* 检查 page count */
	if(page_size == 8192*2) //change for h27ucg8t2btr 16k pagesize
		page_size = 8192;

	/* burn boot0 */
    for( i = NAND_BOOT0_BLK_START;  i < Uboot_Start_Block;  i++ )
    {
        printf("down boot0 %x \n", i);

		/* 擦除块 */
		para.chip  = 0;
		para.block = i;
		if( PHY_SimpleErase( &para ) <0 )
		{
		    printf("Fail in erasing block %d.\n", i );
    		//continue;
    	}

        /* 在块中烧写boot0备份, lsb mode下，每个块只能写前4个page */
		for( k = 0;  k < 4;  k++ )
		{
			para.chip  = 0;
			para.block = i;
			para.page  = k;
			para.mainbuf = (void *) (Boot0_buf + k * page_size);
			para.oobbuf = oob_buf;
			if( PHY_SimpleWrite_Seq( &para ) <0 )
			{
				printf("Warning. Fail in writing page %d in block %d.\n", k, i );
   			}
   		}

    }
	
	buf = malloc(page_size);
	if(!buf){
		printf("malloc fail\n");
		goto exit;
	}	

    //check boot0
    for( i = NAND_BOOT0_BLK_START;  i < Uboot_Start_Block;  i++ )
    {
		struct boot_physical_param  para;
		__u32  k;

        printf("verify boot0 %x \n", i);

        /* 在块中烧写boot0备份, lsb mode下，每个块只能写前4个page */
		for( k = 0;  k < 4;  k++ )
		{
			para.chip  = 0;
			para.block = i;
			para.page  = k;
			para.mainbuf = (void *) (buf);
			para.oobbuf = oob_buf;
			if( PHY_SimpleRead_Seq( &para ) <0 )
			{
				printf("Warning. Fail in reading page %d in block %d.\n",  k, i );
   			}
   		}

    }
	free(buf);	
exit:
    /* lsb disable */
    NFC_LSBDisable(0, read_retry_type);
    NFC_LSBExit(read_retry_type);
	printf("lsb disalbe \n");

	return 0;

error:
    return -1;
}

__s32  burn_boot0_lsb_FF_mode(__u32 read_retry_type, __u32 Boot0_buf )
{
    __u32 i,k,j,count;
    __u8  oob_buf[64];
    __u32 page_size;
	__u8 * data_FF_buf;
	__u32 pages_per_block;
    struct boot_physical_param  para;
	struct boot_ndfc_cfg cfg;

    printf("burn boot0 lsb + FF mode!\n");

	data_FF_buf = malloc(18048);
	if(data_FF_buf == NULL)
	{
		printf("data_FF_buf malloc error!");
		return -1;
	}

	for(i=0;i<(16384+1664);i++)
		*((__u8 *)data_FF_buf + i) = 0xFF;

    for(i=0;i<64;i++)
        oob_buf[i] = 0xff;

	/* get nand driver version */
    NAND_GetVersion(oob_buf);
	if((oob_buf[0]!=0xff)||(oob_buf[1]!= 0x00))
	{
		printf("get flash driver version error!");
		goto error;
	}

	/* 检查 page count */
	pages_per_block = NAND_GetPageCntPerBlk();

	page_size = NAND_GetPageSize();
	{
		if(page_size %1024)
		{
			printf("get flash page size error!");
			goto error;
		}
	}

	/* burn boot0 */
	for( i = NAND_BOOT0_BLK_START;  i < Uboot_Start_Block;  i++ )
    {
        printf("down boot0 %x \n", i);

		/* 擦除块 */
		para.chip  = 0;
		para.block = i;
		if( PHY_SimpleErase( &para ) <0 )
		{
		    printf("Fail in erasing block %d.\n", i );
    		//continue;
    	}

		if(page_size == 16384)
		{
			/* 在块中烧写boot0备份, lsb mode下，每个块只能写前2个page */
			for( k = 0;  k < pages_per_block;  k++ )
			{
				if(k<2)
				{
					para.chip  = 0;
					para.block = i;
					para.page  = k;
					para.mainbuf = (void *) (Boot0_buf + k * page_size);
					para.oobbuf = oob_buf;
					if( PHY_SimpleWrite_Seq_16K( &para ) <0 )
					{
						printf("Warning. Fail in writing page %d in block %d.\n", k, i );
					}
				}
				else if(((k % 2 == 1)&&(k != 255)) && (k > 2))
				{
					para.chip  = 0;
					para.block = i;
					para.page  = k;
					para.mainbuf = (void *) (Boot0_buf);
					para.oobbuf = oob_buf;
					if( PHY_SimpleWrite_Seq_16K( &para ) <0 )
					{
						printf("Warning. Fail in writing page %d in block %d.\n", k, i );
					}
				}
				else
				{
					para.chip  = 0;
					para.block = i;
					para.page  = k;
					para.mainbuf = (void *) data_FF_buf ;
					para.oobbuf = oob_buf;
					if( PHY_SimpleWrite_0xFF( &para ) <0 )
					{
						printf("Warning. Fail in writing page %d in block %d.\n", k, i );
					}
				}
				
			}
		}
		else if(page_size == 8192)
		{
			oob_buf[0] = 0xff;
			oob_buf[1] = 0x00;
			oob_buf[2] = 0x03;
			oob_buf[3] = 0x01;
			for( j = 0;  j < (pages_per_block/NAND_BOOT0_PAGE_CNT_PER_COPY);  j++ )
	       	{
				count = 0;

				for( k = 0;  k < NAND_BOOT0_PAGE_CNT_PER_COPY;  k++ )
				{
									
					para.chip  = 0;
					para.block = i;
					para.page = j * NAND_BOOT0_PAGE_CNT_PER_COPY + k;
					para.oobbuf = oob_buf;
					
					
					if(((para.page % 2 == 1)&&(para.page != 255)) || (para.page == 0))
					{
						cfg.ecc_mode = 8;
						cfg.page_size_kb = (page_size/1024) - 1;
						cfg.sequence_mode = 1;
						para.mainbuf = (void *) (Boot0_buf + count * 4096);
						if( PHY_SimpleWrite_CFG( &para , &cfg) <0)
						{
							printf("Warning. Fail in writing page %d in block %d.\n", para.page, i );
		       			}
						count ++;
					}
					else
					{
						para.mainbuf = (void *) data_FF_buf ;
						if( PHY_SimpleWrite_0xFF_8K( &para ) <0 )
						{
							printf("Warning. Fail in writing page %d in block %d.\n", k, i );
						}
					}
					
	       		}
	       	}
		}
        
    }

	free(data_FF_buf);
	return 0;

error:
    return -1;
}

__s32  burn_boot0_1k_fullpage_mode(__u32 read_retry_type, __u32 Boot0_buf )
{
    __u32 i,k,j;
    __u8  oob_buf[64];
    __u32 page_size;
	__u32 pages_per_block;
    struct boot_physical_param  para;
	struct boot_ndfc_cfg cfg;

    printf("burn boot0 1k fullpage mode!\n");

    for(i=0;i<64;i++)
        oob_buf[i] = 0xff;

	/* get nand driver version */
    NAND_GetVersion(oob_buf);
	if((oob_buf[0]!=0xff)||(oob_buf[1]!= 0x00))
	{
		printf("get flash driver version error!");
		goto error;
	}

	/* 检查 page count */
	pages_per_block = NAND_GetPageCntPerBlk();

	page_size = NAND_GetPageSize();
	{
		if(page_size %1024)
		{
			printf("get flash page size error!");
			goto error;
		}
	}

	/* burn boot0 */
    for( i = NAND_BOOT0_BLK_START;  i < Uboot_Start_Block;  i++ )
    {
        printf("down boot0 %x \n", i);

		/* 擦除块 */
		para.chip  = 0;
		para.block = i;
		if( PHY_SimpleErase( &para ) <0 )
		{
		    printf("Fail in erasing block %d.\n", i );
    		//continue;
    	}

		oob_buf[0] = 0xff;
		oob_buf[1] = 0x00;
		oob_buf[2] = 0x03;
		oob_buf[3] = 0x01;
		for( j = 0;  j < (pages_per_block/NAND_BOOT0_PAGE_CNT_PER_COPY);  j++ )
       	{
			for( k = 0;  k < NAND_BOOT0_PAGE_CNT_PER_COPY;  k++ )
			{				
				para.chip  = 0;
				para.block = i;
				para.page = j * NAND_BOOT0_PAGE_CNT_PER_COPY + k;
				para.oobbuf = oob_buf;				
				
				cfg.ecc_mode = 8;
				cfg.page_size_kb = (page_size/1024)-1;
				cfg.sequence_mode = 1;
				para.mainbuf = (void *) (Boot0_buf + k * 1024);
				if( PHY_SimpleWrite_CFG( &para , &cfg) <0)
				{
					printf("Warning. Fail in writing page %d in block %d.\n", para.page, i );
       			}
       		}
       	} 
    }

	return 0;

error:
    return -1;
}


int NAND_BurnBoot0(uint length, void *buffer)
{
	__u32 read_retry_type = 0, read_retry_mode;
	//int blk_index, page_index;
	//int page_cnt_per_block;

	read_retry_type = NAND_GetReadRetryType();
	read_retry_mode = (read_retry_type>>16)&0xff;
	if( (read_retry_type>0)&&(read_retry_mode < 0x10))
	{
		if((read_retry_mode == 0x4)||(read_retry_mode == 0x5))
		{
			msg("NAND_BurnBoot0:burn_boot0_lsb_FF_mode start\n");
			if( burn_boot0_lsb_FF_mode(read_retry_type, (__u32)buffer) )
		        goto error;
		}
		else
		{
			if( burn_boot0_lsb_mode(read_retry_type, (__u32)buffer) )
	        	goto error;
		}
	    
	}
	else
	{
	    if( burn_boot0_1k_fullpage_mode(read_retry_type, (__u32)buffer) )
	        goto error;
	}

	return 0;

error:
    return -1;

}


__s32 burn_uboot_in_one_blk(__u32 UBOOT_buf, __u32 length)
{
    __u32 i, k;
    __u8  oob_buf[32];
    __u32 page_size, pages_per_block, pages_per_copy;
    struct boot_physical_param  para;
    void* buf;

     printf("burn uboot normal mode!\n");
     //printf("uboot_buf: 0x%x \n", UBOOT_buf);

    for(i=0;i<32;i++)
        oob_buf[i] = 0xff;

	/* get nand driver version */
    NAND_GetVersion(oob_buf);
    if((oob_buf[0]!=0xff)||(oob_buf[1]!= 0x00))
	{
		printf("get flash driver version error!");
		goto error;
	}


	/* 检查 page count */
	page_size = NAND_GetPageSize();
	{
		if(page_size %1024)
		{
			printf("get flash page size error!");
			goto error;
		}
	}

	/* 检查 page count */
	pages_per_block = NAND_GetPageCntPerBlk();
	/*if(pages_per_block%64)
	{
		debug("get page cnt per block error %x!", pages_per_block);
		goto error;
	}*/

	printf("pages_per_block: 0x%x\n", pages_per_block);

	/* 计算每个备份所需page */
	/*if(length%page_size)
	{
		debug("uboot length check error!\n");
		goto error;
	}*/
	pages_per_copy = length/page_size;
	if(length%page_size)
		pages_per_copy++;
		
	if(pages_per_copy>pages_per_block)
	{
		printf("pages_per_copy check error!\n");
		goto error;
	}

	printf("pages_per_copy: 0x%x\n", pages_per_copy);

	//while((*(volatile unsigned int *)0) != 0x1234);
	/* burn uboot */
	printf("fill uboot(normal mode) block with dummy data\n");
    for( i = Uboot_Start_Block;  i < Uboot_Next_Block;  i++ )
    {
        printf("uboot %d \n", i);

		/* 擦除块 */
		para.chip  = 0;
		para.block = i;
		if( PHY_SimpleErase( &para ) <0 )
		{
		    printf("Fail in erasing block %d.\n", i );
    		//continue;
    	}

        /* 在块中烧写boot0备份, lsb mode下，每个块只能写前4个page */
		for( k = 0;  k < pages_per_copy;  k++ )
		{
			para.chip  = 0;
			para.block = i;
			para.page  = k;
			para.mainbuf = (void *) (UBOOT_buf + k * page_size);
			para.oobbuf = oob_buf;
			//printf("burn uboot: block: 0x%x, page: 0x%x, mainbuf: 0x%x, maindata: 0x%x \n", para.block, para.page, (__u32)para.mainbuf, *((__u32 *)para.mainbuf));
			if( PHY_SimpleWrite( &para ) <0 )
			{
				printf("Warning. Fail in writing page %d in block %d.\n", k, i );
   			}
   		}
		
		for( k = pages_per_copy;  k < pages_per_block;  k++ )
		{
			para.chip  = 0;
			para.block = i;
			para.page  = k;
			para.mainbuf = (void *) (UBOOT_buf);
			para.oobbuf = oob_buf;
			//printf("burn uboot: block: 0x%x, page: 0x%x, mainbuf: 0x%x, maindata: 0x%x \n", para.block, para.page, (__u32)para.mainbuf, *((__u32 *)para.mainbuf));
			if( PHY_SimpleWrite( &para ) <0 )
			{
				printf("Warning. Fail in writing page %d in block %d.\n", k, i );
   			}
   		}		
    }

	memset(oob_buf, 0, 32);
	
	buf = malloc(page_size);
	if(!buf){
		printf("malloc fail\n");
		goto exit;
	}		
    //check uboot
    for( i = Uboot_Start_Block;  i < Uboot_Next_Block;  i++ )
    {
	    printf("verify uboot blk %x \n", i);

        /* 擦除块 */

      	for( k = 0;  k < pages_per_copy;  k++ )
		{
			para.chip  = 0;
			para.block = i;
			para.page  = k;
			para.mainbuf = (void *) (buf);
			para.oobbuf = oob_buf;
			//printf("burn uboot: block: 0x%x, page: 0x%x, mainbuf: 0x%x, maindata: 0x%x \n", para.block, para.page, (__u32)para.mainbuf, *((__u32 *)para.mainbuf));

			if( PHY_SimpleRead( &para ) <0 )
			{
				printf("Warning. Fail in read page %d in block %d.\n", k, i );
   			}
   		}
    }
	free(buf);
exit:

	return 0;

error:
    return -1;
}

__s32 burn_uboot_in_many_blks(__u32 UBOOT_buf, __u32 length)
{
     __u32 i, k;
    __u8  oob_buf[32];
    __u32 page_size, pages_per_block, pages_per_copy, page_index;
    struct boot_physical_param  para;

     printf("burn_uboot_in_many_blks!\n");

    for(i=0;i<32;i++)
        oob_buf[i] = 0xff;

	/* get nand driver version */
    NAND_GetVersion(oob_buf);
	if((oob_buf[0]!=0xff)||(oob_buf[1]!= 0x00))
	{
		printf("get flash driver version error!");
		goto error;
	}


	/* 检查 page count */
	page_size = NAND_GetPageSize();
	{
		if(page_size %1024)
		{
			printf("get flash page size error!");
			goto error;
		}
	}

	/* 检查 page count */
	pages_per_block = NAND_GetPageCntPerBlk();
	/*if(pages_per_block%64)
	{
		printf("get page cnt per block error %x!", pages_per_block);
		goto error;
	}*/

	/* 计算每个备份所需page */
	/*if(length%page_size)
	{
		printf("uboot length check error!\n");
		goto error;
	}*/
	pages_per_copy = length/page_size;
	if(length%page_size)
		pages_per_copy++;
		
	if(pages_per_copy<=pages_per_block)
	{
		printf("pages_per_copy check error!\n");
		goto error;
	}


	/* burn uboot */
	page_index = 0;
    for( i = Uboot_Start_Block;  i < Uboot_Next_Block;  i++ )
    {
        printf("uboot %d \n", i);

		/* 擦除块 */
		para.chip  = 0;
		para.block = i;
		if( PHY_SimpleErase( &para ) <0 )
		{
		    printf("Fail in erasing block %d.\n", i );
    		continue;
    	}

        /* 在块中烧写boot0备份, lsb mode下，每个块只能写前4个page */
		for( k = 0;  k < pages_per_block;  k++ )
		{
			para.chip  = 0;
			para.block = i;
			para.page  = k;
			if(page_index >= pages_per_copy)
				para.mainbuf = (void *) (UBOOT_buf);
			else
				para.mainbuf = (void *) (UBOOT_buf + page_index* page_size);
			para.oobbuf = oob_buf;
			if( PHY_SimpleWrite( &para ) <0 )
			{
				printf("Warning. Fail in writing page %d in block %d.\n", k, i );
   			}
   			page_index++;

   			//if(page_index >= pages_per_copy)
   				//break;
   		}

   		if(page_index >= pages_per_copy)
   			page_index = 0;

    }

    //if(page_index >= pages_per_copy)
	return 0;
	//else
	//	goto error;

error:
    return -1;
}


int NAND_BurnUboot(uint length, void *buffer)
{
	int ret = 0;
//	int blk_index, page_index;
	__u32 page_size, pages_per_block, block_size;
	__u32 package_size;
	__u32 *temp_buf = NULL;

	temp_buf = (__u32 *)malloc((length+32*1024));
    if(!temp_buf) {
    	printf("malloc for temp_buf failed.\n");
    	goto error;
    }

	memcpy(temp_buf, buffer, length);
	
	package_size = Physic_Info_Add_To_Uboot_Tail(temp_buf, length);

	/* 检查 page count */
	page_size = NAND_GetPageSize();
	{
		if(page_size %1024)
		{
			printf("get flash page size error!\n");
			goto error;
		}
	}

	/* 检查 page count */
	pages_per_block = NAND_GetPageCntPerBlk();
	/*if(pages_per_block%64)
	{
		printf("get page cnt per block error %x!\n", pages_per_block);
		goto error;
	}*/

	block_size = page_size*pages_per_block;
	/*if(length%page_size)
	{
		printf(" uboot length check error!\n");
		goto error;
	}*/

	if(package_size<=block_size)
	{
		ret = burn_uboot_in_one_blk((__u32)temp_buf, package_size);
	}
	else
	{
		ret = burn_uboot_in_many_blks((__u32)temp_buf, package_size);
	}

	free(temp_buf);
	return ret;

error:
	return -1;

}

int NAND_GetParam_store(void *buffer, uint length)
{
	if(!flash_scaned)
	{
		printf("sunxi flash: force flash init to begin hardware scanning\n");
		NAND_PhyInit();
		NAND_PhyExit();
		printf("sunxi flash: hardware scan finish\n");
	}
	NAND_GetParam((boot_nand_para_t *)nand_para_store);
	memcpy(buffer, nand_para_store, length);

	return 0;
}

int NAND_FlushCache(void)
{
	return nftl_flush_write_cache();
}

