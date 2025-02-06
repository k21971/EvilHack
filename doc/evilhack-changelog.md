## EvilHack Changelog

### Version 0.1.0

First version of EvilHack, which was forked off of NetHack 362-hdf as of October 20th, 2018.
Commits from NetHack 3.6.2 beta (vanilla) will be incorporated on a regular basis so as to
stay current with the 3.6.x codebase.

Version 0.1.0 of EvilHack is for the most part laying down the foundation for content from
other sources to be added (primarily GruntHack and Sporkhack), and for new custom content to
be created and implemented.

The following changes to date are:

- Edits to config.h, patchlevel.h, allmain.src, sysconf, Makefiles and hints files to
  differentiate EvilHack from NetHack
- adjust install script for EvilHack for use on public server
- significant merges of latest 'vanilla' NetHack 3.6.2 code


### Version 0.1.1

- Latest merges from 'vanilla' NetHack 3.6.2 (as of January 1st, 2019)
- Allow two-weaponing with an artifact weapon in each hand
- Allow passive properties of an off-hand magical or artifact weapon to function
- Artifact additions and changes (affects some quest artifacts)
- Changes to the 'Knight' role
- Import of SporkHack and GruntHack monsters (round one)
- New custom made monsters
- Tweaks to some existing monsters
- Descriptive hit messages
- Monster hit point generation modifications
- Various monsters can ride steeds (from SpliceHack)
- Removal of the mysterious force, nasties now spawn at upstairs while traveling up
  with the Amulet
- Enable displacement for shimmering dragons
- Grudge Patch (by Nephi, modified)
- Zombies (from GruntHack, modified)
- Revamp of The Riders
- Descriptive hit messages
- Armor adjustments, racial armor bonues
- Dragon armor and secondary extrinsics (from SporkHack, modified)
- Spell casting while wearing body armor (from SporkHack, modified)
- Pet behavior/AI revamp (from SpliceHack/GruntHack)


### Version 0.1.2

- Latest merges from 'vanilla' NetHack 3.6.2 (as of March 11th, 2019)
- 'Give' extended command (from SpliceHack)
- Displace peaceful monsters (from xNetHack)
- Unicorn horns are one-handed, no longer cure attribute loss
- Magic missile changes (from SporkHack, modified)
- Reflection is only partial (from SporkHack, modified)
- Conflict is based off players charisma score (from SporkHack)
- Telepathy intrinsic changes (from SporkHack, modified)
- Intelligent monsters can loot containers and put gear into carried bags (from GruntHack) 
- Tileset support
- Tweaks to strength and two-weaponing damage/to-hit bonuses
- Changes to blessed genocide scroll behavior (from SporkHack)
- Scrolls of identify are universally known
- Some monsters that are scared will scream (from SporkHack)
- Potion of speed can cure extrinsic 'slow' effect
- Cursed amulet of life saving will fail (from SporkHack, modified)
- Racial shopkeepers (from SporkHack, modified)
- Changed color of mind flayers and the Wizard of Yendor
- more changes to zombies
- Giant as a playable race (from GruntHack/SliceHack, modified)
- Boulders in inventory stack, and can be quivered/fired if able
- more adjustments to the 'grudge patch'
- Altar sacrificing changes (from SporkHack, modified)
- Colored walls and floors patch
- New intrinsic: Food Sense
- Hobbit as a playable race
- Centaurs as a playable race (adjusted armor choices for both player and monster)
- All roles can train riding to basic
- Changes to spell level difficulty
- New spells (reflection, lightning, acid blast, poison blast, repair armor)
  (from SporkHack and GruntHack)
- Dexterity affects your AC
- Monks get some love
- New wizard starting pet (pseudodragon)
- Pick-axe skill affects how fast you can dig
- Intelligent monsters can loot portable containers
- Owned artifact patch
- Tweaks to higher-level monsters
- New monster spells; revamp of others (notably 'destroy armor')
- Monster stoning behaves like player stoning
- Exploding bag of holding does not destroy everything
- New container: iron safe (from SporkHack)
- Unlock your quest by killing your quest leader if you made them angry
- More artifact revamps
- Unskilled/restricted weapon use feedback
- Killer can go through your possessions upon death (affects bones)
- New items; changes to existing items
- Invisibility/see invisible is no longer permanent in most cases
- Diluted potions have diminished effects
- Pets will stop eating once satiated
- Izchak changes
- Sokoban End revamp
- New item: amulet of flying (from UnNetHack)
- Monster Ring patch
- More new monster spells


### Version 0.2.0

- Latest merges from 'vanilla' NetHack 3.6.2 (as of April 6th, 2019)
- Object Materials patch (from xNetHack)
- Several tweaks and adjustments to various things to take the new object materials patch
  into account
- New monsters: elven wizard, hobbit rogue
- Add some flavor to the Magic 8-Ball
- Magic markers no longer randomly generated, chance of one being found in Sokoban
- Two-weaponing artifacts is back in, but with restrictions (lawful and chaotic don't play
  nice together)
- Artifact changes: Fire/Frost brand are now short swords, made of mithril
- Spellbook weight determined by its level
- Leprechauns will steal anything made of gold (from xNetHack)
- Melee combat can wake up nearby monsters
- Wish tracker (from SporkHack)
- Intelligent monsters can use wands of wishing (from GruntHack/SpliceHack)
- Female counterparts to several monster lord/king types (lady/queen)
- Racial soldiers: major change to soldier/sergeant/lieutenant/captain monsters, can be several
  different humanoid races
- Major mplayer overhaul: spellcasters can cast spells, rogues can steal, all mplayers can and
  will steal the Amulet, kitted out with ascension-kit level gear appropriate for their level
- Chest traps can now affect looting monsters the same way as the player is affected
- Monsters with low AC enjoy damage reduction much like the player does
- Updated monsters using wands logic (offensively use wands of polymorph)
- Monster can sacrifice the Amulet of Yendor
- Much larger array of female names for female mplayers
- Spellcasting monsters can cast 'stone to flesh' on themselves
- Monster AI improvements (melee and object use)
- Reduce how much luck affects to-hit
- Additional monster spell tweaks
- Monsters can recognize a resistance the player has, and can adjust attacks accordingly
- Beholder tweaks and changes (stoning gaze is no longer instant, cannot be genocided,
  are generated asleep 80% of the time)
- Medusa revamp and overhaul: a more worthy opponent now
- Magical eye now has a luck diminishing attack
- Luck timeout is slowed instead of halted by luckstones
- Additional monster MR2 flags: water walking and jumping
- Monsters can get displacement from armor that grants it
- Random wands of wishing are generated already recharged once
- Tweaks to the Wizard of Yendor and the summon nasties list
- Blessed stethoscopes can reveal egg contents
- Wresting a wand is directly affected by its BUC status
- Livelog additions and updates


### Version 0.2.1

- Latest merges from 'vanilla' NetHack 3.6.2 (as of April 12th, 2019)
- Significant changes to some of the gnomish mines towns (especially Orcish Town)
- Iron bars can be blown apart by wands of striking or force bolt spell
- Booby-trapped doors actually explode and can cause fire damage
- Several new traps added
- Dipping for Excalibur livelogging got a much-needed refresh
- The gnomish mines have rivers running through them (and 1 out of 100 times,
  it's lava instead of water)
- Default castle changes, new castles added to the rotation
- New Fort Ludios maps
- Changes and tweaks to the Valley of the Dead
- Major Gehennom revamp (mines-style levels with lots of lava instead of random mazes)
- All demon lords and princes have their own special level now in Gehennom. Yes, you have
  to deal with Demogorgon
- Special room generation/random 'vaults' added to the regular dungeon
- New mines end map - Orc Temple (by Khor)
- Several new livelogged events (killing another players' ghost, BoH exploding, sokoban
  tweak, stealing from a shop)
- Wands of light as well as casting the spell or reading a scroll of light has a 20% chance
  of blinding any monsters within range that can see
- Viewing monsters via farlook provides a bit more basic information
- Players cannot successfully engrave Elbereth until they have learned it in-game (thank you Tangles)
- Various bug fixes and code clean-up throughout


### Version 0.3.0

- Latest merges from 'vanilla' NetHack 3.6.2 (as of April 16th, 2019)
- Zombie movement speed and corpse revival time adjusted
- Significant changes to zombie claw/bite attack and zombie sickness
- Peaceful monsters no longer get angry if damaged by a hostile monsters AoE spell
- Eating a zombie corpse infects you with their zombie sickness
- Multitude of various fixes to player races, monster players, livelogging, tool use,
  random vault room selection, rivers in the gnomish mines, and descriptive hit messages


### Version 0.3.1

- Latest merges from 'vanilla' NetHack 3.6.2 (as of May 5th, 2019)
- Another major fix for mplayer monsters using the correct rank based on their role
  (more of a kludge, but it works)
- Fix racial shopkeeper pricing (mind flayers were only charging 3 zm for anything,
  couple other tweaks)
- Bribe amounts from demon lords/princes significantly increased to something much more realistic
- New feature: player can view item weight in inventory (off by default, use OPTIONS=invweight
  in rc config to turn on)
- Fixes concerning eating magical eyes (confers luck)
- Excalibur livelog tweaks
- Player can cure extrinsic slow with a wand of speed monster
- Fix: vibrating square maze was not loading, allowing the player to skip the invocation and
  go straight to the sanctum
- Fix: really odd pet eating behavior (disabled code section imported from GruntHack)
- Fix: special instakill/heavy damage attacks from certain artifacts (was applying to 
  all monsters and not their intended target race only)
- Add demon lords/princes to the list of monsters who are not affected via magical scaring
- Change the Barbarian quest artifact base ring type from stealth to free action
- Magic traps can confer permanent see invisible (on/off) as well as invisibility
- Medusa revamp, round two (based on player feedback and some of my own ideas)
- New dungeon feature: forges - player interacts with them just like they would a fountain,
  but instead of water, it's molten lava. See the 'forges' commit for what all it can do
- Fix: instakill from poison vs poison reistance percentage
- Fix: 'turning into a zombie' status not showing when checking yourself via wand of probing
  or a stethoscope
- Added more rumors (mostly true)
- Fix: 2nd attack for GM monk so cockatrices don't instapetrify when wearing gloves
- Fix: 'placing monster over another' and 'can't place branch' impossibe messages
- Add inventory weight/carrying cap and number of slots left to inventory display
- Fix: 'named object not in disco' when forgetting known objects (amnesia or brain eaten)
- Tweak pseudodragon/elder pseudodragon behavior in regards to items
- Fix: monsters using a bag of tricks would show as '<monster> reads a bag of tricks'
- Fix: properly map steed ids when loading bones files to prevent panic()
- Fix: priests and shopkleepers stepping on zombies


### Version 0.3.2

- Latest merges from 'vanilla' NetHack 3.6.3 'work-in-progress' (as of August 31st, 2019)
- Fix: correctly handle player and steed getting polymorphed on a trap
- Change status lines' "End Game" behavior back to old method
- Elder minotaurs will spawn with wands of digging
- Fix: displaced monsters attacking non-displaced would behave as if the non-displaced
  monster was the one with displacement
- Tweaks to the Red Horse
- Added a third tier to the wizard starting pet pseudodragon
- New monsters: enormous rat, rodent of unusual size, rabid dog, jumping spider,
  honey badger
- Random dragonhide-based armor is a bit more rare to find
- Fix: eating a tengu corpse gave the player teleportis and teleport control
  immediately, first time
- Proper feedback when trying to destroy iron bars with a wand of striking/spell
  of force bolt and can't
- Fix: adjusted behavior of spellcasting monsters using stone to flesh on themselves
- Randomize the message received when the player dies and the attacking monster
  starts to go through their possessions
- Add feedback when zapping a wand of cancellation at a monster
- New monster spell: MGC_CANCELLATION
- Fix: monsters zapped by a wand of polymorph wielded by another monster would not
  change form and/or die
- Fix: The Ring of P'hul can now be wished for
- Added an EvilHack splash screen for curses mode
- Tweak castle-2 level
- Fix: impossible when monster's armor gets destroyed by a spell
- Updated the item array for what monsters will wish for
- Tweaks to spear traps
- Adjust pet behavior is regards to zombie corpses
- Record killing of unique monsters, other special monster types in the xlogfile
  (primarily for the JunetHack tournament)
- Twoweapon patch
- Priests are penalized for using edged weapons
- Most mplayers will spawn with a key or other unlocking tool
- Fix: force bolt/wand of striking feedback when hitting iron bars
- Room generation code clean up
- Sokoban end revamp (round three)
- Change livelogging of killing a players bones ghost to any creature they rise as
- Shopkeeper race selection improvements
- Minor zombie tweaks
- Fix: temporary fix preventing segfault when splitting puddings and their corpses
  are absorbed into another corpse
- Adjustments to what players receive via altar sacrificing (regular items)
- Fix: giant player race wasn't getting hungerless regeneration intrinsic
- Fix: a burnt partly eaten <corpse name here>
- Nazgul and lichen tweaks
- Fix: monsters engulfing other monsters off of their steeds
- Fix: Monster steeds in bones files are detatched from their riders
- Adjust zombie movement speed and claw attack
- Fix: handling of weapon damage based on material type
- More Izchak tweaks
- Specific livelog message for 'killing' Death
- Fix: segfault when monster player has the Amulet of Yendor and the Wizard of Yendor appears
- Excalibur and Dirge never have a negative enchantment
- Fix: Monk triggering 'hitting with a wielded weapon for first time' conduct when fighting
  bare-handed but weapon was flagged as 'alternate weapon; not wielded'
- Change various objects base material from iron to generic metal
- Fix: tame high-level spellcasting monster pets would still cast MGC_CANCELLATION at the hero
- Tweak starting gear if playing Giant race
- More spell casting while wearing body armor revamps
- Fix: Prevent intelligent monsters from unlocking an iron safe with keys/lock picks/credit cards
- Fix: Boojum AD_TLPT attack wasn't teleporting the hero
- Fix: change The Sword of Bheleu's base material from mithril to gemstone
- Fix: Monks did not have weapon skill restriction lifted when receiving an artifact gift weapon
- Another Monk weapon skills revamp
- Fix: being killed by an exploding forge was displaying the wrong 'killed by' message
- Fix: livelog output when the player blows up their bag of holding
- Fix: spear traps should not be able to reach the hero if they are flying
- Quest leaders nor nemesis can be frightened by magical means
- Monsters that revive (trolls, zombies) will not revive if they lose their heads
- Fix: spear traps properly handle steeds
- Added a random set of death messages from being killed by a spear trap
- Artifact properties when in offhand and two-weaponing are active
- Limit amount of times a wand of death can be recharged
- Putting Magicbane into a bag of holding has a chance of causing the bag to explode
- Wands of cancellation self-identify when the player observes their use and effects
- Player can see colored flashes when dropping a container on an altar
- Objects can be completely destroyed via rusting/rotting/corroding
- Fix: broken 'killed by' format when the player dies due to eating a zombie corpse
- Unique death messages when killed by a water elemental or gelatinous cube while engulfed
- Improved feedback when cancelling a monster
- Hobbits can eat significantly more food before becoming satiated
- Tweaks to Hobbit eating/hunger feedback
- Fix: opening a trapped tin while in Sokoban would trip the doors at Sokoban End
- Neutral aligned summon minion has small chance of producing something other than an elemental
- Fix: Monsters that have an aversion to a certain material should never spawn with an object made of that material
- Fix: disallow obtaining Excalibur or Dirge by using a helm of opposite alignment
- Magic lamps can be wished for
- Fix: segfault when playing as priest and attacking a monster bare-handed
- Several changes to dragon scales/scale mail secondary extrinsics (humanoid monsters wearing same armor have most
  DSM extrinsics as do dragons themselves, BDSM can disintegrate you or certain inventory objects,
  GDSM has a cancellation secondary extrinsic)
- Fix: several issues regarding monster steeds
- Medusa's stoning bite can be cancelled
- Tame vampires no longer revert to fog/animal form
- Fix: magic marker not showing up as 'sokoban prize tool'
- Allow any slash/pierce weapon to be poisonable
- Improved logic concerning passive attacks
- Unique death messages when engulfed or digested
- Elves (player and monster) have a true aversion to iron
- Orcs (player and monster) have a true aversion to mithril
- Additional logic to compel monsters in possession of the Amulet of Yendor to go sacrifice it if on the Astral Plane
- Primary spellcasters can be gifted spellbooks via altar sacrificing
- Allow Rogue's 'strike from behind' damage bonus while twoweaponing
- Monsters will fire their ranged weapon at melee range if no other option exists
- Player races elves and orcs will not received items from their deity made of a hated materail
- Beholders will always show as a 5 via warning
- Tame spellcasting pets should not summon nasties
- Amulet of life saving will not work if the wearer is non-living (player and monster)
- Skills flagged as > in the #enhance menu require just a bit more practice to advance
- Small chance the game will continue if the player rises from the grave (undead)
- HTML dumplogs
- Fooproofed items have a chance to resist being disintegrated if they come in contact with a black dragon,
  any invocation item and the Amulet of Yendor are immune to being disintegrated
- Mithril, bone and mineral materials can grant various levels of MC to elves/orcs
- Rogue new skill: Thievery
- Monk new ability: break boulders or statues using martial arts
- Add materials to list of helmet objects that can protect you from falling objects
- Improvements to the 'Owned Artifact' patch
- Fixes and tweaks to passive disintegration attack
- Fix: Hobbit player race receiving 'this satiates your stomach' too early
- Fix: iron hook spawning with a different material other than iron
- Monster spells cancellation, fire bolt and ice bolt require that their target is lined up
- HTML dumplogs - adjust object/terrain colors, add font for windows users
- Magical staves keep their base material when given to the player via altar sacrifice
- Fix: a burnt partly eaten <corpse name here> (last time)
- Fix: 'dealloc_obj: obj not free' crash when a bag of holding explodes with gold pieces inside of it
- Fix: intelligent monsters going through locked containers when not seen
- Fix: 'impossible: Can't place branch!' panic messages in Gehennom
- Monster player tweaks and fixes (Monks finally spawn with gear, Knights steed randomized)
- Fix: 'impossible - breaking non-equipped glass obj? Program in disorder!' when wielding
  and attacking with a stack of glass weapons
- Fix: game crash when last rustable/corrodible object in player inventory rusts/corrodes
  away and is gone
- Player spell 'haste self' can cure slow effect
- Improved feedback when a monster activates a figurine
- Fix: restricted weapon penalty messages when using a different form of attack
- Tweak dfficultly level of goblin outriders / goblin-captains
- Fix: Intelligent pet code issues
- Player monsters or any covetous monsters will grudge any monster that has
  the Amulet of Yendor
- A mounted steed will attack hostiles on its own without having to be attacked first
  if its tameness is high enough
- Player race orc can ride tame wargs


### Version 0.4.0

- Latest merges from 'vanilla' NetHack 3.6.3 'beta' (as of December 1st, 2019)
- Amulet material fixes (by ogmobot)
- Object properties patch
- Adjust material frequency for items regularly made of iron/metal, orcish gear, dwarvish weapons
- New weapons/armor (orcish scimitar, orcish boots, regular gauntlets
- Fix: crash due to various dragon scales/scale mail and Dragonbane passive attack
- Healers gain sickness resistance intrinsic at experience level 15
- Fix: inventory weight and available/total slots showing up as a menu selection
- Fix: player races that hated a certain material would still be adversely affected while
  twoweaponing and wearing gloves
- Dirge does proper acid damage to worn armor
- Prevent monsters' starting gear from being a material they hate
- Fix: not all cases were covered in regards to protecting the players (or monsters) head
  for a hard helmet made of a hard material.
- Heavy objects falling on your head do more damage than normal
- Glass helmets when worn can shatter if a heavy object falls on the players head
- When polymorphed into forms that have certain attacks, don't use those attacks under
  certain conditions
- New player race: Illithid
- Fix: MR_PSYCHIC was not applied correctly for all monsters with the flag
- Fix: panic when trying to wear orcish boots
- Fix: the <weapon> welds itself to the hobbit rogue's claw!
- Add tentacle brain-eating attack to Illithid player race
- Make shimmering scales use toggle_stealth and toggle_displacement (by ogmobot)
- Gold DSM shouldn't stop glowing if it hasn't yet been worn (by ogmobot)
- Adjust Illithid tentacle attack damage based on experience level
- New race/role combinations (dwarf barbarian, samurai or wizard, giant wizard)
- New player role: Convict
- Add Luck Blade to the array of chaotic artifacts
- Allow the Iron Ball of Liberation to be wished for
- Fix: cursed amulet of life saving could appear in bones file after use
- Illithid player race fixes (by ogmobot)
- Slight adjustments to mines' end 'Orc Temple'
- Reduce amount of feedback received with fighting with a weapon that requires more practice
  to train or is restricted
- High priest in the sanctum has a magic marker in inventory
- Fix: various intelligent monster AI issues
- Fix: monster steeds lagging behind one turn when it and its rider are teleported
- Fix: incorrect feedback when invisible monster rides a steed
- Forge revamp
- Fix: crash bug introduced from last commit (tame lava demon from forge-dipping)
- Dropping a ring of polymorph down a sink has a chance of creating a forge
- Forge tweaks - extra feedback when dipping and nothing of consequence happens,
  slightly increased odds again of repairing damage to metal objects
- Illithid race not allowed to play healer role
- Create three levels of the Dark Knight's steed, the 'nightmare' (lesser nightmare,
  nightmare, cauchemar)
- Adjust stats of all three levels of pseudodragon
- Bugbear edit (now S_ORC as it should have been)
- New vampire types
- Fix: crash when trying to check steed of invisible monster (by ogmobot)
- Owlbear makeover
- Lava demons like lava
- Use set_material for Croesus' gear (by ogmobot)
- New monster: Sea dragon
- New objects: Helm of speed, amulet of magic resistance
- Two sokoban end prizes changed
- New object: gauntlets of protection
- Readjust dwarvish/elven chain mail MC
- The Eyes of the Overworld protect against several gaze attacks
- Monks wearing body armor penalty actually means something (penalty is more severe)
- Player feedback for monks wearing/removing body armor
- Artifact weapons used against black drgaons have much greater chance
  of not being disintegrated
- Killing a dragon while engulfed by it will produce a dragon corpse
- Tame dragons don't drop scales
- Adjust shopkeepers' base level to 13
- New monster: Woolly Mammoth
- Elephant-like monsters gain a gripping trunk attack
- Mumak revamp
- Revamp of the monster HP modification code (from xNetHack)
- Ghost overhaul
- Suppress livelogging of the Red Horse being killed
- Adjust spear trap damage based on level depth
- Fix: how helmets are supposed to block attacks to your head
- Dragonbane is immune to black dragon's passive disintegration
- Fix: better feedback for 'you hear something being opened'
- Mindless monsters are unaffected by any means of scaring
- Fix: breaking a statue that had contents would not clear cobj
- The Lady of the Lake wants your sword
- Reverting back to wands of death can only be recharged once before exploding
- Hobbit rogues, human rogues always have gold on them
- Croesus can move other monsters out of his way
- Nazgul scream attack can potentially shatter glass objects
- Sokoban end: walls that replace disppearing doors prevent phasing
- Fix: if player has an object that is already ID'ed and that object is also
  one of the sokoban prizes, don't show that sokoban prize object as
  known until it's actually been picked up by the player.
- Wand of death/finger of death spell is not completely negated by magic
- Monsters with magic resistance are affected the same way as the player
  when hit by a wand of death/finger of death spell
- Livelog ID'ed sokoban prize and not 'sokoban prize object'
- More scenarios where fire damage can destroy objects
- Less chance of waking monsters in combat if stealthy
- New conducts
- Don't break petless conduct upon entering the Astral Plane
- New object: crystal chest
- Fix: artifactless conduct wasn't broken by kicking an artifact
- New monster: giant centipede (hi Grasshopper!)
- Fix: msize bug with shambling horror
- Fix: player monsters taking the wrong rank when created
- Tweak damage from iron/mithril as a hated material
- Additional missing monsters added to monsters.txt
- Correct numbered order of objects in other.txt
- Tweak making friendlies angry when blinding them
- Silencing compiler warnings
- Fix: issues when Death attacks another monster
- Add in missing code from GruntHack in regards to monsters charging wands
- Address some minor issues with spellcasting while wearing body armor
- Fix: ogresmasher and trollsbane special attack was not working
- Fix: don't need to wear a hat to wield Excalibur
- Fix: Rotten food is no longer burnt
- Added psionic resistance to enlightenment feedback
- Better feedback when hitting a black dragon with something disintegration-proof
- Clean up function call to add_erosion_words
- Ensure monsters with invisible steeds are mapped properly
- Better feedback when looking at an invisible monster riding a steed
- Attempt to improve monster AI logic in regards to charging wands
- Update install scripts for version 0.4.0
- Establish a distinct debug mode hints file
- Fix: monsters and tame pets
- Fix: launcher and ammo tweaks to object properties
- Fix: launchers should not spawn as 'poisoned'
- Fix: Your crystal plate mail stone deflects its attack
- Some monsters go berserk (added new M3_BERSERK flag)
- Fix: you multiply from its heat!
- Arise as a barrow wight if killed by a Nazgul
- Minimum dungeon level depth set for random vaults (by paxed)
- Convict's striped shirt can be read
- Fix: variant type title (HTML dumplogs)
- Tweak pet satiation again
- Don't need to wear a hat to wield Dirge either...
- Fix: SPFX_WARN artifacts without a glow color specified shouldn't glow
- Fix: shambling horror settings
- Tweaks to random vaults again
- Another shambling horror revamp
- Tweak shambling horror damage output a bit
- Add some shambling horrors to the sanctum
- Material damage for contact with monsters made of a hated material
- Partial fix: attacking certain monsters with tentacle/bite attack
- Blinding monsters with a wand/scroll/spell of light only happens at a close distance
  to the player
- Naming/wishing for artifacts tweaks
- Tweak hard material helmet deflection against attacks to the players head
- Lower the odds of receiving a wish from the magic 8-ball
- Tweak chance appearance of special neutral minion
- Fix: crash when monster hits another monster under certain conditions
- Fix: HTML dumplog display errors
- Fix: crash when removing starting t-shirt (object properties)
- Monks do not utilize their extra attack at grand master martial arts skill
  if wearing a shield
- Floating eyes can regenerate hit points
- Clean up some behavior and feedback when naming Sting or Orcrist
- Fix: impossible when monster's armor or weapon erodes away when hero is blind
- Fix: double-free or corruption crash (ball & chain erodes completely away)
- Fix: panic 'explosion base type 8?' caused by monster spell 'acid blast'
- Fix: Convicts aren't supposed to spawn with anything extra in inventory
- Fix: proper feedback when eating zombie corpses while having sickness resistance
- Monster steeds will spawn wearing a saddle if they are allowed to wear one


### Version 0.4.1

- Latest merges from 'vanilla' NetHack 3.6.3 official release (as of December 17th, 2019)
- Prep for version 0.4.1
- Zombies can make other monsters ill, not just the player
- Grimtooth now has a disease attack
- Fix: correct 'killed by' message if infected by Grimtooth
- Comment correction in include/monst.h
- New monster: queen ant
- Salamanders can pull you into lava
- Fix: white dragons gaining hit points each time its passive attack activated
- Fix: do passive attacks for green/red/white dragons the correct way
- Adjust color of queen ant
- Fix: shopkeepers can talk regardless of their race
- Fix: mysterious force and teleporting with the Amulet of Yendor should be removed for
  monsters as well as the hero
- Convict role starting striped shirt and heavy iron ball are fooproof
- The heavy iron ball generated from reading a scroll of punishment will be rustproof
  50% of the time
- New dungeon features: shallow water and sewage, new monsters: giant cockroach, giant
  leech, giant crocodile, new mines' end map: The Sewers of Waterdeep
- Fix: some formatting issues and small errors with the shallow water patch
- Fix: text-based dumplog was not being generated
- Readjust (down) lower-level zombies' spawn rate
- Convict role starting iron chain needs to be rustproof along with the heavy iron ball
- The iron chain generated with the heavy iron ball from reading a scroll of punishment
  will also be rustproof 50% of the time
- Fix: dipping cursed worn items into sewage
- Fix: intelligent pet patch bug that made monsters with ranged attacks never approach
- Fix: rework Nazgul scream attack
- Fix: monster spell 'stone to flesh' now uses mspec
- Partial fix: heavy iron ball rusting away
- Fix: priests using ammo as a melee weapon
- Fix: objects spawning in unseen shallow pools
- Fix: heavy iron ball rusting away while attached if being dragged on floor
- Reorganization of server-side config and hints files
- Cure sickness is now directional and can be used to cure monsters as well as yourself


### Version 0.4.2

- Latest merges from 'vanilla' NetHack 3.6.5 official release (as of January 27th, 2020)
- Prep for version 0.4.2
- All types of healing potions, unicorn horns, and eucalyptus leaves can cure ill monsters 
- Fix: prevent 'destroyed it' when livelogging destroying the ghost/undead of another player
- Tweak conditions for monsters drinking healing potions when ill
- Fix: description/color issues with the spell 'acid blast'
- AD_PSYC is not a ray type attack
- New unique monster: The Rat King
- Fix: lycanthropy from The Rat King changes you into a wererat
- Fix: autotravel using _ will stop at shallow water and sewage
- Update to the README file
- Boulders have some weight if playing Giant race
- Selecting 'Q' to quiver now counts boulders as an option when playing as Giant race
- Naming Sting or Orcrist has a chance of summoning more than just angry elf
- Fix: objects spawning in unseen shallow pools (again); certain traps and stairs will not
  spawn in same space as shallow water or sewage
- Fix: non-placement of certain traps under shallow water or sewage
- Fix: no for real this time - objects spawning in unseen shallow pools (yet again)
- Fix: sometimes stairs not being generated on levels with lots of shallow water or sewage
- Tweaks to livelogging being killed by bones monster of another player
- Fix: racial shopkeepers attacking peaceful pets, and getting 'you murderer!' when killing
  a shopkeeper that has been turned into a zombie.
- Another tweak to livelogging being killed by bones monster of another player
- Adjust shopkeepers starting gear 
- Partial fix/revert: game freezing on creation of minetn-1 or minetn-6
- Fix: game freezing on creation of minetn-1 or minetn-6
- Spear traps can form under shallow water or sewage
- Fix: Illithid player race aren't supposed to get INT loss/amnesia
- Settings correction for the convict quest artifact 'The Iron Ball of Liberation'
- Tweak livelogging of genocided monsters
- Fix: killed by message when killed via certain monster spells
- Fix: unicorn horns weren't one-handed
- Fix: inventory weight header showing as selectable object in certain inventory windows
- Slightly improve the odds of not getting the owner when wishing for an artifact
- Adjust odds even more in favor of the player when wishing for an artifact, but the
  owner will always be of an appropriate high level no matter what level the player is
- Fix: getting 'the raw sewage boils away' when evaporating shallow pools
- Fix: chest containing the wand of wishing on castle-3.lev would sometimes spawn as trapped
- Tweak mines' end level 'Orc Temple'
- Adjust odds of random object spawning with some type of object property
- Fix: player giants climbing down through trap doors/gaping holes
- Fix: build errors when livelog and paniclog_fmt2 are not defined
- Fix: wizmode wishing - if artifact owner would appear, ask if this is wanted
- Fix: update_inventory() and the spell 'repair armor'
- When invoking the Iron Ball of Liberation, the chain generated when it attaches itself
  to the player is always rustproof
- Fix: player could not phase through objects diagonally via invoking the Iron Ball of
  Liberation if carrying a lot of objects in open inventory or if playing as a giant
- Fix: "You can hear again" when deafness times out while permadeaf
- Fix: permahallucination doesn't abuse wisdom
- Fix: killed by 'the Rat King', not killed by 'Rat King'
- Certain monsters can spawn in the Gnomish Mines' rivers
- Fix: 'Do so? [y/n]' message when moving onto boulders in sokoban as a giant and that
  level is already solved
- Fix: issues with monster spells fire bolt, ice bolt and acid blast
- Fix: Giants aren't supposed to be stealthy. Ever.
- Fix: player role Priest should not receive an edged crowning gift
- Fix: displaying plural form of artifact names as a priest trying to use an edged
  or piercing type artifact weapon
- Revert alignment level for crowning back to normal
- Fix: player role Priest should not receive an edged artifact weapon via
  altar sacrifice
- Fix: boulder feedback as a giant moving into a boulder's location
- Fix: more giant walking onto boulder feedback
- Role selection filtering and fixes
- Adjust feedback when trying to cast spells using #monster
- Adjust how a player could arise as a spectre
- Door traps won't start to appear until dungeon level 13
- Adjust monster generation rates
- Fix: getting immune to sickness feedback from a monster attacking a gray fungus but
  the player not viewing said monster
- Fix: non-magical flutes and harps in src/objects.c
- Fix: obtaining invisibility/see invisible from eating a stalker corpse
- Fix: exploit with sentient_arise() and god destroying your undead form while unchanging
  set no killer
- Fix: no feedback when hitting an unseen monster with various artifact weapons or
  weapons with offensive object properties
- Adjust which monsters are sick resistant; move permonst drain resistance and
  player-style-MR checks to macros
- Fix: The Rat King shouldn't have the M2_PNAME flag
- Fix: having both magic resistance and reflection together didn't stack against
  death rays
- Elven boots shouldn't be made of a hard material
- Fix: Orcs made sick by zombies weren't arising as zombies when they died
- Fix: another artifact wishing revamp
- Being crowned has chance of getting sick resistance
- Tweak monster generation rates again; player can see monster generation rates via
  enlightenment
- Monks can break a monster's wielded weapon under certain conditions
- Fix: monster wishes; djinn will sometimes use their wish on themselves if hostile
- Fix: DYWYPI and items with object properties
- Fix: magic missile causing half damage to target if the player had half spell damage
- Fix: could hear zombie sounds when deaf
- Tweak nutrition gained by Illithid tentacle attack
- Fix: the sounds various monsters make when scared
- Fix: couldn't name regular flutes or harps
- Fix: 'You float gently to the floor' with intrinsic flying (Illithid) and taking off
  an amulet of flying
- Randomize how often Illithid player race uses tentacle attack
- Add Lifestealer to the SPFX_EXCLUDE list
- Adjust odds of random object spawning with some type of object property again
- Some changes to a few monsters, as well as nasties() and wizapp()
- Non-artifact sacrifice gifts revamp
- Fix: missing quest message for Convict role
- Fix: count shallow water and sewage as a safe landing spot in goodpos()
- Rogue thievery skill can be used on peaceful monsters without them knowing
- Tweaks to M3_BERSERK
- Fix: hunger levels and dexterity abuse for Hobbits
- Fix: player gets credit for the kill if monster dies from Grimtooth's disease effect
- Adjust chances of getting sick resistance when crowned
- Fix: rounding formula apply after racial shopkeeper price adjustments
- Fix: missing hit feedback from the Staff of Aesculapius
- Fix: make partial disintegration resistance actually mean something
- Fix: a searmsg call didn't check validity of hated_obj
- Another case of mindless monsters should not become scared
- Fix: Izchak isn't supposed to be immortal
- Refer to objects made of gemstone as 'crystal'
- Change Sunsword's base material from silver to gemstone
- Fix: monsters with a disease-based attack wasn't having an effect on other monsters
- Revamp feedback when attacking with/being hit by a weapon with offensive object
  properties
- Reset rogue's initial thievery skill back to basic level 
- Healers guaranteed to have enough power to cast a level one spell at game start
- Offering the Amulet of Yendor while wearing a helm of opposite alignment does not
  always fool your deity
- Fix: intelligent monsters that had no hands could open and loot containers
- Remove riding skill for Centaurs
- Certain monsters will move slower through sewage just like the player
- Partial fix: obfree deleting worn obj (spellcasting monster casting stone to flesh on
  itself while wearing a ring that could be affected)
- Fix: player being blamed for monster spell drying up a fountain
- Partial fix: active extrinsic from offhand weapon but not twoweaponing
- Fix: active extrinsic from offhand weapon but not twoweaponing
- Fix: monsters kept displacement extrinsic after removing a worn cloak of displacement
- Fix: getting reflection from Dragonbane if wielded
- Fix: placing defunct monster onto map, mstate:%lx, on Dlvl:%s


### Version 0.5.0

- Latest merges from 'vanilla' NetHack 3.6.6 official release (as of March 7th, 2020)
- Prep for version 0.5.0
- Gehennom changes (round one) - area around the wizard towers
- Gehennom changes (round two) - Cerberus guards the entrance to Gehennom
- Fix: shambling horrors stats persist across save/restore
- Fix: throwing Grimtooth up in the air and having it hit the player caused a segfault
- Bypassing Cerberus
- Fix: giant wizard starting with a cloak of magic resistance
- Fix: switch cloak of protection for gauntlets of protection (one of the sokoban prizes)
- Fix: jumping spiders getting stuck in webs
- Fix: kicking or throwing a spellbook of force bolt or a wand of striking at iron bars
  would destroy the iron bars
- Death (rider) livelogging tweak
- Another Izchak revamp
- Fix: sokoban door exploit
- Fix: monster wearing gear with extrinsic could negate same intrinsic once gear was
  removed
- Rogue tribute level disabled
- Re-add Frontier Town (mines)
- Partial fix: monsters switching back and forth between melee and ranged weapon in a
  loop at a certain range
- Fix: monsters hovering over an unlocked container they can't open that contains objects
  they want
- Fix: better handling of trapped tins in Sokoban
- Wishing for a quest artifact brings its true rightful owner
- Bows and crossbows are now two-handed weapons
- Amulet of life saving does not fully heal the wearer
- Using a wand of digging to escape being engulfed does not always drop the engulfer's
  hit points down to one
- Hey shopkeeper, stop polymorphing yourself!
- Increase odds to be pulled into water/lava
- Clobber attack from various monsters
- Fix: wielding the Longbow of Diana was not granting reflection
- Bows and crossbows are two-handed weapons even for a giant
- Some angels and priests on astral level have 'purple rain' insurance
- Fix: giant wizard starting with a ring of stealth
- Fix: wands were not being created when wished for by a monster
- New trap: crossbow bolt trap
- Gnomish rangers quest artifact is a crossbow
- Fix: various issues with crystal chests and iron safes
- Fix: wishing for elven helms of (object property)
- Get rid of CREATE_AMMO2
- Switch Magicbane from an athame to a quarterstaff
- Fix: material damage messages after monster is already dead
- Being in the presence of demon lords or princes negates self teleportation
- Gehennom changes (round three) - The Sanctum
- Being in the presence of demon lords or princes negates the effects of drinking
  a cursed potion of gain level
- Gehennom changes (round four) - Vlad's tower
- Fix: address some issues found with Vlad's tower revamp
- Fix: verbage when being killed by a salamander when it pulls you into lava
- Fix: prevent pets that aren't sick resistant from attacking gray fungi
- Juiblex tweak
- Fix: crash regarding booby-trapped doors
- Prevent the Amulet of Yendor or any of the invocation items from being destroyed
  if dropped in Vlad's cavern
- Sokoban prizes made of metal spawn made of a neutral material
- Redo of the Amulet/invocation items handling if dropped in Vlad's cavern
- Fix: feedback from various monster spells
- New monster: weredemon
- Zruty tweak
- Fix: rumors concerning Fire/Frost Brand
- Fix: ignore gold in n/52 items display
- Clean up player monster code/formatting in src/makemon.c
- Fix: a proper tileset can now be created
- Change .nethackrc to .evilhackrc
- Tweak damage output when digging your way out of an engulfer
- Fix: attempt to prevent monster steeds from disappearing when their rider dies
- Dilapidated armories
- Fix: safety valve against candelabrum's age becoming 0
- Fix: correct timing when attaching lit candles to the candelabrum
- Valkyrie quest artifact changes
- Minor adjustments to Valkyrie quest artifact changes
- More minor adjustments to Valkyrie quest artifact changes
- Fix: add a missing weapon that orcs should always know
- Occasional earthquake during the ascension run
- Changes to covetous monster behavior
- Projectiles fired from its proper launcher get a range increase
- Chance the Amulet of Yendor will teleport away if dropped
- Adjust shopkeepers starting gear again
- Major amnesia revamp
- Monks receive a random extra kick attack using martial arts at Master level or higher
- New monsters: mind flayer larva and alhoon
- Mind flayer larva won't infiltrate its own kind
- Re-order monsters in src/monst.c for mkclass
- Teleportation updates
- Fix: impossible rn2() when monster uses its spit attack
- Tweaks to wishing for certain objects
- Better feedback with corpses being destroyed and artifacts
- Healers can sense how injured a monster is
- Fix: 'You slip the leash around your Fiesty's gnome mummy'
- New branch: The Ice Queen's Realm
- Fix: seeing feedback from monster vs monster AD_CLOB attacks when they weren't
  visible
- Fix: monsters shouldn't be able to unlock crystal chests
- Fix: iron ball being deallocated when saving
- Rewrite monster steed / monster rider handling (SpliceHack)
- Tweaks to a few random vaults
- Fix: various compile warnings (clang)
- Fix: minor fix that allows mkstairs() to run prior to find_branch_room()
- Fix: small fixes regarding the Ice Queen's Realm branch
- Some tweaks to the Ice Queen's Realm branch 
- Adjust illithid race starting statistics
- Mummies have a constitution-draining attack
- Autounlock doors and containers
- Fix: add dead trees to win/share/tilemap.c
- Don't 'kill' the Ice Queen's dogs
- Fix: level sounds indicating portal to the Ice Queen's realm 
- Some tweaks to player monster container types, object property odds
- Fix: three separate issues concerning mind flayer larva
- Defeating the Ice Queen should not produce a 'You killed' message
- Proper attack feedback from Koa/Ozzy
- Some monsters should never randomly spawn in Gehennom; sea dragon tweak
- Fix: fire traps appearing where they shouldn't
- Fix: hobbit rogues shouldn't be lawful
- Fix: displaced monster attacking the player by surprise if they are immobile
- Fix: poisoned weapons other than projectiles were not flagged correctly
- Grimtooth is treated just as poisoned weapons are (player alignment)
- Remove monster projectile range changes
- Fix: crash when wielding a weapon as a priest and polymorphed into something else
- Fix: check for and prevent multiple light sources on the same object/monster
- Fix: proper checking deleted light sources
- Fix: prevent monsters using close-quarters range attack from getting free
  melee attack 
- Reverse 'prevent monsters using close-quarters range attack from getting free
  melee attack'
- Fix: a couple issues with monsters casting spells against other monsters
- Fix: don't show material on randomized types of armor if not known
- Fix: monsters being knocked off their steeds
- Update build files for other operating systems
- Bit of formatting clean-up in src/trap.c
- Edit of stock hints/linux file
- Fix: self-induced error in sys/share/dgn_lex.c
- Fix: missing comma in win/X11/winstat.c
- Fix: missing define in win/X11/winstat.c
- Potential fix: src\do_name.c(1328) : error C4703: potentially uninitialized local
  pointer variable 'mtmp' used (windows/visual studio build)
- Update travis.yml file
- Potential fix: src\mthrowu.c(955 and 1177) : error C4703: potentially uninitialized
  local pointer variable 'otmp' used (windows/visual studio build)
- Fix: uninitialized local variables/pointers in src/muse.c
  (windows/visual studio build)
- Add missing travis-gcc.sh script (from NetHack 3.7)
- Fix: missing separators in sys/winnt/Makefile.gcc
- Fix: some errors during compile (windows/visual studio and mingw builds)
- Fix: self-induced error with sys/winnt/Makefile.*
- Update travis.yml file
- Fix: monsters not walking across shallow water/sewage
- Tweak various files for windows/visual studio build (travis-ci)
- More tweaks to various files for windows/visual studio build (travis-ci)
- Rename windows build binaries and rc files to reflect variant name (tty)
- Remove duplicate code block in makemon.c
- Fix: pets not melee attacking peaceful targets
- Fix: Sunsword should only destroy zombie corpses, not everything under S_ZOMBIE
- Fix: rename .nethackrc.template to .evilhackrc.template (windows builds)
- Fix: stethoscopes not unlocking an iron safe via apply
- Tweak various settings in evilhackrc (windows builds)
- Even more tweaks to various files for windows/visual studio build (windows builds)
- Edit default folder creation (windows builds)
- Fix: can't chop down a dead tree on a no-dig level
- More tweaks (windows builds)
- Disable travis-ci compiling for windows-mingw
- Partial fix: certain giant player race flags returning FALSE when entering bones
  level of a non-giant race player
- Hacky fix for remaining race flags for giant/centaur being wiped by restmonchn()
- Refactor shambling horror logic into its own function
- Adjust odds of various events when dipping an object into a forge
- Prevent the Ice Queen from leaving her realm via magical means
- Fix: 'You die... The <foo> starts to ransack your possessions' while wearing
  an amulet of life saving
- Fix: any monster could pick up a boulder if the player was giant race
- Greatly reduce the chance of a kick missing if the player is skilled in
  martial arts
- Fix: various room/level generation improvements
- Adjust amount of AC dragonhide provides
- Fix: seeing 'It looks diseased' when defending monster is not in sight
- Adjust loot in special room for the Mines' End 'sewers' level
- Fix: Double hit messages (monsters wielding certain artifacts against other
  monsters)
- Fix: racial shopkeepers not always giving same-race discounts
- Fix: vulnerabilities and resistances while in monster form
- Fix: reading scroll of magic detection blanking out locations of known non-magical
  objects
- Some adjustments and improvements to the Ice Queen's realm
- Remove a bit of unnecessary code from the last commit 
- Cursed wand backfire patch
- Adjust odds of a wand of fire randomly appearing in a monster's inventory while
  in the Ice Queen's realm
- Trees and dead trees are affected by fire
- Fix: booby-trapped doors still exist after blowing up
- Fix: slightly better fix for booby-trapped doors still exist after blowing up
- Fix: event feedback player shouldn't get if blind
- Zombies will always be visible via warning
- Fix: several artifact weapons that were supposed to do double damage were not
- Artifact armor uses its artifact name when it blocks an attack instead of a
  simple name
- Edit 'you enter an ice cave full of monsters!'
- Fix: crash when summoning nasties
- Fix: fire damage (Gehennom) and cold damage (Ice Queen branch) only occurs if
  the player moves
- Fix: handling of monster riders if polymorphed and still riding their steed
- Levelporting and cursed potion of gain level effects in the Valley of the Dead
  resume once Cerberus is defeated
- Closing off one more Cerberus bypass
- Fix: Kathryn the Enchantress cannot be killed
- Specific chat feedback for Kathryn the Enchantress
- Fix: segfault when ball & chain rusts completely away under certain circumstances
- Fix: couple more issues regarding ball & chain
- Fix: livelog - revise spelling to more correct but less aesthetically
  pleasing "mimicking"
- Fix: resolve some issues with the Enchantress and both dogs still attacking once
  the ice queen branch is completed
- Update the README file
- Tweak damage bonus for using two-handed weapons, other code clean-up
- Add some more t-shirt slogans
- Fix: dying and then being life-saved via roasting/freezing to death
- Adjust one type of DSM for player monsters
- Fix: duplicate-ls-catcher crashing
- Fix: special artifacts should not be wished for
- Fix: Death becoming ultra-uber from death rays
- Another Juiblex tweak
- Fix: prevent the Red Horse from already being starved upon creation
- Remove Luck Blade's intelligent flag
- Fix: monsters switching back and forth between melee and ranged
- Items with detrimental object properties should sometimes spawn cursed and have a
  negative enchantment
- Fix: 'You try to feel what is lying here on the water' when feeling sewage
- Fix: applying a stethoscope to an iron safe while adjacent to it
- Fix: 'Chih Sung-tzu has forbidden you from using edged weapons such as wands of
  slow monster!'
- Fix: colored walls in special rooms not holding their color
- Fix: bug with Medusa's gaze being reflected back and the player being blind
- Adjust odds for Medusa to stone herself with her reflected gaze
- Adjust material for various artifacts
- Fix: prevent giants from cheating in Sokoban
- Tweak to bringing in your own boulders to Sokoban
- Redo what happens when giants bring boulders into Sokoban to drop/throw
- Fix: zombified monsters kept their mextra structs
- Fix: incorrect feedback when attacking via shift+f as rogue while twoweaponing
- Fix: certain metallic shields did not incur a substantial casting penalty
- Fix: being able to attack peacefuls with a weapon as a rogue using shift+f and
  not making the peaceful monster angry
- Earthquakes that can occur after the Wizard has been killed won't happen unless
  the player has the Amulet of Yendor
- Change earthquake event from having the Amulet to having performed the invocation
- Fix: playing as giant race, polymorphing into another form and can still throw
  boulders
- Fix: make prayer timeouts longer in very long games
- Monster steeds can use special attacks, and monsters will dynamically mount nearby
  steeds
- Allow player monsters to also randomly mount and ride a steed
- Fix: scattered objects landing on water or lava without being affected
- Fix: prevent monsters from mounting steeds they shouldn't
- Steel
- Fix: weird monster/steed combinations
- More adjustments to possible monster/steed combinations
- A few more adjustments to possible monster/steed combinations
- Adjust the price of steel (METAL)
- Fix: tame chameleons able to shapeshift after being 'killed' after a save/reload
- Adjust how often a monster could randomly mount a suitable steed
- Fix: player could hit incorporeal monsters with their bare hands
- Descriptive hit messages for player attacks
- Fix: allow a bit of insanity for monster steed location
- Fix: 'The <monster's> corpse falls away and disappears' for monsters that can't
  leave a corpse
- Adjust some of the monsters found in 'Zootown'
- Fix: monster spell 'vulnerability' not working vs other monsters
- Fix: quirk with cursed potions of gain level depositing the player on the
  staircase going down
- Fix: heap corruption regarding objects eroding away completely
- Record amount of gold in hero's possession in xlogfile
- Fix: address some 'read/write after free' issues 
- Fix: weird behavior with monster steed/rider pair and the rider is zombified
- Fix: minor inventory issues with player monster archeologists
- Fix: typo in src/dothrow.c and the Crossbow of Carl
- Fix: missing 'typ ==' in src/muse.c
- Fix: incorrect value in water_damage() in src/mon.c
- Fix: non-flying steeds falling to their death in Vlad's cavern 
- Fix: monster rider/steed tweaks and fixes
- Fix: peaceful spellcasters casting summon elemental/summon insects against player
- Fix: impossible() when prison guard tries to initiate a bribe while carrying
  no gold
- Fix: heavy iron ball and chain are always rustproof
- Proper xlogfile death reason when decapitated via AD_BHED damage
- Fix: 'read after free' with pets gaining intrinsics from corpses
- Fix: some tweaks to 'read/write after free' issues
- Minor tweak to livelog message for player losing their long sword via dipping in a
  fountain
- Tweak monster AI logic concerning wand use
- Fix: "placing %s over %s at <%d,%d>, mstates:%lx %lx on %s?" panic in Fort Ludios
- Proper xlogfile death reason when disintegrated by a black dragon's passive attack
- Fix: metallic armor should always give at least one point of AC if unenchanted
- Fix: "warning: address of array 'killer.name' will always evaluate to 'true'"
- Fix: monsters killed while standing on shallow water or sewage weren't leaving
  a corpse
- Wizmode wishing for 'puddle' creates shallow water terrain
- Fix: "warning C4244: '=': conversion from 'int' to 'float', possible loss of data"
- Tweak some xlogfile death messages
- Fix: movement messages while standing still in shallow water or sewage
- Fix: segfault when dropping objects with timers in Vlad's cavern


### Version 0.6.0

- Latest merges from 'vanilla' NetHack 3.6.6 official release (as of March 7th, 2020)
- Fix: address skill issues of various roles
- Fix: seeing feedback about an object disintegrating off of a black dragon's scales when
  the event wasn't witnessed
- Fix: death rays were killing monsters that should be immune to death magic
- Update include/patchlevel.h for version 0.6.0
- Some racial shopkeeper tweaks
- Fix: correct misspelling with Werebane extra damage feedback
- Fix: The gnome lord's silver pick-axe shatters from the force of your blow!
- Fix: beheading a troll that was wearing an amulet of life saving shouldn't cancel it
- Fix: monsters leaving corpses after being disintegrated by black dragon's passive attack
- Fix: aged corpses placed inside an ice box show as 'burnt'
- All spell staves are now properly magical
- Various objects.c edits
- New monster: lava gremlin
- Some tweaks to the lava gremlin pull request
- Change katana's base material from iron to steel
- Ensure the artifact Snickersnee is always made of steel
- Add a couple monsters to the 'can never be tamed' list
- Additions to which monsters resist_sick() and resist_drain()
- Add M1_TPORT_CNTRL flag to the ice queen
- Monsters that are M3_BERSERK should only 'howl with rage' when attacking and at
  low health, not *just* low health
- Some forge adjustments
- Cursed weapons get to-hit/damage bonuses to angelic beings
- Fix: passive AD_DISE attack to properly use diseasemu()
- Defeating Kathryn the Ice Queen now counts as an achievement
- Encode minor/major oracle consultations into the xlogfile
- Disclose achievements
- Record number of wishes and artifact wishes in xlogfile
- Make hostile monsters with launcher and ammo keep away
- Dark/unlit corridors are actually dark
- Snow golems can throw snowballs
- Show skill caps and percentage towards next level in #enhance
- New player role: Infidel (by Tomsod)
- Various fixes and tweaks to the Infidel role
- Fix: illithids playing as Infidel weren't starting with their 'psionic wave' spell
- Realizing the Infidel role author's intent
- Fix: a few more incorrect alignment type reporting, livelogging (Infidel role)
- Fix: killed uniques livelogged as bogusmons if player was hallucinating
- Fix: three bugs with ghosts displaying as I incorrectly
- Effects of praying/sacrificing as an Infidel
- Crowned Infidels (demonic form) can never handle silver objects
- Infidels can train trident weapon skill
- A couple more minor Infidel tweaks
- Show if inediate via enlightenment
- Fix: Demonbane not suppressing demon gating when in offhand while twoweaponing
- Fix: proper way of saving shambling horror data across save/restore
- Fix: default engraving/epitaph/bogusmon corruption
- Add amulet of guarding from NetHack 3.7.0
- Add new monsters from NetHack 3.7.0
- Fix: weapon skill in ^X status output was wrong
- More tweaks, fixes, and changes with Infidel role
- Fix: a couple 'oops' from the last commit
- Mummies have a withering attack
- Let monsters use wand of undead turning
- Wearing mud boots negates being slowed when walking through sewage
- Allow giant race to play as Samurai, tweak to the Tsurugi of Muramasa
- Even more Infidel role changes
- Reverting spell power drain for Infidels; minor altar sacrifice tweaks
- Infidel ascension feedback correction, power regen tweak
- New artifacts: Angelslayer, Bag of the Hesperides, Butcher, Wand of Orcus
- Fix: various issues from the previous new artifact commit
- Fix: issues with centaur race and kicking
- New monster: eldritch ki-rin
- Herbivore pets consider more types of food as treats
- New level (nymph) and new special room (garden)
- Change the Sceptre of Might's base item to heavy mace
- Using a cursed key/lock pick/credit card has a chance to break if used
- Extended achievement and conduct fields for xlogfile
- Added new event to extended achievement and conduct fields
- Armor made of a hated material deflecting a monsters attack 
- Tiamat's stinger is supposed to be poisonous
- Tiamat's engulf attack should be last in the attack chain
- Descriptive miss messages (player vs monster, monster vs monster)
- Fix: minor bug reporting amulet or idol in Infidel's possession upon death
- Fix: suppress 'with the amulet' death messages for Infidels
- Specific sacrifice offering being consumed message for unaligned altars
- Player monster Infidel's weapon and armor should spawn cursed
- Fix: monsters riding steeds and polymorph traps
- Fix: several instances of incorrectly receiving visual feedback
- Fix: prevent ice nymphs from spawning where they shouldn't
- Fix: crowned infidel's stinger attack against monsters whose touch petrifies
- Fix: incorrect crowned infidel title via enlightenment
- Fix: crowned infidel's (demon form) and various silver object interactions
- Show if immune to death magic via enlightenment
- Fix: crowned infidel's wings stay confined under their body armor on save/restore
- Fix: trees spawning on stairs on the nymph level
- Fix: add missing roles to appropriate casting type
- Fix: infidels and spell casting bonuses/penalties
- Fix: stop demon lords/princes from being so damn chatty to infidels they like
- Fix deaf & blind hero 'seeing' monster (artifact owner appearance)
- Remove bit from a previous fix that does absolutely nothing
- Don't push around stacks of boulders as one
- Fix: tweak to chatty demon lords that like you
- Make boulders weigh 8 aum EACH, not together, for giant PCs
- Bit of formatting housekeeping
- Fix: the Bag of the Hesperides was not granting MC1 when carried
- Fix: giant samurai breaking out of their large armor when poly'd into a large form
- Make the Ice Queen not aggro on multihits
- Some slight code improvements (ice queen branch)
- Fix: your deity probably shouldn't gift the player a helm of opposite alignment
- Fix: only demons can wield the Wand of Orcus
- Fix: only transform a dead ice queen
- Clear up a couple extra conditions on transformation
- The Wand of Orcus is always known
- Fix: make scrolls of fire wishable again
- Fix: make player monsters show the right rank titles
- Code formatting tweaks
- Fix: tame vampires no longer revert back to animal/fog form
- Fix: proper handling of monster's weapon/gloves when attacking a black dragon
- Fix: correct feedback when a monster dies due to wearing a cursed amulet of
  life saving
- Fix: player monster infidel armor/weapon objects not truly cursed
- Player monster infidels won't use scrolls of remove curse
- Some tweaks to random vaults
- Fix: huge/gigantic monsters 'squashing' tiny monsters in their way
- Running and traveling no longer push boulders
- Allow skilled attack spell casters to cast 'basic' level for various attack spells
- Fix: guardians could attack hostile quest leader
- Conflict negates Elbereth and scare monster
- Lifesaving livelog shows the would-be killer
- Allow teleportation into unteleportable spots in wizard mode
- Unify monnam calls for livelogs that should ignore hallucination
- Fix: livelog for killing pet should happen even if pet is unnamed
- Elven/orcish undead are unaffected by the materials their living counterparts hate
- Fix: prevent potential segfault using a stethoscope to listen to eggs
- Angel tweaks and adjustments
- All demon lords/princes have at least a base speed of 12
- Player monsters that spawn in the endgame use names from the topten list
- Fix: Gjallar not affecting various monsters predisposed to wait
- Fix: proper handling of wielding a chickatrice/cockatrice corpse if worn gloves
  erode away
- Fix: 'Boyabai the dwarf lady swings his staff at the orc zombie'
- Infidel player monsters can cast spells
- Balrog adjustments
- Increase starting pets survivability versus zombies
- Object materials: fixes for material hatred (round one)
- Object materials: fixes for material hatred (round two)
- No, really. Wizards shouldn't receive plate mail as an altar sacrifice gift
- Prevent monsters zero-turn equipping and equipping with player nearby
- Fix: monsters using ranged weapon at melee range
- Fix: possible 'glorkum' event
- Fix: pets attacking peaceful quest guardians/leader with ranged attacks
- Fix: the weredemon turns into a demon
- Fix: monster wearing a cursed amulet of life saving wouldn't die from it if hero
  was blind
- Preparation for official version 0.6.0 release
- Fix: sear messages for items made of non-hated material
- Fix: material bonuses in dmgval applied to more than weapons
- Fix: mind flayer types being zombified
- Fix: you see here a cloth saddle
- Tweak extra_pref()
- Fix: prevent tools that spawn inside a dilapidated armory from having zero charges
- Fix: heap use after free (sp_lev.c)
- Fix: killed by the the <foo>
- Fix: Sokoban zoo entrance door could become sealed shut while retrieving the prize
- Fix: pet location inconsistency
- Fix: containers made of non-flammable materials catching fire and burning up
- More tweaks to tool class object materials
- Fix: prevent shape-changing pets from reverting back to their original form while
  wearing a ring of protection from shape changers
- Prevent the Mitre of Holiness from blocking an illithid's psionic attack
- Fix: strength attribute abuse (giants)
- Fix: champions/agents spawning when the player is not playing as an infidel
- Fix: The elven king's ring of protection blocks the warg's attack
- Fix: debug pline left in death-aversion livelog code
- Fix: You crush the the ogre's skull!
- Fix: water-based monsters being hurt or killed by blasts of water
- Fix: Butcher weight adjustment
- Fix: slight revamp of wielding objects made of an adverse material
- Fix: Don't learn 'Elbereth' from headstones
- Fix: Butcher is supposed to be two-handed
- Fix: 'Your wooden shield smoulders' with non-wooden shields
- Remove multiple unnecessary instances of '#define a_align' 
- Fix: random earthquakes caused after performing the invocation were making peaceful
  monsters angry at the player
- Fix: Yeenoghu wasn't spawning with Butcher
- Improvements to how healers can sense injuries
- Fix: half spell damage stacking twice for player against magic missile attack
- Fix: possible to destroy the Amulet and the invocation items in sewage
- Fix: looking at monsters wearing armor
- Fix: giants could hit incorporeal monsters with anything
- Fix: check_wings() function could sometimes cause crash on game restore
- Fix: prevent non-wishable artifacts from appearing in bones pile
- Fix: missing ranks from enemy samurai during the samurai quest
- Colored altars revamp (part 1?)
- Fix: Angelslayer and the Tsurugi of Muramasa were conferring properties on carry
  when it should have been on wield
- Fix: prevent vampire mages from spawning in Mines End catacombs
- Fix: prevent master mind flayers or alhoons from spawning in the Gnomish Mines
- Chance for a hobbit rogue to spawn with a lock pick
- Fix: Genetic engineers dropping Schroedinger's cat box
- Fix: Getting 'Your flail slips from your hand' as a Convict wielding a heavy iron ball
- Prevent dog from getting stuck on 'treat' food
- Fix: Convict wielding a heavy iron ball, showing as a flail via enlightenment
- Fix: several cases in trap.c where the player could hear events while deaf
- Prevent certain extraplanar beings from being diseased
- Fix: altars changing from aligned to unaligned when a booby-trapped door exploded


### Version 0.7.0

- Latest merges from 'vanilla' NetHack 3.6.6 official release (as of March 7th, 2020)
- Replace 'samurai' in the samurai quest with new monster - Ronin
- New object property - Excellence
- Fix: correctly handle how <foo> of excellence becomes known when worn/wielded
- More tweaks to 'excellence' object property
- Adjust odds of artifact weapon and weapon with object property generation
- Fix: proper handing of luck adjustment when removing <armor> of excellence
- Minor formatting tweak in makewish()
- Fix: timed clairvoyance, add 'quick_farsight' option
- Fix: Oracle not defending itself against the player when being attacked
- Several changes to the Oracle
- Ronin (samurai quest) spawn with basic kit
- Wizard of Yendor buff
- Keep Charon stationary unless he becomes hostile
- Improvements to monster wishing
- Fix: material restirctions for sokoban prize objects
- Fix: crash on reloading game while wielding Grayswandir
- Bit of code cleanup in mcalcdistress()
- Fix: Sceptre of Might as 'heavy mace' in Caveman.des
- Allow player as a giant to force-move onto a boulder without having to push it
- Fix: Monks were getting their extra kick attack while riding on a steed
- New monster spell: reflection
- Bring secondary effects of worn black dragon scales/scale mail inline with black
  dragons
- Fix: most armor special properties were still in effect if it eroded away
- New monster spell: protection
- Check bones data directly for deja vu messages
- Copyright info update for 2021
- Wand wresting change
- Ki-rin changes
- Extra Monk abilities at higher experience levels
- Fix: crash when killing monster with a wielded potion
- Fix: better handling making ball/chain rustproof
- Remove mkpuddles() function
- Prevent rust monsters from 'unfixing' fixed heavy iron ball/chain
- Some object tweaks
- Fix: monsters wearing gray dragon scales/scale mail passive attack happening
  too often
- Mind flayer larva nurseries to show up a bit sooner in the dungeon
- Fix: some monsters that are resistant to fire were also flagged as vulnerable
  to fire
- Correction to chances for passive attack from BDSM (monster vs player)
- Exploding Bag of the Hesperides
- Fix: non-artifact weapons behaving as the Secespita artifact weapon if power
  was not 100%
- Fix: ensure that only zombie corpses confer zombie sickness when eaten
- Fix: "It attacks the displaced image of it."
- Fix: expelled hero kept suffocating when levelporting or branchporting
  out of the engulfer
- Prevent monsters from using weapons made of a material they hate
- Fix: The iron of your helm blocks your psionic attack.  The watchman gets angry!
- Fix: various artifacts 'incinerating' a monster but they still left a corpse 
- Fix: 'glorkum' from appearing in player monsters inventory
- Fix: black dragon/BDSM passive attacks disintegrating wrong armor pieces
- Fix: naming an object will apply to the same type of object if it's one of the
  undiscovered sokoban prizes, giving away what it is before it's been obtained
- Fix: certain artifact attacks wouldn't kick in unless target monster was seen
- Fix: crash when monster threw glass weapon into iron bars
- Changes to 'magical' item xname for Wizards
- Fix: don't leak identity of soko prize tool
- Add 'readable' Hawaiian shirt designs
- Revamp dragon engulfing/post-death ransacking feedback
- Resurrection/Kathryn messages
- Kathryn the Enchantress will still wave to you if hostile
- Fix: the Oracle and Charon changed states not restored on save
- Fix: better fix for crash when killing monster with a wielded potion
- Fix: thrown/kicked objects hitting a black dragon would disintegrate 100%
  of the time
- Fix: check_wings() doesn't respect if an amulet of flying is being worn or not
- Fix: Illithids can still use psionic blast if poly'd into a non-illithid form
- Fix: actual proper fix for booby-trapped doors still existing after blowing up
- Fix: Stormbringer wasn't doing its 'bloodthirsty' attack against tame/peaceful
  targets while being dual-wielded in the offhand position 
- Fix: prevent 'impossible d(-1,250) attempted'
- Fix: player taking damage from missed fire/ice bolt monster spell
- Encyclopedia update: round one (monsters)
- Encyclopedia update: round two (objects and miscellaneous)
- Fix: The black dragon eats a black dragon corpse. The black dragon looks very
  firm.
- Adjustments to throne wishes
- Shallow water is a finite resource
- Fix: rabid dog encyclopedia entry
- Fix: containers made of iron would rust away and not drop their contents
- Redo encyclopedia entry for turtle
- Fix: another case of 'placing steed onto map, mstate:0, on Dlvl:() ?'
- Pseudodragons given a range of different sounds
- Fix: prevent removal of special artifacts on bones level and the
  owner is present
- Fix: Oracle missing its artifact on the new oracle levels
- Fix: Oracle stayed peaceful when the gods took notice of thievery
- Fix: Cerberus was still susceptible to AD_BHED attack
- Fix: instances where article was missing from the Bag of the Hesperides
- Create three tiers of homunculus, Infidel now has a pet that can grow
- Slight changes to intrinsics gained via crowning for infidel role
- Extra effects/consequences for wielding or wearing any of the 'banes 
- Fix: ensure cursed flag is set for infidel NPC water potions
- Some tweaks to infidel NPC gear
- Fix: snow golem entry for the encyclopedia
- Honey badger adjustments
- Racial priests and priestesses
- Some minor fixes to the racial priests and priestesses commit
- Bit of code formatting clean-up (racial shopkeepers)
- More tweaks to racial priests/priestesses
- True dwarf/gnome placeholder templates, bit of a racial shopkeeper
  revamp
- Fix: crash when setting nudist option in config
- Artifact wielding function
- Enable sunsword's shining effect when wielded in the offhand
- New weapon: dwarvish bearded axe
- Fix: A shimmering globe appears around it!
- Fix: typo in objects.txt for dwarvish bearded axe
- Kathryn's birthday
- Fix: adjust ACCESSIBLE to accept shallow water and sewage tiles
- Fix: wrong race with racial identifier for racial priests
- Racial player monsters
- Fix: make sure iflags.use_color is enabled when changing player monster
  color
- Fix: prevent rangers from wishing for their other quest artifact
- Fix: remove chokepoint final ranger quest level (centaurs)
- Fix: random monster cannot spawn if unaligned (A_NONE)
- Fix: game freeze with rivers spawning in certain versions of minetown
- Add elementals to the list of monsters that are sick resistant
- Some minor adjustments to certain monster grudges
- Small adjustment to elves and orcs starting stats, searching for elves
- Reintroduce nymph shopkeepers
- Make sure illithids fall under telepathic()
- Racial shopkeeper dialogue based on racial love/hatred
- Small tweak to feedback - ensure magic traps ignore (see) invisible
  timeouts
- Fix: 'The skeleton's skin looks flaky'
- Fix: another missing article with an artifact
- Fix: 'Your pair of leather gloves looks completely burnt' printing twice
- Fix: racial player monster fixup
- Fix: clean-up a few warnings from the last PR
- Fix: spelling error in README
- Remove M3_SKITTISH from ranger monster template
- State gender for monster using a portable container
- Rename 'hobbit rogue' to 'hobbit pickpocket'
- Racial soldiers and other mercenary types
- Tweaks to player monster alignments
- Add and use 'rndrace' function for picking races
- A couple more minor tweaks to player monster alignment
- Fix: silence a few warnings (clang)
- Fix: knight player monster titles match their alignment
- Adjust centaurian race player monster gear
- Player can't regen hit points while in the Valley of the Dead
- Wand of death acts like full healing if used against undead
- Give players a hint about no hit point regen when entering the VotD
- Fix: all races could carry boulders like a giant
- Cerberus can be put to sleep with a magic flute
- Hit point adjustments placed in correct spot
- Have to be close to hear a monster 'jump into a saddle'
- Use player racial adjectives for monsters
- Tweak racial adjectives (dwarves, centaurs)
- Racial priests to use player racial adjectives
- Create proper racial names for player monsters
- Fix: missing 'end of string' in do_name.c
- Fix: monsters turned into zombies still showed as 'turning into a
  zombie' via probing or stethoscope
- Handle M2_NEUTER monsters when using a stethoscope
- Cleaner way of processing racial player monster names
- Silence warning from last commit
- Fix: tweaks to player monster flags and allowed races
- Properly set status of 0.7.0
- Prevent centaur monks from using random kick attack against certain
  targets
- Proper handling of monsters killed via artifact special fatal attacks
- Fix: giant samurai wearing large splint mail and polyform
- Fix: add/remove a few M2_NOPOLY flags (monst.c)
- More better grammar
- Addition of README.md
- Dynamically name noble/royal monsters
- Fix: force monster rider from steed if under influence of conflict
- Fix: 'three rooms' random vault sometimes not spawning any doors
- Prevent cannibalism from illithid tentacle attack against
  werecreatures if infected with lycanthropy
- Monsters can revive corpses on floor with undead turning
- Spheres cause real explosions / new spells: flame sphere and freeze
  sphere
- Some tweaks to sphere spells
- Proper timeout for sphere spells
- Magic 8-ball tweaks
- Fix: feedback with spear traps and flying steeds
- Fix: You start playing your Gjallar
- The spotted jelly flollops!
- Small chance a player monster's container is a bag of tricks
- Suppress 'You appear to be only partially affected.  You aren't affected.'
- Fix: DOORMAX exceeded issue / acid explode type
- Fix: 'Swallower has no engulfing attack?' impossible
- Fix: drawbridge stuck if water underneath were frozen by a monster via
  moving over it
- Tweaks to last commit (drawbridge)
- Even more tweaks to last commit (drawbridge)
- Fix: tautological statement in lock.c
- Yet another Gehennom revamp
- Fix: monsters.txt incorrect order
- Fix: another monsters.txt error
- Fix: rearrange the logic of starting inventory adjustments
- Demon boss/lair adjustments, new object
- Minor tweaks to last commit (demon boss templates)
- Fix: Dragonbane could still be disintegrated by attacking a black dragon
- Encyclopedia entries for newer monsters/objects
- Tiamat has a chance to drop chromatic dragon scales upon death
- Addition of a third tier 3 demon prince level
- Tweaks to hellc-2 map
- A couple more tweaks to hellc-2 map
- Lolth has two arms
- Set mspec for several spellcasting boss-type monsters to 0
- Fix: 'Couldn't place lregion type %d!' for various gehennom levels
- Update level data on various platforms (Gehennom revamp)
- Small chance dragon scale mail will revert back to a set of scales
  if cancelled
- Fix: entry message repeating every time the player enters the Ice
  Queen's realm
- New branch: Vecna's domain (part one)
- More verbose message when entering Vlad's cavern for the first time
- New branch: Vecna's domain (part two) - artifacts, Kas (unique monster)
- Fix: Kas not flagged as a vampshifter, pets would eat the Eye of Vecna
- Cover all the bases for lich/alhoon genocide prevention while Vecna
  exists, switch Vecna's gaze attack to a death gaze
- Refactor tracking Cerberus alive/dead state for opening up Gehennom
- Invoking the Eye of Vecna uses actual death magic
- Fix: invoking the Eye of Vecna and it killing a monster
- Fix: 'It flies into a berserker rage!'
- Kit out Vecna and Kas, other tweaks and adjustments
- Fix: 'The seemingly dead warg suddenly transforms and rises as a
  vampire royal!'
- New branch: Vecna's domain (part three) - levels, minor tweaks
- Fix: mstone timer still active on monster that was stoned and then
  unstoned
- Fix: part two of mstone timer still active on monster that was stoned
  and then unstoned
- Fix: tame monsters stashing gear into locked containers without
  unlocking it first
- Fix: vaults.dat being copied twice during build, tweak .yml file
- Create hints file specifically for github actions
- Fix: 'The goblin outrider is burned by your invisible vampire mage's
  pair of mithril tekko!'
- Update level data on various platforms (Vecna branch).
- Fix: segfault if killed by certain monsters that go through your possessions
- Fix: better grammar/object handling (priest attacking with forbidden
  weapons)
- Redo vecna-3 map
- Fix: Caveperson quest nemesis not spawning with the Bell of Opening
- Some rumor tweaks
- Fix: drain_weapon_skill() irregularities, tone down occurence of the same
- The Sword of Kas adjustment
- Small tweak to is_lord() gear enchantments, monsters receive racial bonus
  for wearing aligned armor 
- Fix: impossible/panic when monster dies to self-read scroll of earth
- Don't allow the debug fuzzer to enter explore mode
- Fix: segfault caused by thrown objects calling searmsg()
- Fix: account for hiding monsters in mdisplacem
- Fix: buffer underflow with gendered monster names
- Better handling of dead trees and digging
- Attempt windows build via github actions
- Fix: error C4703: potentially uninitialized local pointer variable
  'mdat' used
- Fix: another 'error C4703' case
- Potential fix for missing file/folder location
- Second try, potential fix for missing file/folder location
- Correct version number in sys/winnt Makefiles
- Update version number in windows build files
- Fix: water_damage_chain could wet some objects twice
- Encyclopedia entries for Vecna, Kas, and associated artifacts
- Abusing your alignment can cause your quest leader to ask you to return
  the quest artifact
- Fix: mon 457 doesn't like any materials for obj 269
- Fix: incorrect MC calculation
- Exclude fumble boots/gauntlets and dunce cap from regular item sacrifice
  gift selection
- Randomize odds of a river forming in the gnomish mines
- Tweak Wizard of Yendor's kit
- Give the player a chance to escape from a demon lord/prince using
  teleportation or cursed potions of gain level
- Wizard quest leader/nemesis tweaks, MR2_TELEPATHY extrinsic for monsters
  from worn objects
- The Dark One's familiar
- Fix: The Grey-elf resists the death magic, but appears drained!
  The death ray hits the Grey-elf! 
- Improvement to random river generation in the gnomish mines
- Adjust wand creation probabilities
- Some minor curses mode fixes
- Fix: wish prompt
- New command (^U) to remove remembered 'I' monster markers from the map
- Rename EvilHack changelog extension to .md and update formatting
- Add AUTOCOMPLETE to new 'remove remembered 'I' monster markers' command
- Mind flayer larvae grudge humanoids it considers a suitable host, can
  become a mind flayer on successful brain burrow attack
- Take greased/slippery head gear into consideration for mind flayer
  larva brain burrowing attack
- Alignment abuse value guard
- Fix: various objects incapable of being ID'ed under certain conditions
- Fix: errors in icequeenrevive() function, passive disintegration
- Mind flayer larva nurseries sometimes spawn with live host prisoners
- More wizard quest nemesis tweaks
- Fix: dmonsfree panic when certain monsters are killed via passive
  disintegration
- Fix: add missing disintegration/cancellation gaze attacks for monster
  vs monster, overall corrections to gaze attacks
- Suppress certain feedback upon player death, prevent crash
- Fix: The Archangel points all around, then curses.  ≡┌╫ the saddled
  black dragon / new monster vs monster attack function (buzzmm).
- Fix: death magic resistance feedback
- Some tweaks/improvements to castmm() and buzzmm(), elven wizard
- Identify a wand when it prints an unambiguous message while engraving
- Remove #give command
- Implement giving and taking pets' items via #loot
- Fix: findgold always picked first gold item in the object chain
- Give pit fiends a pit-creation attack
- Hezrous stink
- Barbed devils use hellfire
- Add some new demonic and angelic maledictions
- Remove feedback to use #monster when poly'd into a monster that
  has AT_MAGC
- Fixed and inherently fixed rings/wands can resist destruction due to
  shock damage
- Fix: player changed into something with big wings and wearing body
  armor could still fly
- Allow wishing for fractured (glass) or deteriorated (super material)
  objects
- Correct oversight with various demons not being able to fly
- Add some new race/role combinations
- Allow dwarves and orcs to play as Knights
- Adjust odds of sentient_arise() when killed by various monsters
- Barding
- Fix: could place barding onto humanoid-shaped angel class
- Koa tribute
- Fix: death magic resistance feedback (again)
- Tweaks to the final Ice Queen branch level
- Fix: weird autorun behavior through fog clouds
- Lower level needed to be accepted for the quest
- Give orcish knights an orcish long sword
- Mephistopheles gets an extra fire attack
- Switch Angelslayer alignment to unaligned
- Fix: 'This Grey-elf corpse tastes okay.  You wake up.
  You feel a bit perkier.'
- Caveperson changes
- Few more caveperson tweaks
- Allow player monsters to spawn using newest race/role additions
- Fix: broke (a)pply and #rub for all objects except gem class types
- New Sokoban levels
- Fix: show items given to monster with inventory letters
- Gnolls
- Uruk-hai/orc-captains will sometimes spawn riding a warg
- Acid spheres
- Add macos build to github actions
- Discontinue using Travis CI
- Giant spiders, Lolth can entangle their target with a web attack
- Fix: 'Magical energies are absorbed from <target>' printed twice
- Restore guaranteed first sacrifice gifts
- Change CFLAGS -g to -g3 (linux builds)
- Tweak encyclopedia entry for Yeenoghu
- Covetous monsters will equip wearable items that they target
- Allow Infidel quest leader to demand the quest artifact
- Neothelids
- Fix: prevent Infidel from escaping the dungeon with the Amulet of
  Yendor via drinking a cursed potion of gain level
- Fix: bug allowing potential bypass of most of the elemental planes
- Infidel role: sanctum high priest will give both hints if the
  quest leader is dead
- Fix: create_pit_under() feedback being shown while not witnessing
  the target being affected
- Fix: seeing dobuzz() feedback while not viewing intended target
- Allow erac monsters to use racial glyphs
- The Dark One's pointy hat isn't guaranteed
- Fix: reflecting Medusa's gaze with a mirror
- Fix: address various monsters being affected by death magic that
  shouldn't
- Luck plays a role in whether player can continue if they rise from
  the grave
- Alternate ending to the Ice Queen branch if the player is an Infidel
- Fix: broke the wish parser for objects like hats/boots/gloves
- Change enchantress 'freeze' from 75 to 100 turns (infidel role)
- Overhaul damage type 'withering'
- Use erac for racial priests and shopkeepers
- Fix: racial priests race not matching alignment
- More robust version of could_twoweap()
- Add SERVERSEED option to obfuscate hash functions on servers
- Actually utilize SERVERSEED
- Monster lookup
- Object lookup
- Add 'lookup_data' option
- Fix: prevent racial angels
- Add G_NOGEN to monster lookup
- Include orcish morning star in small/large monster damage
- Fix: null pointer crash (water_damage_chain)
- Add a few minor enhancements from vanilla NetHack/xNetHack
- Some encyclopedia entries and tweaks
- Revert ball & chain being rustproof, prevent either from rusting
  away completely
- Fix: crash if acid sphere explodes at nothing
- Prevent vault guards from having random race
- More ball & chain fixes
- Fix: ball & chain handling and passive disintegration
- Fix: #wizgenesis 'giant' caused rn2(0) 'program in disorder'
- Fix: duplicate lightsource attempting to be created, type 0
- Fix: crash due to monster wishing for an artifact and not receiving it
- Balrogs don't have forehooves
- Tweak/fix to last commit
- Fix: tame monsters should not be allowed to take a heavy iron ball
  you if you're still chained to it
- Feedback improvements to #loot (tame pet)
- More feedback improvements to #loot (tame pet)
- Little bit extra variation of water creatures on the Plane of Water
- Tweak odds of rising as a zombie and continuing to play
- Put a distance limit on how far away you can hear a monster
  rummaging through a container
- Remove monster maximum difficulty limit with demigod flag set
- Experience level gain formula adjustment
- More tweaks to pet #looting
- Change status from 'work-in-progress' to 'beta'
- Some 2nd level sokoban levels had pits instead of holes
- Don't allow hobbits to be priests (monsters)
- Fix: racial shopkeeper love/hate system not using erac
- Greater to-hit bonus at experience level 30
- Show player how badly they've abused their alignment via #conduct
- Fix: pet unable to "return" item to shop
- Fix: missing period (Infidel and casting)
- Fix: minimum level was absent regarding altar sacrificing and
  receiving a gift
- Fix: giving pet wielded/quivered item via #loot
- Handle welded equipment in #loot more consistently
- Adjust how often Grimtooth can disease its target
- Fix: allow dwarvish bearded axe to chop down trees/doors
- Fix: pets not attacking monsters under most circumstances
- Fix: peacefuls would never take any items given via #loot
- Fix: ghosts attempting to scare already paralyzed hero
- Fix: alignment hit infidels take when not sacrificing in a timely
  manner
- Fix: super tame steeds attacking monsters they shouldn't while 
  being ridden
- Fix: prevent select_newcham_form() from choosing player monsters
  as a valid polyform (except for doppelgangers)
- Change dlord() to ndemon() for sacrificing same race on chaotic
  or unaligned altar
- Fix: The lichen nimbly dodges the gnome zombie's bite!
- Fix: player monster monks spawning without gear
- Fix: using monster lookup on the Rat King returned bad information
- Fix: crowned infidel transforming into a demon wasn't fully
  a demon
- Fix: erac now adjusts for monster size/weight
- Monster lookup shows incremental percentages for conveyed
  resistances
- Fix: prevent monsters jumping through iron bars
- Fix: eyeballs (food object) were appearing randomly
- Fix: mstone timer would reset everytime a monster attacked another
  monster with a stoning attack
- Fix: engraving didn't auto-id wand of polymorph
- Fix: various bugs with Race_if() conditions
- Fix: take erac into account when performing same-race sacrifice
- Prevent intelligent pets from stashing rocks in their bag
- Fix: 'Your two weapon skill is also limited by being basic
  with with broadsword.'
- Fix: better way to prevent cannibalism as illithid while infected
  with lycanthropy
- Fix: monster spell 'touch of death' with monster vs monster
- Fix: crowned infidels (PM_DEMON) gating in hell-p
- Adjustments to mspec_used for boss-level monsters, Wizard of Yendor
  gear tweaks
- Change 'summon nasties' list for Vecna
- Adjust how often Vecna warps to the upstairs
- Fix: segfault with summon nasties
- Fix: player getting credit for a monster passively disintegrating
  another monster
- Fix: monster casting 'summon minion' was doing damage to the target
- Fix: monster spellcasting level cap was changing the monster's
  actual level
- Fix: 'program in disorder' when destroying Vecna and he drops the
  Eye of Vecna
- Fix: Izchak was not transforming after his human form was killed
- Fix: wishing for any type of sword of excellence, monster creation
  in wizmode
- Feedback for varying levels of incremental resistances
- Fix: chaotics/unaligned getting alignment penalty for healing or
  curing their pets via spell
- Fix: pudding conveyances in monster lookup
- Fix: trees were unaffected by fire from explosions
- Better encyclopedia entry for Tiamat
- Set a default pet name for Infidel role
- Fix: Illithid player monsters were not being sensed via telepathy
- Fix: player monster giants and centaurs spawning with armor they
  can't use
- More adjustments to what intelligent monsters stash in containers
- Fix: crash if artifact owner shows up while level is full
- Fix: object materials: sort items correctly according to their
  material
- Readjust alignment penalties for Knights (again)
- Fix: monsters eating eucalyptus leaves when not ill
- Fix: engulfers swallowing racial giants
- Fix: proper feedback incrementing poison resistance intrinsic
  while also having it extrinsically
- Fix: zombie form into a zombie that has been genocided
- Fix: charge for #looting pet in a shop
- Fix: Vorpal Blade wasn't vorpalling
- Fix: Healer role was still starting with poison resistance
- Fix: crowned Infidel giants could not wear body armor/cloaks
- Allow dead trees in Vecna's domain to burn
- Fix: corpses removed from ice box by a monster wouldn't rot
- Fix: incorrect alignment for attendants (healer quest) and
  warriors (valkyrie quest)
- Fix: crash due to incorrect handling of rot timer
- Fix: confusion from psionic wave attack required seeing the
  target
- Fix: various armor made from a rigid material was allowing
  dexterity bonus to be applied
- Fix: Sokoban zoo entrance door not affected by various rays
- Change 'its' to 'his/her' under certain circumstances when
  a monster is killed via illness or disease
- Different feedback if a shopkeeper nymph tries to steal your
  mirror
- Fix: prevent shallow water in mindflayer nurseries from
  forming on stairs
- Fix: prevent shallow water that has formed over stairs from
  drying up
- Fix: wresting odds for spent wands was backwards
- Fix: greased armor allowed for corrosion loop
- Fix: how using the Eye of Vecna adjusts luck
- Fix: crash if a monsters maximum hit point value reached zero
  or less
- Fix: prevent player hit points from being greater than their
  maximum hit points (wand of orcus)
- Fix: clean up a few -Wmaybe-uninitialized warnings during compile
- Fix: incorrect feedback if Graz'zt steals something from the player
- Fix: 'The dust vortex bites!'
- Fix: Monk extra kick attack would work with wounded legs
- Update to 'Fix: charge for #looting pet in a shop'
- Adjust maximum hit points the Wand of Orcus can decrease per hit
- Fix: getting incorrect damage feedback from poisoned/diseased
  weapons if fully resistant to their special attack
- Make 'The golden haze around the <foo> becomes less dense' less
  spammy
- Fix: alignment penalty for killing quest leader that isn't yours
- Give Twoflower proper attire and gear
- Fix: set location pits forming under shallow water (Barbarian quest)
- Fix: 'It tasted bad' while quaffing a cursed potion of gain level
  in the Valley of the Dead after defeating Cerberus
- Fix: vampires in alternate form were grudging zombies and vice versa
- Fix: 'The shade squeaks in terror!'
- Fix: monster reading a scroll of remove curse
- Fix: monster opens trapped container, stun/halu effects affect player
- Code format cleanup in trap.c
- Fix: 'The barbed devil zaps you with a fireball!'
- Generate monsters on branch stairs after performing the invocation
- Improvements to monsters looting containers
- Warn player if their withering effect speeds up
- Fix: dungeon features falling into chasms (earthquake)
- Code formatting clean up (colored flashes dropping container on an
  altar).
- Merge known BUC status objects in container dropped onto an altar
- Fix: ggetobj bug when dropping just gold
- Fix: merging objects of known BUC status in container (altar)
- Remove saddle/barding specific #loot option
- Fix: weight of various elven weapons
- Hallucinatory armor/weapon descriptions
- Add a few more hallucinatory armor/weapon descriptions
- Auto-identify thrown potion of hallucination if hit with one 
- Manually set timeout when using #wizintrinsic command
- Some proposals for the Infidel role
- Minor sysconf edits
- Fix: shapeshifter types riding/being ridden while in an inappropraite
  form 
- Fix: if monster steed is teleported, take its rider along with it
- Fix: feedback tweak to commit regarding shapeshifters
  riding/being ridden
- Fix: zapping at an object with teleportation resulted in scrambled zap
  targetting as soon as any object was hit
- Monsters can use potions of oil as an offensive item
- README.md update
- Another README.md update
- Yet another README.md update
- Fix: github issue #63 - follow up to Moloch crowning proposal
- Fix: 'an ukulele'
- Fix: heap-use-after-free when attacking monster with potion
- Slight adjustment to previous commit (follow up to Moloch...)
- Fix: remove create_oprop() where it's not needed
- Fix: weird behavior with monster lookup for Kas and Mephistopheles
- Fix: player monster Monks wearing body armor
- Give The Rat King's scimitar the venom object property instead of
  just coating it in poison
- Fix: various issues with monster riders/steeds
- Fix: bugle playing should not scare certain monsters
- Fix: give rings/wands/artifacts in monsters inventory the same
  consideration as the player vs shock damage
- Dragons have proper alignment
- Fix: replace makeplural with s_suffix from recent commit
- Fix: monsters riding steeds were not highlighted in curses mode
- Adjust monster spawns in morgues in Vecna's domain
- Fix: small issue with #loot(ing) pets in a shop
- Fix: monsters unable to walk on water while wearing water walking boots
- Fix: a Centaur's kick is more powerful than other player races
- Fix: minor incorrect spelling 
- Fix: various curses color handling
- Fix: buffer underrun in curses
- Gold dragon revamp
- Fix: adjust list for what's acceptable as a monster rider/steed
- Fix: include steed's worn barding when tossing potions at it
- Fix: signal induced panictrace under curses
- Remove curse vs saddle/barding
- Get rid of any fleecy/bundle references
- Fix: zombies shuffles in your direction but they can't/don't move
- Scroll of magic dectection adjustments
- Remove skill flag > from #enhance menu
- Fix: prevent thievery skill from incrementing when only thing in
  the monster's inventory is gold
- Fix: allow Rogues to steal gold via thievery
- Bit of code cleanup in muse.c
- Additional monsters in green_mon()
- Fix: impossible if punished (ball & chain) when hero dies and killer
  rummages through inventory
- Fix: (racial monsters) xlogfile shows proper race of monster that
  killed the hero
- Ability to choose worn piece of armor to enchant/repair
- Fix: handful of issues discovered with air terrain and Vlad's cavern
- Fix: air terrain in Vlad's cavern and is_clinger()
- Fix: player can ride a wumpus across open air in Vlad's cavern
- Fix: barrow wight's spell attack should be last in the chain
- tty: use bright colors directly on supporting terminals
- Fix: polymorphed player (by monster) and their inventory
- Follow-up to last commit (player poly'd by monster)
- New wizard-mode command - #wizcrown
- Adjust feedback given when using #wizsmell
- Fix: logic with using forges
- Change how Infidel quest leader handles asking for the quest artifact
- Change penalty for Infidel casting spells without the Amulet of
  Yendor in inventory
- Fix: Infidels affected by slipping when throwing cursed objects
- Fix: Moloch uncursing cursed objects via prayer
- Adjustment to last commit (Moloch uncursing objects via prayer)
- Fix: could reach a forge while levitating
- Prevent healing magic abuse as Infidel w/o Amulet of Yendor
- Fix: gold dragon scale mail remained lit in bones
- Revamp descriptive hit messages (player vs mon, mon vs mon)
- Fix: price abuse with non-human shopkeepers, player matching race
- Fix: stoning touch monster vs monster not triggering
- Fix: no really, fix price abuse with non-human shopkeepers,
  player matching race
- Fix: object material detection bug
- Fix: elves were peaceful towards elven Infidels
- Shopkeepers will treat Infidels as a hated race during conversation
- Fix: shopkeeper prices weren't taking hobbit race into account
- Rogue thievery skill improvements
- Infidels shouldn't have clairvoyance as one of their starting
  spellbooks
- Fix: rogues shouldn't be able to use thievery while engulfed
- Adjust descriptive miss messages based on armor worn over other armor
- "ascended (in dishonor)" when align and align0 don't match
- Tweak baby/gold dragon tile
- Fix: certain hallucinatory messages in dumplog
- Fix: errors/warnings during tileset creation
- Fix: various monster/object tiles
- Some roles as giant race were missing chance to start with a gem
- Fix: better way to handle same race being peaceful towards Infidel
- Tweak maledictions to take Infidel into account
- Fix: crystal chests were affected by teleportation spell/wand
- Small tweak to latest malediction
- Fix: #wizidentify and inventory weight display
- Priests are penalized for firing/throwing edged weapons and landing
  a hit
- Cursed wand of light causes darkness, blindingflash() fix
- Cursed magic lamps give off darkness instead of light
- Fix: prevent various non-directional monster attack spells from being
  cast while player is invulnerable
- Tweak wizard spell skill levels


### Version 0.7.1

- Latest merges from 'vanilla' NetHack 3.6.6 official release (as of January 23rd, 2022)
- Initial preparation for new version (0.7.1)
- Centaurs can be knights
- Revamp corpse revival via zombie death, zombie illness timer
- Zombies can dig themselves out if buried
- Fix: 'Suddenly you cannot see it'
- Changes to wished for artifact chances, altar sacrifice gifts
- Player won't lose see invisible trinsic from magic trap if crowned
- Fix: odds for receiving an artifact gift via altar sacrifice
- New artifact (Dramborleg), minor fixes and changes to other artifacts 
- Fix: nimbly evading zombie bite attack while asleep
- Update various monsters per NetHack 3.7 changes/fixes
- Fix: crash during restore (hallucination resistance)
- Monsters being diseased uses proper timer
- New object - sling bullets
- Allow various sphere-type monsters to be genocided
- Wearing non-metallic gloves protect worn rings from shock damage 
- Fix: racial monsters can turn into zombies (again)
- New shields - shield of light, shield of mobility
- Monsters wearing non-metallic gloves protect their worn rings
  from shock damage
- Adjustment to last commit (monsters/gloves/rings)
- Fix: monsters/pets ailments not always cured via eating certain foods 
- Player can use a hammer and a forge to free themselves from ball & chain
- Attacking with a cursed weapon can sometimes do odd things
- Add condition for hitting oneself with a cursed artifact weapon
- Fix: chameleons stuck at never changing form again by selecting their
  own form
- Fix: hallucination would block object property descriptions
- Fix: horses that trumpet in fear
- Fix: a couple corrections to the last commit (horses and trumpets)
- Autorecover crashed games
- Fix: crash bug with 'attacking with a cursed weapon' commit
- Fix: some racial monster armor rules
- Fix: holdovers from ini_inv in ini_mon_inv
- Fix: minor error in this changelog
- Fix: quest artifact effects linger after giving it up to quest leader
- Fix: crash via draining a monster with wand of death to zero hit points
- Fix: quest leader wanting quest artifact back and it's thrown to them
- Fix: player getting credit for monster dying to another monster's
  armor blocking their attack (hated material)
- Fix: during initial player creation, prevent changing object material
  to something invalid
- Update NetHack 3.6.6 official release 'latest merges' date
- Fix: player poly'd into a giant had wrong movement speed and size
- Remove a bit of unnecessary code
- Block monster health regeneration in the Valley of the Dead.
- Paranoid water/lava
- Add option to filter by unidentified in item menus
- Fix: illithids should not be able to use brain-sucking tentacle
  attack while engulfed
- New monster - antimatter vortex
- Revert 'Remove a bit of unnecessary code'
- Increase the odds of encountering a neothelid
- Fix: fooproofing GEM_CLASS should only affect sling bullets
- Raise odds of a cursed wand exploding
- Monsters that are innately magic resistant are immune to magic missile
- Tweak how not abusing alignment affects altar sacrifice gift chance
- Change minimum dexterity needed to throw Xiuhcoatl
- Fix: some sokoban levels could easily be flagged as 'never solved'
- Tweak to Xiuhcoatl dexterity requirements
- Allow angel lord types to spawn with Demonbane again
- Fix: crash caused by last commit (angel lords / Demonbane)
- Fix: silence compiler warning in droppables()
- Casting stone to flesh at a monster in the process of being stoned will
  cure it
- Fix: zero out rooms[] in savelev()
- Fix: corpses showing as rotten after being removed from an ice box
- Allow player monster knights to be centaurs
- Fix: prevent racial elf/orc grudge between the watch/priests/shopkeepers
- Fix: "You feel a malignant aura surround it." and 'it' isn't visible
- Allow #wizgenesis to specify racial monsters
- Adjust 'prevent racial elf/orc grudge between the watch/priests/shopkeepers'
- Fix: recalc_mapseen vs rooms
- Fix: clear any hold over slow-stoning timers on statues being reanimated
- Fix: object stacking display bug in curses mode
- Fix: spellcasting pets casting invisibility on themselves when player
  can't see invisible
- Fix: Fell Beast's aren't supposed to become tame
- Fix: various object material fixes
- Fix: potential use-after-free when monster threw lit oil
- Racial statues
- Fix: tweak to racial statues commit
- Fix: zombies not reviving
- More descriptive monsndx panic feedback
- Fix: refusing to hand over the quest artifact to your quest leader was
  still flagging the quest as complete
- Fix: quest leader not engaging player after quest completion
- Fix: Illithids playing Convict role did not have psionic wave
- Fix: warning when eating corpse
- Fix: alignment hit when killing a mind flayer that started out as a
  peaceful monster and was transformed by a mind flayer larva
- Fix: 'impossible placing steed onto map, mstate:x, on Dlvl:x' when
  causing steed to lose tameness via blinding
- Fix: crash when leaving level after a monsters worn light source was
  destroyed
- Fix: same race could still be peaceful towards Convicts or Infidels
- Fix: how demons view Infidels (crowned and not crowned)
- Include race in statues of previous players, corpses/statues for bones
- Fix: don't allow vault guards to be displaced
- Fix: silence shadow warning, bones corpse has name of dead player
- Fix: untrap steed sanity
- Expand/simplify racial monster macro usage
- Fix: croc corpse poly produces 'flesh low boots'
- Adjust minimum distance a monster will cast fire/ice bolt or acid blast
- Fix: 'could maneuver over it' vs 'maneuver over it' in Sokoban
- Fix: cases where an artifact would be silently removed from the game
- Fix: breaking wielded fragile item against iron bars
- Weapon with poison object property will identify itself if it doesn't
  affect its target
- Livelog quest completion
- Fix: alignment penalty for killing summoned elves when naming Sting/Orcrist
- Fix: better handling of what monsters can ride/what monsters can be ridden
- Fix: bad cast making sp_lev chameleon light source
- Fix: various artifacts 'flickering no color'
- Monsters can drink potions of restore ability to cure cancellation
- Quest completion livelog tweaks
- Fix: Lieutenants wand creation routine
- Fix: no option to repair an attached rusted/corroded ball & chain
- Add missing feedback when abusing alignment
- Fix: peaceful monsters casting spells interrupting player actions
- Fix: black colored tame monsters not showing in the HTML dumplog
- Make destroy armor vs monster match effect vs hero
- Fix: itemized shop billing (curses mode)
- Mitre of Holiness buff
- Fix: refer to wielded iron ball as such, and not as 'flail' (Convict)
- Fix: scrolls of magic detection were not identifying artifacts
- Formatting clean-up in mcastu.c
- Bit more formatting clean-up in mcastu.c
- Fix: lack of capitalization (monster stole something feedback)
- Allow wizmode override of VOTD levelport restriction
- Fix: various issues with freezing water/lava and WDSM
- Fix: scroll of magic detection price so it's not unique
- Fix: monster interaction
- Formatting clean-up in muse.c
- Fix: non-illithid players selling spellbooks/scrolls to illithid
  shopkeepers for 10x the fair market price
- Fix: incorrect monster display on gravestone when killed via
  suffocation while engulfed and hallucinating
- Fix: Dragonbane passive damage to dragons not working if wearing dragon
  scales or scale mail
- Fix: gloves and hated container material
- Fix: exploding acid spheres and damage
- Additional bogusmons
- Fix: Show proper artifact name when it resists being held second
- Fix: unnecessary feedback when gaining an intrinsic from leveling up
  and the player already has that intrinsic
- Fix: monsters not curing themselves of sickness/disease as they should,
  other adjustments to muse.c
- Some more formatting tweaks to muse.c, fix for reflection_skip 
- Fix: unreachable return statement in dog.c
- Another adjustment to max hit points the Wand of Orcus can decrease per hit
- Convict's starting ball/chain is fully identified across the board
- Accurate tracking of how many doors seal after touching sokoban prize
- Fix: heavy iron ball rusting while being held when walking through
  shallow water
- Pseudodragon tweak
- Fix: potential dmonsfree error (monster vs monster and sear damage)
- Fix: two riding issues
- Fix: edit stock sys/unix/hints/linux file for installation use
- Fix: github workflow error (windows build)
- Fix: weird door artifacts in walls with certain vault special rooms
- Fix: towel wetness
- Fix: differing price with stacked object removed from the stack (shop)
- Return polearm skill for Priests
- Apply a wielded polearm with fire-command
- Fix: 'You feel guilty' feedback when alignment wasn't abused
- Fix: perm_invent update when applying a bag of tricks
- Fix: offhand bloodthirsty attacks (wielding Stormbringer and/or the
  Sword of Kas)
- Fix: some perm_invent curses mode issues
- Adjust alignment record penalty for Convict's killing 'always peaceful'
  monsters that became hostile just from seeing the Convict
- Format cleanup in zap.c
- Bit more formatting cleanup in zap.c
- Blessed objects in inventory have a chance to resist being cancelled
- Indestructable objects have same chance to resist cancellation as
  intelligent artifacts
- Tweaks to object cancellation routine
- Block 'artificial' player monster creation
- Fix: don't 'fall' and self-stone when flying down
- Add a space after <TAB> in curses mode (msgtype alert)
- Fix: odd messages caused by buffer re-use
- More varied feedback when gaining partial intrinsics
- Fix: proper feedback incrementing intrinsic resistances
- Container vulnerability to water damage
- Throttle excessive HP and En gains
- Don't stop running next to a peaceful monster
- Copyright info update for 2022
- Make demon lords hostile if wielding Demonbane as well as Excalibur
- Update linux and linux-debug hints files to suppress -Wunused-result
- Fix: monster hiding under hatching egg staying hidden
- Fix: random role segfault
- Fix: hellish flagged levels between Medusa's lair and the castle
- Correction to last commit (hellish flagged levels)
- Fix: showing that you're aware of demons because of wielding Demonbane
  when actually wielding Dramborleg
- Fix: incorrect weight of spellbooks received via altar sacrifice
- Cancellation explodes magical traps
- Falling through a hole or trap door will cause damage
- Fix: drinking from a forge
- Fix: fall depth from Castle to the Valley of the Dead


### Version 0.8.0

- Latest merges from 'vanilla' NetHack 3.6.6 official release (as of January 23rd, 2022)
- Initial preparation for new version (0.8.0)
- Fix: artifacts in wizkit.txt (wizard mode)
- Fix: warning: unsigned conversion from ‘int’ to ‘unsigned char:5’ changes value from
  ‘32’ to ‘0’ [-Woverflow] (ICED_MOAT)
- Fix: correction to wizkit commit
- Fix: shambling horror generating with different stats on bones file load
- Allow magical traps to be affected by wands of cancellation when monsters zap them
- Tweak to last commit (magical traps and cancellation)
- Fix: heap-use-after-free error in mpickstuff()
- Fix: heap-use-after-free error with passive_obj()
- Fix: heap-use-after-free in relobj()
- Fix: global-buffer-overflow in tactics()
- Fix: improved Charon interactions
- Fix: buffer overflow with player monster names
- Fix: heap-use-after-free errors in apply.c
- Fix: impossible if killed while poly'd into a monster that emits light
- Fix: invalid material 7 (otyp 427)
- Fix: unnecessary impossible() in restmonsteeds()
- Fix: impossible placing 'monster' over 'monster' at 'location'
- Some minor artifact tweaks and improvements
- Fix: heap-use-after-free errors with stacked ammo that have object properties
- Fix: ammo with object properties used against player (or mon vs mon) 
- Fix: potential that branch stairs won't form in the gnomish mines
- Fix: giants incurring luck penalty in sokoban after level was already solved
- Fix: warning: variable 'permdmg' is used uninitialized whenever switch case is
  taken [-Wsometimes-uninitialized]
- Fix: better handling when trying to drink from a puddle or sewage
- Fix: monster vs closed door messaging
- Fix: cursed light vs worn light
- Fix: tty map weirdness after 'full-screen' menu
- Fix: return DOORMAX to proper value, level creation fixes
- Fix: armor with 'excellence' object property adversely affecting charisma
- Infidels cannot regenerate hit points without the Amulet of Yendor in
  their possesion (pre-imbued Idol of Moloch)
- Fix: large box/chest spawning inside of another chest at Mines' End
  (Sewers of Waterdeep)
- Fix: wishing for gold pieces
- Allow specification of length of sparkle animation
- Fix: tweak goodpos() fix when a monster engulfs the player
- Fix: monster casting 'summon insects' spell at another monster would
  target the player instead
- Fix: sparkle animation patch
- Monster spell 'reflection' can timeout
- Knights get no caitiff penalty against undead
- Cancellation can remove certain spell effects
- Fix: monsters benefit from resistances imparted by worn/wielded gear
- Fix: rotting corpse part of a highlited pile
- Tweak monster gear preference
- Hurtling monsters/players tweaks and fixes
- Fix: preserve the Oracle and Charon's starting stats in bones 
- Tweaks to hurtling monsters/players
- More tweaks to hurtling monsters/players
- Oracle fully reset if oracle level is bones level
- Fix: tweak to last commit
- Wand of striking/force bolt spell has a chance to knock back its target
- Various spell handling tweaks
- Huge monsters easily climb out of pits
- Allow tipping container directly into another
- Fix: unpaid_cost() impossible
- Master Po
- Fix: wand of striking/force bolt spell knocking back its target
- Monks will not receive melee weapons via altar sacrifice if following
  weaponless conduct
- Fix: stack breakage issues
- Wearing gray dragon scales/scale mail or being a gray dragon can resist
  cancellation
- Adjust livelogging of altar sacrifice gifts
- Fix: prevent monster hurtling outside map
- Fix: various bug fixes (traps, monster engulfing trapped monster, etc)
- Fix: partial revert of commit 654b1ce
- Fix: missing guard in mk_mplayer()
- Fix: monster using a figurine of a genocided monster
- Fix: 'You feel guilty' out of nowhere
- Fix: non-lawful Angels spawning with lawful artifacts
- Fix: prevent dmonsfree with a couple mhurtle scenarios
- Fix: dmonsfree pending when hero was hit with drain-life artifact
- Tweak dmonsfree/mhurtle handling
- Fix: adjust minimum range in new_light_source()
- Fix: a couple bugs with kicking vs black dragon
- Fix: impossible 'Webbing over trap type'
- Remove unnecessary bit from last dmonsfree/mhurtle tweak
- Fix: partially revert 'a couple bugs with kicking vs black dragon'
- Formatting cleanup in u_init.c
- Fix: a couple monsters hiding in water cases
- Fix: same handling for hiding monsters in sewage and shallow water
- Tweak to 'You feel guilty' commit
- Fix: remove create_gas_cloud bit from evaporating fountains
- Collisions while hurtling can stone participants
- Fix: teleporting not unhiding polyed hero
- Fix: fixes and sanity checks for monster undetected and trapped states
- Fix: boulder-carrying monster killed twice
- Allow bone non-weapons to harm ghosts
- The Hand of Vecna
- Fix: player monster samurai having giant-like attributes
- Handle spiders, cockatrices in mbodypart
- Invoking the Staff of Aesculapius cures withering
- Implement new dragon armor system: DSM replaced by scaled armor (dtsund)
- Fix: a couple small issues with dtsund-DSM implementation
- Some minor player monster armor tweaks
- Tortle as a playable race
- Tortle tilemap
- Fix: remove unnecessary impossible in dealloc_obj()
- Toque tilemap
- Fix: cavepersons receiving spellbook via pat_on_head function
- Tortles receive a to-hit bonus when using spears or a trident
- Tortles and tridents
- Tortles as player monsters
- Tortles as aligned priests
- Fix: items destroyed by exploding chest
- Fix: teleporting onto pits
- Fix: remove unneeded bit from delobj()
- Fix: wishtracker not capturing every detail of the wish
- Tortle ability to hide in its shell
- Handle more cases when tortle is hiding in its shell
- Small tweak to thievery skill
- Fix: player monster Infidel cursed weapons were welding to their hands
- Tortles can't ride
- Force tortle to emerge from its shell if hiding in it and is undergoing
  polymorph
- Fix: don't pacify unique monsters when untrapping them from web
- Handle some more cases when tortle is hiding in its shell
- Handle even more cases when tortle is hiding in its shell
- A few more cases covered when tortle is hiding in its shell
- Implement a cooldown timer after a tortle hides in its shell
- Fix: warning: array subscript [0, 1] is outside array bounds
  of ‘const char * const[2]’ [-Warray-bounds]
- New monsters - sea tortle, tortle shaman
- Fix: polymorphing into a red dragon no matter what type of dragon scales
  are being worn
- Fix: crash when over-enchanting worn armor
- Fix: add message for failed fountain monster detection
- New branch: Goblin Town
- Prevent falling down past Mine Town if the Goblin King has not been
  defeated yet
- Fix: remove Goblin King template from quest nemesis block
- Tortle encyclopedia entry
- Add goblin shaman monster to Goblin Town
- Only certain types of monsters can randomly spawn in Goblin Town
- Tortles are not slowed when walking through sewage
- Tortles and giant turtles are vulnerable to cold-based attacks
- Player can create various types of armor and weapons using a forge
- Fix: tweaks to initial forging commit
- Fix: incorrect feedback with attacking monsters weapon resisting
  passive cancellation
- More tweaks to forging
- The Blacksmith's Cookbook
- Fix: improper feedback when a monster spellcaster destroys a secret
  door with fire/ice bolt spell
- Fix: improper death reason when killed by a thrown potion of oil
- Fix: end of game crash if killed by a lit thrown/scattered potion
  of oil before the potion could explode
- Fix: accessing freed memory after charging causes a ring to explode
- Tortles are not affected by turbulence whilst swimming
- Ensure tortles don't receive regular gear sacrifice gifts they can't use
- Tortles have a chance of spawning with an oilskin sack
- Corrections to commit concerning sacrifice gifts and tortles
- Make tortles immune to various attacks while in their shell
- Tortles gradually gain AC as they level up
- Fix: monster steed not properly released if rider is turned into a 
  mind flayer (mind flayer larva attack)
- More tortle tweaks - cooldown timer change for going back into shell,
  immune to bullwhip/bearded axe disarm while in shell
- Unintelligent monsters could ignore tortle hiding in its shell from a
  distance
- Fix: polymorphed Tortle werejackal has protective shell (issue #94)
- Fix: livelog reporting of various unique monsters
- Fix: livelog reporting of sacrifice gift that had a quantity larger
  than one
- Tweak to latest sacrifice gift livelog fix
- As a tortle, show enshellment via enlightenment and death reason
- Prevent tortle monks from using random kick attack against
  certain targets
- Fix: priest tortle couldn't wear the Mitre of Holiness
- Allow helm of speed to be made of more materials
- Give tortles a slightly higher weight carry cap
- Fix: weight of forged object could sometimes be incorrect
- Fix: forging and object properties, stacks
- Include player name in the title of HTML dumplogs
- Fix: in_hell_effects() and in_iceq_effects()
- Fix: Vecna wasn't always dropping its artifact
- Allow Dirge to slice through iron bars
- Tortle enshelling improvements
- Aklys improvements
- Goblin Town tweaks.
- More Goblin Town tweaks, defeating boss monster feedback
- Fix: re-enable gnome's default #chat sound
- Cursed wand of make invisible makes target visible
- Remove non-Knight reflection restrictions for the Magic Mirror of Merlin
- Prevent covetous monsters from riding other monsters
- Remove restriction of casting targeted/area of effect spells on
  the plane of water
- Prevent non-wizards from receiving a cornuthaum as a sacrifice gift
- Change critera for which body armor is allowed to be worn whilst
  having wings
- Fix: magic beam trap hit/miss feedback when target is unseen
- Illithids now 'force learn' psionic wave instead of starting with a
  spellbook of it
- Adjust blocked access to Mine Town/Mines' End while Goblin King is alive
- Fix: two-weapon paralysis
- Fix: update to 'two-weapon paralysis' commit
- Include tortle race in shopkeeper price adjustments
- Add chance for doors on jail cells in Goblin Town; adjust odds for shop
  and hammer spawning
- Enshelled tortle blocks stinging/crushing attacks
- Change 'splash of acid' to 'torrent of acid'
- Forge recipe improvements, slight recipe changes
- Fix: saving versus ball & chain
- Fix: a couple typos
- Eating the Hand of Vecna is bad for you
- Fix: the Hand of Vecna could disappear if the game is saved/reloaded after
  attempting to over-enchant
- Fix: 'qt_pager: message 90 not found. program in disorder!' during
  Rogue quest
- Fix: suppress Monk's extra kick attack when engulfed
- Give notice to infidels why they aren't regenerating hit points when
  they don't have the Amulet of Yendor in their possession
- Fix: prevent sokoban prizes from being engulfed
- Fix: certain forge recipes could cause splitobj panic
- Additional forge recipes, tweaks to a few existing recipes
- Adjust tortle movement speed
- Make crystal plate mail more rare
- Fix: 'Your leather studded armor seems to have holes for wings' when
  putting on dragonhide studded armor
- Fix: saving follow up
- Lower 'confuse monster' spell from level 2 to 1, make it part of a
  Monk's starting spell selection
- Fix: enchant armor selection and dragon scales; wishing for dragon
  scale mail
- Fix: What weird race is this? (Dem) Program in disorder!
- Fix: follow up to commit b616c1c
- Fix: melting ice timeout issues
- Fix: extract_nobj: object lost (magic beam traps)
- Enshelled tortles are supposed to be harder to hit
- Fix: failure restoring embedded dragon scales
- Fix: end of game crash during bones creation under certain conditions
- Fix: inconsistencies with illithids and psionic attack feedback
- Fix: Hand of Vecna cold damage should only apply to hand-to-hand combat
- Fix: nurse healing attack whilst 'wearing' nothing but the Hand of Vecna
- Switch heavy war hammer to regular war hammer (Goblin Town)
- Fix: correct feedback trying to genocide liches via throne genocide
- Fix: farlook information for monster steeds ridden by unseen rider 
- Allow barding to be forged
- Fix: quest leader not appearing when wishing for a quest artifact
- Fix: all unlocking tools being treated as magic when unlocking a
  trapped container
- Fix: bad grammar trying to put barding on a monster you can't
- Fix: non-zap buglets / oil burning secret door described as spell
- Successful spellcasting helps in remembering the spell cast
- Fix: follow up to 'oil burning secret door described as spell'
- Enshelled tortles are immune to most bite attacks
- Fix: reading scroll of enchant armor while confused and wearing dragon
  scales
- Fix: more blast vs spell
- Fix: could not dip worn rings while 'wearing' the Hand of Vecna
- Fix: remove unnecessary line in xkilled()
- Fix: alignment adjustment when casting 'cure sickness' on diseased
  tame or peaceful monsters
- Denote gender when a monster tries to disarm with a bearded axe
- Fix: typo/omission in hints files
- Create is_open_air() function
- Fix: prevent player from hiding their quest artifact in a container to
  avoid interaction with quest leader
- Fix: couple of revive() bits
- Allow intrinsic gain from pet's digestion attack
- Fix: pushing boulders, ball & chain handling in Vlad's cavern
- Fix: shallow water not drying up from objects being dropped into them
- Fix: metalvores eating a metal container would also consume all contents
- Fix: run travel was not stopping at terrain/objects of interest
- Fix: a couple more ball & chain in open air fixes
- Fix: saving/freeing ball and chain
- Fix: prevent 'oilskin oilskin cloak'
- Fix: crash/memory leak with thrown/falling boulders in Vlad's cavern
- Fix: prevent tame vampires from shifting back into animal/fog form
- Fix: Rider revival without corpse removal
- Update README file (install instructions), tweak a couple hints files
- Fix: cross-aligned player priests blasted by crowning gift weapon
  (Mjollnir)
- Fix: passive green slime attack not active
- Make looting less tedious
- Add new #wizkill command
- Don't allow fuzzer mode to use #wizkill 
- Fix: spawning giant <monster> in wizmode
- Fix: filename buffer overflow
- Monsters that gain resistance to a type of damage also lose any
  vulnerability to the same damage type
- Fix: foo, poisoned by a cursed amulet of life saving
- Adjust shambling horror attack/damage type combinations
- Fix: memory corruption issues caught with address sanitizer
- Fix: don't use 'zombified' as a death reason for player races that
  can't turn into zombies
- Fix: very small monsters (and players) vs shallow water/sewage
- Fix: poisonous corpse can give conflicting message (issue #95)
- Fix: remove unnecessary feedback when a monster goes berserk
- Fix: any artifacts can glow to warn of specific monsters
- Adjust feedback when hit by an artifact or weapon that deals poison
  damage


### Version 0.8.1

- Latest merges from 'vanilla' NetHack 3.6.6 official release (as of January 23rd, 2022)
- Initial preparation for new version (0.8.1)
- Fix: fire damage feedback
- Retool berserkers
- Make berserkers more willing to fight hand-to-hand
- Fix: compatability checking for development
- Add missing wizard mode commands to guidebook
- Fix: a couple minor issues with character creation
- Unlock trident skill for tortles playing a role that normally wouldn't have it
- Tortles, various monsters can move fast whilst underwater
- Fix: tortles and breathing underwater
- Fix: You are very fast from something
- Fix: sections of wall being visible when they shouldn't yet
- Fix: triggering knockback feedback on an already dead monster
- Sickness countdown
- Valkyries start with a spear instead of a long sword; Giantslayer changes
- Grid bugs don't have hands
- Pyrolisk, fire damage changes
- Mimics and dead trees
- Fix: typo in 'The Adventurer's Journal'
- Kathryn grudges all things that can sting
- Fix: warnings during compile using gcc version 11
- Make hitting with a wielded potion hit you far less
- Fix: engulfing mounted hero
- Adjust random item artifact chances
- Monsters will use teleportation wands offensively
- Change kitchen sink glyph and color
- Adjust random item artifact chances again
- Monsters can blind you with a camera
- Monsters will use wands of slow monster offensively
- Allow normal sight whilst underwater given the right conditions
- Fix: paranoid swim and wearing white dragon-scaled armor
- Fix: See_underwater as a tortle when saving/reloading, hidden
  monsters underwater
- Fix: better way to handle hidden monsters underwater; new hints file
- Minor README update
- Fix: ammo damage from the Longbow of Diana/Crossbow of Carl
- Fix: silence several compile warnings (clang)
- Fix: corrections to last commit (silencing compile warnings w/ clang)
- Fix: move check for starting L1 spellbook to include non-random spellbooks
- Fix: lit status of worn light sources appearing twice in inventory
- Fix: could not blind self with an expensive camera
- Monsters can use rings/boots of levitation
- To-hit calculation on the bottom line
- Magic negation (MC) value on the bottom line
- Fix: MC and to-hit status not appearing in dumplog
- Allow looting of containers that are in water
- New object: goggles
- Fix: quirk with underwater vision
- Revamp monsters eating containers
- Fix: tipping into containers
- Resurrecting named pets will show their name once resurrected
- Fix: force-fight 'unknown obstacle' descriptions
- Fix: to-hit display weirdness under certain conditions
- Fix: to-hit display revealing weapon enchantment, other tweaks
- Fix: impossible from splitting named stack on bill
- Physical knockback attack doesn't affect unsolid monsters
- Weapon familiarity
- Better way to suppress weapon enchantment on to-hit display, include
  rings of increase accuracy
- Fix: exploiting artifact name rejection
- Fix: unique monster doppelganger and bones bug
- Better way of handling livelogging of destroying a player's ghost
- Goblin Town expansion: Gollum's Cave
- Adjust Graz'zt's magic resistance
- Sanctum/ascension run revamp: part 1 (sanctum)
- Feedback for gate forming when u.uachieve.amulet is triggered
- Sanctum/ascension run revamp: part 2 (Track Lucifer alive/dead
  status, foundation for Purgatory levels)
- Lucifer tweaks
- Fix: shambling horror pokedex lookup
- Remove unused return portals (sokoban <-> purgatory)
- Place Fort Ludios portal in the first eligible vault generated
- Wielding Vorpal Blade protects against decapitation attacks
- Allow centaurs to be Infidels
- Remove 'noteleport' flag from the three-tiered demon boss levels
- Remove unnecessary mspec code from mcastu.c
- Fix: zapping self with slow monster doesn't always cause slowness
- Fix: logic with monsters recharging wands
- Refactor intelligent monster recharging code
- Fix: a bit more logic with monsters recharging wands
- Fix: monster recharging a wand of wishing
- Fix: misc issues in searmsg
- Try for unique topten names for Astral player monsters
- Fix: ghosts appearing as 'name' and not 'name's ghost'
- Prevent renaming bones monsters of former heroes
- Fix: use after free when thrown object is destroyed by passive attack
- Fix: memory leak when selecting appropriate material for sacrifice gift
- Fix: memory leak when looting monster
- Fix: two use-after-free issues revealed by asan
- Fix: Minetown food shop doesn't convert to health food store for monks
- Fix: remove/replace compromised web link from sys/winnt/Install.nt
- Fix: various issues with monster wishing
- Fix: add null check for mdef in explum()
- Fix: remove shop cost fix for pricing exploit that no longer exists
- Fix: use-after-free when fountain dipping
- Fix: wishing for a helm of telepathy / gauntlets of fumbling
- Fix: minor feedback issues with Nazgul's scream attack
- Nazgul shouldn't wear the rings of invisibility they carry
- Fix: avoid segfault when null obj passed to water_damage()
- Purgatory map creation
- More purgatory map work, ice queen branch tweaks
- Fix: more use-after-free when fountain dipping
- Fix: use-after-free with engulfer in xkilled()
- Fix: potential use-after-free in expels()
- Fix: AD_LOUD not doing damage; new monster - celestial dragon
- Fix: boolean statement in AD_LOUD (mhitu.c)
- Fix: AT_SCRE / AD_LOUD in mhitm.c 
- Add bits to linux-debug hints file for fuzzing with ASAN
- Fix: segfault when hitting a monster with a poisoned glass object
- Fix: Sunsword and Trollsbane destroying corpses from a distance
- Fix: 'dmonsfree: 1 removed doesn't match 2 pending' when knocking
  back a monster into a pool/lava via force bolt/wand of striking
- Add realtime display option
- Fix: add some missing 'ifdef REALTIME_ON_BOTL' statements
- Fix: cursed wands of invisibility cannot revert intrinsic
  invisibility
- Update various hints files (clang, hints for github workflow)
- Fix: 'Your body rises from the dead as a human zombie...' as an
  illithid
- Fix: enable -Wstringop-truncation and -Wunused-result, correct
  warnings revealed
- Fix: enable -pedantic and -Wunreachable-code, correct warnings
  revealed
- Enable -Wimplicit-function-declaration and -Wimplicit-int
- Fix: windows GUI splash screen
- New monster spell: call undead
- Edit macosx10.14 hints file for github workflow
- Fix: prevent impossible lighting a massive stack of candles
- Finish first Purgatory level
- Fix: peaceful Lucifer
- Suicide by wand
- Enable -DEDIT_GETLIN for public server play
- Secespita tweaks
- Finish second and final Purgatory level
- Adjust odds of the Wizard of Yendor resurrecting (again)
- Fix: if poly form was killed from being hit by the Wand of
  Orcus, max hitpoints were not restored when reverting to
  original form
- Forging artifacts
- Fix: poisoned objects becoming unpoisoned would not update
  their status in curses mode (perm_invent)
- New forging artifact recipe - Glamdring
- New forging artifact recipe - The Staff of the Archmagi
- Make spheres' explosions passive as well as active
- Don't grant tame djinni from lamps/potions if player is petless
- The Staff of the Archmagi keeps curse protection from Magicbane
- Change feedback when the Staff of the Archmagi absorbs a curse
- New forging artifact recipe - Shadowblade
- New forging artifact recipe - Gauntlets of Purity
- Gauntlets of Purity gives damage bonus while worn for lawful
  unarmed players trained in martial arts
- New forging artifact recipe - Ashmar
- Fix: paranoid_swim to use yes/no vs y/n
- Remove SPFX_EXCLUDE
- Ashmar artifact tweaks
- The Gauntlets of Purity cannot be worn by the impious
- Encyclopedia entries for all of the new forged artifacts
- If the Gauntlets of Purity are forced off of your hands, any
  wielded weapons are also forced to be dropped
- Don't forget shields...
- Refactor a couple artifact functions (lawful/chaotic)
- Refactor non_wishable_artifact()
- Refactor: encode artifact material in artilist
- Fix: elemental vulnerability effects against monsters
- Fix: various issues with the Sword of Annihilation's attack
- Fix: proper credit for passive disintegration
- Minor Purgatory tweaks
- Fix: crash when unlocking a container with a wielded cursed
  locking tool
- Fix: impossible, eel hiding out of water (gnomish mines)
- Fix: wishing for an object with an invalid material in wizmode
  while fuzzing
- Fix: hilite_pile shenanigans with ball & chain
- Fix: monster recharging wands inside of a sack
- Fix: some minor issues with spear traps
- Fix: learning wielded weapon familiarity from unrelated thrown
  weapons
- Fix: guard against monster casting a spell at an already dead
  monster
- Give players a hint about the open air terrain on the second
  icequeen level
- Fix: impossible (hiding under nothing?)
- Tweak the type/number of monsters in Gollum's cave
- Prevent waking up Lucifer before actual encounter
- Allow pokedex for shambling horrors in wizmode
- Fix: sequencing issue: dismounting from dying steed
- Greatly increase odds of sewage vs water rivers forming in the
  Gnomish Mines
- Fix: impossible (dmonsfree) when monster is killed by monster
  spells that call explode()
- Allow pokedex to show shambling horror stats after one has been killed
- Fix: AT_MAGC / AD_FIRE attack in regards to demons
- Some more Purgatory tweaks
- Fix: AD_LOUD feedback cases
- One more Gollum's cave tweak
- Fix: crash when wearing white dragon-scaled armor and no boots while
  100% fire resistant
- Correct 'too many hooves' feedback
- Fix: impossible 'mon_arrive: no corresponding portal?' when leaving Purgatory
- Fix: unable to dig if wielding a cursed pick-axe/mattock
- Another Goblin Town tweak
- Fix: crowned Infidel that started as a centaur kept jump ability
- Fix: make the Eye and Hand of Vecna immune to fire/lava damage
- Fix: Dramborleg damage output
- Fix: dead trees dropping fruit/petrified status, icequeen 2nd map tweaks
- Fix: placement of monster rider/steed when separated
- Allow magic lamps to operate underwater
- Tortles that start with a trident, the trident is rustproof
- Fix: feedback for Infidel killing themselves from life force drain
- Use actions/checkout@v3 (github workflow)
- Fix: remove melting ice timeouts when terrain changes
- Fix: latest Xcode build issue
- Fix: underwater vision update frequency
- Update copyright year to 2023
- Fix: the Wizard Of Yendor would respawn every time the player entered
  Purgatory
- Fix: dried up puddles would not unhide the monster hiding in it
- Fix: placing antimatter vortex over itself after disintegrating
  a shapechanger
- Fix: Sword of Annihilation's disintegration vs target and target's worn armor
- Fix: Angelslayer's fire damage vs target, Sword of Annihilation's target
  armor tweaks
- Fix: the Sword of Annihilation could disintegrate pieces of armor that
  are inherently disintegration proof
- Fix: instances where article was missing from the Gauntlets of Purity
- Fix: underwater far vision in unlit areas
- Fix: heap-use-after-free in munstone() function
- Bourbon and Ozzy birthdays
- Fix: monster spell 'Summon nasties'
- Fix: missing a couple cases where Death_resistance should have been included
- Blowing a whistle will identify it
- Identity of the Riders hidden via farlook or 'More info about' prompt
- Fix: thrown potions of polymorph by hostiles at hero that hit peacefuls
  anger them
- Fix: cure sickness being cast on a peaceful monster that wasn't sick would
  anger it
- Fix: queen ants were laying queen ant eggs
- Fix: higher level tourist player monsters spawning with eggs that could
  hatch S_ORC type monsters
- Fix: giant crocodiles and their eggs
- Goblin King feedback if the player is wielding Orcrist or Glamdring
- Fix: memory leak in oracle-3 map
- Fix: mon_currwep not set to NULL if the monsters wielded weapon was made
  of glass and it shattered
- Fix: heap-use-after-free when projectiles rusted away on passive rust attack
- Fix: Grimtooth's instakill vs elves
- Intelligent peacefuls avoid digging shop or temple walls
- Fix: disarming swallowed hero
- Fix: mkgate() was called every time the Amulet of Yendor was dropped and
  picked back up while in the Sanctum
- Fix: feedback when attacking with certain artifacts (Angelslayer, Grimtooth,
  the Sword of Annihilation)
- Fix: prevent Lucifer from escaping the Sanctum
- Fix: domonability declared twice in extern.h
- Fix: ancient subroom location bug
- Fix: teleported water creature hiding in dry land
- Reimplement damage bonus from worn gloves enchantment if fighting bare-handed
- Generate random eroded, erodeproof, or greased items
- Fix: missing period
- Reading a blessed scroll of amnesia allows option to reset accumulated skills 
- Fix: prevent Valley of the Dead bones files
- Fix: worn rings covered by worn gloves could do material damage to monsters
- Tweak to Valley of the Dead bones files
- Fix: don't allow non-artifact sacrifice gifts to spawn with erosion
- Prevent certain items from random erosion
- Fix: monsters could jump into poly traps when trapped
- Fix: suppress Hand of Vecna cold-based attack if target is already dead
- Update to 'monsters could jump into poly traps when trapped'
- Fix: containers flagged as waterproof were not subject to water damage
- Allow food appraisal intrinsic to warn against the Eye of Vecna
- Tweaks to maybe_unhide_at()
- More tweaks to maybe_unhide_at()
- Fix: splash screen (windows build, GUI)
- Another attempt to fix the windows GUI splash screen
- Fix: missing bracket for Lucifer tile entry
- Fix: do not allow the Idol of Moloch to be cancelled


### Version 0.8.2

- Latest merges from 'vanilla' NetHack 3.6.7 official release (as of February 24th, 2023)
- Initial preparation for new version (0.8.2)
- Prevent bypassing demon boss lairs via level teleport
- Tweak feedback if killing yourself from a cursed weapon
- Fix: logic in amulet/tool naming after 3.6.7 merge
- Fix: the Riders being disintegrated and not leaving a corpse
- Fix: monster jumping logic
- Monsters flagged as a lord/prince don't spawn with eroded gear
- Fix: Infidel role - repeatedly chatting with your quest leader causes
  'completed his/her quest without incident' livelog to run every time
- Fix: track the Idol of Moloch's imbuement properly
- Fix: 'potentially uninitialized local pointer' from last commit
- Fix: describing barding
- Fix: object materials - several wish parsing bugs
- Fix: x-ray vision issues when using multiple objects that grant it
  at the same time
- Fix: dwarvish monks that started with a pick-axe were wielding it
- Fix: artifacts warning of monsters would flicker on/off for every
  monster kill
- Shield skill
- Utilize shield skill to shield bash monsters the player is attacking
- Prevent shield from being taken away via dwarvish bearded axe attack
  if expert skill or higher in shields
- Wand of wishing in explore mode charges
- Some descriptive miss messages for thrown/fired objects at the player
- Fix: crash if attacking bare-handed and not wearing a shield if role
  has shield skill
- Tortle shamans are telepathic and will grant telepathy intrinsic if eaten
- Gnomes hate eggs
- Fix: some 'gnomes hate eggs' tweaks
- Adjust Wizard of Yendor resurrecting back to normal odds
- Regular eggs have a small chance to cancel gnomes
- Mind flayer larva victims, if peaceful, will become hostile when turned
  into a mind flayer
- Fix: wrong deity was made upset if caught trying to ascend while wearing
  a helm of opposite alignment
- Fix: paranoid swim would kick in while phasing under certain conditions
- Remove unnecessary code from in_hell_effects() and in_iceq_effects()
- Fix: illithids receiving metallic helmets for sacrifice gifts
- Tweak to last commit (illithids and metallic helmets as sacrifice gift)
- Huge or gigantic monsters on a pool/lava tile will not be instakilled
  if a boulder fills up the pool/lava spot
- Fix: engraving with a bladed weapon or magic marker would not update
  permanent inventory
- Fix: objects created with an object property would sometimes be blessed
  and cursed simultaneously
- Fix: corrections to separate_steed_and_rider() function
- Fix: forged objects having an object property when they shouldn't
- Fix: player poly'd into a centaur reported incorrect body parts
- Fix: tweak to forged objects having an object property when they shouldn't
- Fix: multishot bonuses with various races
- Fix: locking then unlocking the Sokoban closet doors prevents them from
  being sealed
- Fix: inconsistent attack behavior while submerged in water
- Follow up to 'inconsistent attack behavior while submerged in water'
- m_canseeu() allows seeing the player underwater
- Being underwater (player or monster) provides protection against fire/acid
  attacks
- Fix: heap-use-after-free when tipping a container with a wand of
  cancellation into a bag of holding
- Fix: worn chromatic dragon scales as a giant could be destroyed if poly'd
  into a smaller monster and then revert back to giant form
- Fix: Izchak keeps transforming back into angelic form after every death
- Fix: some follow-up to last commit (Izchak and angelic form)
- Fix: zombie bite attack slipping off of a greased/oilskin helm feedback
- Fix: player monsters inheriting race flags from the player 
- Fix: remove urace test from player race macros
- Fix: prevent forging an object if neither base objects material is valid
- Fix: multiple objects in a stack highlighting
- Fix: refactor and fix various bugs in artifact_hit() function
- Fix: Giant race using a club damage bonus was applied to every weapon
- Fix: armor losing fooproofing via a monsters 'destroy armor' spell was
  not updated in perm_invent (curses)
- Fix: grammar when welcoming the player back to a saved game
- Fix: help prevent inherently fixed objects from spawning eroded/corroded
- Make the archangel Saint Michael a unique monster
- Fix: stealth and waking nearby monsters during combat
- Fix: missing race from 'Dark Knight' template
- Create new artifact flag and macro (SPFX_FORGED and is_forged_artifact())
- Gauntlets of Purity tweaks
- Fix: more separate_steed_and_rider() tweaks, other monster/steed tweaks
- Fix: fire/acid explosions not affecting various objects in open inventory
- Fix: Demonbane's instakill damage vs demons
- Fix: monsters sonic attack subject to passive attacks from player
- Fix: remove unnecessary check for Monk's random kick attack
- Fix: Vecna's artifacts never forming if destroyed over lava or a pool
- New forging artifact recipe - Hammer of the Gods
- Fix: genociding all lich types and alhoons before destroying Vecna
- Fix: some Hammer of the Gods tweaks and fixes
- Some format cleanup in artifact.c
- Fix: monsters stuck in a loop switching between melee and ranged weapon
- Fix: monster steed being knocked back while being ridden
- New forging artifact recipe - Tempest
- Fix: missing window tiles for new explosion type/color
- Fix: no really, fix missing window tiles for new explosion type/color
- Fix: adjustments to racial abilities
- Fix: one compile warning, formatting cleanup from last commit
- Fix: prevent candles from spawning as 'rotted'
- Livelog wishes granted exclusively by the Magic 8-Ball
- Encyclopedia entries for latest two forged artifacts
- Wielding or wearing an object that grants petrification resistance
  will cure it if in the process of being petrified
- Fix: some Tempest tweaks and fixes
- Defending against cancellation odds adjusted when wearing gray
  dragon-scaled armor
- Fix: various monsters movement speed while underwater
- Don't drop aklys on the floor due to slippery fingers
- Wand of speed gives player temporary speed and potion gives intrinsic
- Changes to 'a'pplying whips
- Consolidate some fire_ and lava_damage() code, and make fiery items
  fireproof
- Fix: two compile warnings, a bit of formatting cleanup from last commit
- Monsters can utilize free action
- Fix: objects being created with materials they shouldn't have
- Restore Tourists skill in twoweaponing to 'skilled'
- New forging artifact recipe - Ithilmar
- Fix: sokoban prizes could still spawn with an invalid material
- Add a couple more sources of cancellation protection
- Fix: prevent Bourbon, Ozzy, and Kathryn in both forms from being
  polymorphed
- Implement 'race0' field to xlogfile, denoting starting race
- Allow some monsters to break boulders
- New forging artifact recipe - Armor of Retribution
- Fix: case for armor falling off when poly'd (Armor of Retribution)
- Drow as a playable race (initial commit)
- Shadowblade tweaks
- Drow ability - conjuring an aura of darkness
- More Drow abilities, strengths and weaknesses
- New object - potion of drow poison
- Fix: correct Drow entry in role.c
- Drow race starts with a pet spider (role dependent), new spider
  type monsters
- Fix: silence warning during windows build from last commit
- Allow Drow to move through webs like spiders can
- Sting warns of spiders
- Adjust weight of large/giant/gargantuan spiders
- Fix: place poisoned/tainted weapon attacks (monster vs player) in
  correct location
- New material type - Adamantine
- Improvements to ultravision; fix to last commit (new material)
- New objects: Drow weapons
- Fix: ultravision and portable light sources
- Fix: material probabilities
- Fix: no really, fix material probabilities
- Fix: temporary light/dark sources affect Drow the same as permanent
  sources
- Drow monsters vs player/other monsters are affected by light/dark
  same as a player Drow
- Fix: prevent using level teleport/cursed potion of gain level to
  bypass the first level of Purgatory
- Fix: permanent inventory not updating various object property
  discoveries (curses mode)
- New material type - Spider Silk
- Drow truly hate light (player and monster)
- Fix: prevent uncharged wands of wishing from randomly spawning on
  the castle level
- New objects: Drow armor
- Added Drow gloves, some other tweaks
- The Staff of the Archmagi will either give off light or darkness
  depnding on the race that wields it
- Fix: player monster Drow spawning with inappropriate objects
- Fix: worn glass helmets would never break when boulders hit them
- Glass armor that blocks an attack has a chance to break
- Improvements to how Drow weaknesses are handled
- Certain Drow armor becomes brittle in the presence of light
- Drow weapons are also susceptible to light
- Catch alternative spellings for dark elven gear
- New monster: Black fungus; some fungal corpses can be used to make
  certain potions
- New monsters: several new drow-based creatures
- Fix: cave lizards can wear barding
- Fix: crash in mount_steed() checking for forged artifact barding
- Fix: a couple crash bugs related to breaking objects and
  poisoned/tainted status
- Fix: order of events when breaking a weapon while attacking a monster
- Drow monsters, driders can cast an aura of darkness around themselves 
- Various Drow monsters can spawn riding drow-specific steeds
- Drow that can cast 'summon insects' will summon spiders instead
- Driders can shoot webbing at a distance at its target
- New monster: Shadow dragon
- Fix: bugs with shadow dragon commit
- Fix: some forge recipes
- Add various Drow objects to the allowed forge recipe list
- Fix: remove unnecessary code from shadow dragon level drain attack
- Fix: feedback when engulfed by a water elemental/sea dragon and
  the player is amphibious
- Shadow dragons emit a continuous aura of darkness
- Drow mages and clerics are telepathic
- Fix: webs in/on water
- Allow monks with intrinsic water walking to enter into and stay in
  pools/moats
- Fix: fix for the 'webs in/on water' patch, as it made the game unwinnable 
- Ogresmasher knockback
- Fix: ogresmasher knockback monster vs player, some feedback formatting
- Don't allow drow mummies/zombies to cast aura of darkness
- Fix: zapping cancellation at magical trap
- Fix: drow monsters using aura of darkness
- Fix: drow and untrapping webs
- Tempest when wielded grants shock resistance to items in open inventory
- Drow and spellbooks of light
- Fix: error in drowcraft forge recipes
- Drow that pray shouldn't be surrounded by 'shimmering light'
- Fix: stacked food objects, weight not updating for split object in curses
  mode if eating was interrupted
- Fix: Drow player was considered being in the light if in a lit room while
  using a temporary source of darkness while blind
- Fix: hurtled monster logic
- Fix: Drow players shouldn't start with lamps
- Fix: restore aklys tether glyph to its original color
- Fix: minor issues with Mines' End 'Orc Temple' map
- Remove peaceful temple priest protection spell spam; fix for monster
  using aura of darkness feedback
- Fix: feedback when objects break
- Fix: minor feedback bug when player's shield blocks a ranged attack
- Drow and Elves, cannibalism and same-race sacrifice
- Fix: damage types via monster lookup
- Fix: 'altar is stained with' feedback, drow/elven same-race sacrifice
- Fix: quest leader dying by means other than by the player directly
- Light haters, exploding yellow lights, and various objects that give
  off light
- Fix: Angelslayer's chance of special attack worked with other fire-based
  weapons
- Fix: drow objects breaking based on incorrect position info
- Fix: thrown fragile objects hitting monsters
- Fix: program in disorder from last commit
- Fix: locking/unlocking containers in sokoban
- Include drow weapon damage bonuses to the pokedex
- Cancelled potions of drow poison turn into fruit juice
- Fix: more cases where certain objects shouldn't spawn eroded
- Fix: some doors would spawn booby-trapped as early as dungeon level one
- Fix: aura of darkness could snuff out/curse other sources of darkness;
  shadow dragon scales aura of darkness hurting light haters
- Fix: revert commit 06c26ff, extra check for what can/can't spawn eroded
- Fix: racial drow were unaffected by hates_light(), gremlins fleeing from
  sources of darkness
- Fix: celestial dragon scales/dragon-scaled armor and flying
- Fix: crash when throwing and hitting a monster with a cream pie or most
  spit-based objects
- Fix: make elemental vulnerability more consistent
- Fix: better fix for cream pie/spit object throwing
- Adjust feedback when freezing a liquid underfoot while riding a steed
- Fix: dismounting crash from last commit
- Fix: proper handling of flying/clinging u.usteed from last commit
- Fix: poisoned/tainted weapon stacks were merging with non-poisoned/tainted
- Add phasing status to botl
- Fix: crowned infidel in demonic form not resistant to death magic
- Ranger artifacts will create ammo based on the race of the Ranger
- Ranger quest artifact, if blessed, has a chance to produce ammo with
  an object property
- Fix: livelogging when Kathryn the Ice Queen is defeated while hallucinating
- Crowned infidel receives sleep resistance
- Fix: dragon-scaled armor that was lit would still shine after taken off
- Drow convict can safely handle the Iron Ball of Liberation
- Convicts can use and train up thievery skill
- Player race Gnomes will not start with eggs in inventory
- Magic lamps won't be extinguished by a rust trap
- Fix: Drow and untrapping monsters from webs
- Drow aura of darkness and sources of 'artifact light'
- Convict thievery skill adjustment
- More 'gnomes hate eggs'
- Fix: The 'weapon' welds itself to the 'monsters' hand!
- Fix: some light edits to light code
- Fix: Drow shouldn't receive a shield of light as a sacrifice gift
- Fix: minor feedback bit with aura of darkness and objects using
  artifact_light()
- Fix: the Wizard of Yendor could be awakened prematurely on wizard1
  level
- The Red Horse has a chance to spawn wearing barding
- Fix: blanked spellbooks from reading not updating in permanent inventory
- Fix: various issues with monsters setting off container traps
- Git info formatting in dumplog
- Show spells known in the dumplog (text and html).
- Show play time/wallclock time in the dumplog (text and html)
- Fix: cavepersons/priests could still have certain weapon skills
  unrestricted that shouldn't have been
- Reflection spell improvements
- Restore ability spell is now directional
- Tweak to 'restore ability spell is now directional'
- Phasing allows escape from being engulfed
- Fix: carried glass objects always broke when used
- Fix: invoking Shadowblade to cause fear could scare monsters it shouldn't
- Fix: oilskin sacks/cloaks spawning with materials other than cloth
- Fix: vampshifted Vlad in bones
- Fix: separate monster from player when determining number of objects
  in inventory to cancel
- Don't allow poisoned/tainted sacrifice gifts when using lawful or neutral
  altars
- Fix: being able to wish for more than one potion of drow poison
- Fix: even more separate_steed_and_rider() fixes
- Fix: wishing for a string with mismatched parens, that doesn't contain any
  actual wish before them
- Fix: material damage message firing after target was already killed
- Fix: racial weapon selection flub for lieutenants
- Fix: dialogue formatting with the Priest quest
- Fix: double message when entering the lemure pits (hellc demon boss level) 
- Fix: heap-use-after-free when long worms where slimed by a player poly'd
  as green slime mid worm hit loop
- Fix: heap-use-after-free caused by an exploding forge destroying the same
  dipped object twice
- Fix: weapon material damage adjustments
- Fix: memory leak in spitmm()
- Fix: global-buffer-overflow when initializing drow characters
- Monsters that spawn with spears have a chance to spawn with a stack of them
- Minor tweaks to receiving a spellbook via altar sacrifice
- Make scrolls of blank paper and potions of water known
- Allow knights to tame dragons of the same alignment
- Spear trap tweaks, remove multiple random death messages
- Cut down on feedback when melee attacking a green dragon and not being
  affected by its hide
- Allow a worn not cursed amulet of life saving to save the player if they
  fall in an 'open air' space
- Make Saint Michael a slightly tougher opponent
- Fix: place trident skill initialization (Tortle) in correct location
- Wearing the Armor of Retribution increases carry cap/max carry cap
- Allow monsters wielding a digging tool to break boulders
- Feedback for trying to wear gloves/gauntlets over the Hand of Vecna
- Fix: prevent #wizbury from being used in open air spaces
- Cleaver multi-attack changes
- Fix: bug with how pets consider intrinsic-granting corpses as treats
- Fix: heap-use-after-free when attacking with an object that breaks
- Fix: heap-use-after-free when a monster attacks with an object that breaks
- Fix: explicitly state various materials so certain forge recipes will work
- Fix: various fixes to polyself.c
- Player monster racial settings refactor
- Fix: brain fart in last commit (player monster racial settings refactor)
- Fix: Drow were not preserving their special deities after a save/reload
- Fix: monster attack that hurtles player caused monster to lose track of
  the players' position
- Consolidate some race-specific functions in u_init.c
- Greased objects have a chance to resist being stolen
- Fix: certain spellbooks would have the wrong weight if received via prayer
  or altar sacrifice
- Fix: material sear messages were being suppressed vs the player
- Adjust some probabilities with grease wearing off during theft attempt
- Include oilskin sacks and the like to have a chance to resist being stolen
- Fix: morgues don't make corpses from races that don't leave a corpse
- Wearing the Hand of Vecna prevents material damage from worn rings
- Fix: crash when poisoned/tainted weapon breaks (monster attacking player)
- Fix: swimming underneath monsters while engulfed/grabbed/swallowed
- Fix: boulder trap pathing could go out of bounds
- Fix: crash when Bourbon or Ozzy were level drained below zero
- Fix: wishing for regular gloves
- Fix: crash if the worn Hand of Vecna became glib
- Fix: weight of recipe stacks not recalculated when forging
- Fix: 'You detect the presence of' array was off by one
- Fix: prevent indestructible objects from being destroyed by a cursed
  bag of holding
- Fix: breaking wielded weapon in combat caused heap-use-after-free if
  the objects material caused sear damage to target
- Fix: curses mode - prevent spurious cursor updates
- Fix: SET_TYPLIT if/else chain broken
- Fix: whitespace in feedback (mcastu.c)
- Fix: incorrect value used in determining mspec->used for monster
  spellcasting
- Fix: ukiller can't be null if killer wants to go through player inventory
- Fix: minor bugs while underwater and trying to fight monsters above water
- Prevent wishing for objects with an invalid material whilst fuzzing
- Don't count non-magical sacrifice gifts towards total gifts received
- Tweaks to thievery skill
- Fix: update to 'ukiller can't be null' fix
- Fix: couldn't forge the Armor of Retribution due to the Sword of
  Annihilation being indestructible
- Fix: the Sword of Annihilation wasn't being used up when forging the
  Armor of Retribution


### Version 0.8.3

- Latest merges from 'vanilla' NetHack 3.6.7 official release (as of August 28th, 2023)
- Initial preparation for new version (0.8.3)
- Allow self-targeting advanced spells when blind
- Fix: Dexcellence
- Stunned/confused jumping
- Black dragon breath documentation
- Don't allow monsters to rise through Purgatory ceilings
- Fix: Demon lords of your alignment should demand less bribe money
- Fix: tortles should feel comfortable in fog clouds
- Make retouch_object more consistent
- Fix: gloves shouldn't allow you to wield banes
- Fix: drop shield when the Gauntlets of Purity remove themselves
- Fix: flying with wings and taking body armor on/off
- Fix: Elemental Planes overview
- Artifact Messages, Angelslayer/Grimtooth tweaks
- Fix: Grease shouldn't protect invocation items from Rodney
- Fix: Allow twoweaponing artifacts in polyform
- Fix: polyform twoweaponing opposite-aligned artifacts
- Monsters zapping wands of striking can break iron bars
- Fix: Safes don't have lids
- More hellfire tweaks 
- Fix: Blessed genocide of 'h'
- Fix: Attempts to engulf spectres &c. from a distance
- Fix: Messages for unseen events
- Fix: defensive knockback
- Fix: Staff of the Archmagi, drow, and light-haters
- Fix: Autodigging and forcefight-digging can sear you
- Fix: Silver dragon scaled armor was not granting cold resistance
- Fix: bracers are plural, not singular
- Fix: a couple places where traps could spawn on top of altars
- Dark elven bracers and dual wielding/two-handed weapons
- Fix: monsters digging through boulders and not seen
- Reduce AC bonus from shield skill if wearing bracers
- Fix: feedback for gaining telepathy via food when already telepathic
  via race
- Fix: release obufs used by sortloot
- Fix: tweaks to objnam.c (dragon-scaled armor)
- Fix: wearing dragon scales and reading an unidentified scroll of enchant
  armor
- Fix: player objects destroyed via passive disintegration handling
- Move the Wizard's Tower into its own branch
- Fix: walls and doors were not being seen in the dark via ultravision
  at a distance
- New race/role combinations: drow monk, tortle samurai
- Flag alhoon as M2_NOPOLY
- Allow vorpal jabberwocks to be steeds
- Make elf zombies and mummies color bright green
- Fix: The Hammer of the Gods wasn't destroying zombie corpses
- Fix: Drow to-hit bonuses and penalties actually in effect, various tweaks
- Allow tortles to wear rigid gloves/helmets
- The Rat King will sometimes spawn with a ring of teleport control
- Vampire types cannot be genocided until Vlad the Impaler is destroyed
- Don't prevent tortles from wearing certain cursed armor
- Refactor tortle enshelling
- Merge various skills
- Track defeating Saint Michael
- Fix: certain armor would disappear on save/reload under specific conditions
- Wizard of Yendor's tower overhaul
- Fix: enshelling tortle feedback
- Fix: throwing an attached ball & chain at a metalvore and it tries to catch
  and eat it
- Fix: objects embedded in wall in Croesus' throne room with one map variant
- Fix: article for describing barding
- Adjust some dragon-scaled secondary resistances/passive defenses
- A couple more dragon-scaled secondary changes
- Convert the named ring of invisibility found in Gollum's cave to an
  actual artifact
- Fix: inconsistencies with slowing attacks and dragon scale passive attacks
- Fix: blue-scaled armor prevents any form of slowing
- Fix: Slowed monsters remain slowed
- Water walking prevents sewage from slowing you
- Fix: rising from the grave as a zombie
- Wearing the One Ring warns of wraith-type monsters
- Some changes to beholders
- New branch: The Hidden Dungeon
- Fix: incorrect beam type when a monster zaps a wand at the player
- Fix: a few more cases where beam type needed to be updated
- Fix: illithid corpses can boost intelligence when eaten
- Fix: monsters ransacking players possessions on death from a distance
- Fix: perm_invent not updating when sokoban prize is acquired
- Fix: pet petrified by eating Medusa's corpse
- New dungeon feature: magic chests
- Magic chest placement
- Fix: warning: the comparison will always evaluate as ‘true’ for the
  address of ‘mons’ will never be NULL [-Waddress] (mhitu.c)
- Fix: warning: the comparison will always evaluate as ‘true’ for the
  pointer operand in ‘s + (sizetype)((long unsigned int)sidx * 128)’ must
  not be NULL [-Waddress] (botl.c)
- Change Convict quest artifact from an iron ball to a striped shirt
- Tweaks to the new Convict quest artifact
- Fix: close a couple loopholes where indestructible artifacts could be
  destroyed
- Spear traps can hit monster's steeds
- Fix: offhand artifact intrinsics
- Fix: close a loophole being able to produce tame non-tameable monsters 
- Fix: container trap outcomes for monsters using them was determined by
  the players luck
- Change Shadowblade forging recipe
- Goblin Town traps spawn in random spots
- New random Goblin Town map (top level)
- Make Kathryn the Enchantress visible if invisible as the Ice Queen
- Fix: undoing monsters' permablindness by accident
- Add archangels to the non-tameable monster list
- Fix: gaining certain resistances would not cure current effect they
  protect against / Snickersnee grants stun resistance when wielded
- Fix: tame monsters and riding
- Sword of Annihilation recipe changes
- Fix: add bracers to pair_of() define
- Fix: forges are always lit
- New forging artifact recipe - Dichotomy
- More 'merge various skills'
- Rangers gain broadsword skill
- Revert 'Allow helm of speed to be made of more materials'
- Wearing a helm of telepathy can focus psychic attacks, tweak to
  ESP object property
- Fix: another incorrect damage type value (mon_explodes)
- Fix: various 'monster hiding under objects' bugs
- Fix: mind flayer attacking without knowing your location
- Dichotomy tweaks
- Stop handing out blank spellbooks as a prayer/sacrifice gift
- Add #genocided command
- Fix: Drow shouldn't be able to handle artifacts that give off light
- Tweak to Drow handling lit artifacts
- Fix: armor-based artifacts could come with an object property
- Fix: spawn rate behavior
- Partial fix: monsters can loot bags on the ground that they can't carry
- Lockable and monster-lootable magic chests
- Tweaks and fixes to magic chests
- Autounlocking and magic chests
- Fix: prevent monsters from hovering over a container with something
  in it that they want but can't carry
- Magic key placement
- Fix: starting/ending alignment reporting in xlogfile
- Fix: don't prompt to autounlock a magic chest if you don't have
  a magic key
- Fix: The bow welds itself to the gnome lord's claws!
- Gold dragon-scaled armor imparts clairvoyance when worn
- Change placement of magic key in Gollum's Cave
- Some Wizard of Yendor behavior tweaks
- Fix: farlooking tools
- Fix: don't reference armor after they're broken
- Fix: AD_FUSE not accounted for in pokedex monster lookup
- Fix: occasional program in disorder when autounlocking magic chests
- Fix: report objects on magic chest square when nearlooking
- Fix: let all and only giants wield two-handed weapons in one hand
- Fix: hated material sear message feedback not appearing if
  attacking unarmed
- Include more monsters as 'flaming'
- Added some new rumors (true and false)
- Wielding Dichotomy will protect worn armor from being burnt
- Baby dragon tweaks
- Fix: shambling horror possible resistances, add more info to pokedex
- Fix: pets and players poly'd and attacking disintegrators
- Fix: follow-up to last commit (kicking and martial arts)
- Fix: 'null obj after quiver merge' panic
- Fix: stun resistance from source in ctrl-x info.
- Fix: a couple bugs with wizmode command #wizspell
- Fix: forging Drow objects
- Improve #wizgenesis monster race parsing
- Fix: zap/explosion types, overhaul
- Allow being unpunished (ball & chain) if poly'd into monster with no limbs
- Prevent waking up the Wizard of Yendor before actual encounter
- Don't allow players to name objects they don't know the appearance of
- Fix: undead don't grudge each other
- Fix: monsters will pick up partial stacks
- Fix: give message when walking onto a magic chest square
- Fix: the Hidden Dungeon and magic mapping
- Adjust odds of the Wizard of Yendor spawning with magic resistance
- Fix: more zap/explosion type corrections
- Fix: grammar when killed by a spear trap
- Fix: don't allow magic keys in bones
- Castle chest with wand of wishing is always locked
- Fix: offhand weapons and warning (bane)
- Fix: magic chest placement and quest leaders
- Fix: drow monsters to-hit in light penalty was affecting all monsters
- Tweaks to monster behavior when wielding/wearing a bane artifact
- Fix: silly typo
- Fix: heap-use-after-free when magic beam trap is destroyed
- Fix: skipping attacks if hero moved away
- Fix: correction to last commit (skipping attacks)
- Fix: crash when trying to loot a magic chest with nothing in inventory
- Fix: crashes from splitobj() caused by can_carry()
- Refactor rndmonst()
- Show forge recipes in object lookup (pokedex)
- Forging sling bullets
- Fix: Display beatitude of (un)holy water if potion type is unknown
- Fix: Grammar in object lookup
- Fix: Messages and IDing polymorph potions when dipping while blind
- Fix: Don't tell player if unseen items resist shock damage
- makemon inventory generation tweaks
- Fix: Messaging when blocking with !dknown shield, instakilling with
  artifacts
- Fix: Don't push hard helmets/gloves off of newly poly'd tortles 
- Fix: defensive knockback didn't stop monster attacks
- Fix: Allow non-chaotics to wield the Sword of Kas again
- Fix: searmsg from unseen monster triggering a trap
- Fix: Don't "discover" warned-of monsters when blind
- Fix: Celestial dragon scale polymorph
- Fix: Summoned spheres stay indefinitely if abandoned
- Fix: Player blamed for monster spells killing monsters
- Fix: Display suffocation attacks differently from drowning
- Fix: Weapons with the excellence property could grant a permanent luck bonus
- Fix: mention locked/empty status of magic chest when walking on it
- Sysconf tweaks
- More sysconf tweaks
- Wand of Orcus changes
- Some further tweaks to the Wand of Orcus
- Clean up some issues from last commit (Wand of Orcus)
- Fix: Wand of Orcus dual-wielding and invoke function
- Fix: various artifact death messages
- Fix: remove extra feedback when Death attacks another monster
- Fix: multiple messages for gnomes noticing eggs
- Allow cutting a known spider web by force-fighting it
- Check obj_resists when destroying items via erosion
- Fix: player could drop a cursed loadstone by wielding it
- Fix: zombie corpses changing when failing to revive
- Fix: prevent magic lamps from being extinguished when entering a pool
- Fix: characters with ultravision shouldn't be visible in dark
- Fix: boomerang sear damage
- Don't allow supermaterials to be erodeproofed
- Fix: gemstone is a supermaterial
- Gloves and shields with certain object properties, if used offensively,
  will cause extra damage based on the object property
- Fix: lawful priests weren't receiving any kind of crowning gift
- Fix: "You bash the orc shaman with your shield of mobility?"
- Fix: monsters using a scroll of fire against the player and noticing they
  have fire resistance
- Fix: demon lords/princes will follow you if you try to levelport past
  them on their home level
- Hallucination potions can confuse monsters
- Fix: avoid impossible when restoring stale steed ptr
- Fix: to-hit formula for player experience levels 21 to 29 inclusive
- Fix: tweaks to demon lords/princes following the player via levelport
- Fix: magic lamps and snuff_lit()
- Pacify Kathryn the Enchantress with a candy bar
- Fix: forging fooproof supermaterials
- Healing/extra healing spells effectiveness is determined by skill in
  casting healing spells
- Cure sickness spell damage vs zombies scales based on skill in casting
  healing spells
- Fix: distinguish case for protecting classes from genocide
- Fix: Shadowblade drain life attack not triggering
- Fix: reviving buried corpse
- Fix: Allow rogues and convicts to train thrown weapons when unarmed
- Fix: Gauntlets of Purity damage bonus applying in too many cases
- Fix: impossible() when killing multiple vault guards in succession
- Make it easier to remove a ball & chain with a forge/hammer if the
  chain is rusted
- Fix: wielding Dichotomy protecting objects in open inventory from
  burning while walking on lava
- Fix: Minor issues with damage of various special attacks
- Adjust potion of paralysis effect vs monsters
- Fix: misspelling (Hidden Dungeon branch)
- Fix: More theft attack damage tweaks
- Fix: Allow repair of deteriorated supermaterials
- Fix: killing quest leader won't flag the quest as complete if they were
  killed while polymorphed
- Fix: Checking floor for tippables can stone you
- Allow tipping into containers on the floor
- Fix: players with intrinsic water walking and paranoid: swim enabled
  still being asked if they really want to enter water/lava
- Fix: tweak to last commit (check for steeds if attempting to walk on lava)
- Fix: don't allow tipping a floor container into itself
- Zombie brain eating won't cause known spells/skill training loss
- Update copyright year to 2024
- Switch status from beta to post release


### Version 0.8.4

- Latest merges from 'vanilla' NetHack 3.6.7 official release (as of May 11th, 2024)
- Initial preparation for new version (0.8.4)
- Fix: prevent the Amulet of Yendor from being teleported whilst in the
  Sanctum
- Fix: prevent monsters that are carrying the Amulet of Yendor from
  being teleported while in the Sanctum
- Fix: don't allow tipping a floor container into itself II
- Fix: confused enchanting dragon scales
- Fix: update gold & green dragon sickness resistance
- Fix: immolation when especially flammable monsters are hit with fiery weapons
- Fix: messaging when hit by fiery weapons underwater
- Fix: messaging when hit by Dichotomy
- Fix: early returns from artifact_hit
- Fix: looting lit armor from pets makes it stop shining
- Fix: monsters wouldn't wield Sunsword/HotG if you were drow
- Fix: only cause light damage when sources begin burning near you
- Fix: convicts don't feel guilty about stealing
- Fix: push boots off tortles when reverting from polyform
- Require manual suppression of attacks vs. dangerous enemies
- Fix: fixedness knowledge preventing merging of supermaterials
- Fix: a couple cases of bracers preventing wielding bimanual weapons
- Restore auto-suppression of some dangerous attacks
- Fix: crash when examining discoveries with '`' in certain cases
- Fix: allow spear traps to be disarmed
- Fix: heap-use-after-free (bones)
- Fix: follow up to crash when examining discoveries with '`' in certain cases
- Adjust auto-recovery feedback
- Fix: Amulets could burn your skin but then allow you to regenerate
- Fix: Peaceful demon lords won't follow you across levels
- Add regular bracers as a forge recipe
- Ammo created by the Ranger quest artifact is fully identified
- Hobbit rangers start with a crossbow instead of a bow, quest artifact is
  the Crossbow of Carl
- Fix: object property from wearing a t-shirt persisted if the t-shirt was
  destoryed or fell off when polymorphing
- Fix: incorrect feedback when attempting to steal from a monster and
  the objects being stolen have the oilskin object property 
- Fix: if the player blows up their bag of holding, make known the bag,
  and the object placed inside of it that made the bag blow up
- Fix: Tiamat couldn't fly
- Fix: disarming spear trap chances luck check was inverted
- Carrying the Master Key of Thievery in open inventory improves thievery
  chances
- Fix: text in the Convict quest still referenced the Iron Ball of Liberation
- Fix: same-race monsters could still spawn peaceful for Convicts and Infidels
- Fix: unique monsters are not bound by maximum hit point value in grow_up()
- Fix: Infidels couldn't drop cursed loadstones
- Fix: misbehavior by #adjust
- Fix: artifact rings have a fixed material
- Revert: artifact rings have a fixed material
- Fix: artifact rings have a fixed material, take two
- Minor monster color changes
- Add object menu glyphs to inventory in curses mode
- Fix: names of new ring types when type is known
- Proper encyclopedia entries/lookup for the artifact rings
- Fix: Convicts felt guilty when encountering the guard from a gold vault
- Hobbit pickpockets are always hostile
- Fix: monster casting summon minion against another monster
- Fix: spec_applies didn't consider if target was underwater
- Fix: monster priests shouldn't wield edged weapons either
- Fix: reactivate offhand weapon intrinsics when catching a returning weapon
- Fix: samurai splint mails were not always rustproof
- Fix: prevent Nazgul from attacking undead hobbit types
- Fix: Monks incurring to-hit penalty when wearing bracers (martial arts)
- Fix: Kathryn the Enchantress and 'summon insects' spell
- Revert Druid role
- Re-add regular bracers, minor bug fixes from initial Druid commit
- Re-add encyclopedia and guidebook entries (bracers, existing roles)
- 'Deslog' the first wizard's tower level
- Fix: Lava gremlin exploding a forge
- Fix: ID_MANIFEST target (windows build)
- New conduct: never forged an artifact
- Underline peaceful monsters via config (tty and curses)
- New trap: ice
- Fix: cmap ordering in tilemap.c (addition of ice trap)
- Prevent levelporting throughout the Wizard's tower until the Wizard of
  Yendor has been defeated
- Fix: various monsters showing as having claws when they don't
- Fix: update to last commit (claws)
- Hippocrates (Healer quest leader) additional gear
- Fix: certain monster steeds should only be ridden by certain monster
  riders
- Lolth will be peaceful towards Drow players under the right
  circumstances
- Penalty for a chaotic Drow killing Lolth
- Attempt to make Wizards less reliant on weapons, focus more
  on spellcasting (round one)
- Attempt to make Wizards less reliant on weapons, focus more
  on spellcasting (round two)
- Changes to to-hit formula for force bolt spell
- Fix: restore ability spell could not be used on ridden steed
- Set max skill for Wizards and quarterstaff back to expert
- Allow lightning spell to be cast as an area of effect  ball of
  lightning at skilled or greater in attack spells
- Sling bullets can have object properties
- New spell - critical healing
- New player race/role combo - giant Healer
- Allow Illithids with intrinsic flying to enter into and stay
  in pools/moats
- New spells - burning hands, shocking grasp
- Fix: repair armor spell will not randomly target the Hand of Vecna
- Suppress livelogging whilst in explore mode
- New conduct: never acquired magic resistance
- New conduct: never acquired reflection
- New race: Draugr (initial commit)
- Revival routine for Draugr race
- Draugr and critical hits from Sunsword/The Hammer of the Gods
- Draugr really are undead
- Clean up role-specific bits for Draugr
- Draugr cannot gain intrinsic telepathy
- Draugr as Infidel and crowning
- Draugr and various interactions with NPC's
- Draugr can regenerate hit points while in the Valley of the Dead
- More role-specific bits for Draugr
- Ensure unique undead monsters are appropriately hostile towards Draugr
- Tweak feedback between shopkeepers and Draugr
- Shop prices for Draugr, other price tweaks
- Allow player monsters to be Draugr, part one
- Draugr really are zombies, which are undead
- Fix: ^X message for being warned of undead
- Merge & use racial_zombie when applicable
- Fix: makedefs.c merge from last commit
- Refactor diseasemu() to allow use of racial_zombie()
- Refactor resists_sick to allow checking racial data
- Draugr infidels don't cast spells
- Allow player monsters to be Draugr, part two
- Draugr can revive
- Fix: if burning hands or shocking grasp is active, don't activate
  whilst using thievery skill
- Fix: Draugr trying to read a spellbook and the spellbook is
  destroyed while reading it
- Fix: force steed dismount if steed becomes hostile from the player
  wielding or wearing a bane artifact vs the steed
- Switch from impossible to raw_print for 'Cannot open record file!'
  errors
- Switch status from work in progress to beta
- Make sure Draugr monk prayer gift isn't a spellbook
- Fix: summoned spheres could zombify victims
- Fix: if statement syntax typo
- Update xkill_flags comment
- Fix: zombie_maker and hero or monster race
- Fix: curses initial race selection
- Fix: Draugr Monks feeling guilty eating meat, other feedback fixes
- Draugr can't revive from being stoned
- Fix: being breathless not taken into account in several scenarios
- Fix: some bugs with various life saving conditions
- Draugr can't revive from being disintegrated
- Fix: artifact rings and wrong name in .des file
- Fix: hit point bar would still show full hit points if life saving
  or revival failed 
- Same food eating rules for monster draugr vs player draugr
- Fix: 'Achievements completed' header in dumplog
- Fix: no magic resistance/reflection conducts were being broken
  by monsters
- Fix: no magic resistance/reflection conducts were being broken
  by monsters, round two
- Force paranoid quit to be yes [no] instead of y/n
- Update paranoid_confirmation in Guidebook
- Fix: undersized buffer for dumplog conducts
- Fix: unaligned priests were hostile to Draugr
- Draugr gained intrinsic changes (fire res, ESP)
- Include a few more races for boulder breaking chances
- Small tweak to Draugr eating feedback
- Prevent phasing in more areas of the Sanctum
- Fix: unaligned priests were hostile to Draugr II
- Abused pets can turn hostile when untamed
- Limit number of pets you can have
- Fix: unleash pets affected by wielding/wearing a bane artifact
- Diluted potions of full healing heal less HP
- Fix: Potion of full healing messaging
- Fix: don't undilute potions in set_material()
- Fix: tweak message about recuperation when entering the
  Valley of the Dead as Draugr
- Fix: Draugr should not be affected by hellfire
- Object/magic detection work through all non-crystal containers
- Draugr are properly vulnerable to fire
- Fix: Infidel Draugr kept getting fire resistance back after
  save/reload
- Fix: Enchanting armor of excellence didn't adjust charisma
- Fix: tty character selection filtering menu
- Fix: drawbridges crush both rider and steed
- Fix: stethoscopes shouldn't open chests
- Fix: blunt weptools had +4 TH bonus
- Count corpses left by zombies for draugr same-race sacrifice
- Minor formatting tweaks to last PR
- Fix: peaceful underline glyph not clearing for shopkeepers
  when rile_shk() routine was called
- Fix: bracers should not be a suitable target when applying
  a dwarvish bearded axe towards the player or a monster
- Fix: crash in relation to tainted (drow poison) objects
- Fix: elemental damage from the Hand of Vecna and other sources
  was suppressing almost all damage if target resisted
- Fix: You feel feel completely chilled
- Fix: players were keeping intrinsics gained from subsequent
  polyform
- Fix: unwielding weapons of warning didn't clear warning symbols
- Fix: crowned Infidel Draugr would become demons after save/reload
- Fix: closet trap engravings
- Fix: grammar/death reason if the player is incinerated by Sunsword
  or the Hammer of the Gods
- Fix: 'The magical energy is released!' with non-directional
  psionic wave
- Fix: crowned centaur infidels should be able to train riding
- Tweak death reason when drinking from a forge
- Add dragonhide material to is_flimsy() 
- Fix: peaceful/tame monsters poly'd into a cross-aligned dragon
  (player is a knight) would stay tame
- Fix: noncorporeal monsters were affected by knockback
- Fix: monster could go berserk if temporarily sleeping or paralyzed
- Fix: consume_tin() bypassed is_edible()
- Forcefighting a door or any type of tree while burning hands spell
  is active can burn them down
- Fix: using burning hands to burn down a tree or a door uses a charge
- Burning hands/shocking grasp spell status can be viewed in inventory
- Wand of probing or a stethoscope used on oneself will show reflection
  status (spell only)
- Tweak power cost for wizards casting force bolt spell
- Adjust feedback when hitting a water-based monster or one that is
  underwater with burning hands spell
- Fix: passive AD_MAGM damage when attacking the Oracle (player or
  another monster)
- Fix: make autoall_menu apply to the 'D' drop menu as well as the loot menu
- Fix: Your dagger also slips from your hand (but a knife was wielded)
- Fix: Giantslayer strength bonus not taken into account whilst twoweaponing 
- Drow shopkeepers/priests rooms are dark
- Fix: force bolt spell and being engulfed
- Follow up to last commit (force bolt and engulfed)
- Fix: cursed Ogresmasher hitting oneself and knockback
- Fix: potential crash with dark elven objects falling down to the level below
- Fix: various player monster attacks not damaging other monsters
- Additional feedback when damaging monsters vulnerable to elemental damage
- Levels of erosion on an object can affect its price
- Show in inventory if the Bell of Opening can't be used (quest not complete)
- Various monsters that can spawn in groups will sometimes be lead by a stronger
  leader
- Some code formatting
- Adamantine has a slightly sharper edge
- Fix: different apostrophe in the in-game encyclopedia
- Fix: killing the apprentices found in the Wizard's Tower would anger
  your deity
- Blessed restore ability only restores a few levels
- Change some monster clerical spells to be cast at range
- Fix: corrections to last commit (monster clerical spells at range)
- Wand of probing or a stethoscope used on oneself will show elemental
  vulnerabilities
- Tweak certain monsters and what they grow up into
- Fix: multiple issues with mcastu.c
- Fix: more fixes in mcastu.c
- Allow monster spellcasters to cast their ranged spells at other
  monsters from a distance
- Pets can help the player other than fighting under certain
  cirmcumstances
- Fix: minor issues with pets acting as a helper
- Fix: handling mind flayer larva turning their hosts into mind flayers
  when mind flayers have been genocided
- Fix: mon vs. mon casting could place summons on dangerous terrain
- Fix: pets moving onto newly summoned monsters
- Fix: crash while attacking with drow weaponry
- Remove trailing whitespaces from various files
- Tweaks to altar sacrifice gifts
- Fix: various 'hiding under' bugs
- Fix: antimatter vortex disentegration engulf attack
- Fix: 'already have amulet?' impossible as Infidel
- Fix: 'hiding monster stuck to you' impossible
- Fix: 'form' vs 'hide'
- Fix: random amulet materials
- Adjust feedback when spellcasting pets cast spells at the player
- More adjustments to monster spellcasting
- Tweak to excess pets becoming peaceful or hostile
- Revert 'Putting Magicbane into a bag of holding has a chance of
  causing the bag to explode'
- Fix: filename in evilhack-cpp.yml, change checkout from v3 to v4
- Switch status from beta to released


### Version 0.9.0

- Latest merges from 'vanilla' NetHack 3.6.7 official release (as of November 6th, 2024)
- Initial preparation for new version (0.9.0)
- Update README.md
- New player role: Druid (initial commit)
- Minor tweaks to Druid role
- Set default Druid pet name if one isn't set in the player config
- Fix: typo during druid cherry-pick
- Fix: crashes when naming wielded objects in some circumstances
- Fix: prevent support monsters from spawning with groups of undead
- Fix: steeds that are loyal or greater (tameness) could get an
  indefinite number of attacks per round vs multiple monsters
- Fix: player took blame for monster using Grimtooth
- Fix: monster would always take blame for explosions affecting
  various objects
- Fix: feedback when having to drop a worn shield vs worn bracers
- Fix: Tortle archeologists could never throw Xiuhcoatl
- Fix: Possible crash when saving bones after second death
- Fix: digesting rings of fire/cold/shock/poison resistance would not
  confer any resistance
- Creation of a dedicated test monster
- Fix: 'zaps you with a !'
- Fix: Draugr players affected by various gases and vapors
- Fix: dmonsfree error when killing a monster via armor blocking
  causing material sear damage
- Fix: shopkeepers offering a negative value for items sold
- Fix: some places where random traps could spawn on top of magic
  chests
- Fix: prevent lawful/neutral players from receiving an altar sacrifice
  weapon gift with the venom object property
- Monsters summoned via the monster spell CLC_INSECTS will rarely leave
  a corpse
- New material type - Bronze
- Fix: assign bronze its proper color
- Fix: change HI_COPPER to be CLR_ORANGE
- Remove old, unused status code (curses)
- Slight adjustment for extra slash/piece damage for steel and bronze
  weaponry
- Rename METAL to STEEL
- Fix: objects.txt (brass -> bronze)
- Initial Druid quest text
- Finish up Druid quest text, fix formatting
- All demons/devils are now unaligned (evil)
- New terrain type - grass
- Fix: error C4703: potentially uninitialized local pointer variable
  'what' used (windows compile)
- Fix: error: for tile 89 (numbered 89) of other.txt, found 'swallow
  top left' while expecting 'cmap 89'
- Fix: issues with portal placement on nymph level
- New terrain type - sand, add grass and sand to various levels
- Fix: shield bash damage with large shields
- Filler levels for Druid quest
- Fix: monster stealing worn gold objects from other monsters (AD_SGLD)
  not properly handled
- New object - helm of caution, change 'shatterproof' to 'tempered'
- Change the recipe for the forged artifact Ithilmar
- Allow regular/spiked barding to be made out of leather or dragonhide
- Allow regular/spiked barding to have certain object properties
- Rename nethack.6 and nethack.txt documents to "evilhack"
- Change environment variable for options to EVILHACKOPTIONS
- The final portal exiting Purgatory will not function until Saint
  Michael is killed
- Fix: being engulfed using lit/unlit state of tile the player was
  standing on when engulfed
- Fix: thrown/projectile damage monster vs monster
- Fix: iterating invent
- Update hints files with recent official NetHack updates
- Do not generate dumplog if game was startscummed
- Fix: 'Your shield crumbles away!' when actually wearing bracers
- Fix: prevent sling bullets from forming inside solid rock
- Fix: dmonsfree when an artifact's instakill ability triggers
  after the target was already killed from the initial hit
- Fix: Monks and gaining stone resistance while being stoned
- Fix: town guards not becoming angry when fountain is used up
- Getting a wish from a water demon (fountain) immediately dries up
  the fountain
- Fix: follow-up to 'town guards not becoming angry when fountain
  is used up'
- Additional info for monster lookup (pokedex)
- Fix: monster alignment for monster lookup (pokedex)
- Grant poison resistance to Kathryn the Ice Queen
- New monsters: Ent, Elder Ent, Petrified Ent
- Fix: Drow monsters aren't supposed to be able to spin webs
- Fix: a couple bugs from commit 61f44f6 (bracers)
- Fix: Infidels would not receive second clue from the sanctum high
  priest if a pet killed the quest leader
- New monster: Satyr
- Tweaks to the initial satyr commit
- Fix: uncontrolled levelporting below the Valley of Dead while
  Cerberus was still alive
- New monster: Skeleton Warrior, repurpose existing Skeleton
- Rearrange a couple monster symbols
- Skeletons don't have a 'thick hide' exactly
- New monster: Assassin Vine
- Add encyclopedia entry for assassin vine
- New monsters: Twig Blight, Tree Blight, Needle Blight
- New level and new special room (forest)
- Tweak assassin vine generation on the new forest level
- New monster: Creeping Mound
- New monsters: Grizzly Bear, Cave Bear
- New monsters: Deer, Stag
- Fix: Stags aren't meant to appear in groups
- New monsters: Wolverine, Dire Wolverine
- Setup defines for woodland creatures and beings
- Add some of the newer monsters to has_claws()
- Fix: winter wolf/hell hound alignment
- Fix: depreciation warning using 'cp -n' during compile
- Wolverines are not affected by onscary()
- Attempt to force github workflow to use gcc13 for linux
  build
- Attempt #2 to force github workflow to use gcc13 for
  linux build
- Attempt #3 to force github workflow to use gcc13 for
  linux build
- Redo stats for grizzly bear/cave bear
- Complete the Druid quest levels
- Fix: monsters can wield two-handed weapons while wearing bracers
- Satyrs enjoy booze
- Code formatting from last commit
- Include satyr, chance of random nymphs on some of the Druid
  quest levels
- Fix: memory leak from magic fountain effect
- Fix: forges and objects coated in various substances
- New spells: barkskin and stoneskin
- Fix: support casters healing same race undead
- New spell: entangle
- Account for new bear monsters in various scenarios
- More spell skills for Druids, other roles gain evocation spell
  skill
- Fix: switch 'its' for his/her in screamm()
- Allow giants to be Druids
- New spell: create grass
- New spell: change metal to wood
- New object: staff of evocation
- Druids and bonuses/penalties vs certain materials
- New function: nexttotree()
- Update copyright year to 2025
- Change 'Be seeing you' message on save
- Fix: explosion causing a crash (mentangled)
- New spell: create trees
- New spells: summon animal, summon elemental
- Bracers can be made of wood, Bracers of the First Circle
  will not interfere with spellcasting
- Fix: wearing bracers would prevent Samurai from using
  their weapon-shattering blow
- Change #wizspell to behave like #wizcast from NetHack 3.7
- Fix: crash when casting 'change metal to wood' with
  nothing wielded
- Implement Druid's shapechanging ability
- Alert Druid player at the start of a new game about
  #shapechange
- Fix: only satyr's are supposed to play the pan flute
- Druids, elves are penalized for killing trees
- Allow Druids that have shapechanged to revert back to
  normal form at will
- Druids that #shapechange retain worn armor, other changes
- Additional conditions that can affect Druid #shapechange
- Druids can sense when they are about to revert back to
  their original form when polymorphed
- Shields won't merge into a Druids #shapechange form
- Rework Druid shapechange form defines
- Druids become immune to nymphs seduction attacks at
  experience level 14
- Unintelligent monsters could ignore a Druid that has
  changed shape from a distance
- Add a fourth tier of monsters a Druid can shapechange into
- Present worn armor in inventory as (merged to your form)
  if activating #shapechange as a Druid
- Adjust grass distribution on some of the Druid quest levels
- Another case where a Druid or Elf is penalized for killing
  a tree
- Redistribute spellbook spawn probabilities
- Allow giant hawks to lay eggs, oviparous changes to ants
- Caveperson staying illiterate hit point boost revamp
- Fix: check alive/dead status of demon boss when trying to
  teleport
- Fix: It douses it in a torrent of acid!
- Fix: livelogging when receiving a spellbook from your deity
- Add handling for Druid role and regular artifact gifts
- Fix: unpaid_cost impossible under certain circumstances
- Fix: sewage and shallow water tiles were lit in makeriver()
- Some Druid tweaks, enhancements while changing form
- New spell: power word kill, shift 'finger of death' from
  attack spell to evocation spell
- Fix: logic when invoking the Eye/Hand of Vecna, make its
  behavior be more inline with how death magic affects monsters
- Peaceful and tame monsters not killed by the spell
  'Power Word Kill' that take damage will become hostile
- New forged artifact: Harbinger
- Fix: kicking an iron safe could break fragile objects inside
- Fix: oversight in rejectcasting() function
- Altar sacrifice and receiving artifacts changes
- Tweaks to altar sacrifice and receiving artifacts changes
- Reduce of number of altars on the mines' end 'orc temple' level
- Altars won't start to appear until dungeon level 3
- Fix: number of guaranteed artifacts from wishing
- Make Druid's first sacrifice gift Werebane
- Harbinger can eat through iron bars like Dirge
- Change Werebane from a saber to a scimitar
- Do not anger peacefuls if zapped with a wand of speed monster
- Highlight hidden stairs
- Fix: windows binary compile error with highlighting hidden
  stairs
- Adjust odds of receiving an artifact via altar sacrifice based
  on experience level
- Changes to forging, new object: blacksmith hammer
- Various player conditions can prevent successful forging
- Fix: allow non-satyrs to use non--pan flutes
- Forged armor/weapon quality
- Fix: crash when not wielding anything
- Rework forged armor/weapon code
- Fix: poison/quality of known artifacts not shown
- Prevent wishing for quality artifacts
- Change enum labels for forged quality code
- Various quality objects can be purposely forged
- Prevent wishing for the Blacksmith hammer
- Fix: logic with purposely forging quality objects
- Fix: more forged quality logic
- Restructure how blacksmith hammer is made unwishable
- Allow armor and weapons to randomly spawn with various
  levels of quality
- Object build quality affects its price
- Object build quality vs breakage
- Object quality random spawn/wishing rules
- Knights and Samurai can tell an objects quality without
  formally identifying it
- Spread out wand of wishing placement
- Add an encyclopedia entry for the artifact Harbinger
- Fix: make gaze disintegration attack consistient with
  other sources of disintegration
- Fix: crash bug concerning inferior objects breaking
- Allow barding to have quality properties
- Change 'while attempting to cheat Death' line with something
  more appropriate for a Draugr player dying
- Draugr race get undead monsters as pets
- Beginning Purgatory level tweaks
- Revamp player monster spawning gear
- Remove hard-coded player monster spawns from the Knight quest
  starting level
- Player Draugr or if poly'd into a mindless monster are immune
  to a mind flayer's psychic blast
- Nurses will not heal the player if they are Draugr or poly'd
  into an undead monster
- Draugr, undead monsters are immune to lycanthropy
- Draugr get a slight AC bonus when wearing armor pieces made
  of bone
- Monsters will use a wand of undead turning directly against
  the player if they are Draugr or poly'd into an undead monster
- Tweak to last commit (wand of undead turning vs player)
- Holy water, wands of death, and Draugr
- Revisiting what and how Draugr can eat
- Alternate ending to the Ice Queen branch if the player is a Draugr
- Fix: racial shopkeepers receive appropriate racial gear
- Allow vampires to use weapons
- Reformat code block handling monster spawn rates
- Having really low intelligence prevents you from reading most things
- Revamp how luck affects to-hit
- Updates to the Hidden Dungeon
- Allow a bit more variety of skeleton monsters when a morgue is
  created
- Tal'Gath cannot be sensed via telepathy
- Increase PREFIX value in objnam.c
- Blessed scroll of taming increases tameness of pets
- The spell 'stone to flesh' can affect armor and weapons
- Some tweaks and fixes to player monster gear
- Fix: account for being polymorphed into an undead monster when
  using wand of death/finger of death spell on yourself 
- Fix: impossible "hiding under nothing?"
- Fix: 'Program in disorder: rnd(0) attempted' when using #wizspell
  while having the Amulet of Yendor in inventory
- Fix: fuzzer making a wish, returns NULL, causing a crash
- Fix: give message when gaining strength via uncursed gain ability
- Fix: panic "relmon: mon not in list"
- Fix: gulpmm "placing [mon] over itself"
- Tweaks to player monster gear and regular objects via altar sacrifice
- Druids #wildshape cooldown between uses is tied to alignment abuse
- Allow Druids that are in one of their wildshape forms to manipulate
  objects and their surroundings
- The spell 'create trees' can sometimes revitalize dead trees
- Add Druids to check_caitiff() routine
- Added a few more monsters to Druids wildshape ability
- Fix: impossible "mon hiding under nonexistent obj (%s)"
  (burned grass)
- Switch status from work in progress to beta (0.9.0)
- Show potion alchemy combinations in the pokedex
- Dipping a unicorn horn into a potion of drow poison will turn it into
  a potion of fruit juice
- Fix: escaping out of #wildshape menu (not selecting anything) would
  still trigger the wildshape use timer
- Tweak the number of charges the Blacksmith hammer starts with and can
  be charged up to
- Adjust sound flag for various monsters
- Fix: grammar when deflecting a thrown projectile whle wearing a
  shield or a pair of bracers
- Evocation spell 'entangle' will not work against unsolid, whirly,
  or phasing monsters
- Druids can chat with woodland creatures, possibly making them
  peaceful or even tame
- Fix: allow burning hands and shocking grasp spells to function
  while polymorphed; monster attack/body part tweaks
- Fix: charge to #loot pet inside shop standing on the free spot
- Fix: prevent armor that is 'merged to your body' from being
  stolen
- Fix: rehumanizing a Druid while in wildshape via prayer
- Fix: 'You can't hold is strongly enough' while using wildshape
- Fix: give proper feedback on why certain objects can't be used
  while in wildshape
- Remove race flag from Dramborleg
- Fix: rotten food showing as 'burnt'
- Fix: limit how often a female Druid can lay eggs while using
  wildshape
- Fix: alchemy recipe was dependent on order of which potion was
  dipped into another potion
- Fix: charge to #loot pet inside shop standing on free spot II
- Change how Druid wildshape hitpoints are calculated
- Fix: crash caused by seduce attack
- Druids can receive the spellbook 'finger of death' as a crowning
  gift
- Druids second spellbook set to be some type of evocation spell
- Display unseen grass & sand normally
- Fix: change how Druid wildshape hitpoints are calculated, take two
- Fix: light sewage and shallow pools on lit mines levels
- Fix: zoo monsters spawning in hallways
- Fix: infinite loop when praying in handsless wildform
- Changes to Druids wildshape use timer
- Fix: Druids not starting with second armor piece
- Fix: various player monster gear for certain races
- Add remaining skeletal monsters to monmaterial()
- Fix: baby gold dragon duplicate in emits_light()
- Fix: domestic undead pets wary of Draugr
- Adjust random spawn odds for evocation spellbooks
- Replace awful "You feel cold" message for freezing a door you can't
  see
- Fix: only allow dead tree 'corpses' to form on room/grass/sand tiles
- Fix: digging out traps on no-dig levels
- Place an altar in the quest levels where none were present

