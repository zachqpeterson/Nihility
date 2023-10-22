#include "ShaderCompiler.hpp"

#if 0

ParseContext ShaderCompiler::context;

void ShaderCompiler::Compile()
{
	bool def = false;
	I32 lookahead = TOKEN_TYPE_EMPTY;
	I32 state = 0;
	I32 length = 0;
	StateType value;
	SymbolType token = SYMBOL_TYPE_EMPTY;

	I16 stateStack[200];
	I16* stateBottom = stateStack;
	I16* stateTop = stateStack;

	StateType semanticStack[200];
	StateType* semanticBottom = semanticStack;
	StateType* semanticTop = semanticStack;

	while (state != FINAL_STATE)
	{
		*stateTop = (I16)state;

		U64 size = stateTop - stateBottom;
		if (size == 200)
		{
			//TODO: Reallocate
			BreakPoint;
		}

		I16 num = PACT[state];

		if (num == PACT_INF) { def = true; }
		else
		{
			if (lookahead == TOKEN_TYPE_EMPTY)
			{
				lookahead = context.NextToken(value);
			}

			if (lookahead <= TOKEN_TYPE_EOF)
			{
				lookahead = TOKEN_TYPE_EOF;
				token = SYMBOL_TYPE_EOF;
			}
			else if (lookahead <= TOKEN_TYPE_ERROR)
			{
				lookahead = TOKEN_TYPE_UNDEF;
				token = SYMBOL_TYPE_ERROR;
				/*TODO: Detect error function*/
			}
			else
			{
				token = (lookahead >= 0 && lookahead < MAX_TOKEN ? (SymbolType)TRANSLATE[lookahead] : SYMBOL_TYPE_UNDEF);
			}

			num += token;
			if (!(num < 0 || num > LAST_INDEX || CHECK[num] != token))
			{
				num = TABLE[num];
				if (num <= 0)
				{
					num = -num;
				}
				else
				{
					state = num;
					*++semanticTop = value;

					lookahead = TOKEN_TYPE_EMPTY;
					++stateTop;
					continue;
				}
			}
			else { def = true; }
		}

		if (def)
		{
			num = DEFACT[state];
			if (num == 0) { /*TODO: Detect error function*/ }
		}

		//Reduce
		length = R2[num];

		//TODO: Ultra switch statement
		switch (num)
		{
		case 2: {

		} break;
		}


		stateTop -= length;
		semanticTop -= length - 1;
		length = 0;

		*semanticTop = value;

		I32 lhs = R1[num] - TOKEN_COUNT;
		I32 i = PACT_GOTO[lhs] + *stateTop;
		state = (i >= 0 && i <= LAST_INDEX && CHECK[i] == *stateTop ? TABLE[i] : DEFACT_GOTO[lhs]);

		++stateTop;
	}

	//TODO: Cleanup
}

#endif