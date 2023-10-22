#include "ShaderDefines.hpp"

#include "Core\Logger.hpp"

#if 0
I32 op_logor(I32 a, I32 b) { return a || b; }
I32 op_logand(I32 a, I32 b) { return a && b; }
I32 op_or(I32 a, I32 b) { return a | b; }
I32 op_xor(I32 a, I32 b) { return a ^ b; }
I32 op_and(I32 a, I32 b) { return a & b; }
I32 op_eq(I32 a, I32 b) { return a == b; }
I32 op_ne(I32 a, I32 b) { return a != b; }
I32 op_ge(I32 a, I32 b) { return a >= b; }
I32 op_le(I32 a, I32 b) { return a <= b; }
I32 op_gt(I32 a, I32 b) { return a > b; }
I32 op_lt(I32 a, I32 b) { return a < b; }
I32 op_shl(I32 a, I32 b) { return a << b; }
I32 op_shr(I32 a, I32 b) { return a >> b; }
I32 op_add(I32 a, I32 b) { return a + b; }
I32 op_sub(I32 a, I32 b) { return a - b; }
I32 op_mul(I32 a, I32 b) { return a * b; }
I32 op_div(I32 a, I32 b) { return a == I32_MIN && b == -1 ? 0 : a / b; }
I32 op_mod(I32 a, I32 b) { return a == I32_MIN && b == -1 ? 0 : a % b; }
I32 op_pos(I32 a) { return a; }
I32 op_neg(I32 a) { return -a; }
I32 op_cmpl(I32 a) { return ~a; }
I32 op_not(I32 a) { return !a; }

struct BinaryOp
{
	I32 token, precedence, (*op)(I32, I32);
} binaryOps[]{
	{ FIXED_ATOM_Or, LOGOR, op_logor },
	{ FIXED_ATOM_And, LOGAND, op_logand },
	{ '|', OR, op_or },
	{ '^', XOR, op_xor },
	{ '&', AND, op_and },
	{ FIXED_ATOM_EQ, EQUAL, op_eq },
	{ FIXED_ATOM_NE, EQUAL, op_ne },
	{ '>', RELATION, op_gt },
	{ FIXED_ATOM_GE, RELATION, op_ge },
	{ '<', RELATION, op_lt },
	{ FIXED_ATOM_LE, RELATION, op_le },
	{ FIXED_ATOM_Left, SHIFT, op_shl },
	{ FIXED_ATOM_Right, SHIFT, op_shr },
	{ '+', ADD, op_add },
	{ '-', ADD, op_sub },
	{ '*', MUL, op_mul },
	{ '/', MUL, op_div },
	{ '%', MUL, op_mod },
};

struct UnaryOp
{
	I32 token, (*op)(I32);
} unaryOps[]{
	{ '+', op_pos },
	{ '-', op_neg },
	{ '~', op_cmpl },
	{ '!', op_not },
};

Input::Input(Input&& other) noexcept : done{ other.done }, type{ other.type }
{
	switch (type)
	{
	case INPUT_TYPE_MACRO: { macro = Move(other.macro); } break;
	case INPUT_TYPE_MARKER: { } break;
	case INPUT_TYPE_ZERO: { } break;
	case INPUT_TYPE_TOKEN: { token = Move(other.token); } break;
	case INPUT_TYPE_UNGOT_TOKEN: { } break;
	case INPUT_TYPE_STRING: { } break;
	}
}

I32 TokenStream::Token::Get(ParseToken& parseToken)
{
	parseToken.Clear();
	parseToken.space = space;
	parseToken.i64val = i64val;
	parseToken.name = name;

	return atom;
}

I32 ParseContext::NextToken(StateType& state)
{
	do
	{
		scanContext.parserToken = state;
		ParseToken parseToken;

		I32 token = Tokenize(parseToken);

		if (token == -1) { return 0; }

		scanContext.tokenText = parseToken.name;
		scanContext.location = parseToken.location;
		scanContext.parserToken.lex.location = scanContext.location;
		switch (token)
		{
		case ';': { scanContext.afterType = false; scanContext.afterBuffer = false;	} return TOKEN_TYPE_SEMICOLON;
		case ',': { scanContext.afterType = false; } return TOKEN_TYPE_COMMA;
		case ':': return TOKEN_TYPE_COLON;
		case '=': { scanContext.afterType = false; } return TOKEN_TYPE_EQUAL;
		case '(': { scanContext.afterType = false; } return TOKEN_TYPE_LEFT_PAREN;
		case ')': { scanContext.afterType = false; } return TOKEN_TYPE_RIGHT_PAREN;
		case '.': { scanContext.field = true; } return TOKEN_TYPE_DOT;
		case '!': return TOKEN_TYPE_BANG;
		case '-': return TOKEN_TYPE_DASH;
		case '~': return TOKEN_TYPE_TILDE;
		case '+': return TOKEN_TYPE_PLUS;
		case '*': return TOKEN_TYPE_STAR;
		case '/': return TOKEN_TYPE_SLASH;
		case '%': return TOKEN_TYPE_PERCENT;
		case '<': return TOKEN_TYPE_LEFT_ANGLE;
		case '>': return TOKEN_TYPE_RIGHT_ANGLE;
		case '|': return TOKEN_TYPE_VERTICAL_BAR;
		case '^': return TOKEN_TYPE_CARET;
		case '&': return TOKEN_TYPE_AMPERSAND;
		case '?': return TOKEN_TYPE_QUESTION;
		case '[': return TOKEN_TYPE_LEFT_BRACKET;
		case ']': return TOKEN_TYPE_RIGHT_BRACKET;
		case '{': { scanContext.afterStruct = false; scanContext.afterBuffer = false; } return TOKEN_TYPE_LEFT_BRACE;
		case '}': return TOKEN_TYPE_RIGHT_BRACE;
		case '\\': { Logger::Error("Illegal Use Of An Escape Character! Found On {}", scanContext.location); } break;
			
		case FIXED_ATOM_AddAssign: return TOKEN_TYPE_ADD_ASSIGN;
		case FIXED_ATOM_SubAssign: return TOKEN_TYPE_SUB_ASSIGN;
		case FIXED_ATOM_MulAssign: return TOKEN_TYPE_MUL_ASSIGN;
		case FIXED_ATOM_DivAssign: return TOKEN_TYPE_DIV_ASSIGN;
		case FIXED_ATOM_ModAssign: return TOKEN_TYPE_MOD_ASSIGN;

		case FIXED_ATOM_Right: return TOKEN_TYPE_RIGHT_OP;
		case FIXED_ATOM_Left: return TOKEN_TYPE_LEFT_OP;

		case FIXED_ATOM_RightAssign: return TOKEN_TYPE_RIGHT_ASSIGN;
		case FIXED_ATOM_LeftAssign: return TOKEN_TYPE_LEFT_ASSIGN;
		case FIXED_ATOM_AndAssign: return TOKEN_TYPE_AND_ASSIGN;
		case FIXED_ATOM_OrAssign: return TOKEN_TYPE_OR_ASSIGN;
		case FIXED_ATOM_XorAssign: return TOKEN_TYPE_XOR_ASSIGN;

		case FIXED_ATOM_And: return TOKEN_TYPE_AND_OP;
		case FIXED_ATOM_Or: return TOKEN_TYPE_OR_OP;
		case FIXED_ATOM_Xor: return TOKEN_TYPE_XOR_OP;

		case FIXED_ATOM_EQ: return TOKEN_TYPE_EQ_OP;
		case FIXED_ATOM_GE: return TOKEN_TYPE_GE_OP;
		case FIXED_ATOM_NE: return TOKEN_TYPE_NE_OP;
		case FIXED_ATOM_LE: return TOKEN_TYPE_LE_OP;

		case FIXED_ATOM_Decrement: return TOKEN_TYPE_DEC_OP;
		case FIXED_ATOM_Increment: return TOKEN_TYPE_INC_OP;

		case FIXED_ATOM_ColonColon: { Logger::Error("'::' Is Not Supported! Found On {}", scanContext.location); } break;

		case FIXED_ATOM_ConstString: { scanContext.parserToken.lex.string = scanContext.tokenText; } return TOKEN_TYPE_STRING_LITERAL;
		case FIXED_ATOM_ConstInt: { scanContext.parserToken.lex.i = parseToken.ival; } return TOKEN_TYPE_INTCONSTANT;
		case FIXED_ATOM_ConstUint: { scanContext.parserToken.lex.i = parseToken.ival; } return TOKEN_TYPE_UINTCONSTANT;
		case FIXED_ATOM_ConstFloat: { scanContext.parserToken.lex.d = parseToken.dval; } return TOKEN_TYPE_FLOATCONSTANT;
		case FIXED_ATOM_ConstInt16: { scanContext.parserToken.lex.i = parseToken.ival; } return TOKEN_TYPE_INT16CONSTANT;
		case FIXED_ATOM_ConstUint16: { scanContext.parserToken.lex.i = parseToken.ival; } return TOKEN_TYPE_UINT16CONSTANT;
		case FIXED_ATOM_ConstInt64: { scanContext.parserToken.lex.i64 = parseToken.i64val; } return TOKEN_TYPE_INT64CONSTANT;
		case FIXED_ATOM_ConstUint64: { scanContext.parserToken.lex.i64 = parseToken.i64val; } return TOKEN_TYPE_UINT64CONSTANT;
		case FIXED_ATOM_ConstDouble: { scanContext.parserToken.lex.d = parseToken.dval; } return TOKEN_TYPE_DOUBLECONSTANT;
		case FIXED_ATOM_ConstFloat16: { scanContext.parserToken.lex.d = parseToken.dval; } return TOKEN_TYPE_FLOAT16CONSTANT;
		case FIXED_ATOM_Identifier: { scanContext.field = false; } return TokenizeIdentifier();

		case -1: return 0;

		default: {
			C8 buf[2];
			buf[0] = (C8)token;
			buf[1] = 0;
			Logger::Error("Unexpected Token! Found On {}", scanContext.location);
		} break;
		}
	} while (true);
}

I32 ParseContext::TokenizeIdentifier()
{
	if (scanContext.reservedSet->find(tokenText) != ReservedSet->end())
		return reservedWord();

	auto it = KeywordMap->find(tokenText);
	if (it == KeywordMap->end())
	{
		// Should have an identifier of some sort
		return identifierOrType();
	}
	scanContext.keyword = it->second;

	switch (scanContext.keyword)
	{
	case TOKEN_TYPE_CONST:
	case TOKEN_TYPE_UNIFORM:
	case TOKEN_TYPE_TILEIMAGEEXT:
	case TOKEN_TYPE_IN:
	case TOKEN_TYPE_OUT:
	case TOKEN_TYPE_INOUT:
	case TOKEN_TYPE_BREAK:
	case TOKEN_TYPE_CONTINUE:
	case TOKEN_TYPE_DO:
	case TOKEN_TYPE_FOR:
	case TOKEN_TYPE_WHILE:
	case TOKEN_TYPE_IF:
	case TOKEN_TYPE_ELSE:
	case TOKEN_TYPE_DISCARD:
	case TOKEN_TYPE_RETURN:
	case TOKEN_TYPE_CASE:
		return scanContext.keyword;

	case TOKEN_TYPE_TERMINATE_INVOCATION:
		if (!parseContext.extensionTurnedOn(E_GL_EXT_terminate_invocation))
			return identifierOrType();
		return scanContext.keyword;

	case TOKEN_TYPE_TERMINATE_RAY:
	case TOKEN_TYPE_IGNORE_INTERSECTION:
		if (!parseContext.extensionTurnedOn(E_GL_EXT_ray_tracing))
			return identifierOrType();
		return scanContext.keyword;

	case TOKEN_TYPE_BUFFER:
		scanContext.afterBuffer = true;
		if ((parseContext.isEsProfile() && parseContext.version < 310) ||
			(!parseContext.isEsProfile() && (parseContext.version < 430 &&
				!parseContext.extensionTurnedOn(E_GL_ARB_shader_storage_buffer_object))))
			return identifierOrType();
		return scanContext.keyword;

	case TOKEN_TYPE_STRUCT:
		scanContext.afterStruct = true;
		return scanContext.keyword;

	case TOKEN_TYPE_SWITCH:
	case TOKEN_TYPE_DEFAULT:
		if ((parseContext.isEsProfile() && parseContext.version < 300) ||
			(!parseContext.isEsProfile() && parseContext.version < 130))
			reservedWord();
		return scanContext.keyword;

	case TOKEN_TYPE_VOID:
	case TOKEN_TYPE_BOOL:
	case TOKEN_TYPE_FLOAT:
	case TOKEN_TYPE_INT:
	case TOKEN_TYPE_BVEC2:
	case TOKEN_TYPE_BVEC3:
	case TOKEN_TYPE_BVEC4:
	case TOKEN_TYPE_VEC2:
	case TOKEN_TYPE_VEC3:
	case TOKEN_TYPE_VEC4:
	case TOKEN_TYPE_IVEC2:
	case TOKEN_TYPE_IVEC3:
	case TOKEN_TYPE_IVEC4:
	case TOKEN_TYPE_MAT2:
	case TOKEN_TYPE_MAT3:
	case TOKEN_TYPE_MAT4:
	case TOKEN_TYPE_SAMPLER2D:
	case TOKEN_TYPE_SAMPLERCUBE:
		scanContext.afterType = true;
		return scanContext.keyword;

	case TOKEN_TYPE_BOOLCONSTANT:
		if (strcmp("true", tokenText) == 0)
			parserToken->sType.lex.b = true;
		else
			parserToken->sType.lex.b = false;
		return scanContext.keyword;

	case TOKEN_TYPE_SMOOTH:
		if ((parseContext.isEsProfile() && parseContext.version < 300) ||
			(!parseContext.isEsProfile() && parseContext.version < 130))
			return identifierOrType();
		return scanContext.keyword;
	case TOKEN_TYPE_FLAT:
		if (parseContext.isEsProfile() && parseContext.version < 300)
			reservedWord();
		else if (!parseContext.isEsProfile() && parseContext.version < 130)
			return identifierOrType();
		return scanContext.keyword;
	case TOKEN_TYPE_CENTROID:
		if (parseContext.version < 120)
			return identifierOrType();
		return scanContext.keyword;
	case TOKEN_TYPE_INVARIANT:
		if (!parseContext.isEsProfile() && parseContext.version < 120)
			return identifierOrType();
		return scanContext.keyword;
	case TOKEN_TYPE_PACKED:
		if ((parseContext.isEsProfile() && parseContext.version < 300) ||
			(!parseContext.isEsProfile() && parseContext.version < 140))
			return reservedWord();
		return scanContext.identifierOrType();

	case TOKEN_TYPE_RESOURCE:
	{
		bool reserved = (parseContext.isEsProfile() && parseContext.version >= 300) ||
			(!parseContext.isEsProfile() && parseContext.version >= 420);
		return identifierOrReserved(reserved);
	}
	case TOKEN_TYPE_SUPERP:
	{
		bool reserved = parseContext.isEsProfile() || parseContext.version >= 130;
		return identifierOrReserved(reserved);
	}

	case TOKEN_TYPE_NOPERSPECTIVE:
		if (parseContext.extensionTurnedOn(E_GL_NV_shader_noperspective_interpolation))
			return scanContext.keyword;
		return es30ReservedFromGLSL(130);

	case TOKEN_TYPE_NONUNIFORM:
		if (parseContext.extensionTurnedOn(E_GL_EXT_nonuniform_qualifier))
			return scanContext.keyword;
		else
			return identifierOrType();
	case TOKEN_TYPE_ATTRIBUTE:
	case TOKEN_TYPE_VARYING:
		if (parseContext.isEsProfile() && parseContext.version >= 300)
			reservedWord();
		return scanContext.keyword;
	case TOKEN_TYPE_PAYLOADNV:
	case TOKEN_TYPE_PAYLOADINNV:
	case TOKEN_TYPE_HITATTRNV:
	case TOKEN_TYPE_CALLDATANV:
	case TOKEN_TYPE_CALLDATAINNV:
	case TOKEN_TYPE_ACCSTRUCTNV:
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_NV_ray_tracing))
			return scanContext.keyword;
		return identifierOrType();
	case TOKEN_TYPE_PAYLOADEXT:
	case TOKEN_TYPE_PAYLOADINEXT:
	case TOKEN_TYPE_HITATTREXT:
	case TOKEN_TYPE_CALLDATAEXT:
	case TOKEN_TYPE_CALLDATAINEXT:
	case TOKEN_TYPE_ACCSTRUCTEXT:
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_ray_tracing) ||
			parseContext.extensionTurnedOn(E_GL_EXT_ray_query))
			return scanContext.keyword;
		return identifierOrType();
	case TOKEN_TYPE_RAYQUERYEXT:
		if (parseContext.symbolTable.atBuiltInLevel() ||
			(!parseContext.isEsProfile() && parseContext.version >= 460
				&& parseContext.extensionTurnedOn(E_GL_EXT_ray_query)))
			return scanContext.keyword;
		return identifierOrType();
	case TOKEN_TYPE_ATOMIC_UINT:
		if ((parseContext.isEsProfile() && parseContext.version >= 310) ||
			parseContext.extensionTurnedOn(E_GL_ARB_shader_atomic_counters))
			return scanContext.keyword;
		return es30ReservedFromGLSL(420);

	case TOKEN_TYPE_COHERENT:
	case TOKEN_TYPE_DEVICECOHERENT:
	case TOKEN_TYPE_QUEUEFAMILYCOHERENT:
	case TOKEN_TYPE_WORKGROUPCOHERENT:
	case TOKEN_TYPE_SUBGROUPCOHERENT:
	case TOKEN_TYPE_SHADERCALLCOHERENT:
	case TOKEN_TYPE_NONPRIVATE:
	case TOKEN_TYPE_RESTRICT:
	case TOKEN_TYPE_READONLY:
	case TOKEN_TYPE_WRITEONLY:
		if (parseContext.isEsProfile() && parseContext.version >= 310)
			return scanContext.keyword;
		return es30ReservedFromGLSL(parseContext.extensionTurnedOn(E_GL_ARB_shader_image_load_store) ? 130 : 420);
	case TOKEN_TYPE_VOLATILE:
		if (parseContext.isEsProfile() && parseContext.version >= 310)
			return scanContext.keyword;
		if (!parseContext.symbolTable.atBuiltInLevel() && (parseContext.isEsProfile() ||
			(parseContext.version < 420 && !parseContext.extensionTurnedOn(E_GL_ARB_shader_image_load_store))))
			reservedWord();
		return scanContext.keyword;
	case TOKEN_TYPE_PATCH:
		if (parseContext.symbolTable.atBuiltInLevel() ||
			(parseContext.isEsProfile() &&
				(parseContext.version >= 320 ||
					parseContext.extensionsTurnedOn(Num_AEP_tessellation_shader, AEP_tessellation_shader))) ||
			(!parseContext.isEsProfile() && parseContext.extensionTurnedOn(E_GL_ARB_tessellation_shader)))
			return scanContext.keyword;

		return es30ReservedFromGLSL(400);

	case TOKEN_TYPE_SAMPLE:
		if ((parseContext.isEsProfile() && parseContext.version >= 320) ||
			parseContext.extensionsTurnedOn(1, &E_GL_OES_shader_multisample_interpolation))
			return scanContext.keyword;
		return es30ReservedFromGLSL(400);

	case TOKEN_TYPE_SUBROUTINE:
		return es30ReservedFromGLSL(400);

	case TOKEN_TYPE_SHARED:
		if ((parseContext.isEsProfile() && parseContext.version < 300) ||
			(!parseContext.isEsProfile() && parseContext.version < 140))
			return identifierOrType();
		return scanContext.keyword;
	case TOKEN_TYPE_LAYOUT:
	{
		const int numLayoutExts = 2;
		const char* layoutExts[numLayoutExts] = { E_GL_ARB_shading_language_420pack,
												  E_GL_ARB_explicit_attrib_location };
		if ((parseContext.isEsProfile() && parseContext.version < 300) ||
			(!parseContext.isEsProfile() && parseContext.version < 140 &&
				!parseContext.extensionsTurnedOn(numLayoutExts, layoutExts)))
			return identifierOrType();
		return scanContext.keyword;
	}

	case TOKEN_TYPE_HIGH_PRECISION:
	case TOKEN_TYPE_MEDIUM_PRECISION:
	case TOKEN_TYPE_LOW_PRECISION:
	case TOKEN_TYPE_PRECISION:
		return precisionKeyword();

	case TOKEN_TYPE_MAT2X2:
	case TOKEN_TYPE_MAT2X3:
	case TOKEN_TYPE_MAT2X4:
	case TOKEN_TYPE_MAT3X2:
	case TOKEN_TYPE_MAT3X3:
	case TOKEN_TYPE_MAT3X4:
	case TOKEN_TYPE_MAT4X2:
	case TOKEN_TYPE_MAT4X3:
	case TOKEN_TYPE_MAT4X4:
		return matNxM();

	case TOKEN_TYPE_DMAT2:
	case TOKEN_TYPE_DMAT3:
	case TOKEN_TYPE_DMAT4:
	case TOKEN_TYPE_DMAT2X2:
	case TOKEN_TYPE_DMAT2X3:
	case TOKEN_TYPE_DMAT2X4:
	case TOKEN_TYPE_DMAT3X2:
	case TOKEN_TYPE_DMAT3X3:
	case TOKEN_TYPE_DMAT3X4:
	case TOKEN_TYPE_DMAT4X2:
	case TOKEN_TYPE_DMAT4X3:
	case TOKEN_TYPE_DMAT4X4:
		return dMat();

	case TOKEN_TYPE_IMAGE1D:
	case TOKEN_TYPE_IIMAGE1D:
	case TOKEN_TYPE_UIMAGE1D:
	case TOKEN_TYPE_IMAGE1DARRAY:
	case TOKEN_TYPE_IIMAGE1DARRAY:
	case TOKEN_TYPE_UIMAGE1DARRAY:
	case TOKEN_TYPE_IMAGE2DRECT:
	case TOKEN_TYPE_IIMAGE2DRECT:
	case TOKEN_TYPE_UIMAGE2DRECT:
		scanContext.afterType = true;
		return firstGenerationImage(false);

	case TOKEN_TYPE_I64IMAGE1D:
	case TOKEN_TYPE_U64IMAGE1D:
	case TOKEN_TYPE_I64IMAGE1DARRAY:
	case TOKEN_TYPE_U64IMAGE1DARRAY:
	case TOKEN_TYPE_I64IMAGE2DRECT:
	case TOKEN_TYPE_U64IMAGE2DRECT:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_image_int64))
		{
			return firstGenerationImage(false);
		}
		return identifierOrType();

	case TOKEN_TYPE_IMAGEBUFFER:
	case TOKEN_TYPE_IIMAGEBUFFER:
	case TOKEN_TYPE_UIMAGEBUFFER:
		scanContext.afterType = true;
		if ((parseContext.isEsProfile() && parseContext.version >= 320) ||
			parseContext.extensionsTurnedOn(Num_AEP_texture_buffer, AEP_texture_buffer))
			return scanContext.keyword;
		return firstGenerationImage(false);

	case TOKEN_TYPE_I64IMAGEBUFFER:
	case TOKEN_TYPE_U64IMAGEBUFFER:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_image_int64))
		{
			if ((parseContext.isEsProfile() && parseContext.version >= 320) ||
				parseContext.extensionsTurnedOn(Num_AEP_texture_buffer, AEP_texture_buffer))
				return scanContext.keyword;
			return firstGenerationImage(false);
		}
		return identifierOrType();

	case TOKEN_TYPE_IMAGE2D:
	case TOKEN_TYPE_IIMAGE2D:
	case TOKEN_TYPE_UIMAGE2D:
	case TOKEN_TYPE_IMAGE3D:
	case TOKEN_TYPE_IIMAGE3D:
	case TOKEN_TYPE_UIMAGE3D:
	case TOKEN_TYPE_IMAGECUBE:
	case TOKEN_TYPE_IIMAGECUBE:
	case TOKEN_TYPE_UIMAGECUBE:
	case TOKEN_TYPE_IMAGE2DARRAY:
	case TOKEN_TYPE_IIMAGE2DARRAY:
	case TOKEN_TYPE_UIMAGE2DARRAY:
		scanContext.afterType = true;
		return firstGenerationImage(true);

	case TOKEN_TYPE_I64IMAGE2D:
	case TOKEN_TYPE_U64IMAGE2D:
	case TOKEN_TYPE_I64IMAGE3D:
	case TOKEN_TYPE_U64IMAGE3D:
	case TOKEN_TYPE_I64IMAGECUBE:
	case TOKEN_TYPE_U64IMAGECUBE:
	case TOKEN_TYPE_I64IMAGE2DARRAY:
	case TOKEN_TYPE_U64IMAGE2DARRAY:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_image_int64))
			return firstGenerationImage(true);
		return identifierOrType();

	case TOKEN_TYPE_IMAGECUBEARRAY:
	case TOKEN_TYPE_IIMAGECUBEARRAY:
	case TOKEN_TYPE_UIMAGECUBEARRAY:
		scanContext.afterType = true;
		if ((parseContext.isEsProfile() && parseContext.version >= 320) ||
			parseContext.extensionsTurnedOn(Num_AEP_texture_cube_map_array, AEP_texture_cube_map_array))
			return scanContext.keyword;
		return secondGenerationImage();

	case TOKEN_TYPE_I64IMAGECUBEARRAY:
	case TOKEN_TYPE_U64IMAGECUBEARRAY:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_image_int64))
		{
			if ((parseContext.isEsProfile() && parseContext.version >= 320) ||
				parseContext.extensionsTurnedOn(Num_AEP_texture_cube_map_array, AEP_texture_cube_map_array))
				return scanContext.keyword;
			return secondGenerationImage();
		}
		return identifierOrType();

	case TOKEN_TYPE_IMAGE2DMS:
	case TOKEN_TYPE_IIMAGE2DMS:
	case TOKEN_TYPE_UIMAGE2DMS:
	case TOKEN_TYPE_IMAGE2DMSARRAY:
	case TOKEN_TYPE_IIMAGE2DMSARRAY:
	case TOKEN_TYPE_UIMAGE2DMSARRAY:
		scanContext.afterType = true;
		return secondGenerationImage();

	case TOKEN_TYPE_I64IMAGE2DMS:
	case TOKEN_TYPE_U64IMAGE2DMS:
	case TOKEN_TYPE_I64IMAGE2DMSARRAY:
	case TOKEN_TYPE_U64IMAGE2DMSARRAY:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_image_int64))
		{
			return secondGenerationImage();
		}
		return identifierOrType();

	case TOKEN_TYPE_DOUBLE:
	case TOKEN_TYPE_DVEC2:
	case TOKEN_TYPE_DVEC3:
	case TOKEN_TYPE_DVEC4:
		scanContext.afterType = true;
		if (parseContext.isEsProfile() || parseContext.version < 150 ||
			(!parseContext.symbolTable.atBuiltInLevel() &&
				(parseContext.version < 400 && !parseContext.extensionTurnedOn(E_GL_ARB_gpu_shader_fp64) &&
					(parseContext.version < 410 && !parseContext.extensionTurnedOn(E_GL_ARB_vertex_attrib_64bit)))))
			reservedWord();
		return scanContext.keyword;

	case TOKEN_TYPE_INT64_T:
	case TOKEN_TYPE_UINT64_T:
	case TOKEN_TYPE_I64VEC2:
	case TOKEN_TYPE_I64VEC3:
	case TOKEN_TYPE_I64VEC4:
	case TOKEN_TYPE_U64VEC2:
	case TOKEN_TYPE_U64VEC3:
	case TOKEN_TYPE_U64VEC4:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_ARB_gpu_shader_int64) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types_int64))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_INT8_T:
	case TOKEN_TYPE_UINT8_T:
	case TOKEN_TYPE_I8VEC2:
	case TOKEN_TYPE_I8VEC3:
	case TOKEN_TYPE_I8VEC4:
	case TOKEN_TYPE_U8VEC2:
	case TOKEN_TYPE_U8VEC3:
	case TOKEN_TYPE_U8VEC4:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_8bit_storage) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types_int8))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_INT16_T:
	case TOKEN_TYPE_UINT16_T:
	case TOKEN_TYPE_I16VEC2:
	case TOKEN_TYPE_I16VEC3:
	case TOKEN_TYPE_I16VEC4:
	case TOKEN_TYPE_U16VEC2:
	case TOKEN_TYPE_U16VEC3:
	case TOKEN_TYPE_U16VEC4:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_AMD_gpu_shader_int16) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_16bit_storage) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types_int16))
			return scanContext.keyword;
		return identifierOrType();
	case TOKEN_TYPE_INT32_T:
	case TOKEN_TYPE_UINT32_T:
	case TOKEN_TYPE_I32VEC2:
	case TOKEN_TYPE_I32VEC3:
	case TOKEN_TYPE_I32VEC4:
	case TOKEN_TYPE_U32VEC2:
	case TOKEN_TYPE_U32VEC3:
	case TOKEN_TYPE_U32VEC4:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types_int32))
			return scanContext.keyword;
		return identifierOrType();
	case TOKEN_TYPE_FLOAT32_T:
	case TOKEN_TYPE_F32VEC2:
	case TOKEN_TYPE_F32VEC3:
	case TOKEN_TYPE_F32VEC4:
	case TOKEN_TYPE_F32MAT2:
	case TOKEN_TYPE_F32MAT3:
	case TOKEN_TYPE_F32MAT4:
	case TOKEN_TYPE_F32MAT2X2:
	case TOKEN_TYPE_F32MAT2X3:
	case TOKEN_TYPE_F32MAT2X4:
	case TOKEN_TYPE_F32MAT3X2:
	case TOKEN_TYPE_F32MAT3X3:
	case TOKEN_TYPE_F32MAT3X4:
	case TOKEN_TYPE_F32MAT4X2:
	case TOKEN_TYPE_F32MAT4X3:
	case TOKEN_TYPE_F32MAT4X4:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types_float32))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_FLOAT64_T:
	case TOKEN_TYPE_F64VEC2:
	case TOKEN_TYPE_F64VEC3:
	case TOKEN_TYPE_F64VEC4:
	case TOKEN_TYPE_F64MAT2:
	case TOKEN_TYPE_F64MAT3:
	case TOKEN_TYPE_F64MAT4:
	case TOKEN_TYPE_F64MAT2X2:
	case TOKEN_TYPE_F64MAT2X3:
	case TOKEN_TYPE_F64MAT2X4:
	case TOKEN_TYPE_F64MAT3X2:
	case TOKEN_TYPE_F64MAT3X3:
	case TOKEN_TYPE_F64MAT3X4:
	case TOKEN_TYPE_F64MAT4X2:
	case TOKEN_TYPE_F64MAT4X3:
	case TOKEN_TYPE_F64MAT4X4:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types_float64))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_FLOAT16_T:
	case TOKEN_TYPE_F16VEC2:
	case TOKEN_TYPE_F16VEC3:
	case TOKEN_TYPE_F16VEC4:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_AMD_gpu_shader_half_float) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_16bit_storage) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types_float16))
			return scanContext.keyword;

		return identifierOrType();

	case TOKEN_TYPE_F16MAT2:
	case TOKEN_TYPE_F16MAT3:
	case TOKEN_TYPE_F16MAT4:
	case TOKEN_TYPE_F16MAT2X2:
	case TOKEN_TYPE_F16MAT2X3:
	case TOKEN_TYPE_F16MAT2X4:
	case TOKEN_TYPE_F16MAT3X2:
	case TOKEN_TYPE_F16MAT3X3:
	case TOKEN_TYPE_F16MAT3X4:
	case TOKEN_TYPE_F16MAT4X2:
	case TOKEN_TYPE_F16MAT4X3:
	case TOKEN_TYPE_F16MAT4X4:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_AMD_gpu_shader_half_float) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types) ||
			parseContext.extensionTurnedOn(E_GL_EXT_shader_explicit_arithmetic_types_float16))
			return scanContext.keyword;

		return identifierOrType();

	case TOKEN_TYPE_SAMPLERCUBEARRAY:
	case TOKEN_TYPE_SAMPLERCUBEARRAYSHADOW:
	case TOKEN_TYPE_ISAMPLERCUBEARRAY:
	case TOKEN_TYPE_USAMPLERCUBEARRAY:
		scanContext.afterType = true;
		if ((parseContext.isEsProfile() && parseContext.version >= 320) ||
			parseContext.extensionsTurnedOn(Num_AEP_texture_cube_map_array, AEP_texture_cube_map_array))
			return scanContext.keyword;
		if (parseContext.isEsProfile() || (parseContext.version < 400 && !parseContext.extensionTurnedOn(E_GL_ARB_texture_cube_map_array)))
			reservedWord();
		return scanContext.keyword;

	case TOKEN_TYPE_TEXTURECUBEARRAY:
	case TOKEN_TYPE_ITEXTURECUBEARRAY:
	case TOKEN_TYPE_UTEXTURECUBEARRAY:
		if (parseContext.spvVersion.vulkan > 0)
			return scanContext.keyword;
		else
			return identifierOrType();

	case TOKEN_TYPE_UINT:
	case TOKEN_TYPE_UVEC2:
	case TOKEN_TYPE_UVEC3:
	case TOKEN_TYPE_UVEC4:
	case TOKEN_TYPE_SAMPLERCUBESHADOW:
	case TOKEN_TYPE_SAMPLER2DARRAY:
	case TOKEN_TYPE_SAMPLER2DARRAYSHADOW:
	case TOKEN_TYPE_ISAMPLER2D:
	case TOKEN_TYPE_ISAMPLER3D:
	case TOKEN_TYPE_ISAMPLERCUBE:
	case TOKEN_TYPE_ISAMPLER2DARRAY:
	case TOKEN_TYPE_USAMPLER2D:
	case TOKEN_TYPE_USAMPLER3D:
	case TOKEN_TYPE_USAMPLERCUBE:
	case TOKEN_TYPE_USAMPLER2DARRAY:
		scanContext.afterType = true;
		return nonreservedKeyword(300, 130);

	case TOKEN_TYPE_SAMPLER3D:
		afterType = true;
		if (parseContext.isEsProfile() && parseContext.version < 300)
		{
			if (!parseContext.extensionTurnedOn(E_GL_OES_texture_3D))
				reservedWord();
		}
		return scanContext.keyword;

	case TOKEN_TYPE_SAMPLER2DSHADOW:
		scanContext.afterType = true;
		if (parseContext.isEsProfile() && parseContext.version < 300)
		{
			if (!parseContext.extensionTurnedOn(E_GL_EXT_shadow_samplers))
				reservedWord();
		}
		return scanContext.keyword;

	case TOKEN_TYPE_TEXTURE2D:
	case TOKEN_TYPE_TEXTURECUBE:
	case TOKEN_TYPE_TEXTURE2DARRAY:
	case TOKEN_TYPE_ITEXTURE2D:
	case TOKEN_TYPE_ITEXTURE3D:
	case TOKEN_TYPE_ITEXTURECUBE:
	case TOKEN_TYPE_ITEXTURE2DARRAY:
	case TOKEN_TYPE_UTEXTURE2D:
	case TOKEN_TYPE_UTEXTURE3D:
	case TOKEN_TYPE_UTEXTURECUBE:
	case TOKEN_TYPE_UTEXTURE2DARRAY:
	case TOKEN_TYPE_TEXTURE3D:
	case TOKEN_TYPE_SAMPLER:
	case TOKEN_TYPE_SAMPLERSHADOW:
		if (parseContext.spvVersion.vulkan > 0)
			return scanContext.keyword;
		else
			return identifierOrType();

	case TOKEN_TYPE_ISAMPLER1D:
	case TOKEN_TYPE_ISAMPLER1DARRAY:
	case TOKEN_TYPE_SAMPLER1DARRAYSHADOW:
	case TOKEN_TYPE_USAMPLER1D:
	case TOKEN_TYPE_USAMPLER1DARRAY:
		scanContext.afterType = true;
		return es30ReservedFromGLSL(130);
	case TOKEN_TYPE_ISAMPLER2DRECT:
	case TOKEN_TYPE_USAMPLER2DRECT:
		scanContext.afterType = true;
		return es30ReservedFromGLSL(140);

	case TOKEN_TYPE_SAMPLERBUFFER:
		scanContext.afterType = true;
		if ((parseContext.isEsProfile() && parseContext.version >= 320) ||
			parseContext.extensionsTurnedOn(Num_AEP_texture_buffer, AEP_texture_buffer))
			return scanContext.keyword;
		return es30ReservedFromGLSL(130);

	case TOKEN_TYPE_ISAMPLERBUFFER:
	case TOKEN_TYPE_USAMPLERBUFFER:
		scanContext.afterType = true;
		if ((parseContext.isEsProfile() && parseContext.version >= 320) ||
			parseContext.extensionsTurnedOn(Num_AEP_texture_buffer, AEP_texture_buffer))
			return scanContext.keyword;
		return es30ReservedFromGLSL(140);

	case TOKEN_TYPE_SAMPLER2DMS:
	case TOKEN_TYPE_ISAMPLER2DMS:
	case TOKEN_TYPE_USAMPLER2DMS:
		scanContext.afterType = true;
		if (parseContext.isEsProfile() && parseContext.version >= 310)
			return scanContext.keyword;
		if (!parseContext.isEsProfile() && (parseContext.version > 140 ||
			(parseContext.version == 140 && parseContext.extensionsTurnedOn(1, &E_GL_ARB_texture_multisample))))
			return scanContext.keyword;
		return es30ReservedFromGLSL(150);

	case TOKEN_TYPE_SAMPLER2DMSARRAY:
	case TOKEN_TYPE_ISAMPLER2DMSARRAY:
	case TOKEN_TYPE_USAMPLER2DMSARRAY:
		scanContext.afterType = true;
		if ((parseContext.isEsProfile() && parseContext.version >= 320) ||
			parseContext.extensionsTurnedOn(1, &E_GL_OES_texture_storage_multisample_2d_array))
			return scanContext.keyword;
		if (!parseContext.isEsProfile() && (parseContext.version > 140 ||
			(parseContext.version == 140 && parseContext.extensionsTurnedOn(1, &E_GL_ARB_texture_multisample))))
			return scanContext.keyword;
		return es30ReservedFromGLSL(150);

	case TOKEN_TYPE_SAMPLER1D:
	case TOKEN_TYPE_SAMPLER1DSHADOW:
		scanContext.afterType = true;
		if (parseContext.isEsProfile())
			reservedWord();
		return scanContext.keyword;

	case TOKEN_TYPE_SAMPLER2DRECT:
	case TOKEN_TYPE_SAMPLER2DRECTSHADOW:
		scanContext.afterType = true;
		if (parseContext.isEsProfile())
			reservedWord();
		else if (parseContext.version < 140 && !parseContext.symbolTable.atBuiltInLevel() && !parseContext.extensionTurnedOn(E_GL_ARB_texture_rectangle))
		{
			if (parseContext.relaxedErrors())
				parseContext.requireExtensions(loc, 1, &E_GL_ARB_texture_rectangle, "texture-rectangle sampler keyword");
			else
				reservedWord();
		}
		return scanContext.keyword;

	case TOKEN_TYPE_SAMPLER1DARRAY:
		scanContext.afterType = true;
		if (parseContext.isEsProfile() && parseContext.version == 300)
			reservedWord();
		else if ((parseContext.isEsProfile() && parseContext.version < 300) ||
			(!parseContext.isEsProfile() && parseContext.version < 130))
			return identifierOrType();
		return scanContext.keyword;

	case TOKEN_TYPE_SAMPLEREXTERNALOES:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_OES_EGL_image_external) ||
			parseContext.extensionTurnedOn(E_GL_OES_EGL_image_external_essl3))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_SAMPLEREXTERNAL2DY2YEXT:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_YUV_target))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_ITEXTURE1DARRAY:
	case TOKEN_TYPE_UTEXTURE1D:
	case TOKEN_TYPE_ITEXTURE1D:
	case TOKEN_TYPE_UTEXTURE1DARRAY:
	case TOKEN_TYPE_TEXTUREBUFFER:
	case TOKEN_TYPE_ITEXTURE2DRECT:
	case TOKEN_TYPE_UTEXTURE2DRECT:
	case TOKEN_TYPE_ITEXTUREBUFFER:
	case TOKEN_TYPE_UTEXTUREBUFFER:
	case TOKEN_TYPE_TEXTURE2DMS:
	case TOKEN_TYPE_ITEXTURE2DMS:
	case TOKEN_TYPE_UTEXTURE2DMS:
	case TOKEN_TYPE_TEXTURE2DMSARRAY:
	case TOKEN_TYPE_ITEXTURE2DMSARRAY:
	case TOKEN_TYPE_UTEXTURE2DMSARRAY:
	case TOKEN_TYPE_TEXTURE1D:
	case TOKEN_TYPE_TEXTURE2DRECT:
	case TOKEN_TYPE_TEXTURE1DARRAY:
		if (parseContext.spvVersion.vulkan > 0)
			return scanContext.keyword;
		else
			return identifierOrType();

	case TOKEN_TYPE_SUBPASSINPUT:
	case TOKEN_TYPE_SUBPASSINPUTMS:
	case TOKEN_TYPE_ISUBPASSINPUT:
	case TOKEN_TYPE_ISUBPASSINPUTMS:
	case TOKEN_TYPE_USUBPASSINPUT:
	case TOKEN_TYPE_USUBPASSINPUTMS:
	case TOKEN_TYPE_ATTACHMENTEXT:
	case TOKEN_TYPE_IATTACHMENTEXT:
	case TOKEN_TYPE_UATTACHMENTEXT:
		if (parseContext.spvVersion.vulkan > 0)
			return scanContext.keyword;
		else
			return identifierOrType();

	case TOKEN_TYPE_F16SAMPLER1D:
	case TOKEN_TYPE_F16SAMPLER2D:
	case TOKEN_TYPE_F16SAMPLER3D:
	case TOKEN_TYPE_F16SAMPLER2DRECT:
	case TOKEN_TYPE_F16SAMPLERCUBE:
	case TOKEN_TYPE_F16SAMPLER1DARRAY:
	case TOKEN_TYPE_F16SAMPLER2DARRAY:
	case TOKEN_TYPE_F16SAMPLERCUBEARRAY:
	case TOKEN_TYPE_F16SAMPLERBUFFER:
	case TOKEN_TYPE_F16SAMPLER2DMS:
	case TOKEN_TYPE_F16SAMPLER2DMSARRAY:
	case TOKEN_TYPE_F16SAMPLER1DSHADOW:
	case TOKEN_TYPE_F16SAMPLER2DSHADOW:
	case TOKEN_TYPE_F16SAMPLER1DARRAYSHADOW:
	case TOKEN_TYPE_F16SAMPLER2DARRAYSHADOW:
	case TOKEN_TYPE_F16SAMPLER2DRECTSHADOW:
	case TOKEN_TYPE_F16SAMPLERCUBESHADOW:
	case TOKEN_TYPE_F16SAMPLERCUBEARRAYSHADOW:

	case TOKEN_TYPE_F16IMAGE1D:
	case TOKEN_TYPE_F16IMAGE2D:
	case TOKEN_TYPE_F16IMAGE3D:
	case TOKEN_TYPE_F16IMAGE2DRECT:
	case TOKEN_TYPE_F16IMAGECUBE:
	case TOKEN_TYPE_F16IMAGE1DARRAY:
	case TOKEN_TYPE_F16IMAGE2DARRAY:
	case TOKEN_TYPE_F16IMAGECUBEARRAY:
	case TOKEN_TYPE_F16IMAGEBUFFER:
	case TOKEN_TYPE_F16IMAGE2DMS:
	case TOKEN_TYPE_F16IMAGE2DMSARRAY:

	case TOKEN_TYPE_F16TEXTURE1D:
	case TOKEN_TYPE_F16TEXTURE2D:
	case TOKEN_TYPE_F16TEXTURE3D:
	case TOKEN_TYPE_F16TEXTURE2DRECT:
	case TOKEN_TYPE_F16TEXTURECUBE:
	case TOKEN_TYPE_F16TEXTURE1DARRAY:
	case TOKEN_TYPE_F16TEXTURE2DARRAY:
	case TOKEN_TYPE_F16TEXTURECUBEARRAY:
	case TOKEN_TYPE_F16TEXTUREBUFFER:
	case TOKEN_TYPE_F16TEXTURE2DMS:
	case TOKEN_TYPE_F16TEXTURE2DMSARRAY:

	case TOKEN_TYPE_F16SUBPASSINPUT:
	case TOKEN_TYPE_F16SUBPASSINPUTMS:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_AMD_gpu_shader_half_float_fetch))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_EXPLICITINTERPAMD:
		if (parseContext.extensionTurnedOn(E_GL_AMD_shader_explicit_vertex_parameter))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_PERVERTEXNV:
		if ((!parseContext.isEsProfile() && parseContext.version >= 450) ||
			parseContext.extensionTurnedOn(E_GL_NV_fragment_shader_barycentric))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_PERVERTEXEXT:
		if ((!parseContext.isEsProfile() && parseContext.version >= 450) ||
			parseContext.extensionTurnedOn(E_GL_EXT_fragment_shader_barycentric))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_PRECISE:
		if ((parseContext.isEsProfile() &&
			(parseContext.version >= 320 || parseContext.extensionsTurnedOn(Num_AEP_gpu_shader5, AEP_gpu_shader5))) ||
			(!parseContext.isEsProfile() && parseContext.version >= 400))
			return scanContext.keyword;
		if (parseContext.isEsProfile() && parseContext.version == 310)
		{
			reservedWord();
			return scanContext.keyword;
		}
		return identifierOrType();

	case TOKEN_TYPE_PERPRIMITIVENV:
	case TOKEN_TYPE_PERVIEWNV:
	case TOKEN_TYPE_PERTASKNV:
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_NV_mesh_shader))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_PERPRIMITIVEEXT:
	case TOKEN_TYPE_TASKPAYLOADWORKGROUPEXT:
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_mesh_shader))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_FCOOPMATNV:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_NV_cooperative_matrix))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_UCOOPMATNV:
	case TOKEN_TYPE_ICOOPMATNV:
		scanContext.afterType = true;
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_NV_integer_cooperative_matrix))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_DEMOTE:
		if (parseContext.extensionTurnedOn(E_GL_EXT_demote_to_helper_invocation))
			return scanContext.keyword;
		else
			return identifierOrType();

	case TOKEN_TYPE_SPIRV_INSTRUCTION:
	case TOKEN_TYPE_SPIRV_EXECUTION_MODE:
	case TOKEN_TYPE_SPIRV_EXECUTION_MODE_ID:
	case TOKEN_TYPE_SPIRV_DECORATE:
	case TOKEN_TYPE_SPIRV_DECORATE_ID:
	case TOKEN_TYPE_SPIRV_DECORATE_STRING:
	case TOKEN_TYPE_SPIRV_TYPE:
	case TOKEN_TYPE_SPIRV_STORAGE_CLASS:
	case TOKEN_TYPE_SPIRV_BY_REFERENCE:
	case TOKEN_TYPE_SPIRV_LITERAL:
		if (parseContext.symbolTable.atBuiltInLevel() ||
			parseContext.extensionTurnedOn(E_GL_EXT_spirv_intrinsics))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_HITOBJECTNV:
		if (parseContext.symbolTable.atBuiltInLevel() ||
			(!parseContext.isEsProfile() && parseContext.version >= 460
				&& parseContext.extensionTurnedOn(E_GL_NV_shader_invocation_reorder)))
			return scanContext.keyword;
		return identifierOrType();

	case TOKEN_TYPE_HITOBJECTATTRNV:
		if (parseContext.symbolTable.atBuiltInLevel() ||
			(!parseContext.isEsProfile() && parseContext.version >= 460
				&& parseContext.extensionTurnedOn(E_GL_NV_shader_invocation_reorder)))
			return scanContext.keyword;
		return identifierOrType();

	default:
		parseContext.infoSink.info.message(EPrefixInternalError, "Unknown glslang keyword", loc);
		return 0;
	}
}

I32 ParseContext::Tokenize(ParseToken& parseToken)
{
	while (true)
	{
		I32 token = ScanToken(parseToken);

		token = TokenPaste(token, parseToken);

		if (token == -1)
		{
			if (context.ifdepth > 0) { Logger::Error("Missing #endif! Found On {}", inputReader.location); }
			return -1;
		}
		if (token == '#')
		{
			if (context.previousToken == '\n')
			{
				token = ReadCPPline(parseToken);
				if (token == -1)
				{
					if (context.ifdepth > 0) { Logger::Error("Missing #endif! Found On {}", inputReader.location); }
					return -1;
				}
				continue;
			}
			else
			{
				Logger::Error("Preprocessor Directive Cannot Be Preceded By Another Token! Found On {}", parseToken.location);
				return -1;
			}
		}

		context.previousToken = token;

		if (token == '\n')
			continue;

		// expand macros
		if (token == FIXED_ATOM_Identifier)
		{
			switch (MacroExpand(parseToken, false, true))
			{
			case MACRO_EXPAND_RESULT_NOT_STARTED: break;
			case MACRO_EXPAND_RESULT_ERROR: return -1;
			case MACRO_EXPAND_RESULT_STARTED:
			case MACRO_EXPAND_RESULT_UNDEF: continue;
			}
		}

		switch (token)
		{
		case FIXED_ATOM_Identifier:
		case FIXED_ATOM_ConstInt:
		case FIXED_ATOM_ConstUint:
		case FIXED_ATOM_ConstFloat:
		case FIXED_ATOM_ConstInt64:
		case FIXED_ATOM_ConstUint64:
		case FIXED_ATOM_ConstInt16:
		case FIXED_ATOM_ConstUint16:
		case FIXED_ATOM_ConstDouble:
		case FIXED_ATOM_ConstFloat16: { if (parseToken.name[0] == '\0') { continue; } } break;
		case FIXED_ATOM_ConstString: {} break;
		case '\'': { Logger::Error("Character Literals Are Not Supported! Found On {}", parseToken.location); } continue;
		default: { parseToken.name = context.atomStrings.GetString(token); } break;
		}

		return token;
	}
}

I32 ParseContext::Scan(Input& input, ParseToken& parseToken)
{
	switch (input.type)
	{
	case INPUT_TYPE_MACRO: {
		MacroSymbol* macro = input.macro.symbol;

		I32 token = -1;
		do { token = macro->body.GetToken(*this, parseToken); } while (token == ' ');

		bool pasting = false;
		if (input.macro.postpaste)
		{
			pasting = true;
			input.macro.postpaste = false;
		}

		if (input.macro.prepaste)
		{
			input.macro.prepaste = false;
			input.macro.postpaste = true;
		}

		if (macro->body.PeekUntokenizedPasting())
		{
			input.macro.prepaste = true;
			pasting = true;
		}

		// TODO: preprocessor:  properly handle whitespace (or lack of it) between tokens when expanding
		if (token == FIXED_ATOM_Identifier)
		{
			U32 i;
			for (i = (U32)macro->macroArgs.Size() - 1; i >= 0; --i)
			{
				if (parseToken.name.Compare(context.atomStrings.GetString(macro->macroArgs[i]))) { break; }
			}

			if (i >= 0)
			{
				TokenStream* arg = input.macro.expandedArgs[i];
				bool expanded = arg && !pasting;

				if (arg == nullptr || pasting)
				{
					arg = input.macro.args[i];
				}

				context.PushTokenStreamInput(*arg, input.macro.prepaste, expanded);

				return ScanToken(parseToken);
			}
		}

		if (token == -1) { input.macro.symbol->busy = 0; }

		return token;
	} break;
	case INPUT_TYPE_MARKER: {
		if (input.done) { return -1; }
		input.done = true;

		return -3;
	} break;
	case INPUT_TYPE_ZERO: {
		if (input.done) { return -1; }

		parseToken.name[0] = '0';
		parseToken.name[1] = 0;
		parseToken.ival = 0;
		parseToken.space = false;
		input.done = true;

		return FIXED_ATOM_ConstInt;
	} break;
	case INPUT_TYPE_TOKEN: {
		I32 token = input.token.tokens->GetToken(*this, parseToken);

		parseToken.fullyExpanded = input.token.preExpanded;
		if (input.token.tokens->AtEnd() && token == FIXED_ATOM_Identifier)
		{
			I32 macroAtom = context.atomStrings.GetAtom(parseToken.name);
			MacroSymbol* macro = macroAtom == 0 ? nullptr : context.LookupMacroDef(macroAtom);
			if (macro && macro->functionLike) { parseToken.fullyExpanded = false; }
		}
		return token;
	} break;
	case INPUT_TYPE_UNGOT_TOKEN: {
		if (input.done) { return -1; }

		I32 ret = input.ungotToken.token;
		parseToken = *input.ungotToken.lval; //TODO: Might be bad, may need to change to pointer
		input.done = true;

		return ret;
	} break;
	case INPUT_TYPE_STRING: {
		bool alreadyComplained = false;
		I32 len = 0;
		I32 ch = 0;
		I32 ii = 0;
		U64 ival = 0;

		static const C8* const Int64_Extensions[]{
			"GL_ARB_gpu_shader_int64",
			"GL_EXT_shader_explicit_arithmetic_types",
			"GL_EXT_shader_explicit_arithmetic_types_int64"
		};
		static constexpr U32 Num_Int64_Extensions = CountOf32(Int64_Extensions);

		static const C8* Int16_Extensions[]{
			"GL_ARB_gpu_shader_int16",
			"GL_EXT_shader_explicit_arithmetic_types",
			"GL_EXT_shader_explicit_arithmetic_types_int16"
		};
		static constexpr U32 Num_Int16_Extensions = CountOf32(Int16_Extensions);

		parseToken.Clear();
		ch = input.Getch(inputReader);
		while (true)
		{
			while (ch == ' ' || ch == '\t')
			{
				parseToken.space = true;
				ch = input.Getch(inputReader);
			}

			parseToken.location = inputReader.location;
			len = 0;
			switch (ch)
			{
			default:
				// Single character token, including EndOfInput, '#' and '\' (escaped newlines are handled at a lower level, so this is just a '\' token)
				if (ch > FIXED_ATOM_MAX_SINGLE) { ch = FIXED_ATOM_BadToken; }
				return ch;

			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F': case 'G': case 'H': case 'I': case 'J':
			case 'K': case 'L': case 'M': case 'N': case 'O':
			case 'P': case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X': case 'Y':
			case 'Z': case '_':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o':
			case 'p': case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x': case 'y':
			case 'z':
				do
				{
					if (len < 1024)
					{
						parseToken.name[len++] = (C8)ch;
						ch = input.Getch(inputReader);
					}
					else
					{
						if (!alreadyComplained)
						{
							Logger::Error("Name Too Long On {}", parseToken.location);
							alreadyComplained = true;
						}
						ch = input.Getch(inputReader);
					}
				} while ((ch >= 'a' && ch <= 'z') ||
					(ch >= 'A' && ch <= 'Z') ||
					(ch >= '0' && ch <= '9') ||
					ch == '_');

				if (len == 0) { continue; }

				parseToken.name[len] = '\0';
				input.Ungetch(inputReader);
				return FIXED_ATOM_Identifier;
			case '0':
				parseToken.name[len++] = (char)ch;
				ch = input.Getch(inputReader);
				if (ch == 'x' || ch == 'X')
				{
					bool isUnsigned = false;
					bool isInt64 = false;
					bool isInt16 = false;
					parseToken.name[len++] = (char)ch;
					ch = input.Getch(inputReader);
					if ((ch >= '0' && ch <= '9') ||
						(ch >= 'A' && ch <= 'F') ||
						(ch >= 'a' && ch <= 'f'))
					{

						ival = 0;
						do
						{
							if (len < 1024 && ival <= 0x0fffffffffffffffull)
							{
								parseToken.name[len++] = (char)ch;
								if (ch >= '0' && ch <= '9')
								{
									ii = ch - '0';
								}
								else if (ch >= 'A' && ch <= 'F')
								{
									ii = ch - 'A' + 10;
								}
								else if (ch >= 'a' && ch <= 'f')
								{
									ii = ch - 'a' + 10;
								}
								else { Logger::Error("Bad Digit In Hexadecimal Literal On {}", parseToken.location); }
								ival = (ival << 4) | ii;
							}
							else
							{
								if (!alreadyComplained)
								{
									if (len < 1024) { Logger::Error("Hexadecimal Literal Too Big On {}", parseToken.location); }
									else { Logger::Error("Hexadecimal Literal Too Long On {}", parseToken.location); }
									alreadyComplained = true;
								}
								ival = 0xffffffffffffffffull;
							}
							ch = input.Getch(inputReader);
						} while ((ch >= '0' && ch <= '9') ||
							(ch >= 'A' && ch <= 'F') ||
							(ch >= 'a' && ch <= 'f'));
					}
					else
					{
						Logger::Error("Bad Digit In Hexadecimal Literal On {}", parseToken.location);
					}
					if (ch == 'u' || ch == 'U')
					{
						if (len < 1024)
							parseToken.name[len++] = (char)ch;
						isUnsigned = true;

						int nextCh = input.Getch(inputReader);
						if (nextCh == 'l' || nextCh == 'L')
						{
							if (len < 1024)
								parseToken.name[len++] = (char)nextCh;
							isInt64 = true;
						}
						else { input.Ungetch(inputReader); }

						nextCh = input.Getch(inputReader);
						if ((nextCh == 's' || nextCh == 'S'))
						{
							if (len < 1024)
								parseToken.name[len++] = (char)nextCh;
							isInt16 = true;
						}
						else { input.Ungetch(inputReader); }
					}
					else if (ch == 'l' || ch == 'L')
					{
						if (len < 1024)
							parseToken.name[len++] = (char)ch;
						isInt64 = true;
					}
					else if ((ch == 's' || ch == 'S'))
					{
						if (len < 1024)
							parseToken.name[len++] = (char)ch;
						isInt16 = true;
					}
					else
						input.Ungetch(inputReader);
					parseToken.name[len] = '\0';

					if (isInt64)
					{
						parseToken.i64val = ival;
						return isUnsigned ? FIXED_ATOM_ConstUint64 : FIXED_ATOM_ConstInt64;
					}
					else if (isInt16)
					{
						parseToken.ival = (int)ival;
						return isUnsigned ? FIXED_ATOM_ConstUint16 : FIXED_ATOM_ConstInt16;
					}
					else
					{
						if (ival > 0xffffffffu && !alreadyComplained)
						{
							Logger::Error("Hexadecimal Literal Too Big On {}", parseToken.location);
						}
						parseToken.ival = (int)ival;
						return isUnsigned ? FIXED_ATOM_ConstUint : FIXED_ATOM_ConstInt;
					}
				}
				else
				{
					bool isUnsigned = false;
					bool isInt64 = false;
					bool isInt16 = false;
					bool octalOverflow = false;
					bool nonOctal = false;
					ival = 0;

					while (ch >= '0' && ch <= '7')
					{
						if (len < 1024)
							parseToken.name[len++] = (char)ch;
						else if (!alreadyComplained)
						{
							Logger::Error("Numeric Literal Too Long On {}", parseToken.location);
							alreadyComplained = true;
						}
						if (ival <= 0x1fffffffffffffffull)
						{
							ii = ch - '0';
							ival = (ival << 3) | ii;
						}
						else
							octalOverflow = true;
						ch = input.Getch(inputReader);
					}

					if (ch == '8' || ch == '9')
					{
						nonOctal = true;
						do
						{
							if (len < 1024)
								parseToken.name[len++] = (char)ch;
							else if (!alreadyComplained)
							{
								Logger::Error("Numeric Literal Too Long On {}", parseToken.location);
								alreadyComplained = 1;
							}
							ch = input.Getch(inputReader);
						} while (ch >= '0' && ch <= '9');
					}

					if (ch == '.' || ch == 'e' || ch == 'E' || ch == 'f' || ch == 'F' || ch == 'h' || ch == 'H') { return FloatConst(len, ch, parseToken); }

					// wasn't a float, so must be octal...
					if (nonOctal) { Logger::Error("Octal Literal Digit Too Long On {}", parseToken.location); }

					if (ch == 'u' || ch == 'U')
					{
						if (len < 1024) { parseToken.name[len++] = (char)ch; }
						isUnsigned = true;

						int nextCh = input.Getch(inputReader);
						if (nextCh == 'l' || nextCh == 'L')
						{
							if (len < 1024) { parseToken.name[len++] = (char)nextCh; }
							isInt64 = true;
						}
						else { input.Ungetch(inputReader); }

						nextCh = input.Getch(inputReader);
						if ((nextCh == 's' || nextCh == 'S'))
						{
							if (len < 1024) { parseToken.name[len++] = (char)nextCh; }
							isInt16 = true;
						}
						else { input.Ungetch(inputReader); }
					}
					else if (ch == 'l' || ch == 'L')
					{
						if (len < 1024) { parseToken.name[len++] = (char)ch; }
						isInt64 = true;
					}
					else if ((ch == 's' || ch == 'S'))
					{
						if (len < 1024)
							parseToken.name[len++] = (char)ch;
						isInt16 = true;
					}
					else
						input.Ungetch(inputReader);
					parseToken.name[len] = '\0';

					if (!isInt64 && ival > 0xffffffffu)
						octalOverflow = true;

					if (octalOverflow) { Logger::Error("Octal Literal Too Big On {}", parseToken.location); }

					if (isInt64)
					{
						parseToken.i64val = ival;
						return isUnsigned ? FIXED_ATOM_ConstUint64 : FIXED_ATOM_ConstInt64;
					}
					else if (isInt16)
					{
						parseToken.ival = (int)ival;
						return isUnsigned ? FIXED_ATOM_ConstUint16 : FIXED_ATOM_ConstInt16;
					}
					else
					{
						parseToken.ival = (int)ival;
						return isUnsigned ? FIXED_ATOM_ConstUint : FIXED_ATOM_ConstInt;
					}
				}
				break;
			case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':

				do
				{
					if (len < 1024)
						parseToken.name[len++] = (char)ch;
					else if (!alreadyComplained)
					{
						Logger::Error("Numerical Literal Too Long On {}", parseToken.location);
						alreadyComplained = 1;
					}
					ch = input.Getch(inputReader);
				} while (ch >= '0' && ch <= '9');
				if (ch == '.' || ch == 'e' || ch == 'E' || ch == 'f' || ch == 'F' || ch == 'h' || ch == 'H')
					return FloatConst(len, ch, parseToken);
				else
				{
					// Finish handling signed and unsigned integers
					int numericLen = len;
					bool isUnsigned = false;
					bool isInt64 = false;
					bool isInt16 = false;
					if (ch == 'u' || ch == 'U')
					{
						if (len < 1024)
							parseToken.name[len++] = (char)ch;
						isUnsigned = true;

						int nextCh = input.Getch(inputReader);
						if (nextCh == 'l' || nextCh == 'L')
						{
							if (len < 1024)
								parseToken.name[len++] = (char)nextCh;
							isInt64 = true;
						}
						else
							input.Ungetch(inputReader);

						nextCh = input.Getch(inputReader);
						if ((nextCh == 's' || nextCh == 'S'))
						{
							if (len < 1024)
								parseToken.name[len++] = (char)nextCh;
							isInt16 = true;
						}
						else
							input.Ungetch(inputReader);
					}
					else if (ch == 'l' || ch == 'L')
					{
						if (len < 1024)
							parseToken.name[len++] = (char)ch;
						isInt64 = true;
					}
					else if ((ch == 's' || ch == 'S'))
					{
						if (len < 1024)
							parseToken.name[len++] = (char)ch;
						isInt16 = true;
					}
					else
						input.Ungetch(inputReader);

					parseToken.name[len] = '\0';
					ival = 0;
					const unsigned oneTenthMaxInt = 0xFFFFFFFFu / 10;
					const unsigned remainderMaxInt = 0xFFFFFFFFu - 10 * oneTenthMaxInt;
					const unsigned long long oneTenthMaxInt64 = 0xFFFFFFFFFFFFFFFFull / 10;
					const unsigned long long remainderMaxInt64 = 0xFFFFFFFFFFFFFFFFull - 10 * oneTenthMaxInt64;
					const unsigned short oneTenthMaxInt16 = 0xFFFFu / 10;
					const unsigned short remainderMaxInt16 = 0xFFFFu - 10 * oneTenthMaxInt16;
					for (int i = 0; i < numericLen; i++)
					{
						ch = parseToken.name[i] - '0';
						bool overflow = false;
						if (isInt64)
							overflow = (ival > oneTenthMaxInt64 || (ival == oneTenthMaxInt64 && (unsigned long long)ch > remainderMaxInt64));
						else if (isInt16)
							overflow = (ival > oneTenthMaxInt16 || (ival == oneTenthMaxInt16 && (unsigned short)ch > remainderMaxInt16));
						else
							overflow = (ival > oneTenthMaxInt || (ival == oneTenthMaxInt && (unsigned)ch > remainderMaxInt));
						if (overflow)
						{
							Logger::Error("Numerical Literal Too Big On {}", parseToken.location);
							ival = 0xFFFFFFFFFFFFFFFFull;
							break;
						}
						else
							ival = ival * 10 + ch;
					}

					if (isInt64)
					{
						parseToken.i64val = ival;
						return isUnsigned ? FIXED_ATOM_ConstUint64 : FIXED_ATOM_ConstInt64;
					}
					else if (isInt16)
					{
						parseToken.ival = (int)ival;
						return isUnsigned ? FIXED_ATOM_ConstUint16 : FIXED_ATOM_ConstInt16;
					}
					else
					{
						parseToken.ival = (int)ival;
						return isUnsigned ? FIXED_ATOM_ConstUint : FIXED_ATOM_ConstInt;
					}
				}
				break;
			case '-':
				ch = input.Getch(inputReader);
				if (ch == '-')
				{
					return FIXED_ATOM_Decrement;
				}
				else if (ch == '=')
				{
					return FIXED_ATOM_SubAssign;
				}
				else
				{
					input.Ungetch(inputReader);
					return '-';
				}
			case '+':
				ch = input.Getch(inputReader);
				if (ch == '+')
				{
					return FIXED_ATOM_Increment;
				}
				else if (ch == '=')
				{
					return FIXED_ATOM_AddAssign;
				}
				else
				{
					input.Ungetch(inputReader);
					return '+';
				}
			case '*':
				ch = input.Getch(inputReader);
				if (ch == '=')
				{
					return FIXED_ATOM_MulAssign;
				}
				else
				{
					input.Ungetch(inputReader);
					return '*';
				}
			case '%':
				ch = input.Getch(inputReader);
				if (ch == '=')
				{
					return FIXED_ATOM_ModAssign;
				}
				else
				{
					input.Ungetch(inputReader);
					return '%';
				}
			case '^':
				ch = input.Getch(inputReader);
				if (ch == '^')
				{
					return FIXED_ATOM_Xor;
				}
				else
				{
					if (ch == '=')
						return FIXED_ATOM_XorAssign;
					else
					{
						input.Ungetch(inputReader);
						return '^';
					}
				}

			case '=':
				ch = input.Getch(inputReader);
				if (ch == '=')
				{
					return FIXED_ATOM_EQ;
				}
				else
				{
					input.Ungetch(inputReader);
					return '=';
				}
			case '!':
				ch = input.Getch(inputReader);
				if (ch == '=')
				{
					return FIXED_ATOM_NE;
				}
				else
				{
					input.Ungetch(inputReader);
					return '!';
				}
			case '|':
				ch = input.Getch(inputReader);
				if (ch == '|')
				{
					return FIXED_ATOM_Or;
				}
				else if (ch == '=')
				{
					return FIXED_ATOM_OrAssign;
				}
				else
				{
					input.Ungetch(inputReader);
					return '|';
				}
			case '&':
				ch = input.Getch(inputReader);
				if (ch == '&')
				{
					return FIXED_ATOM_And;
				}
				else if (ch == '=')
				{
					return FIXED_ATOM_AndAssign;
				}
				else
				{
					input.Ungetch(inputReader);
					return '&';
				}
			case '<':
				ch = input.Getch(inputReader);
				if (ch == '<')
				{
					ch = input.Getch(inputReader);
					if (ch == '=')
						return FIXED_ATOM_LeftAssign;
					else
					{
						input.Ungetch(inputReader);
						return FIXED_ATOM_Left;
					}
				}
				else if (ch == '=')
				{
					return FIXED_ATOM_LE;
				}
				else
				{
					input.Ungetch(inputReader);
					return '<';
				}
			case '>':
				ch = input.Getch(inputReader);
				if (ch == '>')
				{
					ch = input.Getch(inputReader);
					if (ch == '=')
						return FIXED_ATOM_RightAssign;
					else
					{
						input.Ungetch(inputReader);
						return FIXED_ATOM_Right;
					}
				}
				else if (ch == '=')
				{
					return FIXED_ATOM_GE;
				}
				else
				{
					input.Ungetch(inputReader);
					return '>';
				}
			case '.':
				ch = input.Getch(inputReader);
				if (ch >= '0' && ch <= '9')
				{
					input.Ungetch(inputReader);
					return FloatConst(0, '.', parseToken);
				}
				else
				{
					input.Ungetch(inputReader);
					return '.';
				}
			case '/':
				ch = input.Getch(inputReader);
				if (ch == '/')
				{
					context.inComment = true;
					do
					{
						ch = input.Getch(inputReader);
					} while (ch != '\n' && ch != -1);
					parseToken.space = true;
					context.inComment = false;

					return ch;
				}
				else if (ch == '*')
				{
					ch = input.Getch(inputReader);
					do
					{
						while (ch != '*')
						{
							if (ch == -1)
							{
								Logger::Error("End Of Input In Comment On {}", parseToken.location);
								return ch;
							}
							ch = input.Getch(inputReader);
						}
						ch = input.Getch(inputReader);
						if (ch == -1)
						{
							Logger::Error("End Of Input In Comment On {}", parseToken.location);
							return ch;
						}
					} while (ch != '/');
					parseToken.space = true;
					break;
				}
				else if (ch == '=')
				{
					return FIXED_ATOM_DivAssign;
				}
				else
				{
					input.Ungetch(inputReader);
					return '/';
				}
				break;
			case '\'': { return '\''; }
			case '"':
				ch = input.Getch(inputReader);
				while (ch != '"' && ch != '\n' && ch != -1)
				{
					if (len < 1024)
					{
						if (ch == '\\' && !context.disableEscapeSequences)
						{
							int nextCh = input.Getch(inputReader);
							switch (nextCh)
							{
							case '\'': ch = 0x27; break;
							case '"':  ch = 0x22; break;
							case '?':  ch = 0x3f; break;
							case '\\': ch = 0x5c; break;
							case 'a':  ch = 0x07; break;
							case 'b':  ch = 0x08; break;
							case 'f':  ch = 0x0c; break;
							case 'n':  ch = 0x0a; break;
							case 'r':  ch = 0x0d; break;
							case 't':  ch = 0x09; break;
							case 'v':  ch = 0x0b; break;
							case 'x':
								// Hex value, arbitrary number of characters. Terminated by the first
								// non-hex digit
							{
								int numDigits = 0;
								ch = 0;
								while (true)
								{
									nextCh = input.Getch(inputReader);
									if (nextCh >= '0' && nextCh <= '9')
										nextCh -= '0';
									else if (nextCh >= 'A' && nextCh <= 'F')
										nextCh -= 'A' - 10;
									else if (nextCh >= 'a' && nextCh <= 'f')
										nextCh -= 'a' - 10;
									else
									{
										input.Ungetch(inputReader);
										break;
									}
									numDigits++;
									ch = ch * 0x10 + nextCh;
								}
								if (numDigits == 0)
								{
									Logger::Error("Expected Hex Value In Escape Sequence On {}", parseToken.location);
								}
								break;
							}
							case '0':
							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
							case '6':
							case '7':
								// Octal value, up to three octal digits
							{
								int numDigits = 1;
								ch = nextCh - '0';
								while (numDigits < 3)
								{
									nextCh = input.Getch(inputReader);
									if (nextCh >= '0' && nextCh <= '7')
										nextCh -= '0';
									else
									{
										input.Ungetch(inputReader);
										break;
									}
									numDigits++;
									ch = ch * 8 + nextCh;
								}
								break;
							}
							default:
								Logger::Error("Invalid Escape Sequence On {}", parseToken.location);
								break;
							}
						}
						parseToken.name[len] = (char)ch;
						len++;
						ch = input.Getch(inputReader);
					}
					else
						break;
				};
				parseToken.name[len] = '\0';
				if (ch != '"')
				{
					input.Ungetch(inputReader);
					Logger::Error("End Of Line In String On {}", parseToken.location);
				}
				return FIXED_ATOM_ConstString;
			case ':':
				ch = input.Getch(inputReader);
				if (ch == ':')
					return FIXED_ATOM_ColonColon;
				input.Ungetch(inputReader);
				return ':';
			}

			ch = input.Getch(inputReader);
		}
	} break;
	}
}

I32 ParseContext::FloatConst(I32 len, I32 ch, ParseToken& parseToken)
{
	const auto saveName = [&](int ch) {
		if (len <= 1024) { parseToken.name[len++] = static_cast<char>(ch); }
	};

	// find the range of non-zero digits before the decimal point
	int startNonZero = 0;
	while (startNonZero < len && parseToken.name[startNonZero] == '0')
		++startNonZero;
	int endNonZero = len;
	while (endNonZero > startNonZero && parseToken.name[endNonZero - 1] == '0')
		--endNonZero;
	int numWholeNumberDigits = endNonZero - startNonZero;

	// accumulate the range's value
	bool fastPath = numWholeNumberDigits <= 15;  // when the number gets too complex, set to false
	unsigned long long wholeNumber = 0;
	if (fastPath)
	{
		for (int i = startNonZero; i < endNonZero; ++i)
			wholeNumber = wholeNumber * 10 + (parseToken.name[i] - '0');
	}
	int decimalShift = len - endNonZero;

	// Decimal point:
	bool hasDecimalOrExponent = false;
	if (ch == '.')
	{
		hasDecimalOrExponent = true;
		saveName(ch);
		ch = GetChar();
		int firstDecimal = len;

		// Consume leading-zero digits after the decimal point
		while (ch == '0')
		{
			saveName(ch);
			ch = GetChar();
		}
		int startNonZeroDecimal = len;
		int endNonZeroDecimal = len;

		// Consume remaining digits, up to the exponent
		while (ch >= '0' && ch <= '9')
		{
			saveName(ch);
			if (ch != '0')
				endNonZeroDecimal = len;
			ch = GetChar();
		}

		// Compute accumulation up to the last non-zero digit
		if (endNonZeroDecimal > startNonZeroDecimal)
		{
			numWholeNumberDigits += endNonZeroDecimal - endNonZero - 1; // don't include the "."
			if (numWholeNumberDigits > 15)
				fastPath = false;
			if (fastPath)
			{
				for (int i = endNonZero; i < endNonZeroDecimal; ++i)
				{
					if (parseToken.name[i] != '.')
						wholeNumber = wholeNumber * 10 + (parseToken.name[i] - '0');
				}
			}
			decimalShift = firstDecimal - endNonZeroDecimal;
		}
	}

	// Exponent:
	bool negativeExponent = false;
	double exponentValue = 0.0;
	int exponent = 0;
	{
		if (ch == 'e' || ch == 'E')
		{
			hasDecimalOrExponent = true;
			saveName(ch);
			ch = GetChar();
			if (ch == '+' || ch == '-')
			{
				negativeExponent = ch == '-';
				saveName(ch);
				ch = GetChar();
			}
			if (ch >= '0' && ch <= '9')
			{
				while (ch >= '0' && ch <= '9')
				{
					exponent = exponent * 10 + (ch - '0');
					saveName(ch);
					ch = GetChar();
				}
			}
			else
			{
				Logger::Error("Bad Character In Float Exponent On {}", parseToken.location);
			}
		}

		// Compensate for location of decimal
		if (negativeExponent)
			exponent -= decimalShift;
		else
		{
			exponent += decimalShift;
			if (exponent < 0)
			{
				negativeExponent = true;
				exponent = -exponent;
			}
		}
		if (exponent > 22)
			fastPath = false;

		if (fastPath)
		{
			// Compute the floating-point value of the exponent
			exponentValue = 1.0;
			if (exponent > 0)
			{
				double expFactor = 10;
				while (exponent > 0)
				{
					if (exponent & 0x1)
						exponentValue *= expFactor;
					expFactor *= expFactor;
					exponent >>= 1;
				}
			}
		}
	}

	// Suffix:
	bool isDouble = false;
	bool isFloat16 = false;
	if (ch == 'l' || ch == 'L')
	{
		if (context.ifdepth == 0 && !hasDecimalOrExponent)
		{
			Logger::Error("Float Literal Needs A Decimal Point Or Exponent On {}", parseToken.location);
		}

		int ch2 = GetChar();
		if (ch2 != 'f' && ch2 != 'F')
		{
			UngetChar();
			UngetChar();
		}
		else
		{
			saveName(ch);
			saveName(ch2);
			isDouble = true;
		}
	}
	else if (ch == 'h' || ch == 'H')
	{
		if (context.ifdepth == 0 && !hasDecimalOrExponent)
		{
			Logger::Error("Float Literal Needs A Decimal Point Or Exponent On {}", parseToken.location);
		}

		int ch2 = GetChar();
		if (ch2 != 'f' && ch2 != 'F')
		{
			UngetChar();
			UngetChar();
		}
		else
		{
			saveName(ch);
			saveName(ch2);
			isFloat16 = true;
		}
	}
	else
		if (ch == 'f' || ch == 'F')
		{
			if (context.ifdepth == 0 && !hasDecimalOrExponent)
			{
				Logger::Error("Float Literal Needs A Decimal Point Or Exponent On {}", parseToken.location);
			}

			saveName(ch);
		}
		else { UngetChar(); }

	// Patch up the name and length for overflow

	if (len > 1024)
	{
		len = 1024;
		Logger::Error("Float Literal Is Too Long On {}", parseToken.location);
	}
	parseToken.name[len] = '\0';

	// Compute the numerical value
	if (fastPath)
	{
		// compute the floating-point value of the exponent
		if (exponentValue == 0.0) { parseToken.dval = (F64)wholeNumber; }
		else if (negativeExponent) { parseToken.dval = (F64)wholeNumber / exponentValue; }
		else { parseToken.dval = (F64)wholeNumber * exponentValue; }
	}
	else
	{
		// slow path
		parseToken.dval = 0.0;

		// remove suffix
		String numstr = parseToken.name;
		if (numstr.Back() == 'f' || numstr.Back() == 'F') { numstr.PopBack(); }
		if (numstr.Back() == 'h' || numstr.Back() == 'H') { numstr.PopBack(); }
		if (numstr.Back() == 'l' || numstr.Back() == 'L') { numstr.PopBack(); }

		parseToken.dval = numstr.ToType<F64>();
	}

	// Return the right token type
	if (isDouble) { return FIXED_ATOM_ConstDouble; }
	else if (isFloat16) { return FIXED_ATOM_ConstFloat16; }
	else { return FIXED_ATOM_ConstFloat; }
}

I32 ParseContext::TokenPaste(I32 token, ParseToken& parseToken)
{
	// starting with ## is illegal, skip to next token
	if (token == FIXED_ATOM_Paste)
	{
		Logger::Error("Unexpected Location! Found On {}", parseToken.location);
		return ScanToken(parseToken);
	}

	I32 resultToken = token;

	// ## can be chained, process all in the chain at once
	while (context.PeekPasting())
	{
		ParseToken pastedParseToken;

		// next token has to be ##
		token = ScanToken(pastedParseToken);

		// This covers end of macro expansion
		if (context.EndOfReplacementList())
		{
			Logger::Error("Unexpected Location; End Of Replacement List! Found On {}", parseToken.location);
			break;
		}

		// Get the token(s) after the ##.
		// Because of "space" semantics, and prior tokenization, what
		// appeared a single token, e.g. "3A", might have been tokenized
		// into two tokens "3" and "A", but the "A" will have 'space' set to
		// false.  Accumulate all of these to recreate the original lexical
		// appearing token.
		do
		{
			token = ScanToken(pastedParseToken);

			// This covers end of argument expansion
			if (token == Input::marker)
			{
				Logger::Error("Unexpected Location; End Of Argument! Found On {}", parseToken.location);
				return resultToken;
			}

			// get the token text
			switch (resultToken)
			{
			case FIXED_ATOM_Identifier:
				// already have the correct text in token.names
				break;
			case '=':
			case '!':
			case '-':
			case '~':
			case '+':
			case '*':
			case '/':
			case '%':
			case '<':
			case '>':
			case '|':
			case '^':
			case '&':
			case FIXED_ATOM_Right:
			case FIXED_ATOM_Left:
			case FIXED_ATOM_And:
			case FIXED_ATOM_Or:
			case FIXED_ATOM_Xor:
				pastedParseToken.name = context.atomStrings.GetString(token);
				parseToken.name = context.atomStrings.GetString(resultToken) + pastedParseToken.name;
				break;
			default:
				Logger::Error("Not Supported For These Tokens! Found On {}", parseToken.location);
				return resultToken;
			}

			// correct the kind of token we are making, if needed (identifiers stay identifiers)
			if (resultToken != FIXED_ATOM_Identifier)
			{
				I32 newToken = context.atomStrings.GetAtom(parseToken.name);
				if (newToken > 0) { resultToken = newToken; }
				else { Logger::Error("Combined Token Is Invalid! Found On {}", parseToken.location); }
			}
		} while (context.PeekContinuedPasting(resultToken));
	}

	return resultToken;
}

I32 ParseContext::ReadCPPline(ParseToken& parseToken)
{
	int token = ScanToken(parseToken);

	if (token == FIXED_ATOM_Identifier)
	{
		switch (context.atomStrings.GetAtom(parseToken.name))
		{
		case FIXED_ATOM_Define: { token = CPPdefine(parseToken); } break;
		case FIXED_ATOM_Else:
			if (context.elseSeen[context.elsetracker]) { Logger::Error("#else After #else! Found On {}", parseToken.location); }
			context.elseSeen[context.elsetracker] = true;
			if (context.ifdepth == 0) { Logger::Error("Mismatched Statements! Found On {}", parseToken.location); }
			token = ExtraTokenCheck(FIXED_ATOM_Else, parseToken, ScanToken(parseToken));
			token = CPPelse(0, parseToken);
			break;
		case FIXED_ATOM_Elif:
			if (context.ifdepth == 0) { Logger::Error("Mismatched Statements! Found On {}", parseToken.location); }
			if (context.elseSeen[context.elsetracker]) { Logger::Error("#elif After #else! Found On {}", parseToken.location); }
			// this token is really a dont care, but we still need to eat the tokens
			token = ScanToken(parseToken);
			while (token != '\n' && token != -1) { token = ScanToken(parseToken); }
			token = CPPelse(0, parseToken);
			break;
		case FIXED_ATOM_Endif:
			if (context.ifdepth == 0) { Logger::Error("Mismatched Statements! Found On {}", parseToken.location); }
			else
			{
				context.elseSeen[context.elsetracker] = false;
				--context.elsetracker;
				--context.ifdepth;
			}
			token = ExtraTokenCheck(FIXED_ATOM_Endif, parseToken, ScanToken(parseToken));
			break;
		case FIXED_ATOM_If: { token = CPPif(parseToken); } break;
		case FIXED_ATOM_Ifdef: { token = CPPifdef(1, parseToken); } break;
		case FIXED_ATOM_Ifndef: { token = CPPifdef(0, parseToken); } break;
		case FIXED_ATOM_Include:
			token = CPPinclude(parseToken);
			break;
		case FIXED_ATOM_Pragma: { token = CPPpragma(parseToken); } break;
		case FIXED_ATOM_Undef: { token = CPPundef(parseToken); } break;
		case FIXED_ATOM_Error: { token = CPPerror(parseToken); } break;
		default: { Logger::Error("Invalid Directive '{}'! Found On {}", parseToken.name, parseToken.location); } break;
		}
	}
	else if (token != '\n' && token != -1) { Logger::Error("Invalid Directive! Found On {}", parseToken.location); }

	while (token != '\n' && token != -1) { token = ScanToken(parseToken); }

	return token;
}

I32 ParseContext::CPPdefine(ParseToken& parseToken)
{
	MacroSymbol macro;

	// get the macro name
	I32 token = ScanToken(parseToken);
	if (token != FIXED_ATOM_Identifier)
	{
		Logger::Error("'#define' Must Be Followed By A Macro Name! Found On {}", parseToken.location);
		return token;
	}
	if (parseToken.location.string >= 0 && parseToken.name.CompareN("defined", 8))
	{
		// We are in user code; check for reserved name use:
		Logger::Error("'defined' Cannot Be Defined! Found On {}", parseToken.location);
	}

	// save the macro name
	const I32 defAtom = context.atomStrings.GetAddAtom(parseToken.name);
	Location defineLoc = parseToken.location; // because parseToken might go to the next line before we report errors

	// gather parameters to the macro, between (...)
	token = ScanToken(parseToken);
	if (token == '(' && !parseToken.space)
	{
		macro.functionLike = 1;
		do
		{
			token = ScanToken(parseToken);
			if (macro.macroArgs.Size() == 0 && token == ')') { break; }
			if (token != FIXED_ATOM_Identifier)
			{
				Logger::Error("Bad Argument Found On {}", parseToken.location);

				return token;
			}
			const I32 argAtom = context.atomStrings.GetAddAtom(parseToken.name);

			// check for duplication of parameter name
			bool duplicate = false;
			for (size_t a = 0; a < macro.macroArgs.Size(); ++a)
			{
				if (macro.macroArgs[a] == argAtom)
				{
					Logger::Error("Duplicate Macro Parameter Found On {}", parseToken.location);
					duplicate = true;
					break;
				}
			}
			if (!duplicate)
				macro.macroArgs.Push(argAtom);
			token = ScanToken(parseToken);
		} while (token == ',');
		if (token != ')')
		{
			Logger::Error("Missing Parenthesis Found On {}", parseToken.location);

			return token;
		}

		token = ScanToken(parseToken);
	}
	else if (token != '\n' && token != -1 && !parseToken.space)
	{
		Logger::Warn("Missing Space After Macro Name! Found On {}", parseToken.location);

		return token;
	}

	// record the definition of the macro
	while (token != '\n' && token != -1)
	{
		macro.body.PutToken(token, parseToken);
		token = ScanToken(parseToken);
		if (token != '\n' && parseToken.space)
			macro.body.PutToken(' ', parseToken);
	}

	// check for duplicate definition
	MacroSymbol* existing = context.LookupMacroDef(defAtom);
	if (existing != nullptr)
	{
		if (!existing->undef)
		{
			// Already defined -- need to make sure they are identical:
			// "Two replacement lists are identical if and only if the
			// preprocessing tokens in both have the same number,
			// ordering, spelling, and white-space separation, where all
			// white-space separations are considered identical."
			if (existing->functionLike != macro.functionLike)
			{
				Logger::Error("Macro Redefined; Function-like Versus Object-like! Found On {}", defineLoc);
			}
			else if (existing->macroArgs.Size() != macro.macroArgs.Size())
			{
				Logger::Error("Macro Redefined; Different Number Of Arguments! Found On {}", defineLoc);
			}
			else
			{
				if (existing->macroArgs != macro.macroArgs)
				{
					Logger::Error("Macro Redefined; Different Argument Names! Found On {}", defineLoc);
				}
				// set up to compare the two
				existing->body.Reset();
				macro.body.Reset();
				I32 newToken;
				bool firstToken = true;
				do
				{
					I32 oldToken;
					ParseToken oldParseToken;
					ParseToken newParseToken;
					oldToken = existing->body.GetToken(*this, oldParseToken);
					newToken = macro.body.GetToken(*this, newParseToken);
					// for the first token, preceding spaces don't matter
					if (firstToken)
					{
						newParseToken.space = oldParseToken.space;
						firstToken = false;
					}
					if (oldToken != newToken || oldParseToken != newParseToken)
					{
						Logger::Error("Macro Redefined; Different Substitutions! Found On {}", defineLoc);
						break;
					}
				} while (newToken != -1);
			}
		}

		*existing = Move(macro);
	}
	else { context.AddMacroDef(defAtom, macro); }

	return '\n';
}

I32 ParseContext::CPPundef(ParseToken& parseToken)
{
	I32 token = ScanToken(parseToken);
	if (token != FIXED_ATOM_Identifier)
	{
		Logger::Error("'#undef' Must be Followed By A Macro Name! Found On {}", parseToken.location);

		return token;
	}

	if (parseToken.name.CompareN("defined", 8))
	{
		// We are in user code; check for reserved name use:
		Logger::Error("'defined' Cannot Be Undefined! Found On {}", parseToken.location);
	}

	MacroSymbol* macro = context.LookupMacroDef(context.atomStrings.GetAtom(parseToken.name));

	if (macro != nullptr) { macro->undef = 1; }

	token = ScanToken(parseToken);

	if (token != '\n') { Logger::Error("'#undef' Can Only Be Followed By A Single Macro Name! Found On {}", parseToken.location); }

	return token;
}

I32 ParseContext::CPPelse(I32 matchelse, ParseToken& parseToken)
{
	I32 depth = 0;
	I32 token = ScanToken(parseToken);

	while (token != -1)
	{
		if (token != '#')
		{
			while (token != '\n' && token != -1)
				token = ScanToken(parseToken);

			if (token == -1)
				return token;

			token = ScanToken(parseToken);
			continue;
		}

		if ((token = ScanToken(parseToken)) != FIXED_ATOM_Identifier)
			continue;

		int nextAtom = context.atomStrings.GetAtom(parseToken.name);
		if (nextAtom == FIXED_ATOM_If || nextAtom == FIXED_ATOM_Ifdef || nextAtom == FIXED_ATOM_Ifndef)
		{
			depth++;
			if (context.ifdepth >= context.maxIfNesting || context.elsetracker >= context.maxIfNesting)
			{
				Logger::Error("Maximum Nesting Depth Of '#if/#ifdef/#ifndef' Exceeded! Found On {}", parseToken.location);
				return -1;
			}
			else
			{
				context.ifdepth++;
				context.elsetracker++;
			}
		}
		else if (nextAtom == FIXED_ATOM_Endif)
		{
			token = ExtraTokenCheck(nextAtom, parseToken, ScanToken(parseToken));
			context.elseSeen[context.elsetracker] = false;
			--context.elsetracker;
			if (depth == 0)
			{
				// found the #endif we are looking for
				if (context.ifdepth > 0)
					--context.ifdepth;
				break;
			}
			--depth;
			--context.ifdepth;
		}
		else if (matchelse && depth == 0)
		{
			if (nextAtom == FIXED_ATOM_Else)
			{
				context.elseSeen[context.elsetracker] = true;
				token = ExtraTokenCheck(nextAtom, parseToken, ScanToken(parseToken));
				// found the #else we are looking for
				break;
			}
			else if (nextAtom == FIXED_ATOM_Elif)
			{
				if (context.elseSeen[context.elsetracker]) { Logger::Error("#elif After #else! Found On {}", parseToken.location); }
				/* we decrement ifdepth here, because CPPif will increment
				* it and we really want to leave it alone */
				if (context.ifdepth > 0)
				{
					--context.ifdepth;
					context.elseSeen[context.elsetracker] = false;
					--context.elsetracker;
				}

				return CPPif(parseToken);
			}
		}
		else if (nextAtom == FIXED_ATOM_Else)
		{
			if (context.elseSeen[context.elsetracker]) { Logger::Error("#else After #else! Found On {}", parseToken.location); }
			else { context.elseSeen[context.elsetracker] = true; }
			token = ExtraTokenCheck(nextAtom, parseToken, ScanToken(parseToken));
		}
		else if (nextAtom == FIXED_ATOM_Elif)
		{
			if (context.elseSeen[context.elsetracker]) { Logger::Error("#elif After #else! Found On {}", parseToken.location); }
		}
	}

	return token;
}

I32 ParseContext::CPPif(ParseToken& parseToken)
{
	I32 token = ScanToken(parseToken);
	if (context.ifdepth >= context.maxIfNesting || context.elsetracker >= context.maxIfNesting)
	{
		Logger::Error("Maximum Nesting Depth of '#if' Exceeded! Found On {}", parseToken.location);
		return -1;
	}
	else
	{
		context.elsetracker++;
		context.ifdepth++;
	}
	I32 res = 0;
	bool err = false;
	token = Eval(token, MIN_PRECEDENCE, false, res, err, parseToken);
	token = ExtraTokenCheck(FIXED_ATOM_If, parseToken, token);
	if (!res && !err) { token = CPPelse(1, parseToken); }

	return token;
}

I32 ParseContext::CPPifdef(I32 defined, ParseToken& parseToken)
{
	I32 token = ScanToken(parseToken);
	if (context.ifdepth > context.maxIfNesting || context.elsetracker > context.maxIfNesting)
	{
		Logger::Error("Maximum Nesting Depth of '#ifdef' Exceeded! Found On {}", parseToken.location);
		return -1;
	}
	else
	{
		context.elsetracker++;
		context.ifdepth++;
	}

	if (token != FIXED_ATOM_Identifier)
	{
		if (defined) { Logger::Error("'#ifdef' Must Be Followed By A Macro Name! Found On {}", parseToken.location); }
		else { Logger::Error("'#ifndef' Must Be Followed By A Macro Name! Found On {}", parseToken.location); }
	}
	else
	{
		MacroSymbol* macro = context.LookupMacroDef(context.atomStrings.GetAtom(parseToken.name));
		token = ScanToken(parseToken);
		if (token != '\n')
		{
			Logger::Error("Unexpected Tokens Following '#ifdef' Directive; Expected A Newline! Found On {}", parseToken.location);
			while (token != '\n' && token != -1) { token = ScanToken(parseToken); }
		}

		if (((macro != nullptr && !macro->undef) ? 1 : 0) != defined) { token = CPPelse(1, parseToken); }
	}

	return token;
}

I32 ParseContext::CPPinclude(ParseToken& parseToken)
{
	const Location directiveLoc = parseToken.location;
	bool startWithLocalSearch = true; // to additionally include the extra "" paths
	I32 token;

	// Find the first non-whitespace char after #include
	I32 ch = GetChar();
	while (ch == ' ' || ch == '\t')
	{
		ch = GetChar();
	}
	if (ch == '<')
	{
		// <header-name> style
		startWithLocalSearch = false;
		token = ScanHeaderName(parseToken, '>');
	}
	else if (ch == '"')
	{
		// "header-name" style
		token = ScanHeaderName(parseToken, '"');
	}
	else
	{
		// unexpected, get the full token to generate the error
		UngetChar();
		token = ScanToken(parseToken);
	}

	if (token != FIXED_ATOM_ConstString)
	{
		Logger::Error("'#include' Must Be Followed By A Header Name! Found On {}", parseToken.location);
		return token;
	}

	// Make a copy of the name because it will be overwritten by the next token scan.
	String filename = Move(parseToken.name);

	// See if the directive was well formed
	token = ScanToken(parseToken);
	if (token != '\n')
	{
		Logger::Error("Expected A Newline After The Header Name For #include '{}'! Found On {}", filename, parseToken.location);
		return token;
	}

	// Process well-formed directive

	// Find the inclusion, first look in "Local" ("") paths, if requested,
	// otherwise, only search the "System" (<>) paths.
	TShader::Includer::IncludeResult* res = nullptr;
	if (startWithLocalSearch)
		res = includer.includeLocal(filename.c_str(), currentSourceFile.c_str(), includeStack.size() + 1);
	if (res == nullptr || res->headerName.empty())
	{
		includer.releaseInclude(res);
		res = includer.includeSystem(filename.c_str(), currentSourceFile.c_str(), includeStack.size() + 1);
	}

	// Process the results
	if (res != nullptr && !res->headerName.empty())
	{
		if (res->headerData != nullptr && res->headerLength > 0)
		{
			// path for processing one or more tokens from an included header, hand off 'res'
			const bool forNextLine = parseContext.lineDirectiveShouldSetNextLine();
			std::ostringstream prologue;
			std::ostringstream epilogue;
			prologue << "#line " << forNextLine << " " << "\"" << res->headerName << "\"\n";
			epilogue << (res->headerData[res->headerLength - 1] == '\n' ? "" : "\n") <<
				"#line " << directiveLoc.line + forNextLine << " " << directiveLoc.getStringNameOrNum() << "\n";
			PushInput(new TokenizableIncludeFile(directiveLoc, prologue.str(), res, epilogue.str(), this));
			parseContext.intermediate.addIncludeText(res->headerName.c_str(), res->headerData, res->headerLength);
			// There's no "current" location anymore.
			parseContext.setCurrentColumn(0);
		}
		else
		{
			// things are okay, but there is nothing to process
			includer.releaseInclude(res);
		}
	}
	else
	{
		// error path, clean up
		String message =
			res != nullptr ? std::string(res->headerData, res->headerLength)
			: std::string("Could not process include directive");
		parseContext.ppError(directiveLoc, message.c_str(), "#include", "for header name: %s", filename.c_str());
		includer.releaseInclude(res);
	}

	return token;
}

I32 ParseContext::CPPerror(ParseToken& parseToken)
{
	context.disableEscapeSequences = true;
	I32 token = ScanToken(parseToken);
	context.disableEscapeSequences = false;
	String message;
	Location loc = parseToken.location;

	while (token != '\n' && token != -1)
	{
		if (token == FIXED_ATOM_ConstInt16 || token == FIXED_ATOM_ConstUint16 ||
			token == FIXED_ATOM_ConstInt || token == FIXED_ATOM_ConstUint ||
			token == FIXED_ATOM_ConstInt64 || token == FIXED_ATOM_ConstUint64 ||
			token == FIXED_ATOM_ConstFloat16 ||
			token == FIXED_ATOM_ConstFloat || token == FIXED_ATOM_ConstDouble)
		{
			message.Append(parseToken.name);
		}
		else if (token == FIXED_ATOM_Identifier || token == FIXED_ATOM_ConstString)
		{
			message.Append(parseToken.name);
		}
		else
		{
			message.Append(context.atomStrings.GetString(token));
		}

		message.Append(" ");
		token = ScanToken(parseToken);
	}

	Logger::Error("#error: {}", message);

	return '\n';
}

I32 ParseContext::ExtraTokenCheck(I32 contextAtom, ParseToken& parseToken, I32 token)
{
	if (token != '\n' && token != -1)
	{
		switch (contextAtom)
		{
		case FIXED_ATOM_Else: { Logger::Error("Unexpected '#else' Following Directive! Found On {}", parseToken.location); } break;
		case FIXED_ATOM_Elif: { Logger::Error("Unexpected '#elif' Following Directive! Found On {}", parseToken.location); } break;
		case FIXED_ATOM_Endif: { Logger::Error("Unexpected '#endif' Following Directive! Found On {}", parseToken.location); } break;
		case FIXED_ATOM_If: { Logger::Error("Unexpected '#if' Following Directive! Found On {}", parseToken.location); } break;
		default: { Logger::Error("Unexpected Token Following Directive! Found On {}", parseToken.location); } break;
		}

		while (token != '\n' && token != -1) { token = ScanToken(parseToken); }
	}

	return token;
}

void ParseContext::ReservedKeywordCheck(const Location& location, const String& identifier, const String& op)
{
	if (identifier.CompareN("defined", 8))
	{
		Logger::Error("'defined' Cannot Be Defined Or Undefined! Found On {}", location);
	}
}

I32 ParseContext::Eval(I32 token, I32 precedence, bool shortCircuit, I32& res, bool& err, ParseToken& parseToken)
{
	Location loc = parseToken.location;  // because we sometimes read the newline before reporting the error
	if (token == FIXED_ATOM_Identifier)
	{
		if (parseToken.name.Compare("defined"))
		{
			if (context.IsMacroInput())
			{
				Logger::Error("'defined' Cannot Be Used In Preprocessor Expressions When Expanded From Macros! Found On {}", parseToken.location);
			}
			bool needclose = 0;
			token = ScanToken(parseToken);
			if (token == '(')
			{
				needclose = true;
				token = ScanToken(parseToken);
			}
			if (token != FIXED_ATOM_Identifier)
			{
				Logger::Error("Incorrect Directive From Preprocessor Evaluation, Expected Identifier! Found On {}", loc);
				err = true;
				res = 0;

				return token;
			}

			MacroSymbol* macro = context.LookupMacroDef(context.atomStrings.GetAtom(parseToken.name));
			res = macro != nullptr ? !macro->undef : 0;
			token = ScanToken(parseToken);
			if (needclose)
			{
				if (token != ')')
				{
					Logger::Error("Expected ')' After Preprocessor Evaluation! Found On {}", loc);
					err = true;
					res = 0;

					return token;
				}
				token = ScanToken(parseToken);
			}
		}
		else
		{
			token = TokenPaste(token, parseToken);
			token = EvalToToken(token, shortCircuit, res, err, parseToken);
			return Eval(token, precedence, shortCircuit, res, err, parseToken);
		}
	}
	else if (token == FIXED_ATOM_ConstInt)
	{
		res = parseToken.ival;
		token = ScanToken(parseToken);
	}
	else if (token == '(')
	{
		token = ScanToken(parseToken);
		token = Eval(token, MIN_PRECEDENCE, shortCircuit, res, err, parseToken);
		if (!err)
		{
			if (token != ')')
			{
				Logger::Error("Expected ')' After Preprocessor Evaluation! Found On {}", loc);
				err = true;
				res = 0;

				return token;
			}
			token = ScanToken(parseToken);
		}
	}
	else
	{
		I32 op = CountOf32(unaryOps) - 1;
		for (; op >= 0; op--)
		{
			if (unaryOps[op].token == token)
				break;
		}
		if (op >= 0)
		{
			token = ScanToken(parseToken);
			token = Eval(token, UNARY, shortCircuit, res, err, parseToken);
			res = unaryOps[op].op(res);
		}
		else
		{
			Logger::Error("Bad Expression For Preprocessor Evaluation! Found On {}", loc);
			err = true;
			res = 0;

			return token;
		}
	}

	token = EvalToToken(token, shortCircuit, res, err, parseToken);

	// Perform evaluation of binary operation, if there is one, otherwise we are done.
	while (!err)
	{
		if (token == ')' || token == '\n')
			break;
		I32 op;
		for (op = CountOf32(binaryOps) - 1; op >= 0; op--)
		{
			if (binaryOps[op].token == token)
				break;
		}
		if (op < 0 || binaryOps[op].precedence <= precedence) { break; }
		I32 leftSide = res;

		if ((token == FIXED_ATOM_Or && leftSide == 1) || (token == FIXED_ATOM_And && leftSide == 0)) { shortCircuit = true; }

		token = ScanToken(parseToken);
		token = Eval(token, binaryOps[op].precedence, shortCircuit, res, err, parseToken);

		if (binaryOps[op].op == op_div || binaryOps[op].op == op_mod)
		{
			if (res == 0)
			{
				Logger::Error("Division By 0 In Preprocessor Evaluation! Found On {}", loc);
				res = 1;
			}
		}
		res = binaryOps[op].op(leftSide, res);
	}

	return token;
}

// Expand macros, skipping empty expansions, to get to the first real token in those expansions.
I32 ParseContext::EvalToToken(I32 token, bool shortCircuit, I32& res, bool& err, ParseToken& parseToken)
{
	while (token == FIXED_ATOM_Identifier && !parseToken.name.Compare("defined"))
	{
		switch (MacroExpand(parseToken, true, false))
		{
		case MACRO_EXPAND_RESULT_NOT_STARTED:
		case MACRO_EXPAND_RESULT_ERROR: {
			Logger::Error("Can't Evaluate Expression! Found On {}", parseToken.location);
			err = true;
			res = 0;
		} break;
		case MACRO_EXPAND_RESULT_STARTED: break;
		case MACRO_EXPAND_RESULT_UNDEF: { if (!shortCircuit) { Logger::Error("Undefined Macros Are Not Allowed In Expressions! Found On {}", parseToken.location); } } break;
		}
		token = ScanToken(parseToken);
		if (err)
			break;
	}

	return token;
}

MacroExpandResult ParseContext::MacroExpand(ParseToken& parseToken, bool expandUndef, bool newLineOkay)
{
	parseToken.space = false;
	int macroAtom = context.atomStrings.GetAtom(parseToken.name);
	if (parseToken.fullyExpanded) { return MACRO_EXPAND_RESULT_STARTED; }

	switch (macroAtom)
	{
	case FIXED_ATOM_LineMacro:
		// Arguments which are macro have been replaced in the first stage.
		if (parseToken.ival == 0) { parseToken.ival = inputReader.location.line; }

		parseToken.name = parseToken.ival;
		UngetToken(FIXED_ATOM_ConstInt, parseToken);
		return MACRO_EXPAND_RESULT_STARTED;

	case FIXED_ATOM_FileMacro: {
		parseToken.ival = inputReader.location.string;
		parseToken.name = parseToken.location.GetStringNameOrNum();
		UngetToken(FIXED_ATOM_ConstInt, parseToken);
		return MACRO_EXPAND_RESULT_STARTED;
	}

	case FIXED_ATOM_VersionMacro:
		parseToken.ival = 450; //TODO: Don't hardcode this
		parseToken.name = parseToken.ival;
		UngetToken(FIXED_ATOM_ConstInt, parseToken);
		return MACRO_EXPAND_RESULT_STARTED;

	default:
		break;
	}

	MacroSymbol* macro = macroAtom == 0 ? nullptr : context.LookupMacroDef(macroAtom);

	// no recursive expansions
	if (macro != nullptr && macro->busy)
	{
		parseToken.fullyExpanded = true;
		return MACRO_EXPAND_RESULT_NOT_STARTED;
	}

	// not expanding undefined macros
	if ((macro == nullptr || macro->undef) && !expandUndef) { return MACRO_EXPAND_RESULT_NOT_STARTED; }

	// 0 is the value of an undefined macro
	if ((macro == nullptr || macro->undef) && expandUndef)
	{
		Input input;
		input.done = false;
		input.type = INPUT_TYPE_ZERO;
		PushInput(input);
		return MACRO_EXPAND_RESULT_UNDEF;
	}

	Input in;
	in.done = false;
	in.type = INPUT_TYPE_MACRO;
	in.macro.prepaste = false;
	in.macro.postpaste = false;

	Location loc = parseToken.location;  // in case we go to the next line before discovering the error
	in.macro.symbol = macro;
	if (macro->functionLike)
	{
		// We don't know yet if this will be a successful call of a
		// function-like macro; need to look for a '(', but without trashing
		// the passed in parseToken, until we know we are no longer speculative.
		ParseToken parenToken;
		int token = ScanToken(parenToken);
		if (newLineOkay)
		{
			while (token == '\n')
				token = ScanToken(parenToken);
		}
		if (token != '(')
		{
			// Function-like macro called with object-like syntax: okay, don't expand.
			// (We ate exactly one token that might not be white space; put it back.
			UngetToken(token, parenToken);
			return MACRO_EXPAND_RESULT_NOT_STARTED;
		}
		in.macro.args.Resize(in.macro.symbol->macroArgs.Size());
		for (U64 i = 0; i < in.macro.symbol->macroArgs.Size(); ++i) { in.macro.args[i] = {}; }
		in.macro.expandedArgs.Resize(in.macro.symbol->macroArgs.Size());
		for (U64 i = 0; i < in.macro.symbol->macroArgs.Size(); ++i) { in.macro.expandedArgs[i] = nullptr; }
		U64 arg = 0;
		bool tokenRecorded = false;
		do
		{
			Vector<C8> nestStack;
			while (true)
			{
				token = ScanToken(parseToken);
				if (token == -1 || token == Input::marker)
				{
					Logger::Error("End Of Input In Macro! Found On {}", loc);
					return MACRO_EXPAND_RESULT_ERROR;
				}
				if (token == '\n')
				{
					if (!newLineOkay)
					{
						Logger::Error("End Of Line In Macro! Found On {}", loc);
						return MACRO_EXPAND_RESULT_ERROR;
					}
					continue;
				}
				if (token == '#')
				{
					Logger::Error("Unexpected '#'! Found On {}", loc);
					return MACRO_EXPAND_RESULT_ERROR;
				}
				if (in.macro.symbol->macroArgs.Size() == 0 && token != ')') { break; }
				if (nestStack.Size() == 0 && (token == ',' || token == ')')) { break; }
				if (token == '(') { nestStack.Push(')'); }
				else if (nestStack.Size() > 0 && token == nestStack.Back()) { nestStack.Pop(); }

				//Macro replacement list is expanded in the last stage.
				if (context.atomStrings.GetAtom(parseToken.name) == FIXED_ATOM_LineMacro)
					parseToken.ival = inputReader.location.line;

				in.macro.args[arg]->PutToken(token, parseToken);
				tokenRecorded = true;
			}
			// end of single argument scan

			if (token == ')')
			{
				// closing paren of call
				if (in.macro.symbol->macroArgs.Size() == 1 && !tokenRecorded)
					break;
				arg++;
				break;
			}
			arg++;
		} while (arg < in.macro.symbol->macroArgs.Size());
		// end of all arguments scan

		if (arg < in.macro.symbol->macroArgs.Size())
		{
			Logger::Error("Too Few Arguments In Macro! Found On {}", loc);
		}
		else if (token != ')')
		{
			// Error recover code; find end of call, if possible
			int depth = 0;
			while (token != -1 && (depth > 0 || token != ')'))
			{
				if (token == ')' || token == '}')
					depth--;
				token = ScanToken(parseToken);
				if (token == '(' || token == '{')
					depth++;
			}

			if (token == -1)
			{
				Logger::Error("End Of Input In Macro! Found On {}", loc);
				return MACRO_EXPAND_RESULT_ERROR;
			}
			Logger::Error("Too Many Arguments In Macro! Found On {}", loc);
		}

		// We need both expanded and non-expanded forms of the argument, for whether or
		// not token pasting will be applied later when the argument is consumed next to ##.
		for (U64 i = 0; i < in.macro.symbol->macroArgs.Size(); i++)
			in.macro.expandedArgs[i] = PrescanMacroArg(*in.macro.args[i], parseToken, newLineOkay);
	}

	PushInput(in);
	macro->busy = 1;
	macro->body.Reset();

	return MACRO_EXPAND_RESULT_STARTED;
}

I32 ParseContext::ScanHeaderName(ParseToken& parseToken, C8 delimit)
{
	bool tooLong = false;

	if (context.inputStack.Empty()) { return -1; }

	I32 len = 0;
	parseToken.name[0] = '\0';
	do
	{
		I32 ch = context.inputStack.Peek().Getch(inputReader);

		if (ch == delimit)
		{
			parseToken.name[len] = '\0';
			if (tooLong) { Logger::Error("Header Name Is Too Long! Found On {}", parseToken.location); }
			return FIXED_ATOM_ConstString;
		}
		else if (ch == -1) { return -1; }

		if (len < 1024) { parseToken.name[len++] = (C8)ch; }
		else { tooLong = true; }
	} while (true);
}

// Macro-expand a macro argument 'arg' to create 'expandedArg'.
// Does not replace 'arg'.
// Returns nullptr if no expanded argument is created.
TokenStream* ParseContext::PrescanMacroArg(TokenStream& arg, ParseToken& parseToken, bool newLineOkay)
{
	// expand the argument
	TokenStream* expandedArg = new TokenStream();

	Input input{};
	input.type = INPUT_TYPE_MARKER;

	PushInput(input);
	context.PushTokenStreamInput(arg);
	I32 token;
	while ((token = ScanToken(parseToken)) != Input::marker && token != -1)
	{
		token = TokenPaste(token, parseToken);
		if (token == FIXED_ATOM_Identifier)
		{
			switch (MacroExpand(parseToken, false, newLineOkay))
			{
			case MACRO_EXPAND_RESULT_NOT_STARTED: break;
			case MACRO_EXPAND_RESULT_ERROR: { while ((token = ScanToken(parseToken)) != Input::marker && token != -1); } break;
			case MACRO_EXPAND_RESULT_STARTED:
			case MACRO_EXPAND_RESULT_UNDEF: continue;
			}
		}
		if (token == Input::marker || token == -1) { break; }
		expandedArg->PutToken(token, parseToken);
	}

	if (token != Input::marker)
	{
		delete expandedArg;
		expandedArg = nullptr;
	}

	return expandedArg;
}

#endif