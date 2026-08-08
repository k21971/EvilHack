-- NetHack gehennom hellb-1.lua	$NHDT-Date: 1432512783 2015/05/25 00:13:03 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
-- Previously Yeenoghu's realm
--
des.level_init({ style = "mines", fg = ".", bg = "}", smoothed = false,
                 joined = true, lit = 1, walled = false });

des.level_flags("mazelevel", "hardfloor")
--         1         2
--1234567890123456789012
des.map({ halign = "right", valign = "center", map = [[
}}}p}}ppppp}}}}}}}}}}}
}---------------p}}p}}
p|..|...|..|...|..p}}}
.-S----+--+-+--|.p}p}}
.|..+..........|..p}}}
.|..|..........|.p}p}}
.----+------S---.p}p}}
.+.......|.....|..pp}}
.+.......|..\..|.p}}p}
.+.......|.....|..pp}}
.----+------S---..p}}}
.|..|..........|.p}p}}
.|..+..........|..p}p}
.-S----+--+-+---..p}}}
p|..|...|..|...|.p}p}}
}---------------ppp}}}
}}p}ppppp}p}pp}}}}}}}}
]], contents = function()
   des.levregion({ region = { 1, 1, 19, 19 }, region_islev = 1,
                   exclude = { 1, 1, 1, 1 }, exclude_islev = 1,
                   type = "stair-up" })
   des.levregion({ region = { 1, 1, 19, 19 }, region_islev = 1,
                   exclude = { 1, 1, 1, 1 }, exclude_islev = 1,
                   type = "branch" })
   des.teleport_region({ region = { 1, 1, 19, 19 }, region_islev = 1,
                         exclude = { 1, 1, 1, 1 }, exclude_islev = 1 })
   des.stair("down", 14, 8)
   -- Doors
   des.door({ state = "locked", material = "bronze", x = 7, y = 3 })
   des.door({ state = "locked", material = "bronze", x = 10, y = 3 })
   des.door({ state = "locked", material = "bronze", x = 12, y = 3 })
   des.door({ state = "locked", material = "bronze", x = 4, y = 4 })
   des.door({ state = "locked", material = "bronze", x = 5, y = 6 })
   des.door({ state = "locked", material = "bronze", x = 1, y = 7 })
   des.door({ state = "locked", material = "bronze", x = 1, y = 8 })
   des.door({ state = "locked", material = "bronze", x = 1, y = 9 })
   des.door({ state = "locked", material = "bronze", x = 5, y = 10 })
   des.door({ state = "locked", material = "bronze", x = 4, y = 12 })
   des.door({ state = "locked", material = "bronze", x = 7, y = 13 })
   des.door({ state = "locked", material = "bronze", x = 10, y = 13 })
   des.door({ state = "locked", material = "bronze", x = 12, y = 13 })
   -- Secret doors
   des.door({ state = "secret", material = "bronze", x = 2, y = 3 })
   des.door({ state = "secret", material = "bronze", x = 2, y = 13 })
   des.door({ state = "secret", material = "bronze", x = 12, y = 6 })
   des.door({ state = "secret", material = "bronze", x = 12, y = 10 })
   --
   des.region({ region = { 0, 0, 21, 16 }, lit = 1, type = "ordinary" })
   des.region({ region = { 10, 7, 14, 9 }, lit = 1, type = "throne",
                filled = 2 })
   des.non_diggable(selection.area(1, 1, 15, 15))
   -- random demon boss
   if percent(50) then
      if percent(50) then
         des.monster("Mephistopheles", 12, 8)
         -- minions
         des.monster({ id = "hell hound", coord = { 11, 8 }, peaceful = 0 })
         des.monster({ id = "hell hound", coord = { 13, 8 }, peaceful = 0 })
         des.monster({ id = "horned devil", coord = { 11, 7 }, peaceful = 0 })
         des.monster({ id = "horned devil", coord = { 11, 9 }, peaceful = 0 })
         des.monster({ id = "barbed devil", coord = { 13, 7 }, peaceful = 0 })
         des.monster({ id = "barbed devil", coord = { 13, 9 }, peaceful = 0 })
         des.monster({ class = "i", coord = { 10, 8 }, peaceful = 0 })
         des.monster({ class = "i", coord = { 14, 12 }, peaceful = 0 })
      else
         des.monster("Yeenoghu", 12, 8)
         -- minions
         des.monster({ id = "gnoll", coord = { 11, 8 }, peaceful = 0 })
         des.monster({ id = "gnoll", coord = { 13, 8 }, peaceful = 0 })
         des.monster({ id = "gnoll hunter", coord = { 11, 7 }, peaceful = 0 })
         des.monster({ id = "gnoll hunter", coord = { 11, 9 }, peaceful = 0 })
         des.monster({ id = "gnoll cleric", coord = { 13, 7 }, peaceful = 0 })
         des.monster({ id = "flind", coord = { 13, 9 }, peaceful = 0 })
         des.monster({ id = "gnoll", coord = { 10, 8 }, peaceful = 0 })
         des.monster({ id = "gnoll", coord = { 14, 12 }, peaceful = 0 })
      end
   else
      if percent(50) then
         des.monster("Dispater", 12, 8)
         -- minions
         des.monster({ id = "hell hound", coord = { 11, 8 }, peaceful = 0 })
         des.monster({ id = "hell hound", coord = { 13, 8 }, peaceful = 0 })
         des.monster({ id = "Olog-hai", coord = { 11, 7 }, peaceful = 0 })
         des.monster({ id = "Olog-hai", coord = { 11, 9 }, peaceful = 0 })
         des.monster({ id = "Olog-hai", coord = { 13, 7 }, peaceful = 0 })
         des.monster({ id = "Olog-hai", coord = { 13, 9 }, peaceful = 0 })
         des.monster({ class = "G", coord = { 10, 8 }, peaceful = 0 })
         des.monster({ class = "G", coord = { 14, 12 }, peaceful = 0 })
      else
         des.monster("Lolth", 12, 8)
         -- minions
         des.monster({ id = "drider", coord = { 11, 8 }, peaceful = 0 })
         des.monster({ id = "drider", coord = { 13, 8 }, peaceful = 0 })
         des.monster({ id = "drow warrior", coord = { 11, 7 }, peaceful = 0 })
         des.monster({ id = "drow warrior", coord = { 11, 9 }, peaceful = 0 })
         des.monster({ id = "drow cleric", coord = { 13, 7 }, peaceful = 0 })
         des.monster({ id = "gargantuan spider", coord = { 13, 9 },
                       peaceful = 0 })
         des.monster({ class = "s", coord = { 10, 8 }, peaceful = 0 })
         des.monster({ class = "s", coord = { 14, 12 }, peaceful = 0 })
      end
   end
end });

-- the .des switches to NOMAP here: everything below is placed
-- level-wide, not map-relative
-- slaves
for i = 1, 10 do
   des.monster("@")
end
-- other denizens
for i = 1, 5 do
   des.monster({ class = "&", peaceful = 0 })
end
for i = 1, 10 do
   des.monster({ class = "i", peaceful = 0 })
end
for i = 1, 5 do
   des.monster({ class = "T", peaceful = 0 })
end
for i = 1, 5 do
   des.monster({ class = "d", peaceful = 0 })
end
for i = 1, 2 do
   des.monster({ class = "G", peaceful = 0 })
end
for i = 1, 3 do
   des.monster({ class = " ", peaceful = 0 })
end
-- the throne room loot
des.object("iron safe", 13, 8)
-- some wasteland-ish stuff
for i = 1, 5 do
   des.object("boulder")
end
-- other loot
des.object("[")
des.object("[")
des.object(")")
des.object(")")
des.object("(")
des.object("/")
des.object("=")
des.object("?")
des.object("?")
des.object("!")
des.object("!")
for i = 1, 8 do
   des.object("*")
end
-- traps
for i = 1, 10 do
   des.trap("fire")
end
for i = 1, 3 do
   des.trap("magic")
end
for i = 1, 5 do
   des.trap()
end
