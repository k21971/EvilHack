-- NetHack Infidel Inf-goal.lua	$NHDT-Date: 1781994876 2026/06/20 22:34:36 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2024 by Keith Simpson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "goal" level for the quest.
--
--	In this part of the Howling Forest the Paladin awaits you for a duel
--	amidst some ancient ruins.  Kill her to claim the Idol of Moloch!
--
--	Note: some may find the Paladin a tough opponent.  It's intentional!
--	You come here to duel her at HER behest, on HER conditions.
--	She expects to defeat you and came prepared.  To win, you need to
--	outwit her.
--

des.level_flags("mazelevel", "hardfloor")
des.level_init({ style = "mines", fg = ".", bg = "T", smoothed = true,
                 joined = true, lit = 0, walled = true })

--         1         2
--123456789012345678901234567
des.map({ halign = "half-left", valign = "center", map = [[
xxxxxxxxxx........xxxxxxxxxx
xxxxxxx..............xxxxxxx
xxxx....................xxxx
xx........--..-...........xx
x.........|.....|..........x
.............--.|...........
x........--................x
xx........|..|...--.......xx
xxxx.........|..........xxxx
xxxxxxx..............xxxxxxx
xxxxxxxxxx........xxxxxxxxxx
]], contents = function()
   -- The Paladin and her entourage.
   des.monster({ id = "Paladin", coord = { 13, 06 }, peaceful = 0 })
   des.object({ id = "figurine", name = "The Idol of Moloch",
                buc = "cursed", coord = { 13, 06 } })
   des.monster({ id = "templar", coord = { 13, 03 }, asleep = 1 })
   des.monster({ id = "templar", coord = { 15, 04 }, asleep = 1 })
   des.monster({ id = "templar", coord = { 17, 06 }, asleep = 1 })
   des.monster({ id = "aligned priest", coord = { 16, 08 }, align = "law",
                 peaceful = 0, asleep = 1 })
   des.monster({ id = "templar", coord = { 13, 09 }, asleep = 1 })
   des.monster({ id = "templar", coord = { 10, 08 }, asleep = 1 })
   des.monster({ id = "templar", coord = { 09, 05 }, asleep = 1 })
   des.monster({ id = "aligned priest", coord = { 11, 04 }, align = "law",
                 peaceful = 0, asleep = 1 })

   -- Just pretend they're tripwires.
   local around = (selection.ellipse(13, 5, 9, 5)
                   - selection.ellipse(13, 5, 4, 3)):filter_mapchar('.')
   des.trap({ type = "board", coord = around:rndcoord(1) })
   des.trap({ type = "board", coord = around:rndcoord(1) })
   des.trap({ type = "board", coord = around:rndcoord(1) })
   des.trap({ type = "board", coord = around:rndcoord(1) })

   -- There's still some treasure buried in the ruins.
   local ruins = selection.circle(13, 6, 4)
   des.object({ id = "gold piece", quantity = d(10, 100), buried = true,
                coord = ruins:rndcoord(0) })
   des.object({ id = "gold piece", quantity = d(10, 100), buried = true,
                coord = ruins:rndcoord(0) })
   des.object({ class = "*", buried = true, coord = ruins:rndcoord(0) })
   des.object({ class = "*", buried = true, coord = ruins:rndcoord(0) })
   des.object({ class = "*", buried = true, coord = ruins:rndcoord(0) })
   if percent(60) then
      des.object({ class = "=", buried = true, coord = ruins:rndcoord(0) })
   end
   if percent(50) then
      des.object({ class = "=", buried = true, coord = ruins:rndcoord(0) })
   end
   if percent(40) then
      des.object({ class = "\"", buried = true, coord = ruins:rndcoord(0) })
   end
end })

-- the .des switches to NOMAP here: everything below is placed
-- level-wide, not map-relative
des.levregion({ region = { 45, 0, 79, 20 }, region_islev = 1,
                type = "stair-up" })
des.monster("werewolf")
des.monster("werewolf")
des.monster("werewolf")
des.monster("werewolf")
for i = 1, d(5, 4) do
   des.monster("d")
end
des.monster("u")
des.monster("u")
des.monster("u")
if percent(80) then
   des.monster("forest centaur")
end
if percent(50) then
   des.monster("woodchuck")
end

-- And a bit of random junk.
des.object()
des.object()
des.object()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
