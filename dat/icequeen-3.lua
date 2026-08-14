-- NetHack iceq icequeen-3.lua	$NHDT-Date: 1786716969 2026/08/14 14:16:09 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 2020 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--	The final level for this branch.
--
--	The player discovers the ice castle of
--	Kathryn the Ice Queen. Can they defeat her
--	before winter permanently sets in?
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "icedpools",
                "shortsighted", "cold")
--         1         2         3         4         5         6         7    7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map({ halign = "left", valign = "center", map = [[
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIII                                                      II
IIIIIIIIIIIIIIIIIIII -----------  -------   ------   -------  ----------- II
IIIIIIIIIIIIIIIIIIIII|..F......|  |.....|  -|....|-  |.....|  |.........| II
IIIIIIIIIIIIIIIIIIIIIF..|---...----.....---|......|---.....----...---...| II
IIIIIIIIIIIIIIIIIIIIIF..S..................+......+.....................|  I
IIIIIIIIIIIIIIIIIIIIIF..|---...------------|......|------------....------- I
IIIIIIIIIIIIIIIIIIIII|..F......|...........-|....|-...........|....|.....| I
IIIIIIIIIIIIIIIIIIII -FF-...----............|-FF-|............|....|.....| I
IIIIIIIIIIIIIIIIIIIIIIII+...|..F............S....+............+....S.....| I
IIIIIIIIIIIIIIIIIIIIIIII+...|..F............S....+............+....S.....| I
IIIIIIIIIIIIIIIIIIII -FF-...----............|-FF-|............|....|.....| I
IIIIIIIIIIIIIIIIIIIII|..F......|...........-|....|-...........|....|.....| I
IIIIIIIIIIIIIIIIIIIIIF..|---...------------|......|------------....------- I
IIIIIIIIIIIIIIIIIIIIIF..S..................+......+.....................|  I
IIIIIIIIIIIIIIIIIIIIIF..|---...----.....---|......|---.....----...---...| II
IIIIIIIIIIIIIIIIIIIII|..F......|  |.....|  -|....|-  |.....|  |.........| II
IIIIIIIIIIIIIIIIIIII -----------  -------   ------   -------  ----------- II
II.IIIIIIIIIIIIIIIII                                                      II
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
]] });
-- random monster spawn spots
local spawn = { {29,05},{29,14},{37,04},{37,15},
                {56,04},{56,15},{64,05},{64,14} }
shuffle(spawn);
des.region({ region = { 00, 00, 75, 19 }, lit = 1, type = "ordinary" })
-- the ice caves ('zoo' rooms) and the shrine are engine-stocked
des.region({ region = { 45, 03, 48, 07 }, lit = 0, type = "zoo", filled = 1 })
des.region({ region = { 45, 12, 48, 16 }, lit = 0, type = "zoo", filled = 1 })
des.region({ region = { 51, 07, 61, 12 }, lit = 0, type = "zoo", filled = 1 })
des.region({ region = { 68, 07, 72, 12 }, lit = 1, type = "temple",
             filled = 1 })
-- Stairs
des.stair("up", 02, 18)

-- Magic chest
des.feature("magic chest", 72, 10)

-- Evil altar
des.altar({ coord = { 70, 10 }, align = "noalign", type = "shrine" })

-- Castle entrance
des.door("locked", 24, 09)
des.door("locked", 24, 10)

-- Doors
des.door("locked", 43, 05)
des.door("locked", 43, 14)
des.door("locked", 44, 09)
des.door("locked", 44, 10)
des.door("locked", 49, 09)
des.door("locked", 49, 10)
des.door("locked", 50, 05)
des.door("locked", 50, 14)
des.door("locked", 62, 09)
des.door("locked", 62, 10)
des.door("locked", 67, 09)
des.door("locked", 67, 10)

-- Traps
des.trap("spear", 23, 09)
des.trap("spear", 23, 10)
des.trap("ice")
des.trap("ice")
des.trap("ice")
des.trap("ice")
des.trap("ice")
des.trap("ice")

-- Objects and terrain
des.object({ class = "$", coord = { 29, 09 }, quantity = 2000 })
des.object({ class = "$", coord = { 29, 10 }, quantity = 2000 })
des.object({ class = "$", coord = { 30, 09 }, quantity = 2000 })
des.object({ class = "$", coord = { 30, 10 }, quantity = 2000 })
des.terrain(34, 09, "\\")
if percent(20) then
  des.object({ id = "egg", coord = { 29, 10 }, montype = "pegasus" })
end
des.object("crystal chest", 32, 07)
des.object("crystal chest", 32, 12)
des.object("crystal chest", 42, 07)
des.object({ id = "crystal chest", coord = { 42, 12 }, contents = function()
  des.object({ id = "apple", quantity = 3 })
  des.object({ id = "carrot", quantity = 3 })
  des.object({ id = "saddle", buc = "uncursed" })
  des.object({ id = "acid", quantity = 3 })
  if percent(20) then
    if percent(80) then
      des.object({ id = "spiked barding", buc = "uncursed" })
    else
      des.object({ id = "barding of reflection", buc = "uncursed" })
    end
  end
end })
des.object({ id = "crystal chest", coord = { 29, 10 }, contents = function()
  if percent(10) then
    des.object({ id = "magic marker", buc = "uncursed" })
  end
  des.object({ id = "bag of holding", buc = "uncursed", name = "Bag of the Hesperides" })
end })
des.feature("fountain", 37, 08)
des.feature("fountain", 37, 11)

-- Random monsters
des.monster({ coord = spawn[1] })
des.monster({ coord = spawn[2] })
des.monster({ coord = spawn[3] })
des.monster({ coord = spawn[4] })
des.monster({ coord = spawn[5] })
des.monster({ coord = spawn[6] })
des.monster({ coord = spawn[7] })
des.monster({ coord = spawn[8] })
des.monster()
des.monster()
des.monster()
des.monster()

-- Out front
des.monster("woolly mammoth", 18, 09)
des.monster("frost giant", 18, 10)
des.monster("saber-toothed tiger", 19, 09)

-- Watch towers
des.monster({ id = "ice troll", coord = { 22, 05 },
              keep_default_invent = true, inventory = function()
  des.object({ id = "orcish bow", buc = "uncursed" })
  des.object({ id = "orcish arrow", buc = "uncursed", quantity = 100 })
end })
des.monster({ id = "ice troll", coord = { 22, 14 },
              keep_default_invent = true, inventory = function()
  des.object({ id = "orcish bow", buc = "uncursed" })
  des.object({ id = "orcish arrow", buc = "uncursed", quantity = 100 })
end })

-- Outside the main zoo and inside the temple
des.monster("ice devil", 65, 09)
des.monster("ice devil", 65, 10)
des.monster("ice devil", 70, 07)
des.monster("ice devil", 70, 12)

-- Main zoo augment
des.monster("mastodon", 50, 08)
des.monster("ice troll", 50, 09)
des.monster("cave bear", 50, 10)
des.monster("woolly mammoth", 50, 11)

-- Mini-foyer
des.monster("snow golem", 47, 09)
des.monster("snow golem", 47, 10)

-- The Ice Queen and her pets
des.monster({ id = "Kathryn the Ice Queen", coord = { 34, 09 }, peaceful = 0 })
des.monster({ id = "Bourbon", coord = { 37, 07 }, peaceful = 1, asleep = 1 })
des.monster({ id = "Ozzy", coord = { 37, 12 }, peaceful = 1, asleep = 1 })

-- Koa tribute. Miss you.
des.grave(34, 10, "Koa: July 25, 2009 - November 29th, 2020. Rest in peace Koa, we miss you")

-- The prize
des.monster({ id = "pegasus", coord = { 29, 10 }, peaceful = 1, asleep = 1 })

-- None shall pass
des.non_diggable(selection.area(00,00,75,19))
des.non_passwall(selection.area(00,00,75,19))
