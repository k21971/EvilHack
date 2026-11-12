-- $NHDT-Header: NetHack 5.0 minetn-6.lua 2026/06/20_22:35:38 NetHack-5.0:1.9 $
--	Copyright (c) 1989-95 by Jean-Christophe Collet
--	Copyright (c) 1991-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- "Zoo Town" by Derek S. Ray (SporkHack)

des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "inaccessibles")

des.level_init({ style="mines", fg=".", bg="-", smoothed=true, joined=true,lit=1,walled=true })

des.map({ halign = "center", valign = "top", map = [[
.-----...................-------------.-
.|...|...................|...|...|...|..
.|...+..--+--...-FFF+F...|...|...|...|..
.|...|..|...|...|ggggF...|...|-+---+--..
.-----..|...|...|ggggF...---+-.........|
........|...|...|ggggF...........-----..
........-----...------....--+-...|...|..
.----...|...|.{........{..|..|...+...|..
.|..+...|...|.............|..|...|...|..
.|..|...|...|---.....---+-------------.|
.----...--+--..|..---|..................
...|........|..|..|..|----....---------.
...|..T.....-+--..+..|...+....|......|-.
...|-....T........|..|...|....+......|-.
...--..-....T.....--------....|......|-.
.......--.....................----------
]] });

des.region(selection.area(00,00,38,15),"lit")

des.levregion({ type="stair-up", region={01,03,20,19}, region_islev=1, exclude={0,0,39,15} })
des.levregion({ type="stair-down", region={61,03,75,19}, region_islev=1, exclude={0,0,39,15} })

des.feature("fountain",23,07)
des.feature("fountain",14,07)

-- protect the zoo against dwarves and inquisitive players
des.non_diggable(selection.area(13,0,24,8))

des.region({ region={09,07, 11,09}, lit=1, type="candle shop", filled=1 })
des.region({ region={09,03, 11,05}, lit=1, type="tool shop", filled=1 })
des.region({ region={26,01, 28,03}, lit=1, type="shop", filled=1 })
des.region({ region={22,12, 24,13}, lit=1, type=monkfoodshop(), filled=1 })
des.region({ region={31,12, 36,14}, lit=1, type="temple", filled=1 })
des.altar({ x=35,y=13,align=align[1],type="shrine"})
-- Magic chest
des.feature("magic chest", 36, 13)

des.door("closed",5,2)
des.door("locked",4,8)
des.door("closed",10,2)
des.door("closed",10,10)
des.door("locked",13,12)
des.door("locked",20,2)
des.door("closed",24,9)
des.door("closed",25,12)
des.door("closed",28,4)
des.door("locked",28,6)
des.door("closed",30,13)
des.door("closed",31,3)
des.door("closed",35,3)
des.door("closed",33,7)

local in_zoo = selection.floodfill(18,4)
local outside = selection.floodfill(18,8) & selection.area(1,0,37,15)

for i = 1, 4 + d(8) do
  des.monster({ id = "rock gnome", coord = outside:rndcoord(0) })
end
des.monster({ id = "rock gnome", coord = outside:rndcoord(0) })
des.monster({ id = "gnome noble", coord = outside:rndcoord(0) })
des.monster({ id = "rock gnome", coord = outside:rndcoord(0) })
des.monster({ id = "gnome noble", coord = outside:rndcoord(0) })
des.monster({ id = "gnome noble", coord = outside:rndcoord(0) })
des.monster({ id = "gnomish wizard", coord = outside:rndcoord(0) })
des.monster({ id = "gnomish wizard", coord = outside:rndcoord(0) })
des.monster({ id = "mountain dwarf", coord = outside:rndcoord(0) })

des.monster({ id = "watchman", coord = outside:rndcoord(0), peaceful = 1 })
des.monster({ id = "watchman", coord = outside:rndcoord(0), peaceful = 1 })
des.monster({ id = "watchman", coord = outside:rndcoord(0), peaceful = 1 })
des.monster({ id = "watch captain", coord = outside:rndcoord(0), peaceful = 1 })
des.monster({ id = "watch captain", coord = outside:rndcoord(0), peaceful = 1 })
-- The zoo needs some stuff...
if percent(65) then
  des.monster({ id = "tiger", coord = in_zoo:rndcoord(0), peaceful = 0 })
else
  des.monster({ id = "saber-toothed tiger", coord = in_zoo:rndcoord(0), peaceful = 0 })
end
des.monster({ id = "jaguar", coord = in_zoo:rndcoord(0), peaceful = 0 })
des.monster({ id = "giant anaconda", coord = in_zoo:rndcoord(0), peaceful = 0 })
if percent(85) then
  des.monster({ id = "mumak", coord = in_zoo:rndcoord(0), peaceful = 0 })
else
  des.monster({ id = "mastodon", coord = in_zoo:rndcoord(0), peaceful = 0 })
end
-- and a couple token handlers
des.monster({ id = "Mordor orc", coord = in_zoo:rndcoord(0), peaceful = 0,
              inventory = function()
                 des.object("bullwhip")
              end })
des.monster({ id = "Mordor orc", coord = in_zoo:rndcoord(0), peaceful = 0,
              inventory = function()
                 des.object("bullwhip")
              end })
-- and a couple shiny prizes as temptation (for the handlers to use, possibly?)
des.object({ class = "/", coord = in_zoo:rndcoord(0) })
des.object({ class = "/", coord = in_zoo:rndcoord(0) })
des.object({ class = "=", coord = in_zoo:rndcoord(0) })
