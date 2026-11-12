-- $NHDT-Header: NetHack 5.0 tower1.lua 2026/06/20_22:35:38 NetHack-5.0:1.9 $
--	Copyright (c) 1989 by Jean-Christophe Collet
-- NetHack may be freely redistributed.  See license for details.
--
--
-- Upper stage of Vlad's tower
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "solidify")
--         1         2         3
--123456789012345678901234567890
des.map({ halign = "half-left", valign = "center", map = [[
  --- ---   ------   --- ---  
  |.| |.|  -|....|-  |.| |.|  
---S---S---|--SS--|---S---S---
|..........S......|.........\|
|..........|......|..........|
----....---|--++--|----++-----
   |....|  |......|  |....|   
   |....|  |......|  |....|   
   |....|  |......|  |....|   
   |....|  |......|  |....|   
-----++----|--++--|---....----
|..........|......|..........|
|..........|......S..........|
---S---S---|--SS--|---S---S---
  |.| |.|  -|....|-  |.| |.|  
  --- ---   ------   --- ---  
]] });

-- Random 8 niches
local niches = { {03,01},{07,01},{22,01},{26,01},
                 {03,14},{07,14},{22,14},{26,14} }
shuffle(niches);

des.ladder("down", 01, 12)
-- The lord and his court
-- Vlad's makemon kit already carries the Candelabrum of Invocation;
-- keep it rather than re-creating the unique item here
des.monster({ id = "Vlad the Impaler", coord = { 28, 03 },
              keep_default_invent = true,
              inventory = function()
                 des.object({ id = "two-handed sword",
                              spe = math.random(1, 3),
                              name = "Lifestealer",
                              buc = "cursed", eroded = -1 })
              end
})
des.monster({ class = "V", coord = niches[1] })
des.monster({ class = "V", coord = niches[2] })
des.monster({ class = "V", coord = niches[3] })
-- The brides; they weren't named in Bram Stoker's original _Dracula_
-- and when appearing in umpteen subsequent books and movies there is
-- no consensus for their names.  According to the Wikipedia entry for
-- "Brides of Dracula", the "Czechoslovakian TV film Hrabe Drakula (1971)"
-- gave them titles rather than (or perhaps in addition to) specific names
-- and we use those titles here.  Marking them as 'waiting' forces them to
-- start in vampire form instead of vampshifted into bat/fog/wolf form.
local Vgenod = nh.is_genocided("vampire noble");
local Vnames = { nil, nil, nil };
if (not Vgenod) then
   Vnames = { "Madame", "Marquise", "Countess" };
end
des.monster({ id = "vampire noble", female = 1, coord = niches[4],
              name = Vnames[1], waiting = 1 })
des.monster({ id = "vampire noble", female = 1, coord = niches[5],
              name = Vnames[2], waiting = 1 })
des.monster({ id = "vampire noble", female = 1, coord = niches[6],
              name = Vnames[3], waiting = 1 })
des.monster({ class = "e", coord = niches[7] })
des.monster({ class = "&", coord = niches[8] })
des.monster("shambling horror", 14, 01)
des.monster("shambling horror", 15, 14)
des.monster("skeleton warrior", 13, 07)
des.monster("skeleton warrior", 13, 08)
des.monster("vampire royal", 14, 07)
des.monster("vampire mage", 14, 08)
des.monster("vampire royal", 15, 07)
des.monster("vampire mage", 15, 08)
des.monster("skeleton warrior", 16, 07)
des.monster("skeletal dragon", 16, 08)
des.monster("hell hound", 23, 04)
des.monster("hell hound", 24, 04)
-- Locked doors
des.door({ state = "locked", material = "steel", x = 05, y = 10 })
des.door({ state = "locked", material = "steel", x = 06, y = 10 })
des.door({ state = "locked", material = "steel", x = 14, y = 05 })
des.door({ state = "locked", material = "steel", x = 15, y = 05 })
des.door({ state = "locked", material = "steel", x = 14, y = 10 })
des.door({ state = "locked", material = "steel", x = 15, y = 10 })
des.door({ state = "locked", material = "steel", x = 23, y = 05 })
des.door({ state = "locked", material = "steel", x = 24, y = 05 })
-- treasures
des.object("chest", 28, 04)
des.object("iron safe", 13, 01)
des.object("crystal chest", 16, 14)

des.object({ id = "chest", coord = niches[1] })
des.object({ id = "chest", coord = niches[2] })
des.object({ id = "chest", coord = niches[3] })
des.object({ id = "chest", coord = niches[4] })
des.object({ id = "chest", coord = niches[5] })
des.object({ id = "chest", coord = niches[6], contents = function()
   des.object("amulet of flying")
end })
des.object({ id = "chest", coord = niches[7], contents = function()
   des.object({ id = "wax candle", quantity = math.random(4, 8) })
end })
des.object({ id = "chest", coord = niches[8], contents = function()
   des.object({ id = "tallow candle", quantity = math.random(4, 8) })
end })
-- Random corpses of past adventurers
des.object({ id = "corpse", montype = "monk" })
des.object({ id = "corpse", montype = "caveman" })
des.object({ id = "corpse", montype = "healer" })
des.object({ id = "corpse", montype = "wizard" })
des.object({ id = "corpse", montype = "rogue" })
des.object({ id = "corpse", montype = "archeologist" })
des.object({ id = "corpse", montype = "tourist" })
des.object({ id = "corpse", montype = "samurai" })
des.object({ id = "corpse", montype = "priestess" })
des.object({ id = "corpse", montype = "ranger" })
-- Walls in the tower are non diggable and non passable
des.non_diggable(selection.area(00,00,29,15))
des.non_passwall(selection.area(00,00,29,15))
