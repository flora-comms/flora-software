namespace chat_ui.Models
{
    public class Chat
    {
        // Public properties to access message lists
        public string? Message { get; set; }
        public List<string> MessagesSent { get; set; } = new List<string>();
        public List<string> MessagesReceived { get; set; } = new List<string>();

        // Public method to send a message
        public async Task SendMessage()
        {
            if (!string.IsNullOrWhiteSpace(Message))
            {
                MessagesSent.Add(Message);
                Message = string.Empty;

                await Task.Delay(1000); // Simulate delay
                ReceiveMessage();
            }
        }

        // Public method to receive a message
        public void ReceiveMessage()
        {
            MessagesReceived.Add("auto-reply"); // For testing purposes
            Message = string.Empty;
        }
    }
}

