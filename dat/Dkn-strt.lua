-- NetHack DKnight Dkn-strt.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2025 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The Dark Knight quest - alternate quest for chaotic Knights.
--
--	The "start" level for the quest.
--
--	Here you meet your class leader and receive your quest assignment.
--

des.level_flags("mazelevel", "noteleport", "hardfloor")
des.level_init({ style = "mines", fg = ".", bg = "P", smoothed = false,
                 joined = true, lit = 0, walled = false })

--         1         2         3
--1234567890123456789012345678901234567
des.map({ halign = "center", valign = "center", map = [[
................TT....................
.--F--.T................TT......--F--.
.F...F.......TT.............T...F...F.
.--|+-------------++-------------+|--.
..T|..............................|...
...|-----------........-----------|T..
T..|..........|........|..........|..T
T..|....\.....+........+..........|...
...|..........|........|..........|T..
..T|-----------........-----------|T..
...|..............................|...
.--|+----------------------------+|--.
.F...F.....T......T.............F...F.
.--F--.TT.................T.....--F--.
..............T...............T.......
PPPPPPPPP.T..............TT..PPPPPPPPP
]] })
-- Dungeon Description
des.region({ region = { 00, 00, 37, 15 }, lit = 0, type = "ordinary" })
des.region({ region = { 04, 06, 13, 08 }, lit = 1, type = "throne", filled = 2 })
-- Portal arrival point
des.levregion({ region = { 18, 15, 18, 15 }, type = "branch" })
-- Stairs
des.stair("down", 32, 07)
-- Doors
-- Castle entrance
des.door("locked", 18, 03)
des.door("locked", 19, 03)
-- Tower entry doors
des.door("closed", 04, 03)
des.door("closed", 33, 03)
des.door("closed", 04, 11)
des.door("closed", 33, 11)
-- Chamber doors
des.door("closed", 14, 07)
des.door("closed", 23, 07)
-- Quest Leader
des.monster("Morgan le Fay", 08, 07)
-- Magic chest
des.feature("magic chest", 06, 07)
-- Corner tower guardians
des.monster({ id = "page", coord = { 03, 02 }, peaceful = 1 })
des.monster({ id = "page", coord = { 34, 02 }, peaceful = 1 })
des.monster({ id = "page", coord = { 03, 12 }, peaceful = 1 })
des.monster({ id = "page", coord = { 34, 12 }, peaceful = 1 })
-- Main chamber guardians
des.monster({ id = "page", coord = { 16, 07 }, peaceful = 1 })
des.monster({ id = "page", coord = { 21, 07 }, peaceful = 1 })
-- Throne room guardians
des.monster("page", 06, 06)
des.monster("page", 08, 06)
des.monster("page", 06, 08)
des.monster("page", 08, 08)
-- Non diggable walls
des.non_diggable(selection.area(00,00,37,15))
-- Random traps
for i = 1, 4 do
  des.trap()
end
-- Monsters on siege duty
des.monster({ id = "wolf", coord = { 14, 01 }, peaceful = 0 })
des.monster({ id = "large hawk", coord = { 16, 01 }, peaceful = 0 })
des.monster({ id = "wolf", coord = { 18, 01 }, peaceful = 0 })
des.monster({ id = "hawk", coord = { 20, 01 }, peaceful = 0 })
des.monster({ id = "wolf", coord = { 22, 01 }, peaceful = 0 })
des.monster({ id = "wolf", coord = { 14, 13 }, peaceful = 0 })
des.monster({ id = "hawk", coord = { 16, 13 }, peaceful = 0 })
des.monster({ id = "wolf", coord = { 18, 13 }, peaceful = 0 })
des.monster({ id = "hawk", coord = { 20, 13 }, peaceful = 0 })
des.monster({ id = "wolf", coord = { 22, 13 }, peaceful = 0 })
des.monster({ id = "large hawk", coord = { 01, 06 }, peaceful = 0 })
des.monster({ id = "stag", coord = { 01, 08 }, peaceful = 0 })
des.monster({ id = "large hawk", coord = { 36, 06 }, peaceful = 0 })
des.monster({ id = "grizzly bear", coord = { 36, 08 }, peaceful = 0 })
