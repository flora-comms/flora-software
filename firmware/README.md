# Petal Radio firmware

Things to do:

## power monitor task.
  - gets event when the websocket starts, times out, or the last client disconnects and starts a timer.
  - after timer expires, clean up the webserver task.
  - go into modem sleep
  - make sure everything is cleared from the queues and the lora task isn't dealing with anything.
  - set up wakeup sources and go to sleep.
  - when woken up, find the source and deal with it as appropriate. 
## led task
  - toggle led colour based on device status every second or so. 
