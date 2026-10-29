-- NetHack themerms.lua	$NHDT-Date: 1781994889 2026/06/20 22:34:49 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.43 $
--	Copyright (c) 2020 by Pasi Kallinen
-- NetHack may be freely redistributed.  See license for details.
--
-- themerooms is an array of tables.
-- the tables define "name", "frequency", "contents", "mindiff" and "maxdiff".
-- * "name" is not shown in-game; it is so that developers can specify a
--   certain room to generate by using the THEMERM or THEMERMFILL environment
--   variable. While technically optional, it should be provided on all the
--   rooms; if it isn't, the room can't be directly specified.
-- * "frequency" is optional; if omitted, 1 is assumed.
-- * "contents" is a function describing what gets put into the room.
-- * "mindiff" and "maxdiff" are optional and independent; if omitted, the
--    room is not constrained by level difficulty.
-- * "eligible" is optional; if omitted, True is assumed.
--
-- themeroom_fills is an array of tables with the exact same structure as
-- themerooms. It is used for contents of a room that are independent of its
-- shape, so that interestingly-shaped themerooms can be filled with a variety
-- of contents.
-- * The "contents" functions in themeroom_fills take the room they are
--   filling as an argument.
-- * Frequency of themeroom_fills is a separate pool from themerooms, and has
--   no effect on how likely that any given room will receive a themeroom_fill.
--
-- des.room({ type = "ordinary", filled = 1 })
--   - ordinary rooms can be converted to shops or any other special rooms.
--   - filled = 1 means the room gets random room contents, even if it
--     doesn't get converted into a special room. Without filled,
--     the room only gets what you define in here.
--   - use type = "themed" to force a room that's never converted
--     to a special room, such as a shop or a temple.
--
-- for each level, the core first calls pre_themerooms_generate(),
-- then it calls themerooms_generate() multiple times until it decides
-- enough rooms have been generated, and then it calls
-- post_themerooms_generate(). When the level has been generated, with
-- joining corridors and rooms filled, the core calls post_level_generate().
-- The lua state is persistent through the gameplay, but not across saves,
-- so remember to reset any variables.

local postprocess = { };

themeroom_fills = {

   {
      name = "Ice room",
      contents = function(rm)
         local ice = selection.room();
         des.terrain(ice, "I");
         if (percent(25)) then
            local mintime = 1000 - (nh.level_difficulty() * 100);
            local ice_melter = function(x,y)
               nh.start_timer_at(x,y, "melt-ice", mintime + nh.rn2(1000));
            end;
            ice:iterate(ice_melter);
         end
      end,
   },

   {
      name = "Cloud room",
      contents = function(rm)
         local fog = selection.room();
         for i = 1, (fog:numpoints() / 4) do
            des.monster({ id = "fog cloud", asleep = true });
         end
         des.gas_cloud({ selection = fog });
      end,
   },

   {
      name = "Boulder room",
      mindiff = 4,
      contents = function(rm)
         local locs = selection.room():percentage(30);
         local func = function(x,y)
            if (percent(50)) then
               des.object("boulder", x, y);
            else
               des.trap("rolling boulder", x, y);
            end
         end;
         locs:iterate(func);
      end,
   },

   {
      name = "Spider nest",
      contents = function(rm)
         local spooders = nh.level_difficulty() > 8;
         local locs = selection.room():percentage(30);
         local func = function(x,y)
            des.trap({ type = "web", x = x, y = y,
                       spider_on_web = spooders and percent(80) });
         end
         locs:iterate(func);
      end,
   },

   {
      name = "Trap room",
      contents = function(rm)
         local traps = { "arrow", "dart", "falling rock", "bear",
                        "land mine", "sleep gas", "rust",
                        "anti magic" };
         shuffle(traps);
         local locs = selection.room():percentage(30);
         local func = function(x,y)
            des.trap(traps[1], x, y);
         end
         locs:iterate(func);
      end,
   },

   {
      name = "Garden",
      eligible = function(rm) return rm.lit == true; end,
      contents = function(rm)
         local s = selection.room();
         local npts = (s:numpoints() / 6);
         for i = 1, npts do
            des.monster({ id = "wood nymph", asleep = true });
            if (percent(30)) then
               des.feature("fountain");
            end
         end
         table.insert(postprocess, { handler = make_garden_walls,
                                     data = { sel = selection.room() } });
      end
   },

   {
      name = "Buried treasure",
      contents = function(rm)
         des.object({ id = "chest", buried = true, contents = function(otmp)
            local xobj = otmp:totable();
            -- keep track of the last buried treasure
            if (xobj.NO_OBJ == nil) then
               table.insert(postprocess, { handler = make_dig_engraving,
                                        data = { x = xobj.ox, y = xobj.oy }});
            end
            for i = 1, d(3,4) do
               des.object();
            end
         end });
      end,
   },

   {
      name = "Buried zombies",
      contents = function(rm)
         local diff = nh.level_difficulty()
         -- start with [1..4] for low difficulty
         local zombifiable = { "kobold", "gnome", "orc", "dwarf" };
         if diff > 3 then          -- medium difficulty
            zombifiable[5], zombifiable[6] = "elf", "human";
            if diff > 6 then       -- high difficulty (relatively speaking)
               zombifiable[7], zombifiable[8] = "ettin", "giant";
            end
         end
         for i = 1, (rm.width * rm.height) / 2 do
            shuffle(zombifiable);
            local o = des.object({ id = "corpse", montype = zombifiable[1],
                                 buried = true });
            o:stop_timer("rot-corpse");
            o:start_timer("zombify-mon", math.random(990, 1010));
         end
      end,
   },

   {
      name = "Massacre",
      contents = function(rm)
         local mon = { "apprentice", "warrior", "ninja", "thug",
                     "hunter", "acolyte", "abbot", "page",
                     "attendant", "neanderthal", "chieftain",
                     "student", "wizard", "valkyrie", "tourist",
                     "samurai", "rogue", "ranger", "priestess",
                     "priest", "monk", "knight", "healer",
                     "cavewoman", "caveman", "barbarian",
                     "archeologist",
                     -- EvilHack roles and their quest guardians
                     "convict", "druid", "infidel",
                     "inmate", "aspirant", "cultist" };
         local idx = math.random(#mon);
         for i = 1, d(5,5) do
            if (percent(10)) then idx = math.random(#mon); end
            des.object({ id = "corpse", montype = mon[idx] });
         end
      end,
   },

   {
      name = "Statuary",
      contents = function(rm)
         for i = 1, d(5,5) do
            des.object({ id = "statue" });
         end
         for i = 1, d(3) do
            des.trap("statue");
         end
      end,
   },


   {
      name = "Light source",
      eligible = function(rm) return rm.lit == false; end,
      contents = function(rm)
         des.object({ id = "oil lamp", lit = true });
      end
   },

   {
      name = "Temple of the gods",
      contents = function(rm)
         des.altar({ align = align[1] });
         des.altar({ align = align[2] });
         des.altar({ align = align[3] });
      end,
   },

   {
      name = "Ghost of an Adventurer",
      contents = function(rm)
         local loc = selection.room():rndcoord(0);
         des.monster({ id = "ghost", asleep = true, waiting = true,
                       coord = loc });
         if percent(65) then
            des.object({ id = "dagger", coord = loc, buc = "not-blessed" });
         end
         if percent(55) then
            des.object({ class = ")", coord = loc, buc = "not-blessed" });
         end
         if percent(45) then
            des.object({ id = "bow", coord = loc, buc = "not-blessed" });
            des.object({ id = "arrow", coord = loc, buc = "not-blessed" });
         end
         if percent(65) then
            des.object({ class = "[", coord = loc, buc = "not-blessed" });
         end
         if percent(20) then
            des.object({ class = "=", coord = loc, buc = "not-blessed" });
         end
         if percent(20) then
            des.object({ class = "?", coord = loc, buc = "not-blessed" });
         end
      end,
   },

   {
      name = "Storeroom",
      contents = function(rm)
         local locs = selection.room():percentage(30);
         local func = function(x,y)
            if (percent(25)) then
               des.object("chest");
            else
               des.monster({ class = "m", appear_as = "obj:chest" });
            end
         end;
         locs:iterate(func);
      end,
   },

   {
      name = "Teleportation hub",
      contents = function(rm)
         local locs = selection.room():filter_mapchar(".");
         for i = 1, 2 + nh.rn2(3) do
            local pos = locs:rndcoord(1);
            if (pos.x > 0) then
               pos.x = pos.x + rm.region.x1 - 1;
               pos.y = pos.y + rm.region.y1;
               table.insert(postprocess, { handler = make_a_trap,
                                      data = { type = "teleport", seen = true,
                                               coord = pos, teledest = 1 } });
            end
         end
      end,
   },

   {
      name = "Meat",
      contents = function(rm)
         local meats = { "meatball", "meatball", "meat ring",
                         "meat stick", "enormous meatball" };
         local i = d(5, 5);
         while (i > 0) do
            local meatidx = math.random(#meats);
            des.object({ id = meats[meatidx] });
            i = i - meatidx;
         end
         if (nh.level_difficulty() > 8 and percent(75)) then
            des.monster({ id = "flesh golem", waiting = true });
         end
      end,
   },

};

themerooms = {
   {
      name = "default",
      frequency = 1000,
      contents = function()
         des.room({ type = "ordinary", filled = 1 });
      end
   },

   {
      name = "Fake Delphi",
      contents = function()
         des.room({ type = "ordinary", w = 11,h = 9, filled = 1,
                  contents = function()
                     des.room({ type = "ordinary", x = 4,y = 3, w = 3,h = 3,
                                filled = 1,
                                contents = function()
                                   des.door({ state="random", wall="all" });
                                end
                     });
                  end
         });
      end,
   },

   {
      name = "Room in a room",
      contents = function()
         des.room({ type = "ordinary", filled = 1,
                  contents = function()
                     des.room({ type = "ordinary",
                                contents = function()
                                   des.door({ state="random", wall="all" });
                                end
                     });
                  end
         });
      end,
   },

   {
      name = "Huge room with another room inside",
      contents = function()
         des.room({ type = "ordinary", w = nh.rn2(10)+11,h = nh.rn2(5)+8,
                    filled = 1,
            contents = function()
               if (percent(90)) then
                  des.room({ type = "ordinary", filled = 1,
                             contents = function()
                                des.door({ state="random", wall="all" });
                                   if (percent(50)) then
                                      des.door({ state="random", wall="all" });
                                   end
                              end
                  });
               end
            end
         });
      end,
   },

   {
      name = "Nesting rooms",
      contents = function()
         des.room({ type = "ordinary", w = 9 + nh.rn2(4), h = 9 + nh.rn2(4),
                    filled = 1,
            contents = function(rm)
               local wid = math.random(math.floor(rm.width / 2), rm.width - 2);
               local hei = math.random(math.floor(rm.height / 2),
                                       rm.height - 2);
               des.room({ type = "ordinary", w = wid,h = hei, filled = 1,
                  contents = function()
                     if (percent(90)) then
                        des.room({ type = "ordinary", filled = 1,
                           contents = function()
                              des.door({ state="random", wall="all" });
                              if (percent(15)) then
                                 des.door({ state="random", wall="all" });
                              end
                           end
                        });
                     end
                     des.door({ state="random", wall="all" });
                     if (percent(15)) then
                        des.door({ state="random", wall="all" });
                     end
                  end
               });
            end
         });
      end,
   },

   {
      name = "Default room with themed fill",
      frequency = 6,
      contents = function()
         des.room({ type = "themed", contents = themeroom_fill });
      end
   },

   {
      name = "Unlit room with themed fill",
      frequency = 2,
      contents = function()
         des.room({ type = "themed", lit = 0, contents = themeroom_fill });
      end
   },

   {
      name = "Room with both normal contents and themed fill",
      frequency = 2,
      contents = function()
         des.room({ type = "themed", filled = 1, contents = themeroom_fill });
      end
   },

   {
      name = 'Pillars',
      contents = function()
         des.room({ type = "themed", w = 10, h = 10,
                  contents = function(rm)
                     local terr = { "-", "-", "-", "-", "L", "P", "T" };
                     shuffle(terr);
                     for x = 0, (rm.width / 4) - 1 do
                        for y = 0, (rm.height / 4) - 1 do
                           des.terrain({ x = x * 4 + 2, y = y * 4 + 2, typ = terr[1], lit = -2 });
                           des.terrain({ x = x * 4 + 3, y = y * 4 + 2, typ = terr[1], lit = -2 });
                           des.terrain({ x = x * 4 + 2, y = y * 4 + 3, typ = terr[1], lit = -2 });
                           des.terrain({ x = x * 4 + 3, y = y * 4 + 3, typ = terr[1], lit = -2 });
                        end
                     end
                  end
         });
      end,
   },

   {
      name = 'Mausoleum',
      contents = function()
         des.room({ type = "themed", w = 5 + nh.rn2(3)*2, h = 5 + nh.rn2(3)*2,
                  contents = function(rm)
                     des.room({ type = "themed",
                                 x = (rm.width - 1) / 2, y = (rm.height - 1) / 2,
                                 w = 1, h = 1, joined = false,
                                 contents = function()
                                    if (percent(50)) then
                                       local mons = { "M", "V", "L", "Z" };
                                       shuffle(mons);
                                       des.monster({ class = mons[1], x=0,y=0, waiting = 1 });
                                    else
                                       des.object({ id = "corpse", montype = "@", coord = {0,0} });
                                    end
                                    if (percent(20)) then
                                       des.door({ state="secret", wall="all" });
                                    end
                                 end
                     });
                  end
         });
      end,
   },

   {
      name = 'Random dungeon feature in the middle of an odd-sized room',
      contents = function()
         local wid = 3 + (nh.rn2(3) * 2);
         local hei = 3 + (nh.rn2(3) * 2);
         des.room({ type = "ordinary", filled = 1, w = wid, h = hei,
                  contents = function(rm)
                     local feature = { "C", "L", "I", "P", "T" };
                     shuffle(feature);
                     des.terrain((rm.width - 1) / 2, (rm.height - 1) / 2,
                                 feature[1]);
                  end
         });
      end,
   },

   {
      name = 'L-shaped',
      contents = function()
         des.map({ map = [[
-----xxx
|...|xxx
|...|xxx
|...----
|......|
|......|
|......|
--------]], contents = function(m) filler_region(1,1); end });
      end,
   },

   {
      name = 'L-shaped, rot 1',
      contents = function()
         des.map({ map = [[
xxx-----
xxx|...|
xxx|...|
----...|
|......|
|......|
|......|
--------]], contents = function(m) filler_region(5,1); end });
      end,
   },

   {
      name = 'L-shaped, rot 2',
      contents = function()
         des.map({ map = [[
--------
|......|
|......|
|......|
----...|
xxx|...|
xxx|...|
xxx-----]], contents = function(m) filler_region(1,1); end });
      end,
   },

   {
      name = 'L-shaped, rot 3',
      contents = function()
         des.map({ map = [[
--------
|......|
|......|
|......|
|...----
|...|xxx
|...|xxx
-----xxx]], contents = function(m) filler_region(1,1); end });
      end,
   },

   {
      name = 'Blocked center',
      contents = function()
         des.map({ map = [[
-----------
|.........|
|.........|
|.........|
|...LLL...|
|...LLL...|
|...LLL...|
|.........|
|.........|
|.........|
-----------]], contents = function(m)
            if (percent(30)) then
               local terr = { "-", "P" };
               shuffle(terr);
               des.replace_terrain({ region = {1,1, 9,9},
                                     fromterrain = "L",
                                     toterrain = terr[1] });
            end
            filler_region(1,1);
         end });
      end,
   },

   {
      name = 'Circular, small',
      contents = function()
         des.map({ map = [[
xx---xx
x--.--x
--...--
|.....|
--...--
x--.--x
xx---xx]], contents = function(m) filler_region(3,3); end });
      end,
   },

   {
      name = 'Circular, medium',
      contents = function()
         des.map({ map = [[
xx-----xx
x--...--x
--.....--
|.......|
|.......|
|.......|
--.....--
x--...--x
xx-----xx]], contents = function(m) filler_region(4,4); end });
      end,
   },

   {
      name = 'Circular, big',
      contents = function()
         des.map({ map = [[
xxx-----xxx
x---...---x
x-.......-x
--.......--
|.........|
|.........|
|.........|
--.......--
x-.......-x
x---...---x
xxx-----xxx]], contents = function(m) filler_region(5,5); end });
      end,
   },

   {
      name = 'T-shaped',
      contents = function()
         des.map({ map = [[
xxx-----xxx
xxx|...|xxx
xxx|...|xxx
----...----
|.........|
|.........|
|.........|
-----------]], contents = function(m) filler_region(5,5); end });
      end,
   },

   {
      name = 'T-shaped, rot 1',
      contents = function()
         des.map({ map = [[
-----xxx
|...|xxx
|...|xxx
|...----
|......|
|......|
|......|
|...----
|...|xxx
|...|xxx
-----xxx]], contents = function(m) filler_region(2,2); end });
      end,
   },

   {
      name = 'T-shaped, rot 2',
      contents = function()
         des.map({ map = [[
-----------
|.........|
|.........|
|.........|
----...----
xxx|...|xxx
xxx|...|xxx
xxx-----xxx]], contents = function(m) filler_region(2,2); end });
      end,
   },

   {
      name = 'T-shaped, rot 3',
      contents = function()
         des.map({ map = [[
xxx-----
xxx|...|
xxx|...|
----...|
|......|
|......|
|......|
----...|
xxx|...|
xxx|...|
xxx-----]], contents = function(m) filler_region(5,5); end });
      end,
   },

   {
      name = 'S-shaped',
      contents = function()
         des.map({ map = [[
-----xxx
|...|xxx
|...|xxx
|...----
|......|
|......|
|......|
----...|
xxx|...|
xxx|...|
xxx-----]], contents = function(m) filler_region(2,2); end });
      end,
   },

   {
      name = 'S-shaped, rot 1',
      contents = function()
         des.map({ map = [[
xxx--------
xxx|......|
xxx|......|
----......|
|......----
|......|xxx
|......|xxx
--------xxx]], contents = function(m) filler_region(5,5); end });
      end,
   },

   {
      name = 'Z-shaped',
      contents = function()
         des.map({ map = [[
xxx-----
xxx|...|
xxx|...|
----...|
|......|
|......|
|......|
|...----
|...|xxx
|...|xxx
-----xxx]], contents = function(m) filler_region(5,5); end });
      end,
   },

   {
      name = 'Z-shaped, rot 1',
      contents = function()
         des.map({ map = [[
--------xxx
|......|xxx
|......|xxx
|......----
----......|
xxx|......|
xxx|......|
xxx--------]], contents = function(m) filler_region(2,2); end });
      end,
   },

   {
      name = 'Cross',
      contents = function()
         des.map({ map = [[
xxx-----xxx
xxx|...|xxx
xxx|...|xxx
----...----
|.........|
|.........|
|.........|
----...----
xxx|...|xxx
xxx|...|xxx
xxx-----xxx]], contents = function(m) filler_region(6,6); end });
      end,
   },

   {
      name = 'Four-leaf clover',
      contents = function()
         des.map({ map = [[
-----x-----
|...|x|...|
|...---...|
|.........|
---.....---
xx|.....|xx
---.....---
|.........|
|...---...|
|...|x|...|
-----x-----]], contents = function(m) filler_region(6,6); end });
      end,
   },

   {
      name = 'Water-surrounded vault',
      contents = function()
         des.map({ map = [[
}}}}}}
}----}
}|..|}
}|..|}
}----}
}}}}}}]], contents = function(m)
            des.region({ region={3,3,3,3}, type="themed", irregular=true,
                         filled=0, joined=false });
            local nasty_undead = { "giant zombie", "ettin zombie", "vampire noble" };
            local chest_spots = { { 2, 2 }, { 3, 2 }, { 2, 3 }, { 3, 3 } };

            shuffle(chest_spots)
            -- Guarantee an escape item inside one of the chests, to prevent
            -- the hero falling in from above and becoming permanently stuck
            -- [cf. generate_way_out_method(sp_lev.c)].
            -- If the escape item is made of glass or crystal, make sure that
            -- the chest isn't locked so that kicking it to gain access to its
            -- contents won't be necessary; otherwise retain lock state from
            -- random creation.
            -- "pick-axe", "dwarvish mattock" could be included in the list of
            -- escape items but don't normally generate in containers.
            local escape_items = {
               "scroll of teleportation", "ring of teleportation",
               "wand of teleportation", "wand of digging"
            };
            local itm = obj.new(escape_items[math.random(#escape_items)]);
            local itmcls = itm:class()
            local box
            if itmcls[ "material" ] == "glass" then
                  -- explicitly force chest to be unlocked
                  box = des.object({ id = "chest", coord = chest_spots[1],
                                    olocked = "no" });
            else
                  -- accept random locked/unlocked state
                  box = des.object({ id = "chest", coord = chest_spots[1] });
            end;
            box:addcontent(itm);

            for i = 2, #chest_spots do
                  des.object({ id = "chest", coord = chest_spots[i] });
            end

            shuffle(nasty_undead);
            des.monster(nasty_undead[1], 2, 2);
            des.exclusion({ type = "teleport", region = { 2,2, 3,3 } });
         end });
      end,
   },

   {
      name = 'Twin businesses',
      mindiff = 4, -- arbitrary
      contents = function()
         -- Due to the way room connections work in mklev.c, we must guarantee
         -- that the "aisle" between the shops touches all four walls of the
         -- larger room. Thus it has an extra width and height.
         des.room({ type="themed", w=9, h=5, contents = function()
               -- There are eight possible placements of the two shops, four of
               -- which have the vertical aisle in the center.
               southeast = function() return percent(50) and "south" or "east" end
               northeast = function() return percent(50) and "north" or "east" end
               northwest = function() return percent(50) and "north" or "west" end
               southwest = function() return percent(50) and "south" or "west" end
               placements = {
                  { lx = 1, ly = 1, rx = 4, ry = 1, lwall = "south", rwall = southeast() },
                  { lx = 1, ly = 2, rx = 4, ry = 2, lwall = "north", rwall = northeast() },
                  { lx = 1, ly = 1, rx = 5, ry = 1, lwall = southeast(), rwall = southwest() },
                  { lx = 1, ly = 1, rx = 5, ry = 2, lwall = southeast(), rwall = northwest() },
                  { lx = 1, ly = 2, rx = 5, ry = 1, lwall = northeast(), rwall = southwest() },
                  { lx = 1, ly = 2, rx = 5, ry = 2, lwall = northeast(), rwall = northwest() },
                  { lx = 2, ly = 1, rx = 5, ry = 1, lwall = southwest(), rwall = "south" },
                  { lx = 2, ly = 2, rx = 5, ry = 2, lwall = northwest(), rwall = "north" }
               }
               ltype,rtype = "weapon shop","armor shop"
               if percent(50) then
                  ltype,rtype = rtype,ltype
               end
               shopdoorstate = function()
                  if percent(1) then
                     return "locked"
                  elseif percent(50) then
                     return "closed"
                  else
                     return "open"
                  end
               end
               p = placements[d(#placements)]
               des.room({ type=ltype, x=p["lx"], y=p["ly"], w=3, h=3, filled=1, joined=false,
                           contents = function()
                     des.door({ state=shopdoorstate(), wall=p["lwall"] })
                  end
               });
               des.room({ type=rtype, x=p["rx"], y=p["ry"], w=3, h=3, filled=1, joined=false,
                           contents = function()
                     des.door({ state=shopdoorstate(), wall=p["rwall"] })
                  end
               });
            end
         });
      end
   },

   -- EvilHack additions: the unique-to-EvilHack random vaults from
   -- master's dat/vaults.des (the SporkHack rndvault set), merged in
   -- as themed rooms; vaults overlapping an upstream entry above were
   -- deduplicated in favor of the upstream version. frequency = 5
   -- keeps each entry at roughly master's per-vault appearance rate.

   -- lava/ice/pool/tree/fountain/grass/sand room
   {
      name = "Terrain spots",
      frequency = 5,
      contents = function()
         des.room({ type = "themed", w = 3, h = 3, filled = 1,
                    contents = function()
            local terr = { "L", "I", "P", "T", "t", "{", "g", "a" };
            local pick = terr[math.random(#terr)];
            for y = 0, 2 do
               for x = 0, 2 do
                  if ((x == 1 and y == 1) or percent(10)) then
                     des.terrain(x, y, pick);
                  end
               end
            end
         end });
      end,
   },
   -- tiny cage, big monster
   {
      name = "Tiny cage",
      frequency = 5,
      mindiff = 7,
      contents = function()
         des.room({ type = "themed", w = 5, h = 5, filled = 1,
                    contents = function()
            for y = 1, 3 do
               for x = 1, 3 do
                  if (x ~= 2 or y ~= 2) then
                     des.terrain(x, y, "F");
                  end
               end
            end
            if (percent(80)) then
               local mons = { "M", "D", "O", "Z", "T", "H" };
               des.monster({ class = mons[math.random(#mons)],
                             x = 2, y = 2, peaceful = 0 });
            else
               des.trap({ coord = { 2, 2 } });
            end
         end });
      end,
   },
   -- random cloud/lava/ice/pool/tree/grass/sand room
   {
      name = "Terrain flood",
      frequency = 5,
      contents = function()
         des.room({ type = "themed", filled = 1,
                    contents = function()
            local terr = { "C", "L", "I", "P", "T", "t", "g", "a" };
            des.terrain(selection.floodfill(1, 1):percentage(30),
                        terr[math.random(#terr)]);
         end });
      end,
   },
   -- room split by a wall or iron bars, with a gap in the middle
   {
      name = "Split",
      frequency = 5,
      contents = function()
         des.room({ type = "themed", w = 5, h = 5, filled = 1,
                    contents = function()
            local terr = percent(50) and "-" or "F";
            local idx = math.random(1, 3);
            if (percent(50)) then
               for x = 0, 4 do
                  des.terrain(x, idx, terr);
               end
               des.terrain(2, idx, ".");
            else
               for y = 0, 4 do
                  des.terrain(idx, y, terr);
               end
               des.terrain(idx, 2, ".");
            end
         end });
      end,
   },
   -- Split horizontally
   {
      name = "Split, horizontal",
      frequency = 5,
      mindiff = 5,
      contents = function()
         des.map({ map = [[
--------
|......|
|......|
|------|
|......|
|......|
|......|
--------]], contents = function(m)
            if (percent(40)) then
               des.replace_terrain({ region = { 1, 3, 6, 3 },
                                     fromterrain = "-",
                                     toterrain = percent(50) and "L" or "F",
                                     chance = 100 });
            end
            des.region({ region = { 1, 1, 6, 2 }, type = "ordinary",
                         filled = 1 });
            des.region({ region = { 1, 4, 6, 6 }, type = "ordinary",
                         filled = 1 });
         end });
      end,
   },
   -- Split vertically
   -- (master replaces mapchar '-' here, i.e. HWALL, but this divider
   -- is drawn with '|', VWALL, so its 40 percent arm never fires;
   -- replace_terrain matches exact terrain types in both trees)
   {
      name = "Split, vertical",
      frequency = 5,
      mindiff = 5,
      contents = function()
         des.map({ map = [[
---------
|...|...|
|...|...|
|...|...|
|...|...|
|...|...|
|...|...|
---------]], contents = function(m)
            if (percent(40)) then
               des.replace_terrain({ region = { 4, 1, 4, 6 },
                                     fromterrain = "|",
                                     toterrain = percent(50) and "L" or "F",
                                     chance = 100 });
            end
            des.region({ region = { 1, 1, 3, 6 }, type = "ordinary",
                         filled = 1 });
            des.region({ region = { 5, 1, 7, 6 }, type = "ordinary",
                         filled = 1 });
         end });
      end,
   },
   -- U-shaped
   {
      name = "U-shaped",
      frequency = 5,
      contents = function()
         des.map({ map = [[
-----xxx-----
|...|xxx|...|
|...|xxx|...|
|...-----...|
|...........|
|...........|
|...........|
-------------]], contents = function(m) filler_region(5, 5); end });
      end,
   },
   -- C-shaped
   {
      name = "C-shaped",
      frequency = 5,
      contents = function()
         des.map({ map = [[
--------
|......|
|......|
|...----
|...|xxx
|...|xxx
|...----
|......|
|......|
--------]], contents = function(m) filler_region(1, 1); end });
      end,
   },
   -- L-shaped, big
   {
      name = "L-shaped, big",
      frequency = 5,
      contents = function()
         des.map({ map = [[
-------xxxxxx
|.....|xxxxxx
|.....|xxxxxx
|.....|xxxxxx
|.....|xxxxxx
|.....-------
|...........|
|...........|
|...........|
|...........|
|...........|
-------------]], contents = function(m) filler_region(3, 3); end });
      end,
   },
   -- with a corner room (top right)
   {
      name = "Corner room, top right",
      frequency = 5,
      contents = function()
         des.room({ type = "themed", w = 6, h = 6, filled = 1,
                    contents = function()
            local walls = { "south", "west" };
            des.room({ type = "ordinary", x = 4, y = 0, w = 2, h = 2,
                       contents = function()
               des.door({ state = percent(50) and "secret" or "random",
                          wall = walls[math.random(#walls)] });
            end });
         end });
      end,
   },
   -- with a corner room (bottom left)
   {
      name = "Corner room, bottom left",
      frequency = 5,
      contents = function()
         des.room({ type = "themed", w = 6, h = 6, filled = 1,
                    contents = function()
            local walls = { "north", "east" };
            des.room({ type = "ordinary", x = 0, y = 4, w = 2, h = 2,
                       contents = function()
               des.door({ state = percent(50) and "secret" or "random",
                          wall = walls[math.random(#walls)] });
            end });
         end });
      end,
   },
   -- three rooms (right side)
   -- (master's 6x6 shape lets the two subrooms and their walls cover
   -- the parent's entire east wall; 5.0's corridor join refuses a wall
   -- whose inner tile belongs to a subroom instead of forcing a door
   -- through it, so the parent could generate sealed. h = 8 keeps one
   -- aisle row on that wall - the Twin businesses rule above)
   {
      name = "Three rooms, right",
      frequency = 5,
      contents = function()
         des.room({ type = "themed", w = 6, h = 8, filled = 1,
                    contents = function()
            des.room({ type = "ordinary", x = 4, y = 0, w = 2, h = 2,
                       contents = function()
               des.door({ state = percent(50) and "secret" or "random",
                          wall = "west" });
            end });
            local walls = { "north", "west" };
            des.room({ type = "ordinary", x = 4, y = 5, w = 2, h = 3,
                       contents = function()
               des.door({ state = percent(50) and "secret" or "random",
                          wall = walls[math.random(#walls)] });
            end });
         end });
      end,
   },
   -- three rooms (left side)
   {
      name = "Three rooms, left",
      frequency = 5,
      contents = function()
         des.room({ type = "themed", w = 6, h = 8, filled = 1,
                    contents = function()
            des.room({ type = "ordinary", x = 0, y = 6, w = 2, h = 2,
                       contents = function()
               des.door({ state = percent(50) and "secret" or "random",
                          wall = "east" });
            end });
            local walls = { "south", "east" };
            des.room({ type = "ordinary", x = 0, y = 0, w = 2, h = 3,
                       contents = function()
               des.door({ state = percent(50) and "secret" or "random",
                          wall = walls[math.random(#walls)] });
            end });
         end });
      end,
   },
   -- house of spears
   {
      name = "House of spears",
      frequency = 5,
      mindiff = 4,
      contents = function()
         des.room({ type = "themed", filled = 1,
                    contents = function()
            for i = 1, d(4) + 3 do
               des.trap("spear");
            end
         end });
      end,
   },
   -- house of horrors
   {
      name = "House of horrors",
      frequency = 5,
      mindiff = 14,
      contents = function()
         des.room({ type = "themed", w = 6, h = 6, filled = 1,
                    contents = function()
            local terr = { "F", "L", "P" };
            local pick = terr[math.random(#terr)];
            for x = 0, 5 do
               des.terrain(x, 2, pick);
            end
            des.monster("shambling horror");
            if (percent(10)) then
               des.monster("shambling horror");
            end
         end });
      end,
   },
   -- room with dangerous monster and loot
   {
      name = "Dangerous lair",
      frequency = 5,
      mindiff = 15,
      contents = function()
         des.room({ type = "themed", filled = 1,
                    contents = function()
            local mons = { "M", "D", "O", "Z", "T", "H" };
            local cls = mons[math.random(#mons)];
            des.monster({ class = cls });
            if (percent(50)) then des.monster({ class = cls }); end
            if (percent(25)) then des.monster({ class = cls }); end
            des.object();
            if (percent(50)) then des.object(); end
            if (percent(25)) then des.object(); end
            if (percent(10)) then des.object(); end
         end });
      end,
   },
   -- beam me up scotty
   {
      name = "Beam me up scotty",
      frequency = 5,
      mindiff = 12,
      contents = function()
         des.room({ type = "themed", filled = 1,
                    contents = function()
            for i = 1, d(4) + 3 do
               des.trap("magic beam");
            end
         end });
      end,
   },
   -- bazaar: four corner shops
   {
      name = "Bazaar",
      frequency = 5,
      mindiff = 8,
      contents = function()
         -- as with Twin businesses above, the aisle between the corner
         -- shops touches all four walls of the parent room, so corridor
         -- connections always reach it
         local layouts = {
            { "weapon shop", "armor shop", "food shop", "tool shop" },
            { "scroll shop", "potion shop", "wand shop", "book shop" },
            { "armor shop", "ring shop", "weapon shop", "scroll shop" },
            { "tool shop", "food shop", "potion shop", "armor shop" },
         };
         local corners = {
            { x = 0, y = 0, walls = { "south", "east" } },
            { x = 6, y = 0, walls = { "south", "west" } },
            { x = 0, y = 6, walls = { "north", "east" } },
            { x = 6, y = 6, walls = { "north", "west" } },
         };
         des.room({ type = "themed", lit = 1, w = 9, h = 9,
                    contents = function()
            local shops = layouts[math.random(#layouts)];
            for i = 1, 4 do
               local c = corners[i];
               des.room({ type = shops[i], lit = 1, x = c.x, y = c.y,
                          w = 3, h = 3, filled = 1, joined = false,
                          contents = function()
                  des.door({ state = percent(50) and "secret" or "random",
                             wall = c.walls[math.random(#c.walls)] });
               end });
            end
         end });
      end,
   },

};

-- store these at global scope, they will be reinitialized in
-- pre_themerooms_generate
debug_rm_idx = nil
debug_fill_idx = nil

-- Given a point in a themed room, ensure that themed room is stocked with
-- regular room contents.
-- With 30% chance, also give it a random themed fill.
function filler_region(x, y)
   local rmtyp = "ordinary";
   local func = nil;
   if (percent(30)) then
      rmtyp = "themed";
      func = themeroom_fill;
   end
   des.region({ region={x,y,x,y}, type=rmtyp, irregular=true, filled=1, contents = func });
end

function is_eligible(room, mkrm)
   local t = type(room);
   local diff = nh.level_difficulty();
   if (room.mindiff ~= nil and diff < room.mindiff) then
      return false
   elseif (room.maxdiff ~= nil and diff > room.maxdiff) then
      return false
   end
   if (mkrm ~= nil and room.eligible ~= nil) then
      return room.eligible(mkrm);
   end
   return true
end

-- given the name of a themed room or fill, return its index in that array
function lookup_by_name(name, checkfills)
   if name == nil then
      return nil
   end
   if checkfills then
      for i = 1, #themeroom_fills do
         if themeroom_fills[i].name == name then
            return i
         end
      end
   else
      for i = 1, #themerooms do
         if themerooms[i].name == name then
            return i
         end
      end
   end
   return nil
end

-- called repeatedly until the core decides there are enough rooms
function themerooms_generate()
   if debug_rm_idx ~= nil then
      -- room may not be suitable for stairs/portals, so create the "default"
      -- room half of the time
      -- (if the user specified BOTH a room and a fill, presumably they are
      -- interested in what happens when that room gets that fill, so don't
      -- bother generating default-with-fill rooms as happens below)
      local actualrm = lookup_by_name("default", false);
      if percent(50) then
         if is_eligible(themerooms[debug_rm_idx]) then
            actualrm = debug_rm_idx
         else
            pline("Warning: themeroom '"..themerooms[debug_rm_idx].name
                  .."' is ineligible")
         end
      end
      themerooms[actualrm].contents();
      return
   elseif debug_fill_idx ~= nil then
      -- when a fill is requested but not a room, still create the "default"
      -- room half of the time, and "default with themed fill" half of the time
      -- (themeroom_fill will take care of guaranteeing the fill in it)
      local actualrm = lookup_by_name(percent(50) and "Default room with themed fill"
                                                  or "default")
      themerooms[actualrm].contents();
      return
   end
   local pick = nil;
   local total_frequency = 0;
   for i = 1, #themerooms do
      if (type(themerooms[i]) ~= "table") then
         nh.impossible('themed room '..i..' is not a table')
      elseif is_eligible(themerooms[i], nil) then
         -- Reservoir sampling: select one room from the set of eligible rooms,
         -- which may change on different levels because of level difficulty.
         local this_frequency;
         if (themerooms[i].frequency ~= nil) then
            this_frequency = themerooms[i].frequency;
         else
            this_frequency = 1;
         end
         total_frequency = total_frequency + this_frequency;
         -- avoid rn2(0) if a room has freq 0
         if this_frequency > 0 and nh.rn2(total_frequency) < this_frequency then
            pick = i;
         end
      end
   end
   if pick == nil then
      nh.impossible('no eligible themed rooms?')
      return
   end
   themerooms[pick].contents();
end

-- called before any rooms are generated
function pre_themerooms_generate()
   local debug_themerm = nh.debug_themerm(false)
   local debug_fill = nh.debug_themerm(true)
   local xtrainfo = ""
   debug_rm_idx = lookup_by_name(debug_themerm, false)
   debug_fill_idx = lookup_by_name(debug_fill, true)
   if debug_themerm ~= nil and debug_rm_idx == nil then
      if lookup_by_name(debug_themerm, true) ~= nil then
         xtrainfo = "; it is a fill type"
      end
      pline("Warning: themeroom '"..debug_themerm
            .."' not found in themerooms"..xtrainfo, true)
   end
   if debug_fill ~= nil and debug_fill_idx == nil then
      if lookup_by_name(debug_fill, false) ~= nil then
         xtrainfo = "; it is a room type"
      end
      pline("Warning: themeroom fill '"..debug_fill
            .."' not found in themeroom_fills"..xtrainfo, true)
   end
end

-- called after all rooms have been generated
-- but before creating connecting corridors/doors, or filling rooms
function post_themerooms_generate()
end

function themeroom_fill(rm)
   if debug_fill_idx ~= nil then
      if is_eligible(themeroom_fills[debug_fill_idx], rm) then
         themeroom_fills[debug_fill_idx].contents(rm);
      else
         -- ideally this would be a debugpline, not a full pline, and offer
         -- some more context on whether it failed because of difficulty or
         -- because of eligible function returning false; the warning doesn't
         -- necessarily mean anything.
         pline("Warning: fill '"..themeroom_fills[debug_fill_idx].name
               .."' is not eligible in room that generated it")
      end
      return
   end
   local pick = nil;
   local total_frequency = 0;
   for i = 1, #themeroom_fills do
      if (type(themeroom_fills[i]) ~= "table") then
         nh.impossible('themeroom fill '..i..' must be a table')
      elseif is_eligible(themeroom_fills[i], rm) then
         -- Reservoir sampling: select one room from the set of eligible rooms,
         -- which may change on different levels because of level difficulty.
         local this_frequency;
         if (themeroom_fills[i].frequency ~= nil) then
            this_frequency = themeroom_fills[i].frequency;
         else
            this_frequency = 1;
         end
         total_frequency = total_frequency + this_frequency;
         -- avoid rn2(0) if a fill has freq 0
         if this_frequency > 0 and nh.rn2(total_frequency) < this_frequency then
            pick = i;
         end
      end
   end
   if pick == nil then
      nh.impossible('no eligible themed room fills?')
      return
   end
   themeroom_fills[pick].contents(rm);
end

-- postprocess callback: create an engraving pointing at a location
function make_dig_engraving(data)
   local floors = selection.negate():filter_mapchar(".");
   local pos = floors:rndcoord(0);
   local tx = data.x - pos.x - 1;
   local ty = data.y - pos.y;
   local dig = "";
   if (tx == 0 and ty == 0) then
      dig = " here";
   else
      if (tx < 0 or tx > 0) then
         dig = string.format(" %i %s", math.abs(tx), (tx > 0) and "east" or "west");
      end
      if (ty < 0 or ty > 0) then
         dig = dig .. string.format(" %i %s", math.abs(ty), (ty > 0) and "south" or "north");
      end
   end
   des.engraving({ coord = pos, type = "burn", text = "Dig" .. dig });
end

-- postprocess callback: turn room walls into trees
function make_garden_walls(data)
   local sel = data.sel:grow();
   -- change walls to trees
   des.replace_terrain({ selection = sel, fromterrain="w", toterrain = "T" });
   -- update secret doors; attempting to change to AIR will set arboreal flag
   des.replace_terrain({ selection = sel, fromterrain="S", toterrain = "A" });
end

-- postprocess callback: make a trap
function make_a_trap(data)
   if (data.teledest == 1 and data.type == "teleport") then
      local locs = selection.negate():filter_mapchar(".");
      repeat
         data.teledest = locs:rndcoord(1);
      until (data.teledest.x ~= data.coord.x and data.teledest.y ~= data.coord.y);
   end
   des.trap(data);
end

-- called once after the whole level has been generated
function post_level_generate()
   for i, v in ipairs(postprocess) do
      v.handler(v.data);
   end
   postprocess = { };
end
