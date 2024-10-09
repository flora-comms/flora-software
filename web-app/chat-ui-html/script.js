
class Message {
    constructor(content, user) {
        this.messageContent = content;
        this.timestamp = new Date().toISOString();
        this.userIp = user;
    }
}
let messageHistory = [];
let socket;

function openWebSocket() {
    socket = new WebSocket('ws://localhost:8080');  // change to esp32 IP address
    socket.onopen = function(event) {
        console.log('WebSocket is connected.');
    };
    socket.onmessage = function(event) {
        let message = event.data;
        receiveMessage();
    };
    socket.onclose = function(event) {
        console.log('WebSocket is closed.');
    };
    socket.onerror = function(error) {
        console.log('WebSocket error: ' + error.message);
    };
}

function updateChatContainer(message){
    const chatContainer = document.getElementById("chatContainer");
    const newMessageElement = document.createElement("div");
    newMessageElement.className = "message";
    newMessageElement.innerText = message.messageContent;
    chatContainer.appendChild(newMessageElement);
    chatContainer.scrollTop = chatContainer.scrollHeight;  
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
function sendMessageToServer(messageObject) {
    if (socket && socket.readyState === WebSocket.OPEN) { // Ensure the socket is open
        // Convert the Message object to JSON and send it
        socket.send(JSON.stringify(messageObject)); 
        console.log('Message sent:', messageObject); // Log the sent message for debugging
    } else {
        console.log('WebSocket is not connected. Message not sent.');
    }
}

function receiveMessage(event) {
    const messageObject = JSON.parse(event.data);
    messageHistory.push(messageObject); 
    // Sort messages by timestamp
    messagesHistory.sort((a, b) => new Date(a.timestamp) - new Date(b.timestamp));
    updateChatContainer(messageObject);
}






    


