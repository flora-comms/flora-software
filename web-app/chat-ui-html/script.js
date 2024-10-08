
class Message {
    constructor(content, user) {
        this.content = content;
        this.timestamp = new Date().toISOString();
        this.user = user;
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

function sendMessage()
{
    let inputElement = document.getElementById("message");
    let messageText = inputElement.value;
    // add to array if conditions are met 
    if(messageText.trim() !== "")
    {
        messages.push(messageText)
        inputElement.value = "";
        console.log(messages);
    }
}
