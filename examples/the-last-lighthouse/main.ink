=== start ===
#openvn scene lighthouse_storm
#openvn music storm
#openvn sound thunder
The windows tremble as the storm reaches the lighthouse.
Erik has kept this light for twenty-seven years. Tonight, every ship in the channel depends on it.
#openvn show erik neutral
The radio crackles. At the same moment, the generator coughs and the beam fades.

* [Answer the radio first] -> radio_room
* [Run to the generator] -> generator_room

=== radio_room ===
#openvn scene radio_room
#openvn sound radio
Anna's voice breaks through the static from the rescue station.
"Erik, we have three calls. Jonas is taking water. Mina is trapped above the north cliff. The supply boat has lost the channel."
The signal is weak, and the lighthouse battery will not last through the night.

* [Guide Jonas toward the harbour] -> help_jonas
* [Guide Mina down from the cliff] -> help_mina
* [Use the beacon to relay all three positions] -> relay_plan

=== help_jonas ===
#openvn show jonas radio
Jonas answers between bursts of static. His engine is dead and the sea is pushing him toward the reef.
Erik gives him the old bearing used before the modern buoy was installed.
#openvn sound beacon
The lighthouse beam returns for one slow sweep, enough for Jonas to see the harbour mouth.

* [Keep the beam on Jonas until he is safe] -> ending_jonas
* [Cut the beam and save the remaining power] -> ending_light

=== help_mina ===
#openvn show mina radio
Mina is sheltering beside an abandoned signal hut. The path below her has collapsed.
Erik remembers a wartime stair carved into the eastern rock.
#openvn sound thunder
Another strike hits close enough to turn the radio white with noise.

* [Talk Mina down the hidden stair] -> ending_mina
* [Leave the lighthouse and cross the cliff yourself] -> ending_sacrifice

=== relay_plan ===
#openvn music tension
Erik connects the emergency transmitter directly to the beacon controller.
The old system was never designed for this. If it works, every receiver in the channel will hear the same bearings.
#openvn show anna radio
Anna warns him that the relay may drain the battery and burn out the lamp motor.

* [Transmit the three positions and trust Anna] -> relay_success
* [Abort and preserve the lighthouse] -> ending_light

=== relay_success ===
#openvn sound beacon
The transmitter carries Jonas, Mina, and the supply boat on one narrow pulse.
Anna repeats each bearing. The rescue crew divides without hesitation.
The beacon motor begins to smoke.

* [Shut the system down before it burns] -> ending_all
* [Hold the transmission until every rescue confirms] -> ending_sacrifice

=== generator_room ===
#openvn scene generator_room
#openvn sound generator
The generator room smells of oil and hot copper.
One fuel line has split. Erik can bypass it, but the repair will take most of the emergency reserve.
The radio calls continue upstairs.

* [Repair the generator completely] -> generator_repaired
* [Start it briefly and return to the radio] -> short_power
* [Abandon the machine and light the manual lamp] -> manual_lamp

=== generator_repaired ===
#openvn music tension
The generator catches and the great lens turns again.
For the first time all night, the lighthouse has full power—but the radio has gone silent.
Erik searches the channel and sees one small light near the reef.

* [Use the beam to guide the fishing boat] -> ending_jonas
* [Sweep the north cliff for Mina] -> ending_mina

=== short_power ===
#openvn scene radio_room
#openvn sound radio
Erik gives the generator five minutes, then returns to the radio.
Anna has enough power to hear him, but not enough time to dispatch two crews.

* [Send Anna to Jonas] -> ending_jonas
* [Send Anna to Mina] -> ending_mina
* [Ask Anna to trust an improvised relay] -> relay_success

=== manual_lamp ===
#openvn scene lighthouse_storm
#openvn music stop
Erik climbs into the lantern room with the old paraffin reserve.
The manual lamp is small, but its light is steady.
Outside, the storm swallows every sound except the sea.

* [Stay with the lamp until dawn] -> ending_light
* [Carry the lamp to the cliff path] -> ending_sacrifice

=== ending_all ===
#openvn scene dawn
#openvn music dawn
#openvn hide erik
At dawn, three signals arrive in succession.
Jonas is inside the harbour. Mina is aboard Anna's rescue boat. The supply vessel has found the channel.
The lighthouse is dark, but everyone who followed its final message is alive.
ENDING: The Last Signal
-> END

=== ending_jonas ===
#openvn scene harbour_dawn
#openvn music dawn
Jonas reaches the harbour moments before the generator fails.
Mina is rescued later by the cliff team, but the supply boat turns back to sea.
Erik watches the fishing boat tie up beneath the first grey light.
ENDING: Safe Harbour
-> END

=== ending_mina ===
#openvn scene cliff
#openvn music dawn
Mina reaches the eastern stair and meets Anna's crew below the cliff.
Jonas loses his boat on the reef, but he is found alive at sunrise.
The lighthouse beam remains dark for the rest of the night.
ENDING: The Hidden Stair
-> END

=== ending_light ===
#openvn scene lighthouse_dawn
#openvn music dawn
Erik protects the remaining power and keeps the beacon turning until morning.
Ships beyond the channel see the light and wait offshore.
Not every distress call is answered, but no vessel strikes the reef.
ENDING: The Light Endures
-> END

=== ending_sacrifice ===
#openvn scene cliff
#openvn music dawn
Erik leaves the safety of the tower and disappears into the storm.
His final transmission gives Anna the bearings she needs.
By morning, the others are safe. Erik's coat is found beside the old cliff stair.
ENDING: Keeper of the Storm
-> END
