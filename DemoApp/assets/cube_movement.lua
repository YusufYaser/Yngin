local tween = Yngin.Services.Tween
local input = Yngin.InputSystem

local tweenId = 0
local cycle = 0

local obj

local tweenSettings = TweenSettings.new()
tweenSettings.duration = 2.0
tweenSettings.tweenFunction = TWEEN_FUNCTION.EASE_INOUT

function onReady()
	obj = Yngin.ScenesManager:getScene(0):getGameObjectsManager():getGameObject(1)
end

function onUpdate(dt)
	if not tween:isActive(tweenId) then
		cycle = (cycle + 1) % 4
		local pos = Vec3.new(0, 0, -2)

		if cycle <= 1 then
			pos.x = 1.5
		else
			pos.x = -1.5
		end

		if ((cycle + 3) % 4) <= 1 then
			pos.y = 1.5
		else
			pos.y = -1.5
		end

		tweenId = tween:tweenPos(obj, pos, tweenSettings)
	end

	if input:isKeyJustPressed(KEY.SPACE) then
		tween:setPaused(tweenId, not tween:isPaused(tweenId))
	end
end
