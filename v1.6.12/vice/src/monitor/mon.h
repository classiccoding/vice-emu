/*
 * mon.h - The VICE built-in monitor.
 *
 * Written by
 *  Daniel Sladic <sladic@eecg.toronto.edu>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#ifndef _MON_H
#define _MON_H

#include "types.h"

/* Types */

typedef int bool;

struct mon_cmds {
   const char *str;
   const char *abbrev;
   int token;
   int next_state;
   const char *param_names;
   const char *description;
};

enum mon_int {
    MI_NONE = 0,
    MI_BREAK = 1 << 0,
    MI_WATCH = 1 << 1,
    MI_STEP = 1 << 2
};

enum t_reg_id {
   e_A,
   e_X,
   e_Y,
   e_PC,
   e_SP,
   e_FLAGS
};
typedef enum t_reg_id REG_ID;

enum t_memory_op {
   e_load,
   e_store,
   e_load_store
};
typedef enum t_memory_op MEMORY_OP;

enum t_memspace {
   e_default_space = 0,
   e_comp_space,
   e_disk8_space,
   e_disk9_space,
   e_invalid_space
};
typedef enum t_memspace MEMSPACE;

typedef unsigned int MON_ADDR;
typedef unsigned int MON_REG;

enum t_conditional {
   e_INV,
   e_EQU,
   e_NEQ,
   e_GT,
   e_LT,
   e_GTE,
   e_LTE,
   e_AND,
   e_OR
};
typedef enum t_conditional CONDITIONAL;

enum t_radixtype {
   e_default_radix,
   e_hexadecimal,
   e_decimal,
   e_octal,
   e_binary
};
typedef enum t_radixtype RADIXTYPE;

enum t_action {
   e_OFF = 0,
   e_ON = 1,
   e_TOGGLE = 2
};
typedef enum t_action ACTION;

struct t_cond_node {
   int operation;
   int value;
   MON_REG reg_num;
   bool is_reg;
   bool is_parenthized;
   struct t_cond_node *child1;
   struct t_cond_node *child2;
};
typedef struct t_cond_node CONDITIONAL_NODE;

struct t_breakpoint {
   int brknum;
   MON_ADDR start_addr;
   MON_ADDR end_addr;
   int hit_count;
   int ignore_count;
   CONDITIONAL_NODE *condition;
   char *command;
   bool trace;
   bool enabled;
   bool watch_load;
   bool watch_store;
   bool temporary;
};
typedef struct t_breakpoint breakpoint;

struct t_break_list {
   breakpoint *brkpt;
   struct t_break_list *next;
};
typedef struct t_break_list BREAK_LIST;

typedef void monitor_toggle_func_t(int value);

struct cpu_int_status_s;
struct mos6510_regs_s;

/* This is the standard interface through which the monitor accesses a
   certain CPU.  */
struct monitor_interface_s {

    /* Pointer to the registers of the CPU.  */
    struct mos6510_regs_s *cpu_regs;

    /* Pointer to the alarm/interrupt status.  */
    struct cpu_int_status_s *int_status;

    /* Pointer to the machine's clock counter.  */
    CLOCK *clk;

#if 0
    /* Pointer to a function that writes to memory.  */
    read_func_t *read_func;

    /* Pointer to a function that reads from memory.  */
    store_func_t *store_func;
#endif

    int current_bank;
    const char **(*mem_bank_list)(void);
    int (*mem_bank_from_name)(const char *name);
    BYTE (*mem_bank_read)(int bank, ADDRESS addr);
    BYTE (*mem_bank_peek)(int bank, ADDRESS addr);
    void (*mem_bank_write)(int bank, ADDRESS addr, BYTE byte);

    /* Pointer to a function to disable/enable watchpoint checking.  */
    monitor_toggle_func_t *toggle_watchpoints_func;

};
typedef struct monitor_interface_s monitor_interface_t;

/* Defines */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define HI16(x) ((x)&0xffff0000)
#define LO16(x) ((x)&0xffff)
#define LO16_TO_HI16(x) (((x)&0xffff)<<16)
#define HI16_TO_LO16(x) (((x)>>16)&0xffff)

#define STATE_INITIAL  0
#define STATE_FNAME    1
#define STATE_REG_ASGN 2
#define STATE_ROL      3
#define STATE_BNAME    4
#define STATE_CTYPE    5

#define FIRST_SPACE e_comp_space
#define LAST_SPACE e_disk9_space

#define NUM_MEMSPACES 4
#define DEFAULT_DISASSEMBLY_SIZE 40

#define check_breakpoints(mem, addr) mon_check_checkpoint(mem, addr, breakpoints[mem])
#define any_breakpoints(mem) (breakpoints[(mem)] != NULL)
#define any_watchpoints_load(mem) (watchpoints_load[(mem)] != NULL)
#define any_watchpoints_store(mem) (watchpoints_store[(mem)] != NULL)
#define any_watchpoints(mem) (watchpoints_load[(mem)] || watchpoints_store[(mem)])

#define new_cond ((CONDITIONAL_NODE *)(xmalloc(sizeof(CONDITIONAL_NODE))))
#define addr_memspace(ma) (HI16_TO_LO16(ma))
#define addr_location(ma) (LO16(ma))
#define new_addr(m, l) (LO16_TO_HI16(m) | (l))
#define new_reg(m, r) (LO16_TO_HI16(m) | (r))
#define reg_memspace(mr) (HI16_TO_LO16(mr))
#define reg_regid(mr) (LO16(mr))

/* Global variables */

extern const char *_mon_space_strings[];

extern struct mon_cmds mon_cmd_array[];

struct console_s;

extern struct console_s *console_log;
extern int sidefx;
extern int exit_mon;
extern int mon_console_close_on_leaving;
extern RADIXTYPE default_radix;
extern MEMSPACE default_memspace;
extern bool asm_mode;
extern MEMSPACE caller_space;
extern unsigned mon_mask[NUM_MEMSPACES];
extern char *playback_name;
extern bool playback;

extern BREAK_LIST *breakpoints[NUM_MEMSPACES];
extern BREAK_LIST *watchpoints_load[NUM_MEMSPACES];
extern BREAK_LIST *watchpoints_store[NUM_MEMSPACES];

/* Function declarations */
struct monitor_cpu_type_s;

extern void monitor_init(monitor_interface_t *maincpu_interface,
                         monitor_interface_t *drive8_interface_init,
                         monitor_interface_t *drive9_interface_init,
                         struct monitor_cpu_type_s **asmarray);
extern bool mon_force_import(MEMSPACE mem);
extern void mon_check_icount(ADDRESS a);
extern void mon_check_icount_interrupt(void);
extern void mon_check_watchpoints(ADDRESS a);
extern void mon(ADDRESS a);
extern void mon_open(ADDRESS a);
extern int  mon_process(char *cmd);
extern void mon_close(int check);

extern void mon_add_number_to_buffer(int number);
extern void mon_add_string_to_buffer(char *str);
extern int mon_cmd_lookup_index(char *str, int *push_back);
extern int mon_cmd_get_token(int mon_index);
extern int mon_cmd_get_next_state(int mon_index);

extern void mon_start_assemble_mode(MON_ADDR addr, char *asm_line);
extern void mon_disassemble_lines(MON_ADDR start_addr, MON_ADDR end_addr);
extern int mon_assemble_instr(char *opcode_name, unsigned operand);

extern void mon_display_memory(int radix_type, MON_ADDR start_addr, MON_ADDR end_addr);
extern void mon_display_data(MON_ADDR start_addr, MON_ADDR end_addr, int x, int y);
extern void mon_display_screen(void);
extern void mon_move_memory(MON_ADDR start_addr, MON_ADDR end_addr, MON_ADDR dest);
extern void mon_compare_memory(MON_ADDR start_addr, MON_ADDR end_addr, MON_ADDR dest);
extern void mon_fill_memory(MON_ADDR start_addr, MON_ADDR end_addr, unsigned char *data);
extern void mon_hunt_memory(MON_ADDR start_addr, MON_ADDR end_addr, unsigned char *data);
extern void mon_load_file(char *filename, MON_ADDR start_addr, bool is_bload);
extern void mon_save_file(char *filename, MON_ADDR start_addr, MON_ADDR end_addr, bool is_bsave);
extern void mon_verify_file(char *filename, MON_ADDR start_addr);
extern void mon_instructions_step(int count);
extern void mon_instructions_next(int count);
extern void mon_instruction_return(void);
extern void mon_stack_up(int count);
extern void mon_stack_down(int count);
extern void mon_block_cmd(int op, int track, int sector, MON_ADDR addr);
extern void mon_print_convert(int val);
extern void mon_change_dir(char *path);
extern void mon_execute_disk_command(char *cmd);
extern void mon_print_help(char *cmd);
extern void mon_cpu_type(char *cpu_type);
extern void mon_bank(MEMSPACE mem, char *bank);
extern void mon_display_io_regs(void);

extern unsigned int mon_get_reg_val(MEMSPACE mem, REG_ID reg_id);
extern void mon_set_reg_val(MEMSPACE mem, REG_ID reg_id, WORD val);
unsigned char mon_get_mem_val(MEMSPACE mem, unsigned mem_addr);
unsigned char mon_get_mem_val_ex(MEMSPACE mem, int bank, unsigned mem_addr);
extern void mon_print_registers(MEMSPACE mem);
extern void mon_jump(MON_ADDR addr);

extern char *mon_symbol_table_lookup_name(MEMSPACE mem, ADDRESS addr);
extern int mon_symbol_table_lookup_addr(MEMSPACE mem, char *name);
extern void mon_add_name_to_symbol_table(MON_ADDR addr, char *name);
extern void mon_remove_name_from_symbol_table(MEMSPACE mem, char *name);
extern void mon_print_symbol_table(MEMSPACE mem);
extern void mon_load_symbols(MEMSPACE mem, char *filename);
extern void mon_save_symbols(MEMSPACE mem, char *filename);

extern void mon_record_commands(char *filename);
extern void mon_end_recording(void);

extern int mon_check_checkpoint(MEMSPACE mem, ADDRESS addr, BREAK_LIST *list);
extern int mon_add_checkpoint(MON_ADDR start_addr, MON_ADDR end_addr, bool is_trace,
                              bool is_load, bool is_store, bool is_temp);
extern void mon_delete_checkpoint(int brknum);
extern void mon_print_checkpoints(void);
extern void mon_switch_checkpoint(int op, int breakpt_num);
extern void mon_set_ignore_count(int breakpt_num, int count);
extern void mon_set_checkpoint_condition(int brk_num, CONDITIONAL_NODE *cnode);
extern void mon_set_checkpoint_command(int brk_num, char *cmd);
extern void mon_watch_push_load_addr(ADDRESS addr, MEMSPACE mem);
extern void mon_watch_push_store_addr(ADDRESS addr, MEMSPACE mem);

extern const char *mon_disassemble_to_string(MEMSPACE, ADDRESS addr, BYTE x, BYTE p1,
                                             BYTE p2, int hex_mode);
extern const char *mon_disassemble_to_string_ex(MEMSPACE, ADDRESS addr, BYTE x, BYTE p1,
                                             BYTE p2, int hex_mode, unsigned *len);

#endif
