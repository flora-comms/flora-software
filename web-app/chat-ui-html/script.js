
class Message {
    constructor(content, user) {
        this.messageContent = content;
        this.timestamp = new Date().toISOString();
        this.userIp = user;
    }
}
let messages = [];
let socket;

function openWebSocket() {
    socket = new WebSocket('ws://localhost:8080');  // change to esp32 IP address
    socket.onopen = function(event) {
        console.log('WebSocket is connected.');
    };
    socket.onmessage = function(event) {
        let message = event.data;
        updateChat(message);
    };
    socket.onclose = function(event) {
        console.log('WebSocket is closed.');
    };
    socket.onerror = function(error) {
        console.log('WebSocket error: ' + error.message);
    };
}

function updateChat(message)
{
    let chatContainer = document.getElementById("chat-container");
    let messageElement = document.createElement("div");
    messageElement.innerHTML = message;
    chatContainer.appendChild(messageElement);
}

function sendMessage() {
    const inputElement = document.getElementById("messageInput");
    const messageContent = inputElement.value; // Original content (with whitespace)
    const trimmedContent = messageContent.trim(); // Trimmed content for validation    

    // Check if the message content is not empty
    if (trimmedContent !== "") {
        const userIP = "192.168.0.100"; // Placeholder IP, maybe need to find another method ID'ing user
        const newMessage = new Message(messageContent, userIP);
        inputElement.value = "";
        console.log(newMessage); // Corrected variable name here
        sendMessageToServer(newMessage);
    }
}
    


