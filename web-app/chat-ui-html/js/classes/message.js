export class Message {
    constructor(content) {
      this.Payload = content;
      this.NodeID = myNodeID;  // myNodeID will be set after readLookup()
      this.SOS = 0;
    }
  }