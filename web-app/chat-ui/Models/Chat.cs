namespace chat_ui.Models
{
    public class Chat
    {
        public List<ChatMessage> Messages { get; set; } = new List<ChatMessage>();
        public string? Message { get; set; }

        public void SendMessage()
        {
            if (!string.IsNullOrEmpty(Message))
            {
                Messages.Add(new ChatMessage { Message = Message, IsSent = true });
                Message = string.Empty; // Clear input after sending



                // Send an auto-reply message
                ReceiveMessage("auto-reply");
            }
        }

        public void ReceiveMessage(string message)
        {
            Messages.Add(new ChatMessage { Message = message, IsSent = false });
        }
    }
}

