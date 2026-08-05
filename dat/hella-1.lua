-- NetHack gehennom hella-1.lua	$NHDT-Date: 1781994882 2026/06/20 22:34:42 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.7 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--

des.level_flags("mazelevel", "hardfloor", "shortsighted", "noflip", "temperate")
des.level_init({ style = "solidfill", fg = ".", lit = 0 });
des.terrain(selection.fillrect(0,0,78,20):percentage(33), "}")
des.terrain(selection.fillrect(0,0,78,20):percentage(15), "s")
-- guarantee at least one open spot to ensure successful stair placement
des.map({ halign = "left", valign = "bottom", map = [[
xxxxxxxx
xxpppxxx
xxx..pxx
xxxxpxxx
xxxxxxxx
]] });
des.object("boulder")
des.map({ halign = "right", valign = "top", map = [[
xxxxxxxx
xxxxpxxx
xxx..pxx
xxpppxxx
xxxxxxxx
]] });
des.object("boulder")
-- lair
des.map([[
xx}}}}}x}}ss}x}ss}}x}}}}}x}}}}}x}}}}}x}}}}}x}}}}}xx
x}}}p}}}}}ss}}}ss}}}}}pp}}}pp}}}}}pp}}}pp}}}}}p}}}x
}}}p.p}}pp}}s}s}}s}}p}}}ppp}}}p}}}pp}}}pp}}}}p.p}}}
x}}}p}}p}}}s}}s}}s}}ppp}}p}}ppppp}}ppppp}pppp}p}}}x
xx}}}pp}}}s}}s}}s}}pp}}ppppp}}p}}}p}}p}}}}}}}}}}}xx
x}}}pp}}}}}s}}s}}s}}ppp}}}}}ppppp}}p}}}}}}ppppp}}}x
}}}pp}}ppp}}ss}}s}}}p}}}ppp}}}p}}}p}p}}}}p.P.P.p}}}
}}p}}}}ppp}}}}ss}ppp}}}p.P.p}}}p}p}}}p}}}}p...p}}}}
}p}}}}p}}p}ssss}}}}}}}p.P.P.p}}}p}}}p}}pp}}p.p}}}}x
x}}}}p}}}}p..s}}}}}p}}}p.P.p}}}p}}}}p}}pp}}ppp}}}p}
}}}}pp}}p}}sss}}}ppp}}}}ppp}}}p}}}}}p}}}}p}}}}}}p}}
}}}p.p}}pps}}sss}}}}}}}}}}}}pp.pp}}}}p}}ppp}pp}p}}}
x}}}pp}}p}}}}s..s}}pp}}}pp}}pp.pp}}}}p}}}p}p..p}}}x
xx}}}p}}}}pp}}sss}pp}}pp}}pp}}p}}}pp}p}pp}}}pp}}}xx
x}}}p}}}}p..p}}}sss}}p..p}}}}}}}ppp}}}p..p}}}}p}}}x
}}}p.p}}}p..p}}}ss}}}p..p}}}pp}}ppp}}}p..p}}}ppp}}}
x}}}p}}}}}pp}}}ss}}}}}pp}}}pp}}}}}pp}}}pp}}}}}p}}}x
xx}}}}}x}}}}}x}s}}}x}}}}}x}}}}}x}}}}}x}}}}}x}}}}}xx
]]);
-- Random registers
local monster = { "j","b","P","F" }
shuffle(monster)

local place = selection.new();
place:set(04,02);
place:set(46,02);
place:set(04,15);
place:set(46,15);

-- Dungeon description
des.region({ region={00,00,50,17}, lit=0, type="swamp", filled=2 })
des.mazewalk(0, 9, "west")
des.mazewalk(50, 8, "east")
des.levregion({ region = {01,00,11,20}, region_islev=1, exclude={0,0,50,17}, type="stair-down" });
des.levregion({ region = {69,00,79,20}, region_islev=1, exclude={0,0,50,17}, type="stair-up" });
des.levregion({ region = {01,00,11,20}, region_islev=1, exclude={0,0,50,17}, type="branch" });
des.teleport_region({ region = {01,00,11,20}, region_islev=1, exclude={0,0,50,17},dir="up" })
des.teleport_region({ region = {69,00,79,20}, region_islev=1, exclude={0,0,50,17},dir="down" })
des.feature("fountain", place:rndcoord(1))
des.monster({ id = "giant mimic", coord = place:rndcoord(1), appear_as = "ter:fountain" })
des.monster({ id = "giant mimic", coord = place:rndcoord(1), appear_as = "ter:fountain" })
des.monster({ id = "giant mimic", coord = place:rndcoord(1), appear_as = "ter:fountain" })
-- random demon boss
if percent(50) then
   if percent(50) then
      des.monster("Juiblex", 25, 8)
   else
      des.monster("Baalzebub", 25, 8)
   end
else
   if percent(50) then
      des.monster("Geryon", 25, 8)
   else
      des.monster("Baphomet", 25, 8)
   end
end
-- And a couple demons
des.monster("lemure",43,08)
des.monster("lemure",44,08)
des.monster("lemure",45,08)
-- Some liquids and gems
des.object("*",43,06)
des.object("*",45,06)
des.object("!",43,09)
des.object("!",44,09)
des.object("!",45,09)
-- And lots of blobby monsters
des.monster(monster[4],25,06)
des.monster(monster[1],24,07)
des.monster(monster[2],26,07)
des.monster(monster[3],23,08)
des.monster(monster[3],27,08)
des.monster(monster[2],24,09)
des.monster(monster[1],26,09)
des.monster(monster[4],25,10)
des.monster("j")
des.monster("j")
des.monster("j")
des.monster("j")
des.monster("P")
des.monster("P")
des.monster("P")
des.monster("P")
des.monster("b")
des.monster("b")
des.monster("b")
des.monster("F")
des.monster("F")
des.monster("F")
des.monster("m")
des.monster("m")
des.monster("jellyfish")
des.monster("jellyfish")
des.monster("creeping mound", 12, 2)
des.monster("giant leech", 13, 6)
des.monster("giant leech", 11, 10)
des.monster("creeping mound", 16, 14)
des.monster("giant cockroach")
des.monster("giant cockroach")
des.monster("giant cockroach")
-- Some random objects
des.object("!")
des.object("!")
des.object("!")
des.object("%")
des.object("%")
des.object("%")
des.object("boulder")
-- Some traps
des.trap("sleep gas")
des.trap("sleep gas")
des.trap("anti magic")
des.trap("anti magic")
des.trap("magic")
des.trap("magic")
