import ChatHandler from '/src/classes/ChatHandler.js';
import Message from '/src/classes/Message.js';
import {closeModal, showModal} from '/src/utilities/modal.js';

const chatHandler = new ChatHandler();
chatHandler.openWebSocket();

// Event listeners
document.getElementById('sendButton').onclick = () =>
    chatHandler.sendMessage();                     // Send Button
document.addEventListener('keydown', (event) => {  // Enter Key
  if (event.key === 'Enter') {
    chatHandler.sendMessage();
  }
});
document.getElementById('sosButton').onclick = showModal;  // SOS Ribbon
document.getElementById('confirmSOS').onclick =
    () => {
      chatHandler.sendMessage(true);
      closeModal();
    }  // Send SOS
          document.getElementById('cancelSOS')
              .onclick = closeModal;                    // Cancel SOS
document.querySelector('.close').onclick = closeModal;  // Close Modal x

window.onclick = (event) => {  // Close modal, background clock
  const modal = document.getElementById('sosModal');
  if (event.target === modal) {
    closeModal();
  }
};
