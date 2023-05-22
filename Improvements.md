# Improvements

- Send back a "confirmation byte" from the arduino when the lights have successfully been updated
    * Hopefully this resolves some small color issues which can be resolved by pressing the reset button the arduino (not ideal)
- Setup a socket (probably in /run/user/{uid here}/) to handle a more direct approach to interfacing with this daemon
- Wait for serial port to be available and restart and wait if the connection to the arduino is lost
- In general, I want to move this project to be more of a system service
