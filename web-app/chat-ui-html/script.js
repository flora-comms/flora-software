

class Message {
  constructor(content) {
    this.Payload = content;
    this.NodeID = myNodeID;
  }
}

class ChatHandler {
  constructor() {
    this.messageHistory = [];
  }
  parseHistory(file) {
    fetch(file)
        .then(response => response.json())
        .then(data => {
          if (data.Type === 'History' && Array.isArray(data.Messages)) {
            data.Messages.forEach(message => {
              this.updateChatContainer(message);  // Call the function to display the message
            });
          } else {
            console.error('Invalid message history structure.');
          }
        })
        .catch(error => console.error('Error loading JSON:', error));
  }

  sendMessage() {
    const inputElement = document.getElementById('messageInput');
    const Payload = inputElement.value;
    const trimmedContent = Payload.trim();

    if (socket &&
        socket.readyState === WebSocket.OPEN) {  // Ensure the socket is open
      // Check if the message content is not empty
      if (trimmedContent !== '') {
        const newMessage = new Message(Payload);
        inputElement.value = '';
        console.log(newMessage);
        socket.send(JSON.stringify(newMessage));  //
      }
    } else {
      console.log('WebSocket is not connected. Message not NodeID.');
    }
  }

  receiveMessage(event) {
    const messageObject = JSON.parse(event.data);
    this.messageHistory.push(messageObject);
    console.log('Received message object:', messageObject);
    this.updateChatContainer(messageObject);
  }

  updateChatContainer(message) {
    const chatContainer = document.getElementById('chatContainer');
    const newMessageElement = document.createElement('div');
    newMessageElement.className = 'message';

    // Check the nodeID to determine if the message is sent or received
    if (message.NodeID === myNodeID) {
      newMessageElement.classList.add('sent');
    } else {
      newMessageElement.classList.add('received');
    }

    newMessageElement.innerText = message.Payload;
    chatContainer.appendChild(newMessageElement);
    chatContainer.scrollTop =
        chatContainer.scrollHeight;  // Scroll to the bottom
  }
}

// Globals
const maxMessageLength = 255;
const myNodeID = 0x00;
document.getElementById('nodeIDDisplay').textContent = myNodeID;
const publicChatHandler = new ChatHandler();
let socket;

function openWebSocket() {
  socket =
      new WebSocket('ws://localhost:8080');  // change to DNS URL once setup
  socket.onopen = function(event) {
    console.log('WebSocket is connected.');
    publicChatHandler.parseHistory('history.JSON');
  };
  socket.onmessage = function(event) {
    publicChatHandler.receiveMessage(event);
  };
  socket.onclose = function(event) {
    console.log('WebSocket is closed.');
  };
  socket.onerror = function(error) {
    console.log('WebSocket error: ' + error.message);
  };
}
