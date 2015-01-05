/* Multiple False Programming Language : Lexical Scanner
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

#ifndef _MF_LEXER_H_
#define _MF_LEXER_H_

#include <stdio.h>

#include "multiply_lexer.h"

/* Token Types */
enum
{
    TOKEN_VARIABLE = CUSTOM_TOKEN_STARTPOINT,
    TOKEN_CHAR,            /* `. */
    TOKEN_OP_ADD,          /* + */
    TOKEN_OP_SUB,          /* - */
    TOKEN_OP_MUL,          /* * */
    TOKEN_OP_DIV,          /* / */
    TOKEN_OP_UNARY_MINUS,  /* _ */
    TOKEN_OP_EQ,           /* = */
    TOKEN_OP_G,            /* > */
    TOKEN_OP_L,            /* < */
    TOKEN_OP_AND,          /* & */
    TOKEN_OP_OR,           /* | */
    TOKEN_OP_NOT,          /* ~ */
    TOKEN_OP_ASSIGN,       /* : */
    TOKEN_OP_GET_VALUE,    /* ; */
    TOKEN_OP_LEFT_BRACKET, /* [ */
    TOKEN_OP_RIGHT_BRACKET,/* ] */
    TOKEN_OP_APPLY,        /* ! */
    TOKEN_OP_DUP,          /* $ */
    TOKEN_OP_DROP,         /* % */
    TOKEN_OP_SWAP,         /* \ */
    TOKEN_OP_ROTATE3,      /* @ */
    TOKEN_OP_PICK,         /* ø (0 slash) */
    TOKEN_OP_IF,           /* ? */
    TOKEN_OP_WHILE,        /* # */
    TOKEN_OP_PRINT_INT,    /* . */
    TOKEN_OP_PRINT_CHAR,   /* , */
    TOKEN_OP_READ_CHAR,    /* ^ */
    TOKEN_OP_FLUSH,        /* ß (beta) */
};

/* Get token name */
int mf_token_name(char **token_name, size_t *token_name_len, const int value);

/* Lexical scan source code */
int mf_tokenize(struct multiple_error *err, struct token_list **list_out, const char *data, const size_t data_len);

#endif


