-- NetHack gehennom hellc-2.lua	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
-- Previously the New Asmodeus Level
des.level_init({ style="mazegrid", bg ="-" });

des.level_flags("mazelevel", "hardfloor")
des.map({ halign = "center", valign = "center", map = [[
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL  LLLLLLLLLLLLLLLLLLLLLLLLL----
--------------------LLLL---F---LLLLLLLLL|LLLL  LLLLLLLLLLLLLLLLLLL------|..|
|............S.....|LLLL|.....|--F---LLL|LLLLL -------------LLLLLL|#####+..|
|---+-----------...|LLLL|.....|.....|------LLLL|...........|LLLLLL|#----|..|
|.....|........|-+------|.....|.....|.....FLLLLF...........|LLLL---+---L----
|..---|........|........|.....|.....|.....|LLLL|..{..{..{..|LLLL|.....|LLLLL
|..|..S........|..........................|L.|L............+####S.....S#LLLL
|..|..|........|........|.....|.....|.....|LLLL|..{..{..{..|LLLL|.....|LLLLL
|..|..|........|-+------|.....|.....|.....FLLLLF...........|LLLL---+---LLLLL
|..|..-----S----...|LLLL|.....|.....|------LLLL|...........|LLLLLL|H|LLLLLLL
|..S.....|...|.....|LLLL|.....|--F---LLLL|LLLLL-------------LLLLLL|#|LLLLLLL
---------|...|------LLLL---F---LLLLLLLLL--LLLLLLL--------LLLLLLLLL|#|LLLLLLL
LLLLLLLLL|...|LLLLLLLLLLLL###LLLLLLLLLLL|LLLLLL--|......|----------#|LLLLLLL
LLLLLLLLL|...|LLLLLLLLLLLLLL#LL###LLLLLL|LLLLLL|.+......+###########|LLLLLLL
LLLLL-----+++-----LLLL####LL#LL#LLLLLLLL|LLLLLL|.+......+###########|LLLLLLL
LLLLL|...........|LLLL#LLLLL######LL-----LLLLLL--|......|----------#|LLLLLLL
LLLLLF...........S#####LLLLL#LLLL#LL|..|LLLLLLLLL---FF---LLLLLLL---+---LLLLL
LLLLLF...........|LLLL#LLLLL#LLLL###+..FLLLLLLLLLLLLLLLLLLLLLLLL|.....|LLLLL
LLLLL|.....\.....|LLLL#######LLLLLLL|..|LLLLLLLLLLLLLLLLLLLLLLLL|.....|LLLLL
LLLLL-------------LLLLLLLLLLLLLLLLLL----LLLLLLLLLLLLLLLLLLLLLLLL-------LLLLL
]] });
des.stair("up", 65, 17)
des.stair("down", 11, 16)
des.levregion({ type = "branch", region = { 73, 1, 74, 3 } })
des.teleport_region({ region = { 60, 12, 65, 14 } })
-- Doors
for _, c in ipairs({ {67,16},{56,14},{56,13},{49,14},{49,13},{67,8},{67,4},
                     {72,2},{64,6},{70,6},{17,8},{17,4},{13,2},{4,3},{3,10},
                     {6,6},{11,9},{17,16},{36,17} }) do
   des.door({ state = "locked", material = "bronze", x = c[1], y = c[2] })
end
for _, c in ipairs({ {10,14},{11,14},{12,14} }) do
   des.door({ state = "closed", material = "bronze", x = c[1], y = c[2] })
end
-- Drawbridges
des.drawbridge({ x = 46, y = 6, dir = "west", state = "closed" })
des.drawbridge({ x = 43, y = 6, dir = "west", state = "closed" })
-- Non diggable walls
des.non_diggable(selection.area(0,0,75,19))
-- Regions of light and dark
des.region({ region = { 0, 0, 75, 19 }, lit = 0, type = "ordinary" })
for _, r in ipairs({ {25,2,29,5},{31,3,35,5},{37,4,41,5},
                     {37,7,41,8},{31,7,35,9},{25,7,29,10} }) do
   des.region({ region = r, lit = 0, type = "lemurepit", filled = 1 })
end
des.region({ region = { 48, 3, 58, 9 }, lit = 1, type = "ordinary" })
des.region({ region = { 6, 15, 16, 18 }, lit = 1, type = "ordinary" })
-- random demon boss
if percent(50) then
   if percent(50) then
      des.monster({ id = "Demogorgon", coord = { 11, 16 }, peaceful = 0 })
      -- entourage
      des.monster({ id = "balrog", coord = { 8, 16 }, name = "Xennulig",
                    asleep = 1 })
      des.monster({ id = "pit fiend", coord = { 9, 17 }, name = "Brarkonod",
                    asleep = 1 })
      des.monster({ id = "succubus", coord = { 14, 16 }, asleep = 1 })
      des.monster({ id = "succubus", coord = { 13, 17 }, asleep = 1 })
      des.monster({ id = "incubus", coord = { 11, 18 }, asleep = 1 })
      des.monster({ id = "sandestin", coord = { 10, 13 }, name = "Izamon",
                    asleep = 1 })
      des.monster({ id = "horned devil", coord = { 12, 13 }, name = "Arozud",
                    asleep = 1 })
   else
      des.monster({ id = "Tiamat", coord = { 11, 16 }, peaceful = 0 })
      -- entourage - chaotic aligned dragons
      des.monster({ id = "black dragon", coord = { 8, 16 }, asleep = 1 })
      des.monster({ id = "green dragon", coord = { 9, 17 }, asleep = 1 })
      des.monster({ id = "red dragon", coord = { 14, 16 }, asleep = 1 })
      des.monster({ id = "yellow dragon", coord = { 13, 17 }, asleep = 1 })
      des.monster({ id = "balrog", coord = { 11, 18 }, name = "Dagul",
                    asleep = 1 })
      des.monster({ id = "blue dragon", coord = { 10, 13 }, asleep = 1 })
      des.monster({ id = "shadow dragon", coord = { 12, 13 }, asleep = 1 })
   end
else
   if percent(50) then
      des.monster({ id = "Asmodeus", coord = { 11, 16 }, peaceful = 0 })
      -- entourage
      des.monster({ id = "marilith", coord = { 8, 16 }, name = "Voran",
                    asleep = 1 })
      des.monster({ id = "marilith", coord = { 9, 17 }, name = "Joggeruuth",
                    asleep = 1 })
      des.monster({ id = "succubus", coord = { 14, 16 }, asleep = 1 })
      des.monster({ id = "succubus", coord = { 13, 17 }, asleep = 1 })
      des.monster({ id = "incubus", coord = { 11, 18 }, asleep = 1 })
      des.monster({ id = "barbed devil", coord = { 10, 13 }, name = "Gogmelon",
                    asleep = 1 })
      des.monster({ id = "barbed devil", coord = { 12, 13 },
                    name = "Ulgaxicus", asleep = 1 })
   else
      des.monster({ id = "Graz'zt", coord = { 11, 16 }, peaceful = 0 })
      -- entourage
      des.monster({ id = "nalfeshnee", coord = { 8, 16 }, name = "Dirozius",
                    asleep = 1 })
      des.monster({ id = "nalfeshnee", coord = { 9, 17 }, name = "Thorakoth",
                    asleep = 1 })
      des.monster({ id = "succubus", coord = { 14, 16 }, asleep = 1 })
      des.monster({ id = "succubus", coord = { 13, 17 }, asleep = 1 })
      des.monster({ id = "incubus", coord = { 11, 18 }, asleep = 1 })
      des.monster({ id = "vrock", coord = { 10, 13 }, name = "Ugek",
                    asleep = 1 })
      des.monster({ id = "vrock", coord = { 12, 13 }, name = "Ozroth",
                    asleep = 1 })
   end
end
-- A few traps on the way in
des.trap("board", 10, 12)
des.trap("board", 11, 12)
des.trap("board", 12, 12)
des.trap("ice", 11, 11)
des.trap("fire", 10, 10)
des.trap("fire", 12, 10)
-- The 'fake' throne room
des.monster("&", 7, 4)
des.monster("&", 14, 4)
des.monster("&", 7, 8)
des.monster("&", 14, 8)
des.object({ class = "!", id = "invisibility", x = 7, y = 4 })
des.object({ class = "!", id = "invisibility", x = 14, y = 4 })
des.object({ class = "!", id = "invisibility", x = 7, y = 8 })
des.object({ class = "!", id = "invisibility", x = 14, y = 8 })
des.monster({ id = "giant mimic", coord = { 11, 6 },
              appear_as = "ter:staircase down" })
des.trap("magic", 9, 6)
des.trap("magic", 13, 6)
des.trap("magic", 11, 5)
des.trap("magic beam", 11, 7)
-- Some random weapons and armor.
des.object("[")
des.object("[")
des.object("[")
des.object("[")
des.object(")")
des.object(")")
des.object(")")
des.object(")")
des.object("!")
des.object("!")
des.object("?")
des.object("?")
des.object("?")
-- Traps in the outer chambers
des.trap("spiked pit", 2, 8)
des.trap("anti magic", 21, 6)
des.trap("sleep gas", 19, 5)
des.trap("fire", 56, 6)
des.trap("spear", 53, 6)
des.trap("ice", 50, 3)
des.trap("polymorph", 69, 2)
-- Teleport scrolls are kept in a closet
des.object({ class = "?", id = "teleportation", x = 63, y = 6 })
-- Traps in the inner spiral
des.trap("magic", 22, 14)
des.trap("anti magic", 26, 18)
des.trap("fire", 28, 13)
des.trap("spiked pit", 32, 13)
-- Treasure room
des.gold(500, 38, 16)
des.gold(500, 38, 17)
des.gold(500, 38, 18)
des.object("amulet of change", 37, 16)
des.object({ class = "=", id = "hunger", x = 37, y = 18, buc = "cursed",
             spe = 0 })
des.object({ class = "*", id = "diamond", x = 38, y = 16, buc = "cursed",
             spe = 0, name = "The Tears of Koth" })
des.object({ class = "*", id = "ruby", x = 38, y = 17, buc = "cursed",
             spe = 0, name = "The Heart of Cassanova" })
des.object({ class = "*", id = "emerald", x = 38, y = 18, buc = "cursed",
             spe = 0, name = "The Eye of Hera" })
des.monster("&", 38, 16)
des.monster("&", 38, 17)
des.monster("&", 38, 18)
-- Outer Trap Room
des.gold(1, 48, 13)
des.gold(1, 48, 14)
des.object({ class = "?", id = "fire", x = 48, y = 13, buc = "cursed",
             spe = 0 })
des.object({ class = "?", id = "earth", x = 48, y = 14, buc = "cursed",
             spe = 0 })
des.trap("magic", 48, 13)
des.trap("land mine", 48, 14)
des.gold(1, 51, 12)
des.gold(1, 50, 14)
des.gold(1, 52, 15)
des.gold(1, 53, 13)
des.trap("fire", 54, 13)
des.trap("ice", 54, 14)
des.trap("magic", 52, 12)
des.trap("magic beam", 52, 15)
des.trap("spiked pit", 51, 13)
des.trap("anti magic", 50, 14)
des.object({ class = "!", id = "polymorph", x = 50, y = 13 })
des.object({ class = "?", id = "create monster", x = 50, y = 12,
             buc = "cursed", spe = 0 })
des.object({ class = "?", id = "amnesia", x = 50, y = 12, buc = "cursed",
             spe = 0 })
-- Guardians in the fountain room
des.monster("&", 49, 4)
des.monster("&", 49, 8)
des.monster("&", 52, 4)
des.monster("&", 52, 8)
des.monster("V", 55, 4)
des.monster("V", 55, 8)
-- Guardians in the outer chamber
des.monster("hell hound", 67, 6)
des.monster("hell hound", 69, 6)
des.monster("hell hound", 68, 5)
des.monster("hell hound", 68, 7)
-- Guardian in the branch chamber
des.monster("blue jelly", 73, 2)
-- Greeting in the outer chamber
des.monster("imp", 69, 17)
des.object({ id = "helm of opposite alignment", x = 65, y = 17,
             buc = "cursed", spe = -2 })
des.object({ id = "loadstone", x = 65, y = 18, buc = "cursed", spe = 0 })
des.object({ id = "amulet of strangulation", x = 65, y = 18, buc = "cursed",
             spe = 0 })
-- Drawbridge sentinels
des.monster("salamander", 45, 5)
des.monster("salamander", 45, 7)
des.monster("salamander", 20, 13)
des.monster("salamander", 30, 17)
des.monster("salamander", 36, 12)
des.monster("salamander", 61, 4)
des.monster("salamander", 61, 8)
des.monster("salamander", 73, 8)
des.monster("salamander", 73, 14)
