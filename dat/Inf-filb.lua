-- NetHack Infidel Inf-filb.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2024 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The lower "fill" level for the quest, between the locate
--	and goal levels.
--
--	These are some nondescript areas of the Howling Forest.
--	There's a lot of werewolves and other d and perhaps a woodchuck.
--

des.level_flags("mazelevel", "hardfloor")
des.level_init({ style = "mines", fg = ".", bg = "T", smoothed = true,
                 joined = true, lit = 0, walled = true })

des.levregion({ region = { 45, 00, 79, 20 }, region_islev = 1,
                type = "stair-up" })
des.levregion({ region = { 00, 00, 34, 20 }, region_islev = 1,
                type = "stair-down" })

des.monster("werewolf")
des.monster("werewolf")
des.monster("werewolf")
des.monster("werewolf")
des.monster("werewolf")
for i = 1, d(6, 4) do
   des.monster("d")
end
des.monster("u")
des.monster("u")
des.monster("u")
des.monster("u")
des.monster("forest centaur")
if percent(60) then
   des.monster("woodchuck")
end

des.object()
des.object()
des.object()
des.object()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
