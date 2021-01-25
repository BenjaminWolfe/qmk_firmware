# Intro

This keymap is for the Kyria split, column-staggered keyboard
designed by Thomas Baart of https://www.splitkb.com.
I use mine primarily on a Mac, so it's very much optimized for that,
and use a lot of VS Code and Chrome in my work as a data scientist,
so you'll see a lot of that in here as well.

# Notes

I use the Colemak DHm keymap as my base: https://colemak.com.
I considered using modifiers in my home row throughout (so-called home-row modifiers),
but they just weren't for me.
I would rather have dedicated keys for modifiers.

I did use home-row modifiers, though, in the "nav" layer,
where they cannot interfere with typing or confuse me.
My exploration of HROs was influenced by this article:
https://precondition.github.io/home-row-mods.

I noticed that using home-row modifiers with Colemak presents a unique challenge.
The common issue with HROs is preventing rollover issues,
the keyboard registering a modifier when you were only typing fast.
Rollover issues are magnified in Colemak, where the main 8 keys of the home row
(a, r, s, t, n, e, i, and o) are some of the most quickly typed letters.

Instead I used the thumb for all but control.
On a Mac, control is just not used very often.

On a Mac, several letters can be held down for extra diacritical marks:
`luy` (top row, RH), `asneio` (home row), and `zc` (bottom row, LH).
In an earlier version I tried to salvage this functionality;
currently it's impossible to use.
I will look for a way to add it later if it becomes important to me.

The Kyria has no number row, so I needed a place to put the numbers and symbols
that are usually in that row.
To preserve and take advantage of muscle memory, I put symbols in a dedicated layer,
one in which the top row is those keys instead of the top row of letters.
Numbers I put in a layer mimicking the traditional number pad,
adding several common symbols as well on the left hand.

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

The Kyria has a dizzying total of 7 thumb keys on each hand:
5 in a bottom row and 2 in a row above the central 2 bottom keys.
I try not to use the keys that require the most movement in toward the other fingers.
The designer himself told me he's originally intended the thumbs to rest
in the second thumb key from the center of the layout,
so that's what I've chosen to do.
I've put the space bar on the right hand where I tend to use it,
and the backspace on the left hand.
Tab and delete are thematically organized next to space and backspace, respectively.
I originally had them reversed to match historical handedness,
but I decided the elegance was worth the departure from precedent.
You can also access space, tab, and shift tab from the left hand using the hold layer.
This is useful when I want to be able to use them all with my left hand alone.

Other layers include unicode (designed for Mac input), adjustment, and nav,
with extra nav layers specifically for Chrome and VSCode tab navigation.
The `adjust` layer is something of a catch-all right now, with the function keys,
media controls, and underglow lighting adjustment.

The same key is used for both backtick and escape;
just hold it a little longer for the latter.

I would love to do more with the underglow lighting;
I'm currently leaving that simple for file size reasons.
I particularly like the idea of visual feedback for caps locks and layer changes;
color works well for that since you can see it with your peripheral vision.
Oddly that worked at one point, and then it didn't, and I don't remember when.
I do have nice functionality in place for changing the color and getting its numbers,
thanks in large part to [this StackOverflow post][lighting-stackoverflow].

At one point I used [tapdance][tapdance] for all kinds of fancy functionality
in the nav layer:
a key for cut, copy, and paste,
another for undo and redo,
and a third for find and replace.
None of these added enough value though
(especially not cut, copy, and paste, as the shortcuts are already so easy to find),
and removing them drastically improved my firmware size (by more than a kilobyte!).
I'm currently tinkering with new ways to handle undo-redo and find & replace.

At one point I used a "hold" layer to be able to hold space, tab, backspace and delete
without activating the hold equivalent of each.
Then I realized that on the default settings I could tap + hold them and it would work.

# Open Tasks and Ideas

* Continue tinkering with the placement of shortcut keys.
* Consider auto-shift; nothing wrong with using that _and_ the modifier key.
* Consider using aliases for keys as outlined in the home row modifiers article.
  It certainly makes the code easier to read.
* More intentional arrangement of RGB, media, and function keys.
* Reduce file size to be able to use the leader key functionality.
* Add emoji support, in some way?
* Get back my unicode 2212 key?
* Write out anything else worth documenting?

# Ecosystem

* Get a longer TRRS cable so as to be able to move the halves of the keyboard
  farther apart.
* Figure out how to actually place my hands farther apart at my desk…
  like a very wide keyboard tray.
* Most importantly, be very sure I follow ergonomic best practices.

[lighting-stackoverflow]: https://stackoverflow.com/questions/65556317/qmk-rgb-saturation-bottoms-out
[tapdance]: https://beta.docs.qmk.fm/using-qmk/software-features/feature_tap_dance
