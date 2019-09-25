# CyberBlades
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

This is where CyberBlades comes in: It is a historian application that connects
to BeatSaber/HttpStatus. Via a simple UI (this currently is emulated via SDL
for development and against libcairo/framebuffer) you can select multiple
people (this is not something that is yet implemented in BeatSaber and it's
kindof annoying, because we usually have multiple people playing and don't want
to sign out/sign in every time).

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

## Installation Instructions for Raspberry Pi
I'm using a Raspberry Pi 3B+ with a noname (AliExpress) 2.4" ILI9341 320x240
pixel display board. Here are the steps to set it up:

  - dd the Raspbian Lite image on your microSD card. I've tested it with
    `2019-07-10-raspbian-buster-lite.img`.
  - Touch `/boot/ssh` to activate the OpenSSH server once you turn your Pi on
    (or attach a keyboard/monitor otherwise).
  - Boot the Pi for the first time, change the default credentials and put your
    ssh public key into `/root/.ssh/authorized_keys` on the Pi.
  - Log into the Pi and install some dependencies: `apt-get intstall git cmake build-essential`
  - Get the display to run. For my case, I used the
    [fbcp-ili9341](https://github.com/juj/fbcp-ili9341) driver. I built it with
    ```
	$ cmake -DSPI_BUS_CLOCK_DIVISOR=6 -DILI9341=ON -DGPIO_TFT_DATA_CONTROL=22 -DGPIO_TFT_RESET_PIN=27 -DSTATISTICS=0 ..
    ```
    and configured my `/boot/config.txt` like this:
    ```
    hdmi_group=2
    hdmi_mode=87
    hdmi_cvt=320 240 60 1 0 0 0
    hdmi_force_hotplug=1
    ```
  - Reboot the Pi, then start the fbcp-ili9341 application. If your display
    driver works, while that application is running you should now be able to
    do
    ```
    dd if=/dev/zero of=/dev/fb0
    ```
    to clear the display or 
    ```
    dd if=/dev/urandom of=/dev/fb0
    ```
    to show some random garbage.

## Acknowledgements
  - [opl-](https://github.com/opl-) -- you totally rock. Thank you so much for
    building your amazing BeatSaber plugin and making it possible for non-C# people
    to build cool stuff.
  - [juj](https://github.com/juj) -- you too! What a great and
    simple-to-use display driver for the Raspberry Pi, so incredibly well
    documented. Works like a charm, thanks!

## What's with the name?
Once my wife couldn't remember the name of BeatSaber and called it
"CyberBlades". I love the name and we use it all the time now as an inside
joke. So now CyberBlades is something "official" :-)

## Used other work
  - Font "Beon" by Bastien Sozeau (contact@sozoo.fr) used in accordance with
    the SIL OPEN FONT LICENSE Version 1.1

## License
GNU GPL-3 for all of my code, external dependencies under their respective
licenses.
