#include "lua_wrapper.hpp"
#include "script_math.hpp"
#include "setup.hpp"

#include <game_overlay/log.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

namespace LeagueController
{
	extern GameOverlay::LogCategory g_scriptLog;

	template<typename vec>
	vec Add(const vec& left, const vec& right)
	{
		return left + right;
	}

	template<typename vec>
	vec Sub(const vec& left, const vec& right)
	{
		return left - right;
	}

	template<typename vec>
	vec Mul(const vec& left, const vec& right)
	{
		return left * right;
	}

	template<typename vec, typename ftype>
	vec AddNum(const vec& left, ftype right)
	{
		return left + (f32)right;
	}

	template<typename vec, typename ftype>
	vec SubNum(const vec& left, ftype right)
	{
		return left - (f32)right;
	}

	template<typename vec, typename ftype>
	vec MulNum(const vec& left, ftype right)
	{
		return left * (f32)right;
	}

	template<typename vec, typename ftype>
	vec MulNum2(ftype left, const vec& right)
	{
		return (f32)left * right;
	}

	template<typename vec, typename ftype>
	vec AddNum2(ftype left, const vec& right)
	{
		return (f32)left + right;
	}

	template<typename vec, typename ftype>
	vec SubNum2(ftype left, const vec& right)
	{
		return (f32)left - right;
	}

	template<typename vec>
	f64 Length2(const vec& input)
	{
		return (f64)glm::length2(input);
	}

	template<typename vec>
	f64 Length(const vec& input)
	{
		return (f64)glm::length(input);
	}

	template<typename vec>
	void Normalize(vec& input)
	{
		input = glm::normalize(input);
	}

	template<typename vec, typename constructors>
	sol::usertype<vec> Setup(const char* name, sol::state& state)
	{
		Leacon_Profiler;
		return state.new_usertype<vec>
		(
			name,
			constructors(),

			"x", &vec::x,
			"y", &vec::y,
			"length2", sol::property(&Length2<vec>),
			"length", sol::property(&Length<vec>),
			"normalize", &Normalize<vec>,

			sol::meta_function::to_string,		[](const vec& inVec) -> std::string { return glm::to_string(inVec); },
			sol::meta_function::addition,		sol::overload(&Add<vec>, &AddNum<vec, f32>, &AddNum2<vec, f32>, &AddNum<vec, f64>, &AddNum2<vec, f64>),
			sol::meta_function::subtraction,	sol::overload(&Sub<vec>, &SubNum<vec, f32>, &SubNum2<vec, f32>, &SubNum<vec, f64>, &SubNum2<vec, f64>),
			sol::meta_function::multiplication,	sol::overload(&Mul<vec>, &MulNum<vec, f32>, &MulNum2<vec, f32>, &MulNum<vec, f64>, &MulNum2<vec, f64>)
		);
	}

	void AddMathScriptSystem(sol::state& state)
	{
		Leacon_Profiler;
		using Vec2Constructors = sol::constructors<glm::vec2(), glm::vec2(float, float), glm::vec2(const glm::vec2&)>;
		using Vec3Constructors = sol::constructors<glm::vec3(), glm::vec3(float, float, float), glm::vec3(const glm::vec3&)>;

		Setup<glm::vec2, Vec2Constructors> ("Vec2", state);
		auto vec3 = Setup<glm::vec3, Vec3Constructors>("Vec3", state);
		vec3["z"] = &glm::vec3::z;

		state.set_function("CastPassableRay", [](const glm::vec3& origin, const glm::vec3& dir, float length)
		{
			glm::vec3 outPos;
			bool hasHit = GetNavGrid()->CastRay2DPassable(origin, dir, length, outPos);
			return std::make_tuple(hasHit, outPos);
		});

		state.set_function("Dot", [](const glm::vec3& first, const glm::vec3& second)
		{
			return glm::dot(first, second);
		});
	}
	
	void DestroyScriptMath()
	{
		Leacon_Profiler;

	}
}