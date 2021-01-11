/*
 * Copyright (c) 2016-2017 HiSilicon Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <common.h>
#include <linux/mtd/mtd.h>
#include "spi_ids.h"

#define SPI_DRV_VERSION       "2.00"
/*****************************************************************************/

#if 1
#  define DBG_MSG(_fmt, arg...)
#else
#  define DBG_MSG(_fmt, arg...)   \
	printf("%s(%d): " _fmt, __FILE__, __LINE__, ##arg);
#endif

#ifndef DBG_BUG
#define DBG_BUG(fmt, args...) do{\
	printf("%s(%d): BUG !!! " fmt, __FILE__, __LINE__, ##args); \
	while(1); \
} while (0)
#endif

/*****************************************************************************/
struct spi_tag spitag[1] = {{{0},{0},0,0,0,0,{{0},},{{0},},{{0},}}};

struct spi_info *spi_serach_ids(struct spi_info *spi_info_table,
	unsigned char ids[8])
{
	struct spi_info *info;
	struct spi_info *fit_info = NULL;

	for (info = spi_info_table; info->name; info++) {
		if (memcmp(info->id, ids, info->id_len))
			continue;

		if (fit_info == NULL
			|| fit_info->id_len < info->id_len)
			fit_info = info;
	}

	if (fit_info != NULL) {
		int ix;
		strncpy(spitag->name, fit_info->name, 16);
		spitag->name[sizeof(spitag->name)-1] = '\0';

		memcpy(spitag->id, fit_info->id, 8);
		spitag->id_len = fit_info->id_len;

		spitag->chipsize  = fit_info->chipsize;
		spitag->erasesize = fit_info->erasesize;
		spitag->addrcycle = fit_info->addrcycle;

		for (ix = 0; ix < MAX_SPI_OP; ix++) {
			if (fit_info->read[ix] != NULL)
				memcpy(&spitag->read[ix],
					fit_info->read[ix],
					sizeof(struct spi_operation));
		}
		for (ix = 0; ix < MAX_SPI_OP; ix++) {
			if (fit_info->write[ix] != NULL)
				memcpy(&spitag->write[ix],
					fit_info->write[ix],
					sizeof(struct spi_operation));
		}
		for (ix = 0; ix < MAX_SPI_OP; ix++) {
			if (fit_info->erase[ix] != NULL)
				memcpy(&spitag->erase[ix],
					fit_info->erase[ix],
					sizeof(struct spi_operation));
		}
	}

	return fit_info;
}
/*****************************************************************************/

void spi_search_rw(struct spi_info *spiinfo, struct spi_operation *spiop_rw,
	unsigned int iftype, unsigned int max_dummy, int is_read)
{
	int ix = 0;
	struct spi_operation **spiop, **fitspiop;

	for (fitspiop = spiop = (is_read ? spiinfo->read : spiinfo->write);
		(*spiop) && ix < MAX_SPI_OP; spiop++, ix++) {
		DBG_MSG("dump[%d] %s iftype:0x%02X\n", ix,
			(is_read ? "read" : "write"),
			(*spiop)->iftype);

		if (((*spiop)->iftype & iftype)
			&& ((*spiop)->dummy <= max_dummy)
			&& (*fitspiop)->iftype < (*spiop)->iftype)
			fitspiop = spiop;
	}
	memcpy(spiop_rw, (*fitspiop), sizeof(struct spi_operation));
}
/*****************************************************************************/
#ifndef CONFIG_SPI_FLASH_HISFC300
void spi_get_erase(struct spi_info *spiinfo, struct spi_operation *spiop_erase)
{
	int ix;

	spiop_erase->size = 0;
	for (ix = 0; ix < MAX_SPI_OP; ix++) {
		if (spiinfo->erase[ix] == NULL)
			break;
		if (spiinfo->erasesize == spiinfo->erase[ix]->size) {
			memcpy(&spiop_erase[ix], spiinfo->erase[ix],
			sizeof(struct spi_operation));

			break;
		}
	}
	if (!spiop_erase->size)
		DBG_BUG("Spi erasesize error!");
}
#else
void spi_get_erase(struct spi_info *spiinfo, struct spi_operation *spiop_erase,
	unsigned int *erasesize)
{
	int ix;

	(*erasesize) = spiinfo->erasesize;
	for (ix = 0; ix < MAX_SPI_OP; ix++) {
		if (spiinfo->erase[ix] == NULL)
			break;

		memcpy(&spiop_erase[ix], spiinfo->erase[ix],
			sizeof(struct spi_operation));

		switch (spiop_erase[ix].size) {
		case SPI_IF_ERASE_SECTOR:
			spiop_erase[ix].size = spiinfo->erasesize;
			break;
		case SPI_IF_ERASE_CHIP:
			spiop_erase[ix].size = spiinfo->chipsize;
			break;
		}
		DBG_MSG("erase[%d]->erasesize:%s, cmd:0x%02X\n",
			ix, ultohstr(spiop_erase[ix].size), spiop_erase[ix].cmd);

		if ((int)(spiop_erase[ix].size) < _2K) {
			DBG_BUG("erase block size mistaken: spi->erase[%d].size:%s\n",
				ix, ultohstr(spiop_erase[ix].size));
		}

		if (spiop_erase[ix].size < (*erasesize))
			(*erasesize) = spiop_erase[ix].size;
	}
}
#endif
/*****************************************************************************/

int spi_ids_init(void)
{
	printf("Spi id table Version %s\n", SPI_DRV_VERSION);
	return 0;
}
/*****************************************************************************/

void spi_ids_exit(void)
{
}
/*****************************************************************************/

