local tween = Yngin.Services.Tween
local input = Yngin.InputSystem

local tweenId = 0
local cycle = 0

local obj

local tweenSettings = TweenSettings.new()
tweenSettings.duration = 2.0
tweenSettings.tweenFunction = TWEEN_FUNCTION.EASE_INOUT

local raycastedMesh

function onReady()
	obj = Yngin.ScenesManager:getScene(0):getGameObjectsManager():getGameObject(1)
end

function onUpdate(dt)
	if raycastedMesh ~= nil then
		raycastedMesh:setColor(Vec3.new(1))
		raycastedMesh = nil
	end
	
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

	local ray = Ray.new()
	ray.origin = Vec3.new(-2, 0, -4)
	ray.direction = Vec3.new(0, 0, 1)
	local raycastedColl = Yngin.PhysicsEngine:raycast(Yngin.ScenesManager:getActive(), ray)

	if raycastedColl ~= nil then
		raycastedMesh = raycastedColl:getGameObject():getComponentMesh()
		raycastedMesh:setColor(Vec3.new(0, 1, 0))
	end
end
