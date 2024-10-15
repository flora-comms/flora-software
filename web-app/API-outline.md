# API Outline

Outlines the functionality of the APIs for the chat server

# JSON Objects

JSON strings used for comms and control

## Message Tx/Rx

{
    Type: "Msg"
    Payload: char[255] (254 chars plus \0)
    NodeID: byte
}

## Message History
file name: history.JSON

{
    "Type": "History",
        "Messages": [
         {
             "Type": "Msg",
             "Payload": "Hello, world!",
             "NodeID": 1
         },
         {
             "Type": "Msg",
             "Payload": "How are you?",
             "NodeID": 0

         },
     ]
} 

## ACK

{
    Type: "ACK"
}

## Populate - client initiated

Gets previous chats to populate chat box.
Occurs when user starts the app or scrolls to the top of the chat box/wants more message history.

### Client side

1. Determine what messages to request (i.e. init or more history)
2. Request those messages
3. Wait for server response
4. Sort messages by timestamp
5. Populate chat area.

### Server side

1. Determine offset (what range of messages to grab)
2. Open message history file
3. Read in messages based on length
4. Send those messgages to client

## SendMsg - client initaited

Send a chat from client to rest of network

### Client side
1. Serialize data
2. Create payload
3. Send to server
4. Wait for acknowledge
5. Display once acknowledge or give user error if no ack timeout


### Server side

1. Deserialize & timestamp
2. Put into Tx queue for LoRa
3. Store message in volatile memory for writing to sd card
4. Acknowledge client (send ACK)

## MsgRecv - server initiated

Recieve a chat from the network and send to any client currently up.

### Server side

1. Package as message type json
2. Send to client
3. Wait for acknowledge
4. If no ack, resend
5. Save in volatile memory array.

### Client side

1. Recieve message
2. Lookup node ID
3. Display in chat
4. Ackowledge
