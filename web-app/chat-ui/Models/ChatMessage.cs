namespace chat_ui.Models
{
    public class ChatMessage
    {
        public string? Message { get; set; }
        public bool IsSent { get; set; } // true for sent messages, false for received
        public DateTime Timestamp { get; set; } = DateTime.Now; // Automatically set to current time
    }
}
