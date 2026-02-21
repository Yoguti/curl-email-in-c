C libcurl SMTP sender

Sends an email via SMTP using libcurl.
The body is read from a file and wrapped into a valid mail message.

--------------------------------

SETUP

Edit in main():

msg_info msg = {
    .app_pwd = "APP PASSWORD",
    .from_email = "sender@email.com",
    .to_email = "target@email.com",
    .smtp = "smtp://smtp.gmail.com:587"
};

Gmail requires an App Password (normal password will fail).

In create_payload() change the file path:
fopen("body.txt", "r");

--------------------------------

BUILD

gcc main.c -o sendmail -lcurl

--------------------------------

RUN

./sendmail

--------------------------------

NOTES

Keep the blank line after Subject.
SMTP requires CRLF (\r\n).
Do not commit your password.
