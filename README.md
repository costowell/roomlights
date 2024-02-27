# Roomlights

Roomlights does exactly what the name suggests and controls the individually addressable RGB LEDs strung up around my room. This project was primarily so that I could listen to music and have my lights respond accordingly. My ambition grew as I wanted to be able to switch between some lights modes and so here we are.

## Features

* Light modes can be easily added and switched through using signals
    * e.g. `pkill -RTMIN+7 roomlights` selects the 8th light mode (7th index in the light modes array)
* Light modes include:
    * *Sound responsive* - connects to the Pulseaudio server on your machine and makes the lights act like an equalizer
    * *Wave* - Sends a wave of blue across all the lights
    * *Slow clear* - Slowly clears the lights (a very fancy way to end your music jam session)

## What to Know

This arduino code is stored in `arduino/` and the code you run on your machine is stored in `computer/`.

## Future

By the time that I had gotten this project to a stable state I wanted more features, hence the `Improvements.md`. However, I know that the scope of what I wanted warrants a higher level language as I do not want to handle the headache of a project that would be to write in C. I am currently in the process of implementing a more system service approach that will be more robust and cover a wider variety of use cases. The biggest issue of the bunch is that you must run the program as a regular user if you're going to connect to a Pulseaudio server, meaning that if you want the lights to be on in the background doing some preprogrammed animation, you must always be logged in. At some point, I will have something worth sharing and update this file to contain the new repository.
