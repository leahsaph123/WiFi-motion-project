import smtplib
from email.mime.text import MIMEText

def send_email(to, subject, body):
    msg = MIMEText(body)
    msg['From'] = "youareskills@gmail.com"
    msg['To'] = to
    msg['Subject'] = subject

    with smtplib.SMTP_SSL('smtp.gmail.com', 465) as server:
        server.login("youareskills@gmail.com", "rxea lwix lmgi jsba")
        server.send_message(msg)

send_email("nbrizuela7396@sdsu.edu", "Test", "Testing sending email")