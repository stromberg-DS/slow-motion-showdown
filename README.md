# slow-motion-showdown
Daniel Stromberg

Slow Motion Showdown is simple 2-player game where you try to press your button before your opponent
BUT, you cannot set off your motion detector in the process.

This game is simple, two players each have a large button in front of them that they are trying to
press before the other. Facing towards each player's button is a motion sensor. If you hit your
button, you win the round (and lots of points), but if you set off your motion detector, you end
the round (and give your opponent points).

The game is run on a Particle Photon 2 that detects the button presses and motion sensor inputs. 
The Photon 2 lights up the arcade buttons as well as edge-lighting the acrylic wall to provide
feedback to the players. Each player has a small OLED screen that gives them information about
what to do.

The game has an optional manual smart room controller mode that is activated with a toggle switch.
When switched, the attached encoder can change the color and brightness of the Hue bulbs in 
the room.