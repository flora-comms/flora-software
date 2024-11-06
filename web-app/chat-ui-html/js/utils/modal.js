export function showModal() {
    const modal = document.getElementById('sosModal');
    modal.style.display = 'flex';
  }
  
export function closeModal() {
    const modal = document.getElementById('sosModal');
    modal.style.display = 'none';
  }
  
export function sendSOS() {
    const sosMessage = new Message('SOS');
    sosMessage.SOS = 1;
    socket.send(JSON.stringify(sosMessage));
    closeModal();
  }