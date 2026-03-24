local input = Yngin.InputSystem
local window = Yngin.Window

function onUpdate(dt)
	window:setCursorLocked(input:isMousePressed(MOUSE_BUTTON.RIGHT))
	
	if (input:isMouseJustPressed(MOUSE_BUTTON.RIGHT)) then
		input:setMousePosition(IVec2.new(0))
	end
	
	local camera = Yngin.ScenesManager:getActive():getCamerasManager():getCamera(0)

	if (input:isMousePressed(MOUSE_BUTTON.RIGHT)) then
		local o = camera:getOrientation()
		local m = input:getMousePosition(true)

		local senstivity = 0.002

		local yaw = math.atan2(o.x, o.y)
		local pitch = math.asin(o.z)

		o.x = math.cos(pitch) * math.sin(yaw + m.x * senstivity)
		o.y = math.cos(pitch) * math.cos(yaw + m.x * senstivity)
		o.z = math.sin(math.max(math.min(pitch - m.y * senstivity, 1.57), -1.57))

		camera:setOrientation(o)

		input:setMousePosition(IVec2.new())

		local forward = o:normalize()
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

		local speed = 5.0
		if (input:isKeyPressed(KEY.LSHIFT)) then
			speed = speed * 1.5
		end

		camera:setPosition(camera:getPosition() + change * dt * speed)
	end
end
