#pragma once
#include "Editor.h"

std::vector<EditorScript> defaultScripts = {
	{
		"Freecam",
		uint32_t(-1),
		R"(-- Yngin Default Freecam Script
local settings = {
	speed = 5.0,
	sensitivity = 0.002,
	invertY = false
}

local input = Yngin.InputSystem
local window = Yngin.Window

function onUpdate(dt)
	local scene = Yngin.ScenesManager:getActive()
	if scene == nil then return end
	local camera = scene:getCamerasManager():getCamera(0)
	if camera == nil then return end
	
	window:setCursorLocked(input:isMousePressed(MOUSE_BUTTON.RIGHT))
	
	if (input:isMouseJustPressed(MOUSE_BUTTON.RIGHT)) then
		input:setMousePosition(IVec2.new(0))
	end

	if (input:isMousePressed(MOUSE_BUTTON.RIGHT)) then
		local orientation = camera:getOrientation()
		local movement = input:getMousePosition(true)
		if settings.invertY then
			movement.y = -movement.y
		end

		local yaw = math.atan2(orientation.x, orientation.y)
		local pitch = math.asin(orientation.z)

		orientation.x = math.cos(pitch) * math.sin(yaw + movement.x * settings.sensitivity)
		orientation.y = math.cos(pitch) * math.cos(yaw + movement.x * settings.sensitivity)
		orientation.z = math.sin(math.max(math.min(pitch - movement.y * settings.sensitivity, 1.57), -1.57))

		camera:setOrientation(orientation)

		input:setMousePosition(IVec2.new())

		local forward = orientation:normalize()
		local right = forward:cross(Vec3.new(0, 0, 1)):normalize()
		local realUp = forward:cross(right)

		local change = Vec3.new()

		if (input:isKeyPressed(KEY.W)) then
			change = change + forward
		end
		if (input:isKeyPressed(KEY.S)) then
			change = change - forward
		end
		if (input:isKeyPressed(KEY.D)) then
			change = change + right
		end
		if (input:isKeyPressed(KEY.A)) then
			change = change - right
		end
		if (input:isKeyPressed(KEY.Q)) then
			change = change + realUp
		end
		if (input:isKeyPressed(KEY.E)) then
			change = change - realUp
		end

		local speed = settings.speed
		if (input:isKeyPressed(KEY.LSHIFT)) then
			speed = speed * 1.5
		end

		camera:setPosition(camera:getPosition() + change * dt * speed)
	end
end
)"
	}
};
