-- NetHack nymph nymph.lua	$NHDT-Date: 1786886710 2026/08/16 13:25:10 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $
--
-- Nymph level (from UnNetHack, which came from Slash'EM)
-- Level is modified from the original
--

des.room({ type = "garden", lit = 0, w = 12, h = 7,
           contents = function()
   for i = 1, 4 do
      if percent(30) then
         des.terrain(selection.room():rndcoord(), "T")
      end
   end
   des.feature("fountain", 6, 3)
   des.trap("board")
   des.trap("board")
   for i = 1, 3 do
      des.object("chest")
   end
   des.object("crystal chest")
   for i = 1, 4 do
      des.object("=")
   end
   for i = 1, 5 do
      des.object("*")
   end
   for i = 1, 4 do
      des.object()
   end
   for i = 1, 3 do
      des.monster({ class = "n", peaceful = 0 })
   end
   if percent(75) then
      if percent(75) then
         des.monster({ id = "centipede", peaceful = 0 })
      else
         des.monster({ id = "scorpion", peaceful = 0 })
      end
   end
   -- nymph queen
   des.monster({ id = "Aphrodite", peaceful = 0 })
end })

-- stairs up room
des.room({ type = "garden", contents = function()
   des.stair("up")
   des.trap("board")
   des.object()
end })

-- stairs down room
des.room({ type = "garden", contents = function()
   des.stair("down")
   des.trap("board")
   des.trap()
   des.object()
end })

-- normal room, in case of branch portal/stairs placement
des.room({ type = "garden", contents = function()
   for i = 1, 2 do
      if percent(30) then
         des.terrain(selection.room():rndcoord(), "T")
      end
   end
   des.feature("fountain")
   des.trap("board")
   des.trap()
   des.object()
   des.monster({ class = "n", peaceful = 0 })
   des.monster({ class = "n", peaceful = 0 })
end })

des.room({ type = "garden", contents = function()
   des.trap("board")
   des.object()
   des.object()
   if percent(50) then
      if percent(75) then
         des.monster({ id = "centipede", peaceful = 0 })
      else
         des.monster({ id = "snake", peaceful = 0 })
      end
   end
end })

des.room({ type = "garden", contents = function()
   des.trap("board")
   des.trap()
   des.object()
   if percent(50) then
      if percent(75) then
         des.monster({ id = "centipede", peaceful = 0 })
      else
         des.monster({ id = "pit viper", peaceful = 0 })
      end
   end
end })

des.random_corridors()
