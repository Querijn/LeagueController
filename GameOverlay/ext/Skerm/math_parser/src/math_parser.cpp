#include <skerm/math_parser.hpp>

#if __EMSCRIPTEN__

#include <emscripten.h>

extern "C"
{
	void SkermReduceEquationString(char* inInput)
	{
		printf("Attempting to reduce %s\n", inInput);
		auto result = Skerm::Tokenize(inInput);
		result->Equation.ReduceSelf();
		
		if (result->Errors.empty() == false)
		{
			for (auto& error : result->Errors)
				printf("[ERROR] %s\n", error.c_str());
			printf("'%s' -> '%s' (Potentially wrong due to errors)\n", inInput, result->Equation.ToString().c_str());
		}
		else
		{
			printf("'%s' -> '%s'\n", inInput, result->Equation.ToString().c_str());
		}

		free(inInput);
	}
}
#endif
