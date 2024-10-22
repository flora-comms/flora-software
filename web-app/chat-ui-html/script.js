class Message {
  constructor(content) {
    this.Payload = content;
    this.NodeID = myNodeID;  // myNodeID will be set after lookupNodes()
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

  lookupNodes(file, callback) {  // Add callback for synchronization
    fetch(file)
    .then(response => response.json())
    .then(data => {
      nodeTable = data;  
      console.log("Data fetched and stored:", nodeTable); 

      myNodeID = nodeTable.nodeTable[0];  // Set myNodeID after nodeTable is fetched
      document.getElementById('nodeIDDisplay').textContent = myNodeID;

      if (callback) {
        callback();  // Call the callback to proceed after lookupNodes is done
      }
    })
    .catch(error => console.error('Error fetching the file:', error));
  }

  sendMessage() {
    const inputElement = document.getElementById('messageInput');
    const Payload = inputElement.value;
    const trimmedContent = Payload.trim();

    if (socket && socket.readyState === WebSocket.OPEN) {
      if (trimmedContent !== '') {
        const newMessage = new Message(Payload);
        inputElement.value = '';
        console.log(newMessage);
        socket.send(JSON.stringify(newMessage));
      }
    } else {
      console.log('WebSocket is not connected.');
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

    console.log("message.NodeID:", message.NodeID);
    console.log("Type of message.NodeID:", typeof message.NodeID);
    console.log("myNodeID:", myNodeID);
    console.log("Type of myNodeID:", typeof myNodeID);
    

    if (nodeTable[message.NodeID] === myNodeID) {
      newMessageElement.classList.add('sent');
    } else {
      newMessageElement.classList.add('received');
    }

    newMessageElement.innerText = message.Payload;
    chatContainer.appendChild(newMessageElement);
    chatContainer.scrollTop = chatContainer.scrollHeight;  // Scroll to the bottom
  }
}

// Globals
let nodeTable = null;
let myNodeID = null;  
const publicChatHandler = new ChatHandler();
let socket;

function openWebSocket() {
  socket = new WebSocket('ws://localhost:8080');  // Change to DNS URL once setup
  socket.onopen = function(event) {
    console.log('WebSocket is connected.');

    // Fetch the nodeTable first, then fetch history
    publicChatHandler.lookupNodes('lookup.JSON', () => {
    publicChatHandler.parseHistory('history.JSON');
    });
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
