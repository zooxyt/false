/* Multiple False Programming Language : Intermediate Code Generator
 * Global Context
   Copyright(C) 2014 Cheryl Natsu

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

#ifndef _MF_ICG_CONTEXT_H_
#define _MF_ICG_CONTEXT_H_

#include "multiple_ir.h"

#include "multiply.h"

#include "mf_icg_fcb.h"

struct mf_icg_context
{
    struct mf_icg_fcb_block_list *icg_fcb_block_list;
    struct multiple_ir *icode;
    struct multiply_resource_id_pool *res_id;
};

int mf_icg_context_init(struct mf_icg_context *context);
int mf_icg_context_uninit(struct mf_icg_context *context);

#endif


