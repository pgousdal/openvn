=== start ===
#openvn scene lighthouse_storm
#openvn music storm
The storm is closing in around the lighthouse.
#openvn show erik neutral

* [Answer the radio] -> radio
* [Inspect the generator] -> generator

=== radio ===
#openvn sound radio
A distress call breaks through the static.
#openvn hide erik
-> ending_radio

=== generator ===
The emergency generator starts after a struggle.
#openvn music stop
-> ending_generator

=== ending_radio ===
You guide a fishing boat to safety.
-> END

=== ending_generator ===
The light stays on through the night.
-> END
