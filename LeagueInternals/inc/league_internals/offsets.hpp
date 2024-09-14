#pragma once

#include <league_internals/flag_duo.hpp>
#include <league_internals/game_object.hpp>

#include <memory_helper/address.hpp>
#include <league_controller/settings.hpp>

#include <spek/util/types.hpp>
#include <string>
#include <functional>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

namespace LeagueController
{
	struct ObjectManager;

	enum class GuiMenuLayerType : u8
	{
		None = 0x0,
		Chat = 0x5,
		Shop = 0x6
	};

	struct GuiMenu
	{
		union
		{
			DEFINE_MEMBER_N(GuiMenuLayerType openedLayer, 0x8);
		};
	};

	template<typename T>
	struct RiotVector
	{
		T* begin;
		int count;
		int capacity;
	};

	struct VisibleObject
	{
		void* sceneGraph;

		union
		{
			DEFINE_MEMBER_N(GameObject* gameObject, 0x4);
		};
	};

	struct GameWorld
	{
		union
		{
			DEFINE_MEMBER_N(void* sceneGraph, 0x30); // TODO: investigate?
			DEFINE_MEMBER_N(RiotVector<VisibleObject*> visibleObjects, 0x34);
		};
	};

	struct Manager
	{
		void* vtable;
		RiotVector<GameObject*> items;
	};

	struct OffsetData : public AddressData
	{
		Manager* attackableManager = nullptr;
		Manager* heroManager = nullptr;
		void* hudManager = nullptr;

		glm::mat4* viewMatrix = nullptr;
		glm::mat4* projMatrix = nullptr;

		GameObject*	underMouseObject = nullptr;
		GameObject* localPlayer = nullptr;
		f32* gameTime = nullptr;
		GuiMenu* guiMenu = nullptr;

		template<typename Serialiser>
		bool ForEachOffset(Serialiser serialiser)
		{
			if (serialiser(attackableManager) == false) return false;
			if (serialiser(heroManager) == false) return false;
			if (serialiser(hudManager) == false) return false;
			if (serialiser(viewMatrix) == false) return false;
			if (serialiser(projMatrix) == false) return false;
			if (serialiser(underMouseObject) == false) return false;
			if (serialiser(localPlayer) == false) return false;
			if (serialiser(gameTime) == false) return false;
			if (serialiser(guiMenu) == false) return false;
			return true;
		}

		template<typename Serialiser>
		void ForEachAddressedOffset(AddressData& addressData, Serialiser serialiser)
		{
			serialiser(attackableManager);
			serialiser(heroManager);
			serialiser(hudManager);
			serialiser(viewMatrix);
			serialiser(projMatrix);
			serialiser(underMouseObject);
			serialiser(localPlayer);
			serialiser(gameTime);
			serialiser(guiMenu);
		}
	};

	bool AreOffsetsLoaded();
	void LoadOffsets();
	void OnOffsetsLoaded(std::function<void()> callback);

	std::string GetMissingAddresses();
	bool IsInGame();
	bool HasGameTime();
	float GetGameTime();
	GameObject* GetLocalPlayer();
	glm::mat4* GetViewMatrix();
	glm::mat4* GetProjMatrix();
	GameObject* GetSelectedGameObject();
	GuiMenu* GetGuiMenu();
	Manager* GetAttackableManager();
	Manager* GetHeroManager();
	bool HasMouseWorldPosition();
	bool GetMouseWorldPosition(glm::vec3& outPosition);
	u32 GetImageBase();
}
