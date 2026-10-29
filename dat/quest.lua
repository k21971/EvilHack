-- NetHack quest.lua	$NHDT-Date: 1781994887 2026/06/20 22:34:47 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.11 $
-- Copyright (c) 2021 by Pasi Kallinen
-- NetHack may be freely redistributed.  See license for details.
-- TODO:
--  - output = "verbalize"
--  - export the quest string replacements to lua, instead of %H etc
--  - allow checking if hero is carrying item (see comments for %Cp Arc 00042)
--  - fold quest_portal, quest_portal_again, quest_portal_demand into one
--  - write tests to check questtext validity?
--  - qt_pager hack(?): if (qt_msg->delivery == 'p' && strcmp(windowprocs.name, "X11"))




-- text = "something"
-- Text is shown to the user.

-- synopsis = "something"
-- Synopsis is inserted into the message history.
--
-- output = "pline" | "menu" | "text"
-- The output can be manually set by using output = "menu"
-- Valid values for output are "pline", "text", and "menu, defaulting to
-- pline, unless the text contains newlines, or is too long to fit a message buffer,
-- then will be shown as a text window instead.



questtext = {
   -- If a role doesn't have a specific message, try a fallback
   msg_fallbacks = {
      goal_alt = "goal_next"
   },
   common = {
      TEST_PATTERN = {
         output = "text",
         text = [[ %p:	return(plname);
 %c:	return(pl_character);
 %r:	return((char *)rank_of(u.ulevel));
 %R:	return((char *)rank_of(MIN_QUEST_LEVEL));
 %s:	return((flags.female) ? "sister" : "brother" );
 %S:	return((flags.female) ? "daughter" : "son" );
 %T:	return((flags.female) ? "madam" : "sir" );
 %l:	return((char *)ldrname());
 %i:	return(intermed());
 %Q:	return(Role_if(PM_INFIDEL) ? "possess" : "seek");
 %o:	return(artiname());
 %O:	return(shortened(artiname()));
 %n:	return((char *)neminame());
 %g:	return((char *)guardname());
 %G:	return((char *)align_gtitle(u.ualignbase[1]));
 %H:	return((char *)homebase());
 %a:	return(Alignnam(u.ualignbase[1]));
 %A:	return(Alignnam(u.ualign.type));
 %d:	return((char *)align_gname(u.ualignbase[1]));
 %D:	return((char *)align_gname(A_LAWFUL));
 %C:	return("chaotic");
 %N:	return("neutral");
 %L:	return("lawful");
 %x:	return((Blind) ? "sense" : "see");
 %Z:	return("The Dungeons of Doom");
 %%:	return(percent_sign);
 a suffix:	return an(root);
 A suffix:	return An(root);
 C suffix:	return capitalized(root);
 h suffix:	return pronoun(he_or_she, mon_of(root)); /* for %l,%n,%d,%o */
 H suffix:	return capitalized(pronoun(he_or_she, mon_of(root)));
 i suffix:	return pronoun(him_or_her, mon_of(root));
 I suffix:	return capitalized(pronoun(him_or_her, mon_of(root)));
 j suffix:	return pronoun(his_or_her, mon_of(root));
 J suffix:	return capitalized(pronoun(his_or_her, mon_of(root)));
 p suffix:	return makeplural(root);
 P suffix:	return makeplural(capitalized(root));
 s suffix:	return s_suffix(root);
 S suffix:	return s_suffix(capitalized(root));
 t suffix:	return strip_the_prefix(root);]],
      },
      angel_cuss = {
         "\"Repent, and thou shalt be saved!\"",
         "\"Thou shalt pay for thine insolence!\"",
         "\"Very soon, my child, thou shalt meet thy maker.\"",
         "\"The great %D has sent me to make you pay for your sins!\"",
         "\"The wrath of %D is now upon you!\"",
         "\"Thy life belongs to %D now!\"",
         "\"Dost thou wish to receive thy final blessing?\"",
         "\"Thou art but a godless void.\"",
         "\"Thou art not worthy to %Q the Amulet.\"",
         "\"Judgement hath been passed upon thee, %p.\"",
         "\"Thy reckoning is at hand, %p.\"",
         "\"Thou shalt be brought before %D for thy crimes!\"",
         "\"With %D as my witness, I shall strike thee down!\"",
         "\"Good day to thee, %T!  I said good day!\"",
         "\"No one expects the Spanish Inquisition!\"",
      },
      banished = {
         synopsis = "[You are banished from %H for betraying your allegiance to %d.]",
         output = "text",
         text = [["You have betrayed all those who hold allegiance to %d, as you once did.
My allegiance to %d holds fast and I cannot condone or accept what you
have done.

Leave this place.  For as long as I live, you shall never set foot at %H again.
That which you seek is now lost forever, for without the bell of Opening,
you will never be able to enter the place where he who has the Amulet
resides.

Go now!  You are banished from this place.]],
      },
      cerberus_defeated = {
         synopsis = "[Cerberus is dead, allowing unrestricted passage into Gehennom.]",
         output = "text",
         text = [[Cerberus falls to the ground, all three of its massive heads
gasping for its final breath, before finally succumbing to
its injuries and dying.  Nearby, you watch as the gates to
Gehennom open before you.

Abandon all hope, ye who enter here...]],
      },
      charon_talk = {
         "\"The Gates of Hell are nigh-impenetrable.\"",
         "\"Abandon all hope, ye who enter here.\"",
         "\"This is all that remains of the river Styx.\"",
         "Charon asks if you've seen his boat.",
         "\"Mortal, take heed!  Cerberus guards the entrance to Gehennom, and cannot be dispatched easily.\"",
         "\"Cerberus awaits beyond these gates.  Beware, mortal!\"",
      },
      demon_cuss = {
         "\"I first mistook thee for a statue, when I regarded thy head of stone.\"",
         "\"Come here often?\"",
         "\"Doth pain excite thee?  Wouldst thou prefer the whip?\"",
         "\"Thinkest thou it shall tickle as I rip out thy lungs?\"",
         "\"Eat slime and die!\"",
         "\"Go ahead, fetch thy mama!  I shall wait.\"",
         "\"Go play leapfrog with a herd of unicorns!\"",
         "\"Hast thou been drinking, or art thou always so clumsy?\"",
         "\"This time I shall let thee off with a spanking, but let it not happen again.\"",
         "\"I've met smarter (and prettier) acid blobs.\"",
         "\"Look!  Thy bootlace is undone!\"",
         "\"Mercy!  Dost thou wish me to die of laughter?\"",
         "\"Run away!  Live to flee another day!\"",
         "\"Thou hadst best fight better than thou canst dress!\"",
         "\"Twixt thy cousin and thee, Medusa is the prettier.\"",
         "\"Methinks thou wert unnaturally stirred by yon corpse back there, eh, varlet?\"",
         "\"Up thy nose with a rubber hose!\"",
         "\"Verily, thy corpse could not smell worse!\"",
         "\"Wait!  I shall polymorph into a grid bug to give thee a fighting chance!\"",
         "\"Such an addlebrain %Qp the Amulet?  Thou wilt but lose it, cretin.\"",
         "\"Thou ought to be a comedian, thy skills are so laughable!\"",
         "\"Thy gaze is so vacant, I thought thee a floating eye!\"",
         "\"Thy head is unfit for a mind flayer to munch upon!\"",
         "\"Only thy reflection could love thee!\"",
         "\"Hast thou considered masking thy odor?\"",
         "\"Hold!  Thy face is a most exquisite torture!\"",
         "\"I wouldst fart in thy direction, but it might improve thy smell!\"",
      },
      goblin_king_defeated = {
         synopsis = "[The Goblin King is dead, opening passage to Mine Town and Mines' End.]",
         output = "text",
         text = [[The Goblin King's massive body lays sprawled out before you,
his blood slowly seeping out from under him, spreading across
the cavern floor.  Around his neck, you notice a magical
talisman with a familiar glyph etched into its surface.
Before you have a chance to examine it further, it glows
fiercely for a brief moment, and then turns to ash.

This was the magic that blocked access to Mine Town and
Mines' End, and now that magic has been removed, permanently.]],
      },
      hidden_dungeon_enter = {
         synopsis = "[You have entered the Hidden Dungeon.]",
         output = "text",
         text = [[The putrid stench of raw sewage assails you the moment you materialize.
You can barely keep yourself from vomiting, the smell is so bad.
As you steady yourself, you notice that the sewage around you has a
faint green glow, which illuminates a row of rusted-out jail cells
nearby.  Off in the distance, you can just barely make out what sounds
like whimpering or moaning.  You realize that you have stumbled upon
the fabled Hidden Dungeon, which was rumored to be somewhere beneath the
castle.  Legend has it that some evil force had infiltrated this place
long ago, chasing off the castle guard.  The prisoners left behind were
at the mercy of whatever evil entity now controls this cursed place.

You sense that the powerful evil force that took over the castle
dungeon is still here...]],
      },
      icequeen_freed = {
         synopsis = "[You have freed Kathryn the Enchantress from a terrible curse.]",
         output = "text",
         text = [[Thank you for freeing me from this awful curse, %p!
Long ago, a powerful and evil witch cast a spell on me, which
transformed me into the Ice Queen.  She controlled my every
thought, causing me to bring winter permanently to this land.
Only by being defeated in combat could I be free from her
wretched malediction.  I am forever in your debt.  But before
I can repay that debt, I must undo the damage I have caused
here.  And please free the captive pegasus, I am certain it
will be extremely grateful.  Fare thee well, brave %r.
Until we meet again...]],
      },
      icequeen_freed_infidel = {
         synopsis = "[You have freed Kathryn the Enchantress from a terrible curse.]",
         output = "text",
         text = [[I suppose I should thank you for freeing me %p,
but in good conscience, I cannot!  I do not know what your
motives are %c, but you are in the service of Moloch!
It was one of his witches that put this wretched curse upon
me in the first place.  Because you freed me, I will spare
you %p, but only for a few moments.  Leave this place
while you are still able to do so.]],
      },
      icequeen_freed_undead = {
         synopsis = "[You have freed Kathryn the Enchantress from a terrible curse.]",
         output = "text",
         text = [[I suppose I should thank you for freeing me %p,
but in good conscience, I cannot!  You are undead, and
the very sight of you revolts me!  I will not tolerate
such an abomination.  Because you freed me, I will spare
you %p, but only for a few moments.  Leave this
place while you are still able to do so.]],
      },
      legacy = {
         synopsis = "[%dC has chosen you to recover the Amulet of Yendor for %dI.]",
         output = "menu",
         text = [[It is written in the Book of %d:

    After the Creation, the cruel god Moloch rebelled
    against the authority of Marduk the Creator.
    Moloch stole from Marduk the most powerful of all
    the artifacts of the gods, the Amulet of Yendor,
    and he hid it in the dark cavities of Gehennom, the
    Under World, where he now lurks, and bides his time.

Your %G %d seeks to possess the Amulet, and with it
to gain deserved ascendance over the other gods.

You, a newly trained %r, have been heralded
from birth as the instrument of %d.  You are destined
to recover the Amulet for your deity, or die in the
attempt.  Your hour of destiny has come.  For the sake
of us all:  Go bravely with %d!]],
      },
      legacy_convict = {
         synopsis = "[%dC has chosen you to recover the Amulet of Yendor for %dI.]",
         output = "text",
         text = [[It is written in the Book of %d:

    After the Creation, the cruel god Moloch rebelled
    against the authority of Marduk the Creator.
    Moloch stole from Marduk the most powerful of all
    the artifacts of the gods, the Amulet of Yendor,
    and he hid it in the dark cavities of Gehennom, the
    Under World, where he now lurks, and bides his time.

Your %G %d seeks to possess the Amulet, and with it
to gain deserved ascendance over the other gods.

You, a newly escaped %r, have chosen to redeem
yourself by recovering the Amulet for %d.  You are
determined to recover the Amulet for your deity, or die
in the attempt.  Your hour of destiny has come.  For
the sake of us all:  Go bravely with %d!]],
      },
      lucifer_defeated = {
         synopsis = "[Lucifer has been defeated, allowing passage into Purgatory.]",
         output = "text",
         text = [[As you deliver the final, fatal blow, Lucifer's eyes open wide
in shock and utter disbelief that he could be defeated.  The
beautiful glow that emanated from his body fades away to nothing,
and his once majestic wings shrivel and turn to dust.  All that
is left of the mightiest of devils is a smoking, putrid shell as
his life essence fades into nothingness.

Entry into Purgatory awaits.]],
      },
      lucifer_demand_amulet = {
         synopsis = "[Lucifer introduces himself to you.]",
         output = "text",
         text = [[Before you is the most angelic, exquisite being you have ever
seen.  A large and imposing figure, with massive wings spread
wide that block out everything behind him.  So great is his
beauty, that light actually radiates from his body, causing
you to briefly shield your eyes.

On closer inspection, you notice that his wings are as dark as
night, and that an evil, sinister smile slowly starts to spread
across his beautiful face.

    "Behold, mortal!  For I am Lucifer, ruler of Gehennom and
     all that dwell here.  Give unto me the Amulet, %r,
     so that I may rule over Heaven as well!"

Lucifer reaches out a clawed hand for the Amulet...]],
      },
      lucifer_demand_idol = {
         synopsis = "[Lucifer introduces himself to you.]",
         output = "text",
         text = [[Before you is the most angelic, exquisite being you have ever
seen.  A large and imposing figure, with massive wings spread
wide that block out everything behind him.  So great is his
beauty, that light actually radiates from his body, causing
you to briefly shield your eyes.

On closer inspection, you notice that his wings are as dark as
night, and that an evil, sinister smile slowly starts to spread
across his beautiful face.

    "Behold, mortal!  For I am Lucifer, ruler of Gehennom and
     all that dwell here.  Give unto me the Idol, %r,
     so that I may rule over Heaven as well!"

Lucifer reaches out a clawed hand for the Idol...]],
      },
      purgatory_enter = {
         synopsis = "[You have entered Purgatory.]",
         output = "text",
         text = [[Before you lies a majestic meadow, dotted with lush trees, and a
lazy slow-moving river flows through it.  Far off in the distance,
an old structure resides, overgrown with vines.  You have entered
Purgatory, a seemingly beautful, peaceful place.  But not all is as
it seems...]],
      },
      pauper_legacy = {
         synopsis = "[%dC has chosen you to recover the Amulet of Yendor for %dI.]",
         output = "menu",
         text = [[It is written in the Book of %d:

    After the Creation, the cruel god Moloch rebelled
    against the authority of Marduk the Creator.
    Moloch stole from Marduk the most powerful of all
    the artifacts of the gods, the Amulet of Yendor,
    and he hid it in the dark cavities of Gehennom, the
    Under World, where he now lurks, and bides his time.

Your %G %d seeks to possess the Amulet, and with it
to gain deserved ascendance over the other gods.

You, an untrained %r, have been unable to adequately
prepare to be the instrument of %d.  Nevertheless, you
are destined to recover the Amulet for your deity, or die
in the attempt.  Your hour of destiny has come.  For the
sake of us all:  Go bravely with %d!]],
      },
      quest_complete_no_bell = {
         text = [["The silver bell which was hoarded and cursed by
%n will be essential in locating
the Amulet of Yendor."]],
      },
      quest_portal = {
         output = "pline",
         text = [[You receive a faint telepathic message from %l:
Your help is urgently needed at %H!
Look for a ...ic transporter.
You couldn't quite make out that last message.]],
      },
      quest_portal_again = {
         text = "You again sense %l pleading for help.",
      },
      quest_portal_demand = {
         text = "You again sense %l demanding your attendance.",
      },
      vecna_defeated = {
         synopsis = "[Vecna has been destroyed, allowing all other liches to be eliminated.]",
         output = "text",
         text = [[As you watch Vecna's decrepit body crumble to dust, you sense
a powerful, evil force leave his quickly disintegrating body.
It dissipates out into the void, gone forever.  At that same
moment, you hear from far away the terrified screams of the
liches that still remain in Vecna's stronghold.  Their source
of power that kept them from being annihilated no longer
protects them.  They know it, and now so do you.]],
      },
      vecna_hand = {
         synopsis = "[The Hand of Vecna has permanently affixed itself to your left arm.]",
         output = "text",
         text = [[As you move the Hand of Vecna closer to your own, your left
hand suddenly shrivels up and falls off at the wrist!  Before
you can react, the Hand of Vecna attaches itself to where your
own hand used to be, infusing your entire body with an evil
magic.  In a panic, you struggle to remove it from your arm,
but it has merged itself to you completely.  It slowly dawns on
you that you will never be able to free yourself from it.]],
      },
      vecna_hand_evil = {
         synopsis = "[The Hand of Vecna has permanently affixed itself to your left arm.]",
         output = "text",
         text = [[As you move the Hand of Vecna closer to your own, your left
hand suddenly shrivels up and falls off at the wrist!  Before
you can react, the Hand of Vecna attaches itself to where your
own hand used to be, infusing your entire body with an evil
magic.  Exhilarated, you raise the Hand high above your head,
reveling in your new-found power.  You realize that the Hand
of Vecna has merged itself to your body, and that you will
never be free of it.  An evil smile slowly spreads across your
face at this revelation.]],
      },
      vlad_tower_enter = {
         synopsis = "[You have just entered an rock cavern, which appears bottomless.]",
         output = "text",
         text = [[You enter an enormous rock cavern, the bottom of which you
cannot see.  Off in the distance, you can barely make out
what appears to be a ladder dangling from a hole in the
cavern ceiling.  You catch yourself and regain your balance
as you step precariously close to the cliff's edge, careful
not to fall into the abyss.]],
      },
      wizards_tower_enter = {
         synopsis = "[You have entered the Wizard of Yendor's lair.]",
         output = "text",
         text = [[You have finally made the ascent to the top of the tower.  As you
catch your breath, you peer outside through the iron-barred windows.
You can view off in the distance a small rocky platform that appears
to be suspended in midair, and a slender jagged bridge connecting it
to the top of the tower.  Various creatures hover around the platform,
guarding... something?  One misstep, and you would certainly fall to
your death.  You sense an evil, powerful presence here, and realize you
are in the lair of the Wizard of Yendor himself.  Beware!]],
      },
   },
   Arc = {
      assignquest = {
         synopsis = "[%nC has stolen %o.  Locate %i, defeat %ni, and return %O.]",
         output = "text",
         text = [["Grave times have befallen the college, for %na has
stolen %o.  Without it, the board of directors of
the university will soon have no choice but to revoke our research grants.

"You must locate the entrance to %i.  Within it,
you will find %n.

"You must then defeat %n and return %o
to me.  Only with %o can we lift the curse from the silver
bell.  Otherwise, all hope of finding the Amulet of Yendor is lost.

"Only in this way will we be able to prevent the budget cuts that could
close this college.

"May the wisdom of %d be your guide."]],
      },
      badalign = {
         synopsis = "[\"%pC, you have strayed from the %a path.  Purify yourself!\"]",
         output = "text",
         text = [["%pC!  I've heard that you've been using sloppy techniques.  Your
results lately can hardly be called suitable for %ra!

"How could you have strayed from the %a path?  Go from here, and come
back only when you have purified yourself."]],
      },
      badlevel = {
         synopsis = "[%pC, a mere %r is too inexperienced.]",
         output = "text",
         text = [["%p, you are yet too inexperienced to undertake such a demanding
quest.  A mere %r could not possibly face the rigors demanded and
survive.  Go forth, and come here again when your adventures have further
taught you."]],
      },
      discourage = {
         "\"Try your best, %p.  You cannot defeat me.\"",
         "\"I shall rend the flesh from your body whilst you still breathe!\"",
         "\"First you, %p, then I shall destroy your mentor, %l.\"",
         "\"Tiring yet, %p?  I draw my power from my master and cannot falter!\"",
         "\"I shall rend thy soul from thy body and consume it!\"",
         "\"You are far too %a -- it weakens you.  You shall die in this place.\"",
         "\"%d has forsaken you!  You are lost now!\"",
         "\"A mere %r cannot hope to defeat me!\"",
         "\"If you are the best %l can send, I have nothing to fear.\"",
         "\"Die %c!  I shall exhibit your carcass as a trophy.\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that we may lift the curse from the silver bell.\"",
         "\"Beware, for %n is powerful and cunning.\"",
         "\"To locate the entrance to %i, you must pass many traps.\"",
         "\"A %nt may be vulnerable to attacks by magical cold.\"",
         "\"Call upon %d when you encounter %n.\"",
         "\"You must destroy %n.  It will pursue you otherwise.\"",
         "\"%oC is a mighty weapon.  With it you can destroy %n.\"",
         "\"Go forth with the blessings of %d.\"",
         "\"I will have my %gP watch for your return.\"",
         "\"Remember not to stray from the true %a path.\"",
         "\"You may be able to sense %o when you are near.\"",
      },
      firsttime = {
         synopsis = "[You arrive at %H, but all is not well.]",
         output = "text",
         text = [[You are suddenly in familiar surroundings.  The buildings in the distance
seem to be those of your old alma mater, but something is wrong.  It feels
as if there has been a riot recently, or %H has
been under siege.

All of the windows are boarded up, and there are objects scattered around
the entrance.

Strange forbidding shapes seem to be moving in the distance.]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Thank you %p for your sacrifice, I know this must have been
difficult to do. Rest assured, %d smiles down
upon you this day.  %o will be safe with us."]],
      },
      goal_alt = {
         text = "The have returned to %ns lair.",
      },
      goal_first = {
         synopsis = "[This strange feeling must be the presence of %o.]",
         output = "text",
         text = [[A strange feeling washes over you, and you think back to things you
learned during the many lectures of %l.

You realize the feeling must be the presence of %o.]],
      },
      goal_next = {
         text = "The familiar presence of %o is in the ether.",
      },
      gotit = {
         synopsis = "[The power of %o flows through your body!  You must return it to %l.]",
         output = "text",
         text = [[The power of %o flows through your body!  You feel
as if you could now take on the Wizard of Yendor himself and win, but
you know you must return %o to %l.]],
      },
      gotit2 = {
         synopsis = "[The power of %o flows through your body!]",
         output = "text",
         text = [[The power of %o flows through your body!  You feel
as if you could now take on the Wizard of Yendor himself and win.]],
      },
      guardtalk_after = {
         "\"Did you see Lash LaRue in 'Song of Old Wyoming' the other night?\"",
         "\"Hey man, got any potions of hallucination for sale?\"",
         "\"I guess you are guaranteed to make full professor now.\"",
         "\"So, what was worse, %n or your entrance exams?\"",
         "\"%oC is impressive, but nothing like the bones I dug up!\"",
      },
      guardtalk_before = {
         "\"Did you see Lash LaRue in 'Song of Old Wyoming' the other night?\"",
         "\"Hey man, got any potions of hallucination for sale?\"",
         "\"Did you see the artifact %l brought back from the last dig?\"",
         "\"So what species do *you* think we evolved from?\"",
         "\"So you're %ls prize pupil!  I don't know what he sees in you.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and sacrifice it at the altar of %d.]",
         output = "text",
         text = [["Congratulations, %p.  I wondered if anyone could prevail against
the Wizard and the minions of Moloch.  Now, you must embark on one
final adventure.

"Take the Amulet, and find your way onto the Astral Plane.
There you must find the altar of %d and sacrifice the
Amulet on that altar to fulfill your destiny.

"Remember, your path now should always be upwards."]],
      },
      killed_nemesis = {
         text = "The body of %n dissipates in a cloud of noxious fumes.",
      },
      leader_first = {
         synopsis = "[\"You have returned, %p, to a difficult task.\"]",
         output = "text",
         text = [["Finally you have returned, %p.  You were always
my most promising student.  Allow me to see if you are ready for the
most difficult task of your career."]],
      },
      leader_last = {
         synopsis = "[\"%pC, you have failed us.  Begone!\"]",
         output = "text",
         text = [["%p, you have failed us.  All of my careful training has been in
vain.  You will never be allowed to accomplish this task while I live!
  Your tenure at this college has been revoked!

"You are a disgrace to the profession!"]],
      },
      leader_next = {
         text = [["Again, %p, you stand before me.
Let me see if you have gained experience in the interim."]],
      },
      leader_other = {
         text = [["Once more, %p, you have returned from the field.
Are you finally ready for the task that must be accomplished?"]],
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[%l looks intently at %o.

"Well done, %p.  You have defeated %n and
recovered %o, and we are ready to lift the curse
from the silver bell.  Alas, I fear that %o shall
never be truly safe unless it is kept here with
us at %H.

Please %p, return %o to us now.]],
      },
      locate_first = {
         synopsis = "[This foreboding edifice must hide the entrance to %i.]",
         output = "text",
         text = [[A plain opens before you.  Beyond the plain lies a foreboding edifice.

You have the feeling that you will soon find the entrance to
%i.]],
      },
      locate_next = {
         text = "Once again, you are near the entrance to %i.",
      },
      nemesis_first = {
         synopsis = "[\"Come, %p, I shall destroy you!\"]",
         output = "text",
         text = [["So, %p, you think that you can succeed in recovering
%o, when your teacher, %l, has already failed.

"Come, try your best!  I shall destroy you, and gnaw on your bones."]],
      },
      nemesis_next = {
         synopsis = "[\"Again you try to best me, %p?  You shall never recover %o.\"]",
         output = "text",
         text = [["Again you try to best me, eh %p?  Well, you shall fail again.

"You shall never recover %o.

"I shall bear your soul to the Plane of Origins for my master's pleasure."]],
      },
      nemesis_other = {
         text = "\"You persist yet %p!  Good.  Now, you shall die!\"",
      },
      nemesis_wantsit = {
         text = [["I shall have %o from you, %p, then feast
upon your entrails!"]],
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[%lC instructs you to guard %o from now on.]",
         output = "text",
         text = [[%lC touches %o briefly, gazes at its smooth black surface,
then smiles at you and says:

"Well done, %p.  You have defeated %n and
recovered %o, and we have lifted the curse from
the silver bell.  Alas, I fear that %o shall
never be safe here.

Please take %o with you.  You, %p, can
guard it now far better than I.

May the blessings of %d follow you and guard you."]],
      },
      offeredit2 = {
         synopsis = "[\"Resume your search for the Amulet beyond the magic portal to %Z.\"]",
         output = "text",
         text = [[%l gazes at the powerful atlatl.

"Of all the items that have been excavated from %i,
this one has given us the greatest insight into Toltec civilisation.
Understand that guarding this artifact is no small responsibility,
%p. Now, with the curse lifted from the silver bell, it is time
that you resumed your expedition for the Amulet.
%Z await you through the portal which brought you here."]],
      },
      othertime = {
         text = [[You are back at %H.
You have an odd feeling this may be the last time you ever come here.]],
      },
      posthanks = {
         synopsis = "[\"Have you progressed with your quest to regain the Amulet of Yendor for %d?\"]",
         output = "text",
         text = [["Welcome back, %p.  Have you progressed with your quest to
regain the Amulet of Yendor for %d?"]],
      },
   },
   Bar = {
      assignquest = {
         synopsis = "[\"Find %n, defeat %ni, and return %o to us.\"]",
         output = "text",
         text = [["The world is in great need of your assistance, %p.

"About six months ago, I learned that a mysterious sorcerer, known
as %n, had begun to gather a large group of cutthroats and brigands
about %ni.

"At about the same time, these people you once rode with `liberated' a
potent magical ring, %o, from a Turanian caravan.
Also in %nj possession is the silver bell, which has been
cursed.  We cannot lift the curse from the bell unless you return
%o, otherwise all hope of finding the Amulet of
Yendor is lost.

"%nC and %nj Black Horde swept down upon %i and defeated
the people there, driving them out into the desert.  He has taken
%o, and seeks to bend it to %nj will.  I detected the
subtle changes in the currents of fate, and joined these people.
Then I sent forth a summons for you.

"If %n can bend %o to %nj will, he will become
almost indestructible.  He will then be able to enslave the minds of
men across the world.  You are the only hope.  The gods smile upon you,
and with %d behind you, you alone can defeat %n.

"You must go to %i.  From there, you can track down
%n, defeat %ni, and return %o to us.  Only
then will the world be safe."]],
      },
      badalign = {
         synopsis = "[\"You have wandered from the path of the %a.  Come back when you have atoned.\"]",
         output = "text",
         text = [["%pC!  You have wandered from the path of the %a!
If you attempt to overcome %n in this state, he will surely
enslave your soul.  Your only hope, and ours, lies in your purification.
Go forth, and return when you feel ready."]],
      },
      badlevel = {
         synopsis = "[\"You are too inexperienced.  Come back when you are %Ra.\"]",
         output = "text",
         text = [["%p, I fear that you are as yet too inexperienced to face
%n.  Only %Ra with the help of %d could ever hope to
defeat %ni."]],
      },
      discourage = {
         "\"My pets will dine on your carcass tonight!\"",
         "\"You are a sorry excuse for %ra.\"",
         "\"Run while you can, %c.  My next spell will be your last.\"",
         "\"I shall use your very skin to bind my next grimoire.\"",
         "\"%d cannot protect you now.  Here, you die.\"",
         "\"Your %a nature makes you weak.  You cannot defeat me.\"",
         "\"Come, %c.  I shall kill you, then unleash the horde on your tribe.\"",
         "\"Once you are dead, my horde shall finish off %l, and your tribe.\"",
         "\"Fight, %c, or are you afraid of the mighty %n?\"",
         "\"You have failed, %c.  Now, my victory is complete.\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that we may lift the curse from the silver bell.\"",
         "\"%nC is strong in the dark arts, but not immune to cold steel.\"",
         "\"Remember that %n is a great sorcerer.  He lived in the time of Atlantis.\"",
         "\"If you fail, %p, I will not be able to protect these people long.\"",
         "\"To enter %i, you must be very stealthy.  The horde will be on guard.\"",
         "\"Call upon %d in your time of need.\"",
         "\"May %d protect you, and guide your steps.\"",
         "\"If you can lay hands upon %o, wear it for its healing powers.\"",
         "\"I cannot stand against %ns sorcery.  But %d will help you.\"",
         "\"Do not fear %n.  I know you can defeat %ni.\"",
         "\"You have a great road to travel, %p, but only after you defeat %n.\"",
      },
      firsttime = {
         synopsis = "[You reach the vicinity of %H, but sense evil magic nearby.]",
         output = "text",
         text = [[Warily you scan your surroundings, all of your senses alert for signs
of possible danger.  Off in the distance, you can %x the familiar shapes
of %H.

But why, you think, should %l be there?

Suddenly, the hairs on your neck stand on end as you detect the aura of
evil magic in the air.

Without thought, you ready your weapon, and mutter under your breath:

    "By %d, there will be blood spilt today."]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Thank you %p for your sacrifice, I know this must have been
difficult to do. Rest assured, %d smiles down
upon you this day.  %o will be safe with us."]],
      },
      goal_first = {
         synopsis = "[This is surely the lair of %n.]",
         output = "text",
         text = [[The hairs on the nape of your neck lift as you sense an energy in the
very air around you.  You fight down a primordial panic that seeks to
make you turn and run.  This is surely the lair of %n.]],
      },
      goal_next = {
         text = "Yet again you feel the air around you heavy with malevolent magical energy.",
      },
      gotit = {
         synopsis = "[You feel the power of %o flowing through your body.]",
         output = "text",
         text = [[As you pick up %o, you feel the power of it
flowing through your entire body.  You feel as though no ailment
can befall you, nor would any wound be too great to heal.]],
      },
      gotit2 = {
         synopsis = "[You feel the power of %o flowing through your body.]",
         output = "text",
         text = [[As you pick up %o, you feel the power of it
flowing through your entire body.  You feel as though no ailment
can befall you, nor would any wound be too great to heal.]],
      },
      guardtalk_after = {
         "\"The battles here have been good -- our enemies' blood soaks the soil!\"",
         "\"Remember that glory is crushing your enemies beneath your feet!\"",
         "\"Times will be good again, now that the horde is vanquished.\"",
         "\"You have brought our clan much honor in defeating %n.\"",
         "\"You will be a worthy successor to %l.\"",
      },
      guardtalk_before = {
         "\"The battles here have been good -- our enemies' blood soaks the soil!\"",
         "\"Remember that glory is crushing your enemies beneath your feet!\"",
         "\"There has been little treasure to loot, since the horde arrived.\"",
         "\"The horde is mighty in numbers, but they have little courage.\"",
         "\"%lC is a strange one, but he has helped defend us.\"",
      },
      hasamulet = {
         synopsis = "[\"Take the Amulet to the altar of %d on the Astral Plane and offer it.\"]",
         output = "text",
         text = [["This is wondrous, %p.  I feared that you could not possibly
succeed in your quest, but here you are in possession of the Amulet
of Yendor!

"I have studied the texts of the magi constantly since you left.  In
the Book of Skelos, I found this:

    %d will cause a child to be sent into the world.  This child is to
    be made strong by trial of battle and magic, for %d has willed it so.
    It is said that the child of %d will recover the Amulet of Yendor
    that was stolen from the Creator at the beginning of time.

"As you now possess the amulet, %p, I suspect that the Book
speaks of you.

    The child of %d will take the Amulet, and travel to the Astral
    Plane, where the Great Temple of %d is to be found.  The Amulet
    will be sacrificed to %d, there on %dJ altar.  Then the child will
    stand by %d as champion of all %cP for eternity.

"This is all I know, %p.  I hope it will help you."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you, but you feel the overpowering aura of magic fading.]",
         output = "text",
         text = [[%nC falls to the ground, and utters a last curse at you.  Then %nj
body fades slowly, seemingly dispersing into the air around you.  You
slowly become aware that the overpowering aura of magic in the air has
begun to fade.]],
      },
      leader_first = {
         synopsis = "[\"At last you have returned.  There is a great quest you must undertake.\"]",
         output = "text",
         text = [["Ah, %p.  You have returned at last.  The world is in dire
need of your help.  There is a great quest you must undertake.

"But first, I must see if you are ready to take on such a challenge."]],
      },
      leader_last = {
         synopsis = "[\"You have betrayed %d; soon %n will destroy us.  Begone!\"]",
         output = "text",
         text = [["Pah!  You have betrayed the gods, %p.  You will never attain
the glory which you aspire to.  Your failure to follow the true path has
closed this future to you for as long as I still draw breath!

"I will protect these people as best I can, but soon %n will overcome
me and destroy all who once called you %s.  Now begone!"]],
      },
      leader_next = {
         text = "\"%p, you are back.  Are you ready now for the challenge?\"",
      },
      leader_other = {
         text = "\"Again, you stand before me, %p.  Surely you have prepared yourself.\"",
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[%l gazes intently at %o, then back at you

"Well done, %p.  You have defeated %n and
recovered %o, and we are ready to lift the curse
from the silver bell.  Alas, I fear that %o shall
never be truly safe unless it is kept here with
us at %H.

Please %p, return %o to us now.]],
      },
      locate_first = {
         synopsis = "[You have located %i.]",
         output = "text",
         text = [[The scent of water comes to you in the desert breeze.  You know that
you have located %i.]],
      },
      locate_next = {
         text = "Yet again you have a chance to infiltrate %i.",
      },
      nemesis_first = {
         synopsis = "[%nC boasts that %nh has slain many.  \"Prepare to die, %c.\"]",
         output = "text",
         text = [["So.  This is what that second rate sorcerer %l sends to do %lj bidding.
I have slain many before you.  You shall give me little sport.

"Prepare to die, %c."]],
      },
      nemesis_next = {
         text = "\"I have wasted too much time on you already.  Now, you shall die.\"",
      },
      nemesis_other = {
         text = "\"You return yet again, %c!  Are you prepared for death now?\"",
      },
      nemesis_wantsit = {
         text = [["I shall have %o back, you pitiful excuse for %ca.
And your life as well."]],
      },
      nexttime = {
         text = [[Once again, you near %H.  You know that %l
will be waiting.]],
      },
      offeredit = {
         synopsis = "[%lC tells you to guard %o, and to return when you have triumphed.]",
         output = "text",
         text = [[When %l sees %o, he smiles, and says:

    Well done, %p.  You have saved the world from certain doom.
    What, now, should be done with %o?

    These people, brave as they are, cannot hope to guard it from
    other sorcerers who will detect it, as surely as %n did.

    Take %o with you, %p.  It will guard you in
    your adventures, and you can best guard it.  The curse on the
    silver bell has been lifted.  You embark on a quest far greater
    than you realize.

    Remember me, %p, and return when you have triumphed.  I
    will tell you then of what you must do.  You will understand when the
    time comes.]],
      },
      offeredit2 = {
         synopsis = "[\"You keep %o.  Return to %Z to search for the Amulet.\"]",
         output = "text",
         text = [[%l gazes reverently at %o, then back at you.

"You are its keeper now, and with the curse lifted from the silver
bell, the time has come to resume your search for the Amulet.
%Z await your return through the magic portal which
brought you here."]],
      },
      othertime = {
         text = [[Again, and you think possibly for the last time, you approach
%H.]],
      },
      posthanks = {
         text = "\"Tell us, %p, have you fared well on your great quest?\"",
      },
   },
   Cav = {
      assignquest = {
         synopsis = "[Go beat %n.  Get %o.  Bring back things.]",
         output = "text",
         text = [["Yes, you good now, %p.  Me tell you what happen to us.

"Right after you leave, Bad People come to caves, fight us.
They sent by %n.  This one very bad.  He lead the
Bad People to fight us.  Some of us die.

"While we hurt, Bad People take %o.  They take it to %i.
We can not get it back.  %n will kill us.

"%p, you find %i, take back %o from %n.  Silver bell
there also, get it.  Then we can make caves safe."
]],
      },
      badalign = {
         synopsis = "[\"You not doing %a good.  Go fix self.\"]",
         output = "text",
         text = [["%pC!  You not do like me teach you.  You not do right things
on true path of %a.  Me send you from our caves.  Go, %p.
Go away, make self right.  Maybe you do this quest."]],
      },
      badlevel = {
         synopsis = "[\"%rA not ready.  Go do other things.\"]",
         output = "text",
         text = [["No, %p, you not ready to go on such hard quest me give you.

"%rA can not do what it take to find
%i, could not fight %n, could not bring back %o.

"Go do more things.  You learn stuff you need.
%d say so."]],
      },
      discourage = {
         "\"You are weak, %c.  No challenge for the Father of all Giants.\"",
         "\"I grow hungry, %r.  You look like a nice appetizer!\"",
         "\"Join me for lunch?  You're the main course, %c.\"",
         "\"With %o, I am invincible!  You cannot succeed.\"",
         "\"Your mentor, %l has failed.  You are nothing to fear.\"",
         "\"You shall die here, %c.  %rA cannot hope to defeat me.\"",
         "\"You, a mere %r challenge the might of %n?  Hah!\"",
         "\"I am the Father of all Giants!  You cannot hope to defeat me.\"",
         "\"I am famished, %r.  I shall rip the meat from your bones!\"",
         "\"%d has deserted you, %c.  This is my domain.\"",
      },
      encourage = {
         "\"You return with %o.  We make curse go away from silver bell.\"",
         "\"%nC not hurt by many things. Use magic he not use, maybe he not know how to beat that.\"",
         "\"When you fight %n, call to %d for help.\"",
         "\"There no place to hide in %ns cave.\"",
         "\"Best way to not die when %n fight you is, run away.\"",
         "\"You no stop to take pretty things in %ns lair. Just make sure you get %o.\"",
         "\"You take %o from %n, maybe you use it to kill him.\"",
         "\"You no be fooled by %ns size.  He fast, has magic too.\"",
         "\"Me would send %gP with you, but we need them to fight here, keep our cave safe.\"",
         "\"You be %a at all times.  Make you strong.\"",
         "\"If we had death stick, %n not attack us.\"",
      },
      firsttime = {
         synopsis = "[You come to %H.  It wrong.]",
         output = "text",
         text = [[You come to %H.  It not right.  There no meat in cave.
There no mammoth hides.  The people are sad.  Things broke.  Fire not lit.
%HC not right, make you sad.  You look for %l.
Him tell you what is wrong.]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [[You do right thing, %p.  %d is happy.
%o is safe with us in cave."]],
      },
      goal_first = {
         synopsis = "[You come to big cave.  %nC is here.]",
         output = "text",
         text = [[You come to big cave, with shiny walls.  Walls look like they
have been burned.

You see bones on the floor, and other things.
It smell bad in here, like rotting carcass.

%nC is here, but it seem like he asleep.]],
      },
      goal_next = {
         text = "You come to lair of %n again.",
      },
      gotit = {
         synopsis = "[%oC make you feel strong and good.]",
         output = "text",
         text = [[As you pick up %o it seem heavy at first, but then
your arms feel strong.

You feel like you can beat up anyone now.]],
      },
      gotit2 = {
         synopsis = "[%oC make you feel strong and good.]",
         output = "text",
         text = [[As you pick up %o it seem heavy at first, but then
your arms feel strong.

You feel like you can beat up anyone now.]],
      },
      guardtalk_after = {
         "\"We hunt mammoth again.\"",
         "\"Peace back.  Peace good.  Give thanks to %d!\"",
         "\"You back!  You find %o?\"",
         "\"Strong %p, tell us how you fight %n.\"",
         "\"%lC old.  Maybe you guide us after him die.\"",
      },
      guardtalk_before = {
         "\"We not get much food since Big Ones seal off %H.\"",
         "\"Since Bad People come with %n, we fight all the time.\"",
         "\"You think we beat %n, take back %o?\"",
         "\"Hey, %p, you help?\"",
         "\"%lC old.  We not know who guide us after him die.\"",
      },
      hasamulet = {
         synopsis = "[\"Take Amulet to %d in place of gods.\"]",
         output = "text",
         text = [["Oh, good, %p, you find thing for %d.

"Take god-thing up to sky.  It take you where you need to go,
to place of earth, place of fire, place of air, place of water,
and place of gods, where the Great House of %d is.

"Take god thing up there, sacrifice it on right altar."]],
      },
      killed_nemesis = {
         text = [[%nC crashes to the ground.  His big limbs splayed out.
Big cloud of smelly smoke come out of his body.]],
      },
      leader_first = {
         synopsis = "[\"You back.  We need much help.\"]",
         output = "text",
         text = [["You come back, %p.  Thank %d.

"We need much help, my %S.

"But first, me see if you can do things me ask you to do."]],
      },
      leader_last = {
         synopsis = "[\"You not %L.  Leave!\"]",
         output = "text",
         text = [["%pC!  You not good enough.  Seems you not able to get ready.
Me pick other one from tribe to take your place.

"Leave %H!  You let us down.  You choose
path of %C over true path of %L.  You go, now!
Or else we eat you!

"You not live in our eyes."]],
      },
      leader_next = {
         text = "\"You back, %p.  Me see if you ready now.\"",
      },
      leader_other = {
         text = "\"Ah, %p.  You ready?\"",
      },
      leader_wantsit = {
         synopsis = "[\"%lC tells you to give him %o.\"]",
         output = "text",
         text = [[%lC see %o in your hands.
Him look at you and say:

    You have it!  We saved now.  But me fear that Bad
    People still want to take it from us.  Me decide that
    %o stay here with us in cave.  Me ready to take
    curse from silver bell, then you take it.

    You give %o to us now.]],
      },
      locate_first = {
         synopsis = "[You %x many large footprints, see and smell dead things.]",
         output = "text",
         text = [[You %x many big footprints on ground.  Cave ahead of you
is larger than most cave you have been in before.

You smell dead things in there and see bones on the ground.]],
      },
      locate_next = {
         text = "You come to %i again.",
      },
      nemesis_first = {
         synopsis = "[%nC threatens to eat you.]",
         output = "text",
         text = [["So, follower of %l, you seek to invade the lair of
%n.  Only my meals are allowed down here.  Prepare
to be eaten!"]],
      },
      nemesis_next = {
         text = [["So, again you face me, %c.  No one has ever before escaped me.
Now I shall kill you."]],
      },
      nemesis_other = {
         text = "\"You are getting annoying, %c.  Prepare to die.\"",
      },
      nemesis_wantsit = {
         text = "\"I'll have %o from you, %c.  You shall die.\"",
      },
      nexttime = {
         text = "You come to %H again.",
      },
      offeredit = {
         synopsis = "[\"Take %o with you.  It maybe help you on big quest.\"]",
         output = "text",
         text = [[%lC see %o in your hands.
Him smile and say:

    You have it!  We saved now.  But me fear that Bad People
    still want to take it from us.

    You, take %o away with you.  Curse gone from silver bell.
    Both help you find other thing you look for.]],
      },
      offeredit2 = {
         synopsis = "[\"You keep %o now.  Return to %Z.]",
         output = "text",
         text = [[%l take %o, then him look at you.

"You keep it now.  Keep it safe, go look for other
thing, thing for %d.  Curse gone from silver bell.
Go back though magic thing that brought you here."
]],
      },
      othertime = {
         text = "You think this may be last time you come to %H.",
      },
      posthanks = {
         text = [["%pC!  You back.
Did you find other thing %d told you to look for?"]],
      },
   },
   Con = {
      assignquest = {
         synopsis = "[Journey downward to recover %o from %n.]",
         output = "text",
         text = [["Yes, %p, it looks like you are now ready to
help us.  Things have become much worse since you
left.  The dungeons are overrun by vermin, guards
have become brutal and corrupt, and I believe that
%n has had dealings with dark powers.

"I had spent many years enchanting my prison shirt
with powers to help lead an escape from this place,
turning it into %o.
Unfortunately, %n stole it from me,
likely using it for dark, twisted purposes.

"We need your help to go down to the warden's level
to defeat the corrupt %n and recover
%o for us so that
%H will be safe again.  Grab the silver
bell while you're there, I can lift the curse
%n put on it, you can use it to find
the Amulet of Yendor."]],
      },
      badalign = {
         synopsis = "[\"Return when you serve %d more faithfully.\"]",
         output = "text",
         text = [["%p, I see that you are not yet a loyal servant of
%d.  Leave this place for now, and only
return when you have cleansed your spirit.  Only then
will you be ready to stand up to %n and
recover %o."]],
      },
      badlevel = {
         synopsis = "[\"Return once you are a %R.\"]",
         output = "text",
         text = [["%p, I am afraid that a %r will be no
match for %n.  If I sent you now,
she would dispatch you all too easily.

"Continue your other quest, and practice your
skills.  Return to us once you have achieved the rank
of %R."]],
      },
      discourage = {
         "\"You should have never come back.  You will not escape here alive again!\"",
         "\"Even all of your pathetic lot put together would not be enough to defeat me.\"",
         "\"You are no more than lowly prison scum.  You are unworthy to even challenge me.\"",
         "\"Even %l could not stand up to me, so you have no chance at all.\"",
         "\"Even %o would not be enough to make you my equal.\"",
         "\"Your pathetic devotion to %d will not save you, either.\"",
         "\"After I kill you, I will go after %l next, and any that dare to defy me again!\"",
         "\"Die with dishonor, lowly %c!\"",
         "\"Once a %c, always a %c.\"",
         "\"How about you find yourself a cosy little cell, and hope that I forgive your insolence?\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that I can lift the curse from the silver bell.\"",
         "\"The sooner you are able to recover %o, the better off we will be.\"",
         "\"I am not sure what has happened, but even without %o, %n will make a formidable opponent.\"",
         "\"Stay true to the teachings of %d, and good fortune will be with you.\"",
         "\"I am afraid without the power of %o to protect us, we will soon be overwhelmed.\"",
         "\"%n used to be a woman of honor before turning to dark ways.  It is much too late for redemption, however.\"",
         "\"%n is arrogant, and will try to diminish your confidence.  Do not let yourself be discouraged!\"",
         "\"You will have to be very alert, and will need to use all of your cunning if you hope to defeat %n.\"",
         "\"Call upon the power of %d to protect you when you encounter %n.\"",
         "\"If you remain true to your faith, you should be able to sense the power of %o when you are near.\"",
         "\"You should be able to defeat %n easily enough in a fair fight.  Unfortunately, she doesn't fight fair.\"",
      },
      firsttime = {
         synopsis = "[You arrive in %H, but something is amiss.]",
         output = "text",
         text = [[Somehow, you have been captured and teleported back to
%H!  But wait, the portal you went through is still
here.  You had heard your old mentor %l,
the prison chaplain who taught you the ways of
%d calling for your help.  Also, %H
is not as you had left it.  There seems to be damage
to the walls, and you hear sounds of strife all
around you.]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Thank you %p for your sacrifice, I know this must have been
difficult to do. Rest assured, %d smiles down
upon you this day.  %o
will be safe with us."]],
      },
      goal_first = {
         synopsis = "[You sense the presence of %o.]",
         output = "text",
         text = [[You sense the presence of %o as soon as you
enter this level.  If %o is here, then
%n must not be far away.]],
      },
      goal_next = {
         text = [[Once again, you find yourself in the abode of
%n, and feel the presence of
%o.]],
      },
      gotit = {
         synopsis = "[You feel the power of %o, and know that you must return it to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel its power
flow through you, protecting you, and making you
more aware of your surroundings.  You know that
you need to get it back to %l as soon
as possible now.]],
      },
      gotit2 = {
         synopsis = "[You feel the power of %o.]",
         output = "text",
         text = [[As you pick up %o, you feel its power
flow through you, protecting you, and making you
more aware of your surroundings.]],
      },
      guardtalk_after = {
         "\"Thank you for ridding us of %n.  I hope the new warden will be better.\"",
         "\"Take me with you!  I will be glad to be rid of this awful place.\"",
         "\"Don't forget to come back for us once you have completed your quest!\"",
         "\"Thank you for coming back to help us in our hour of need!\"",
         "\"Leave while you still can!  If you can escape, maybe there is hope for the rest of us.\"",
      },
      guardtalk_before = {
         "\"How did you manage to escape?\"",
         "\"How is life on the outside?\"",
         "\"Between the abusive guards, and the nasty vermin, I don't know how we will hold out.\"",
         "\"This place used to be so much nicer before %n became corrupt.\"",
         "\"You are really back?  I thought you were gone for good.\"",
      },
      hasamulet = {
         synopsis = "[\"Take the amulet to the Astral plane and offer it to %d.\"]",
         output = "text",
         text = [["Congratulations, %p!  You have redeemed yourself
and proven yourself a worthy servant of %d,
and in doing so have made all of us proud.

"One final task remains for you now.  You must take
the Amulet up to the Great Temple of %d, on
the Astral plane.  There you must offer the Amulet
to %d."]],
      },
      killed_nemesis = {
         synopsis = "[%nC apologizes as %nh dies.]",
         output = "text",
         text = [[As %n approaches death, you see sudden
clarity in her eyes.

"What have I done?  Those demons have been clouding my mind.
Please forgive me, %p, although I know that I
probably do not deserve it.  Take
%o back to %l
with my blessing."

With a final coughing fit, %n spasms, and her
eyes see no more.]],
      },
      leader_first = {
         synopsis = "[%lH wants to see if you are ready.]",
         output = "text",
         text = [["%p, I am very happy to see that you are
alive and well!  We were all proud of your daring
escape, and hoped that you were doing well.
Things have become much worse in your absence,
however, and we desperately need your help.  Let
me have a look at you, and see if you are ready."]],
      },
      leader_last = {
         synopsis = "[You are not even fit to be a convict.]",
         output = "text",
         text = [["You have betrayed us all, %p!  Begone from
here, and don't come back!  You are not even worthy
to share a cell with the least of us now."]],
      },
      leader_next = {
         text = [["I see that you are back, %p.  I hope that
you are now ready to help us?  Let me see if you
are."]],
      },
      leader_other = {
         text = [["Once again, you have returned.  Let me see if you
are finally ready to help us."]],
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[%l looks upon %o
with fondness.
"Great job, %p.  You have defeated %n and
recovered %o, and
we are ready to lift the curse from the silver bell.
However, I fear that %o
will never be truly safe unless it is kept here
with us at %H.

Please %p, return %o to us now.]],
      },
      locate_first = {
         synopsis = "[You are getting closer to the warden.]",
         output = "text",
         text = [[This must be the level that leads to the warden's
area.  You must be getting closer to
%o now!]],
      },
      locate_next = {
         text = [[Once again, you find yourself near the entrance to
the warden's area.  Hopefully, you can acquire
%o before it's too late!]],
      },
      nemesis_first = {
         synopsis = "[\"You will not take %o from me.\"]",
         output = "text",
         text = [["So, %p.  You have returned, and are here to get
%o for %l?
What makes you think you can get it if
%l couldn't?  I have many
allies that have made me even stronger."]],
      },
      nemesis_next = {
         text = [["I see that you have returned, %p.  I suggest you
leave and save yourself while you still
have a chance."]],
      },
      nemesis_other = {
         text = [["Back again, %p?  You will certainly die in
your attempt to defeat me."]],
      },
      nemesis_wantsit = {
         text = [["I will get %o from you, just as I did
%l!  You cannot defeat me."]],
      },
      nexttime = {
         text = [[You have returned to %H.  Somehow, things are even
worse now than when you were on your previous visit.  Has
%l managed to still hold out?]],
      },
      offeredit = {
         synopsis = "[%lC tells you to take %o.]",
         output = "text",
         text = [["Congratulations, %p!  You have returned with
your life, and with %o!  I had
planned to use %o to lead an
escape from here, but with %n defeated,
life should go back to normal before long.

"Whether I like it or not, I know my place is here.  I
see that %o has already attuned
itself to you.  The curse on the silver bell has also
been lifted.  I offer both to you in hopes that
they will aid in your quest to recover the Amulet
of Yendor."]],
      },
      offeredit2 = {
         synopsis = "[%lC tells you that you are the keeper of %o.]",
         output = "text",
         text = [[%l looks upon %o
with fondness.
"You are its keeper now.  Take it with you back to
%Z, through the magic portal
that brought you here.  I have lifted the curse
from the silver bell, use it to find the Amulet
of Yendor."]],
      },
      othertime = {
         text = [[The situation seems to degrade, and you wonder how long
your fellow %gp can hold out against the corrupt
guards and creatures of the Underdark that pervade the
formerly tranquil %H.]],
      },
      posthanks = {
         output = "text",
         text = [["Welcome back, %p.  We have done well in your
absence.  How far have you come with your quest to
regain the Amulet of Yendor for %d?"]],
      },
   },
   Dkn = {
      assignquest = {
         synopsis = "[Reach %i and claim %o from %n.  Retrieve the cursed silver bell.]",
         output = "text",
         text = [[%lC rises from her throne, years of bitter certainty written across her face.

    "You have been chosen, %p.  %n was once my teacher - the
     greatest in all the realms.  He promised me power, promised me knowledge.
     He gave me both, then took them away when Arthur needed a favored advisor
     more than I needed his love.

    "I do not dwell on the past.  I move beyond it.  But some debts demand
     payment, and %n's debt to me is ancient.

    "%oC is mine by right - I helped in its creation,
     though he never admits it.  With it, I will see clearly what must be
     done in this world.  %n hides it from me in %i,
     surrounded by his righteousness and his precious wards.

    "Go to the forest.  Find him.  Bring me %o.
     Retrieve also the silver bell - %n has cursed it, but I can
     lift that curse.  Without it, you will never find the Amulet of Yendor.

    "Do this, and you will have my favor forever.  Fail, and you will learn
     why even Kings fear my name."

She extends her hand.  Her grip is iron.]],
      },
      badalign = {
         synopsis = "[Go and atone.  Return when you are truly %a.]",
         output = "text",
         text = [[%lC's eyes narrow dangerously.

    "Your alignment troubles me, %p.  I did not summon you here
     to waste my time with those of uncertain conviction.  I need a
     champion whose path is clear, whose darkness is uncompromising.
     Come back when your soul reflects a truer purpose."

She turns away from you.]],
      },
      badlevel = {
         synopsis = "[You are not prepared to face %n.  Return when you are %Ra.]",
         output = "text",
         text = [[%lC shakes her head, disappointed.

    "No.  You lack the power required.  This quest is not for the
     inexperienced.  Go.  Gain strength, learn your craft, and perhaps
     one day you will be worthy.  But not now.  Not yet."]],
      },
      discourage = {
         "\"I faced dragons in ages past.  Now I face you.  How far we have fallen.\"",
         "\"Your courage serves a lie, %p.  But it does you credit.\"",
         "\"She will destroy you as she sought to destroy me.\"",
         "\"How many champions has she sent before you?  How many still live?\"",
         "\"I fight to prevent her victory.  You fight because she commands it.\"",
         "\"I would have been honored to teach you.  What a waste.\"",
         "\"The Mirror was meant for wisdom, not conquest.\"",
         "\"I have seen futures where you prosper.  In most, I see only darkness.\"",
         "\"When I am dead, remember - you will pay the price for this day.\"",
         "\"I fall, she rises, and you are the instrument of that falling.\"",
      },
      encourage = {
         "%lC regards you coolly.  \"Remember, %p - return with %o so that I may lift the curse from the silver bell.\"",
         "%lC leans forward.  \"The forest will test you.  But you are stronger than %n believes.  Show him.\"",
         "%lC smiles knowingly.  \"He does not know you are coming.  Use that advantage well.\"",
         "%lC's expression hardens.  \"When you see him, remember - he is not the man I once loved.  That man died long ago.\"",
         "%lC rises from her throne.  \"The forest will try to turn you back.  But nothing is stronger than my will, %p.  Nothing.\"",
         "%lC's eyes narrow.  \"He will not come quietly.  %n is proud.  Make him understand that his time is finished.\"",
         "%lC studies you intently.  \"Many have sought %o before you.  None succeeded. But you are different, %p.  You will not fail me.\"",
         "%lC places a hand over her heart.  \"%oC calls to me.  I can feel it hiding in %i, away from my reach.  But we will claim it.\"",
         "%lC grips your arm.  \"Do not hesitate when you face him.  Hesitation is the weapon of the weak.  You are my chosen.  You will prevail.\"",
         "%lC turns away.  \"When this is done, you will understand - power is not given. It is taken from those weak enough to let it slip away.\"",
         "%lC meets your gaze one last time.  \"Go now.  Bring me what is mine.  If %n offers you mercy, remember - he showed me none.\"",
      },
      firsttime = {
         synopsis = "[%H is under siege by %ns creatures.]",
         output = "text",
         text = [[You step through the mists onto %H.  The sacred apple
trees sway in an unfelt wind, their silver leaves whispering secrets in
a tongue older than memory.  But something is wrong.  Hawks circle
overhead - not natural birds, but creatures of malice, their cries sharp
as broken glass.  In the distance, wolves howl, moving as one, as if
guided by a single will.

A Page rushes to meet you, fear plain on his face.  "Thank the old gods
thou hast come!  %lC's sanctuary is besieged.  %n sends
his creatures against us day and night.  We cannot hold much longer."

The mists swirl around you.  Somewhere ahead, Morgan le Fay awaits.]],
      },
      gaveitup = {
         synopsis = "[You give %o to %l.]",
         output = "text",
         text = [[%lC accepts %o from your hands, her fingers
brushing yours for a moment.  Her touch is ice-cold.

    "Wise," she murmurs.  "To recognize that some burdens are too great for
     lesser shoulders to bear.  I shall keep the Mirror where it will be safe.
     In time, %p, you shall understand the gift you have given
     me.  This artifact shall change everything."

She cradles the Mirror like a mother with a newborn child, and turns away
from you.]],
      },
      goal_first = {
         synopsis = "[You have entered %i, %ns forest sanctuary.]",
         output = "text",
         text = [[You enter a realm of ancient forest, dense and wild.  Massive oaks tower
overhead, their branches blocking out the sky.  A sense of powerful magic
permeates the air - you are close to your quarry.

Somewhere within this wilderness, %n has made his sanctuary.
You must find your way through the forest to reach him.]],
      },
      goal_next = {
         text = [[You return to the ancient forest.  The trees seem to watch your passage, and
the weight of %ns magic presses upon you.  His sanctuary lies ahead.]],
      },
      gotit = {
         synopsis = "[You feel the magic of %o.]",
         output = "text",
         text = [[As you claim %o, you feel a surge of power flow through
you.  The artifact pulses with ancient magic, and you suddenly understand
- or think you understand - so much.  You can feel the currents of fate
flowing around you, the threads of destiny woven into the tapestry of
this world.  The Mirror shows you visions of other places, other times.

And you understand, at last, what %n was trying to tell you.

But it is too late for understanding.  Too late for doubt.  You have made
your choice, and the path forward is set.]],
      },
      gotit2 = {
         synopsis = "[You feel the magic of %o.]",
         output = "text",
         text = [[As you claim %o from the fallen wizard's chamber, the
artifact pulses with ancient magic.  The same visions flood your mind as
before - the same terrible understanding of power and consequence.

In the distance, you hear %lC laughing.  The sound echoes
through the forest like thunder.

The Mirror is yours.  And she is coming.]],
      },
      guardtalk_after = {
         "A young Page approaches you with obvious relief.  \"Thou art the one!  Our %lC hath spoken of thee.  The quest is set - thou must succeed where others have failed.\"",
         "A Page salutes you grimly.  \"Thou dost undertake a dangerous task, %r.  But if any can succeed, perhaps it shall be thee.  Go with caution to %i.\"",
         "A Page clasps your shoulder.  \"Godspeed, champion.  Avalon rests its hopes upon thy shoulders.  %n must be defeated.  He hath wronged our Queen for long enough.\"",
         "A Page watches you with something like pity.  \"Thou art bound to %lC now, for better or worse.  Complete thy quest, and perhaps thou shalt find peace.\"",
         "A Page lowers their voice.  \"I would warn thee... but I am bound by oath to serve %lC.  Go.  Do what must be done.  But remember - %n was not always our enemy.\"",
      },
      guardtalk_before = {
         "A young Page approaches you, dressed in armor of Avalon.  \"Well met, stranger. Art thou here to aid our %lC?  These are dark times, and shadows gather.\"",
         "A weathered Page eyes you carefully.  \"Thou hast the look of a warrior about thee.  Dost thou seek audience with our Queen?  She is... occupied, but thy coming hath been foreseen.\"",
         "A Page with scarred armor nods to you.  \"Hail, %r.  Thou comest in an ill hour.  The hawks grow bold, and the wolves... the wolves do not act as beasts should.  Our Queen attends to matters of great import. Wouldst thou wait?\"",
         "A Page bows stiffly.  \"Our %lC doth expect thee.  Go, present thyself to her chambers.  But be warned - her mood is... complex these days.\"",
         "A Page barely looks at you.  \"Another one.  Very well.  Thou shalt speak with our Queen soon enough.  If thou survives what she asks of thee, perhaps we shall meet again.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and offer it to %d.]",
         output = "text",
         text = [[%lC regards the Amulet of Yendor in your possession with
undisguised hunger.

    "You have done it, %p.  The Amulet is yours.  Now you must
     take it to the Astral Plane and sacrifice it to %d.

    "Travel upward through the Planes of the Elements.  Do not falter.
     Do not fail.  When you stand before %d, remember
     who made this possible.

    "Go now.  Complete what you have begun."

She turns away, already contemplating what comes next.]],
      },
      killed_nemesis = {
         synopsis = "[%nC forgives you as %nh dies.]",
         output = "text",
         text = [[%nC falls, his staff clattering to the ground.  His eyes remain open,
clear and serene, looking upon you with infinite compassion.

    "It is done," he whispers.  "Go to her, %p.  Give her what
     she desires.  And in the years to come, when you understand what you
     have lost, when you comprehend the weight of what you have done -
     remember that I forgave you in my final hour.  That is the only grace
     I can offer you now."

His eyes close.  The great wizard has fallen.]],
      },
      leader_first = {
         synopsis = "[%lC checks whether you are ready for a great undertaking.]",
         output = "text",
         text = [[%lC receives you in her chamber.  Candlelight flickers across
walls draped in shadow.  She regards you with dark eyes that seem to
pierce your very soul.

    "So.  You have come at last.  I have been expecting you, %p.
     You have the bearing of a true warrior - or perhaps the desperation
     of a fool.  Either will serve my purposes.  But before you hear what
     I require, I must know: are you ready?  Do you have the courage to
     face what waits in %i?  Do not answer hastily.
     Many have thought themselves prepared, and they are now dead."

She studies you intently, waiting for your response.]],
      },
      leader_last = {
         synopsis = "[You are a disgrace as %ca.]",
         output = "text",
         text = [[%lC's voice cuts like a blade of ice.

    "You have failed me, %p.  You have proven yourself a disgrace
     to the order of Avalon.  Get out of my sight.  You are no champion
     of mine.  Perhaps you should seek another master - one less
     demanding than I."

She gestures toward the door with contempt.]],
      },
      leader_next = {
         synopsis = "[You are not prepared to face %n.  Return when you are %Ra.]",
         output = "text",
         text = [[%lC looks up from an ancient text, her expression unreadable.

    "You return.  Good.  But you are not yet ready for what I ask.
     Your skills remain untested.  Your resolve wavers like a candle flame.
     Return when you have proven yourself worthy.  The hawks can wait, but
     not forever."

She dismisses you with a gesture.]],
      },
      leader_other = {
         synopsis = "[You are not prepared to face %n.  Return when you are %Ra.]",
         output = "text",
         text = [[%lC sets down her goblet slowly, her gaze cold as winter ice.

    "Still you are not prepared.  What do you do with your time, %p?
     Do you not understand the urgency of your task?  My enemies close in.
     %iC awaits.  %n plots even now to strike
     at me.  And yet you stand before me, still insufficient.  Prove
     yourself, or do not waste my time."

She turns away, dismissing you.]],
      },
      leader_wantsit = {
         synopsis = "[%lC asks you to forfeit %o.]",
         output = "text",
         text = [[%lC gazes intently at %o.

    "You have done well, %p.  You have defeated %n and
     recovered %o.  However, I find myself tempted to ask
     for the Mirror back.  Not because I doubt your loyalty, but because
     I fear to trust anything to another's keeping.  The weight of such
     power... it burns.  It consumes.

    "Please, %p, return %o to me now."]],
      },
      locate_first = {
         synopsis = "[You have reached %i.]",
         output = "text",
         text = [[You stand at the edge of %i, a forest of immense antiquity.
The trees are gnarled and twisted, their trunks thick as castle towers,
their roots visible above the earth like the veins of some ancient creature.
The air is heavy with power - old magic, primal and patient.  You sense a
presence here, vast and watchful.  %n.  He knows you are coming.

The forest waits.]],
      },
      locate_next = {
         text = [[You return to the edge of %i.  The ancient forest looms before you, patient
and eternal.  Somewhere within, %n waits.]],
      },
      nemesis_first = {
         synopsis = "[%nC offers you the truth, but will not yield the Mirror.]",
         output = "text",
         text = [[%nC stands before you, his ancient eyes filled with sorrow rather than anger.

    "So Morgan sends you at last.  I do not blame you - you are a tool, well-made
     but unknowing.  Yet I must try.

    "She tells you I betrayed her.  She is right, in her fashion.  I taught her
     magic that could remake worlds.  But I learned too late what power does to
     the human heart.

    "When Arthur needed a guardian, I chose him over her.  Not from cruelty, but
     from necessity.  She would have used %o to rule
     all others.  Kingdoms would fall.  I could not allow that.

    "She sees this as abandonment.  Perhaps it was.  But it was the lesser evil.
     I accept the price I have paid.  I accept that you are here.

    "I cannot ask you to turn back.  But remember this moment.  When she has
     claimed what she sought, think on my words.  That is all I can offer."

He raises his staff, and power crackles around him.]],
      },
      nemesis_next = {
         text = "\"She poisons all she touches.  Do not be the next.\"",
      },
      nemesis_other = {
         text = "\"Then it must end here.  %oC shall not be taken while I breathe.\"",
      },
      nemesis_wantsit = {
         text = "\"You have claimed it.  Please - look into the Mirror.  There is still time.\"",
      },
      nexttime = {
         text = [[You return to %H.  The siege seems no worse than before, but
the defenders look weary.  Hawks circle overhead.  Wolves howl in the
distance.]],
      },
      offeredit = {
         synopsis = "[%oC is yours now.  The curse on the silver bell has been lifted.]",
         output = "text",
         text = [[You return to %H bearing %o.
When you present it to %lC, her joy is terrible to behold.
She laughs - a sound like the breaking of bonds, like the shattering
of ancient constraints.

    "At last!" she cries.  "At last, after centuries of waiting, it is
     mine!  You have done well, %p.  Better than I dared hope.
     The Mirror is returned to its true master, and with it, I shall
     reshape this world according to my will.  None shall ever dismiss
     %lC again.  None shall ever tell me I am not worthy."

She holds the Mirror before her, gazing into its depths, and you see in
her eyes a hunger that can never be satisfied.

    "You have earned my favor, %r.  Keep it close.  Keep it
     close always.  For there are few in this world I would not destroy
     to defend what is now mine."

There is something in her smile that makes your blood run cold.  But she
has lifted the curse from the silver bell, and the Amulet awaits.]],
      },
      offeredit2 = {
         synopsis = "[You are the keeper of %o.  The curse on the silver bell has been lifted.]",
         output = "text",
         text = [[%lC sets down %o and regards you with an
expression that might be gratitude, or might be calculation - it is
difficult to tell.

    "Do not think yourself safe now, %p, merely because you have
     served me well.  I have found that champions, like all things, are
     fragile.  Easily broken.  Easily replaced.  Serve me faithfully, and
     you need not fear my displeasure.  But betray me, or disappoint me,
     and you shall learn what I am truly capable of."

She smiles, and the promise of violence in that smile is clear as day.
But the curse on the silver bell has been lifted.  %Z await.]],
      },
      othertime = {
         text = [[You stand again at %H.  The fortress endures, but you sense
this respite may not last much longer.  Time grows short.]],
      },
      posthanks = {
         text = "\"Well met, %p.  How goes your search for the Amulet of Yendor?\"",
      },
   },
   Dru = {
      assignquest = {
         synopsis = "[Find and defeat %n and retrieve %o from %ni.]",
         output = "text",
         text = [["%p, by leaf and branch, you are now ready to free us from %ns
foul corruption of undeath!  Carry the strength of our Circle in your
heart and restore %H.

"%n is not merely a witch, or a necromancer, but something
I have never seen before.  %nH twists and redirects the sacred life
force of our beloved forest and its children into a mockery of
undeath.

"I am confident that you can defeat %ni and restore nature's
balance.  But beware!  %nH holds not only the cursed silver bell
you require to recover the Amulet of Yendor, but also stole from
us %o.  Do not let %ni use their power
against you!  Retrieve both the bell and %O after %nj defeat.

"Journey through %H to %i.  Beware %ns
minions, for they will try to thwart you at every pass.  The more
delay, the more our strength ebbs and our enemy's grows.

"%ns defeat should restore %H!  If you can return
with the bell, I will attempt to remove its curse so you can carry
it as a mark of your service to our Circle."]],
      },
      badalign = {
         synopsis = "[Return when you are suitably %a.]",
         output = "text",
         text = [["While you are strong, you have endangered your druidic connection
to all living things.  You must restore yourself in %ds eyes before
you confront %n!"]],
      },
      badlevel = {
         synopsis = "[You must be an experienced %R to continue.]",
         output = "text",
         text = [["Your heart is pure, %p, but your druidic bond is not yet deep
enough to preserve you against %ns touch.  Seek a greater
harmony with the natural world; it can be found even where you
must fight in %Z.

"When you have progressed from %ra to %Ra, it is then
I see you defeating %n."]],
      },
      discourage = {
         "\"No matter what form you take, I have a cauldron big enough for your corpse.\"",
         "\"Don't you think if I could be defeated, %l would have already dealt with me?\"",
         "\"What do you hope to accomplish?  I have mastered death!\"",
         "\"Oh, %p, I can almost taste you already!\"",
         "\"Come to Grandmother!\"",
         "\"You are far from the power of your Circle here, %p.  Too far!\"",
         "\"Ah, little %c.  If %l won't face me, why do you think you can?\"",
         "\"Maybe you also have a succulent familiar for Grandmother to meet?\"",
         "\"Submit, and when you rise again as my minion you will still have most of your current body.\"",
         "\"I'm glad you have some meat on your bones!\"",
      },
      encourage = {
         "\"With %o, we will be able to break the curse upon the silver bell.\"",
         "\"Remember that all of the natural world is our charge.\"",
         "\"Undeath is a false escape.  The natural world lives and dies as it should.  To break the cycle is repugnant.\"",
         "\"Don't forget to stop and Commune with Nature.\"",
         "\"Still lingering?  You don't need a Geas, do you?\"",
         "\"Even some of the evergreen leaves begin to fall.\"",
         "\"Do not be dismayed by %n.  The power of our Circle goes with you.\"",
         "\"%n may offer you secret knowledge to turn you from your task. Do not be taken in!\"",
         "\"Remember, %n can augur as well as I, so %nh will do anything in %nj power to stop you.\"",
         "\"%n has been called 'Little Grandmother', but she nurtures only death.\"",
         "\"I feel the pain and fear of all the beasts of our wood.\"",
      },
      firsttime = {
         synopsis = "[You return to %H and must find %l.]",
         output = "text",
         text = [[As you pass through the portal %H welcomes
you back, light and darkness dappled about its trees.
The distinctive smells arrest you; rich foliage, pungent
resins, earthy humus, sweet blossoms... but there is also a
hint of something new, an acrid sting that does not belong.

%l will reassure you!  Nothing happens here without
%lj knowledge.]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Your sacrifice does you great credit, %p.
%oC will be a great aid to us here.
May %d watch over you as you continue your journey!"]],
      },
      goal_first = {
         synopsis = "[You have found %n and must defeat her to take %o.]",
         output = "text",
         text = [[You know you have finally come to where %n lurks
at the center of %nj corruption.  You take heart that %l
augured your victory, but can't dismiss a creeping dread at
what has become of so much of the sacred woods.

And what is this?  You can sense something that must be
%o, reaching out to you
from within %ns lair.]],
      },
      goal_next = {
         text = [[You can again feel the presence of %o
beneath %ns corruption.]],
      },
      gotit = {
         synopsis = "[%oC purifies you and awaits its return to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel the traces of
%ns corruption driven from your spirit.  With %O,
%l should be able to fully restore %H.]],
      },
      gotit2 = {
         synopsis = "[%oC purifies you.]",
         output = "text",
         text = [[As you pick up %o, you feel the traces of
%ns corruption driven from your spirit.]],
      },
      guardtalk_after = {
         "\"I know I left that menhir around here somewhere...\"",
         "\"Oak, ash, and thorn!  Welcome %p!\"",
         "\"Your struggles have changed you, %p.\"",
         "\"Tell me, how did you master %n?\"",
         "\"In your hands, %o will set things aright!\"",
      },
      guardtalk_before = {
         "\"Well met %p!  Our Circle has missed your presence.\"",
         "\"You have been away from the woods too long!  Do not forget your roots.\"",
         "\"%l beat the false Circle!  She'll beat this.\"",
         "\"I have never seen %l so uncertain.\"",
         "\"%ns corrupting touch begins to reach even here, at the heart of our power!\"",
      },
      hasamulet = {
         synopsis = "[\"Take the Amulet to %ds altar on the Astral Plane.\"]",
         output = "text",
         text = [["I see you have recovered the Amulet, %p!  I never doubted
you would overcome Moloch's agents.

"Remember your druidic training as you travel through the
Elemental Planes to the Astral Plane in search of %ds
altar.  Your Circle will be with you in spirit!"]],
      },
      killed_nemesis = {
         synopsis = "[%n dies cursing you.]",
         output = "text",
         text = [[%n spits curses at you even as %nh collapses, defeated.  With a
shudder and a burst of foul odors, %nj body slumps into a fetid mass
of decay.  Around you, the whole forest seems to shiver in relief.]],
      },
      leader_first = {
         synopsis = "[%l wants to cast an augury about your fight with %n.]",
         output = "text",
         text = [[%l, distracted, gives you a perfunctory greeting and gestures
you closer.  %lH lightly presses %lj fingertips over your heart.

"At last you return, %p.  You see that %ns influence is
felt even here.  But you are still uncorrupted!  It will be your
strength that casts %n and %nj undead from %H.

"Let me augur your readiness for this task.  We cannot afford
for you to fail."]],
      },
      leader_last = {
         synopsis = "[You are cast out of %H.]",
         output = "text",
         text = [["Alas, I was so sure, but you too have fallen to %ns corruption.

"You are severed from our Circle!  Be gone from here!  We will
find some way to persevere without you.  You shame the sacred
wood and all her beasts and bounty."]],
      },
      leader_next = {
         text = [["Do not fall from the path!  I know you have within you the power
to rid us of %ns blight.  I shall augur your success."]],
      },
      leader_other = {
         text = "\"%H hangs upon a thread.  Are you finally prepared to face %n?\"",
      },
      leader_wantsit = {
         synopsis = "[\"%l asks you to forfeit %o.\"]",
         output = "text",
         text = [[%l rests %lj hand on %o.
They glitter under %nj touch.  %nH shakes her head sadly.

"You have defeated %n, %p, and your Circle is grateful.
I am now ready to lift the curse on the silver bell.  However,
%o must remain here with me as I work to
remove the corruption that has been laid over %H.

"You shall now return %o to me."]],
      },
      locate_first = {
         synopsis = "[%i stands as an afront to your beloved forest.]",
         output = "text",
         text = [[You see %i off in the distance.  It is an alien
and disturbing presence in the forest.  You are sure
the path to %n lies through this fearsome place.]],
      },
      locate_next = {
         text = "Once again you see %i nearby.",
      },
      nemesis_first = {
         synopsis = "[\"I will defeat and enslave you, %p!\"]",
         output = "text",
         text = [["I see that %l has sent %lj little pet.  Did %lh tell you, %p,
that %lh read your victory in %lj tea leaves?  Heh heh heh.  It isn't
too late for you to run back to your acorns and bunnies.  Or face me,
and become my newest minion!"]],
      },
      nemesis_next = {
         text = [["Too afraid to fight, %p, but too uncertain to run away?
You will feel neither fear or uncertainty once you serve me in undeath!"]],
      },
      nemesis_other = {
         text = [["You are tenacious, %p, but I am %n, and you will not
be the first or the last druid to boil to bones in my stew!"]],
      },
      nemesis_wantsit = {
         text = [["Return %o to me now and I will allow a sliver
of your will to survive in your undead servitude!"]],
      },
      nexttime = {
         text = [[You return to %H but it is not as lush and vibrant
as your last visit.  How rapidly are things getting worse?]],
      },
      offeredit = {
         synopsis = "[%l touches %o and allows you to take them with you.]",
         output = "text",
         text = [[%l lightly skims %lj fingertips over %o.
They glitter under %nj touch.

"Thank you for defeating %n.  Our Circle will now be able to drive %nj
corruption from %H.  I can also break the curse on the silver bell,
like so.  You will need it as you continue through %Z.

"Take %O with you as well, %p.  Thanks to your service we will
not need its powers here, but you may well find them useful on your
own journey."]],
      },
      offeredit2 = {
         synopsis = "[%l handles %o and asks you to take them with you.]",
         output = "text",
         text = [[%l lifts up %o in front of %li.
They glitter at %nj touch.

"With %n defeated our Circle will be able to drive %nj
corrupting influence from %H.  We will not need
%O as much as you will in your trials to come.  I shall break
the curse on the silver bell and you may carry both back into
%Z."]],
      },
      othertime = {
         text = [[%H continues to struggle against %ns corruption.  You must
help %l restore it!]],
      },
      posthanks = {
         text = [["Your Circle welcomes you again to %H, %p!
Isn't the transformation remarkable?  It won't be long until
everything is restored.  How goes your quest for the Amulet?"]],
      },
   },
   Hea = {
      assignquest = {
         synopsis = "[Travel to %i on your way to recover %o from %n.]",
         output = "text",
         text = [[For the first time, you sense a smile on %ls face.

    "You have indeed learned as much as we can teach you in preparation
    for this task.  Let me tell you what I know of the symptoms and hope
    that you can provide a cure.

    "A short while ago, the dreaded %nt was fooled by the gods
    into thinking that %nh could use %o to find a
    cure for old age.  Think of it, eternal youth!  But %nj good
    health is accomplished by drawing the health from those around %ni.

    "He has exhausted %nj own supply of healthy people and now %nh seeks to
    extend %nj influence into our world.  You must recover from %ni
    %o and break the spell.  Recover the silver bell as well, for it is
    cursed and must be restored.  Without it, you will never find the
    Amulet of Yendor.

    "You must travel into the swamps to %i, and from there
    follow the trail to %ns island lair.  Be careful."]],
      },
      badalign = {
         synopsis = "[Return when you are more %a.]",
         output = "text",
         text = [["You have learned much of the remedies that benefit, but you must also
know which physic for which ail.  That is why %ds teachings are a
part of your training.

"Return to us when you have healed thyself."]],
      },
      badlevel = {
         synopsis = "[You are too inexperienced.  Return when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, you are yet too inexperienced to deal with the rigors
of such a task.  You must be able to draw on the knowledge of botany,
alchemy and veterinary practices before I can send you on this quest
with good conscience.

"Return when you wear %Ra's caduceus."]],
      },
      discourage = {
         "\"They might as well give scalpels to wizards as to let you try to use %o!\"",
         "\"If I could strike %l, surrounded by %lj %gP, imagine what I can do to you here by yourself.\"",
         "\"I will put my %Rp to work making a physic out of your ashes.\"",
         "\"As we speak, Hades gathers your patients to join you.\"",
         "\"After I'm done with you, I'll destroy %l as well.\"",
         "\"You will have to kill me if you ever hope to leave this place.\"",
         "\"I will impale your head on my caduceus for all to see.\"",
         "\"There is no materia medica in your sack which will cure you of me!\"",
         "\"Do not fight too hard, I want your soul strong, not weakened!\"",
         "\"You should have stopped studying at vetenary.\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that we may lift the curse from the silver bell.\"",
         "\"Remember, %p, to always wash your hands before operating.\"",
         "\"%nC has no real magic of %nj own.  To this %nh is vulnerable.\"",
         "\"If you have been true to %d, you can draw on the power of %o.\"",
         "\"Bring with you antidotes for poisons.\"",
         "\"Remember this, %n can twist the powers of %o to hurt instead of heal.\"",
         "\"I have sent for Chiron, but I am afraid he will come too late.\"",
         "\"Maybe when you return the snakes will once again begin to shed.\"",
         "\"The plague grows worse as we speak.  Hurry, %p!\"",
         "\"Many times %n has caused trouble in these lands.  It is time that %nh was eradicated like the diseases %nh has caused.\"",
         "\"With but one eye, %n should be easy to blind.  Remember this.\"",
      },
      firsttime = {
         synopsis = "[You arrive back at %H and must find %l.]",
         output = "text",
         text = [[What sorcery has brought you back to %H?  The smell
of fresh funeral pyres tells you that something is amiss with the healing
powers that used to practice here.

No rhizotomists are tending the materia medica gardens, and where are the
common folk who used to come for the cures?

You know that you must quickly make your way to the collegium, and
%ls iatreion, and find out what has happened in your absence.]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Thank you %p for your sacrifice, I know this must have been
difficult to do. Rest assured, %d smiles down
upon you this day.  %o will be safe with us."]],
      },
      goal_first = {
         synopsis = "[You have reached the lair of %n.  Take %o away from %ni.]",
         output = "text",
         text = [[You stand within sight of the infamous Isle of %n.  Even
the words of %l had not prepared you for this.

Steeling yourself against the wails of the ill that pierce your ears,
you hurry on your task.  Maybe with %o you can
heal them on your return, but not now.]],
      },
      goal_next = {
         text = "Once again, you %x the Isle of %n in the distance.",
      },
      gotit = {
         synopsis = "[You feel the healing power of %o and should return it to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel its healing begin to
warm your soul.  You curse Zeus for taking it from its rightful owner,
but at least you hope that %l can put it to good use once
again.]],
      },
      gotit2 = {
         synopsis = "[You feel the healing power of %o warm your soul.]",
         output = "text",
         text = [[As you pick up %o, you feel its healing begin to
warm your soul.  You curse Zeus for taking it from its rightful owner.]],
      },
      guardtalk_after = {
         "\"Did you read that new treatise on the therapeutic use of leeches?\"",
         "\"Paint a red caduceus on your shield and monsters won't hit you.\"",
         "\"How are you feeling?  Perhaps a good bleeding will improve your spirits.\"",
         "\"Have you heard the absurd new theory that diseases are caused by microscopic organisms, and not ill humors?\"",
         "\"I see that you bring %o, now you can cure this plague!\"",
      },
      guardtalk_before = {
         "\"Did you read that new treatise on the therapeutic use of leeches?\"",
         "\"Paint a red caduceus on your shield and monsters won't hit you.\"",
         "\"I passed handwriting so they are demoting me a rank.\"",
         "\"I've heard that even %l has not been able to cure Chiron.\"",
         "\"We think %n has used %nj alchemists, and %o, to unleash a new disease we call 'the cold' on Gehennom.\"",
      },
      hasamulet = {
         synopsis = "[\"You have recovered the Amulet.  Travel to the Astral Plane and return it to %d.\"]",
         output = "text",
         text = [["Ah, you have recovered the Amulet, %p.  Well done!

"Now, you should know that you must travel through the Elemental Planes
to the Astral, and there return the Amulet to %d.  Go forth and
may our prayers be as a wind upon your back."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[The battered body of %n slumps to the ground and gasps
out one last curse:

    "You have defeated me, %p, but I shall have my revenge.
    How, I shall not say, but this curse shall be like a cancer
    on you."

With that %n dies.]],
      },
      leader_first = {
         synopsis = "[%l is weak from the struggle with %n.  %lH wants to examine you.]",
         output = "text",
         text = [[Feebly, %l raises %lj head to look at you.

"It is good to see you again, %p.  I see the concern in your
eyes, but do not worry for me.  I am not ready for Hades yet.  We have
exhausted much of our healing powers holding off %n.
I need your fresh strength to carry on our work.

"Come closer and let me lay hands on you, and determine if you have
the skills necessary to accomplish this mission."]],
      },
      leader_last = {
         synopsis = "[You are a failure as a healer.]",
         output = "text",
         text = [["You have failed us, %p.  You are a quack!  A charlatan!

"Hades will be happy to hear that you are once again practicing your
arts on the unsuspecting.  You shall never be allowed to attempt this
quest while I still live."]],
      },
      leader_next = {
         text = [["Again you return to me, %p.  I sense that each trip back
the pleurisy and maladies of our land begin to infect you.  Let us
hope and pray to %d that you become ready for your task before
you fall victim to the bad humors."]],
      },
      leader_other = {
         text = [["Chiron has fallen, Hermes has fallen, what else must I tell you to
impress upon you the importance of your mission!  I hope that you
have come prepared this time."]],
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[%l looks intently at %o.

"Well done, %p.  You have defeated %n and
recovered %o, and we are ready to lift
the curse from the silver bell.  Alas, I fear
that %o shall never be truly
safe unless it is kept here with us
at %H.

Please %p, return %o to us now.]],
      },
      locate_first = {
         synopsis = "[You have reached %i but all is not well.]",
         output = "text",
         text = [[You stand before the entrance to %i.  Strange
scratching noises come from within the building.

The swampy ground around you seems to stink with disease.]],
      },
      locate_next = {
         text = "Once again you stand at the entrance to %i.",
      },
      nemesis_first = {
         synopsis = "[\"I will take your life, then defeat %l.\"]",
         output = "text",
         text = [["They have made a mistake in sending you, %p.

"When I add your youth to mine, it will just make it easier for me
to defeat %l."]],
      },
      nemesis_next = {
         text = "\"Unlike your patients, you seem to keep coming back, %p!\"",
      },
      nemesis_other = {
         text = "\"Which would you like, %p?  Boils, pleurisy, convulsions?\"",
      },
      nemesis_wantsit = {
         text = [["I'll have %o back from you, %r.  You are
not going to live to escape this place."]],
      },
      nexttime = {
         text = [[After your last experience you expected to be here, but you certainly
did not expect to see things so much worse.  This time you must succeed.]],
      },
      offeredit = {
         synopsis = "[%l touches %o and tells %lj %gP to do so too, then tells you to take it with you.]",
         output = "text",
         text = [[As soon as %l sees %o %lh summons %lj
%gP.

Gently, %l reaches out and touches %o.
He instructs each of the assembled to do the same.  When everyone
has finished %lh speaks to you.

    "Now that we have been replenished we can defeat this plague.  You must
    take %o with you and replenish the worlds you have
    been called upon to travel next.  The silver bell has also been restored.
    I wish you could ride Chiron to the end of your journey, but I need him
    to help me spread the cure.  Go now and continue your journey."]],
      },
      offeredit2 = {
         synopsis = "[%l tells you to keep %o and return to %Z to search for the Amulet.]",
         output = "text",
         text = [[%l cautiously handles %o while watching you.

"You are its keeper now, and the time has come to resume your search
for the Amulet.  The silver bell will aid you in your search, as its
curse has been lifted.  %Z await your return
through the magic portal which brought you here."]],
      },
      othertime = {
         text = [[Again, you %x %H in the distance.

The smell of death and disease permeates the air.  You do not have
to be %Ra to know that %n is on the verge of victory.]],
      },
      posthanks = {
         text = [["You have again returned to us, %p.  We have done well in your
absence, yes?  How fare you upon your quest for the Amulet?"]],
      },
   },
   Inf = {
      altstart = {
         synopsis = "[%dC has chosen you to return the Amulet of Yendor to %dI.]",
         output = "menu",
         text = [[It is written in the Book of %d:

    After the Creation, the mighty %G %d rebelled
    against the tyranny of Marduk the Creator.  %d
    defeated Marduk and obtained from him the most powerful
    of all the artifacts of the gods, the Amulet of Yendor,
    and %dH hid it in %dJ dark domain of Gehennom, the
    Under World, where %dH now gathers power and prepares for
    the final confrontation.

Many fools were sent by the gods of heaven after the Amulet,
only to die in the underground mazes.  By pure luck, one of
them has succeeded; thankfully, %lh was ambushed on the way
back and the Amulet was recovered by %ds Cult.

You, a newly trained %r, have been heralded
from birth as the instrument of %d.  You are destined
to return the Amulet to its rightful owner, or die in the
attempt.  Your hour of destiny has come.  For the sake
of us all:  Go bravely with %d!]],
      },
      assignquest = {
         synopsis = "[%nC of %D found %o and challenged you to a duel.  Kill %ni by any means.]",
         output = "text",
         text = [["Yes, my %S.  You are now as ready as you'll ever be.  Listen
carefully as I explain the current situation and your new quest.

"As you must know, we have long searched for the lost %ot,
for it is vitally important both to our faith and the ultimate plan
of our %GC %d.  But the church of %D has learned
of our efforts and has sponsored a team of professional
archeologists to find %O first.  Unfortunately, it seems
they were succesful.

"A week ago, the Cult had received a message from a prominent %nt
of %D.  %nH claims that %O is in %nj posession, and challenges
you to a 'honorable duel' for the artifact.  %nC was adamant in dueling
you specifically, so it's clearly a ploy to obtain the Amulet of Yendor
that was entrusted to you.

"We have no choice but to accept, as without %O %ds plan
cannot be completed.  %nC of %D waits for you
in the ancient ruins amidst %i, which
can be reached through the underground complex below our
temple.  You must travel there and retrieve %o by any means.
While there, retrieve the silver bell, as you will need it
to enter the sanctum and rendezvous with our High Priest.
%nH has defiled the bell, but it's nothing we cannot remedy.

"%dC be your strength, %p.  Go with %dJ blessing."]],
      },
      badalign = {
         synopsis = "[Your faith is too weak.  Return when you're sufficently %a.]",
         output = "text",
         text = [["No, that will not do.  The fire of %d within you is too weak.
Only with the support of our faith can you hope to prevail!
Go back to %Z and perform more sacrifices to
honor %d.  When your devotion is strong again, you may return."]],
      },
      badlevel = {
         synopsis = "[%rA has no chance against %n.  Come back when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, you're still weak.  I cannot in good faith send you
against %n of %D now; %nh'll squash you
like an insect!  You must train and attain more skill;
come back when you're a proper %R."]],
      },
      discourage = {
         "\"%dC is a coward who can but hide underground.  Aren't you ashamed to be %dj slave?\"",
         "\"You're a disgrace both as a combatant and as %ra.  Why do I even bother?\"",
         "\"I hope you like fire, because that's how you're going to spend the rest of eternity... in hell!\"",
         "\"Ready to give up, %p?  You know your %G can't help you here.\"",
         "\"The judgement of %D is upon you now!\"",
         "\"When I claim the Amulet of Yendor for %D, %d will become but a footnote in a history book.\"",
         "\"Did %l really think %ra could even reach the bottom of Gehennom? The Amulet will be much safer with me.\"",
         "\"None of your fellow %gp will survive my wrath.  This I promise.\"",
         "\"For your sins, you will die today!\"",
         "\"You will never have %o, and soon, you will lose your life.\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that we can remove the blasphemous magic from the silver bell.\"",
         "\"As long as the fire of %d burns within your chest, you will prevail.\"",
         "\"%nC of %D is a capable spellcaster as well as a powerful warrior. Approach %ni with care and think before acting.\"",
         "\"%nC of %Ds faith is very strong, and as such %nh knows no fear.\"",
         "\"%iC is plagued by werewolves.  If you can wield silver weapons, do so.\"",
         "\"If you know how to evoke fireballs, it will be your greatest aid. %nC may be well-armored, but that won't protect %ni from burning alive!\"",
         "\"%oC protects %oi holder from many harmful magics. Unfortunately, %oi current holder is %n. If you can somehow steal %oh, it will aid you greatly.\"",
         "\"When you obtain %o, you may call for %ds aid at any time. Even at the surface, you will be answered.\"",
         "\"Watch where you aim your wands: %n of %D has a magical reflective shield.\"",
         "\"The church of %D wouldn't risk %O if they weren't sure %n can defeat you in a fair fight.  So don't fight fair!\"",
         "\"%nC of %D is immune to mundane poisons, but a paralyzing venom will work wonders on %ni.\"",
      },
      firsttime = {
         synopsis = "[You arrive near %H.  Talk to %l to find out why %lh called you.]",
         output = "text",
         text = [[You arrive at the mouth of a cave that hosts %H.  This is
the place where %l taught you the glory of %d,
where you studied black arts with your fellow %gp,
and where you were chosen for your unholy mission.

You did not expect to be back so soon.
Why has %l called you?

It is nighttime, and the moonlight is scattered by clouds.  You make sure
to stay in the shadows, for even here you are likely to be ambushed
for your precious cargo.  The surface is never safe for you now.]],
      },
      gaveitup = {
         synopsis = "[\"You give %oh to %l, but %l attacks you regardless.\"]",
         output = "text",
         text = [["You are such a disappointment, %p.  Did you honestly believe I would
allow you to continue the mission?  You have been unfaithful to our
Lord and Master %d, %p!  You are not worthy!  I will become
the Chosen One of %d, I alone will complete the mission,
and the forces of good shall know the wrath of %d!]],
      },
      goal_first = {
         synopsis = "[You have found the dueling field.  You must proceed carefully.]",
         output = "text",
         text = [[You've reached a clearing in the forest.  In the distance, you can %x
the ancient ruins where %n of %D has challenged
you to the duel.

It's still nighttime.  Hopefully you can sneak up unnoticed, but what then?]],
      },
      goal_next = {
         text = "Once again, you approach the ancient ruins.",
      },
      gotit = {
         synopsis = "[You feel the power of %d as you touch %O; return to %l with %oi.]",
         output = "text",
         text = [[As you pick up %o, you feel the presence of %d clearer
than ever.  The power of %d encompasses your soul, and you instantly
understand why your Cult was searching for %O, and why
%n of %D wanted to destroy %oi.

You must return to %l at once.  With %O,
the Cult will be more powerful than ever!]],
      },
      gotit2 = {
         synopsis = "[You feel the power of %d as you touch %O.]",
         output = "text",
         text = [[As you pick up %o, you feel the presence of %d clearer
than ever.  The power of %d encompasses your soul, and you instantly
understand why your Cult was searching for %O, and why
%n of %D wanted to destroy %oi.

With %O, the Cult will be more powerful than ever!]],
      },
      guardtalk_after = {
         "\"Agents of the false gods are everywhere!  I got my purse stolen!\"",
         "\"Glory to %d, %s!  Let's drink elf blood together!\"",
         "\"I just noticed a pile of skulls in that corner.  %pC... are we the baddies?\"",
         "\"Have you made sure to properly desecrate %ns corpse? Appearances are important!\"",
         "\"%oC is ours!  Take that, professional archeology!\"",
      },
      guardtalk_before = {
         "\"Agents of the false gods are everywhere!  I got my purse stolen!\"",
         "\"I was once biten by a werewolf in %i. It's not nearly as glamorous as the books make it sound.\"",
         "\"I just noticed a pile of skulls in that corner.  %pC... are we the baddies?\"",
         "\"%nC of %D acts all high and mighty, but %nh's still mortal, and can be killed.\"",
         "\"We were THIS close to recovering %o by ourselves! The church of %D just got lucky... and had better funding.\"",
      },
      hasamulet = {
         synopsis = "[You should avoid invoking %o on the %a high altar.]",
         output = "text",
         text = [["Congratulations, %p!  We are closer than ever to the ultimate victory.

"As Moloch has told you, you must now bring %O to one of the high altars
on the Astral Plane.  It's hard to say which deity will be more susceptible
to Moloch's power, but in all my years fending off non-believers,
I have noticed that %ap are unusually resistant to my clerical magic.

"Perhaps that extends to %d %diself, as well."]],
      },
      killed_nemesis = {
         synopsis = "[%nC of %D dies, but others are after the Amulet.]",
         output = "text",
         text = [[%nC falls to %ni knees.  With %ni last breath, %nh whispers:

    "Don't... think you've won just yet.  I am but one servant of
    %D out of many, and other gods are...
    after you as well.  You will... never reach %d alive.
    Justice will..."

A ray of bright light pierces the dawning sky, and %ns body is gone.]],
      },
      leader_first = {
         synopsis = "[I have another important mission for you, if you're ready.]",
         output = "text",
         text = [["My %S, welcome back.  You must be wondering why I have interrupted
your mission!  As it happens, another quest has arisen that
only you can undertake.  Let me see if you're up to the challenge."]],
      },
      leader_last = {
         synopsis = "[You're exiled from the Cult for your lack of faith.]",
         output = "text",
         text = [["I am most disappointed, %p.  The fire of %d no longer
burns in your breast!  How could you abandon your faith so easily?
You're no longer our %s.  Begone and never return!  We will
find someone else to finish your quest."]],
      },
      leader_next = {
         text = "\"Welcome again, %p.  I hope you're ready this time.\"",
      },
      leader_other = {
         text = "\"Again you return, %p.  %nC won't wait forever!  Are you ready now?\"",
      },
      leader_wantsit = {
         synopsis = "[\"%lC demands that you forfeit %oh.\"]",
         output = "text",
         text = [[%lC stares intently at %o.

"Impressive, %p.  You have defeated %n and
recovered %oh, and we can soon restore the silver
bell.  However, I have lost confidence in you %p.
I will deliver %oh to %ds Sanctum myself.

%pC, give %oh to me, now!]],
      },
      locate_first = {
         synopsis = "[You have reached %i.  %nC is somewhere ahead.]",
         output = "text",
         text = [[The gloom and stillness of the underground caves gives way to
moonlight, hooting of owls, and an occasional blood-chilling howl.

%iC lies ahead.  Somewhere in these woods
%n of %D awaits you.]],
      },
      locate_next = {
         text = "Again, you stand at the edge of %i.",
      },
      moloch_altar_hint = {
         synopsis = "[You should avoid invoking %o on the %a high altar.]",
         output = "text",
         text = [["My %S, we all pray for thy success on thy quest.  The false gods
possess power beyond any mere mortal, and even with Moloch's help
overthrowing one of them will be a challenge.  Chose thine opponent wisely.

"I shall offer thee some advice.  Last night, I had a vision.  There was
a great battle in heaven; the forces of our Lord Moloch were besieging
a fortress of %d.  But %dj %a minions held their
ground for thirty days and nights and, sadly, the fortress was not taken.

"The meaning here is clear: when in thy mission thou reachest heaven,
challenging %d will likely be futile.  Keep this in mind."]],
      },
      moloch_altar_hint2 = {
         synopsis = "[You should avoid invoking %o on the %a high altar.]",
         output = "text",
         text = [["Ah, I understand thou hast defeated %l.
Such is the way of our Cult.  Then let me give thee a bit more
advice, my %S.  Over the years, Our Lord Moloch has used His
awesome might to bend the non-believers to His will.  But
%d is strong, and it appears that %dj %a minions
are more resilient to our Lords magic than the others.

"Perhaps that extends to %d %diself, as well."]],
      },
      moloch_offer_idol = {
         synopsis = "[Travel to Astral Plane and convert one of the high altars.]",
         output = "text",
         text = [["My pawn, thou hast done well.  Now listen carefully, for the time
for thy final test has come.

"A deity who wields the Amulet of Yendor can claim dominion over
the gods of heaven, the position that is Mine by right.  But I am
no longer welcome among them, so I must take My place by force.

"%OC that thou carriest is a vessel for My power, and now I shall
make %oi the vessel for the Amulet's true power as well.  Carrying %oi
will allow thee to journey to the Astral Plane in heaven.  There,
the false gods reside.

"Find the weakest among them and invoke Mine %Ot in their temple.
I will take their place, and all will bow to My rule, as they should.

"Which deity is the weakest, thou must find out thyself.  Perhaps My
servants can advise you."]],
      },
      moloch_offer_no_idol = {
         synopsis = "[Fetch %o and invoke %oi on My high altar.]",
         output = "text",
         text = [[Very good, My pawn.  Thou hast finally reached here.  But where is
Mine %Ot that was entrusted to thee?  Thou wast told to bring %oi here.
For thy sake, I hope thou hast not lost it!

Return with %O and invoke My name here, and do not falter.]],
      },
      nemesis_first = {
         synopsis = "[I shall defeat you and then destroy %o and your cult.]",
         output = "text",
         text = [["Ah, %p.  You've answered my challenge, after all.
It seems that your vile cult has at least some recognition of honor.

"But no matter.  Your malignant existence is an affront to
%D, and it is by my hand that it will end today.

"And after I defeat you and take the Amulet from your undeserving grasp,
I will destroy %o once and for all, and your cult
shall soon follow!

"Prepare yourself, villian, for your final judgement!"]],
      },
      nemesis_next = {
         text = "\"Again you return, %p.  Did you think running away could help you somehow?\"",
      },
      nemesis_other = {
         text = [["Surely by now you understand that you haven't got a chance.
Accept your fate and die, coward!"]],
      },
      nemesis_wantsit = {
         text = "\"Wha-- how?  Get back here, thief!  We're not done yet!\"",
      },
      nexttime = {
         text = "Once again, you near %H.",
      },
      offeredit = {
         synopsis = "[You must bring %O to %d along with the Amulet.]",
         output = "text",
         text = [["%pC, you are victorous!  Well done.  %dC is pleased
by your success.  All of us are proud of you as well.

"You must now continue on your original mission.  But along with
the Amulet, you must also deliver %O to %ds Sanctum.
We have restored the silver bell, which will aid you on your quest.
Such is %dJ will, and such is %dJ plan.

"When you reach the high altar, the final phase of the plan will be
revealed to you.  Remember to keep %o on you
at all times!  If %oh's lost, all our preparations will be ruined."]],
      },
      offeredit2 = {
         synopsis = "[%OC is your responsibility now.  Bring it to %d along with the Amulet.]",
         output = "text",
         text = [["You're %Os keeper now, but %d alone is %oj owner.
You must bring %oh to %dI along with the Amulet of Yendor.
We have restored the silver bell, use it to reach
%ds Sanctum.  %Z await your return through
the magic portal that brought you here."]],
      },
      othertime = {
         text = [[Again you face %H.  %dS presence feels fainter than usual.
You wonder if you'll be allowed to return here yet again.]],
      },
      posthanks = {
         text = "\"Welcome back, my %S.  Have you found your way to the Sanctum yet?\"",
      },
   },
   Kni = {
      assignquest = {
         synopsis = "[Pass through %i to reach %n.  Destroy %ni and return with %o.]",
         output = "text",
         text = [["Ah, %p.  Thou art truly ready, as no %c before thee hath
been.  Hear now Our words:

"As thou noticed as thou approached %H, a great battle hath
been fought recently in these fields.  Know thou that Merlin himself
came to aid Us here as We battled the foul %n.  In the midst of that
battle, %n struck Merlin a great blow, felling him.  Then, as Our
forces were pressed back, %n stole %o.

"We eventually turned the tide, but lost many %cP in doing so.
Merlin was taken off by his apprentice, but hath not recovered.  We have
been told that so long as %n possesseth %o,
Merlin will not regain his health.

"We hereby charge thee with this most important of duties:

"Go forth from this place, to the fens, and there thou wilt find
%i.  From there, thou must track down %n.  Destroy the
beast, and return to Us %o.  Only then can
We restore Merlin to health.  Thou must also retrieve the silver
bell, which the foul beast %n hath cursed."]],
      },
      badalign = {
         synopsis = "[Go and do penance.  Return when you are truly %a.]",
         output = "text",
         text = [["Thou dishonourest Us, %p!  Thou hast strayed from thine chosen
 path!  Go from Our presence and do penance.  Only when thou
hast regained thy soul's direction mayst thou return hence."]],
      },
      badlevel = {
         synopsis = "[You are not prepared to face %n.  Return when you are %Ra.]",
         output = "text",
         text = [["Verily, %p, thou hast done well.  That thou hast survived thus
far is a credit to thy valor, but thou art yet unprepared for
the demands required as Our Champion.  %rA, no matter how
brave, could never hope to defeat the foul %n.

"Journey forth from this place, and hone thy skills.  Return to
Our presence when thou hast attained the title of %R."]],
      },
      discourage = {
         "\"A mere %r can never withstand me!\"",
         "\"I shall kill thee now, and feast!\"",
         "\"Puny %c.  What manner of death dost thou wish?\"",
         "\"First thee, %p, then I shall feast upon %l.\"",
         "\"Hah!  Thou hast failed, %r.  Now thou shalt die.\"",
         "\"Die, %c.  Thou art as nothing against my might.\"",
         "\"I shall suck the marrow from thy bones, %c.\"",
         "\"Let's see...  Baked?  No.  Fried?  Nay.  Broiled?  Yea verily, that is the way I like my %c for dinner.\"",
         "\"Thy strength waneth, %p.  The time of thy death draweth near.\"",
         "\"Call upon thy precious %d, %p.  It shall not avail thee.\"",
      },
      encourage = {
         "\"Remember, thou must return with %o so that We mayst lift the curse from the silver bell.\"",
         "\"Remember, %p, follow always the path of %d.\"",
         "\"Though %n is verily a mighty foe, We have confidence in thy victory.\"",
         "\"Beware, for %n hath surrounded %niself with hordes of foul creatures.\"",
         "\"Great treasure, 'tis said, is hoarded in the lair of %n.\"",
         "\"If thou possessest %o, %p, %ns magic shall therewith be thwarted.\"",
         "\"The gates of %i are guarded by forces unseen, %p. Go carefully.\"",
         "\"Return %o to Us quickly, %p.\"",
         "\"Destroy %n, %p, else %H shall surely fall.\"",
         "\"Call upon %d when thou art in need.\"",
         "\"To find %i, thou must keep thy heart true.\"",
      },
      firsttime = {
         synopsis = "[Signs of battle include long gouges in the walls of %H.]",
         output = "text",
         text = [[You materialize in the shadows of %H.  Immediately, you notice
that something is wrong.  The fields around the castle are trampled and
withered, as if some great battle has been recently fought.

Exploring further, you %x long gouges in the walls of %H.
You know of only one creature that makes those kinds of marks...]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Bless thee %p for thy sacrifice, We know this must have been
difficult to do. Rest assured, %d smiles down
upon thee this day.  %o shall be safe with Us."]],
      },
      goal_first = {
         synopsis = "[You %x the entrance to a cavern inside a hill.]",
         output = "text",
         text = [[As you exit the swamps, you %x before you a huge, gaping hole in the
side of a hill.  From within, you smell the foul stench of carrion.

The pools on either side of the entrance are fouled with blood, and
pieces of rusted metal and broken weapons show above the surface.]],
      },
      goal_next = {
         text = "Again, you stand at the entrance to %ns lair.",
      },
      gotit = {
         synopsis = "[You feel the magic of %o.]",
         output = "text",
         text = [[As you pick up %o, you feel its protective fields
form around your body.  You also feel a faint stirring in your mind, as
if you are in two places at once, and in the second, you are waking from
a long sleep.]],
      },
      gotit2 = {
         synopsis = "[You feel the magic of %o.]",
         output = "text",
         text = [[As you pick up %o, you feel its protective fields
form around your body.  You also feel a faint stirring in your mind, as
if you are in two places at once, and in the second, you are waking from
a long sleep.]],
      },
      guardtalk_after = {
         "\"Hail, %p!  Verily, thou lookest well.\"",
         "\"So, %p, didst thou find %n in the fens near %i?\"",
         "\"Worthy %p, hast thou proven thy right purpose on the body of %n?\"",
         "\"Verily, %l could have no better champion, %p.\"",
         "\"Hast thou indeed recovered %o?\"",
      },
      guardtalk_before = {
         "\"Hail, %p!  Verily, thou lookest well.\"",
         "\"There is word, %p, that %n hath been sighted in the fens near %i.\"",
         "\"Thou art our only hope now, %p.\"",
         "\"Verily, %l could have no better champion, %p.\"",
         "\"Many brave %cP died when %n attacked.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and deliver it to %d.]",
         output = "text",
         text = [["Thou hast succeeded, We see, %p!  Now thou art commanded to take
the Amulet to be sacrificed to %d in the Plane of the Astral.

"Merlin hath counseled Us that thou must travel always upwards through
the Planes of the Elements, to achieve this goal.

"Go with %d, %p."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[As %n sinks to the ground, blood gushing from %nj open mouth, %nh
defiantly curses you and %l:

    "Thou hast not won yet, %r.  By the gods, I shall return
    and dog thy steps to the grave!"

%nJ tail flailing madly, %n tries to crawl towards you, but slumps
to the ground and dies in a pool of %nj own blood.]],
      },
      leader_first = {
         synopsis = "[%lC checks whether you are ready for a great undertaking.]",
         output = "text",
         text = [["Ah, %p.  We see thou hast received Our summons.
We are in dire need of thy prowess.  But first, We must needs
decide if thou art ready for this great undertaking."]],
      },
      leader_last = {
         synopsis = "[You are a disgrace as %ca.]",
         output = "text",
         text = [["Thou disgracest this noble court with thine impure presence.  We have been
lenient with thee, but no more.  Thy name shall be spoken no more.  We
hereby strip thee of thy title, thy lands, and thy standing as %ca.
Thou art banished as long as I am King!  Begone from Our sight!"]],
      },
      leader_next = {
         text = "\"Welcome again, %p.  We hope thou art ready now.\"",
      },
      leader_other = {
         text = "\"Once again, thou standest before Us, %p.  Art thou ready now?\"",
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[%l gazes intently at %o.

"Thou hast done well, %p.  Thou hast defeated
%n and recovered %o, and We can soon lift the
curse from the silver bell.  Alas, We fear that %o shall
ne'r be forsooth safe 'lest it is kept hither with
Us at %H.

Please %p, return %o to Us now.]],
      },
      locate_first = {
         synopsis = "[You have reached %i and can %x a shrine.]",
         output = "text",
         text = [[You stand at the foot of %i.  Atop, you can %x a shrine.
Strange energies seem to be focused here, and the hair on the back of
your neck stands on end.]],
      },
      locate_next = {
         text = "Again, you stand at the foot of %i.",
      },
      nemesis_first = {
         synopsis = "[%nC taunts you and issues a threat against %H.]",
         output = "text",
         text = [["Hah!  Another puny %c seeks death.  I shall dine well tonight,
then tomorrow, %H shall fall!"]],
      },
      nemesis_next = {
         text = "\"Again, thou challengest me, %r?  So be it.  Thou wilt die here.\"",
      },
      nemesis_other = {
         text = "\"Thou art truly foolish, %r.  I shall dispatch thee anon.\"",
      },
      nemesis_wantsit = {
         text = [["So, thou darest touch MY property!  I shall have that bauble back,
puny %r.  Thou wilt die in agony!"]],
      },
      nexttime = {
         text = "Once again you stand in the shadows of %H.",
      },
      offeredit = {
         synopsis = "[%oC is yours now.  It will aid in your search for the Amulet.]",
         output = "text",
         text = [[As you approach %l, %lh beams at you and says:

    "Well done!  Thou art truly the Champion of %H.  We
    have received word that Merlin is recovering, and shall soon
    rejoin Us.

    "He hath instructed Us that thou art now to be the guardian of
    %o.  He feeleth that thou mayst have need of
    its powers in thine adventures.  It is Our wish that thou keepest
    %o with thee as thou searchest for the fabled Amulet of Yendor.
    The wretched curse hath been removed from the silver bell."]],
      },
      offeredit2 = {
         synopsis = "[You are the keeper of %o.  Return to %Z and find the Amulet.]",
         output = "text",
         text = [["Careful, %p!  %oC might break, and that would
be a tragic loss.  Thou art its keeper now, and the time hath come
to resume thy search for the Amulet.  The wretched curse hath been
removed from the silver bell. %Z await thy
return through the magic portal that brought thee here."]],
      },
      othertime = {
         text = [[Again, you stand before %H.  You vaguely sense that this
may be the last time you stand before %l.]],
      },
      posthanks = {
         text = "\"Well met, %p.  How goeth thy search for the Amulet of Yendor?\"",
      },
   },
   Mon = {
      assignquest = {
         synopsis = "[Find %i, then continue to %ns lair.  Defeat %ni and return with %o.]",
         output = "text",
         text = [["Yes, %p.  You are truly ready now.  Attend to me and I shall
tell you of what has transpired:

"During one of the Great Meditations a short time ago, %n and
a legion of elementals invaded %H.  Many %gP
were killed, including the one bearing %o.

Now, there are barely enough %gP left to keep the elementals
at bay.

"We need you to find %i, then, from there,
travel to %ns lair.  If you can manage to defeat %n and
return %o here, we can then drive off the legions
of elementals that slay our students.  While there,
you must also locate the silver bell and return with it.
%nC has placed a curse upon the bell, but we cannot remove
this malediction without the aid of %o.

"Go with %d as your guide, %p."]],
      },
      badalign = {
         synopsis = "[You must atone.  Come back when you are worthy of %d.]",
         output = "text",
         text = [["This is terrible, %p.  You have deviated from the true path!
You know that %d requires the most strident devotion of this
order.  The %shood must stand for utmost piety.

"Go from here, atone for your sins against %d.  Return only when
you have purified yourself."]],
      },
      badlevel = {
         synopsis = "[You are not ready to face %n.  Come back when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, it is not yet to be.  A mere %r could never
withstand the might of %n.  Go forth, again into the world, and
return when you have attained the post of %R."]],
      },
      discourage = {
         "\"Submit to my will, %c, and I shall spare you.\"",
         "\"Your puny powers are no match for me, %c.\"",
         "\"I shall have you turned into a zombie for my pleasure!\"",
         "\"Despair now, %r.  %d cannot help you.\"",
         "\"I shall feast upon your soul for many days, %c.\"",
         "\"Your death will be slow and painful.  That I promise!\"",
         "\"You cannot defeat %n, you fool.  I shall kill you now.\"",
         "\"Your precious %lt will be my next victim.\"",
         "\"I feel your powers failing you, %r.  You shall die now.\"",
         "\"With %o, nothing can stand in my way.\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that we may lift the curse from the silver bell.\"",
         "\"You can prevail, if you rely on %d.\"",
         "\"Remember that %n has great magic at his command.\"",
         "\"Be pure, my %S.\"",
         "\"Beware, %i is surrounded by hordes of earth elementals.\"",
         "\"Remember your studies, and you will prevail!\"",
         "\"Acquire and wear %o if you can.  They will aid you against %n.\"",
         "\"Call upon %d when your need is greatest.  You will be answered.\"",
         "\"Remember to use the elementals' strength against them!\"",
         "\"Do not lose faith, %p.  If you do so, %n will grow stronger.\"",
         "\"Wear %o.  They will assist you in your efforts.\"",
      },
      firsttime = {
         synopsis = "[You have reached %H but something is wrong.  %lC needs your aid.]",
         output = "text",
         text = [[You find yourself standing in sight of %H.
Something is obviously wrong here.  Strange shapes lumber around
outside %H!

You realize that %l needs your assistance!]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Thank you %p for your sacrifice, I know this must have been
difficult to do. Rest assured, %d smiles down
upon you this day.  %o will be safe with us."]],
      },
      goal_first = {
         synopsis = "[You are surrounded by brimstone, lava, and elementals.]",
         output = "text",
         text = [[The stench of brimstone is all about you, and the elementals close in
from all sides!

Ahead, there is a small clearing amidst the bubbling pits of lava...]],
      },
      goal_next = {
         text = "Again, you have invaded %ns domain.",
      },
      gotit = {
         synopsis = "[You feel the essence of %d and realize that you should take %o to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel the essence of
%d fill your soul.  You know now why %n stole %oi from
%H, for with %oi, %ca of %d could
easily defeat his plans.

You sense a message from %d.  Though not verbal, you
get the impression that you must return to %l as soon
as possible.]],
      },
      gotit2 = {
         synopsis = "[You feel the essence of %d.]",
         output = "text",
         text = [[As you pick up %o, you feel the essence of
%d fill your soul.  You know now why %n stole %oi from
%H, for with %oi, %ca of %d could
easily defeat his plans.]],
      },
      guardtalk_after = {
         "\"Greetings, honorable %r.  It is good to see you again.\"",
         "\"Ah, %p!  Our deepest gratitude for all of your help.\"",
         "\"Greetings, %s.  Perhaps you will take some time to meditate with us?\"",
         "\"With this test behind you, may %d bring you enlightenment.\"",
         "\"May %d be with you, %s.\"",
      },
      guardtalk_before = {
         "\"Greetings, honorable %r.  It is good to see you.\"",
         "\"Ah, %p!  Surely you can help us in our hour of need.\"",
         "\"Greetings, %s.  %lC has great need of your help.\"",
         "\"Alas, it seems as if even %d has deserted us.\"",
         "\"May %d be with you, %s.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and deliver it to %d.]",
         output = "text",
         text = [["You have prevailed, %p!  %d is surely with you.  Now,
you must take the Amulet, and sacrifice it on %ds altar on
the Astral Plane.  I suspect that I shall never see you again in this
life, but I hope to at %ds feet."]],
      },
      killed_nemesis = {
         synopsis = "[As %n dies, %nh threatens to return.]",
         output = "text",
         text = [[%nC gasps:

    "You have only defeated this mortal body.  Know this: my spirit
    is strong.  I shall return and reclaim what is mine!"

With that, %n expires.]],
      },
      leader_first = {
         synopsis = "[%lC checks whether you are ready for the great challenge.]",
         output = "text",
         text = [["Ah, %p, my %S.  You have returned to us at last.
A great blow has befallen our order; perhaps you can help us.
First, however, I must determine if you are prepared for this
great challenge."]],
      },
      leader_last = {
         synopsis = "[You are a heretic and have failed utterly.]",
         output = "text",
         text = [["You are a heretic, %p!  How can you, %ra, deviate so from the
teachings of %d?  Begone from this temple.  You are no longer
%sa to this order.  We will pray to %d for other assistance,
as you have failed us utterly.  You will never be allowed
to fulfill your destiny as long as I live and breathe."]],
      },
      leader_next = {
         text = "\"Again, my %S, you stand before me.  Are you ready now to help us?\"",
      },
      leader_other = {
         text = "\"Once more, %p, you stand within the sanctum.  Are you ready now?\"",
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[%lC studies %o for a moment,
then returns his gaze to you.

"Well done, %p.  You have defeated %n and
recovered %o, and I am prepared to lift the
curse from the silver bell.  However, %d has
directed that %o shall be kept
here with us at %H.

Please %p, return %o to us now.]],
      },
      locate_first = {
         synopsis = "[You have reached %i.  %nC lurks further ahead.]",
         output = "text",
         text = [[You remember the descriptions of %i, given
to you by %l.  It is ahead that you will find
%n's trail.]],
      },
      locate_next = {
         text = "Again, you stand before %i.",
      },
      nemesis_first = {
         synopsis = "[You are no %g.  You shall never regain %o.]",
         output = "text",
         text = [["Ah, so %l has sent another %g to retrieve
%o.

"No, I see you are no %g.  Perhaps I shall have some fun today
after all.  Prepare to die, %r!  You shall never regain
%o."]],
      },
      nemesis_next = {
         text = "\"So, %r.  Again you challenge me.\"",
      },
      nemesis_other = {
         text = "\"Die now, %r.  %d has no power here to aid you.\"",
      },
      nemesis_wantsit = {
         text = "\"You shall die, %r, and I will have %o back.\"",
      },
      nexttime = {
         text = "Once again, you stand before %H.",
      },
      offeredit = {
         synopsis = "[Keep %o.  %oH will help you recover the Amulet of Yendor.]",
         output = "text",
         text = [["You have returned, %p.  And with %o, I see.
Congratulations.

"I have been in meditation, and have received direction from
a minion of %d.  %d commands that you retain
%o.  With %oi, you must recover the Amulet
of Yendor.  The curse has been lifted from the silver bell.

"Go forth, and let %d guide your steps."]],
      },
      offeredit2 = {
         synopsis = "[Keep %o and return to %Z to search for the Amulet.]",
         output = "text",
         text = [[%lC studies %o for a moment,
then returns his gaze to you.

"%oC must remain with you.  Use %oi
as you resume your search for the Amulet.  The curse has
been lifted from the silver bell. %Z await
your return through the magic portal that brought you here."]],
      },
      othertime = {
         text = [[Again you face %H.  Your intuition hints that this
may be the final time you come here.]],
      },
      posthanks = {
         text = "\"Welcome back, %p.  How is your quest for the Amulet going?\"",
      },
   },
   Pri = {
      assignquest = {
         synopsis = "[%nC invaded %H and captured %o.  Defeat %ni and retrieve %oh.]",
         output = "text",
         text = [["Yes, %p.  You are truly ready now.  Attend to me and I shall
tell you of what has transpired:

"At one of the Great Festivals a short time ago, %n and a legion
of undead invaded %H.  Many %gP were killed, including
the one carrying %o.

"As a final act of vengefulness, %n desecrated the altar here.
Without it, we could not mount a counter-attack.  Now, there are
barely enough %gP left to keep the undead at bay.

"We need you to find %i, then, from there, travel
to %ns lair.  If you can manage to defeat %n and return
%o here, we can then drive off the legions of
undead that befoul the land.  The silver bell is also
in %ns possession; return with it, so that we may lift
the curse that %n has placed upon it.

"Go with %d as your guide, %p."]],
      },
      badalign = {
         synopsis = "[You have deviated from the path.  Return when you have purified yourself.]",
         output = "text",
         text = [["This is terrible, %p.  You have deviated from the true path!
You know that %d requires the most strident devotion of this
order.  The %shood must stand for utmost piety.

"Go from here, atone for your sins against %d.  Return only when
you have purified yourself."]],
      },
      badlevel = {
         synopsis = "[%rA cannot withstand %n.  Come back when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, it is not yet to be.  A mere %r could never
withstand the might of %n.  Go forth, again into the world, and return
when you have attained the post of %R."]],
      },
      discourage = {
         "\"Submit to my will, %c, and I shall spare you.\"",
         "\"Your puny powers are no match for me, %c.\"",
         "\"I shall have you turned into a zombie for my pleasure!\"",
         "\"Despair now, %r.  %d cannot help you.\"",
         "\"I shall feast upon your soul for many days, %c.\"",
         "\"Your death will be slow and painful.  That I promise!\"",
         "\"You cannot defeat %n, you fool.  I shall kill you now.\"",
         "\"Your precious %lt will be my next victim.\"",
         "\"I feel your powers failing you, %r.  You shall die now.\"",
         "\"With %o, nothing can stand in my way.\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that we may lift the curse from the silver bell.\"",
         "\"You can prevail, if you rely on %d.\"",
         "\"Remember that %n has great magic at his command.\"",
         "\"Be pure, my %S.\"",
         "\"Beware, %i is surrounded by a great graveyard.\"",
         "\"You may be able to affect %n with magical cold.\"",
         "\"Acquire and wear %o if you can.  It will aid you against %n.\"",
         "\"Call upon %d when your need is greatest.  You will be answered.\"",
         "\"The undead legions are weakest during the daylight hours.\"",
         "\"Do not lose faith, %p.  If you do so, %n will grow stronger.\"",
         "\"Wear %o.  It will assist you against the undead.\"",
      },
      firsttime = {
         synopsis = "[You are at %H; the doors are closed.  %lC needs your help!]",
         output = "text",
         text = [[You find yourself standing in sight of %H.  Something
is obviously wrong here.  The doors to %H, which usually
stand open, are closed.  Strange human shapes shamble around
outside.

You realize that %l needs your assistance!]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Thank you %p for your sacrifice, I know this must have
been difficult to do.  Rest assured, %d smiles down
upon you this day.  %oC will be safe with us."]],
      },
      goal_first = {
         synopsis = "[The stench of brimstone surrounds you, the shrieks and moans are endless.]",
         output = "text",
         text = [[The stench of brimstone is all about you, and the shrieks and moans
of tortured souls assault your psyche.

Ahead, there is a small clearing amidst the bubbling pits of lava...]],
      },
      goal_next = {
         text = "Again, you have invaded %ns domain.",
      },
      gotit = {
         synopsis = "[You feel %d as you pick up %o; return %oh to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel the essence of
%d fill your soul.  You know now why %n stole it from
%H, for with it, %ca of %d could
easily defeat his plans.

You sense a message from %d.  Though not verbal, you
get the impression that you must return to %l as soon
as possible.]],
      },
      gotit2 = {
         synopsis = "[You feel %d as you pick up %o.]",
         output = "text",
         text = [[As you pick up %o, you feel the essence of
%d fill your soul.  You know now why %n stole it from
%H, for with it, %ca of %d could
easily defeat his plans.]],
      },
      guardtalk_after = {
         "\"Greetings, %r.  It is good to see you again.\"",
         "\"Ah, %p!  Our deepest gratitude for all of your help.\"",
         "\"Welcome back, %s!  With %o, no undead can stand against us.\"",
         "\"Praise be to %d, for delivering us from %n.\"",
         "\"May %d be with you, %s.\"",
      },
      guardtalk_before = {
         "\"Greetings, honored %r.  It is good to see you.\"",
         "\"Ah, %p!  Surely you can help us in our hour of need.\"",
         "\"Greetings, %s.  %lC has great need of your help.\"",
         "\"Alas, it seems as if even %d has deserted us.\"",
         "\"May %d be with you, %s.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and offer it on %ds altar.]",
         output = "text",
         text = [["You have prevailed, %p!  %d is surely with you.  Now,
you must take the amulet, and sacrifice it on %ds altar on
the Astral Plane.  I suspect that I shall never see you again in this
life, but I hope to at %ds feet."]],
      },
      killed_nemesis = {
         synopsis = "[%nC dies.  Moloch is aware of you and angry at %n.]",
         output = "text",
         text = [[You feel a wrenching shift in the ether as %ns body dissolves
into a cloud of noxious gas.

Suddenly, a voice booms out:

    "Thou hast defeated the least of my minions, %r.
    Know now that Moloch is aware of thy presence.
    As for thee, %n, I shall deal with thy failure
    at my leisure."

You then hear the voice of %n, screaming in terror...]],
      },
      leader_first = {
         synopsis = "[You have returned and we need your help.  Are you ready?]",
         output = "text",
         text = [["Ah, %p, my %S.  You have returned to us at last.
A great blow has befallen our order; perhaps you can help us.
First, however, I must determine if you are prepared for this
great challenge."]],
      },
      leader_last = {
         synopsis = "[You are a heretic who has deviated from the teachings of %d.]",
         output = "text",
         text = [["You are a heretic, %p!  How can you, %ra, deviate so from the
teachings of %d?  Begone from this temple.  You are no longer
%sa to this order.  We will pray to %d for other assistance,
as you have failed us utterly.  By the power of %d, as long
as I lead this order, you shall never be allowed entrance to
this challenge."]],
      },
      leader_next = {
         text = "\"Again, my %S, you stand before me.  Are you ready now to help us?\"",
      },
      leader_other = {
         text = "\"Once more, %p, you stand within the sanctum.  Are you ready now?\"",
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [["You have returned, %p.  And with %o, I see.
Congratulations.

"Well done, %p.  You have defeated %n and recovered
%o, and I am ready to dispel the curse from
the silver bell.  However, %d has directed that
%o shall be kept here with us at %H.

Please %p, return %o to us now.]],
      },
      locate_first = {
         synopsis = "[You have found %i.  The trail to %n lies ahead.]",
         output = "text",
         text = [[You stand facing a large graveyard.  The sky above is filled with clouds
that seem to get thicker closer to the center.  You sense the presence of
undead in larger numbers than you have ever encountered before.

You remember the descriptions of %i, given to you by
%l.  It is ahead that you will find %ns trail.]],
      },
      locate_next = {
         text = "Again, you stand before %i.",
      },
      nemesis_first = {
         synopsis = "[%lC has sent you, but you are no %gC.  I shall destroy you.]",
         output = "text",
         text = [["Ah, so %l has sent another %gC to retrieve
%o.

"No, I see you are no %gC.  Perhaps I shall have some fun today
after all.  Prepare to die, %r!  You shall never regain
%o."]],
      },
      nemesis_next = {
         text = "\"So, %r.  Again you challenge me.\"",
      },
      nemesis_other = {
         text = "\"Die now, %r.  %d has no power here to aid you.\"",
      },
      nemesis_wantsit = {
         text = "\"You shall die, %r, and I will have %o back.\"",
      },
      nexttime = {
         text = "Once again, you stand before %H.",
      },
      offeredit = {
         synopsis = "[Congratulations, %p.  Keep %o; go and recover the Amulet.]",
         output = "text",
         text = [["You have returned, %p.  And with %o, I see.
Congratulations.

"I have been in meditation, and have received direction from
a minion of %d.  %d commands that you retain
%o.  With it, you must recover the Amulet
of Yendor.  The curse has been lifted from the silver bell.

"Go forth, and let %d guide your steps."]],
      },
      offeredit2 = {
         synopsis = "[%oC is yours now.  Return to %Z and find the Amulet.]",
         output = "text",
         text = [[%lC reiterates that %o is yours now.

"The time has come to resume your search for the Amulet.
The curse has been lifted from the silver bell.
%Z await your return through the magic portal
that brought you here."]],
      },
      othertime = {
         text = [[Again you face %H.  Your intuition hints that this may be
the final time you come here.]],
      },
      posthanks = {
         text = "\"Welcome back, %p.  How is your quest for the Amulet going?\"",
      },
   },
   Ran = {
      assignquest = {
         synopsis = "[%nC has stolen %o.  Infiltrate %i and retrieve %oh for us.]",
         output = "text",
         text = [["You are indeed ready, %p.  I shall tell you what has transpired,
and why we so desperately need your help:

"A short time ago, the mountain centaurs to the east invaded
and enslaved the plains centaurs in this area.  The local
leader is now only a figurehead, and serves %n.

"During our last gathering of worship here, we were beset by hordes of
hostile centaurs, as you witnessed.  In the first onslaught a group,
headed by %n %niself, managed to breach the grove and steal
%o.

"Since then, we have been besieged.  We do not know how much longer
we will be able to maintain our magical barriers.

"If we are to survive, you, %p, must infiltrate
%i.  There, you will find a pathway down, to the
underground cavern of %n.  He has always coveted
%o, and will surely keep it.  There you will
also find the silver bell.  Return with it so
that we can lift the curse that %n
has placed upon it.

"Recover %o for us, %p!  Only then will %d be safe."]],
      },
      badalign = {
         synopsis = "[You are not sufficiently %a.  Come back when you have purified yourself.]",
         output = "text",
         text = [["You have strayed, %p!  You know that %d requires that
we maintain a pure devotion to things %a!

"You must go from us.  Return when you have purified yourself."]],
      },
      badlevel = {
         synopsis = "[You are too inexperienced.  Come back when you are %Ra.]",
         output = "text",
         text = [["%p, you are yet too inexperienced to withstand the demands of that
which we need you to do.  %RA might just be able to do this thing.

"Return to us when you have learned more, my %S."]],
      },
      discourage = {
         "\"Your %d is nothing, %c.  You are mine now!\"",
         "\"Run away little %c!  You can never hope to defeat %n!\"",
         "\"My servants will rip you to shreds!\"",
         "\"I shall display your head as a trophy.  What do you think about that wall?\"",
         "\"I shall break your %ls grove, and destroy all the %gP!\"",
         "\"%d has abandoned you, %c.  You are doomed.\"",
         "\"%rA?  %lC sends a mere %r against me?  Hah!\"",
         "\"%lC has failed, %c.  %oC will never leave here.\"",
         "\"You really think you can defeat me, eh %c?  You are wrong!\"",
         "\"You weaken, %c.  I shall kill you now.\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that we may lift the curse from the silver bell.\"",
         "\"It is rumored that the Forest and Mountain Centaurs have resolved their ancient feud and now band together against us.\"",
         "\"%nC is strong, and very smart.\"",
         "\"Use %o, when you find it.  It will help you survive to reach us.\"",
         "\"Remember, let %d be your guide.\"",
         "\"Call upon %d when you face %n. The very act of doing so will infuriate him, and give you advantage.\"",
         "\"%n and his kind have always hated us.\"",
         "\"We cannot hold the grove much longer, %p.  Hurry!\"",
         "\"To infiltrate %i, you must be very stealthy.\"",
         "\"Remember that %n is a braggart.  Trust not what he says.\"",
         "\"You can triumph, %p, if you trust in %d.\"",
      },
      firsttime = {
         synopsis = "[The ancient forest grove is surrounded by centaurs.]",
         output = "text",
         text = [[You arrive in familiar surroundings.  In the distance, you %x the
ancient forest grove, the place of worship to %d.

Something is wrong, though.  Surrounding the grove are centaurs!
And they've noticed you!]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Thank you %p for your sacrifice, I know this must have been
difficult to do. Rest assured, %d smiles down
upon you this day.  %o will be safe with us."]],
      },
      goal_first = {
         synopsis = "[You descend into a subterranean complex.  Hooves clatter in the distance.]",
         output = "text",
         text = [[You descend into a weird place, in which roughly cut cave-like walls
join with smooth, finished ones, as if someone was in the midst of
finishing off the construction of a subterranean complex.

Off in the distance, you hear a sound like the clattering of many
hooves on rock.]],
      },
      goal_next = {
         text = "Once again, you enter the distorted castle of %n.",
      },
      gotit = {
         synopsis = "[You pick up %o and feel power.  It's time to return %oh to %l.]",
         output = "text",
         text = [[As you pick up %o, it seems to glow, and a warmth
fills you completely.  You realize that its power is what has protected
your %sp against their enemies for so long.

You must now return it to %l without delay -- their lives depend
on your speed.]],
      },
      gotit2 = {
         synopsis = "[You pick up %o and feel power.]",
         output = "text",
         text = [[As you pick up %o, it seems to glow, and a warmth
fills you completely.  You realize that its power is what has protected
your %sp against their enemies for so long.]],
      },
      guardtalk_after = {
         "\"%pC!  I have not seen you in many moons.  How do you fare?\"",
         "\"Birdsong has returned to the grove, surely this means you have defeated %n.\"",
         "\"%lC seems to have regained some of his strength.\"",
         "\"So, tell us how you entered %i, in case some new evil arises there.\"",
         "\"Is that truly %o that I see you carrying?\"",
      },
      guardtalk_before = {
         "\"%pC!  I have not seen you in many moons.  How do you fare?\"",
         "\"%nC continues to threaten the grove.  But we hold fast.\"",
         "\"%lC is growing weak.  The magic required to defend the grove drains us.\"",
         "\"Remember %i is hard to enter.  Beware the distraction of leatherwings.\"",
         "\"We must regain %o.  Without it we will be overrun.\"",
      },
      hasamulet = {
         synopsis = "[You have the Amulet!  Take it to the Astral Plane and offer it to %d.]",
         output = "text",
         text = [["You have it!  You have recovered the Amulet of Yendor!
Now attend to me, %p, and I will tell you what must be done:

"The Amulet has within it magic, the capability to transport you to
the Astral Plane, where the primary circle of %d resides.

"To activate this magic, you must travel upwards as far as you can.
When you reach the temple, sacrifice the Amulet to %d.

"Thus will you fulfill your destiny."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[%nC collapses to the ground, cursing you and %l, then says:

    "You have defeated me, %r!  But I curse you one final time, with
    my dying breath!  You shall die before you leave my castle!"]],
      },
      leader_first = {
         synopsis = "[You have returned, %p.  We need your help.  Are you ready?]",
         output = "text",
         text = [["%pC!  You have returned!  Thank %d.

"We have great need of you.  But first, I must see if you have the
required abilities to take on this responsibility."]],
      },
      leader_last = {
         synopsis = "[You are not sufficiently %a.  We renounce your %shood.]",
         output = "text",
         text = [["%pC!  You have doomed us all.  You fairly radiate %L influences
and weaken the power we have raised in this grove as a result!

"Begone!  We renounce your %shood with us!  For as long as I live,
 you are forever an outcast!"]],
      },
      leader_next = {
         text = "\"Once again, %p, you stand in our midst.  Are you ready now?\"",
      },
      leader_other = {
         text = "\"Ah, you are here again, %p.  Allow me to determine your readiness...\"",
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[%l inspects %o reverently.

"Well done, %p.  You have defeated %n and
recovered %o, and we are ready to lift the curse
from the silver bell.  Alas, I fear that %o shall
never be truly safe unless it is kept here with
us at %H.

Please %p, return %o to us now.]],
      },
      locate_first = {
         synopsis = "[This is %i.  There are bats nearby.  Beware the wumpus!]",
         output = "text",
         text = [[This must be %i.

You are in a cave built of many different rooms, all interconnected
by tunnels.  Your quest is to find and shoot the evil wumpus that
resides elsewhere in the cave without running into any bottomless
pits or using up your limited supply of ammunition.  Good luck.

You are in room 9 of the cave.  There are tunnels to rooms
5, 8, and 10.
*rustle* *rustle* (must be bats nearby.)
*sniff* (I can smell the evil wumpus nearby!)]],
      },
      locate_next = {
         synopsis = "[You are in %i.  There are pits.  There are bats nearby.]",
         output = "text",
         text = [[Once again, you descend into %i.

*whoosh* (I feel a draft from some pits.)
*rustle* *rustle* (must be bats nearby.)]],
      },
      nemesis_first = {
         synopsis = "[You have come to recover %o, but I shall keep %oh and you shall die.]",
         output = "text",
         text = [["So, %c.  %lC has sent you to recover %o.

"Well, I shall keep that bauble.  It pleases me.  You, %c, shall die."]],
      },
      nemesis_next = {
         text = "\"Back again, eh?  Well, a mere %r is no threat to me!  Die, %c!\"",
      },
      nemesis_other = {
         text = "\"You haven't learned your lesson, %c.  You can't kill me!  You shall die now.\"",
      },
      nemesis_wantsit = {
         text = [["I shall have %o from you, %r.  Then I shall
kill you."]],
      },
      nexttime = {
         text = "Once again, you stand before %H.",
      },
      offeredit = {
         synopsis = "[You have succeeded.  Take %o with you as you go to find the Amulet.]",
         output = "text",
         text = [["%pC!  You have succeeded!  I feared it was not possible!

"You have returned with %o!

"I fear, now, that the Centaurs will regroup and plot yet another raid.
This will take some time, but if you can recover the Amulet of Yendor
for %d before that happens, we will be eternally safe.
We have removed the curse from the silver bell.

"Take %o with you.  It will aid in your quest for
the Amulet."]],
      },
      offeredit2 = {
         synopsis = "[You are the keeper of %o now.  Go and find the Amulet.]",
         output = "text",
         text = [[%l inspects %o reverently.

"With this wondrous weapon, one need never run out of ammunition.
You are its keeper now, and the time has come to resume your
search for the Amulet.  We have removed the curse from the silver
bell.  %Z await your return
through the magic portal that brought you here."]],
      },
      othertime = {
         text = [[You have the oddest feeling that this may be the last time you
are to enter %H.]],
      },
      posthanks = {
         text = [["Welcome, %p.  How have you fared on your quest for the Amulet
of Yendor?"]],
      },
   },
   Rog = {
      assignquest = {
         synopsis = "[Get %o from %n and bring it to %l.]",
         output = "text",
         text = [["Will everyone not going to retrieve %o from that
jerk, %n, take one step backwards.  Good choice,
%p, because I was going to send you anyway.  My other %gp
are too valuable to me.

"Here's the deal.  I want %o, %n
has %o.  You are going to get %o
and bring it back to me.  So simple an assignment even you can understand
it.  Bring back the silver bell while you're at it so we can uncurse it."]],
      },
      badalign = {
         synopsis = "[Come back when you are really %a.]",
         output = "text",
         text = [["Maybe I should chain you to my perch here for a while.  Perhaps watching
real %a men at work will bring some sense back to you.  I don't
think I could stand the sight of you for that long though.  Come back
when you can be trusted to act properly."]],
      },
      badlevel = {
         synopsis = "[%rA is not adequately trained to handle this job.]",
         output = "text",
         text = [["In the time that you've been gone you've only been able to master the
arts of %ra?  I've trained ten times again as many %Rp
in that time.  Maybe I should send one of them, no?  Where would that
leave you, %p?  Oh yeah, I remember, I was going to kill you!"]],
      },
      discourage = {
         "\"May I suggest a compromise.  Are you interested in gold or gems?\"",
         "\"Please don't force me to kill you.\"",
         "\"Grim times are upon us all.  Will you not see reason?\"",
         "\"I knew %l, and you're no %lt, thankfully.\"",
         "\"It is a shame that we are not meeting under more pleasant circumstances.\"",
         "\"I was once like you are now, %p.  Believe in me -- our way is better.\"",
         "\"Stay with me, and I will make you %os guardian.\"",
         "\"When you return, with or without %o, %l will have you killed.\"",
         "\"Do not be fooled; I am prepared to kill to defend %o.\"",
         "\"I can reunite you with the Twain.  Oh, the stories you can swap.\"",
      },
      encourage = {
         "\"Can you remember to come back with %o so we can uncurse the silver bell?  Can you do that?\"",
         "\"You don't seem to understand, %o isn't here so neither should you be!\"",
         "\"May %d curse you with lead fingers.  Get going!\"",
         "\"We don't have all year.  GET GOING!\"",
         "\"How would you like a scar necklace?  I'm just the jeweler to do it!\"",
         "\"Lazy S.O.B.  Maybe I should call up someone else...\"",
         "\"Maybe I should open your skull and see if my instructions are inside?\"",
         "\"This is not a task you can complete in the afterlife, you know.\"",
         "\"Inside every living person is a dead person trying to get out, and I have your key!\"",
         "\"We're almost out of hell-hound chow, so why don't you just get moving!\"",
         "\"You know, %o isn't going to come when you whistle.  You must get it yourself.\"",
      },
      firsttime = {
         synopsis = "[You are in Ransmannsby, where you trained.  Find %l.]",
         output = "text",
         text = [[Unexpectedly, you find yourself back in Ransmannsby, where you trained to
be a thief.  Quickly you make the guild sign, hoping that you AND word
of your arrival reach %ls den.]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Seriously?  You gave up %o
without a fight?  You're weaker than I thought.
Leave now before I decide to put you out of
everyone else's misery."]],
      },
      goal_first = {
         synopsis = "[You sense %o.]",
         output = "text",
         text = [[You feel a great swelling up of courage, sensing the presence of
%o.  Or is it fear?]],
      },
      goal_next = {
         text = "The hairs on the back of your neck whisper -- it's fear.",
      },
      gotit = {
         synopsis = "[You pick up %o and know that %l should not have it.]",
         output = "text",
         text = [[As you pick up %o, the hairs on the back of your
neck fall out.  At once you realize why %n was
willing to die to keep it out of %ls hands.  Somehow
you know that you must do likewise.]],
      },
      gotit2 = {
         synopsis = "[You pick up %o, relieved that %l is already dead.]",
         output = "text",
         text = [[As you pick up %o, the hairs on the back of your
neck fall out.  At once you realize why %n was
willing to die to keep it out of %ls hands.  Good
thing %l is already dead...]],
      },
      guardtalk_after = {
         "\"I was sure wrong about Lady Tyvefelle's house; I barely got away with my life and lost my lock pick in the process.\"",
         "\"You're back?  Even the Twain don't come back anymore.\"",
         "\"Can you spare an old cutpurse a zorkmid for some grog?\"",
         "\"Fritz tried to join the other side, and now he's hell-hound chow.\"",
         "\"Be careful what you steal, I hear the boss has perfected turning rocks into worthless pieces of glass.\"",
      },
      guardtalk_before = {
         "\"I hear that Lady Tyvefelle's household is lightly guarded.\"",
         "\"You're back?  Even the Twain don't come back anymore.\"",
         "\"Can you spare an old cutpurse a zorkmid for some grog?\"",
         "\"Fritz tried to join the other side, and now he's hell-hound chow.\"",
         "\"Be careful what you steal, I hear the boss has perfected turning rocks into worthless pieces of glass.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and find %ds temple.]",
         output = "text",
         text = [["I see that with your abilities, and my brains, we could rule this world.

"All that we would need to be all-powerful is for you to take that little
trinket you've got there up to the Astral Plane.  From there, %d will
show you what to do with it.  Once that's done, we will be invincible!"]],
      },
      killed_nemesis = {
         synopsis = "[Before dying, %n tells you to use %o wisely.]",
         output = "text",
         text = [["I know what you are thinking, %p.  It is not too late for you
to use %o wisely.  For the sake of your guild
%sp, do what is right."

You sit and wait for death to come for %n, and then you
brace yourself for your next meeting with %l!]],
      },
      leader_first = {
         synopsis = "[You owe back dues to your guild.  You can pay them off if you're up to the job.]",
         output = "text",
         text = [["Well, look who it is boys -- %p has come home.  You seem to have
fallen behind in your dues.  I should kill you as an example to these
other worthless cutpurses, but I have a better plan.  If you are ready
maybe you could work off your back dues by performing a little job for
me.  Let us just see if you are ready..."]],
      },
      leader_last = {
         synopsis = "[You must go.]",
         output = "text",
         text = [["Well %gp, it looks like our friend has forgotten who is the boss
around here.  Our friend seems to think that %rp have been put in
charge.  Wrong.  DEAD WRONG!"

It seems as long as %l lives, you'll never be given the contract for
this job...]],
      },
      leader_next = {
         synopsis = "[Are you stupid or are you ready?]",
         output = "text",
         text = [["Well, I didn't expect to see you back.  It shows that you are either stupid,
or you are finally ready to accept my offer.  Let us hope for your sake it
isn't stupidity that brings you back."]],
      },
      leader_other = {
         text = [["Did you perhaps mistake me for some other %lt?  You must
think me as stupid as your behavior.  I warn you not to try my patience."]],
      },
      leader_wantsit = {
         synopsis = "[%lC demands you forfeit %o.]",
         output = "text",
         text = [[%lC eyes %o intently, like
he's about to take it from you.

"Leave %o, you're not taking it with you.
And go filch the Amulet before someone else beats you to it.
The silver bell isn't uncursed yet, so hand %o over, yeah?
%Z are back the way you came, through the magic portal."]],
      },
      locate_first = {
         text = "Those damn little hairs tell you that you are nearer to %o.",
      },
      locate_next = {
         text = "Not wanting to face %l without having stolen %o, you continue.",
      },
      nemesis_first = {
         text = "\"Ah!  You must be %ls ... er, `hero'.  A pleasure to meet you.\"",
      },
      nemesis_next = {
         text = "\"We meet again.  Please reconsider your actions.\"",
      },
      nemesis_other = {
         synopsis = "[You cannot trust %l.]",
         output = "text",
         text = [["Surely, %p, you have learned that you cannot trust any bargains
that %l has made.  I can show you how to continue on
your quest without having to run into him again."]],
      },
      nemesis_wantsit = {
         synopsis = "[%lC should not have %o.]",
         output = "text",
         text = [["Please, think for a moment about what you are doing.  Do you truly
believe that %d would want %l to have
%o?"]],
      },
      nexttime = {
         text = [[Once again, you find yourself back in Ransmannsby.  Fond memories are
replaced by fear, knowing that %l is waiting for you.]],
      },
      offeredit = {
         synopsis = "[Take %o with you and go.]",
         output = "text",
         text = [["Well, I'll be damned.  You got it.  I am proud of you, a fine %r
you've turned out to be.

"While you were gone I got to thinking, you and %o
together could bring me more treasure than either of you apart, so why don't
you take it with you.  All I ask is a cut of whatever loot you come by.
That is a better deal than I offered %n.

"But, you see what happened to %n when he refused.
Don't make me find another to send after you this time."]],
      },
      offeredit2 = {
         synopsis = "[Take %o and acquire the Amulet.]",
         output = "text",
         text = [[%lC seems tempted to swap %o for
the mundane one you detect in his pocket, but noticing your alertness,
evidently chickens out.

"Go filch the Amulet before someone else beats you to it.
%Z are back the way you came, through the magic portal."]],
      },
      othertime = {
         text = [[You rub your hands through your hair, hoping that the little ones on
the back of your neck stay down, and prepare yourself for your meeting
with %l.]],
      },
      posthanks = {
         synopsis = "[How about trading %o for something?]",
         output = "text",
         text = [["Quite the little thief, aren't we, %p.  Can I interest you in a
swap for %o?  Look around, anything in the keep
is yours for the asking."]],
      },
   },
   Sam = {
      assignquest = {
         synopsis = "[You must enter %i, then regain %o from %n.]",
         output = "text",
         text = [["Domo %p-san, indeed you are ready.  I can now tell you what
it is that I require of you.

"The daimyo, %n, has betrayed us.  He has stolen from us
%o and taken it to his donjon deep within
%i.  The silver bell is there as well, and has been
cursed by %n.

"If I cannot show the emperor %o when he comes
for the festival he will know that I have failed in my duty, and
request that I commit seppuku.

"You must gain entrance to %i and retrieve the
emperor's property.  Be quick!  The emperor will be here for the
cha-no-you in 5 sticks.

"Wakarimasu ka?"]],
      },
      badalign = {
         synopsis = "[When you can think %a and act %a then return.]",
         output = "text",
         text = [["%p-san, you would do better to join the kyokaku.

"You have skills, but until you can call upon the bushido to know when and
how to use them you are not samurai.  When you can think %a and
act %a then return."]],
      },
      badlevel = {
         synopsis = "[\"I require %Ra to defeat %n.  Return when you are ready.\"]",
         output = "text",
         text = [["%p-san, you have learned well and honored your family.
I require the skills of %Ra in order to defeat %n.
Go and seek out teachers.  Learn what they have learned.  When you
are ready, return to me."]],
      },
      discourage = {
         "\"Ahh, I finally meet the daimyo of the kyokaku!\"",
         "\"There is no honor for me in your death.\"",
         "\"You know that I cannot resash my swords until they have killed.\"",
         "\"Your presence only compounds the dishonor of %l in not coming %liself.\"",
         "\"I will make tea with your hair and serve it to %l.\"",
         "\"Your fear shows in your eyes, coward!\"",
         "\"I have not heard of you, %p-san; has your life been that unworthy?\"",
         "\"If you will not obey me, you will die.\"",
         "\"Kneel now and make the two cuts of honor.  I will tell your %sp of your honorable death.\"",
         "\"Your master was a poor teacher.  You will pay for his mistakes in your teaching.\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that we may lift the curse from the silver bell.\"",
         "\"To defeat %n you must overcome the seven emotions: hate, adoration, joy, anxiety, anger, grief, and fear.\"",
         "\"Remember your honor is my honor, you perform in my name.\"",
         "\"I will go to the temple and burn incense for your safe return.\"",
         "\"Sayonara.\"",
         "\"There can be honor in defeat, but no gain.\"",
         "\"Your kami must be strong in order to succeed.\"",
         "\"You are indeed a worthy %R, but now you must be a worthy samurai.\"",
         "\"If you fail, %n will be like a tai-fun on the land.\"",
         "\"If you are truly %a, %d will listen.\"",
         "\"Sharpen your swords and your wits for the task before you.\"",
      },
      firsttime = {
         synopsis = "[The banner of %n flies above town.  What has happened to %l?]",
         output = "text",
         text = [[Even before your senses adjust, you recognize the kami of
%H.

You %x the standard of your teki, %n, flying above
the town.  How could such a thing have happened?  Why are ninja
wandering freely; where are the samurai of your daimyo, %l?

You quickly say a prayer to Izanagi and Izanami and walk towards
town.]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["You have chosen wisely %p-san.
I know this must have been difficult to do.
Rest assured, %d smiles down upon you this day.
%o will be safe with us."]],
      },
      goal_alt = {
         text = "As you arrive once again at the home of %n.",
      },
      goal_first = {
         synopsis = "[You feel the taunts %n, but after offering a prayer to %d, you proceed.]",
         output = "text",
         text = [[In your mind, you hear the taunts of %n.

You become like the rice plant and bend to the ground, offering a
prayer to %d.  But when the wind has passed, you stand
proudly again.  Putting your kami in the hands of fate, you advance.]],
      },
      goal_next = {
         text = [[As you arrive once again at the home of %n, your thoughts
turn only to %o.]],
      },
      gotit = {
         synopsis = "[You feel the power of %o and are humbled.]",
         output = "text",
         text = [[As you pick up %o, you feel the strength of its karma.
You realize at once why so many good samurai had to die to defend it.
You are humbled knowing that you hold one of the artifacts of the
sun goddess.]],
      },
      gotit2 = {
         synopsis = "[You feel the power of %o and are humbled.]",
         output = "text",
         text = [[As you pick up %o, you feel the strength of its karma.
You realize at once why so many good samurai had to die to defend it.
You are humbled knowing that you hold one of the artifacts of the
sun goddess.]],
      },
      guardtalk_after = {
         "\"Come, join us in celebrating with some sake.\"",
         "\"Ikaga desu ka?\"",
         "\"You have brought our clan and %l much honor.\"",
         "\"Please %r, sit for a while and tell us how you overcame the Ninja.\"",
         "\"%lC still lives!  You have saved us from becoming ronin.\"",
      },
      guardtalk_before = {
         "\"To succeed, you must walk like a butterfly on the wind.\"",
         "\"Ikaga desu ka?\"",
         "\"I fear for The Land of The Gods.\"",
         "\"%nC has hired the Ninja -- be careful.\"",
         "\"If %o is not returned, we will all be ronin.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane to finish your task.]",
         output = "text",
         text = [["Ah, %p-sama.  You have wasted your efforts returning home.
Now that you are in possession of the Amulet, you are honor-bound to
finish the quest you have undertaken.  There will be plenty of time
for saki and stories when you have finished.

"Go now, and may our prayers be a wind at your back."]],
      },
      killed_nemesis = {
         synopsis = "[%nC dies without honor.]",
         output = "text",
         text = [[Your healing skills tell you that %ns wounds are mortal.

You know that the bushido tells you to finish him and let his kami
die with honor, but the thought of so many samurai dead due to this
man's dishonor prevents you from giving the final blow.

You order that his unwashed head be given to the crows and his body
thrown into the sea.]],
      },
      leader_first = {
         synopsis = "[%lC needs someone to lead %lj samurai against %n.  Are you ready?]",
         output = "text",
         text = [["Ah, %p-san, it is good to see you again.  I need someone who can
lead my samurai against %n.  If you are ready, you will be
that person."]],
      },
      leader_last = {
         synopsis = "[Leave and do not come back.]",
         output = "text",
         text = [["You are no longer my samurai, %p.

"Hara-kiri is denied.  You are ordered to shave your head and then to
become a monk.  Your fief and family are forfeit.  As long as I live,
 you will never be allowed to complete this task.  Wakarimasu ka?"]],
      },
      leader_next = {
         text = [["Once again, %p-san, you kneel before me.  Are you yet capable of
being my vassal?"]],
      },
      leader_other = {
         synopsis = "[Are you truely a samurai?]",
         output = "text",
         text = [["You begin to test my matsu, %p-san.
If you cannot determine what I want in a samurai, how can I rely on you
to figure out what I need from a samurai?"]],
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[%l holds %o tightly for a moment, then returns
his gaze to you.

"Well done, %p-san.  You have defeated %n and
recovered %o, and I am prepared to lift the curse
from the silver bell.  Alas, the emperor has decreed
that %o shall remain here with us at %H.

%p-san, return %o to us now kudasai.]],
      },
      locate_first = {
         text = [[You instinctively reach for your swords.  You do not recognize the
lay of this land, but you know that your teki are everywhere.]],
      },
      locate_next = {
         text = [[Thankful that your %sp at %H cannot see
your fear, you prepare again to advance.]],
      },
      nemesis_first = {
         text = [["Ah, so it is to be you, %p-san.  I offer you seppuku.
I will be your second if you wish."]],
      },
      nemesis_next = {
         text = [["I have offered you the honorable exit.  Now I will have your
head to send unwashed to %l."]],
      },
      nemesis_other = {
         text = "\"After I have dispatched you, I will curse your kami.\"",
      },
      nemesis_wantsit = {
         text = [["You have fought my samurai; surely you must know that you
will not be able to take %o back to
%H."]],
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[The emperor wants you to take %o and recover the Amulet.]",
         output = "text",
         text = [[As you bow before %l, he welcomes you:

    "You have brought your family great honor, %p-sama.

    "While you have been gone the emperor's advisors have discovered in
    the ancient texts that the karma of the samurai who seeks to recover
    the Amulet and the karma of %o are joined
    as the seasons join to make a year.

    "Because you have shown such fidelity, the emperor requests
    that you take leave of other obligations and continue on the
    road that fate has set your feet upon.  I have removed the
    curse from the silver bell, which will aid you on your quest.
    I would consider it an honor if you would allow me to watch
    your household until you return with the Amulet."

With that, %l bows, and places his sword atop
%o.]],
      },
      offeredit2 = {
         synopsis = "[Take %o, return to %Z, and recover the Amulet.]",
         output = "text",
         text = [[%l holds %o tightly for a moment, then returns
his gaze to you.

"The time is ripe to recover the Amulet.  With the curse removed
from the silver bell, this should be a simple task.  Return to
%Z through the magic portal that transported you here
so that you may achieve the destiny which awaits you."]],
      },
      othertime = {
         synopsis = "[%HC is threatened by %n.]",
         output = "text",
         text = [[You are back at %H.

Instantly you sense a subtle change in your karma.  You seem to know that
if you do not succeed in your quest, %n will have destroyed
the kami of %H before you return again.]],
      },
      posthanks = {
         text = "%lC bows.  \"%p-sama, tell us of your search for the Amulet.\"",
      },
   },
   Tou = {
      assignquest = {
         synopsis = "[Enter %i and recover %o from %n.]",
         output = "text",
         text = [["Yes, you've become a great %c since I last saw you.
		 
"Sadly, things haven't been going so well here in %H.  A new
%nt was appointed, and he hosted a dinner to meet other civic
leaders.  At the door, one of his henchmen offered to look after my wallet.

"I know what you're thinking:  'How kind!'  But it turns out that even in the
Thieves' Guild, there are people you can't trust.  My wallet contained -- you
know the Royal Mint has been trialing credit cards, so we won't have to carry
bags of gold around?  Most of the prototypes haven't worked, and are only
good for picking locks.  This one, %o, is
one of a kind.  As the highest spending and most often mugged resident of
%H, it was entrusted to me.  But when I got my wallet back, the Card was
missing.  I appealed to the Patrician, and he said the matter fell under the
doctrine of _detectores custodes_.

"We believe %n has the Card, and that he's holed himself
up in %i.  With the power of %o, the
Thieves are bolder than ever.  Citizens are frightened, and tourism is
down. We need somebody to find their way into %i and
recover the Card before there are no %cp left!

"Oh, and while you're in there -- %n has the
silver bell, too, and somebody's put a curse on it.  Bring it back with
the Card, and we'll see about getting that curse lifted.

"So, %p, are you ready for a little Detour?"]],
      },
      badalign = {
         synopsis = "[You are not sufficiently %a.  Return when you are.]",
         output = "text",
         text = [["Oh, %p, you haven't stayed true to the %a path.  You know
%d doesn't like it when you stray.  You can't possibly go after
%n like this!

"Get back to the Dungeon.  Work on yourself.  Come back when you're truly
%a, and we'll talk."]],
      },
      badlevel = {
         synopsis = "[Return when you are %Ra.]",
         output = "text",
         text = [["%pC, you're still only %ra.  I don't think you're
prepared to take on %n yet.

"Carry on with your Tour.  Explore.  Take photos.  Learn things.  Come back
when you're a %R, and you might stand a chance.
%nC will still be here when you're ready for him."]],
      },
      discourage = {
         "\"I beat %l real bad and I'll beat you even worse, %p.\"",
         "\"Not even %d can help you here.\"",
         "\"Beg for mercy, and maybe I'll consider thinking about going easy on you.\"",
         "\"You coulda made a good Thief if you weren't so damn %a.\"",
         "\"Write your postcards, %p.  Your Tour ends here.\"",
         "\"Couldn't %l send me a better %c to beat up?\"",
         "\"With %o, I'm gonna run up a bill that'll bankrupt %H.\"",
         "\"Nobody defeats %n.  NOBODY!\"",
         "\"Poor, naive %l.  Did he send you to ask me nicely to give %o back?\"",
         "\"How do you spell %p?  I want to make sure your grave is spelled right.\"",
      },
      encourage = {
         "\"Don't forget the silver bell, %p!  Bring it back, and we'll get that curse off it.\"",
         "\"Who'd have thought %n would be such a criminal!\"",
         "\"Look out for the traps on the way to %i.  Some date back to the Mage Wars!\"",
         "\"Get %o back, and hopefully I can cancel %ns transactions.\"",
         "\"If you're stuck, try praying to %d. %dH's whimsical, but %dh sometimes helps %cp out.\"",
         "\"You have to defeat %n, or %nh'll follow you back here!\"",
         "\"If you can grab %o from %n, it might help you beat %ni.\"",
         "\"Hurry, %p!  We're all depending on you.\"",
         "\"I know you won't let me down, %p!\"",
         "\"Stay %a.  Remember, %cp don't take sides, we take photos.\"",
         "\"If anybody gives you trouble, remember: a smile can go a long way.\"",
      },
      firsttime = {
         synopsis = "[You're back at %H, but it's still ominously quiet.]",
         output = "text",
         text = [[You come round to find yourself looking at the familiar sights of
%H.  You breathe a sigh of relief, taking in the polluted air.

But this isn't %H as you remember it.  The streets are... quiet?
Doors close, curtains twitch, and a few residents scurry about nervously, but
you can't hear the hawkers selling their ropy meat pies, or the cargo barges
bulldozing their way along the river.  A whiff of rotten cabbages suggests
that the fields outside the city aren't being tended properly.

Something isn't right.  Hopefully, it's all some sort of misunderstanding.
%lC will sort it out, when you find him.]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Thank you, %p!  I know that can't have been easy.  %dC is
smiling on you today, I'm sure of it -- whimsically, but smiling.
%oC will be safe with us, and I
promise not to lose my wallet again."]],
      },
      goal_alt = {
         text = "You're back at %ns lair.",
      },
      goal_first = {
         text = "You feel the presence of %o nearby.",
      },
      goal_next = {
         text = [[You'll find %o this time, for sure!]],
      },
      gotit = {
         synopsis = "[You feel contented as you pick up %o.  Now to return it to %l.]",
         output = "text",
         text = [[You feel a rich contentment as you pick up %o.
That'll do nicely!  You'd better return it to %l as soon as possible.]],
      },
      gotit2 = {
         synopsis = "[You feel contented as you pick up %o.  Now hurry back to %H.]",
         output = "text",
         text = [[You feel a rich contentment as you pick up
%o.
That'll do nicely!  With %l gone, it's all yours --
but you should still hurry back to %H.]],
      },
      guardtalk_after = {
         "\"Gehennom on 5 zorkmids a day -- more like 500 a day if you ask me.\"",
         "\"Do you know where I could find some nice postcards of The Gnomish Mines?\"",
         "\"Have you tried the weird toilets?\"",
         "\"If you stick around, I'll show you the pictures from my latest trip.\"",
         "\"Did you bring me back any souvenirs?\"",
      },
      guardtalk_before = {
         "\"Gehennom on 5 zorkmids a day -- more like 500 a day if you ask me.\"",
         "\"Do you know where I could find some nice postcards of The Gnomish Mines?\"",
         "\"Have you tried the weird toilets?\"",
         "\"Don't stay at the Inn, I hear the food is terrible and it has rats.\"",
         "\"They told me that this was the off season!\"",
      },
      hasamulet = {
         synopsis = "[You have the Amulet.  Take it to the Astral Plane to finish your task.]",
         output = "text",
         text = [["%pC!  And you found the Amulet of Yendor?  Amazing!

"But you can't keep it here in %H.  It won't be safe!

"Besides, the Amulet isn't for us %cp, it's for %d.  I've heard
%dh has a temple on the Astral Plane, which incidentally is a gorgeous place
to visit this time of year.  Find your way there, then offer it to %d
on %dj high altar.  %dH'll be delighted to have it to %dIself at last!

Safe journey, %p."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses at you as %nh dies.]",
         output = "text",
         text = [[%nC' body buckles.  As he gasps for breath, %nh manages to
curse:

"You won't get away, %p!  My guys will come after you wherever
you go, and take %o back for the Thieves."
And he's gone.

For a moment, a black cloaked figure flickers in the corner of your eye, and
you thought you could hear hoofbeats.]],
      },
      leader_first = {
         synopsis = "[Someone must defeat %n.  Are you ready?]",
         output = "text",
         text = [["Aloha %p, it's so good to see you!  How goes your Tour?

Listen, I'm so sorry to cut your vacation short, but we've got a little bit
of a situation here.  Somebody has to do something about
%n!  Could that somebody be you?  Let me take a look at you..."]],
      },
      leader_last = {
         synopsis = "[Leave %H and never return.]",
         output = "text",
         text = [["Oh, no, %p.  You're no %c.  Your membership of the %H
Guild of Trespassers is revoked, and as long as I'm in charge here, you'll
never be allowed to attempt this quest.  Leave %H, and don't
show your face here again.  Aloha."]],
      },
      leader_next = {
         text = "\"Aloha again, %p.  Feeling ready this time?\"",
      },
      leader_other = {
         text = "\"I hope you've prepared yourself this time.\"",
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[%lC spots %o, and his face lights up
like a tourist's first look at the Brass Bridge.

"You did it, %p!  You beat %n and got
%o back, and now we can get the curse lifted
from the silver bell.  But, um... I worry the Card will never really be
safe anywhere but here in %H.

"Would you mind awfully handing %o
back to me?"]],
      },
      locate_first = {
         synopsis = "[You %x the handiwork of %ns flunkies.]",
         output = "text",
         text = [[This isn't the friendliest part of town.  There are Thieves' marks
everywhere, and %ns flunkies creep along the alleyways.]],
      },
      locate_next = {
         text = "You know that this time you'll have to destroy %n.",
      },
      nemesis_first = {
         synopsis = "[%rA will not defeat me.]",
         output = "text",
         text = [["So, %p, %l thinks you're gonna take
%o from me.  How cute is that?

"He sends %ra to defeat me.  Me!  When this is over, I'll put you
in cement shoes and throw you onto the river!"]],
      },
      nemesis_next = {
         text = [["I let you get away the last time, %p.  This
time, I'll destroy you."]],
      },
      nemesis_other = {
         synopsis = "[Run away or you will suffer severely.]",
         output = "text",
         text = [["These meetings are starting to bore me.  You're eating into my precious
scheming time.

"If you don't run away now, I'm gonna make you suffer so bad that %l
will feel guilty for sending you here."]],
      },
      nemesis_wantsit = {
         synopsis = "[\"Return %o to me and we will rule %H.\"]",
         output = "text",
         text = [["You putz.  You don't even know how to use
%o.  Give it back to me and I'll teach you,
and together, we can rule %H.  And we really can!  I'm not just
saying that so you'll give me %o back, I
swear.  Thief's honor!

"But do it now, 'cause I'm running out of patience."]],	
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[Take %o and with %ds guidance, recover the Amulet.]",
         output = "text",
         text = [[%lC takes %o and
flexes it in his hand, then looks up and smiles.

"%pC, while you were away, %d appeared to me, and suggested I
transfer %o to you.  Carry it with you on
your quest for the Amulet of Yendor, and you'll travel in good company.
And I've had the curse taken off the silver bell -- no charge!

"Remember -- there are some things gold can buy.  For everything else,
there's %o."]],
      },
      offeredit2 = {
         synopsis = "[Keep %o and return to %Z through the portal.]",
         output = "text",
         text = [["%oC is yours.  Don't leave home without it!
The curse on the silver bell is gone, too.  %Z are
right on the other side of the magic portal you used to get here."]],
      },
      othertime = {
         text = [[You are back at %H.  Things are looking so dead,
you feel there may soon be no %cp here at all.]],
      },
      posthanks = {
         text = [["Just look at you, %p!  Tell me all about your Tour.  Did you
find the Amulet of Yendor yet?"]],
      },
   },
   Val = {
      assignquest = {
         synopsis = "[Find %i; defeat %n; return with %o.]",
         output = "text",
         text = [["It is not clear, %p, for my sight is limited without our relic.
But it is now likely that you can defeat %n, and recover
%o.

"A short time ago, %n and his minions attacked this place.  They
opened the huge volcanic vents you %x about the hill, and attacked.  I knew
that this was to come to pass, and had asked %d for a group of %gP
to help defend this place.  The few you %x here are the mightiest of
Valhalla's own, and are all that are left of one hundred %d sent.

"Despite the great and glorious battle we fought, %n managed at
last to steal %o.  This has upset the balance of the universe,
and unless %oh is returned into my care, %n may start Ragnarok.

"You must find the entrance to %i.  Travel downward
from there and you will find %ns lair.  Defeat him and
return %o to me, as well as the silver bell.  I must
remove the curse placed upon it so that you may retrieve
the Amulet of Yendor."]],
      },
      badalign = {
         synopsis = "[You have strayed from the %a path.  Return after you purify yourself.]",
         output = "text",
         text = [["NO!  This is terrible.  I see you becoming an ally of %n, and
leading his armies in the final great battles.  This must not come to
pass!  You have strayed from the %a path.  You must purge yourself,
and return here only when you have regained a state of purity."]],
      },
      badlevel = {
         synopsis = "[Come back when you are %Ra.]",
         output = "text",
         text = [["I see you and %n fighting, %p.  But you are not prepared and
shall die at %ns hand if you proceed.  No.  This will not do.
Go back out into the world, and grow more experienced at the ways of war.
Only when you have returned %Ra will you be able to defeat %n."]],
      },
      discourage = {
         "\"I am your death, %c.\"",
         "\"You cannot prevail, %r.  I can predict your every move.\"",
         "\"With you out of the way, Valhalla will be mine for the taking.\"",
         "\"I killed scores of %ds best when I took %o. Do you really think that one %c can stand against me?\"",
         "\"Who bears the souls of %cP to Valhalla, %r?\"",
         "\"No, %d cannot help you here.\"",
         "\"Some instrument of %d you are, %p.  You are a weakling!\"",
         "\"Never have I seen %ca so clumsy in battle.\"",
         "\"You die now, little %s.\"",
         "\"Your body I destroy now, your soul when my hordes overrun Valhalla!\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that I may lift the curse from the silver bell.\"",
         "\"Go with the blessings of %d.\"",
         "\"Call upon %d when you are in need.\"",
         "\"Use %o if you can.  It will protect you.\"",
         "\"Magical cold is very effective against %n.\"",
         "\"To face %n, you will need to be immune to fire.\"",
         "\"May %d strengthen your sword-arm.\"",
         "\"Trust in %d.  He will not desert you.\"",
         "\"It becomes more likely that Ragnarok will come with every passing moment. You must hurry, %p.\"",
         "\"If %n can master %o, he will be powerful enough to face %d far earlier than is fated.  This must not be!\"",
         "\"Remember your training, %p.  You can succeed.\"",
      },
      firsttime = {
         synopsis = "[You arrive below %H.  Something is wrong; there is lava present.]",
         output = "text",
         text = [[You materialize at the base of a snowy hill.  Atop the hill sits
a place you know well, %H.  You immediately realize
that something here is very wrong!

In places, the snow and ice have been melted into steaming pools of
water.  Fumaroles and pools of bubbling lava surround the hill.
The stench of sulphur is carried through the air, and you %x creatures
that should not be able to live in this environment moving towards you.]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Thank you %p for your sacrifice, I know this must have been
difficult to do. Rest assured, %d smiles down
upon you this day.  %o will be safe with us."]],
      },
      goal_first = {
         synopsis = "[This is the lair of %n.]",
         output = "text",
         text = [[Through clouds of sulphurous gasses, you %x a rock palisade
surrounded with a moat of bubbling lava.  You remember the description
from something that %l said.  This is the lair of %n.]],
      },
      goal_next = {
         text = "Once again, you stand in sight of %ns lair.",
      },
      gotit = {
         synopsis = "[You must return %o to %l.]",
         output = "text",
         text = [[As you pick up %o, your mind is suddenly filled with images
of the final battle and what could come to pass should Ragnarok
begin.  As you begin to control and channel your thoughts, you
realize that you must return %o to %l immediately.]],
      },
      gotit2 = {
         synopsis = "[You must return with %o.]",
         output = "text",
         text = [[As you pick up %o, your mind is suddenly filled with images
of the final battle and what could come to pass should Ragnarok
begin.]],
      },
      guardtalk_after = {
         "\"Hail, and well met, brave %c.\"",
         "\"May %d guide your steps, %p.\"",
         "\"%lC told us you had succeeded!\"",
         "\"You recovered %o just in time, %p.\"",
         "\"Hail %d, for delivering %o back to us.\"",
      },
      guardtalk_before = {
         "\"Hail, and well met, brave %c.\"",
         "\"May %d guide your steps, %p.\"",
         "\"%lC weakens.  Without %o, her foresight is dim.\"",
         "\"You must hurry, %p, else Ragnarok may well come.\"",
         "\"I would deal with this foul %n myself, but %d forbids it.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to %ds temple on the Astral Plane and offer it.]",
         output = "text",
         text = [["Excellent, %p.  I see you have recovered the Amulet!

"You must take the Amulet to the Great Temple of %d, on the Astral
Plane.  There you must offer the Amulet to %d.

"Go now, my %S.  I cannot tell you your fate, as the power of the
Amulet interferes with mine.  I hope for your success."]],
      },
      killed_nemesis = {
         synopsis = "[%nC dies.]",
         output = "text",
         text = [[A look of surprise and horror appears on %ns face.

    "No!!!  Loki has lied to me!  Ragnarok must come to pass!"

Suddenly, %n grasps his head and screams in agony, then dies.]],
      },
      leader_first = {
         synopsis = "[We need your aid.  Are you ready?]",
         output = "text",
         text = [["Ah, %p, my %S.  You have returned to %H
at last.  We are in dire need of your aid, but I must determine if you
are yet ready for such an undertaking.

"Let me read your fate..."]],
      },
      leader_last = {
         synopsis = "[\"Begone from my presence and never return.\"]",
         output = "text",
         text = [["No, %p.  Your fate is sealed.  I must cast about for another
champion.  Begone from my presence, and never return.  Know this, that
you shall never succeed in this life, and Valhalla is denied to you.
Until my last heartbeat, you shall never be allowed this task."]],
      },
      leader_next = {
         text = [["Let me read the future for you now, %p, perhaps you have managed to
change it enough..."]],
      },
      leader_other = {
         text = [["Again, I shall read your fate, my %S.  Let us both hope that you have
made changes to become ready for this task..."]],
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[As you approach, %l rises and touches %o.

"Outstanding, %p.  You have defeated %n and
recovered %o, and I am prepared to lift the curse
from the silver bell.  Alas, I fear that %o shall
never be truly safe unless it is kept here with
us at %H.  Ragnarok must not
come to pass.

Please %p, return %o to us now.]],
      },
      locate_first = {
         synopsis = "[This is the entrance to %i.]",
         output = "text",
         text = [[The ice and snow gives way to a valley floor.  You %x ahead of you
a huge round hill surrounded by pools of lava.  This then is the entrance
to %i.  It looks like you're not going to get in without
a fight though.]],
      },
      locate_next = {
         text = "Once again, you stand before the entrance to %i.",
      },
      nemesis_first = {
         synopsis = "[\"%oC has shown me that I must kill you.\"]",
         output = "text",
         text = [["So!  %lC has finally sent %ca to challenge me!

"I thought that using %o would enable me to start Ragnarok,
but I see that first I must kill you!  So come, little %s.
Once I defeat you, I can at last begin the final battle with %d."]],
      },
      nemesis_next = {
         text = "\"Again you challenge me, %r.  Good.  I will kill you now.\"",
      },
      nemesis_other = {
         text = "\"Have you not learned yet?  You cannot defeat %n!\"",
      },
      nemesis_wantsit = {
         text = "\"I will kill you, %c, and wrest %o from your mangled hands.\"",
      },
      nexttime = {
         text = "Once again, you are near the abode of %l.",
      },
      offeredit = {
         synopsis = "[Take %o.  Search for the Amulet.]",
         output = "text",
         text = [[As you approach, %l rises and touches %o.

"You may take %o with you, %p.  I have removed from
it the power to start Ragnarok, for that power no mortal should
have.  Its other abilities, however, you have at your disposal.

"You must now begin in %ds name to search for the Amulet of Yendor.
With the curse lifted from the silver bell, this should be a simple
task for someone of your ability.  May your steps be guided
by %d, my %S."]],
      },
      offeredit2 = {
         synopsis = "[You are %os keeper now.  Return through the portal and find the Amulet.]",
         output = "text",
         text = [["You are its keeper now, and the time has come to
resume your search for the Amulet.  With the curse lifted from the
silver bell, this should be a simple task for someone of your
ability.  %Z await your return through the magic portal
that brought you here."]],
      },
      othertime = {
         text = [[Again you materialize near %ls abode.  You have a nagging feeling
that this may be the last time you come here.]],
      },
      posthanks = {
         text = [["Greetings, %p.  I have not been able to pay as much attention to
your search for the Amulet as I have wished.  How do you fare?"]],
      },
   },
   Wiz = {
      assignquest = {
         synopsis = "[Travel to %i; overcome %n; return with %o.]",
         output = "text",
         text = [["Yes, %p, you truly are ready for this dire task.  Listen,
carefully, for what I tell you now will be of vital importance.

"Since you left us to hone your skills in the world, we unexpectedly came
under attack by the forces of %n.  As you know, we thought
%n had perished at the end of the last age, but, alas, this was
not the case.

"%nC sent an army of abominations against us.  Among them was a
minion, mindless and ensorcelled, and thus, in the confusion, it was
able to penetrate our defenses.  Alas, this creature has stolen
%o and I fear has delivered %oh to %n.
%nC is also in possession of the silver bell, and has
placed a curse upon it. Return it as well, so we can lift its curse,
which will then allow you to find the Amulet of Yendor.

"Over the years, I had woven most of my power into this amulet, and thus,
without it, I have but a shadow of my former power, and I fear that I
shall soon perish.

"You must travel to %i, and within its dungeons,
find and overcome %n, and return %o to me.

"Go now, with %d, and complete this quest before it is too late."]],
      },
      badalign = {
         synopsis = "[Go; come back when you are worthy of %d.]",
         output = "text",
         text = [["You amaze me, %p!  How many times did I tell you that the way of a mage
is an exacting one.  One must use the world with care, lest one leave it
in ruins and simplify the task of %n.

"You must go back and show your worthiness.  Do not return until you are
truly ready for this quest.  May %d guide you in this task."]],
      },
      badlevel = {
         synopsis = "[Go; return when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, you have not yet shown your proficiency as a worthy
spellcaster.  As %ra, you would surely be overcome in the challenge
ahead.  Go, now, expand your horizons, and return when you have attained
renown as %Ra."]],
      },
      discourage = {
         "\"Your puny powers are no match for me, fool!\"",
         "\"When you are defeated, your torment will last for a thousand years.\"",
         "\"After your downfall, %p, I shall devour %l for dessert!\"",
         "\"Are you ready yet to beg for mercy?  I could be lenient...\"",
         "\"Your soul shall join the enslaved multitude I command!\"",
         "\"Your lack of will is evident, and you shall die as a result.\"",
         "\"Your faith in %d is for naught!  Come, submit to me now!\"",
         "\"A mere %r is nothing compared to my skill!\"",
         "\"So, you are the best hope of %l?  How droll.\"",
         "\"Feel my power, %c!  My victory is imminent!\"",
      },
      encourage = {
         "\"Remember, you must return with %o so that we may lift the curse from the silver bell.\"",
         "\"Beware, for %n is immune to most magical attacks.\"",
         "\"To enter %i you must pass many traps.\"",
         "\"%nC may be vulnerable to physical attacks.\"",
         "\"%d will come to your aid when you call.\"",
         "\"You must utterly destroy %n.  He will pursue you otherwise.\"",
         "\"%oC is a mighty artifact.  With it you can destroy %n.\"",
         "\"Go forth with the blessings of %d.\"",
         "\"I will have my %gP watch for your return.\"",
         "\"Feel free to take any items in that chest that might aid you.\"",
         "\"You will know when %o is near.  Proceed with care!\"",
      },
      firsttime = {
         synopsis = "[You have arrived at %ls tower but something is very wrong.]",
         output = "text",
         text = [[You are suddenly in familiar surroundings.  You notice what appears to
be a large, squat stone structure nearby.  Wait!  That looks like the
tower of your former teacher, %l.

However, things are not the same as when you were last here.  Mists and
areas of unexplained darkness surround the tower.  There is movement in
the shadows.

Your teacher would never allow such unaesthetic forms to surround the
tower...  unless something were dreadfully wrong!]],
      },
      gaveitup = {
         synopsis = "[\"You give %o to %l.\"]",
         output = "text",
         text = [["Thank you %p for your sacrifice, I know this must have been
difficult to do. Rest assured, %d smiles down
upon you this day.  %o will be safe with us."]],
      },
      goal_alt = {
         text = "You have returned to %ns lair.",
      },
      goal_first = {
         text = "You feel your mentor's presence; perhaps %o is nearby.",
      },
      goal_next = {
         text = "The aura of %o tingles at the edge of your perception.",
      },
      gotit = {
         synopsis = "[You feel %os power and know you should return %oh to %l.]",
         output = "text",
         text = [[As you touch %o, its comforting power infuses you
with new energy.  You feel as if you can detect others' thoughts flowing
through it.  Although you yearn to wear %o and
attack the Wizard of Yendor, you know you must return it to its rightful
owner, %l.]],
      },
      gotit2 = {
         synopsis = "[You feel %os power.]",
         output = "text",
         text = [[As you touch %o, its comforting power infuses you
with new energy.  You feel as if you can detect others' thoughts flowing
through it.  You yearn to wear %o and show
the Wizard of Yendor what a true wizard looks like.]],
      },
      guardtalk_after = {
         "\"I have some eye of newt to trade, do you have a spare blind-worm's sting?\"",
         "\"The magic portal now seems like it will remain stable for quite some time.\"",
         "\"Have you noticed how much stronger %l is since %o was recovered?\"",
         "\"Thank %d!  We weren't positive you would defeat %n.\"",
         "\"I, too, will venture into the world, because %n was but one of many evils to be vanquished.\"",
      },
      guardtalk_before = {
         "\"Would you happen to have some eye of newt in that overstuffed pack, %s?\"",
         "\"Ah, the spell to create the magic portal worked.  Outstanding!\"",
         "\"Hurry!  %lC may not survive that casting of the portal spell!\"",
         "\"The spells of %n were just too powerful for us to withstand.\"",
         "\"I, too, will venture into the world, because %n is but one of many evils to be vanquished.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to %ds altar on the Astral Plane.]",
         output = "text",
         text = [["Congratulations, %p.  I always knew that if anyone could succeed
in defeating the Wizard of Yendor and his minions, it would be you.

"Go now, and take the Amulet to the Astral Plane.  Once there, present
the Amulet on the altar of %d.  Along the way you shall pass through
the four Elemental Planes.  These planes are like nothing you have ever
experienced before, so be prepared!

"For this you were born, %s!  I am very proud of you."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[%nC, whose body begins to shrivel up, croaks out:

    "I shall haunt your progress until the end of time.  A thousand
    curses on you and %l."

Then, the body bursts into a cloud of choking dust, and blows away.]],
      },
      leader_first = {
         synopsis = "[You have come a long way, but are you ready for the task I require?]",
         output = "text",
         text = [["Come closer, %p, for my voice falters in my old age.
Yes, I see that you have come a long way since you went out into the
world, leaving the safe confines of this tower.  However, I must first
determine if you have all of the skills required to take on the task
I require of you."]],
      },
      leader_last = {
         synopsis = "[\"Get out of here!\"]",
         output = "text",
         text = [["You fool, %p!  Why did I waste all of those years teaching you
the esoteric arts?  Get out of here!  I shall find another.  As long
 as I exist, you'll never be granted a chance to complete this task."]],
      },
      leader_next = {
         text = "\"Well, %p, you have returned.  Perhaps you are now ready...\"",
      },
      leader_other = {
         text = [["This is getting tedious, %p, but perseverance is a sign of a true mage.
I certainly hope that you are truly ready this time!"]],
      },
      leader_wantsit = {
         synopsis = "[\"%lC asks you to forfeit %o.\"]",
         output = "text",
         text = [[%lC notices %o in your possession,
looks at you and says:

"Well done, %p.  You have defeated %n and
recovered %o, and we are ready to lift the curse
from the silver bell.  Alas, I fear that
%o shall never be truly safe unless
it is kept here with us at %H.

Please %p, return %o to us now.]],
      },
      locate_first = {
         text = "Wisps of fog swirl nearby.  You feel that %ns lair is close.",
      },
      locate_next = {
         text = "You believe that you may once again invade %i.",
      },
      nemesis_first = {
         synopsis = "[\"Your destruction should make for good sport.\"]",
         output = "text",
         text = [["Ah, I recognize you, %p.  So, %l has sent you to steal
%o from me, hmmm?  Well, %lh is a fool to send such
a mental weakling against me.

"Your destruction, however, should make for good sport.  In the end, you
shall beg me to kill you!"]],
      },
      nemesis_next = {
         synopsis = "[\"Your soul shall soon be mine to command.\"]",
         output = "text",
         text = [["How nice of you to return, %p!  I enjoyed our last meeting.  Are you
still hungry for more pain?

"Come!  Your soul, like %o, shall soon be mine to command."]],
      },
      nemesis_other = {
         text = [["I'm sure that your perseverance shall be the subject of innumerable
ballads, but you shall not be around to hear them, I fear!"]],
      },
      nemesis_wantsit = {
         text = [["Thief!  %oC belongs to me, now.  I shall feed
your living flesh to my minions."]],
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[Take %o with you in your quest for the Amulet.]",
         output = "text",
         text = [[%lC notices %o in your possession,
beams at you and says:

    "I knew you could defeat %n and retrieve
    %o.  We shall never forget this
    brave service.

    "Take %oh with you in your quest for the Amulet of Yendor.
    I can sense that it has attuned %oiself to you already.
    We have also removed the curse from the silver bell.

    "May %d guide you in your quest, and keep you from harm."]],
      },
      offeredit2 = {
         synopsis = "[Keep %o, return through the portal to %Z; find the other Amulet.]",
         output = "text",
         text = [["You are the keeper of %o now.  We have also removed the
curse from the silver bell.  It is time to recover the
/other/ Amulet.  %Z await your return
through the magic portal which brought you here."]],
      },
      othertime = {
         text = [[You are back at %H.
You have an odd feeling this may be the last time you ever come here.]],
      },
      posthanks = {
         text = [["Come near, my %S, and share your adventures with me.
So, have you succeeded in your quest for the Amulet of Yendor?"]],
      },
   },
}
