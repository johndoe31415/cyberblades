# pibeatsaber
[BeatSaber](https://beatsaber.com) is a brilliant, fantastic VR game
([Steam link](https://store.steampowered.com/app/620980/Beat_Saber/)). The community
has created a number of plugins for it, my favorite of which is
[HttpStatus](https://github.com/opl-/beatsaber-http-status) created by
[opl-](https://github.com/opl-). It opens up a web server that serves a
WebSockets stream of data which provides all kinds of data about the running
game.  The reason I love their plugin so much is beacuse
[opl-](https://github.com/opl-) understands *exactly* what kind of information
would be important to export. It is perfectly documented, easily understood,
just a total pleasure to work with.

This is where pibeatsaber comes in: It builds a historian application that
connects to BeatSaber/HttpStatus. Via a simple UI (this is tbd) you can select
multiple people (this is not something that is yet implemented in BeatSaber and
it's kindof annoying, because we usually have multiple people playing and don't
want to sign out/sign in every time).

The historian then essentially (for now) records all raw event data that
HttpStatus provides and writes them to files.

## Where is this going?
I love data and I love analytics. I want to see how I improved in songs over
time. I'm looking to hook up a heart rate monitor to my Raspberry Pi and enrich
the data with the heart rate of the current player. I want to create plots,
plots, plots of songs over time, improvement of performance of the same song
over time, etcetc. I want the Pi to emit data in various electronic hardware
elements (e.g., LED ring progress meters or similar).

That said, it's in a very early development stage and currently not meant yet
for public use. Bear with me, please.

## Raspberry Pi Configuration
I'm using a Raspberry Pi 3B+ with an ILI9340 display board that uses the
fb_ili9340 kernel driver. /boot/config.txt has been extended by:

```
dtoverlay=waveshare32bOC:rotate=270
```

## Acknowledgements
[opl-](https://github.com/opl-) -- you totally rock. Thank you so much for
building your amazing BeatSaber plugin and making it possible for non-C# people
to build cool stuff.

## License
GNU GPL-3
