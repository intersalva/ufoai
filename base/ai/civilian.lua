--[[
Copyright (C) 2002-2014 UFO: Alien Invasion.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
--]]

local ai = ai

function ai.hidetus ()
	return ai.actor():TU() - 4
end

function ai.flee()
	local flee_pos = ai.positionflee()
	if flee_pos then
		return flee_pos:goto()
	end
	return false
end

function ai.hide (team)
	local hide_pos = ai.positionhide(team)
	if hide_pos then
		return hide_pos:goto()
	end
	return false
end

function ai.herd (targets)
	if #targets > 0 then
		for i = 1, #targets do
			local herd_pos = ai.positionherd(targets[i])
			if herd_pos then
				return herd_pos:goto()
			end
		end
	end
	return false
end

function ai.approach (targets)
	for i = 1, #targets do
		for j = 1, 2 do
			local near_pos = ai.positionapproach(targets[i], ai.hidetus(), j == 1)
			if near_pos then
				return near_pos:goto()
			end
		end
	end
	return false
end

function ai.route (waypoints)
	if #waypoints > 0 then
		for i = 1, #waypoints do
			local target_pos = ai.positionmission(waypoints[i])
			if target_pos then
				if target_pos:goto() then
					ai.setwaypoint(waypoints[i])
				end
			end
		end
		-- Can't get to any waypoints, try to approach the nearest one
		return ai.approach(waypoints)
	end
end

function ai.wander ()
	local next_pos = ai.positionwander("rand", (ai.actor():TU() + 1) / 6)
	if next_pos then
		next_pos:goto()
	end
end

function ai.think_nf ()
	local aliens = ai.see("sight", "alien")
	if #aliens > 0 then
		if not ai.hide(aliens[1]:team()) then
			ai.flee()
		end
	else
		local waypoints = ai.waypoints(25, "path")
		if #waypoints > 0 then
			ai.route(waypoints)
		else
			ai.setwaypoint()
			local phalanx = ai.see("sight", "phalanx")
			if #phalanx > 0 then
				ai.herd(phalanx)
			else
				local civs = ai.see("sight", "civilian")
				if #civs > 0 then
					ai.herd(civs)
				else
					ai.wander()
				end
			end
		end
	end
	aliens = ai.see("sight", "alien")
	if #aliens > 0 then
		-- Some civ models don't have crouching animations
		-- ai.crouch(true)
	end
end

function think ()
	if not ai.isfighter() then
		ai.think_nf()
	else
		-- We don't currently have fight capable civ teams, so no point in implementing this yet
		ai.print("Warning: no fight capable lua ai available for civilian ", ai.actor())
		ai.think_nf()
	end
end
