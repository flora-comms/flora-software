import Message from '/src/classes/Message.js';
import ChatHandler from '/src/classes/ChatHandler.js';

const chatHandler = new ChatHandler();
chatHandler.openWebSocket();

document.getElementById("sendButton").onclick = () => chatHandler.sendMessage();
document.getElementById("sosButton").onclick = showModal;
document.getElementById("confirmSOS").onclick = () => {
  const sosMessage = new Message('SOS', chatHandler.myNodeID);
  sosMessage.SOS = 1;
  chatHandler.socket.send(JSON.stringify(sosMessage));
  closeModal();
};
document.getElementById("cancelSOS").onclick = closeModal;
document.querySelector(".close").onclick = closeModal;

window.onclick = (event) => {
  const modal = document.getElementById("sosModal");
  if (event.target === modal) {
      closeModal();
  }
};

function showModal() {
  const modal = document.getElementById('sosModal');
  modal.style.display = 'flex';
}

function closeModal() {
  const modal = document.getElementById('sosModal');
  modal.style.display = 'none';
}
