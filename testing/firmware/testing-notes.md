# Testing Notes

Notes from firmware tests

## 29 Nov

- Removed TimeOnAir global variable

_SETUP_

We had 4 nodes set up with different names scattered through TEC229. We had devices connected to each node's Wi-Fi trying to send messages.

_RESULTS_

- Took a while to get going. Messages weren't being recieved.
    - Discovered that if the channel was busy, a flag wasn't being reset causing an infinite loop. fixed that.
    - Discovered that clearOnExit in the xEventGroupSetBits() call wasn't clearing the event bit after leaving, even though it was set to true. Have to explicitly call xEventGroupClearBits(EVENTBIT) after returning from wait.
- Lots of packet loss. Very inconsistent which nodes recieved which messages.
- The retry mechanism sucked. Truly sucked. Replace with timer retry
- Had issues with messages being sent twice.

_TODO_

- Fix the retry. Add a ticker to each message object? should be a timeout rather than a spot in the log list that determines when to retry a message.
- Lets get rid of doing the lora protocol in the lora task. Make another task to do this called protocol or something. Lora task should be highest priortiy, but should never doing the channel checking. that should be protocol task. Lora task should only deal with hardware.
- Enter critical section when dealing with lora hardware. write a class extension of startTransmit and startRecieve for the radio that uses a critical section and strips out all the GFSK/LRFHSS stuff.
