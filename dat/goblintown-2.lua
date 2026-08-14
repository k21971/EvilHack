-- NetHack gtown goblintown-2.lua	$NHDT-Date: 1781994884 2026/06/20 22:34:44 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2022 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
-- Gollum's Cave, below Goblin Town
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "hardfloor", "solidify")


--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
}}......p}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
.......pp}}}}}}}}}}..}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
..t...}}ppp}}}}}}}....}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
.....}}}}}ppp}}}}}}p..}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
.....}}}}}}pp}}}}}}pppp}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
p.....}}}}}}pp}}}}ppp}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
pp.....}}}}}}}ppppp}}}}}}}}}}}}}}}}}}}}}p}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
.......p}}}}}}}p}}}}}}}}}}}}}}}}}}}}}}}...}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
......p}}}}}}}}}p}}}}}}}}}}}}}}}}}}}}}..t...}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
.t....p}}}}}}}}}}p}}}}}}}}}}}}}}}}}...........}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
.t.....p}}}}}}}}}p}}}}}}}}}}}}}}}}........t..}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
.....pp}}}}}}}}}}ppp}}}}}}}}}}}}}}}}........}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
....}}}}}}}}}}}ppp}}}}}}}}}}}}}}}}}}}.....}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
p..}}}}}}}}}}}}}pppp}}}}}}}}}}}}}}}}}}...}}}}}}}}}}}}}}}}}}}}}}...}}}}}}}}}}
....}}}}}}}}}}}}}pp}}}}}}}}}}}}}}}}}}}.t..p}}}}}}}}}}}}}}}}}}...t..}}}}}}}}}
.....}}}}}}}}}}}}ppp}}}}}}}}}}}}}}}}}}}..pp}}}}}}}}}}}}}}}}}}}...p}}}}}}}}}}
.......}}}}}}}}}}}pppp}}}}}}}}}}}}}}}}}}pp}}}}}}}}}}}}}}}}}}}}}..p}}}}}}}}}}
........}}}}}}}}}}}}pppp}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}pp}}}}}}}}}}
..tt....}}}}}}}}}}}p}}}pp}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
...t.....}}}}}}}}}pppppp}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
.............pppppp}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
]] });
des.teleport_region({ region = { 00, 05, 07, 10 } })
des.stair("up", 02, 19)
des.region({ region = { 00, 00, 75, 20 }, lit = 0, type = "ordinary" })

-- random spots Gollum can spawn
local gollum_spawn = { {20,02},{01,13},{16,12},{34,10},{04,18},{02,01} }
shuffle(gollum_spawn);

-- Make the path to the center island a bit unpredictable
if percent(50) then
   des.terrain(selection.line(25,18, 28,18), "p")
   des.terrain(selection.line(28,17, 31,17), "p")
   des.terrain(selection.line(31,18, 33,18), "p")
   des.terrain(selection.line(34,19, 36,19), "p")
   des.terrain(selection.line(36,18, 39,18), "p")
   des.terrain(selection.line(39,17, 40,17), "p")
else
   des.terrain(selection.line(23,04, 29,04), "p")
   des.terrain(selection.line(29,03, 31,03), "p")
   des.terrain(selection.line(32,02, 35,02), "p")
   des.terrain(selection.line(36,03, 37,03), "p")
   des.terrain(selection.line(37,04, 38,04), "p")
   des.terrain(selection.line(38,05, 40,05), "p")
end

-- Small chance to jump across to the far island
if percent(33) then
   des.terrain(47, 09, "p")
   des.terrain(50, 09, "p")
   des.terrain(52, 11, "p")
   des.terrain(54, 11, "p")
   des.terrain(57, 11, "p")
   des.terrain(59, 13, "p")
   des.terrain(61, 13, "p")
end

-- various objects
des.object({ class = "=", id = "lustrous", buc = "cursed",
             name = "The One Ring" })

des.object({ id = "chest", coord = { 40, 10 }, contents = function()
   if percent(65) then
      des.object({ id = "goggles", buc = "uncursed" })
   end
   des.object("*")
   des.object("*")
   des.object("!")
end })

des.object({ id = "chest", trapped = 0, locked = 1, coord = { 64, 15 }, contents = function()
   if percent(35) then
      des.object("elven chain mail")
   end
   if percent(50) then
      des.object("elven cloak")
   end
   des.object("elven dagger")
   des.object({ id = "magic key", buc = "uncursed" })
   des.object("*")
   des.object("*")
   des.object("*")
   des.object({ class = "$", quantity = 200 })
end })

-- Gollum's victims
des.object({ id = "corpse", coord = { 01, 01 }, montype = "goblin" })
des.object({ id = "corpse", coord = { 03, 03 }, montype = "hobgoblin" })
des.object({ id = "corpse", coord = { 02, 07 }, montype = "piranha" })
des.object({ id = "corpse", coord = { 06, 07 }, montype = "piranha" })
des.object({ id = "corpse", coord = { 05, 08 }, montype = "piranha" })
des.object({ id = "corpse", coord = { 02, 16 }, montype = "goblin" })
des.object({ id = "corpse", coord = { 37, 10 }, montype = "hobgoblin" })
des.object({ id = "corpse", coord = { 39, 12 }, montype = "goblin" })
des.object({ id = "corpse", coord = { 43, 09 }, montype = "goblin" })
des.object({ id = "corpse", coord = { 63, 13 }, montype = "elf" })
des.object({ id = "corpse", coord = { 64, 16 }, montype = "elf" })

-- Gollum
des.monster({ id = "Gollum", coord = gollum_spawn[1], peaceful = 0 })

-- random monsters
des.monster({ id = "piranha", peaceful = 0 })
des.monster({ id = "piranha", peaceful = 0 })
des.monster({ id = "piranha", peaceful = 0 })
des.monster({ id = "jellyfish", peaceful = 0 })
des.monster({ id = "jellyfish", peaceful = 0 })
des.monster({ id = "bat", peaceful = 0 })
des.monster({ id = "bat", peaceful = 0 })
des.monster({ id = "bat", peaceful = 0 })
des.monster({ id = "bat", peaceful = 0 })
des.monster({ id = "bat", peaceful = 0 })
des.monster({ id = "giant bat", peaceful = 0 })
des.monster({ id = "giant bat", peaceful = 0 })