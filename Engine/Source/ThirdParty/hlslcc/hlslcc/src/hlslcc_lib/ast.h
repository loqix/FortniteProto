// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

// This code is modified from that in the Mesa3D Graphics library available at
// http://mesa3d.org/
// The license for the original code follows:

/* -*- c++ -*- */
/*
* Copyright © 2009 Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice (including the next
* paragraph) shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#pragma once
#ifndef AST_H
#define AST_H

#include "cclist.h"
#include "glsl_parser_extras.h"

struct _mesa_glsl_parse_state;

struct YYLTYPE;

/**
* \defgroup AST Abstract syntax tree node definitions
*
* An abstract syntax tree is generated by the parser.  This is a fairly
* direct representation of the gramma derivation for the source program.
* No symantic checking is done during the generation of the AST.  Only
* syntactic checking is done.  Symantic checking is performed by a later
* stage that converts the AST to a more generic intermediate representation.
*
*@{
*/

/**
* Base class of all abstract syntax tree nodes
*/
class ast_node
{
public:
	/* Callers of this ralloc-based new need not call delete. It's
	* easier to just ralloc_free 'ctx' (or any of its ancestors). */
	static void* operator new(size_t size, void *ctx)
	{
		void *node;

		node = rzalloc_size(ctx, size);
		check(node != NULL);

		return node;
	}

	/* If the user *does* call delete, that's OK, we will just
	* ralloc_free in that case. */
	static void operator delete(void *table)
	{
		ralloc_free(table);
	}

	/**
	* Print an AST node in something approximating the original GLSL code
	*/
	virtual void print(void) const;

	/**
	* Convert the AST node to the high-level intermediate representation
	*/
	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	/**
	* Retrieve the source location of an AST node
	*
	* This function is primarily used to get the source position of an AST node
	* into a form that can be passed to \c _mesa_glsl_error.
	*
	* \sa _mesa_glsl_error, ast_node::set_location
	*/
	struct YYLTYPE get_location(void) const
	{
		struct YYLTYPE locp;

		locp.source_file = this->location.source_file;
		locp.first_line = this->location.line;
		locp.first_column = this->location.column;
		locp.last_line = locp.first_line;
		locp.last_column = locp.first_column;

		return locp;
	}

	/**
	* Set the source location of an AST node from a parser location
	*
	* \sa ast_node::get_location
	*/
	void set_location(const struct YYLTYPE &locp)
	{
		this->location.source_file = locp.source_file;
		this->location.line = locp.first_line;
		this->location.column = locp.first_column;
	}

	/**
	* Print attributes attached to a node.
	*/
	void print_attributes() const;

	/**
	* Source location of the AST node.
	*/
	struct
	{
		const char* source_file;    /**< Source file. */
		unsigned line;      /**< Line number within the source string. */
		unsigned column;    /**< Column in the line. */
	} location;

	SSourceLocation GetSourceLocation(const _mesa_glsl_parse_state* State) const
	{
		SSourceLocation Location;
		Location.SourceFile = location.source_file ? location.source_file : (State->current_source_file ? State->current_source_file : State->base_source_file);
		Location.Line = location.line;
		Location.Column = location.column;
		return Location;
	}
	/**
	* List of attributes for the node
	*/
	exec_list attributes;

	exec_node link;

protected:
	/**
	* The only constructor is protected so that only derived class objects can
	* be created.
	*/
	ast_node(void);
};


/**
* Operators for AST expression nodes.
*/
enum ast_operators
{
	ast_assign,
	ast_plus,        /**< Unary + operator. */
	ast_neg,
	ast_add,
	ast_sub,
	ast_mul,
	ast_div,
	ast_mod,
	ast_lshift,
	ast_rshift,
	ast_less,
	ast_greater,
	ast_lequal,
	ast_gequal,
	ast_equal,
	ast_nequal,
	ast_bit_and,
	ast_bit_xor,
	ast_bit_or,
	ast_bit_not,
	ast_logic_and,
	ast_logic_xor,
	ast_logic_or,
	ast_logic_not,

	ast_mul_assign,
	ast_div_assign,
	ast_mod_assign,
	ast_add_assign,
	ast_sub_assign,
	ast_ls_assign,
	ast_rs_assign,
	ast_and_assign,
	ast_xor_assign,
	ast_or_assign,

	ast_conditional,

	ast_pre_inc,
	ast_pre_dec,
	ast_post_inc,
	ast_post_dec,
	ast_field_selection,
	ast_array_index,

	ast_function_call,
	ast_initializer_list,

	ast_identifier,
	ast_int_constant,
	ast_uint_constant,
	ast_float_constant,
	ast_bool_constant,

	ast_sequence,

	ast_type_cast,
};

/**
* Representation of any sort of expression.
*/
class ast_expression : public ast_node
{
public:
	ast_expression(int oper, ast_expression *,
		ast_expression *, ast_expression *);

	ast_expression(const char *identifier) :
		oper(ast_identifier)
	{
		subexpressions[0] = NULL;
		subexpressions[1] = NULL;
		subexpressions[2] = NULL;
		primary_expression.identifier = identifier;
		this->non_lvalue_description = NULL;
	}

	static const char *operator_string(enum ast_operators op);

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	/**
	* Initializers are provided with the type they are initializing. Initializers
	* that are simple expressions don't require that the type be known.
	*/
	virtual ir_rvalue *initializer_hir(exec_list *instructions, _mesa_glsl_parse_state *state, const glsl_type *initializer_type);

	virtual void print(void) const;

	enum ast_operators oper;

	ast_expression *subexpressions[3];

	union
	{
		const char *identifier;
		class ast_type_specifier *type_specifier;
		int int_constant;
		float float_constant;
		unsigned uint_constant;
		int bool_constant;
	} primary_expression;


	/**
	* List of expressions for an \c ast_sequence or parameters for an
	* \c ast_function_call
	*/
	exec_list expressions;

	/**
	* For things that can't be l-values, this describes what it is.
	*
	* This text is used by the code that generates IR for assignments to
	* detect and emit useful messages for assignments to some things that
	* can't be l-values.  For example, pre- or post-incerement expressions.
	*
	* \note
	* This pointer may be \c NULL.
	*/
	const char *non_lvalue_description;
};

class ast_expression_bin : public ast_expression
{
public:
	ast_expression_bin(int oper, ast_expression *, ast_expression *);

	virtual void print(void) const;
};

/**
* Subclass of expressions for function calls
*/
class ast_function_expression : public ast_expression
{
public:
	ast_function_expression(ast_expression *callee)
		: ast_expression(ast_function_call, callee,
		NULL, NULL),
		cons(false)
	{
		/* empty */
	}

	ast_function_expression(class ast_type_specifier *type)
		: ast_expression(ast_function_call, (ast_expression *)type,
		NULL, NULL),
		cons(true)
	{
		/* empty */
	}

	bool is_constructor() const
	{
		return cons;
	}

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

private:
	/**
	* Is this function call actually a constructor?
	*/
	bool cons;
};


/**
* Subclass of expressions for initialization lists.
*/
class ast_initializer_list_expression : public ast_expression
{
public:
	ast_initializer_list_expression()
		: ast_expression(ast_initializer_list, NULL, NULL, NULL)
	{
	}

	virtual ir_rvalue *hir(exec_list *instructions, _mesa_glsl_parse_state *state);

	virtual ir_rvalue *initializer_hir(exec_list *instructions, _mesa_glsl_parse_state *state,
		const glsl_type *initializer_type);
};


/**
* Number of possible operators for an ast_expression
*
* This is done as a define instead of as an additional value in the enum so
* that the compiler won't generate spurious messages like "warning:
* enumeration value ‘ast_num_operators’ not handled in switch"
*/
#define AST_NUM_OPERATORS (ast_type_cast + 1)


class ast_compound_statement : public ast_node
{
public:
	ast_compound_statement(int new_scope, ast_node *statements);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	int new_scope;
	exec_list statements;
};

class ast_declaration : public ast_node
{
public:
	ast_declaration(const char *identifier, int is_array, ast_expression *array_size,
		ast_expression *initializer);
	virtual void print(void) const;

	const char *identifier;
	const char *semantic;

	int is_array;
	int is_unsized_array;
	ast_expression *array_size;

	ast_expression *initializer;
};


enum
{
	ast_precision_none = 0, /**< Absence of precision qualifier. */
	ast_precision_high,
	ast_precision_medium,
	ast_precision_low
};

struct ast_type_qualifier
{
	union
	{
		struct
		{
			unsigned is_static : 1;
			unsigned invariant : 1;
			unsigned constant : 1;
			unsigned attribute : 1;
			unsigned varying : 1;
			unsigned in : 1;
			unsigned out : 1;
			unsigned centroid : 1;
			unsigned uniform : 1;
			unsigned smooth : 1;
			unsigned flat : 1;
			unsigned noperspective : 1;
			unsigned row_major : 1;
			unsigned column_major : 1;
			unsigned coherent : 1;
			unsigned shared : 1;

			/** \name Layout qualifiers for GL_ARB_fragment_coord_conventions */
			/*@{*/
			unsigned origin_upper_left : 1;
			unsigned pixel_center_integer : 1;
			/*@}*/

			/**
			* Flag set if GL_ARB_explicit_attrib_location "location" layout
			* qualifier is used.
			*/
			unsigned explicit_location : 1;

			/** \name Layout qualifiers for GL_AMD_conservative_depth */
			/** \{ */
			unsigned depth_any : 1;
			unsigned depth_greater : 1;
			unsigned depth_less : 1;
			unsigned depth_unchanged : 1;
			/** \} */

			unsigned gs_point : 1;
			unsigned gs_line : 1;
			unsigned gs_lineadj : 1;
			unsigned gs_triangle : 1;
			unsigned gs_triangleadj : 1;
		}
		/** \brief Set of flags, accessed by name. */
		q;

		/** \brief Set of flags, accessed as a bitmask. */
		unsigned i;
	} flags;

	/**
	* Location specified via GL_ARB_explicit_attrib_location layout
	*
	* \note
	* This field is only valid if \c explicit_location is set.
	*/
	int location;

	/**
	* Return true if and only if an interpolation qualifier is present.
	*/
	bool has_interpolation() const;

	/**
	* \brief Return string representation of interpolation qualifier.
	*
	* If an interpolation qualifier is present, then return that qualifier's
	* string representation. Otherwise, return null. For example, if the
	* noperspective bit is set, then this returns "noperspective".
	*
	* If multiple interpolation qualifiers are somehow present, then the
	* returned string is undefined but not null.
	*/
	const char *interpolation_string() const;
};

class ast_struct_specifier : public ast_node
{
public:
	ast_struct_specifier(const char *identifier, ast_node *declarator_list);

	ast_struct_specifier(const char *identifier, const char *parent, ast_node *declarator_list);

	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	const char *name;
	const char *parent_name;
	exec_list declarations;
};

class ast_cbuffer_declaration : public ast_node
{
public:
	ast_cbuffer_declaration(const char *identifier, ast_node *declarator_list);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	const char *name;
	exec_list declarations;
};

class ast_type_specifier : public ast_node
{
public:
	/** Construct a type specifier from a type name */
	ast_type_specifier(const char *name)
		: type_name(name), inner_type(NULL), structure(NULL), texture_ms_num_samples(1), patch_size(0),
		is_array(false), is_unsized_array(0), array_size(NULL),
		is_precision_statement(false), precision(ast_precision_none)
	{
		/* empty */
	}


	/** Construct a type specifier from a type name with an internal sampler type. */
	ast_type_specifier(const char *name, const char *inner_type_name)
		: type_name(name), inner_type(inner_type_name), structure(NULL), texture_ms_num_samples(1), patch_size(0),
		is_array(false), is_unsized_array(0), array_size(NULL),
		is_precision_statement(false), precision(ast_precision_none)
	{
		/* empty */
	}

	/** Construct a type specifier from a type name with an internal sampler type. */
	ast_type_specifier(const char *name, ast_struct_specifier* inner_type)
		: type_name(name), inner_type(nullptr), structure(NULL), InnerStructure(inner_type), texture_ms_num_samples(1), patch_size(0),
		is_array(false), is_unsized_array(0), array_size(NULL),
		is_precision_statement(false), precision(ast_precision_none)
	{
		/* empty */
	}

	/** Construct a type specifier from a structure definition */
	ast_type_specifier(ast_struct_specifier *s)
		: type_name(s->name), inner_type(NULL), structure(s), texture_ms_num_samples(1), patch_size(0),
		is_array(false), is_unsized_array(0), array_size(NULL),
		is_precision_statement(false), precision(ast_precision_none)
	{
		/* empty */
	}

	const struct glsl_type *glsl_type(const char **name,
	struct _mesa_glsl_parse_state *state)
		const;

	virtual void print(void) const;

	ir_rvalue *hir(exec_list *, struct _mesa_glsl_parse_state *);

	const char *type_name;
	const char *inner_type;
	ast_struct_specifier *structure;

	// For [RW]StructuredBuffer<>
	ast_struct_specifier* InnerStructure = nullptr;

	int texture_ms_num_samples;

	int patch_size;

	int is_array;
	int is_unsized_array;
	ast_expression *array_size;

	bool is_precision_statement;
	unsigned precision : 2;
};


class ast_fully_specified_type : public ast_node
{
public:
	virtual void print(void) const;
	bool has_qualifiers() const;

	ast_type_qualifier qualifier;
	ast_type_specifier *specifier;
};


class ast_declarator_list : public ast_node
{
public:
	ast_declarator_list(ast_fully_specified_type *);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	ast_fully_specified_type *type;
	exec_list declarations;

	/**
	* Special flag for vertex shader "invariant" declarations.
	*
	* Vertex shaders can contain "invariant" variable redeclarations that do
	* not include a type.  For example, "invariant gl_Position;".  This flag
	* is used to note these cases when no type is specified.
	*/
	int invariant;
};


class ast_parameter_declarator : public ast_node
{
public:
	ast_parameter_declarator()
	{
		this->identifier = NULL;
		this->semantic = NULL;
		this->is_array = false;
		this->array_size = 0;
		this->default_value = 0;
	}

	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	ast_fully_specified_type *type;
	const char *identifier;
	const char *semantic;
	int is_array;
	ast_expression *array_size;
	ast_expression *default_value;

	static void parameters_to_hir(exec_list *ast_parameters,
		bool formal, exec_list *ir_parameters, unsigned *num_params,
		exec_list *ir_defaults, unsigned *num_defaults,
	struct _mesa_glsl_parse_state *state);

private:
	/** Is this parameter declaration part of a formal parameter list? */
	bool formal_parameter;

	/**
	* Is this parameter 'void' type?
	*
	* This field is set by \c ::hir.
	*/
	bool is_void;
};


class ast_function : public ast_node
{
public:
	ast_function(void);

	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	ast_fully_specified_type *return_type;
	const char *identifier;
	const char *return_semantic;

	exec_list parameters;

private:
	/**
	* Is this prototype part of the function definition?
	*
	* Used by ast_function_definition::hir to process the parameters, etc.
	* of the function.
	*
	* \sa ::hir
	*/
	bool is_definition;

	/**
	* Function signature corresponding to this function prototype instance
	*
	* Used by ast_function_definition::hir to process the parameters, etc.
	* of the function.
	*
	* \sa ::hir
	*/
	class ir_function_signature *signature;

	friend class ast_function_definition;
};


class ast_expression_statement : public ast_node
{
public:
	ast_expression_statement(ast_expression *);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	ast_expression *expression;
};


class ast_case_label : public ast_node
{
public:
	ast_case_label(ast_expression *test_value);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	/**
	* An test value of NULL means 'default'.
	*/
	ast_expression *test_value;
};


class ast_case_label_list : public ast_node
{
public:
	ast_case_label_list(void);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	/**
	* A list of case labels.
	*/
	exec_list labels;
};


class ast_case_statement : public ast_node
{
public:
	ast_case_statement(ast_case_label_list *labels);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	ast_case_label_list *labels;

	/**
	* A list of statements.
	*/
	exec_list stmts;
};


class ast_case_statement_list : public ast_node
{
public:
	ast_case_statement_list(void);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	/**
	* A list of cases.
	*/
	exec_list cases;
};


class ast_switch_body : public ast_node
{
public:
	ast_switch_body(ast_case_statement_list *stmts);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	ast_case_statement_list *stmts;
};


class ast_selection_statement : public ast_node
{
public:
	ast_selection_statement(ast_expression *condition,
		ast_node *then_statement,
		ast_node *else_statement);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	ast_expression *condition;
	ast_node *then_statement;
	ast_node *else_statement;
};


class ast_switch_statement : public ast_node
{
public:
	ast_switch_statement(ast_expression *test_expression,
		ast_node *body);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	ast_expression *test_expression;
	ast_node *body;

protected:
	void test_to_hir(exec_list *, struct _mesa_glsl_parse_state *);
};

class ast_iteration_statement : public ast_node
{
public:
	ast_iteration_statement(int mode, ast_node *init, ast_node *condition,
		ast_expression *rest_expression, ast_node *body);

	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *, struct _mesa_glsl_parse_state *);

	enum ast_iteration_modes
	{
		ast_for,
		ast_while,
		ast_do_while
	} mode;


	ast_node *init_statement;
	ast_node *condition;
	ast_expression *rest_expression;

	ast_node *body;

private:
	/**
	* Generate IR from the condition of a loop
	*
	* This is factored out of ::hir because some loops have the condition
	* test at the top (for and while), and others have it at the end (do-while).
	*/
	void condition_to_hir(class ir_loop *, struct _mesa_glsl_parse_state *);
};


class ast_jump_statement : public ast_node
{
public:
	ast_jump_statement(int mode, ast_expression *return_value);
	virtual void print(void) const;

	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);

	enum ast_jump_modes
	{
		ast_continue,
		ast_break,
		ast_return,
		ast_discard
	} mode;

	ast_expression *opt_return_value;
};

class ast_attribute;
class ast_attribute_arg;
class ast_function_definition : public ast_node
{
public:
	virtual void print(void) const;


	virtual ir_rvalue *hir(exec_list *instructions,
	struct _mesa_glsl_parse_state *state);



	ast_function *prototype;
	ast_compound_statement *body;

private:
	template<typename Type>
	bool convert_attribute_args(ast_attribute * attrib, Type* results, const int expected_arg_count,
		const char* expected_type, bool (glsl_type::*type_check)(void) const, struct _mesa_glsl_parse_state * state) const;

	template<typename T>
	void convert_enum_attribute_args(ast_attribute * attrib, T& result, const char** strings, const T* values, const size_t num_values, struct _mesa_glsl_parse_state * state) const;


};

/**
* Attribute argument class holding holding arguments to attributes
*/
class ast_attribute_arg : public ast_node
{
public:

	ast_attribute_arg(const char *arg) : is_string(true)
	{
		argument.string_argument = arg;
	}

	ast_attribute_arg(ast_expression *exp) : is_string(false)
	{
		argument.exp_argument = exp;
	}

	virtual void print(void) const;

	bool is_string;
	union
	{
		const char* string_argument;
		ast_expression* exp_argument;
	} argument;
};

/**
* Attribute class holding attribute data that can be tagged to statements
*/
class ast_attribute : public ast_node
{
public:

	ast_attribute(const char* name) : attribute_name(name)
	{
	}


	virtual void print(void) const;

	const char* attribute_name;

	/* Allow for a list of attributes*/
	exec_list arguments;
};
/*@}*/

extern void _mesa_ast_to_hir(exec_list *instructions, struct _mesa_glsl_parse_state *state);

extern void _mesa_ast_print(struct _mesa_glsl_parse_state *state);

extern ir_rvalue* _mesa_ast_field_selection_to_hir(const ast_expression *expr,
					exec_list *instructions, struct _mesa_glsl_parse_state *state);

void emit_function(_mesa_glsl_parse_state *state, ir_function *f);

ir_rvalue* convert_component(ir_rvalue *src, const glsl_type *desired_type);

#endif /* AST_H */