/* Multiple False Programming Language : Intermediate Code Generator
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

#include "selfcheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "multiple_ir.h"
#include "multiple_misc.h" 
#include "multiple_err.h"

#include "multiply_assembler.h"
#include "multiply_str_aux.h"

#include "vm_predef.h"
#include "vm_opcode.h"
#include "vm_types.h"

#include "mf_lexer.h"
#include "mf_icg_fcb.h"
#include "mf_icg_context.h"
#include "mf_icg.h"

/* Declarations */
static int mf_icodegen_generic(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token **token_cur_in_out);

static int mf_icg_fcb_block_append_from_precompiled_pic_text( \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct multiply_text_precompiled *icg_text_precompiled)
{
    int ret = 0;
    struct multiply_text_precompiled_line *text_precompiled_line_cur = NULL;
    size_t idx;

    for (idx = 0; idx != icg_text_precompiled->size; idx++)
    {
        text_precompiled_line_cur = &icg_text_precompiled->lines[idx];
        if ((ret = mf_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        text_precompiled_line_cur->opcode, \
                        text_precompiled_line_cur->operand)) != 0)
        { goto fail; }
    }

fail:
    return ret;
}

static int mf_icodegen_constant(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token **token_cur_in_out)
{
    int ret = 0;
    struct token *token_cur = *token_cur_in_out;
    char *buffer_str = NULL;
    size_t buffer_str_len;
    uint32_t id;
    int value_int;

    switch (token_cur->value)
    {
        case TOKEN_CONSTANT_STRING:
            /* String */
            buffer_str_len = token_cur->len;
            buffer_str = (char *)malloc(sizeof(char) * (buffer_str_len + 1));
            if (buffer_str == NULL)
            {
                MULTIPLE_ERROR_MALLOC();
                ret = -MULTIPLE_ERR_MALLOC;
                goto fail; 
            }
            memcpy(buffer_str, token_cur->str, token_cur->len);
            buffer_str[buffer_str_len] = '\0';
            multiply_replace_escape_chars(buffer_str, &buffer_str_len);

            if ((ret = multiply_resource_get_str( \
                            err, \
                            context->icode, \
                            context->res_id, \
                            &id, \
                            buffer_str, \
                            buffer_str_len)) != 0)
            { goto fail; }
            if ((ret = mf_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id)) != 0) 
            { goto fail; }
            if ((ret = mf_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PRINT, 0)) != 0) 
            { goto fail; }

            free(buffer_str);
            buffer_str = NULL;
            break;

        default:
            if (multiply_convert_str_to_int(&value_int, \
                        token_cur->str, 
                        token_cur->len) != 0)
            {
                multiple_error_update(err, -MULTIPLE_ERR_ICODEGEN, \
                        "\'%s\' is an invalid integer", \
                        token_cur->str);
                ret = -MULTIPLE_ERR_ICODEGEN; 
                goto fail; 
            }

            if ((ret = multiply_resource_get_int( \
                            err, \
                            context->icode, \
                            context->res_id, \
                            &id, \
                            value_int)) != 0)
            { goto fail; }
            if ((ret = mf_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id)) != 0) { goto fail; }

            break;
    }

    goto done;
fail:
    if (buffer_str != NULL) free(buffer_str);
done:
    *token_cur_in_out = token_cur;
    return ret;
}

#define IS_TOKEN_NORMAL(x) \
    (((x)==TOKEN_OP_ADD)|| \
     ((x)==TOKEN_OP_SUB)|| \
     ((x)==TOKEN_OP_MUL)|| \
     ((x)==TOKEN_OP_DIV)|| \
     ((x)==TOKEN_OP_UNARY_MINUS)|| \
     ((x)==TOKEN_OP_DUP)|| \
     ((x)==TOKEN_OP_DROP)|| \
     ((x)==TOKEN_OP_PRINT_INT))
static int mf_icodegen_normal(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token *token_cur)
{
    int ret = 0;
    uint32_t op = 0;

    (void)context;

    switch (token_cur->value)
    {
        case TOKEN_OP_ADD: 
            op = OP_ADD; 
            break;
        case TOKEN_OP_SUB: 
            op = OP_SUB; 
            break;
        case TOKEN_OP_MUL: 
            op = OP_MUL; 
            break;
        case TOKEN_OP_DIV: 
            op = OP_DIV; 
            break;
        case TOKEN_OP_UNARY_MINUS: 
            op = OP_NEG; 
            break;
        case TOKEN_OP_DUP: 
            op = OP_DUP; 
            break;
        case TOKEN_OP_DROP: 
            op = OP_DROP; 
            break;
        case TOKEN_OP_PRINT_INT: 
            op = OP_PRINT; 
            break;
        default: 
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }
    if ((ret = mf_icg_fcb_block_append_with_configure(icg_fcb_block, op, 0)) != 0) 
    { goto fail; }

    goto done;
fail:
done:
    return ret;
}

#define IS_TOKEN_PRINT_READ_CHAR(x) \
    (((x)==TOKEN_OP_PRINT_CHAR)|| \
     ((x)==TOKEN_OP_READ_CHAR))
static int mf_icodegen_print_char(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token *token_cur)
{
    int ret = 0;

    (void)err;
    (void)context;

    switch (token_cur->value)
    {
        case TOKEN_OP_PRINT_CHAR:
            if ((ret = mf_icg_fcb_block_append_with_configure(icg_fcb_block, \
                            OP_FASTLIB, OP_FASTLIB_PUTCHAR)) != 0)
            { goto fail; }
            break;
        case TOKEN_OP_READ_CHAR:
            if ((ret = mf_icg_fcb_block_append_with_configure(icg_fcb_block, \
                            OP_FASTLIB, OP_FASTLIB_GETCHAR)) != 0)
            { goto fail; }
            break;
    }

    goto done;
fail:
done:
    return ret;
}

#define IS_TOKEN_CMP(x) \
    (((x)==TOKEN_OP_EQ)|| \
     ((x)==TOKEN_OP_L)|| \
     ((x)==TOKEN_OP_G))
static int mf_icodegen_cmp(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token *token_cur)
{
    int ret = 0;
    const int LBL_SKIP1 = 0, LBL_SKIP2 = 1;
    uint32_t cmp_op = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;

    switch (token_cur->value)
    {
        case TOKEN_OP_EQ: 
            cmp_op = OP_EQ; 
            break;
        case TOKEN_OP_L: 
            cmp_op = OP_L; 
            break;
        case TOKEN_OP_G: 
            cmp_op = OP_G; 
            break;
        default: 
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }
    
    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \
                    MULTIPLY_ASM_OP     , cmp_op     , 
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR   , LBL_SKIP1  ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP_LBLR, OP_JMPR    , LBL_SKIP2  ,
                    MULTIPLY_ASM_LABEL  , LBL_SKIP1  ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_NEG     , 
                    MULTIPLY_ASM_LABEL  , LBL_SKIP2  ,
                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mf_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}

#define IS_TOKEN_LOGICAL_AND_OR(x) \
    (((x)==TOKEN_OP_AND)|| \
     ((x)==TOKEN_OP_OR))
static int mf_icodegen_logical_and_or(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token *token_cur)
{
    int ret = 0;
    const int LBL_SKIP1 = 0, LBL_SKIP2 = 1;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    uint32_t op;

    switch (token_cur->value)
    {
        case TOKEN_OP_AND: 
            op = OP_ANDL; 
            break;
        case TOKEN_OP_OR: 
            op = OP_ORL; 
            break;
        default:
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }

    /* At the beginning, the two elements already been pushed on the
     * top of the stack */
    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    /* Test 1st element */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          , 
                    MULTIPLY_ASM_OP     , OP_NE      , 

                    /* Pick the second element up */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 2          , 
                    MULTIPLY_ASM_OP     , OP_PICK    , 

                    /* Test 2nd element */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          , 
                    MULTIPLY_ASM_OP     , OP_NE      , 

                    /* Logical-and operation */
                    MULTIPLY_ASM_OP     , op         , 
                    MULTIPLY_ASM_OP     , OP_NOTL    , 

                    /* If false */
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR   , LBL_SKIP1  ,

                    /* True */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          , 
                    MULTIPLY_ASM_OP     , OP_NEG     , 
                    MULTIPLY_ASM_OP_LBLR, OP_JMPR    , LBL_SKIP2  ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP1  ,
                    /* False */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          , 

                    MULTIPLY_ASM_LABEL  , LBL_SKIP2  ,
                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mf_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}

#define IS_TOKEN_LOGICAL_NOT(x) \
    ((x)==TOKEN_OP_NOT)
static int mf_icodegen_logical_not(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token *token_cur)
{
    int ret = 0;
    const int LBL_SKIP1 = 0, LBL_SKIP2 = 1;
    struct multiply_text_precompiled *new_text_precompiled = NULL;

    (void)token_cur;

    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    /* Test 1st element */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          , 
                    MULTIPLY_ASM_OP     , OP_NE      , 

                    /* If false */
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR   , LBL_SKIP1  ,

                    /* True */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          , 
                    MULTIPLY_ASM_OP     , OP_NEG     , 
                    MULTIPLY_ASM_OP_LBLR, OP_JMPR    , LBL_SKIP2  ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP1  ,
                    /* False */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          , 

                    MULTIPLY_ASM_LABEL  , LBL_SKIP2  ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP2  ,
                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mf_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}


#define IS_TOKEN_SWAP(x) \
    ((x)==TOKEN_OP_SWAP)
static int mf_icodegen_swap(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token *token_cur)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;

    (void)token_cur;

    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 2          , 
                    MULTIPLY_ASM_OP     , OP_REVERSE , 
                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mf_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}

#define IS_TOKEN_ROTATE3(x) \
    ((x)==TOKEN_OP_ROTATE3)
static int mf_icodegen_rotate3(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token *token_cur)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;

    (void)token_cur;

    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 3          , 
                    MULTIPLY_ASM_OP     , OP_PICK    , 
                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mf_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}

#define IS_TOKEN_PICK(x) \
    ((x)==TOKEN_OP_PICK)
static int mf_icodegen_pick(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token *token_cur)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;

    (void)token_cur;

    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    MULTIPLY_ASM_OP     , OP_PICKCP  , 
                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mf_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}

#define IS_TOKEN_GLOBAL_VAR(x) \
    ((x)==TOKEN_VARIABLE)
static int mf_icodegen_global_variables(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token **token_cur_in_out)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    struct token *token_cur = *token_cur_in_out;
    struct token *token_var;
    struct token *token_op;
    uint32_t id;

    if (token_cur->next == NULL)
    {
        multiple_error_update(err, -MULTIPLE_ERR_ICODEGEN, \
                "%d:%d: error: variable operation expected after variable", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_ICODEGEN;
        goto fail;
    }
    token_var = token_cur; 
    token_cur = token_cur->next;

    if (!((token_cur->value == TOKEN_OP_GET_VALUE) || \
            (token_cur->value == TOKEN_OP_ASSIGN)))
    {
        multiple_error_update(err, -MULTIPLE_ERR_ICODEGEN, \
                "%d:%d: error: variable operation expected after variable", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_ICODEGEN;
        goto fail;
    }
    token_op = token_cur; 

    if ((ret = multiply_resource_get_id( \
                    err, \
                    context->icode, \
                    context->res_id, \
                    &id, \
                    token_var->str, \
                    token_var->len)) != 0)
    { goto fail; }

    switch (token_op->value)
    {
        case TOKEN_OP_GET_VALUE:
            if ((ret = mf_icg_fcb_block_append_with_configure(icg_fcb_block, \
                            OP_PUSHM, id)) != 0)
            { goto fail; }
            if ((ret = mf_icg_fcb_block_append_with_configure(icg_fcb_block, \
                            OP_SLV, 0)) != 0)
            { goto fail; }
            break;
        case TOKEN_OP_ASSIGN:
            if ((ret = mf_icg_fcb_block_append_with_configure(icg_fcb_block, \
                            OP_POPM, id)) != 0)
            { goto fail; }
            break;
    }


    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    *token_cur_in_out = token_cur;
    return ret;
}

#define IS_TOKEN_FUNC_DEFINE(x) \
    ((x)==TOKEN_OP_LEFT_BRACKET)
static int mf_icodegen_func_define(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token **token_cur_in_out)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    struct token *token_cur = *token_cur_in_out;

    struct mf_icg_fcb_block *new_icg_fcb_block = NULL;
    struct multiple_ir_export_section_item *new_export_section_item = NULL;
    uint32_t id;

    /* Skip "[" */
    token_cur = token_cur->next; 

    new_icg_fcb_block = mf_icg_fcb_block_new();
    if (new_icg_fcb_block == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }

    new_export_section_item = multiple_ir_export_section_item_new();
    if (new_export_section_item == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }
    new_export_section_item->blank = 1;

    /* Argument */
    if ((ret = multiply_resource_get_id( \
                    err, \
                    context->icode, \
                    context->res_id, \
                    &id, \
                    "arg", \
                    3)) != 0)
    { goto fail; }
    if ((ret = mf_icg_fcb_block_append_with_configure(new_icg_fcb_block, OP_ARGC, id)) != 0) { goto fail; }
    if ((ret = mf_icg_fcb_block_append_with_configure(new_icg_fcb_block, OP_PUSH, id)) != 0) { goto fail; }

    /* Body */
    if ((ret = mf_icodegen_generic(err, \
                    context, \
                    new_icg_fcb_block, \
                    &token_cur)) != 0)
    { goto fail; }

    /* Return */
    if ((ret = mf_icg_fcb_block_append_with_configure(new_icg_fcb_block, OP_RETURN, 0)) != 0)
    { goto fail; }

    /* Make Lambda */
    if ((ret = mf_icg_fcb_block_append_with_configure_type(icg_fcb_block, \
                    OP_LAMBDAMK, (uint32_t)(context->icg_fcb_block_list->size), MF_ICG_FCB_LINE_TYPE_LAMBDA_MK)) != 0)
    { goto fail; }

    /* Append block */
    if ((ret = mf_icg_fcb_block_list_append(context->icg_fcb_block_list, new_icg_fcb_block)) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        goto fail;
    }

    /* Append blank export section */
    if ((ret = multiple_ir_export_section_append(context->icode->export_section, new_export_section_item)) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        goto fail;
    }

    goto done;
fail:
    if (new_icg_fcb_block != NULL) mf_icg_fcb_block_destroy(new_icg_fcb_block);
    if (new_export_section_item != NULL) multiple_ir_export_section_item_destroy(new_export_section_item);
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    *token_cur_in_out = token_cur;
    return ret;
}

#define IS_TOKEN_FUNC_APPLY(x) \
    ((x)==TOKEN_OP_APPLY)
static int mf_icodegen_func_apply(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token **token_cur_in_out)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    struct token *token_cur = *token_cur_in_out;

    /* Pick the condition up */
    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    /* Pick the argument count */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          , 
                    /* Pick the function */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 2          , 
                    MULTIPLY_ASM_OP     , OP_PICK    , 
                    /* Apply */
                    MULTIPLY_ASM_OP     , OP_CALLC   ,

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mf_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    *token_cur_in_out = token_cur;
    return ret;
}

#define IS_TOKEN_IF(x) \
    ((x)==TOKEN_OP_IF)
static int mf_icodegen_if(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token **token_cur_in_out)
{
    int ret = 0;
    const int LBL_SKIP1 = 0, LBL_SKIP2 = 1;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    struct token *token_cur = *token_cur_in_out;

    /* Pick the condition up */
    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    /* Pick the second element up */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 2          , 
                    MULTIPLY_ASM_OP     , OP_PICK    , 

                    /* Test element */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          , 
                    MULTIPLY_ASM_OP     , OP_NE      , 

                    /* If false */
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR   , LBL_SKIP1  ,

                    /* Drop the function  */
                    MULTIPLY_ASM_OP     , OP_DROP    ,
                    MULTIPLY_ASM_OP_LBLR, OP_JMPR    , LBL_SKIP2  ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP1  ,

                    /* Push the arguments and count */
                    MULTIPLY_ASM_OP_NONE, OP_PUSH    , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          , 
                    /* Pick the function up */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 3          , 
                    MULTIPLY_ASM_OP     , OP_PICK    , 
                    /* Apply the function */
                    MULTIPLY_ASM_OP     , OP_CALLC   , 
                    /* Drop the return value  */
                    MULTIPLY_ASM_OP     , OP_DROP    ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP2  ,
                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mf_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    *token_cur_in_out = token_cur;
    return ret;
}

/* gets two lambda functions as args, 
 * one that results in a boolean, and the second as body */
#define IS_TOKEN_WHILE(x) \
    ((x)==TOKEN_OP_WHILE)
static int mf_icodegen_while(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token **token_cur_in_out)
{
    int ret = 0;
    const int LBL_HEAD = 0, LBL_SKIP1 = 1, LBL_SKIP2 = 2;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    struct token *token_cur = *token_cur_in_out;

    /* [condition][body]# */

    /* Pick the condition up */
    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    /* Head */
                    MULTIPLY_ASM_LABEL  , LBL_HEAD  ,

                    /* Pick (Copy) the second element up */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 2          , 
                    MULTIPLY_ASM_OP     , OP_PICKCP  , 

                    /* Arguments & count */
                    MULTIPLY_ASM_OP_NONE, OP_PUSH    , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          , 
                    /* Pick the condition up again */ 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 3          , 
                    MULTIPLY_ASM_OP     , OP_PICK    , 
                    /* Apply it */
                    MULTIPLY_ASM_OP     , OP_CALLC   , 
                    /* Test the result */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          , 
                    MULTIPLY_ASM_OP     , OP_NE      , 

                    /* If false */
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR   , LBL_SKIP1  ,

                    MULTIPLY_ASM_OP_LBLR, OP_JMPR    , LBL_SKIP2  ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP1  ,

                    /* Duplicate the body */
                    MULTIPLY_ASM_OP     , OP_DUP     ,
                    /* Arguments & count */
                    MULTIPLY_ASM_OP_NONE, OP_PUSH    , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          , 
                    /* Pick the body up again */ 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 3          , 
                    MULTIPLY_ASM_OP     , OP_PICK    , 
                    /* Apply the body */
                    MULTIPLY_ASM_OP     , OP_CALLC   , 
                    /* Drop the return value  */
                    MULTIPLY_ASM_OP     , OP_DROP    ,

                    /* Jump to the head */
                    MULTIPLY_ASM_OP_LBLR, OP_JMPR    , LBL_HEAD   ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP2  ,

                    /* Drop the condition and body */
                    MULTIPLY_ASM_OP     , OP_DROP    ,
                    MULTIPLY_ASM_OP     , OP_DROP    ,

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mf_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    *token_cur_in_out = token_cur;
    return ret;
}

static int mf_icodegen_generic(struct multiple_error *err, \
        struct mf_icg_context *context, \
        struct mf_icg_fcb_block *icg_fcb_block, \
        struct token **token_cur_in_out)
{
    int ret = 0;
    struct token *token_cur = *token_cur_in_out;

    while ((token_cur != NULL) && (token_cur->value != TOKEN_FINISH))
    {
        if (token_cur->value == TOKEN_OP_RIGHT_BRACKET)
        {
            break;
        }
        else if (IS_TOKEN_CONSTANT(token_cur->value))
        {
            if ((ret = mf_icodegen_constant(err, \
                            context, \
                            icg_fcb_block, \
                            &token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_NORMAL(token_cur->value))
        {
            if ((ret = mf_icodegen_normal(err, context, icg_fcb_block, token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_PRINT_READ_CHAR(token_cur->value))
        {
            if ((ret = mf_icodegen_print_char(err, context, icg_fcb_block, token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_CMP(token_cur->value))
        {
            if ((ret = mf_icodegen_cmp(err, context, icg_fcb_block, token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_LOGICAL_AND_OR(token_cur->value))
        {
            if ((ret = mf_icodegen_logical_and_or(err, context, icg_fcb_block, token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_LOGICAL_NOT(token_cur->value))
        {
            if ((ret = mf_icodegen_logical_not(err, context, icg_fcb_block, token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_SWAP(token_cur->value))
        {
            if ((ret = mf_icodegen_swap(err, context, icg_fcb_block, token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_ROTATE3(token_cur->value))
        {
            if ((ret = mf_icodegen_rotate3(err, context, icg_fcb_block, token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_PICK(token_cur->value))
        {
            if ((ret = mf_icodegen_pick(err, context, icg_fcb_block, token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_GLOBAL_VAR(token_cur->value))
        {
            if ((ret = mf_icodegen_global_variables(err, context, icg_fcb_block, &token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_FUNC_DEFINE(token_cur->value))
        {
            if ((ret = mf_icodegen_func_define(err, context, icg_fcb_block, &token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_FUNC_APPLY(token_cur->value))
        {
            if ((ret = mf_icodegen_func_apply(err, context, icg_fcb_block, &token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_IF(token_cur->value))
        {
            if ((ret = mf_icodegen_if(err, context, icg_fcb_block, &token_cur)) != 0)
            { goto fail; }
        }
        else if (IS_TOKEN_WHILE(token_cur->value))
        {
            if ((ret = mf_icodegen_while(err, context, icg_fcb_block, &token_cur)) != 0)
            { goto fail; }
        }
        else
        {
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
        }

        token_cur = token_cur->next;
    }

    goto done;
fail:
done:
    *token_cur_in_out = token_cur;
    return ret;
}

static int mf_icodegen_merge_blocks(struct multiple_error *err, \
        struct mf_icg_context *context)
{
    int ret = 0;
    struct mf_icg_fcb_block *icg_fcb_block_cur;
    struct mf_icg_fcb_line *icg_fcb_line_cur;

    uint32_t instrument_number;
    struct multiple_ir_export_section_item *export_section_item_cur;
    struct multiple_ir_text_section_item *text_section_item_cur;

    uint32_t offset_start;
    uint32_t fcb_size = 0;

    uint32_t count;

	/* Do not disturb the instrument produced by other way */
	offset_start = (uint32_t)(context->icode->text_section->size);
	
	export_section_item_cur = context->icode->export_section->begin;
    icg_fcb_block_cur = context->icg_fcb_block_list->begin;
    while (icg_fcb_block_cur != NULL)
    {
        icg_fcb_line_cur = icg_fcb_block_cur->begin;

        /* Record the absolute instrument number */
        instrument_number = (uint32_t)context->icode->text_section->size;
        if (export_section_item_cur == NULL)
        {
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
        }
        export_section_item_cur->instrument_number = instrument_number;

        while (icg_fcb_line_cur != NULL)
        {
            switch (icg_fcb_line_cur->type)
            {
                case MF_ICG_FCB_LINE_TYPE_NORMAL:
                    if ((ret = multiply_icodegen_text_section_append(err, \
                                    context->icode, \
                                    icg_fcb_line_cur->opcode, icg_fcb_line_cur->operand)) != 0)
                    { goto fail; }
                    break;
                case MF_ICG_FCB_LINE_TYPE_PC:
                    if ((ret = multiply_icodegen_text_section_append(err, \
                                    context->icode, \
                                    icg_fcb_line_cur->opcode, instrument_number + icg_fcb_line_cur->operand)) != 0)
                    { goto fail; }
                    break;
                case MF_ICG_FCB_LINE_TYPE_LAMBDA_MK:
                    /* Operand of this instrument here is the index number of lambda */
                    if ((ret = multiply_icodegen_text_section_append(err, \
                                    context->icode, \
                                    icg_fcb_line_cur->opcode, icg_fcb_line_cur->operand)) != 0)
                    { goto fail; }
                    break;
                case MF_ICG_FCB_LINE_TYPE_BLTIN_PROC_MK:
                    if ((ret = multiply_icodegen_text_section_append(err, \
                                    context->icode, \
                                    icg_fcb_line_cur->opcode, icg_fcb_line_cur->operand)) != 0)
                    { goto fail; }
                    break;
            }

            fcb_size += 1;
            icg_fcb_line_cur = icg_fcb_line_cur->next;
        }

        icg_fcb_block_cur = icg_fcb_block_cur->next;
        export_section_item_cur = export_section_item_cur->next;
    }

    /* 2nd pass, dealing with lambdas */
    icg_fcb_block_cur = context->icg_fcb_block_list->begin;
    /* Skip text body of built-in procedures at the beginning part */
    text_section_item_cur = context->icode->text_section->begin;
    while (offset_start-- > 0)
    {
        text_section_item_cur = text_section_item_cur->next; 
    }
    /* Process lambda mks */
    while (icg_fcb_block_cur != NULL)
    {
        icg_fcb_line_cur = icg_fcb_block_cur->begin;
        while (icg_fcb_line_cur != NULL)
        {
            if (icg_fcb_line_cur->type == MF_ICG_FCB_LINE_TYPE_LAMBDA_MK)
            {
                /* Locate to the export section item */
                count = icg_fcb_line_cur->operand;
                export_section_item_cur = context->icode->export_section->begin;
                while ((export_section_item_cur != NULL) && (count != 0))
                {
                    count--;
                    export_section_item_cur = export_section_item_cur->next;
                }
                if (export_section_item_cur == NULL)
                {
                    MULTIPLE_ERROR_INTERNAL();
                    ret = -MULTIPLE_ERR_INTERNAL;
                    goto fail;
                }
                text_section_item_cur->operand = export_section_item_cur->instrument_number; 
            }
            text_section_item_cur = text_section_item_cur->next; 
            icg_fcb_line_cur = icg_fcb_line_cur->next;
        }

        icg_fcb_block_cur = icg_fcb_block_cur->next;
    }

    goto done;
fail:
done:
    return ret;
}

int mf_irgen(struct multiple_error *err, \
        struct multiple_ir **icode_out, \
        struct token_list *tokens, \
        int verbose)
{
    int ret = 0;
    struct mf_icg_context context;
    struct mf_icg_fcb_block_list *new_icg_fcb_block_list = NULL;
    struct mf_icg_fcb_block *new_icg_fcb_block_main = NULL;
    uint32_t id;
    uint32_t id_null;
    struct multiple_ir_export_section_item *new_export_section_item = NULL;
    struct multiple_ir *new_icode = NULL;
    struct multiply_resource_id_pool *new_res_id = NULL;
    struct token *token_cur = tokens->begin;

    (void)verbose;

    new_icg_fcb_block_list = mf_icg_fcb_block_list_new();
    if (new_icg_fcb_block_list == NULL) 
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    new_icg_fcb_block_main = mf_icg_fcb_block_new();
    if (new_icg_fcb_block_main == NULL) 
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    new_export_section_item = multiple_ir_export_section_item_new();
    if (new_export_section_item == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    new_export_section_item->args_count = 0;
    new_export_section_item->args = NULL;
    new_export_section_item->args_types = NULL;

    new_icode = multiple_ir_new();
    if (new_icode == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    new_res_id = multiply_resource_id_pool_new();
    if (new_res_id == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    mf_icg_context_init(&context);
    context.icg_fcb_block_list = new_icg_fcb_block_list;
    context.icode = new_icode;
    context.res_id = new_res_id;

    /* Generating icode for 'main' */
    if ((ret = mf_icodegen_generic(err, \
                    &context, \
                    new_icg_fcb_block_main, \
                    &token_cur)) != 0)
    { goto fail; }

    /* Return */
    if ((ret = multiply_resource_get_none( \
                    err, \
                    new_icode, \
                    new_res_id, \
                    &id_null)) != 0) 
    { goto fail; }
    if ((ret = mf_icg_fcb_block_append_with_configure(new_icg_fcb_block_main, OP_PUSH, id_null)) != 0)
    { goto fail; }
    if ((ret = mf_icg_fcb_block_append_with_configure(new_icg_fcb_block_main, OP_RETURN, 0)) != 0)
    { goto fail; }

    /* Append block */
    if ((ret = mf_icg_fcb_block_list_append(new_icg_fcb_block_list, new_icg_fcb_block_main)) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        goto fail;
    }
    new_icg_fcb_block_main = NULL;

    /* Append export section item */
    if ((ret = multiply_resource_get_id( \
                    err, \
                    new_icode, \
                    new_res_id, \
                    &id, \
                    "main", 4)) != 0)
    { goto fail; }

    new_export_section_item->name = id;
    new_export_section_item->instrument_number = (uint32_t)new_icode->export_section->size;
    if ((ret = multiple_ir_export_section_append(new_icode->export_section, new_export_section_item)) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        goto fail;
    }
    new_export_section_item = NULL;

    /* Merge blocks */
    if ((ret = mf_icodegen_merge_blocks(err, \
                    &context)) != 0)
    { goto fail; }

    *icode_out = new_icode;

    ret = 0;
    goto done;
fail:
    if (new_icode != NULL) multiple_ir_destroy(new_icode);
    if (new_icg_fcb_block_main != NULL) mf_icg_fcb_block_destroy(new_icg_fcb_block_main);
    if (new_export_section_item != NULL) multiple_ir_export_section_item_destroy(new_export_section_item);
done:
    if (new_res_id != NULL) multiply_resource_id_pool_destroy(new_res_id);
    if (new_icg_fcb_block_list != NULL) mf_icg_fcb_block_list_destroy(new_icg_fcb_block_list);
    return ret;
}

