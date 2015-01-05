/* Multiple False Programming Language : Intermediate Code Generator
 * Floating Code Block
   Copyright(C) 2013-2014 Cheryl Natsu

   This file is part of multiple - Multiple Paradigm Language Interpreter

   multiple is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   multiple is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. 
   */

#ifndef _MF_ICG_FCB_H_
#define _MF_ICG_FCB_H_

#include <stdio.h>
#include <stdint.h>

/* Attributes for each line */

struct mf_icg_fcb_line_attr
{
    uint32_t attr_id;
    uint32_t res_id;
    struct mf_icg_fcb_line_attr *next;
};

struct mf_icg_fcb_line_attr *mf_icg_fcb_line_attr_new(uint32_t attr_id, \
        uint32_t res_id); 
int mf_icg_fcb_line_attr_destroy(struct mf_icg_fcb_line_attr *attr);

struct mf_icg_fcb_line_attr_list
{
    struct mf_icg_fcb_line_attr *begin;
    struct mf_icg_fcb_line_attr *end;
    size_t size;
};

struct mf_icg_fcb_line_attr_list *mf_icg_fcb_line_attr_list_new(void);
int mf_icg_fcb_line_attr_list_destroy(struct mf_icg_fcb_line_attr_list *list);
int mf_icg_fcb_line_attr_list_append(struct mf_icg_fcb_line_attr_list *list, \
        struct mf_icg_fcb_line_attr *new_attr);
int mf_icg_fcb_line_attr_list_append_with_configure(struct mf_icg_fcb_line_attr_list *list, \
        uint32_t attr_id, uint32_t res_id);


enum
{
    /* No needed to do anything to operand */
    MF_ICG_FCB_LINE_TYPE_NORMAL = 0,        

    /* operand = global_start + operand */
    MF_ICG_FCB_LINE_TYPE_PC = 1,            

    /* operand = global_offsets_of_lambda_procs[res_id] */
    MF_ICG_FCB_LINE_TYPE_LAMBDA_MK = 2,     

    /* operand = global_offsets_of_built_in_proces[res_id] 
     * At the beginning of __init__ */
    MF_ICG_FCB_LINE_TYPE_BLTIN_PROC_MK = 3, 
};

struct mf_icg_fcb_line
{
    uint32_t opcode;
    uint32_t operand;
    int type;
    struct mf_icg_fcb_line_attr_list *attrs;

    struct mf_icg_fcb_line *prev;
    struct mf_icg_fcb_line *next;
};
struct mf_icg_fcb_line *mf_icg_fcb_line_new(void);
int mf_icg_fcb_line_destroy(struct mf_icg_fcb_line *icg_fcb_line);
struct mf_icg_fcb_line *mf_icg_fcb_line_new_with_configure(uint32_t opcode, uint32_t operand);
struct mf_icg_fcb_line *mf_icg_fcb_line_new_with_configure_type(uint32_t opcode, uint32_t operand, int type);

struct mf_icg_fcb_block
{
    struct mf_icg_fcb_line *begin;
    struct mf_icg_fcb_line *end;
    size_t size;

    struct mf_icg_fcb_block *prev;
    struct mf_icg_fcb_block *next;
};
struct mf_icg_fcb_block *mf_icg_fcb_block_new(void);
int mf_icg_fcb_block_destroy(struct mf_icg_fcb_block *icg_fcb_block);
int mf_icg_fcb_block_append(struct mf_icg_fcb_block *icg_fcb_block, \
        struct mf_icg_fcb_line *new_icg_fcb_line);
int mf_icg_fcb_block_insert(struct mf_icg_fcb_block *icg_fcb_block, \
        uint32_t instrument_number_insert, \
        struct mf_icg_fcb_line *new_icg_fcb_line);

int mf_icg_fcb_block_append_with_configure(struct mf_icg_fcb_block *icg_fcb_block, \
        uint32_t opcode, uint32_t operand);
int mf_icg_fcb_block_append_with_configure_type(struct mf_icg_fcb_block *icg_fcb_block, \
        uint32_t opcode, uint32_t operand, int type);

uint32_t mf_icg_fcb_block_get_instrument_number(struct mf_icg_fcb_block *icg_fcb_block);

int mf_icg_fcb_block_insert_with_configure(struct mf_icg_fcb_block *icg_fcb_block, \
        uint32_t instrument, uint32_t opcode, uint32_t operand);
int mf_icg_fcb_block_insert_with_configure_type(struct mf_icg_fcb_block *icg_fcb_block, \
        uint32_t instrument, uint32_t opcode, uint32_t operand, int type);

int mf_icg_fcb_block_link(struct mf_icg_fcb_block *icg_fcb_block, \
        uint32_t instrument_number_from, uint32_t instrument_number_to);

struct mf_icg_fcb_block_list
{
    struct mf_icg_fcb_block *begin;
    struct mf_icg_fcb_block *end;
    size_t size;
};
struct mf_icg_fcb_block_list *mf_icg_fcb_block_list_new(void);
int mf_icg_fcb_block_list_destroy(struct mf_icg_fcb_block_list *icg_fcb_block_list);
int mf_icg_fcb_block_list_append(struct mf_icg_fcb_block_list *icg_fcb_block_list, \
        struct mf_icg_fcb_block *new_icg_fcb_block);

#endif

