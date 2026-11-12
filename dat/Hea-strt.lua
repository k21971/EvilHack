-- $NHDT-Header: NetHack 5.0 Hea-strt.lua 2026/06/20_22:35:38 NetHack-5.0:1.5 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991, 1993 by M. Stephenson, P. Winner
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader, Hippocrates
--	and receive your quest assignment.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor")

des.map([[
PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
PPPPp......pPPPPp...pPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPpPppPPPPPp....pPPPPPPPP
PPPp........pPPPPpppPPPPPpppppppppppppppppppppppppPPPPppPPPPPp......pPPPPPPP
PPp..........pPPPPPPPPp............................pPPP...PPPPp....pPPPPPPPP
PpppppPPPPPPPPPPPPPPPp..............................pPPPPPPPPPPPPPPPPPPPPPPP
PPPPppppPPPPPPPPPPPPp.................................pPPPPPpPPPPPPPPPPPPPPP
PPPPp......pPPPPPp........-----------------------.......pPPpppPPPPPPPp...pPP
PPPp..........pPPPPPp...--|.|......S..........S.|--....pPPPPpPPPPPPPp.....pP
PPPPp........pPPPPPp....|.S.|......-----------|S|.|.....pPPPPPPpPPPp.....pPP
PPPPPPp....pPPPPPPp.....|.|.|......|...|......|.|.|....pPPPPPPpppPPp.....pPP
PPPPPPPPPPPPPPPPPPPp....+.|.|......S.\.S......|.|.+.....pPPPPPPpPPPPp.....pP
PPPpppPPPPPpppPPPPp.....|.|.|......|...|......|.|.|......pPPPPPPPPPPPp...pPP
PPp...pPPPp...pPPPp.....|.|S|-----------......|.S.|.....pPPPPPPPPPPPPPPPPPPP
PPPppPPPPPpppPPPPp......--|.S..........S......|.|--....pPPPPPPPPPppppPPPPPPP
PPPPPPPPPPPPPPPPp.........-----------------------.........pPPPPPp........pPP
PPPPPPPPPPPPPPPPPp......................................pPPPPPPp..........pP
PPPp...........pPPPPp.................................pPPP..PPPPp........pPP
PPp.............pPPPPPppppppppppppppppppppppppppppppppPPPP...PPPPp......pPPP
PPPp...........pPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPp..pPPPPPP
PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
]]);

des.replace_terrain({ region={01,01, 74,18}, fromterrain="P", toterrain=".", chance=10 })

-- Dungeon Description
des.region(selection.area(00,00,75,19), "lit")
-- Stairs
des.stair("down", 37,9)
-- Portal arrival point
des.levregion({ region = {04,12,04,12}, type="branch" })
-- altar for the Temple
des.altar({ x=32,y=09,align="neutral",type="altar" })
-- Doors
des.door("locked",24,10)
des.door("closed",26,08)
des.door("closed",27,12)
des.door("locked",28,13)
des.door("closed",35,07)
des.door("locked",35,10)
des.door("locked",39,10)
des.door("closed",39,13)
des.door("locked",46,07)
des.door("closed",47,08)
des.door("closed",48,12)
des.door("locked",50,10)
-- Hippocrates
des.monster({ id = "Hippocrates", coord = {37, 10}, inventory = function()
   des.object({ id = "dagger", spe = 5 });
end })
-- Magic chest
des.feature("magic chest", 31, 09)
-- intern guards for the audience chamber
des.monster("attendant", 29, 08)
des.monster("attendant", 29, 09)
des.monster("attendant", 29, 10)
des.monster("attendant", 29, 11)
des.monster("attendant", 40, 09)
des.monster("attendant", 40, 10)
des.monster("attendant", 40, 11)
des.monster("attendant", 40, 13)
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,19))
-- Random traps
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
-- Monsters on siege duty.
des.monster("rabid rat")
des.monster("rabid rat")
des.monster("rabid rat")
des.monster("rabid rat")
des.monster("rabid rat")
des.monster("rabid rat")
des.monster("rabid rat")
des.monster("rabid rat")
des.monster("enormous rat")
des.monster("enormous rat")
des.monster("giant eel")
des.monster("shark")
des.monster(";")
des.monster({ class = "D", peaceful=0 })
des.monster({ class = "D", peaceful=0 })
des.monster({ class = "D", peaceful=0 })
des.monster({ class = "D", peaceful=0 })
des.monster({ class = "D", peaceful=0 })
des.monster({ class = "S", peaceful=0 })
des.monster({ class = "S", peaceful=0 })
des.monster({ class = "S", peaceful=0 })
des.monster({ class = "S", peaceful=0 })
des.monster({ class = "S", peaceful=0 })
