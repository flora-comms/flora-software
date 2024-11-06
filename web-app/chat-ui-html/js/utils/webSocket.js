//import {ChatHandler} from './classes/chatHandler.js';

export function
openWebSocket(url, publicChatHandler) {
  const newSocket =
      new WebSocket(url);  // ws://avalink.local/chat
  newSocket.onopen = function(event) {
    console.log('WebSocket is connected.');
    publicChatHandler.setSocket(socket);

    // Fetch the nodeTable first, then fetch history
    publicChatHandler.readLookup('data/lookup.JSON', () => {
    publicChatHandler.parseHistory('data/history.csv');
    });
  };

  newSocket.onmessage = function(event) {
    publicChatHandler.receiveMessage(event);
  };

  newSocket.onclose = function(event) {
    console.log('WebSocket is closed.');
  };

  newSocket.onerror = function(error) {
    console.log('WebSocket error: ' + error.message);
  };

  return newSocket;
}