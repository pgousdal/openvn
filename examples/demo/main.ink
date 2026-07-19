=== start ===
#openvn scene room
#openvn music intro
#openvn show alice neutral
#openvn set_bool terminal_online true
#openvn set_int trust 2
#openvn if terminal_online
The terminal responds.
#openvn else
The terminal remains dark.
#openvn end
#openvn if trust >= 2
Alice decides to continue.
#openvn else
Alice steps away.
#openvn end
Alice switches on the old terminal. The OpenVN demo is running on one shared story.
#openvn sound click
The screen asks which path to demonstrate.

* [Show the hopeful ending] -> hopeful
* [Show the quiet ending] -> quiet

=== hopeful ===
Alice smiles as the scene, portrait, choice, music, and sound complete one vertical slice.
#openvn hide alice
#openvn music stop
ENDING: A New Story
-> END

=== quiet ===
Alice lets the music play for one final moment before closing the terminal.
#openvn hide alice
#openvn music stop
ENDING: The Last Note
-> END
