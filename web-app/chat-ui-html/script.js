// Globals
let nodeTable = null;
let myNodeID = null;
let publicChatHandler = null;
let socket;


class Message {
  constructor(content) {
    this.Payload = content;
    this.NodeID = myNodeID;  // myNodeID will be set after readLookup()
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
              this.updateChatContainer(
                  message);  // Call the function to display the message
            });
          } else {
            console.error('Invalid message history structure.');
          }
        })
        .catch(error => console.error('Error loading JSON:', error));
  }

  readLookup(file, callback) {
    fetch(file)
        .then(response => response.json())
        .then(data => {
          nodeTable = data;
          console.log('Data fetched and stored:', nodeTable);

          myNodeID = nodeTable['0'];  // Set myNodeID after nodeTable is fetched
          document.getElementById('nodeIDDisplay').textContent =
              nodeTable[myNodeID];

          if (callback)  // execute ParseHistory as a callback
          {
            callback();
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
    const messageWrapper = document.createElement('div');
    messageWrapper.className = 'message-wrapper';

    const newMessageElement = document.createElement('div');
    newMessageElement.className = 'message';

    if (message.NodeID === myNodeID) {
      newMessageElement.classList.add('sent');
    } else {
      newMessageElement.classList.add('received');
      const nodeIDElement = document.createElement('div');
      nodeIDElement.className = 'node-ID';
      nodeIDElement.innerText = nodeTable[message.NodeID];
      messageWrapper.appendChild(nodeIDElement);
    }

    newMessageElement.innerText = message.Payload;
    messageWrapper.appendChild(newMessageElement);

    chatContainer.appendChild(messageWrapper);

    chatContainer.scrollTop =
        chatContainer.scrollHeight;  // Scroll to the bottom
  }
}

publicChatHandler = new ChatHandler();

function openWebSocket() {
  socket =
      new WebSocket('ws://192.168.1.67:8080');  // Change to DNS URL once setup
  socket.onopen = function(event) {
    console.log('WebSocket is connected.');

    // Fetch the nodeTable first, then fetch history
    publicChatHandler.readLookup('lookup.JSON', () => {
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
