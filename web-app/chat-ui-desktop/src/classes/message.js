export default class Message {
    constructor(content, nodeId) {
        this.Payload = content;
        this.NodeID = nodeId; 
        this.SOS = 0;
    }
  }