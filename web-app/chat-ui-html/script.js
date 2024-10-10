class Message {
    constructor(content) {
        this.messageContent = content;
        this.sent = 0;
        this.serverAck = 0;
    }
}

class ChatHandler {
    constructor() {
        this.messageHistory = [];  
    }

    sendMessage() {
        const inputElement = document.getElementById("messageInput");
        const messageContent = inputElement.value; 
        const trimmedContent = messageContent.trim();
    
        // Check if the message content is not empty
        if (trimmedContent !== "") {
            const newMessage = new Message(messageContent);  
            newMessage.sent = 1;
            newMessage.serverAck = 0;
            inputElement.value = "";    
            console.log(newMessage); 
            this.sendMessageToServer(newMessage); 
        }
    }

    receiveMessage(event) {
        const messageObject = JSON.parse(event.data);
        this.messageHistory.push(messageObject); 
        
        // Sort messages by timestamp
        this.messageHistory.sort((a, b) => new Date(a.timestamp) - new Date(b.timestamp));
        
        this.updateChatContainer(messageObject); 
    }

    updateChatContainer(message) { 
        const chatContainer = document.getElementById("chatContainer");
        const newMessageElement = document.createElement("div");
        newMessageElement.className = "message";
        newMessageElement.innerText = message.messageContent;
        chatContainer.appendChild(newMessageElement);
        chatContainer.scrollTop = chatContainer.scrollHeight;  // Scroll to the bottom
    }
    
    sendMessageToServer(messageObject) {
        if (socket && socket.readyState === WebSocket.OPEN) { // Ensure the socket is open
            // Convert the Message object to JSON and send it
            socket.send(JSON.stringify(messageObject)); 
            console.log('Message sent:', messageObject); // Log the sent message for debugging
        } else {
            console.log('WebSocket is not connected. Message not sent.');
        }
    }
}

// Globals
const publicChatHandler = new ChatHandler();  
let socket;

function openWebSocket() {
    socket = new WebSocket('ws://localhost:8080');  // change to DNS URL once setup
    socket.onopen = function(event) {
        console.log('WebSocket is connected.');
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















    


