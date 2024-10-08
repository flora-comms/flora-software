let messages = [];

function sendMessage()
{
    let inputElement = document.getElementById("message");
    let messageText = inputElement.value;
    // add to array if conditions are met 
    if(messageText.Trim() !== "")
    {
        messages.push(messageText)
        inputElement.value = "";
        console.log(messages);
    }
}