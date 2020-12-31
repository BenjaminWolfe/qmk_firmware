# Notes

The Kyria is a split, column-staggered keyboard
designed by Thomas Baart of https://www.splitkb.com.
I use the Colemak keymap as my base: https://colemak.com.
I chose to put most of my modifiers in the home row ("home-row modifiers").
The specific HRM configurations I used were heavily influenced by this article:
https://precondition.github.io/home-row-mods.
Between my Kyria, Colemak, and home row modifiers,
my aim is the most natural finger positioning and the least movement possible.

Using home-row modifiers with Colemak presents a unique challenge.
The common issue with HROs is preventing rollover issues,
the keyboard registering a modifier when you were only typing fast.
Rollover issues are magnified in Colemak, where the main 8 keys of the home row
(a, r, s, t, n, e, i, and o) are some of the most quickly typed letters.
I even considered using another row entirely to avoid the problem,
but in the end I decided not to try that idea.
Using another row seemed to nullify much of the benefit of home-row modifiers.

I did, however, move the `control` key down one row.
It's simply not used very much on a Mac, which is what I prefer to use.
That avoids one rollover issue in particular:
Control-T on a Mac transposes letters!

In its place I added a modifier to get to a "safe" layer,
which is just like the default layer but has no modifiers.
On a Mac, several letters can be held down for extra diacritical marks:
`luy` (top row, RH), `asneio` (home row), and `zc` (bottom row, LH).
Using the letters as modifiers suppresses that functionality.
On the safe layer these diacritical marks are available as intended.

The Kyria has no number row, so I needed a place to put the numbers and symbols
that are usually in that row.
To preserve and take advantage of muscle memory, I put them in a dedicated layer,
one in which the top row is those keys instead of the top row of letters.

I use a mouse quite a bit.
So it's important to design not just for the usual two-handed experience,
but also for the one in which the right hand (in my case) is on the mouse.
For this reason I've moved the Command key (most common for shortcuts on a Mac)
down from the home row to the left thumb, which is where I usually use it.
This allows me to do any Command-Letter combo on the left hand easily,
including even those that would require the left forefinger.
As it so happens, most important shortcuts are designed to be done with the left hand,
and Colemak tends to maintain QWERTY's handedness.

Removing Command from the home row opens up a space for another modifier key;
in that place I've put the modifier for the symbol layer.
(Also if you hold down a letter with optional diacritical marks from the safe layer,
it will momentarily switch to the symbols layer automatically.)

All told, my home row modifier order, adapted from the article mentioned above,
is safe layer, option, shift, symbol layer,
with command placed prominently at the left thumb,
and control relegated to the Z and forward slash positions.

It's popular among keyboarding enthusiasts to argue
that caps lock should not have a place in the home row.
I disagree.
I use caps lock anytime I have more that one capital letter in a row, honestly.
It just seems more efficient to me that way.
So I've retained caps lock in its traditional place, on the left next to the A key.
I've considered using QMK's auto-shift functionality,
but I'm waiting till I get my current layout down before I consider trying that.

QMK allows layers to be activated a number of ways.
I generally prefer to activate them by pressing a key;
then they're active as long as that key is pressed.
If you want to toggle the layer on and off instead,
hold caps lock down and tap the layer's modifier key.
I've created a "switch" layer just for that sake,
to make it easy to toggle other layers on and off.
For symmetry, you can also activate the swith layer by holding the apostrophe key.

The Kyria has a dizzying total of 7 thumb keys on each hand:
5 in a bottom row and 2 in a row above the central 2 bottom keys.
I try not to use the keys that require the most movement in toward the other fingers.
The designer himself told me he's originally intended the thumbs to rest
in the second thumb key from the center of the layout,
so that's what I've chosen to do.
I've put the space bar on the right hand where I tend to use it,
and the backspace on the left hand.
For now, tab and delete are one position out on the left and right hands, respectively.
(It seems slightly more intuitive to swap those, though.)
You can also reverse the handedness of these keys by switching to the symbols layer.
This is useful when I want to be able to use them all with my left hand alone.

Other layers include numpad, unicode (designed for Mac input), adjustment, and nav,
with extra nav layers specifically for Chrome and VSCode tab navigation.
The `adjust` layer is something of a catch-all right now, with the function keys,
media controls, and underglow lighting adjustment.

The same key is used for both backtick and escape;
just hold it a little longer for the latter.

# Open Tasks and Ideas

* Keep tinkering with positioning of thumb keys:
  space, delete, backspace, and tab.
* Consider auto-shift; nothing wrong with using that _and_ the modifier key.
* Consider using aliases for keys as outlined in the same article mentioned above.
  It certainly makes the code easier to read.
* Command-shift-x for 1Password?
* Nicer use of layer colors. Make it work in the first place!
* More intentional arrangement of RGB, media, and function keys.
  Consider a way to print current color after querying HSV values:
  https://beta.docs.qmk.fm/using-qmk/hardware-features/lighting/feature_rgblight#query
* Reduce file size to be able to use the leader key functionality.
* Add emoji support, in some way?
  Consider even a smile, which should be an easy 2-key adjacent hop.
* Get back my unicode 2212 key?
* Write out anything else worth documenting?

# Ecosystem

* Get a longer TRRS cable so as to be able to move the halves of the keyboard
  farther apart.
* Figure out how to actually place my hands farther apart at my desk…
  like a very wide keyboard tray.
* Most importantly, be very sure I follow ergonomic best practices.
