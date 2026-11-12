-- $NHDT-Header: NetHack 5.0 dungeon.lua 2026/06/20_22:35:38 NetHack-5.0:1.13 $
-- Copyright (c) 1990-1995 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- The dungeon description file.
dungeon = {
   {
      name = "The Dungeons of Doom",
      bonetag = "D",
      base = 25,
      range = 5,
      alignment = "unaligned",
      themerooms = "themerms.lua",
      branches = {
         {
            name = "The Gnomish Mines",
            base = 2,
            range = 3
         },
         {
            name = "Goblin Town",
            base = 2,
            range = 2,
            branchtype = "portal"
         },
         {
            -- the portal is on the level directly above Medusa's
            name = "The Ice Queen's Realm",
            chainlevel = "medusa",
            base = -1,
            branchtype = "portal"
         },
         {
            name = "Sokoban",
            chainlevel = "oracle",
            base = 1,
            direction = "up"
         },
         {
            name = "The Quest",
            chainlevel = "oracle",
            base = 6,
            range = 2,
            branchtype = "portal"
         },
         {
            name = "Fort Ludios",
            base = 18,
            range = 4,
            branchtype = "portal"
         },
         {
            name = "Gehennom",
            chainlevel = "castle",
            base = 0,
            branchtype = "no_down"
         },
         {
            name = "The Elemental Planes",
            base = 1,
            branchtype = "no_down",
            direction = "up"
         }
      },
      levels = {
         {
            name = "rogue",
            bonetag = "R",
            base = 15,
            range = 4,
            flags = "roguelike",
         },
         {
            name = "oracle",
            bonetag = "O",
            base = 5,
            range = 5,
            nlevels = 3,
            alignment = "neutral"
         },
         {
            name = "bigrm",
            bonetag = "B",
            base = 10,
            range = 3,
            chance = 40,
            nlevels = 13
         },
         {
            name = "nymph",
            bonetag = "N",
            base = 8,
            range = 12,
            chance = 45
         },
         {
            name = "forest",
            bonetag = "J",
            base = 12,
            range = 10,
            chance = 40
         },
         {
            name = "medusa",
            base = -5,
            range = 4,
            nlevels = 4,
            alignment = "chaotic"
         },
         {
            name = "castle",
            base = -1,
            nlevels = 3
         }
      }
   },
   {
      name = "Gehennom",
      bonetag = "G",
      base = 16,
      range = 5,
      flags = { "mazelike", "hellish" },
      lvlfill = "hellfill",
      alignment = "noalign",
      branches = {
         {
            name = "Vlad's Tower",
            base = 8,
            range = 5,
            direction = "down"
         },
         {
            -- the chainlevel lookup only searches levels declared before
            -- this branch's position in file order (init_dungeon's chain
            -- scan bound includes the branch index); keep this entry
            -- after Vlad's so wizportal at the end of levels stays in
            -- range, or move wizportal earlier if this list is reordered
            name = "The Wizard's Tower",
            chainlevel = "wizportal",
            base = 0,
            branchtype = "portal",
            direction = "up"
         },
         {
            -- the portal is inside Lucifer's chamber on the sanctum
            name = "Purgatory",
            chainlevel = "sanctum",
            base = 0,
            branchtype = "portal",
            direction = "up"
         },
         {
            -- the valley-side portal is hand-placed by valley.lua
            -- (the level's auto-branch slot is already taken)
            name = "The Hidden Dungeon",
            chainlevel = "valley",
            base = 0,
            branchtype = "portal"
         },
         {
            -- one level below the valley, on an ordinary maze level
            -- whose auto-branch slot is free; find_branch_room falls
            -- back to a corridor tile there
            name = "Vecna's Domain",
            chainlevel = "valley",
            base = 1,
            branchtype = "portal"
         }
      },
      levels = {
         {
            name = "valley",
            bonetag = "V",
            base = 1
         },
         {
            name = "sanctum",
            base = -1
         },
         {
            name = "hella",
            bonetag = "A",
            base = 3,
            range = 2,
            nlevels = 4
         },
         {
            name = "hellb",
            bonetag = "B",
            base = 6,
            range = 2,
            nlevels = 3
         },
         {
            name = "hellc",
            bonetag = "C",
            base = 12,
            range = 2,
            nlevels = 3
         },
         {
            name = "orcus",
            bonetag = "O",
            base = 9,
            range = 2
         },
         {
            name = "wizportal",
            bonetag = "F",
            base = -3
         },
      }
   },
   {
      name = "The Gnomish Mines",
      bonetag = "M",
      base = 8,
      range = 2,
      alignment = "lawful",
      flags = { "mazelike" },
      lvlfill = "minefill",
      levels = {
         {
            name = "minetn",
            bonetag = "T",
            base = 3,
            range = 2,
            nlevels = 8,
            flags = "town"
         },
         {
            name = "minend",
--          5.0.0: minend changed to no-bones to simplify achievement tracking
--          bonetag = "E"
            base = -1,
            nlevels = 5
         },
      }
   },
   {
      name = "Goblin Town",
      base = 2,
      alignment = "chaotic",
      flags = { "mazelike", "gtown" },
      levels = {
         {
            name = "goblintown-1",
            base = 1,
            nlevels = 2
         },
         {
            name = "goblintown-2",
            base = 2
         },
      }
   },
   {
      name = "The Ice Queen's Realm",
      base = 3,
      alignment = "unaligned",
      flags = { "mazelike", "iceq" },
      levels = {
         {
            name = "icequeen-1",
            base = 1
         },
         {
            name = "icequeen-2",
            base = 2
         },
         {
            name = "icequeen-3",
            base = 3
         },
      }
   },
   {
      -- ENTRY -1: arrival from the sanctum portal is at purgatory-2
      name = "Purgatory",
      bonetag = "P",
      base = 2,
      alignment = "neutral",
      entry = -1,
      flags = { "mazelike", "purg" },
      levels = {
         {
            name = "purgatory-1",
            base = 1
         },
         {
            name = "purgatory-2",
            base = 2
         },
      }
   },
   {
      -- no bonetag: no bones anywhere in this dungeon (master parity)
      name = "The Hidden Dungeon",
      base = 1,
      alignment = "chaotic",
      flags = { "mazelike", "hdgn" },
      levels = {
         {
            name = "hdgn",
            base = 1
         },
      }
   },
   {
      -- no bonetag: no bones anywhere in this dungeon (master parity)
      name = "Vecna's Domain",
      base = 3,
      alignment = "chaotic",
      flags = { "mazelike", "vecnad" },
      levels = {
         {
            name = "vecna-1",
            base = 1
         },
         {
            name = "vecna-2",
            base = 2
         },
         {
            name = "vecna-3",
            base = 3
         },
      }
   },
   {
      name = "The Quest",
      bonetag = "Q",
      base = 5,
      range = 2,
      levels = {
         {
            name = "x-strt",
            base = 1,
            range = 1
         },
         {
            name = "x-loca",
            bonetag = "L",
            base = 3,
            range = 1
         },
         {
            name = "x-goal",
            base = -1
         },
      }
   },
   {
      name = "Sokoban",
      base = 4,
      alignment = "neutral",
      flags = { "mazelike" },
      entry = -1,
      levels = {
         {
            name = "soko1",
            base = 1,
            nlevels = 6
         },
         {
            name = "soko2",
            base = 2,
            nlevels = 6
         },
         {
            name = "soko3",
            base = 3,
            nlevels = 6
         },
         {
            name = "soko4",
            base = 4,
            nlevels = 6
         },
      }
   },
   {
      name = "Fort Ludios",
      base = 1,
      bonetag = "K",
      flags = { "mazelike" },
      alignment = "unaligned",
      levels = {
         {
            name = "knox",
            bonetag = "K",
            base = -1,
            nlevels = 3
         }
      }
   },
   {
      name = "Vlad's Tower",
      base = 4,
      bonetag = "T",
      protofile = "tower",
      alignment = "chaotic",
      flags = { "mazelike" },
      entry = -1,
      levels = {
         {
            name = "tower1",
            base = 1
         },
         {
            name = "tower2",
            base = 2
         },
         {
            name = "tower3",
            base = 3
         },
         {
            name = "tower4",
            base = 4
         },
      }
   },
   {
      name = "The Wizard's Tower",
      base = 3,
      bonetag = "W",
      alignment = "chaotic",
      flags = { "mazelike" },
      entry = 3,
      levels = {
         {
            name = "wizard1",
            base = 1
         },
         {
            name = "wizard2",
            bonetag = "X",
            base = 2
         },
         {
            name = "wizard3",
            bonetag = "Y",
            base = 3
         },
      }
   },
   {
      name = "The Elemental Planes",
      bonetag = "E",
      base = 6,
      alignment = "unaligned",
      flags = { "mazelike" },
      entry = -2,
      levels = {
         {
            name = "astral",
            base = 1
         },
         -- the order of the four elemental planes is randomized per
         -- game; each gets a random slot in levels 2-5 (astral stays
         -- 1, dummy stays 6) and the portal chain simply ascends
         {
            name = "water",
            base = 2,
            range = 4
         },
         {
            name = "fire",
            base = 2,
            range = 4
         },
         {
            name = "air",
            base = 2,
            range = 4
         },
         {
            name = "earth",
            base = 2,
            range = 4
         },
         {
            name = "dummy",
            base = 6
         },
      }
   },
   {
      name = "The Tutorial",
      base = 2,
      flags = { "mazelike", "unconnected" },
      levels = {
         {
            name = "tut-1",
            base = 1,
         },
         {
            name = "tut-2",
            base = 2,
         },
      }
   },
}
