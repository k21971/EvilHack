## EvilHack

EvilHack is a NetHack variant that is designed to be a much more challenging
experience that the original, drawing inspiration from and incorporating some
of the best features from variants such as GruntHack and SporkHack, as well
as other interesting bits of code from other variants (Slash'EM, Splicehack,
UnNetHack, xNetHack).  Wrap all that up along with some custom/unique content
never seen before in any other variant, and you have EvilHack.

EvilHack was initially built off of the NetHack 3.6.2 codebase, and will be
updated accordingly as NetHack 3.6.x progresses and evolves.

This variant is designed to be difficult, much like how GruntHack and SporkHack
turned out to be (hence the name 'EvilHack').  It is not impossible to win by
any means, but several aspects of the game that one might take for granted in
the 'vanilla' version of NetHack can easily cost you your game in this variant.
Various monsters are tougher, have more hit points, can fight more intelligently,
and can use a variety of objects against you that previously only the player
could use.

The EvilHack changelog is updated regularly and is an excellent resource to
access for viewing current changes to existing releases, as well as up and
coming changes for future versions -
https://github.com/k21971/EvilHack/blob/master/doc/evilhack-changelog.txt

More information regarding this variant can be accessed at the NetHackWiki -
https://nethackwiki.com/wiki/EvilHack - or by visiting channels #evilhack
or #hardfought on Freenode IRC.

## Design Philosophy

As stated before, the overall design goal for EvilHack is that it's a more
difficult and challenging game than vanilla NetHack or other variants,
*without* sacrificing game balance or (most importantly) the game's fun-factor.

In short:
- EvilHack is meant to be very difficult and challenging
- EvilHack is meant to be fun, even though it is difficult
- Any changes should be balanced
- Replayablilty/randomness is a priority design goal to ensure unpredictable  
  outcomes and to keep the experience fresh
- Addressing bugs/balance issues will always be an ongoing process
- Player feedback/constructive criticism is welcome and encouraged

## Installation

Each OS type found under the `sys` folder has an installation guide for that
particular operating system. Pre-compiled binaries (linux and windows) can be
found here - https://github.com/k21971/EvilHack/releases

For Linux (TL;DR version):
- From the desired directory, `git clone https://github.com/k21971/EvilHack.git`
- Navigate to the `sys/unix` folder, then `./setup.sh hints/linux-debug`  
  (edit the install paths in the hints file to your liking)
- Navigate back to the root EvilHack folder, and `make all && make install`
- Execute the `evilhack` binary

