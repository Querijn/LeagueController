KeyboardImpl	= require("keyboard");
HotkeyBinding	= require("hotkey_binding");
Movement 		= require("movement");
Mouse 			= require("mouse");
Shop 			= require("shop");
Potion			= require("potion"); -- Needed for the potion hotkey called from C++

-- Log("Script has loaded!");
FIRST_FRAME = true;
LOADED_UI = LoadFullscreenUI("data/ui/loaded.ui");
HIDE_UI_AT_TIME = -1;

function Init()
	Mouse.Init();
	KeyboardImpl.Init();

	LOADED_UI.visible = false;

	local width = 300;
	local height = 30;

	LOADED_UI.width = width;
	LOADED_UI.height = math.floor(height);

	LOADED_UI.x = 0;
	LOADED_UI.y = 0;
	Log("LOADED_UI is shown at " .. tostring(LOADED_UI.x) .. ", " .. tostring(LOADED_UI.y) .. " (" .. tostring(LOADED_UI.width) .. ", " .. tostring(LOADED_UI.height) .. ")");
end

function Frame()
	if IsGameReady() == false then
		-- Log("Game is NOT ready");
		return;
	end
	
	if FIRST_FRAME then
		-- LOADED_UI.visible = true;
		HIDE_UI_AT_TIME = GetGameTime() + 5;
		FIRST_FRAME = false;
		Log("First frame, showing Loaded UI for 5 seconds..");

	elseif GetGameTime() > HIDE_UI_AT_TIME and HIDE_UI_AT_TIME > 0 then
		LOADED_UI.visible = false;
		HIDE_UI_AT_TIME = -1;
	end

	local localPlayer = GetLocalPlayer();
	if localPlayer == nil and not IsApp() then
		Log("No players were found");
		return;
	end

	local bindingInProgress = HotkeyBinding.Update(localPlayer);
	KeyboardImpl.Update(localPlayer, bindingInProgress);
	local isMoving = Movement.Update(localPlayer);
	Mouse.UpdatePos(localPlayer, isMoving);
	Shop.Update(localPlayer);
end