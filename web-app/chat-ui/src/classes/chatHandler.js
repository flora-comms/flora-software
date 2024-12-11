import Message from './message.js';

// const WEBSOCKET_URL = 'ws://localhost:5050';  //Test URL for node.js
const WEBSOCKET_URL = 'ws://floranet.local/chat';  // Petal URL

export default class ChatHandler {
  constructor() {
    this.messageHistory = [];
    this.nodeTable = null;
    this.myNodeID = null;
    this.socket = null;
  }

  openWebSocket() {
    this.socket = new WebSocket(WEBSOCKET_URL);

    this.socket.onopen = () => {
      console.log('WebSocket is connected.');
      this.readLookup('/data/lookup.JSON', () => {
        this.parseHistory('/data/history.csv');
      });
    };

    this.socket.onmessage = (event) => this.receiveMessage(event);
    this.socket.onclose = () => console.log('WebSocket is closed.');
    this.socket.onerror = (error) =>
        console.log('WebSocket error:', error.message);
  }

  readLookup(file, callback) {
    fetch(file)
        .then(response => response.json())
        .then(data => {
          this.nodeTable = data;
          console.log('Data fetched and stored:', this.nodeTable);

          this.myNodeID = this.nodeTable['0'];
          document.getElementById('nodeIDDisplay').textContent =
              this.nodeTable[this.myNodeID];

          if (callback) {
            callback();
          }
        })
        .catch(error => console.error('Error fetching the file:', error));
  }

  parseHistory(file) {
    fetch(file)
        .then(response => response.text())
        .then(text => {
          const rows = text.trim().split('\n');
          rows.shift();

          rows.forEach(row => {
            const match = row.match(/(".*?"|[^",\s]+)(?=\s*,|\s*$)/g);

            if (match) {
              const payload = match[0].replace(/^"|"$/g, '').trim();
              const nodeId = parseInt(match[1].trim(), 10);
              const sos = parseInt(match[2].trim(), 10);

              const message = {Payload: payload, NodeID: nodeId, SOS: sos};
              this.updateChatContainer(message);
            }
          });
        })
        .catch(error => console.error('Error loading CSV:', error));
  }

  sendMessage(isSOS = false) {
    if (!isSOS) {
      const inputElement = document.getElementById('messageInput');
      const Payload = inputElement.value.trim();

      if (this.socket && this.socket.readyState === WebSocket.OPEN &&
          Payload !== '') {
        const newMessage = new Message(Payload, this.myNodeID);
        inputElement.value = '';
        console.log(newMessage);
        this.socket.send(JSON.stringify(newMessage));
      } else {
        console.log('WebSocket is not connected.');
      }
    } else {
      const sosMessage = new Message('SOS', this.myNodeID);
      sosMessage.SOS = 1;
      this.socket.send(JSON.stringify(sosMessage));
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

    if (message.SOS === 1) {
      newMessageElement.classList.add('sos-message');
    }

    if (message.NodeID === this.myNodeID) {
      newMessageElement.classList.add('sent');
    } else {
      newMessageElement.classList.add('received');
      const nodeIDElement = document.createElement('div');
      nodeIDElement.className = 'node-ID';
      nodeIDElement.innerText = this.nodeTable[message.NodeID];
      messageWrapper.appendChild(nodeIDElement);
    }

    newMessageElement.innerText = message.Payload;
    messageWrapper.appendChild(newMessageElement);
    chatContainer.appendChild(messageWrapper);
    chatContainer.scrollTop = chatContainer.scrollHeight;
  }
}