local Shop = {};

KeyboardImpl =	require("keyboard");

OPENED_ON_FOUNTAIN = false;

function Shop.Init()
	
end

function Shop.Update(localPlayer)
	if IsShopOpen() then
		-- if opened on fountain, close shop if not on fountain
		if OPENED_ON_FOUNTAIN and Fountain.IsOnFountain(localPlayer) == false then
			ToggleShop();
			OPENED_ON_FOUNTAIN = false;
		end

		-- Once you walk into the fountain, leave the shop on leaving the fountain
		if Fountain.IsOnFountain(localPlayer) then
			OPENED_ON_FOUNTAIN = true;
		end
	end

	SetLizardMode(IsShopOpen());
end

return Shop;