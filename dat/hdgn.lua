-- NetHack hdgn hdgn.lua	$NHDT-Date: 1786752137 2026/08/15 00:02:17 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2023 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--	The Hidden Dungeon -
--
--	EvilHack content: Tal'Gath, beholder, magical eye, prisoner,
--	shambling horror, assassin vine, gulthias tree, creeping mound,
--	skeleton warrior, revenant; crystal chest; iron door material;
--	sewage terrain; nursery and barracks region types.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "shortsighted",
                "nommap")

--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
------------ssssssssssss......sssssss--------sss|...-----------ss..---------
|ssssss..ss||sss|sssss....|.......ssss..s..sssss|.s.|...|.|.|.|.ss.|.|.|.|.|
|ss...ssssss|sss|sss...sss|sss......ss.ss....sss|...-FFF-F-F-F-.ss.-F-F-F-F-
|sssssssssss|sssssss...ssssssssss....ss...s.ssss|.ss..s..........s......ss..
|s.ssssss..--------s..sss-----ssss...--------sss|.....sss.......ss........s.
|..ssssssss+......|...sssss|sssssss.....ssssssss|...-----------..ss.---..---
||sssss----|......|..sssssss|ssssss....sss|sssss|..s|.........|..s..|.F..F.|
s-------sss|......+sssss|sssssssss...sss---sssss|.ss+.........|...s.|-|..|-|
sssssssssss|-------sss--|ssssssss..sssssssssssss|.ss+.....-S--|...s.|.F..F.|
ss|ssssssss|ssssssssssssss--sss...ssssssssss|sss|ss.|.....|...F...ss|-|s.|-|
ss---ssssssssss----ssssssssssss..sssss|sssss---s|...|.....|...F....s|.Fs.F.|
sssssss---sssss|sssssss|ssss....ssssss|ss|ssssss|...--------FF-.....|-|..|-|
ssssssssssssssssssssss--ss...sssssssssss--sss...+.s..sssss......s...|.F.sF.|
sssssss....ssss............sssss--ssssssssss....+........sss........---.s---
--ss...............|.....ssssssssssss|sssss...ss|s..-----------.........ss..
sss....ssss....ss---sssssss|sssssss---sssss..sss|...|.|.|.|.|.|.sss.........
ss...ssss|ssssssssssssssssss|sssssssssssss...sss|...-F-F-F-F-F-..sss.....sss
..|..ssss|ssssssssss....ssss|sssssssssss....ssss|s....ss.s..s......s..ssssss
.--........ssss..................sss......ssssss|s..-F-F-F-F-F-......ssss...
.......ss.........sssssssssssss..........ss--sss|ss.|.|.|.|.|.|.....sss.....
......ssssssssssssss----ssssssssssssssssssssssss|.ss-----------..ssssssss...
]] });
des.teleport_region({ region = { 73, 18, 73, 19 } })
des.region({ region = { 00, 00, 75, 20 }, lit = 0, type = "ordinary",
             arrival_room = true })

-- random wandering monster locations
local corpse = { {59,01},{68,01},{74,01},{74,10},
            {69,06},{69,08},{55,15},{59,19},
            {61,15},{51,10},{66,18} }
shuffle(corpse);

local sewer = { {16,03},{08,09},{05,16},{24,02},
           {25,07},{18,19},{34,00},{35,10},
           {30,16},{46,05},{43,12},{46,17} }
shuffle(sewer);

local beholder = { {06,06},{14,11},{17,01},{36,14},
              {44,20},{51,18},{66,06} }
shuffle(beholder);

-- Portal
des.levregion({ region = {74,19,74,19}, type="branch" })

-- various room types
des.region({ region = { 53, 06, 57, 10 }, lit = 0, type = "barracks",
             filled = 2, irregular = 1 })
des.region({ region = { 12, 05, 17, 07 }, lit = 0, type = "nursery",
             filled = 1 })

-- doors
des.door({ state = "locked", material = "iron", x = 11, y = 05 })
des.door({ state = "locked", material = "iron", x = 18, y = 07 })
des.door({ state = "locked", material = "iron", x = 48, y = 12 })
des.door({ state = "locked", material = "iron", x = 48, y = 13 })
des.door({ state = "locked", material = "iron", x = 52, y = 07 })
des.door({ state = "locked", material = "iron", x = 52, y = 08 })

-- boss monster + loot
des.monster("Tal'Gath", 01, 01)
des.object({ id = "crystal chest", coord = { 01, 01 }, contents = function()
   des.object("!")
   des.object("!")
   des.object("?")
   des.object("+")
   des.object("\"")
   des.object("=")
   des.object("=")
   if percent(15) then
      des.object("magic marker")
   end
end })

-- monsters
des.monster("mastodon", 54, 01)
des.monster("shambling horror", 60, 10)
des.monster("prisoner", 61, 01)
des.monster("prisoner", 74, 12)
des.monster("prisoner", 53, 19)
des.monster("assassin vine", 67, 15)
des.monster({ id = "giant cockroach", coord = sewer[1] })
des.monster({ id = "giant cockroach", coord = sewer[2] })
des.monster({ id = "giant cockroach", coord = sewer[3] })
des.monster({ id = "gulthias tree", coord = sewer[4] })
des.monster({ id = "giant leech", coord = sewer[5] })
des.monster({ id = "giant leech", coord = sewer[6] })
des.monster({ id = "creeping mound", coord = sewer[7] })
des.monster({ id = "creeping mound", coord = sewer[8] })
des.monster({ id = "crocodile", coord = sewer[9] })
des.monster({ id = "giant crocodile", coord = sewer[10] })
des.monster({ id = "troll", coord = sewer[11] })
des.monster({ id = "basilisk", coord = sewer[12] })
des.monster({ id = "beholder", coord = beholder[1], asleep = 0 })
des.monster({ id = "beholder", coord = beholder[2], asleep = 0 })
des.monster({ id = "beholder", coord = beholder[3], asleep = 0 })
des.monster({ id = "beholder", coord = beholder[4], asleep = 0 })
des.monster({ id = "magical eye", coord = beholder[5] })
des.monster({ id = "magical eye", coord = beholder[6] })
des.monster("revenant")
des.monster("revenant")
des.monster("skeleton warrior")

-- random corpses
des.object({ id = "corpse", coord = corpse[1], montype = "archeologist" })
des.object({ id = "corpse", coord = corpse[2], montype = "barbarian" })
des.object({ id = "corpse", coord = corpse[3], montype = "caveman" })
des.object({ id = "corpse", coord = corpse[4], montype = "convict" })
des.object({ id = "corpse", coord = corpse[5], montype = "healer" })
des.object({ id = "corpse", coord = corpse[6], montype = "infidel" })
des.object({ id = "corpse", coord = corpse[7], montype = "priestess" })
des.object({ id = "corpse", coord = corpse[8], montype = "ranger" })
des.object({ id = "corpse", coord = corpse[9], montype = "rogue" })
des.object({ id = "corpse", coord = corpse[10], montype = "valkyrie" })
des.object({ id = "corpse", coord = corpse[11], montype = "wizard" })

-- traps
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap("ice")
des.trap("falling rock")
des.trap("falling rock")
des.trap("falling rock")
des.trap("falling rock")

-- none shall pass
des.non_diggable(selection.area(00,00,58,20))
des.non_passwall(selection.area(00,00,18,08))
