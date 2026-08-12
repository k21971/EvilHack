-- NetHack Infidel Inf-fila.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2024 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The upper "fill" level for the quest, between the start
--	and locate levels.
--
--	It's the underground complex that leads to the Forest;
--	a standard room-and-corridor dungeon populated with hkO.
--

des.level_flags("hardfloor")

des.room({ type = "ordinary", contents = function()
   des.stair("up")
   des.object()
   des.monster("k")
end })

des.room({ type = "ordinary", contents = function()
   des.stair("down")
   des.object()
   des.trap()
   des.monster("h")
   des.monster("k")
end })

-- Quest dungeons always seem so empty, so here's a fountain.
des.room({ type = "ordinary", contents = function()
   des.feature("fountain")
   des.object()
   des.monster("h")
end })

-- Throne monsters roughly suit the hkO theme.
des.room({ type = "throne", chance = 25, contents = function()
   des.object()
   des.trap()
end })

des.room({ type = "ordinary", contents = function()
   des.object()
   des.trap()
   des.monster("k")
   des.monster("k")
end })

des.room({ type = "ordinary", contents = function()
   des.object()
   des.object()
   des.trap()
   des.trap()
   des.monster("O")
   des.monster("h")
end })

des.room({ type = "ordinary", contents = function()
   des.object()
   des.object()
   des.trap()
   des.trap()
   des.monster("O")
   des.monster("k")
end })

des.random_corridors()
