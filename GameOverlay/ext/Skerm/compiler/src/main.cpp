#include <skerm/compiler.hpp>
#include <skerm/tokeniser.hpp>

int main()
{
	static Skerm::Compiler compiler;
	compiler.AddFolder("data/asdf");
	return !compiler.CompileFile("data/index.ui", "D:/output/index.asdf");
}
