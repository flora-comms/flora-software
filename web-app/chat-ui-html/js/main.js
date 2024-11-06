import { Message } from './classes/message.js';
import { ChatHandler } from './classes/chatHandler.js';
import { openWebSocket } from './utils/webSocket.js';
import { showModal, closeModal, sendSOS } from './utils/modal.js';

const publicChatHandler = new ChatHandler();

// In main.js
window.publicChatHandler = publicChatHandler;

window.onload = () => {
  const socket = openWebSocket('ws://avalink.local/chat', publicChatHandler);
};

window.onclick =
    function(event) {
  const modal = document.getElementById('sosModal');
  if (event.target == modal) {
    modal.style.display = 'none';
  }
}

document.querySelector('.close')
        .onclick =
        function() {
  closeModal();
}

document.querySelector('.cancelButton')
            .onclick =
            function() {
  closeModal();
}

document.querySelector('.sosButton').onclick = function() {
  sendSOS(socket, publicChatHandler);
};




