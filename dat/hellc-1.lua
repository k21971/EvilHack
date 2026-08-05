-- NetHack gehennom hellc-1.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
-- Previously Demogorgon's towers
des.level_init({ style = "mines", fg = ".", bg = "}", smoothed = true,
                 joined = true, lit = 1, walled = false });

des.level_flags("mazelevel", "hardfloor", "shortsighted")
des.map([[
}}s}}}}ss}}s}}s}}s}}}}}}}}}}}}}}}}}}}}}}}}}s}s}ss}}ss}}ss}ss
}ss}s}sss}sss}ss}s}}}s}}}}}}}}}}}}}s}}}s}}s}sss}ss}s}}ss}ss}
ss---------------}}}}}}s}}}s}}}}}}}}}}}}}s}---------------}s
}}|.............|ss}}}}}}}}}}}}}}}}}}}}}s}}|....|.|...|..|s}
ss|S-----------.|s}s}}}}}}ss}}}s}}}}}}}}}ss|....S.|.|.S..|ss
}s|...........|.|}}}}s}}}}}}}}}s}}}s}}}sss}|....|.|.----S|s}
s}|...........|.|ssss}}}}}}}}}}}}}}}}}}}}ss|....|...|....|ss
}s----------S--S-}}s}}}}}}}}}}}}}}}}}}}}}}s----S-------S--}s
ss|..|--...|.|..|}}}}}}}}}}}}}}}}}}}}}}}}}}|..|.|...--...|s}
ss|....S...|.|..|}########################}|..|.|...S....Ss}
}s|..|--...|.|..|}}}}}}}}}}}}}}}}}}}}}}}}}}|..|.|...--...|ss
s}--S-------S----}}}}}}}}}}}}}}}}}}}}}}}}}}-S--S----------}s
}s|....|...|....|}s}}}}}}}}}}}}}s}}}}}}}sss|.|...........|s}
}}|S----.|.|....|s}}s}}}ss}}}}}}}}}s}}}s}}s|.|...........|ss
ss|..S.|.|.S....|s}s}}}}}}}}ss}}}}}}}}}}}}}|.-----------S|}s
ss|..|...|.|....|}s}}}}}}}}}}}}}s}}}}}}}}}}|.............|}}
}s---------------s}}}ssss}}}}}}}}}}}}}}}}ss---------------ss
sss}s}sss}sss}ss}s}}}}}s}}}}}}}}}}}}}s}}}}s}sss}ss}s}sss}ss}
}}}ss}}sss}s}}s}s}}}}}}}}}}}}}}}}}}}}s}}}}}}}}ss}}s}ss}}}s}}
]]);
-- from above, land in the left tower; from below, land in the right tower
des.teleport_region({ region = { 43, 2, 57, 16 }, dir = "up" })
des.teleport_region({ region = { 2, 2, 16, 16 }, dir = "down" })
des.region({ region = { 0, 0, 59, 18 }, lit = 1, type = "ordinary" })
des.region({ region = { 2, 2, 16, 16 }, lit = 0, type = "ordinary" })
des.region({ region = { 43, 2, 57, 16 }, lit = 0, type = "ordinary" })
-- can't dig in the towers
des.non_diggable(selection.area(2,2,16,16))
des.non_diggable(selection.area(43,2,58,16))
-- bridge between the two towers
des.drawbridge({ x = 17, y = 9, dir = "west", state = "closed" })
des.drawbridge({ x = 42, y = 9, dir = "east", state = "closed" })
-- stairs in the two towers
des.stair("up", 9, 9)
des.stair("down", 50, 9)
-- Secret doors
for _, c in ipairs({ {3,4},{3,13},{4,11},{5,14},{7,9},{12,7},{12,11},{15,7},
                     {44,11},{47,7},{47,11},{48,4},{52,9},{54,4},{55,7},
                     {56,5},{56,14} }) do
   des.door({ state = "secret", material = "bronze", x = c[1], y = c[2] })
end
-- chance of a secret way into the lair
if percent(50) then
   des.door({ state = "secret", material = "bronze", x = 57, y = 9 })
else
   des.terrain(57, 9, "|")
end
-- random demon boss
if percent(50) then
   if percent(50) then
      des.monster({ id = "Demogorgon", coord = { 50, 9 }, peaceful = 0 })
      -- entourage
      des.monster({ id = "nalfeshnee", coord = { 49, 8 }, peaceful = 0 })
      des.monster({ id = "balrog", coord = { 51, 8 }, peaceful = 0 })
      des.monster({ id = "nalfeshnee", coord = { 49, 10 }, peaceful = 0 })
      des.monster({ id = "balrog", coord = { 51, 10 }, peaceful = 0 })
      des.monster({ id = "pit fiend", coord = { 50, 8 }, peaceful = 0 })
      des.monster({ id = "elder minotaur", coord = { 49, 9 }, peaceful = 0 })
      des.monster({ id = "pit fiend", coord = { 50, 10 }, peaceful = 0 })
   else
      des.monster({ id = "Tiamat", coord = { 50, 9 }, peaceful = 0 })
      -- entourage - chaotic aligned dragons
      des.monster({ id = "blue dragon", coord = { 49, 8 }, peaceful = 0 })
      des.monster({ id = "green dragon", coord = { 51, 8 }, peaceful = 0 })
      des.monster({ id = "shadow dragon", coord = { 49, 10 }, peaceful = 0 })
      des.monster({ id = "black dragon", coord = { 51, 10 }, peaceful = 0 })
      des.monster({ id = "red dragon", coord = { 50, 8 }, peaceful = 0 })
      des.monster({ id = "balrog", coord = { 49, 9 }, peaceful = 0 })
      des.monster({ id = "red dragon", coord = { 50, 10 }, peaceful = 0 })
   end
else
   if percent(50) then
      des.monster({ id = "Asmodeus", coord = { 50, 9 }, peaceful = 0 })
      -- entourage
      des.monster({ id = "marilith", coord = { 49, 8 }, peaceful = 0 })
      des.monster({ id = "balrog", coord = { 51, 8 }, peaceful = 0 })
      des.monster({ id = "ice devil", coord = { 49, 10 }, peaceful = 0 })
      des.monster({ id = "balrog", coord = { 51, 10 }, peaceful = 0 })
      des.monster({ id = "weredemon", coord = { 50, 8 }, peaceful = 0 })
      des.monster({ id = "horned devil", coord = { 49, 9 }, peaceful = 0 })
      des.monster({ id = "marilith", coord = { 50, 10 }, peaceful = 0 })
   else
      des.monster({ id = "Graz'zt", coord = { 50, 9 }, peaceful = 0 })
      -- entourage
      des.monster({ id = "marilith", coord = { 49, 8 }, peaceful = 0 })
      des.monster({ id = "balrog", coord = { 51, 8 }, peaceful = 0 })
      des.monster({ id = "nalfeshnee", coord = { 49, 10 }, peaceful = 0 })
      des.monster({ id = "balrog", coord = { 51, 10 }, peaceful = 0 })
      des.monster({ id = "pit fiend", coord = { 50, 8 }, peaceful = 0 })
      des.monster({ id = "barbed devil", coord = { 49, 9 }, peaceful = 0 })
      des.monster({ id = "pit fiend", coord = { 50, 10 }, peaceful = 0 })
   end
end
-- and some other denizens
for i = 1, 8 do
   des.monster({ class = "&", peaceful = 0 })
end
for i = 1, 16 do
   des.monster({ class = "i", peaceful = 0 })
end
for i = 1, 5 do
   des.monster({ id = "kraken", peaceful = 0 })
end
for i = 1, 10 do
   des.monster({ id = "giant eel", peaceful = 0 })
end
for i = 1, 5 do
   des.monster({ id = "electric eel", peaceful = 0 })
end
for i = 1, 10 do
   des.monster({ id = "shark", peaceful = 0 })
end
des.monster("giant leech", 1, 9)
des.monster("creeping mound", 17, 14)
des.monster("giant leech", 18, 6)
des.monster("creeping mound", 41, 5)
des.monster("giant leech", 41, 12)
des.monster("giant leech", 58, 9)
des.monster("creeping mound", 59, 4)
des.monster("giant leech", 59, 14)
-- a bit of loot
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
-- some traps
for i = 1, 6 do
   des.trap("fire")
end
des.trap("ice")
des.trap("ice")
des.trap("magic")
des.trap("magic")
des.trap("magic beam")
des.trap("spear")
des.trap("spear")
