

export class ChatHandler {
    constructor() {
        this.socket = null;
      this.nodeTable = null;
      this.myNodeID = null;
    }

    setSocket(socket) {
        this.socket = socket;
    }
  
    parseHistory(file) {
      fetch(file)
          .then(response => response.text())
          .then(text => {
            const messages = [];
            const rows = text.trim().split('\n');
            rows.shift();  // Remove header
  
            rows.forEach(row => {
              const match = row.match(/(".*?"|[^",\s]+)(?=\s*,|\s*$)/g);
  
              if (match) {
                const payload = match[0].replace(/^"|"$/g, '').trim();
                const nodeId = parseInt(match[1].trim(), 10);
                const sos = parseInt(match[2].trim(), 10);
  
                const message = {Payload: payload, NodeID: nodeId, SOS: sos};
                this.updateChatContainer(message);  // Display each parsed message
              }
            });
          })
          .catch(error => console.error('Error loading CSV:', error));
    }
  
  
    readLookup(file, callback) {
      fetch(file)
          .then(response => response.json())
          .then(data => {
            this.nodeTable = data;
            console.log('Data fetched and stored:', this.nodeTable);
  
            this.myNodeID = this.nodeTable['0'];  // Set myNodeID after nodeTable is fetched
            document.getElementById('nodeIDDisplay').textContent =
                this.nodeTable[this.myNodeID];
  
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
  
      if (this.socket && this.socket.readyState === WebSocket.OPEN) {
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
  
      // Check if the message is an SOS message
      if (message.SOS === 1) {
        newMessageElement.classList.add(
            'sos-message');  // Apply SOS-specific class
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
  
      chatContainer.scrollTop =
          chatContainer.scrollHeight;  // Scroll to the bottom
    }
  }