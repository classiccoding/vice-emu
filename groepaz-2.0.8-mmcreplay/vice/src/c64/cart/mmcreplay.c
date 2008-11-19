/*
 * mmcreplay.c - Cartridge handling, MMCReplay cart.
 *
 * Written by
 *  Groepaz/Hitmen <groepaz@gmx.net>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

/* indent -gnu -bls -hnl -nut -sc -cli4 -npsl -i4 -bli0 -cbi0 -ci4 -di8 -l80 */

#include "vice.h"

#include <stdio.h>
#include <string.h>

#include "maincpu.h"
#include "c64cart.h"
#include "c64cartmem.h"
#include "c64mem.h"
#include "c64export.h"
#include "c64io.h"
#include "mmcreplay.h"
#include "par-flashrom.h"
#include "ser-eeprom.h"
#include "spi-sdcard.h"
#include "reu.h"
#ifdef HAVE_TFE
#include "tfe.h"
#endif
#include "types.h"
#include "util.h"
#include "log.h"
#include "machine.h"
#include "vicii-phi1.h"

#define DEBUG
#define DEBUG_LOGBANKS          /* log access to banked rom/ram */
#define DEBUG_IOBANKS           /* log access to banked rom/ram in io */

/* #define LOG_CLOCKPORT */     /* log clockport i/o */

//#define TEST_AR_MAPPER /* ok */
//#define TEST_RR_MAPPER /* ok */
//#define TEST_NORDIC_MAPPER
//#define TEST_SUPER_8KCRT /* ok */
//#define TEST_SUPER_16KCRT /* ok */

//#define TEST_RESCUE_MODE

#ifdef DEBUG
#define LOG(_x_) log_debug _x_
#else
#define LOG(_x_)
#endif

static const c64export_resource_t export_res = {
    "MMC Replay", 1, 1
};

int     mmcr_enabled = 1;       // FIXME
int     enable_rescue_mode = 0;

/*
Features

512KB FLASH rom
512KB SRAM
1KB Serial EEPROM for configuration storage
SPI Interface to access MMC / Secure Digital cards
Clockport to connect additional hardware
Freezer functionality (Action Replay / Retro Replay compatible)
Reset button
*/

static unsigned int active_mode_phi1 = 0;
static unsigned int active_mode_phi2 = 0;

/* RAM banking.*/
static unsigned int raml_bank = 0, ramh_bank = 0;
static unsigned int enable_raml = 0;
static unsigned int enable_ramh = 0;

static int ramA000_bank = 0;
static unsigned int enable_ramA000 = 0;

static int romA000_bank = 0;

/* IO banking.*/
static unsigned int io1_bank = 0, io2_bank = 0;
static unsigned int io1_ram_bank = 0, io2_ram_bank = 0;
static unsigned int enable_io1 = 0;
static unsigned int enable_io2 = 0;
static unsigned int enable_io1_ram = 0;
static unsigned int enable_io2_ram = 0;

/*** $DE00:	RR control register write */
/* bit 0:	GAME line (W) */
static unsigned int enable_game = 0;    /* (w) status of game line */
static unsigned int enable_flash_write = 1;     /* (r) */
/* bit 1:	EXROM line (W) */
static unsigned int enable_exrom = 0;   /* status of exrom line */
/* bit 2:	1 = disable RR (W) */
/*              (*) bit can be reset by setting bit 6 of $df12 */
static unsigned int rr_active = 0;      /* (w) Cart is activated.  */
static unsigned int freeze_pressed = 0; /* (r) freeze button pressed.  */
/* ^ bit 3:	bank address 13 (W) */
/* ^ bit 4:	bank address 14 (W) */
/* bit 5:	0 = rom enable, 1 = ram enable (W) */
static unsigned int enable_ram_io = 0;
/* bit 6:	1 = exit freeze mode (W) */
static unsigned int enable_freeze_exit = 0;
/* bit 7:	bank address 15 (W) */
static unsigned int bank_address_13_15 = 0;

/*** $DE01:	extended RR control register write */
/* bit 0:	0 = disable clockport, 1 = enable clockport (W) */
int     mmcr_clockport_enabled = 0;     /* used globally, eg in c64io.c */
/* bit 1:	0 = disable io ram banking, 1 = enable io ram banking (W) */
static unsigned int allow_bank = 1;     /* RAM bank switching allowed.  */
/* bit 2:	0 = enable freeze, 1 = disable freeze (W) */
static unsigned int no_freeze = 0;      /* Freeze is disallowed.  */
/* ^ bit 3:	bank address 13 (mirror of $DE00) (W) */
/* ^ bit 4:	bank address 14 (mirror of $DE00) (W) */
/* bit 5:	0 = enable MMC registers, 1 = disable MMC registers (W) */
/*              (*) Can only be written when bit 6 of $DF12 is 1. Register becomes effective
		when bit 0 of $DF11 is 1.*/
static unsigned int enable_mmc_regs = 0;
static unsigned int enable_mmc_regs_pending = 0;
/* bit 6:	0 = ram/rom @ DFxx, 1 = ram/rom @ $DExx (W) */
static unsigned int enable_ram_io1 = 0; /* REU compatibility mapping.  */
/* ^ bit 7:	bank address 15 (mirror of $DE00) (W) */

/* Only one write access is allowed.  */
static unsigned int write_once;

/* ^ $DE02 - $DE0F: Clockport memory area (when enabled) */

/* ^ $DF10:	MMC SPI transfer register */
/*		byte written is sent to the card & response from the card is read here */

/**** $DF11:	MMC control register */
/* 		bit 0:	0 = MMC BIOS enabled, 1 = MMC BIOS disabled (R/W) (*) */
/*              (*) 	Enabling MMC Bios sets ROM banking to the last 64K bank */
static unsigned int disable_mmc_bios = 0;
/* ^		bit 1:	0 = card selected, 1 = card not selected (R/W) (**) */
/*              (**) 	This bit also controls the green activity LED */
/* ^		bit 2:	0 = 250khz transfer, 1 = 8mhz transfer (R/W) */
/* ^		bit 3:	** ALWAYS 0 ** */
/* ^		bit 4:	** ALWAYS 0 ** */

/* 		bit 5:	(in RR-Mode:) 0 = allow RR rom when MMC BIOS disabled , 1 = disable RR ROM (R/W)	(***) */
/*              (***)	When in mmcreplay bios mode, bit 5 controls RAM banking (0 = $e000 - $ffff, 1 = $8000 - $9fff)
			When in 16K mode, bit 5 enables RAM at $a000 - $bfff */
static unsigned int disable_rr_rom = 0;

/* ^		bit 6:	0 = SPI write trigger mode, 1 = SPI read trigger mode (R/W) */
/* ^		bit 7:	** ALWAYS 0 ** */

/***** $DF12:	MMC status register */
/* 		bit 0:	0 = SPI ready, 1 = SPI busy (R)	*/
/* 		1 = forbid ROM write accesses (W) (*) */
/*		(*) Setting this bit will disable writes to ROM until next reset */
static unsigned int disable_rom_write = 0;      /* (w) disables rom write until reset */

/* ^		bit 1:	feedback of $DE00 bit 0 (GAME) (R)					*/
/* ^		bit 2:	feedback of $DE00 bit 1 (EXROM) (R)*/
/* ^		bit 3:	0 = card inserted, 1 = no card inserted (R)*/
/* ^		bit 4:  0 = card write enabled, 1 = card write disabled (R)*/

/* ^		bit 5:	EEPROM DATA line / DDR Register (R/W) (**)*/
/*              (**) Setting DATA to "1" enables reading data bit from EEPROM at this position.*/
/* 		bit 6:	0 = RR compatibility mode, 1 = Extended mode (W) (***)*/
/*              (***) Selecting RR compatibility mode limits RAM to 32K and
		disables writes to extended banking register.
		Selecting Extended mode enables full RAM banking and enables
		Nordic Power mode in RR mode. */
static unsigned int enable_extended_mode = 0;   /* bit 6 */
/* ^		bit 7:	EEPROM CLK line (W)*/


/***** $DF13:	Extended banking register (*)*/
/*     (*) Can only be read/written to when bit 6 of $DF12 is 1 */
/* ^		bit 0:	bank address 16	(R/W)*/
/* ^		bit 1:	bank address 17 (R/W)*/
/* 		bit 2:	bank address 18 (R/W)*/
static unsigned int bank_address_16_18 = 7;

/* ^		bit 3:	** ALWAYS 0 ***/
/* ^		bit 4:	** ALWAYS 0 ***/
/* 		bit 5:	16K rom mapping (R/W)*/
static unsigned int enable_16k_mapping = 0;
/* 		bit 6:	1 = enable RR register*/
/*              Disabling RR Register disables ALL ROM/RAM banking too. */
static unsigned int enable_rr_regs = 1;
/* ^		bit 7:	** ALWAYS 0 ***/


/********************************************************************************************************************/

static char dumpstr1[0x100];
static char dumpstr2[0x100];
//static char dumpstr3[0x100];

void mmcreplay_dump_cfg (void)
{
    static char ndumpstr[0x100];
    char   *str_mapper[4] = { "MMCBIOS", "RR", "SUPER", "NORMAL" };
    char   *str_config[4] = { "off", "ultimax", "8K Game", "16K Game" };
    int     mapper, config;
#if 0
    LOG (("MMCREPLAY: GAME:%d           EXROM:%d", enable_game, enable_exrom));
    LOG (("MMCREPLAY: rr active: %d  rr regs:%d        mmc bios:%d", rr_active,
          enable_rr_regs, disable_mmc_bios));
    LOG (("MMCREPLAY: extended mode:%d  16k mapping:%d", enable_extended_mode,
          enable_16k_mapping));
    LOG (("MMCREPLAY: disable rom write:%d", disable_rom_write));
    LOG (("MMCREPLAY: ROM Addr: %02x (%d:%d)",
          (bank_address_16_18 << 3) | bank_address_13_15, bank_address_16_18,
          bank_address_13_15));
    LOG (("MMCREPLAY: effective ROM Bank LO: %02x (%d:%d)", roml_bank,
          roml_bank >> 3, roml_bank & 7));
    LOG (("MMCREPLAY: effective ROM Bank HI: %02x (%d:%d)", romh_bank,
          romh_bank >> 3, romh_bank & 7));
    LOG (("MMCREPLAY: RAM REU Mapping:%d", enable_ram_io1));
    LOG (("MMCREPLAY: effective RAM Bank LO: %02x (%d:%d)", raml_bank,
          raml_bank >> 3, raml_bank & 7));
    LOG (("MMCREPLAY: effective RAM Bank HI: %02x (%d:%d)", ramh_bank,
          ramh_bank >> 3, ramh_bank & 7));
#endif
#if 0
    {
        char    str[0x100];
        strcpy (str, "ROML: ");
        if (enable_raml)
            strcat (str, "RAM");
        else
            strcat (str, "ROM");

        strcat (str, " ROMH: ");
        if (enable_ramh)
            strcat (str, "RAM");
        else
            strcat (str, "ROM");
        if (enable_ram_io1)
            strcat (str, " RAMIO1: ");
        else
            strcat (str, " RAMIO2: ");
        if (enable_ram_io)
            strcat (str, "RAM");
        else
            strcat (str, "ROM");
        LOG (("MMCREPLAY: %s", str));
    }
#endif
//    LOG(("MMCREPLAY: cart active: %d rr rom active:%d rr regs active:%d  extended mode:%d  16k mapping:%d RAM IO1 Mapping:%d RAM Banking:%d",rr_active,disable_rr_rom^1,enable_rr_regs,enable_extended_mode,enable_16k_mapping,enable_ram_io1,allow_bank));   
    sprintf (ndumpstr,
             "MMCREPLAY: cart active: %d disable rr rom/opt.mapping:%d rr regs active:%d  extended mode:%d  16k mapping:%d RAM IO1 Mapping:%d RAM Banking:%d",
             rr_active, disable_rr_rom, enable_rr_regs, enable_extended_mode,
             enable_16k_mapping, enable_ram_io1, allow_bank);
    if (strcmp (dumpstr1, ndumpstr) != 0)
    {
        strcpy (dumpstr1, ndumpstr);
        LOG ((dumpstr1));
    }

//      config=export.game|(export.exrom<<1);

//          LOG(("MMCREPLAY: mode phi1 %02x exgm:%02x",active_mode_phi1,config));       
    config = active_mode_phi1;
//          LOG(("MMCREPLAY: %d",export.exrom));        
//          LOG(("MMCREPLAY: %d",config));      
    if (disable_mmc_bios == 0)
    {
        /* mmc bios mapper */
        mapper = 0;
    }
    else if (enable_16k_mapping)
    {
        /* super mapper */
        mapper = 2;
    }
    else if (disable_rr_rom)
    {
        /* normal mapper */
        mapper = 3;
    }
    else
    {
        /* rr mapper */
        mapper = 1;
    }


/*
  mode: (exrom<<1)|game
  	0          0
  	0          1            ultimax   roml ($8000),romh ($e000)
  	1          0            8k game   roml ($8000)
  	1          1		16k game  roml ($8000),romh ($a000)
*/
    {
        char    str[0x100];
        char    romlbank[0x100];
        char    romhbank[0x100];
        char    ramlbank[0x100];
        char    ramhbank[0x100];
        char    ramA000bank[0x100];
        char    romA000bank[0x100];
        char    io1bank[0x100];
        char    io2bank[0x100];
        char    io1rambank[0x100];
        char    io2rambank[0x100];
        sprintf (romlbank, "(%d:%d)", roml_bank >> 3, roml_bank & 7);
        sprintf (romhbank, "(%d:%d)", romh_bank >> 3, romh_bank & 7);
        sprintf (ramlbank, "(%d:%d)", raml_bank >> 3, raml_bank & 7);
        sprintf (ramhbank, "(%d:%d)", ramh_bank >> 3, ramh_bank & 7);
        /* a000- bf00 */
        sprintf (ramA000bank, "(%d:%d)", ramA000_bank >> 3, ramA000_bank & 7);
        sprintf (romA000bank, "(%d:%d)", romA000_bank >> 3, romA000_bank & 7);  /* FIXME: always = raml bank? */

        sprintf (io1bank, "(%d:%d)", io1_bank >> 3, io1_bank & 7);
        sprintf (io2bank, "(%d:%d)", io2_bank >> 3, io2_bank & 7);
        sprintf (io1rambank, "(%d:%d)", io1_ram_bank >> 3, io1_ram_bank & 7);
        sprintf (io2rambank, "(%d:%d)", io2_ram_bank >> 3, io2_ram_bank & 7);
        str[0] = 0;
        switch (config)
        {
            case 0:            /* off */
                strcat (str, "---");
                break;
            case 1:            /* ultimax */
                if (enable_raml)
                {
                    strcat (str, "RAML");
                    strcat (str, ramlbank);
                }
                else
                {
                    strcat (str, "ROML");
                    strcat (str, romlbank);
                }
                strcat (str, "($8000)");
                if (enable_ramA000)
                {
                    strcat (str, "RAMA000");
                    strcat (str, ramA000bank);
                }
                else
                {
                    strcat (str, "ROMA000");
                    strcat (str, romA000bank);
                }
                strcat (str, "($a000)");
                if (enable_ramh)
                {
                    strcat (str, "RAMH");
                    strcat (str, ramhbank);
                }
                else
                {
                    strcat (str, "ROMH");
                    strcat (str, romhbank);
                }
                strcat (str, "($e000)");
                break;
            case 2:
                if (enable_raml)
                {
                    strcat (str, "RAML");
                    strcat (str, ramlbank);
                }
                else
                {
                    strcat (str, "ROML");
                    strcat (str, romlbank);
                }
                strcat (str, "($8000)");
                break;
            case 3:
                if (enable_raml)
                {
                    strcat (str, "RAML");
                    strcat (str, ramlbank);
                }
                else
                {
                    strcat (str, "ROML");
                    strcat (str, romlbank);
                }
                strcat (str, "($8000)");
                if (enable_ramh)
                {
                    strcat (str, "RAMH");
                    strcat (str, ramhbank);
                }
                else
                {
                    strcat (str, "ROMH");
                    strcat (str, romhbank);
                }
                strcat (str, "($a000)");
                break;
        }
        strcat (str, " IO1: ");
        if (enable_io1)
        {
            if (enable_ram_io)
            {
                strcat (str, "RAM ");
                strcat (str, io1rambank);
            }
            else
            {
                strcat (str, "ROM ");
                strcat (str, io1bank);
            }
        }
        else
        {
            strcat (str, " off ");
        }

        strcat (str, " IO2: ");
        if (enable_io2)
        {
            if (enable_ram_io)
            {
                strcat (str, "RAM ");
                strcat (str, io2rambank);
            }
            else
            {
                strcat (str, "ROM ");
                strcat (str, io2bank);
            }
        }
        else
        {
            strcat (str, " off ");
        }
/*    
	    if(enable_ram_io1) strcat(str," IO1: ");
	    else strcat(str," IO2: ");
	    if(enable_ram_io) strcat(str,"RAM");
	    else strcat(str,"ROM");
*/
//LOG(("MMCREPLAY: [%s] %s %s",str_mapper[mapper],str_config[config],str));
        sprintf (ndumpstr, "MMCREPLAY: [%s] %s %s", str_mapper[mapper],
                 str_config[config], str);
        if (strcmp (dumpstr2, ndumpstr) != 0)
        {
            strcpy (dumpstr2, ndumpstr);
            LOG ((dumpstr2));
        }
    }

}

/*
basic operation modes:

$df11:0 (enable_mmc_bios)
$df11:5 (disable_rr_rom)
$df13:5 (enable_16k_mapping)

                       $df11:0 5  $df13:5
C64 standard mode            1 ?        0             (*1)
MMC Replay Bios mode         0 X        X             (*2)
Retro Replay mode            1 0        0             (*3)
Super Mapper mode            1 m        1             (*4)

(*1) In this mode, GAME and EXROM are disabled. However, IO Ram/Rom banking is still active.
(*2) MMC-Replay BIOS Mode. This mode is only intended for official Bios development
(*3) Retro Replay compatible mode. This mode is designed to work exactly like a Retro Replay 
     which is plugged into a MMC64. That means GAME/EXROM is active, and RAM/ROM banking works 
     normally. Extended banking registers at $df13 are active
(*4) 16k Super Mapper Mode. This mode is designed to define an enhanced Retro Replay mode, in 
     which you have 16k ROM/RAM banking at $8000-$bfff and 512 byte Ram/Rom banking at $de00-$dfff. 
     GAME/EXROM works like in Retro Replay mode. Banking bit A13 is disabled, since only 16K banks 
     are selected in this mode. This mode also defines 2 different RAM modes. 

banking, Super Mapper mode:

Lets assume we have RAM enabled (bit 5 of $de00 = 1):

$df11: bit 5 = 0 (16k RAM/ROM mixed mode)
-----------------------------------------
$8000	----------------------------------
			8K RAM bank lo
$a000 ----------------------------------
			8K FLASH ROM bank hi
$c000 ----------------------------------

$df11: bit 5 = 1 (16k RAM mode)
-----------------------------------------
$8000	----------------------------------
			8K RAM bank lo
$a000 ----------------------------------
			8K RAM bank hi
$c000 ----------------------------------

Allowbank masks only banking bits A13-A15 in Super Mapper mode.
*/

void mmcreplay_ramhbank_set (unsigned int bank)
{
    ramh_bank = (int) bank;
}

void mmcreplay_ramlbank_set (unsigned int bank)
{
    raml_bank = (int) bank;
}

void mmcreplay_io1bank_set (unsigned int bank, unsigned int rambank)
{
    io1_bank = (int) bank;
    io1_ram_bank = (int) rambank;
}

void mmcreplay_io2bank_set (unsigned int bank, unsigned int rambank)
{
    io2_bank = (int) bank;
    io2_ram_bank = (int) rambank;
}

/*
  mode: (exrom<<1)|game
  	0          0            off
  	0          1            ultimax   roml ($8000),             romh ($e000)
  	1          0            8k game   roml ($8000)
  	1          1		16k game  roml ($8000),romh ($a000)
*/
/* FIXME: phi1 vs phi2 is probably not quite correct */
void mmcreplay_config_changed (BYTE mode_phi1,  /* game/exrom */
                               BYTE mode_phi2,  /* game/exrom */
                               unsigned int wflag, int release_freeze)
{
    if (wflag == CMODE_WRITE)
    {
        machine_handle_pending_alarms (maincpu_rmw_flag + 1);
    }
    else
    {
        machine_handle_pending_alarms (0);
    }

    export.game = mode_phi2 & 1;
//    export.exrom = ((mode_phi2 >> 1) & 1) ^ 1;
    export.exrom = ((mode_phi2 >> 1) & 1);
//    cartridge_romhbank_set((mode_phi2 >> 3) & 3);
//    cartridge_romlbank_set((mode_phi2 >> 3) & 3);
//    export_ram = (mode_phi2 >> 5) & 1;

    mem_pla_config_changed ();

#if 1
//    if (mode_phi2 & 0x40)
    if (release_freeze)
    {
        cartridge_release_freeze ();
    }

    mode_phi1 ^= 2;             /* FIXME: invert exrom ? */

    cart_ultimax_phi1 = (mode_phi1 & 1) & ((mode_phi1 >> 1) & 1);
    cart_ultimax_phi2 = export.game & (export.exrom ^ 1)
        & ((~mode_phi1 >> 2) & 1);
    machine_update_memory_ptrs ();
#endif
}

#ifdef DEBUG
int     last_mainmode = 0;
int     last_biosmode = 0;
int     last_biosmode444 = 0;
#endif

/* FIXME: mapping in freeze mode */
void mmcreplay_update_mapper_nolog (unsigned int wflag, int release_freeze)
{
//      unsigned int bank;
    unsigned int cartbankl = 0, cartbankh = 0;
    unsigned int rambankl = 0, rambankh = 0;
    unsigned int io1bank = 0, io2bank = 0;
    unsigned int io1bank_ram = 0, io2bank_ram = 0;
//      unsigned int rambankl=0,rambankh=0;

//      int mode_phi1,mode_phi2;
    int     mapped_game, mapped_exrom;
    mapped_exrom = enable_exrom;
    mapped_game = enable_game;

    enable_io1_ram = enable_ram_io;
    enable_io2_ram = enable_ram_io;

    enable_ramA000 = 0;

//      bank=(bank_address_16_18<<3)|(bank_address_13_15&6);

//    export_ram = (mode_phi2 >> 5) & 1;
    /*      df11:0  0 = mmcreplay bios mode */
    if (disable_mmc_bios == 0)
    {
        /*
         * in rescue mode GAME and EXROM are tri-stated if bit0 of DF11 is 0
         */
        if (enable_rescue_mode)
        {
            mapped_exrom = 0;
            mapped_game = 0;
            LOG (("main mode: rescue"));
        }
        else
        {


#ifdef DEBUG
            if (last_mainmode != 111)
            {
                LOG (("main mode: mmc bios"));
                last_mainmode = 111;
            }
#endif
        /**************************************************************************************************
         * mmc bios mapper
         ***************************************************************************************************/

            /*
             * "In MMC Replay Bios Mode, the ROM bank is normaly fixed to bank 7 when
             * RAM is disabled. When enabling RAM, one can select betwen 2 different
             * RAM modes usiing bit#5 of $DF11. Note that RAM enabled at $8000 is
             * read only while RAM mapped at $E000 is both read and write enabled.
             * GAME & EXROM bits have no effect, the serial EEPROM can be accessed."
             */
            cartbankl = ((7 << 3) | (bank_address_13_15)) & (0x3f);     /* always last 64k bank in mmc bios mode */
            cartbankh = cartbankl;



            rambankl = ((7 << 3) | bank_address_13_15) & (0x3f);
            rambankh = rambankl;
            io1bank = ((7 << 3) | bank_address_13_15) & (0x3f);
            io2bank = io1bank;
            /* FIXME */
            io1bank_ram = io1bank;
            io2bank_ram = io2bank;

            /* FIXME */
            mapped_game = 1;
            mapped_exrom = 1;

            enable_raml = 0;
            enable_ramh = 0;

            /* df11:5 When in mmcreplay bios mode, bit 5 controls RAM banking
             * 0 = $e000 - $ffff    (512K read/write window)
             * 1 = $8000 - $9fff    (512K read only  window)
             */
            if (disable_rr_rom) /* 1 = $8000 - $9fff (512K read only  window) */
            {
                /* 16k game mode,  16k game mode */
                mapped_game = 1;
                mapped_exrom = 1;

                if (enable_ram_io1)
                {
#ifdef DEBUG
                    if (last_biosmode != 111)
                    {
                        LOG (("bios mode: 111"));
                        last_biosmode = 111;
                    }
#endif
                    /*
                     * disable_rr_rom=1 (opt.banking)
                     * enable_ram_io1=1  (use io1+2)
                     *
                     * extended_mode=0   (1= toggle IO2 in cfg $22)
                     * allow_bank (active for ram in IO)
                     *
                     * enable_ram_io=0
                     * 00 0Xro0Xro 0Xro---- kern
                     * 00 0Xro0Xro 0Xro---- kern
                     * 00 0Xro0Xro 0Xro---- kern
                     * 00 0Xro0Xro 0Xro---- kern
                     *
                     * $8000 - $9fff      bios rom
                     * $a000 - $bfff      bios rom (mirror)
                     *
                     * enable_ram_io=1
                     * 20 0Xra0Xro 00ra00ra kern
                     * 21 0Xra0Xro 00ra00ra kern
                     * 22 0Xra0Xro 00ra00ra kern
                     * 23 0Xra0Xro 00ra00ra kern
                     *
                     * $8000 - $9fff      512K read only  ram window
                     * $a000 - $bfff      bios rom (mirror)
                     */
                    if (enable_ram_io)
                    {
                        enable_raml = 1;
                        enable_io1 = 1;
                        enable_io2 = 1;
                        if (allow_bank)
                        {
                            io1bank_ram = (7 << 3) | io1bank_ram;
                            io2bank_ram = (7 << 3) | io2bank_ram;
                        }
                        else
                        {
                            io1bank_ram = (0 << 3) | 0;
                            io2bank_ram = (0 << 3) | 0;
                        }
                    }
                    else
                    {
//                                      enable_raml=0;
                        enable_io1 = 1;
                        enable_io2 = 0;
                    }
                }
                else
                {
#ifdef DEBUG
                    if (last_biosmode != 222)
                    {
                        LOG (("bios mode: 222"));
                        last_biosmode = 222;
                    }
#endif
                    /*
                     * disable_rr_rom=1 (opt.banking)
                     * enable_ram_io1=0  (use io2 only)
                     * 
                     * extended_mode=0   (1= toggle IO2 in cfg $22)
                     * allow_bank (active for ram in IO)
                     * 
                     * enable_ram_io=0
                     * 00 0Xro0Xro----0Xrokern
                     * 00 0Xro0Xro----0Xr0kern
                     * 00 0Xro0Xro----0Xrokern
                     * 00 0Xro0Xro----0Xrokern
                     * 
                     * $8000 - $9fff      bios rom
                     * $a000 - $bfff      bios rom (mirror)
                     * 
                     * enable_ram_io=1
                     * 20 0Xra0Xro----00rakern
                     * 21 0Xra0Xro----00rakern
                     * 22 0Xra0Xro----00rakern (*)
                     * 23 0Xra0Xro----00rakern
                     * 
                     * $8000 - $9fff        (512K read only  ram window)
                     * $a000 - $bfff      bios rom (mirror)
                     */
                    if (enable_ram_io)
                    {
                        enable_raml = 1;
                        enable_io1 = 0;
                        enable_io2 = 1;
                        if (allow_bank)
                        {
                            io1bank_ram = (7 << 3) | io1bank_ram;
                            io2bank_ram = (7 << 3) | io2bank_ram;
                        }
                        else
                        {
                            io1bank_ram = (0 << 3) | 0;
                            io2bank_ram = (0 << 3) | 0;
                        }
                        if (enable_extended_mode)
                        {
                            if (((enable_game) | (enable_exrom << 1)) == 2)
                            {
                                /* ??? ROM in io? */
//                                                      enable_io2=0;
                                enable_io1_ram = 0;
                                enable_io2_ram = 0;
                            }
                        }
                    }
                    else
                    {
//                                      enable_raml=0;
                        enable_io1 = 0;
                        enable_io2 = 1;
                    }
                }



            }
            else                /* disable_rr_rom=0   $e000 - $ffff    (512K read/write window) */
            {
                /* 16k game mode,  ultimax */
                /* ultimax, ram at $e000, rom at $8000, rom at $a000 */
                mapped_game = 1;
                mapped_exrom = 1;

                if (enable_ram_io1)
                {
#ifdef DEBUG
                    if (last_biosmode != 333)
                    {
                        LOG (("bios mode: 333"));
                        last_biosmode = 333;
                    }
#endif
                    /*
                     * disable_rr_rom=0 (opt.banking)
                     * enable_ram_io1=1  (use io1+2)
                     *
                     * extended_mode=0   (not active ?)
                     * allow_bank (active for ram in both IO)
                     *
                     * enable_ram_io=0
                     * 00 0Xro0Xro 0Xr0---- kern
                     * 00 0Xro0Xro 0Xr0---- kern
                     * 00 0Xro0Xro 0Xr0---- kern
                     * 00 0Xro0Xro 0Xr0---- kern
                     *
                     * $8000 - $9fff      bios rom
                     * $a000 - $bfff      bios rom (mirror)
                     *
                     * enable_ram_io=1
                     * 20 0Xro0Xro 0Zra0Zra 0Xra
                     * 21 0Xro0Xro 0Zra0Zra 0Xra
                     * 22 0Xro0Xro 0Zra0Zra 0Xra
                     * 23 0Xro0Xro 0Zra0Zra 0Xra
                     *
                     * $8000 - $9fff      bios rom
                     * $a000 - $bfff      bios rom (mirror)
                     * $e000 - $ffff           512K read/write ram window
                     */
                    if (enable_ram_io)  /* ultimax */
                    {
                        /* ultimax, ram at $e000, rom at $8000, rom at $a000 */
                        enable_ramh = 1;
                        enable_io1 = 1;
                        enable_io2 = 1;
                        mapped_game = 1;
                        mapped_exrom = 0;       /* ultimax */
                        if (allow_bank)
                        {
                            io1bank_ram = (7 << 3) | io1bank_ram;
                            io2bank_ram = (7 << 3) | io2bank_ram;
                        }
                        else
                        {
                            io1bank_ram = (0 << 3) | 0;
                            io2bank_ram = (0 << 3) | 0;
                        }
                    }
                    else        /* 16k game */
                    {
                        enable_io1 = 1;
                        enable_io2 = 0;
                        mapped_game = 1;
                        mapped_exrom = 1;       /* 16k game */
                    }
                }
                else
                {
#ifdef DEBUG
                    if (last_biosmode != 444)
                    {
                        LOG (("bios mode: 444"));
                        last_biosmode = 444;
                    }
#endif
                    /*
                     * disable_rr_rom=0 (opt.banking)
                     * enable_ram_io1=0  (use io2 only)
                     * 
                     * extended_mode=0   (1= toggle IO2 in cfg $22)
                     * allow_bank (active for ram in IO)
                     * 
                     * enable_ram_io=0
                     * 00 0Wro0Wro ----0Wr0 kern
                     * 01 0Wro0Wro ----0Wr0 kern
                     * 02 0Wro0Wro ----0Wr0 kern
                     * 03 0Wro0Wro ----0Wr0 kern
                     * 
                     * enable_ram_io=1
                     * 20 0Wro0Wro ----0Zra 0Zra
                     * 21 0Wro0Wro ----0Zra 0Zra
                     * 22 0Wro0Wro ----0Yra 0Zra (*)
                     * 23 0Wro0Wro ----0Zra 0Zra
                     * 
                     * $8000 - $9fff      bios rom
                     * $a000 - $bfff      bios rom (mirror)
                     * $e000 - $ffff           512K read/write ram window
                     * 
                     * X=normal bank
                     * 
                     * Y changes to ? if extended mode
                     * else ==Z
                     * W changes to 2 if extended mode and high bank=7              
                     * full banking if extended mode and high bank!=7
                     * else ==X
                     * 
                     * Z bank 00 if allow_bank=0
                     * full banking in extended mode
                     * high bank=7 if not extended mode
                     */
                    if (enable_ram_io)  /* ultimax */
                    {
//LOG(("aaa"));                         
#ifdef DEBUG
                        if (last_biosmode444 != 111)
                        {
                            LOG (("bios mode 444: ram enabled"));
                            last_biosmode444 = 111;
                        }
#endif
                        /* ultimax, ram at $e000, rom at $8000, rom at $a000 */
                        enable_ramh = 1;
                        enable_io1 = 0;
                        enable_io2 = 1;
                        mapped_game = 1;
                        mapped_exrom = 0;       /* ultimax */

//                                      mapped_game=1;mapped_exrom=1; /* 16k game */
//                                      mapped_game=0;mapped_exrom=1; /* 16k game */
//                                      mapped_game=0;mapped_exrom=0; /* 16k game */

//                                      if(enable_extended_mode)
                        {
                            /* full rom banking */
                            cartbankl =
                                ((bank_address_16_18 << 3) | bank_address_13_15)
                                & (0x3f);
                            cartbankh = cartbankl;
                            rambankl =
                                ((bank_address_16_18 << 3) | bank_address_13_15)
                                & (0x3f);
                            rambankh = rambankl;
                            io1bank =
                                ((bank_address_16_18 << 3) | bank_address_13_15)
                                & (0x3f);
                            io2bank = io1bank;


//                                              cartbankl=(7<<3)|0;
//                                              cartbankh=(7<<3)|0;
//                                              cartbankl=(7<<3)|bank_address_13_15;
//                                              cartbankh=(7<<3)|bank_address_13_15;
#if 0
                            if (bank_address_16_18 == 4)
                            {
                                cartbankl = (7 << 3) | bank_address_13_15;
                                cartbankh = (7 << 3) | bank_address_13_15;
                            }
#endif
#if 1
//                                              if(bank_address_16_18==7)
                            {
                                switch (bank_address_13_15)
                                {
                                    case 6:
                                    case 7:
                                        cartbankl = (7 << 3) | 0;
                                        cartbankh = (7 << 3) | 0;
//LOG(("this"));                                                                        
                                        break;
                                    default:
                                        cartbankl =
                                            (7 << 3) | bank_address_13_15;
                                        cartbankh =
                                            (7 << 3) | bank_address_13_15;
                                        break;
                                }
                            }
#endif

                            if (allow_bank)
                            {
                                /* full ram banking in extended mode */
                                io1bank_ram =
                                    ((bank_address_16_18 << 3) |
                                     bank_address_13_15) & (0x3f);
                                io2bank_ram = io1bank;
                            }
                            else
                            {
                                io1bank_ram = (0 << 3) | 0;
                                io2bank_ram = (0 << 3) | 0;
                            }

                        }


                        romA000_bank = cartbankl;
//                                              rambankl=cartbankl;
                        ramA000_bank = cartbankl;
                        rambankh = rambankl;
#if 0
/* FIXME: hack to make bios (filebrowser) work */

                        if (cartbankl == ((7 << 3) | 1))
                        {
                            cartbankl = (7 << 3) | 0;
                            LOG (("this"));
                        }
#endif
/*					
					
					else
					{
						if(allow_bank)
						{
							io1bank_ram=(7<<3)|io1bank_ram;
							io2bank_ram=(7<<3)|io2bank_ram;
						}
						else
						{
							io1bank_ram=(0<<3)|0;
							io2bank_ram=(0<<3)|0;
						}
						
					}
*/

                        if (enable_extended_mode)
                        {
                            if (((enable_game) | (enable_exrom << 1)) == 2)
                            {
                                /* ??? ROM in io? */
//                                                      enable_io2=0;
                                //enable_io1_ram=0;
                                //enable_io2_ram=0;
                            }
                        }
                    }
                    else        /* 16k game */
                    {
//LOG(("bbb"));                         
#ifdef DEBUG
                        if (last_biosmode444 != 222)
                        {
                            LOG (("bios mode 444: ram disabled"));
                            last_biosmode444 = 222;
                        }
#endif
                        enable_io1 = 0;
                        enable_io2 = 1;
                        mapped_game = 1;
                        mapped_exrom = 1;       /* 16k game */

//                                      bank_address_13_15=0;
                        cartbankl = ((7 << 3) | bank_address_13_15) & (0x3f);
                        cartbankh = cartbankl;
                        rambankl = ((7 << 3) | bank_address_13_15) & (0x3f);
                        rambankh = rambankl;
                        io1bank = ((7 << 3) | bank_address_13_15) & (0x3f);
                        io2bank = io1bank;

                        romA000_bank = cartbankl;


                        if (enable_extended_mode)
                        {
#if 0
                            /* full rom banking */
                            cartbankl =
                                ((bank_address_16_18 << 3) | bank_address_13_15)
                                & (0x3f);
                            cartbankh = cartbankl;
                            rambankl =
                                ((bank_address_16_18 << 3) | bank_address_13_15)
                                & (0x3f);
                            rambankh = rambankl;
                            io1bank =
                                ((bank_address_16_18 << 3) | bank_address_13_15)
                                & (0x3f);
                            io2bank = io1bank;

                            romA000_bank = cartbankl;
#endif

#if 0
//                                              if(bank_address_16_18==7)
                            {
                                switch (bank_address_13_15)
                                {
                                    case 6:
                                    case 7:
/* FIXME: is it really bank 0 ? else bios wont start */
                                        cartbankl = (7 << 3) | 0;
                                        cartbankh = (7 << 3) | 0;
                                        io1bank = (7 << 3) | 0;
                                        io2bank = (7 << 3) | 0;
//LOG(("that"));                                                                        
                                        break;
                                    default:
                                        cartbankl =
                                            (7 << 3) | bank_address_13_15;
                                        cartbankh =
                                            (7 << 3) | bank_address_13_15;
                                        break;
                                }
                            }
#endif
                        }

                    }

                }

            }

        }

    }
    else if (enable_16k_mapping)
    {
#ifdef DEBUG
        if (last_mainmode != 222)
        {
            LOG (("main mode: super mapper"));
            last_mainmode = 222;
        }
#endif
        /*************************************************************************************************
         * super mapper
         *************************************************************************************************/
        cartbankl = ((bank_address_16_18 << 3) | bank_address_13_15) & (0x3e);
        cartbankh = cartbankl + 1;
        rambankl = ((bank_address_16_18 << 3) | bank_address_13_15) & (0x3e);
        rambankh = rambankl + 1;
        io1bank = ((bank_address_16_18 << 3) | bank_address_13_15) & (0x3e);
        io2bank = io1bank;

        romA000_bank = cartbankh;

#if 0
        if (!allow_bank)
        {
            rambankl &= 0x38;   /* FIXME: clear last 3 bits ? */
            rambankh &= 0x38;   /* FIXME: clear last 3 bits ? */
            io1bank &= 0x38;    /* FIXME: clear last 3 bits ? */
            io2bank &= 0x38;    /* FIXME: clear last 3 bits ? */
        }
#endif
        /* FIXME */
        io1bank_ram = io1bank;
        io2bank_ram = io2bank;

        enable_io1 = 1;
        enable_io2 = 1;

#if 0
        LOG (("mapped_game:%d mapped_exrom:%d", mapped_game, mapped_exrom));
        switch (((enable_exrom ^ 1) << 1) | (enable_game))
        {
            case 0:
                mapped_game = 0;
                mapped_exrom = 1;       /*0 0 8k game   roml ($8000) */
                break;
            case 1:
                mapped_game = 1;
                mapped_exrom = 1;       /*1 0 16k game */
                break;
            case 2:
                mapped_game = 0;
                mapped_exrom = 1;       /*0 1 8k game   roml ($8000) */
                break;
            case 3:
                mapped_game = 1;
                mapped_exrom = 0;       /*1 1 ultimax   roml ($8000),romh ($e000) */
                break;
        }
        LOG (("mapped_game:%d mapped_exrom:%d", mapped_game, mapped_exrom));
#endif
        enable_raml = 0;
        enable_ramh = 0;
        if (disable_rr_rom)     /* When in 16K mode, bit 5 enables RAM at $a000 - $bfff */
        {
            /*   
             * disable_rr_rom=1 (opt.banking)
             * 
             * allow_bank        (inactive?)
             * extended_mode     (inactive?)
             * enable_ram_io1=0  (use io2 only)
             * 
             * enable_ram_io=0
             * 00 0Xrobasi0Xro0Xrokern
             * 00 0Xro0Yra0Xro0Xr0kern
             * 00  rambasi0Xro0Xrokern
             * 00 0Xro----0Xro0Xro0Yra
             * enable_ram_io=1
             * 00 0Xrabasi0Xra0Xrakern
             * 00 0Xra0Yra0Xra0Xrakern
             * 00  rambasi0Xra0Xrakern
             * 00 0Xra----0Xra0Xra0Yra
             * 
             * X=bankl Y=bankl+1
             */
            enable_ramh = 1;
            if (enable_ram_io)
            {
                enable_raml = 1;
            }
        }
        else
        {
            /*   
             * disable_rr_rom=0 (opt.banking)
             * 
             * allow_bank        (inactive?)
             * extended_mode     (inactive?)
             * enable_ram_io1=0  (use io2 only)
             * 
             * enable_ram_io=0
             * 00 0Xrobasi0Xro0Xrokern
             * 00 0Xro0Yro0Xro0Xr0kern
             * 00  rambasi0Xro0Xrokern
             * 00 0Xro----0Xro0Xro0Yro
             * enable_ram_io=1
             * 00 0Xrabasi0Xra0Xrakern
             * 00 0Xra0Yro0Xra0Xrakern
             * 00  rambasi0Xra0Xrakern
             * 00 0Xra----0Xra0Xra0Yro
             * 
             * X=bankl Y=bankl+1
             */
            if (enable_ram_io)
            {
                enable_raml = 1;
                enable_ramh = 0;
            }
        }
        LOG (("raml:%d ramh:%d", enable_raml, enable_ramh));
    }
    else if (disable_rr_rom)
    {
                /**************************************************************************************************
			normal mapper
		***************************************************************************************************/
#ifdef DEBUG
        if (last_mainmode != 333)
        {
            LOG (("main mode: normal mapper"));
            last_mainmode = 333;
        }
#endif
        cartbankl = (bank_address_16_18 << 3) | (bank_address_13_15);
        cartbankh = cartbankl;
        rambankl = ((bank_address_16_18 << 3) | bank_address_13_15);
        rambankh = rambankl;
        io1bank = ((bank_address_16_18 << 3) | bank_address_13_15);
        io2bank = io1bank;

        romA000_bank = cartbankl;

        /* FIXME */
        io1bank_ram = io1bank;
        io2bank_ram = io2bank;

        enable_raml = 0;
        enable_ramh = 0;

        if (enable_extended_mode)
        {
            /* FIXME */
            enable_io1 = 1;
            enable_io2 = 1;
        }

    }
    else
    {
                /**************************************************************************************************
			retro replay mapper
		***************************************************************************************************/
#ifdef DEBUG
        if (last_mainmode != 444)
        {
            LOG (("MMCREPLAY: *** main mode: [rr mapper]"));
            last_mainmode = 444;
        }
#endif
        cartbankl = (bank_address_16_18 << 3) | (bank_address_13_15);
        cartbankh = cartbankl;
        rambankl = ((bank_address_16_18 << 3) | bank_address_13_15);
        rambankh = rambankl;
        io1bank = ((bank_address_16_18 << 3) | bank_address_13_15);
        io2bank = io1bank;

        if (allow_bank)
        {
        }
        else
        {
#if 0
            if (!enable_extended_mode)
            {
                io1bank &= 0x38;        /* FIXME: clear last 3 bits ? */
                io2bank &= 0x38;        /* FIXME: clear last 3 bits ? */
            }
#endif
        }

        /* FIXME */
        io1bank_ram = io1bank;
        io2bank_ram = io2bank;

        enable_raml = 0;
        enable_ramh = 0;
        /*
         * Selecting Extended mode enables full RAM banking and enables
         * Nordic Power mode in RR mode.
         */
        /* FIXME: add 64kb-rombank offset also for ram ? */
        if (enable_extended_mode)
        {
            /* extended RR Mode */
            BYTE    value = (enable_game) |     /* bit 0 */
                ((enable_exrom ^ 1) << 1) |     /* bit 1 FIXME: is the bit inverted in the register ?! */
                (((bank_address_13_15 & 3) << 3) | ((bank_address_13_15 & 4) << 5)) |   /* bit 3,4,7 */
                (enable_ram_io << 5) |  /* bit 5 */
                (enable_freeze_exit << 6);      /* bit 6 */

            LOG (("rr mode 111 (extended)"));

            /* nordic power hack */
            if (value == 0x22)
            {
                if (enable_ram_io)
                {
                    mapped_game = 1;
                    mapped_exrom = 1;
                    enable_ramh = 1;
                    LOG (("nordic power hack"));
                }
            }
            else
            {
                if (enable_ram_io)
                {
                    enable_raml = 1;
                }
            }
            /* FIXME */
            enable_io1 = 1;
            enable_io2 = 1;
#if 1
            if (enable_ram_io1)
            {
                enable_io1 = 0;
                enable_io2 = 1;
            }
            else
            {
                enable_io1 = 0;
                enable_io2 = 1;
            }
#endif
        }
        else
        {
            LOG (("rr mode 222 (not extended)"));
            LOG (("enable_ram_io:%d", enable_ram_io));
            /* action replay hack */
            if (enable_ram_io)
            {
                BYTE    value = (enable_game) | /* bit 0 */
                    ((enable_exrom ^ 1) << 1) | /* bit 1 FIXME: is the bit inverted in the register ?! */
                    (((bank_address_13_15 & 3) << 3) | ((bank_address_13_15 & 4) << 5)) |       /* bit 3,4,7 */
                    (enable_ram_io << 5) |      /* bit 5 */
                    (enable_freeze_exit << 6);  /* bit 6 */
                if (value != 0x22)
                {
                    enable_raml = 1;
                }
                LOG (("po2:%02x", value));
            }
            else
            {
                /* ram not mapped */
                enable_raml = 0;
                enable_ramh = 0;
            }
            LOG (("enable_raml:%d", enable_raml));

            LOG (("enable_ram_io1:%d", enable_ram_io1));
            if (enable_ram_io1)
            {
                enable_io1 = 1;
//                              enable_io2=1;
                enable_io2 = 0;
            }
            else
            {
                enable_io1 = 0;
                enable_io2 = 1;
            }
            /* in rr compatibility mode, limit ram to 32k */
            rambankl &= 3;
            rambankh &= 3;
            io1bank_ram = io1bank & 3;
            io2bank_ram = io2bank & 3;

        }

        ramA000_bank = rambankl;
        romA000_bank = cartbankl;


    }

    cartridge_romlbank_set (cartbankl);
    cartridge_romhbank_set (cartbankh);

    mmcreplay_ramlbank_set (rambankl);
    mmcreplay_ramhbank_set (rambankh);

    mmcreplay_io1bank_set (io1bank, io1bank_ram);
    mmcreplay_io2bank_set (io2bank, io2bank_ram);

    active_mode_phi1 = (mapped_exrom << 1) | mapped_game;
    active_mode_phi2 = (mapped_exrom << 1) | mapped_game;
    mmcreplay_config_changed (active_mode_phi1, active_mode_phi2, wflag,
                              release_freeze);

    enable_freeze_exit = 0;     /* reset, it should only trigger once */
}

void mmcreplay_update_mapper (unsigned int wflag, int release_freeze)
{
    mmcreplay_update_mapper_nolog (wflag, release_freeze);
    mmcreplay_dump_cfg ();
}

/********************************************************************************************************************
IO Area

Disabled register free the I/O memory which is underneath them.
FIXME: Disabling RR Register disables ALL ROM/RAM banking too.
********************************************************************************************************************/

#ifdef DEBUG_IOBANKS
int     iobank_read = 0;
int     iobank_write = 0;
#endif

/********************************************************************************************************************
	IO1 - $deXX
********************************************************************************************************************/

BYTE REGPARM1 mmcreplay_io1_read (WORD addr)
{
//    LOG(("MMCREPLAY: IO1 RD %04x",addr));     
    if (rr_active)
    {
        switch (addr & 0xff)
        {
                /* OK $DE00 / $DE01: RR control register read */
                /*
                 * bit 0:     0 = Flash write protected, 1 = Flash write enabled
                 * bit 1:     0 = always map first ram bank in i/o space, 1 = enable io ram banking (R)
                 * bit 2:     1 = freeze button pressed
                 * bit 3:     bank address 13 (R)
                 * bit 4:     bank address 14 (R)
                 * bit 5:     ** ALWAYS 0 **
                 * bit 6:     0 = ram/rom @ DFxx, 1 = ram/rom @ $DExx (R)
                 * bit 7:     bank address 15 (R)
                 */
            case 0:
            case 1:
                io_source = IO_SOURCE_MMCREPLAY;
                return ((bank_address_13_15 & 3) << 3) |        /* bit 3,4 */
                    (enable_flash_write) |      /* bit 0 */
                    (allow_bank << 1) | /* bit 1 */
                    (freeze_pressed << 2) |     /* bit 2 */
                    (enable_ram_io1 << 6) |     /* bit 6 */
                    ((bank_address_13_15 & 4) << 5);    /* bit 7 */
                break;
            default:
#ifdef HAVE_TFE
                /*
                 * $DE02 - $DE0F: Clockport memory area (when enabled)
                 */
                if (mmcr_clockport_enabled)
                {
                    if (tfe_enabled && tfe_as_rr_net && (addr & 0xff) < 0x10)
                    {
#ifdef LOG_CLOCKPORT
                        LOG (("MMCREPLAY: Clockport RD %04x", addr));
#endif
                        io_source = 0;
                        return 0;
                    }
                }
#endif
                break;
        }
        if (enable_io1)
        {
            io_source = IO_SOURCE_MMCREPLAY;
            if (enable_ram_io)
            {
//LOG(("read RAM IO1 %04x %02x (%02x:%04x)", addr,export_ram0[(addr & 0x1fff) + (io1_ram_bank << 13)],io1_ram_bank,(addr & 0x1fff)));
                return export_ram0[(io1_ram_bank << 13) + 0x1e00 +
                                   (addr & 0xff)];
            }
//LOG(("read ROM IO1 %04x %02x (%02x:%04x)", addr,roml_banks[(addr & 0x1fff) + (io1_bank << 13)],io1_bank,(addr & 0x1fff)));
            return roml_banks[(addr & 0x1fff) + (io1_bank << 13)];
        }
        else
        {
            LOG (("read DISABLED IO1 %04x %02x (%02x:%04x)", addr,
                  roml_banks[(addr & 0x1fff) + (io1_bank << 13)], io1_bank,
                  (addr & 0x1fff)));
        }
    }
    else
    {

#if 1
        if (enable_io1)
        {
            io_source = IO_SOURCE_MMCREPLAY;
            if (enable_ram_io)
            {
//LOG(("read RAM IO1 %04x %02x (%02x:%04x)", addr,export_ram0[(addr & 0x1fff) + (io1_ram_bank << 13)],io1_ram_bank,(addr & 0x1fff)));
                return export_ram0[(io1_ram_bank << 13) + 0x1e00 +
                                   (addr & 0xff)];
            }
//LOG(("read ROM IO1 %04x %02x (%02x:%04x)", addr,roml_banks[(addr & 0x1fff) + (io1_bank << 13)],io1_bank,(addr & 0x1fff)));
            return roml_banks[(addr & 0x1fff) + (io1_bank << 13)];
        }
        else
        {
            LOG (("read DISABLED IO1 %04x %02x (%02x:%04x)", addr,
                  roml_banks[(addr & 0x1fff) + (io1_bank << 13)], io1_bank,
                  (addr & 0x1fff)));
        }
#else
        LOG (("read RR DISABLED IO1 %04x %02x (%02x:%04x)", addr,
              roml_banks[(addr & 0x1fff) + (io1_bank << 13)], io1_bank,
              (addr & 0x1fff)));
#endif

    }
    return vicii_read_phi1 ();
}

/*
$DE01:	extended RR control register write
		--------------------------------------
		bit 0:	0 = disable clockport, 1 = enable clockport (W)
(*2)		bit 1:	0 = disable io ram banking, 1 = enable io ram banking (W)
(*2)		bit 2:	0 = enable freeze, 1 = disable freeze (W)
		bit 3:	bank address 13 (mirror of $DE00) (W)
		bit 4:	bank address 14 (mirror of $DE00) (W)
(*2)		bit 5:	0 = enable MMC registers, 1 = disable MMC registers (W)(*1)
(*2)		bit 6:	0 = ram/rom @ DFxx, 1 = ram/rom @ $DExx (W)
		bit 7:	bank address 15 (mirror of $DE00) (W)

(*1) Can only be written when bit 6 of $DF12 is 1. Register becomes effective
		when bit 0 of $DF11 is 1.
(*2) these bits are write-once if bit 0 of $DF11 is 1
*/

void REGPARM2 mmcreplay_io1_store (WORD addr, BYTE value)
{
//LOG (("MMCREPLAY: IO1 ST %04x %02x", addr, value));
    if (rr_active)
    {
        switch (addr & 0xff)
        {
            case 0:            /* OK $DE00:  RR control register */
                /*    bit 0:  GAME line (W)
                 * bit 1:     EXROM line (W)
                 * bit 2:     1 = disable RR (W) (*)
                 * bit 3:     bank address 13 (W)
                 * bit 4:     bank address 14 (W)
                 * bit 5:     0 = rom enable, 1 = ram enable (W)
                 * bit 6:     1 = exit freeze mode (W)
                 * bit 7:     bank address 15 (W)
                 * (*) bit can be reset by setting bit 6 of $df12 */

//LOG(("MMCREPLAY: IO1 ST %04x %02x",addr,value));

                enable_game = value & 1;        /* bit 0 */
                enable_exrom = ((value >> 1) & 1) ^ 1;  /* bit 1 FIXME: is the bit inverted in the register ?! */
                bank_address_13_15 = (((value >> 3) & 3) | ((value >> 5) & 4)); /* bit 3,4,7 */
                if (value & 4)
                {
                    rr_active = 0;      /* bit 2 */
                }
                enable_ram_io = (value >> 5) & 1;       /* bit 5 */
                enable_freeze_exit = (value >> 6) & 1;  /* bit 6 */

                LOG (("MMCREPLAY: IO1 ST %04x enable_ram_io %02x", addr,
                      enable_ram_io));

                mmcreplay_update_mapper (CMODE_WRITE, enable_freeze_exit);
                return;
                break;
            case 1:            /* OK $DE01:   extended RR control register write */
                /* Every bit in $de01 can always be altered in Super Mapper mode+MMC Bios Mode. */
                bank_address_13_15 = (((value >> 3) & 3) | ((value >> 5) & 4)); /* bit 3,4,7 */
                mmcr_clockport_enabled = value & 1;     /* bit 0 */

                /* bits 1,2,5,6 are "write once" if not in mmc bios mode */
                if ((write_once == 0) || (disable_mmc_bios == 0))
                {
                    allow_bank = (value >> 1) & 1;      /* bit 1 */
                    no_freeze = (value >> 2) & 1;       /* bit 2 */
                    enable_ram_io1 = (value >> 6) & 1;  /* bit 6 */
                    if ((enable_extended_mode == 1)
                        || (enable_16k_mapping == 1))
                    {
                        /*bit 5:        0 = enable MMC registers, 1 = disable MMC registers (W) */
                        enable_mmc_regs_pending = ((value >> 5) & 1) ^ 1;       /* bit 5 */
                    }
                    if (disable_mmc_bios != 0)
                    {
                        write_once = 1;
                    }
                }
                LOG (("MMCREPLAY: IO1 ST %04x %02x", addr, value));
                mmcreplay_update_mapper (CMODE_WRITE, 0);
                return;
                break;
                /* not for us */
            default:
#ifdef HAVE_TFE
                /*
                 * $DE02 - $DE0F: Clockport memory area (when enabled)
                 */
                if (mmcr_clockport_enabled)
                {
                    if (tfe_enabled && tfe_as_rr_net && (addr & 0xff) < 0x10)
                    {
#ifdef LOG_CLOCKPORT
                        LOG (("MMCREPLAY: Clockport ST %04x %02x", addr,
                              value));
#endif
                        return;
                    }
                }
#endif
                break;
        }
        if (enable_io1)
        {
            if (enable_ram_io)
            {
                LOG (("store RAM IO1 %04x %02x (%02x:%04x)", addr, value,
                      io1_ram_bank, (addr & 0x1fff)));
                export_ram0[(io1_ram_bank << 13) + 0x1e00 + (addr & 0xff)] =
                    value;
            }
            else
            {
                LOG (("store DISABLED RAM IO1 %04x %02x (%02x:%04x)", addr,
                      value, io1_ram_bank, (addr & 0x1fff)));
            }
        }
        else
        {
            LOG (("store DISABLED IO1 %04x %02x (%02x:%04x)", addr, value,
                  io1_bank, (addr & 0x1fff)));
        }
    }                           /* rr active */
    else
    {
        LOG (("store DISABLED RR IO1 %04x %02x (%02x:%04x)", addr, value,
              io1_bank, (addr & 0x1fff)));
    }
}

/*
	IO2 - $dfXX
*/


/*
$DF10:	MMC SPI transfer register
		-----------------------------
		byte written is sent to the card & response from the card is read here

$DF11:	MMC control register
		------------------------
		bit 0:	0 = MMC BIOS enabled, 1 = MMC BIOS disabled (R/W) (*)
		bit 1:	0 = card selected, 1 = card not selected (R/W) (**)
		bit 2:	0 = 250khz transfer, 1 = 8mhz transfer (R/W)
		bit 3:	** ALWAYS 0 **
		bit 4:	** ALWAYS 0 **
		bit 5:	0 = allow RR rom when MMC BIOS disabled , 1 = disable RR ROM (R/W)	(***)
		bit 6:	0 = SPI write trigger mode, 1 = SPI read trigger mode (R/W)
		bit 7:	** ALWAYS 0 **

(*) 	Enabling MMC Bios sets ROM banking to the last 64K bank
(**) 	This bit also controls the green activity LED.
(***)	When in mmcreplay bios mode, bit 5 controls RAM banking (0 = $e000 - $ffff, 1 = $8000 - $9fff)
			When in 16K mode, bit 5 enables RAM at $a000 - $bfff

$DF12:	MMC status register
		-----------------------
		bit 0:	0 = SPI ready, 1 = SPI busy (R)						
						1 = forbid ROM write accesses (W) (*)
	
		bit 1:	feedback of $DE00 bit 0 (GAME) (R)					
		bit 2:	feedback of $DE00 bit 1 (EXROM) (R)
		bit 3:	0 = card inserted, 1 = no card inserted (R)
		bit 4:  0 = card write enabled, 1 = card write disabled (R)
		
		bit 5:	EEPROM DATA line / DDR Register (R/W) (**)
		bit 6:	0 = RR compatibility mode, 1 = Extended mode (W) (***)
		bit 7:	EEPROM CLK line (W)

(*) Setting this bit will disable writes to ROM until next reset.
(**) Setting DATA to "1" enables reading data bit from EEPROM at this position.
(***) Selecting RR compatibility mode limits RAM to 32K and
		disables writes to extended banking register.
		Selecting Extended mode enables full RAM banking and enables
		Nordic Power mode in RR mode.

$DF13:	Extended banking register (*)
		-----------------------------
		bit 0:	bank address 16	(R/W)
		bit 1:	bank address 17 (R/W)
		bit 2:	bank address 18 (R/W)
		bit 3:	** ALWAYS 0 **
		bit 4:	** ALWAYS 0 **
		bit 5:	16K rom mapping (R/W)
		bit 6:	1 = enable RR register
		bit 7:	** ALWAYS 0 **

(*) Can only be read/written to when bit 6 of $DF12 is 1.

*/

BYTE REGPARM1 mmcreplay_io2_read (WORD addr)
{
    BYTE    value;

//LOG(("MMCREPLAY: IO2 RD %04x",addr));

    if (rr_active)
    {

        switch (addr & 0xff)
        {
            case 0x10:         /* OK $df10 MMC SPI transfer register */
                if (enable_mmc_regs)
                {
//LOG(("MMCREPLAY: IO2 RD %04x",addr));
                    io_source = IO_SOURCE_MMCREPLAY;
                    return spi_mmc_data_read ();
                }
                else
                {

                    if (!enable_io2)
                    {
                        LOG (("MMCREPLAY: IO2 RD %04x (disabled!)", addr));
                    }

                }
                break;
            case 0x11:         /* OK $df11 MMC control register */
                if (enable_mmc_regs)
                {
//    LOG(("MMCREPLAY: IO2 RD %04x",addr)); 
                    io_source = IO_SOURCE_MMCREPLAY;
                    value = disable_mmc_bios;   /* bit 0 */
                    value |= (spi_mmc_card_selected_read () << 1);      /* bit 1 */
                    value |= (spi_mmc_enable_8mhz_read () << 2);        /* bit 2 */
                    /* bit 3,4 always 0 */
                    value |= (disable_rr_rom << 5);     /* bit 5 */
                    value |= (spi_mmc_trigger_mode_read () << 6);       /* bit 6 */
                    /* bit 7 always 0 */
                    return value;
                }
                else
                {

                    if (!enable_io2)
                    {
                        LOG (("MMCREPLAY: IO2 RD %04x (disabled!)", addr));
                    }

                }
                break;
            case 0x12:         /* OK $df12 MMC status register */
                if (enable_mmc_regs)
                {
                    BYTE    value = 0;
                    io_source = IO_SOURCE_MMCREPLAY;
                    /* EEPROM is only accessible in MMC Replay Bios mode */
                    if (disable_mmc_bios == 0)
                    {
                        value = eeprom_data_read () << 5;       /* bit 5 */
                    }
                    value |= (spi_mmc_busy ()); /* bit 0 */
                    value |= (enable_game << 1);        /* bit 1 */
                    value |= (enable_exrom ^ 1) << 2;   /* bit 2 FIXME: inverted in reg ? */
                    value |= (spi_mmc_card_inserted () ^ 1) << 3;       /* bit 3 */
                    value |= (spi_mmc_card_write_enabled () ^ 1) << 4;  /* bit 4 */
                    /* bit 6,7 not readable */
#ifdef DEBUG
                    if ((value != 0x18) && (value != 0x38))
                        if (disable_mmc_bios == 0)
                        {
                            LOG (("MMCREPLAY: IO2 RD %04x (eeprom)", addr));
                            LOG (("MMCREPLAY: IO2 RD %04x %02x", addr, value));
                        }
#endif
                    return value;
                }
                else
                {

                    if (!enable_io2)
                    {
                        LOG (("MMCREPLAY: IO2 RD %04x (disabled!)", addr));
                    }

                }
#if 0
//                  else
                {

                    BYTE    value = 0;
//    LOG(("MMCREPLAY: IO2 RD %04x",addr)); 
                    /* EEPROM is only accessible in MMC Replay Bios mode */
                    if (disable_mmc_bios == 0)
                    {
                        io_source = IO_SOURCE_MMCREPLAY;
//    LOG(("MMCREPLAY: IO2 RD %04x (eeprom,reg disabled?)",addr));
                        value = eeprom_data_read () << 5;       /* bit 5 */
                        value |= (spi_mmc_busy ());     /* bit 0 */
                        value |= (enable_game << 1);    /* bit 1 */
                        value |= (enable_exrom ^ 1) << 2;       /* bit 2 FIXME: inverted in reg ? */
                        value |= (spi_mmc_card_inserted () ^ 1) << 3;   /* bit 3 */
                        value |= (spi_mmc_card_write_enabled () ^ 1) << 4;      /* bit 4 */
                        /* bit 6,7 not readable */
                        return value;
                    }
                }
#endif
                break;
            case 0x13:         /* OK $df13 Extended banking register */

                /* $df13 can only be READ AND WRITTEN if bit6 of $df12 = 1 */
                if (enable_extended_mode)
                {
                    LOG (("MMCREPLAY: IO2 RD %04x", addr));
                    io_source = IO_SOURCE_MMCREPLAY;
                    value = bank_address_16_18; /* bit 0-2 */
                    /* bit 3,4 always 0 */
                    value |= enable_16k_mapping << 5;   /* bit 5 */
                    /*value|=enable_rr_regs<<6; *//* bit 6 FIXME: readable ? */
                    /* bit 7 always 0 */
                    return value;
                }
                else
                {

                    if (!enable_io2)
                    {
                        LOG (("MMCREPLAY: IO2 RD %04x (disabled!)", addr));
                    }

                }
                break;
        }

        if (enable_io2)
        {
            io_source = IO_SOURCE_MMCREPLAY;
            if (enable_ram_io)
            {
//                  LOG(("read RAM IO2 %04x %02x (%02x:%04x)", addr,export_ram0[(addr & 0x1fff) + (io2_ram_bank << 13)],io2_ram_bank,(addr & 0x1fff)));
                return export_ram0[(io2_ram_bank << 13) + 0x1f00 +
                                   (addr & 0xff)];
            }
//              LOG(("read ROM IO2 %04x %02x (%02x:%04x)", addr,roml_banks[(addr & 0x1fff) + (io2_bank << 13)],io2_bank,(addr & 0x1fff)));
            return roml_banks[(addr & 0x1fff) + (io2_bank << 13)];
        }
        else
        {
            LOG (("read DISABLED IO2 %04x %02x (%02x:%04x)", addr,
                  roml_banks[(addr & 0x1fff) + (io2_bank << 13)], io2_bank,
                  (addr & 0x1fff)));
        }
//        return 0;
        return vicii_read_phi1 ();
    }
    else
    {

        LOG (("read RR DISABLED IO2 %04x %02x (%02x:%04x)", addr,
              roml_banks[(addr & 0x1fff) + (io2_bank << 13)], io2_bank,
              (addr & 0x1fff)));

#if 0
        if (enable_io2)
        {
            io_source = IO_SOURCE_MMCREPLAY;
            if (enable_ram_io)
            {
                LOG (("read RAM IO2 %04x %02x (%02x:%04x)", addr,
                      export_ram0[(addr & 0x1fff) + (io2_ram_bank << 13)],
                      io2_ram_bank, (addr & 0x1fff)));
                return export_ram0[(io2_ram_bank << 13) + 0x1f00 +
                                   (addr & 0xff)];
            }
//              LOG(("read ROM IO2 %04x %02x (%02x:%04x)", addr,roml_banks[(addr & 0x1fff) + (io2_bank << 13)],io2_bank,(addr & 0x1fff)));
            return roml_banks[(addr & 0x1fff) + (io2_bank << 13)];
        }
#endif
        return vicii_read_phi1 ();
    }
}

#ifdef DEBUG
int     store_io2_warn = 0;
#endif

void REGPARM2 mmcreplay_io2_store (WORD addr, BYTE value)
{
//    LOG(("MMCREPLAY: IO2 ST %04x %02x",addr,value));  
    /*if (rr_active) */
    /* FIXME: should IO2 regs work even when "cartridge not active" ? 
     * how else would "re-enabling RR" make sense?
     */
    {

        switch (addr & 0xff)
        {
            case 0x10:         /* OK $df10 MMC SPI transfer register */
                if (enable_mmc_regs)
                {
//    LOG(("MMCREPLAY: IO2 ST %04x %02x",addr,value));  
                    spi_mmc_data_write (value);
                    return;
                }
                else
                {
                    LOG (("MMCREPLAY: IO2 ST %04x (disabled!)", addr));
                }
                break;
            case 0x11:         /* OK $df11 MMC control register */
                if (enable_mmc_regs)
                {
                    LOG (("MMCREPLAY: IO2 ST %04x %02x", addr, value));
                    LOG (("MMCREPLAY: IO2 ST %04x  spi_mmc_card_selected %1d",
                          addr, ((value >> 1) ^ 1) & 1));

                    disable_mmc_bios = (value) & 1;     /* bit 0 */
                    disable_rr_rom = (value >> 5) & 1;  /* bit 5 */
                    spi_mmc_card_selected_write (((value >> 1) ^ 1) & 1);       /* bit 1 */
                    spi_mmc_enable_8mhz_write (((value >> 2)) & 1);     /* bit 2 */
                    /* bit 3,4 always 0 */
                    spi_mmc_trigger_mode_write (((value >> 6)) & 1);    /* bit 6 */
                    /* bit 7 always 0 */
                    if (disable_mmc_bios)
                    {
                        //if(enable_mmc_regs_pending)
                        {
                            enable_mmc_regs = enable_mmc_regs_pending;
                        }
                    }
                    else
                    {
                        /* clearing bit 0 resets write once for de01 */
                        write_once = 0;
                    }
//    mmcreplay_update_mapper();
//            mmcreplay_config_changed((enable_exrom<<1)|enable_game, (enable_exrom<<1)|enable_game, CMODE_WRITE);
                    mmcreplay_update_mapper (CMODE_WRITE, 0);
                    return;
                }
                else
                {
                    LOG (("MMCREPLAY: IO2 ST %04x (disabled!)", addr));
                }
                LOG (("MMCREPLAY: IO2 ST %04x disable_mmc_bios %02x", addr,
                      disable_mmc_bios));
                LOG (("MMCREPLAY: IO2 ST %04x disable_rr_rom %02x", addr,
                      disable_rr_rom));
                break;
            case 0x12:         /* OK $df12 MMC status register */
                if (enable_mmc_regs)
                {
                    LOG (("MMCREPLAY: IO2 ST %04x %02x", addr, value));

                    /* FIXME bit 1: forbid write access to ROM */

                    disable_rom_write = (value ^ 1) & 1;        /* bit 0 */
                    enable_extended_mode = (value >> 6) & 1;    /* bit 6 */
                    if (enable_extended_mode)
                    {
                        rr_active = 1;
                    }
                    LOG (("MMCREPLAY: IO2 ST %04x enable_extended_mode %d",
                          addr, enable_extended_mode));

                    /* bit 2 ? FIXME */
                    /* bit 3 ? FIXME */

                    /* EEPROM is only accessible in MMC Replay Bios mode */
                    if (disable_mmc_bios == 0)
                    {
                        LOG (("MMCREPLAY: IO2 ST %04x (eeprom)", addr));
                        /*
                         * bit 1: ddr FIXME
                         * bit 4: status
                         * bit 5: data/ddr
                         * bit 7: clk
                         */
                        eeprom_port_write ((value >> 7) & 1, (value >> 5) & 1,
                                           (value >> 1) & 1, (value >> 4) & 1);
                    }

                    mmcreplay_update_mapper (CMODE_WRITE, 0);
                    return;
                }
                else
                {
                    /* EEPROM is only accessible in MMC Replay Bios mode */
                    if (disable_mmc_bios == 0)
                    {
//    LOG(("MMCREPLAY: IO2 ST %04x (eeprom, reg disabled?)",addr));
                        /* FIXME bit 1: forbid write access to ROM */

                        disable_rom_write = (value ^ 1) & 1;    /* bit 0 */
                        enable_extended_mode = (value >> 6) & 1;        /* bit 6 */
                        if (enable_extended_mode)
                            rr_active = 1;

                        /* bit 2 ? FIXME */
                        /* bit 3 ? FIXME */
                        /*
                         * bit 1: ddr FIXME
                         * bit 4: status
                         * bit 5: data/ddr
                         * bit 7: clk
                         */
                        eeprom_port_write ((value >> 7) & 1, (value >> 5) & 1,
                                           (value >> 1) & 1, (value >> 4) & 1);
                    }
                    mmcreplay_update_mapper (CMODE_WRITE, 0);
                    return;
                }
                LOG (("MMCREPLAY: IO2 ST %04x disable_rom_write %02x", addr,
                      disable_rom_write));
                LOG (("MMCREPLAY: IO2 ST %04x enable_extended_mode %02x", addr,
                      enable_extended_mode));


                break;
            case 0x13:         /* OK $df13 Extended banking register */

                /* $df13 can only be READ AND WRITTEN if bit6 of $df12 = 1 */
                if (enable_extended_mode)
                {
                    LOG (("MMCREPLAY: IO2 ST %04x %02x", addr, value));
                    bank_address_16_18 = (value) & 7;   /* bit 0-2 */
                    LOG (("MMCREPLAY: IO2 ST %04x bank_address_16_18 %02x",
                          addr, bank_address_16_18));
                    enable_16k_mapping = (value >> 5) & 1;      /* bit 5 */
                    enable_rr_regs = (value >> 6) & 1;  /* bit 6 */
                    if ((value >> 6) & 1)       /* bit 6 */
                    {
                        /* re-enable RR cartridge */
                        rr_active = 1;
                    }
//    mmcreplay_update_mapper();
//            mmcreplay_config_changed((enable_exrom<<1)|enable_game, (enable_exrom<<1)|enable_game, CMODE_WRITE);
                    mmcreplay_update_mapper (CMODE_WRITE, 0);
                    return;
                }
                else
                {
#ifdef DEBUG
                    if (store_io2_warn < 10)
                    {
                        LOG (("MMCREPLAY: IO2 ST %04x %02x (disabled!)", addr,
                              value));
                        store_io2_warn++;
                    }
#endif
                }
                LOG (("MMCREPLAY: IO2 ST %04x enable_16k_mapping %02x", addr,
                      enable_16k_mapping));
                LOG (("MMCREPLAY: IO2 ST %04x enable_rr_regs %02x", addr,
                      enable_rr_regs));

                break;
        }

        if (enable_io2)
        {
            if (enable_ram_io)
            {
                LOG (("store RAM IO2 %04x %02x (%02x:%04x)", addr, value,
                      io2_ram_bank, (addr & 0x1fff)));
                export_ram0[(io2_ram_bank << 13) + 0x1f00 + (addr & 0xff)] =
                    value;
            }
            else
            {
#ifdef DEBUG
                if (store_io2_warn < 10)
                {
                    LOG (("store DISABLED RAM in IO2 %04x %02x (%02x:%04x)",
                          addr, value, io2_ram_bank, (addr & 0x1fff)));
                    store_io2_warn++;
                }
#endif
            }
        }
        else
        {
#ifdef DEBUG
            if (store_io2_warn < 10)
            {
                LOG (("store DISABLED IO2 %04x %02x (%02x:%04x)", addr, value,
                      io2_bank, (addr & 0x1fff)));
                store_io2_warn++;
            }
#endif
        }

    }
/*    
    else
    {
	LOG(("store RR DISABLED IO2 %04x %02x (%02x:%04x)", addr,value,io2_bank,(addr & 0x1fff)));
    }
*/
}

/********************************************************************************************************************
MEM Area
********************************************************************************************************************/
#ifdef DEBUG_LOGBANKS
int     logbank_read = 0;
int     logbank_write = 0;
#endif
/*
	ultimax - $1000-$7fff
*/

BYTE REGPARM1 mmcreplay_1000_7fff_read (WORD addr)
{
#ifdef DEBUG_LOGBANKS
    if (logbank_read != (addr & 0xe000))
    {
        LOG (("MMCREPLAY: RAM  RD %04x", addr));
        logbank_read = (addr & 0xe000);
    }
#endif

//    if(!disable_mmc_bios)
    {
        return ram_read (addr);
    }
    return vicii_read_phi1 ();
}
void REGPARM2 mmcreplay_1000_7fff_store (WORD addr, BYTE value)
{
#ifdef DEBUG_LOGBANKS
    if (logbank_write != (addr & 0xe000))
    {
        LOG (("MMCREPLAY: RAM  ST %04x %02x", addr, value));
        logbank_write = (addr & 0xe000);
    }
#endif
//    if(!disable_mmc_bios)
    {
        ram_store (addr, value);
    }
}

/*
	ROML - $8000-$9fff
*/

BYTE REGPARM1 mmcreplay_roml_read (WORD addr)
{
#ifdef DEBUG_LOGBANKS
    if (logbank_read != (addr & 0xe000))
    {
//              LOG(("MMCREPLAY: RD %04x",addr));
        if (enable_raml)
        {
            LOG (("MMCREPLAY: RAML RD %04x %02x (%02x:%04x)", addr,
                  export_ram0[(addr & 0x1fff) + (raml_bank << 13)], raml_bank,
                  (addr & 0x1fff)));
            logbank_read = (addr & 0xe000);
        }
        else
        {
//                      LOG(("MMCREPLAY: ROML RD %04x %02x (%02x:%04x)", addr,roml_banks[(addr & 0x1fff) + (roml_bank << 13)],roml_bank,(addr & 0x1fff)));
//                      logbank_read=(addr&0xe000);
        }


    }
#endif
/*	
    if (export_ram) {
*/
//    if (enable_ram_io) {
    if (enable_raml)
    {
//          LOG(("read RAML %04x %02x (%02x:%04x)", addr,export_ram0[(addr & 0x1fff) + (raml_bank << 13)],raml_bank,(addr & 0x1fff)));
        return export_ram0[(addr & 0x1fff) + (raml_bank << 13)];
    }
//    LOG(("read ROML %04x %02x (%02x:%04x)", addr,roml_banks[(addr & 0x1fff) + (roml_bank << 13)],roml_bank,(addr & 0x1fff)));
//    return roml_banks[(addr & 0x1fff) + (roml_bank << 13)];
    return flashrom_read ((addr & 0x1fff) + (roml_bank << 13));
}

void REGPARM2 mmcreplay_roml_store (WORD addr, BYTE value)
{
#ifdef DEBUG_LOGBANKS
    if (logbank_write != (addr & 0xe000))
    {
        //LOG(("MMCREPLAY: ST %04x",addr));
        if (enable_raml)
        {
            LOG (("MMCREPLAY: RAML ST %04x %02x (%02x:%04x)", addr, value,
                  raml_bank, (addr & 0x1fff)));
        }
        else
        {
            LOG (("MMCREPLAY: DISABLED! RAML ST  %04x %02x (%02x:%04x)", addr,
                  value, raml_bank, (addr & 0x1fff)));
        }
        logbank_write = (addr & 0xe000);
    }
#endif
    if (enable_raml)
    {
//          LOG(("store RAML %04x %02x (%02x:%04x) (WARNING!)", addr,value,raml_bank,(addr & 0x1fff)));
        export_ram0[(addr & 0x1fff) + (raml_bank << 13)] = value;
        return;
    }
    LOG (("raml disabled! store RAML %04x %02x (%02x:%04x)", addr, value,
          raml_bank, (addr & 0x1fff)));
    flashrom_write ((addr & 0x1fff) + (roml_bank << 13), value);
}

BYTE REGPARM1 mmcreplay_a000_read (WORD addr)
{
#ifdef DEBUG_LOGBANKS
    if (logbank_read != (addr & 0xe000))
    {
        LOG (("MMCREPLAY: RD %04x", addr));
        logbank_read = (addr & 0xe000);
    }
#endif
    if (!disable_mmc_bios)
    {
        if (enable_ramA000)
        {
            //          LOG(("read RAML %04x %02x (%02x:%04x)", addr,export_ram0[(addr & 0x1fff) + (raml_bank << 13)],raml_bank,(addr & 0x1fff)));
            return export_ram0[(addr & 0x1fff) + (ramA000_bank << 13)];
        }
        //    LOG(("read ROML %04x %02x (%02x:%04x)", addr,roml_banks[(addr & 0x1fff) + (roml_bank << 13)],roml_bank,(addr & 0x1fff)));
        return roml_banks[(addr & 0x1fff) + (romA000_bank << 13)];
    }
    return vicii_read_phi1 ();
}
void REGPARM2 mmcreplay_a000_store (WORD addr, BYTE value)
{
#ifdef DEBUG_LOGBANKS
    if (logbank_write != (addr & 0xe000))
    {
        LOG (("MMCREPLAY: ST %04x", addr));
        logbank_write = (addr & 0xe000);
    }
#endif
    if (!disable_mmc_bios)
    {
        if (enable_ramA000)
        {
            LOG (("store RAM A000 %04x %02x (%02x:%04x)", addr, value,
                  ramA000_bank, (addr & 0x1fff)));
            export_ram0[(addr & 0x1fff) + (ramA000_bank << 13)] = value;
        }
        else
        {
            LOG (("store DISABLED RAM A000 %04x %02x (%02x:%04x)", addr, value,
                  romA000_bank, (addr & 0x1fff)));
//                  export_ram0[(addr & 0x1fff) + (ramA000_bank << 13)] = value;
        }
    }
}

BYTE REGPARM1 mmcreplay_c000_read (WORD addr)
{
#ifdef DEBUG_LOGBANKS
    if (logbank_read != (addr & 0xe000))
    {
        LOG (("MMCREPLAY: RAM  RD %04x", addr));
        logbank_read = (addr & 0xe000);
    }
#endif
//    if(!disable_mmc_bios)
    {
        return ram_read (addr);
    }
    return vicii_read_phi1 ();
}
void REGPARM2 mmcreplay_c000_store (WORD addr, BYTE value)
{
#ifdef DEBUG_LOGBANKS
    if (logbank_write != (addr & 0xe000))
    {
        LOG (("MMCREPLAY: RAM  ST %04x %02x", addr, value));
        logbank_write = (addr & 0xe000);
    }
#endif
//    if(!disable_mmc_bios)
    {
        ram_store (addr, value);
    }
}

/*
	ROMH - $a000 ($e000 in ultimax mode)
*/

BYTE REGPARM1 mmcreplay_romh_read (WORD addr)
{
#ifdef DEBUG_LOGBANKS
    if (logbank_read != (addr & 0xe000))
    {
        if (enable_ramh)
        {
            if (addr < 0xfff0)
            {
                LOG (("MMCREPLAY: RAMH RD %04x %02x (%02x:%04x)", addr,
                      export_ram0[(addr & 0x1fff) + (ramh_bank << 13)],
                      ramh_bank, (addr & 0x1fff)));
                logbank_read = (addr & 0xe000);
            }
        }
        else
        {
            LOG (("MMCREPLAY: ROMH RD %04x %02x (%02x:%04x)", addr,
                  romh_banks[(addr & 0x1fff) + (romh_bank << 13)], romh_bank,
                  (addr & 0x1fff)));
            logbank_read = (addr & 0xe000);
        }
//              LOG(("MMCREPLAY: RD %04x",addr));

    }
#endif
    if (enable_ramh)
    {
//          LOG(("read RAMH %04x %02x (%02x:%04x)", addr,export_ram0[(addr & 0x1fff) + (ramh_bank << 13)],ramh_bank,(addr & 0x1fff)));
        return export_ram0[(addr & 0x1fff) + (ramh_bank << 13)];
    }


//    LOG(("read ROMH %04x %02x (%02x:%04x)", addr,romh_banks[(addr & 0x1fff) + (romh_bank << 13)],romh_bank,(addr & 0x1fff)));
    return romh_banks[(addr & 0x1fff) + (romh_bank << 13)];
}

void REGPARM2 mmcreplay_romh_store (WORD addr, BYTE value)
{
#ifdef DEBUG_LOGBANKS
    if (logbank_write != (addr & 0xe000))
    {
        if (enable_ramh)
        {
            LOG (("MMCREPLAY: RAMH ST %04x %02x (%02x:%04x)", addr, value,
                  ramh_bank, (addr & 0x1fff)));
        }
        else
        {
            LOG (("MMCREPLAY: DISABLED! RAMH ST %04x %02x (%02x:%04x)", addr,
                  value, ramh_bank, (addr & 0x1fff)));
        }
        logbank_write = (addr & 0xe000);
    }
#endif
    if (enable_ramh)
    {
        export_ram0[(addr & 0x1fff) + (ramh_bank << 13)] = value;
    }
    else
    {
        /* write through to ram ? */
    }
}


/*********************************************************************************************************************

*********************************************************************************************************************/
int mmcreplay_freeze_allowed (void)
{
    if (no_freeze)
    {
        return 0;
    }
    return 1;
}

/*
Freeze DOES NOT CLEAR banking bits 16-18.
Banking bits A13-A15 are cleared when freeze is pressed.
Ram is always enabled in freeze mode. The Ram bit ($de00 bit 5) however is reset
$df11 bit 5 is not changed when freeze, meaning: you can put freeze code in ram!

Super Mapper mapping in freeze mode
-----------------------------------

In freeze mode, the memory map is different than Retro Replay:

$de00: bit 5 = 0
----------------
$de00 - $dfff (minus registers): 512 byte ROM area (mirrored from $9e00-$9fff)

$de00: bit 5 = 1
----------------
Disables any memory at $de00-$dfff due to CPLD routing problems. This is however
no problem, since RAM is always enabled at $8000-$9fff if you need it.

$df11: bit 5 = 0
----------------
$8000 - $9fff: 8K RAM bank low
$e000 - $ffff: 8K RAM bank hi

$df11: bit 5 = 1
----------------
$8000 - $9fff: 8K RAM bank lo
$e000 - $ffff: 8K FLASH rom bank hi

*/
void mmcreplay_freeze (void)
{
    LOG (("MMCREPLAY: freeze"));
    freeze_pressed = 1;
    rr_active = 1;
    /* ultimax mode */
    enable_exrom = 1 ^ 1;
    enable_game = 1;

    bank_address_13_15 = 0;     /* always cleared when freezed */
/*
enable_16k_mapping=0;
disable_mmc_bios=0;
enable_ram_io1=0;
*/

#ifdef TEST_AR_MAPPER           /* bank 0 */
    enable_ram_io = 0;
    enable_rr_regs = 1;
#elif defined(TEST_RR_MAPPER)   /* bank 4 */
    enable_ram_io = 1;          /* FIXME: should not actually be set here! */
    enable_rr_regs = 1;
#elif defined(TEST_NORDIC_MAPPER)       /* bank 1 */
    enable_ram_io = 0;
    enable_rr_regs = 1;
#elif defined(TEST_SUPER_8KCRT) /* bank 2 */
    enable_ram_io = 0;
    enable_rr_regs = 0;
#elif defined(TEST_SUPER_16KCRT)        /* bank 3 */
    enable_ram_io = 0;
    enable_rr_regs = 0;
#else
//    bank_address_13_15=(56&7);
//    bank_address_16_18=((56>>3)&7);
    enable_ram_io = 0;
    enable_rr_regs = 1;
#endif
//    mmcreplay_update_mapper();
//    mmcreplay_config_changed(0x20|(enable_exrom<<1)|enable_game,0x20|(enable_exrom<<1)|enable_game, CMODE_READ);
    /* FIXME: OR 0x20 ? enable ram in freeze mode ? */
    mmcreplay_update_mapper (CMODE_READ, 0);
    flashrom_init ();
}

void mmcreplay_reset (void)
{
    LOG (("mmcreplay_reset"));
    rr_active = 1;

    /* 8 game */
    enable_exrom = 1 ^ 1;
    enable_game = 0;

    enable_ram_io = 0;
    enable_ram_io1 = 0;
    enable_raml = 0;
    enable_ramh = 0;
    allow_bank = 0;
    disable_mmc_bios = 0;
    enable_16k_mapping = 0;
    enable_extended_mode = 0;   /* enable nordic power mode */

    bank_address_13_15 = 0;
    bank_address_16_18 = 7;

    if (enable_rescue_mode)
    {
//        enable_exrom = 0;
        //      enable_game = 0;
        log_debug ("MMCREPLAY: Rescue Mode enabled");
    }



    mmcreplay_update_mapper (CMODE_READ, 0);
    flashrom_init ();
//    mmcreplay_set_stdcfg();
    LOG (("mmcreplay_reset_end"));
}

/*
The MMC-Replay Bios sets the following configurations:
ACTION REPLAY : Allowbank = 0, $df10-$df13 registers disabled. Nordic Power on.
RETRO REPLAY: $de01 unset, $df10-$df13 registers disabled
SUPER MAPPER: 16K mode, Standard cart mode, $df10-$df13 registers enabled
*/

void mmcreplay_set_stdcfg (void)
{
    enable_ram_io = 0;
    enable_ram_io1 = 0;
    enable_raml = 0;
    enable_ramh = 0;
    allow_bank = 0;

    enable_mmc_regs = 0;
    mmcr_clockport_enabled = 0;
    bank_address_13_15 = 0;

    /* start in 8k game config */
    enable_exrom = 1 ^ 1;
    enable_game = 0;

    write_once = 0;


    bank_address_16_18 = 7;
    rr_active = 1;
    enable_rr_regs = 1;
    enable_16k_mapping = 0;
    enable_extended_mode = 0;   /* enable nordic power mode */
    enable_mmc_regs = 1;
    no_freeze = 0;
    allow_bank = 0;

    enable_ram_io = 0;          //?
//    enable_ram_io1=0;


#ifdef TEST_AR_MAPPER
    bank_address_16_18 = 0;
    rr_active = 1;
    enable_rr_regs = 1;
    no_freeze = 0;
    allow_bank = 1;
    enable_ram_io = 0;
    enable_ram_io1 = 0;
    enable_16k_mapping = 0;
    disable_mmc_bios = 0;

#elif defined(TEST_RR_MAPPER)   /* bank 4 */
    bank_address_16_18 = 4;
    rr_active = 1;
    enable_rr_regs = 1;
    no_freeze = 0;
//    allow_bank = 1;
    enable_ram_io = 0;          //mmmmh
    enable_ram_io1 = 0;
    enable_16k_mapping = 0;
    disable_mmc_bios = 0;


#elif defined(TEST_NORDIC_MAPPER)       /* bank 1 */
    bank_address_16_18 = 1;
    rr_active = 1;
    enable_extended_mode = 1;   /* enable nordic power mode */
    enable_rr_regs = 1;
    no_freeze = 0;
    allow_bank = 0;
//    enable_ram_io=0;
    enable_ram_io1 = 0;
//    mmcreplay_config_changed((0<<1)|0, (0<<1)|0, CMODE_READ);
    enable_16k_mapping = 0;
    disable_mmc_bios = 0;

#elif defined(TEST_SUPER_8KCRT) /* bank 2 */
    bank_address_16_18 = 2;
    rr_active = 1;
    enable_extended_mode = 0;   /* enable nordic power mode */
    enable_rr_regs = 0;
    no_freeze = 0;
    allow_bank = 0;
    enable_ram_io = 0;
    enable_ram_io1 = 0;
    enable_16k_mapping = 0;
    disable_mmc_bios = 0;

#elif defined(TEST_SUPER_16KCRT)        /* bank 3 */
    bank_address_16_18 = 3;
    rr_active = 1;
    enable_extended_mode = 0;   /* enable nordic power mode */
    enable_rr_regs = 0;
    no_freeze = 0;
    allow_bank = 0;
    enable_ram_io = 0;
    enable_ram_io1 = 0;
    enable_16k_mapping = 1;
    disable_mmc_bios = 0;

    enable_exrom = 1;
    enable_game = 1;
#elif defined(TEST_RESCUE_MODE)
//    enable_exrom = 1;
//    enable_game = 0;
    enable_rescue_mode = 1;
    enable_mmc_regs = 1;
    enable_mmc_regs_pending = 1;
#else
#endif

}

void mmcreplay_config_init (void)
{
    LOG (("mmcreplay_config_init"));
#if 0
    {
        int     l, h, bank;
        for (h = 0; h < 8; h++)
        {
            for (l = 0; l < 8; l++)
            {
                bank = (h << 3) + l;
//    LOG(("abnk %d",bank));
// for testing with the scanner
                export_ram0[0x0080 + (bank << 13)] = 1 + ('r' - 'a');
                export_ram0[0x0081 + (bank << 13)] = 1 + ('a' - 'a');
                export_ram0[0x0082 + (bank << 13)] = 0x30 + h;
                export_ram0[0x0083 + (bank << 13)] = 0x30 + l;
                export_ram0[0x1e80 + (bank << 13)] = 1 + ('r' - 'a');
                export_ram0[0x1e81 + (bank << 13)] = 1 + ('a' - 'a');
                export_ram0[0x1e82 + (bank << 13)] = 0x30 + h;
                export_ram0[0x1e83 + (bank << 13)] = 0x30 + l;
                export_ram0[0x1f80 + (bank << 13)] = 1 + ('r' - 'a');
                export_ram0[0x1f81 + (bank << 13)] = 1 + ('a' - 'a');
                export_ram0[0x1f82 + (bank << 13)] = 0x30 + h;
                export_ram0[0x1f83 + (bank << 13)] = 0x30 + l;
            }
        }
    }
#endif
    mmcreplay_set_stdcfg ();
//    mmcreplay_update_mapper();
//            mmcreplay_config_changed((enable_exrom<<1)|enable_game, (enable_exrom<<1)|enable_game, CMODE_WRITE);
    mmcreplay_update_mapper (CMODE_WRITE, 0);
    flashrom_init ();
    LOG (("mmcreplay_config_init end"));
}

void mmcreplay_config_setup (BYTE * rawcart)
{
    LOG (("mmcreplay_config_setup"));
    memcpy (roml_banks, rawcart, MMCREPLAY_FLASHROM_SIZE);
    memcpy (romh_banks, rawcart, MMCREPLAY_FLASHROM_SIZE);


    mmcreplay_set_stdcfg ();
//    mmcreplay_update_mapper();
//    mmcreplay_config_changed((enable_exrom<<1)|enable_game, (enable_exrom<<1)|enable_game, CMODE_READ);
    mmcreplay_update_mapper (CMODE_READ, 0);
    flashrom_init ();
    LOG (("mmcreplay_config_setup end"));
}

int mmcreplay_bin_attach (const char *filename, BYTE * rawcart)
{
    if (util_file_load (filename, rawcart, MMCREPLAY_FLASHROM_SIZE,
                        UTIL_FILE_LOAD_SKIP_ADDRESS | UTIL_FILE_LOAD_FILL) < 0)
        return -1;

    if (c64export_add (&export_res) < 0)
        return -1;


    flashrom_init ();
    mmc_open_card_image ("./mmcimage.bin");     /* FIXME */
    eeprom_open_image ("./mmceeprom.bin");      /* FIXME */

    return 0;
}

void mmcreplay_detach (void)
{
    mmc_close_card_image ();
    eeprom_close_image ();
    c64export_remove (&export_res);
}
