import Message from '/src/classes/Message.js';
import ChatHandler from '/src/classes/ChatHandler.js';
import { showModal, closeModal } from '/src/utilities/modal.js';

const chatHandler = new ChatHandler();
chatHandler.openWebSocket();

// Event listeners
document.getElementById("sendButton").onclick = () => chatHandler.sendMessage();
document.getElementById("sosButton").onclick = showModal;
document.getElementById("confirmSOS").onclick = () => {chatHandler.sendMessage(true); closeModal();}
document.getElementById("cancelSOS").onclick = closeModal;
document.querySelector(".close").onclick = closeModal;

window.onclick = (event) => {
  const modal = document.getElementById("sosModal");
  if (event.target === modal) {
      closeModal();
  }
};


