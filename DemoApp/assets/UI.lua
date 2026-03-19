local switchScene
local text
local githubImg

local currentScene = 0
local textShown = 0

local testText = ""

local scenesMgr = Yngin.ScenesManager
local input = Yngin.InputSystem

function onReady()
	switchScene = Yngin.Context:getGlobalUIManager():getRootElement():getChildButton(2)
	text = Yngin.Context:getGlobalUIManager():getRootElement():getChildText(1)
	githubImg = Yngin.ScenesManager:getScene(0):getUIManager():getElement(2)
	
	for c = 0, 255 do
		if c % 16 == 0 then 
			testText = testText .. "\n" 
		end

		if c == string.byte("\n") or c == string.byte("\r") then
			testText = testText .. " "
		else
			testText = testText .. string.char(c)
		end
	end
end

function onUpdate(dt)
	if (switchScene ~= nil and switchScene:isClicked()) then
		currentScene = (currentScene + 1) % 2
		scenesMgr:setActive(currentScene)
	end

	if (input:isKeyJustPressed(KEY.NUM_1)) then
		textShown = (textShown + 1) % 2
	end

	if (text ~= nil) then
		if (textShown == 0) then
			local pos = scenesMgr:getActive():getCamerasManager():getCamera(0):getPosition()
		
			text:setText(string.format(
				"Yngin Demo\nFPS: %i\nPos: %i, %i, %i",
				math.floor(1 / dt + 0.5),
				math.floor(pos.x + 0.5),
				math.floor(pos.y + 0.5),
				math.floor(pos.z + 0.5)
			))
		elseif (textShown == 1) then
			text:setText(testText)
		end
	end

	if (githubImg:isHeld()) then
		githubImg:setColor(Vec4.new(0.5))
	elseif (githubImg:isHovered()) then
		githubImg:setColor(Vec4.new(0.75))
	else
		githubImg:setColor(Vec4.new(1.0))
	end

	if (githubImg:isClicked()) then
		os.execute("start https://github.com/YusufYaser/Yngin")
	end
end
