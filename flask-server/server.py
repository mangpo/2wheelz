#!/usr/bin/python
from flask import Flask
from flask import request

import os
import time
import smtplib
import email.utils
from email.mime.text import MIMEText

app = Flask(__name__)
public_dir = "../express-server/public/"

@app.route('/status', methods=['GET'])
def status():
  f = open('status','r')
  x = f.readline()
  f.close()
  return x

@app.route('/snap', methods=['GET'])
def snap():
  print 'args -------------'
  print request.args
  if request.args:
    f = open('status','w')
    if 'on' in request.args:
      print "camera on"
      f.write(str(1))
      date = time.strftime("On %A, %d %B %Y %H:%M", time.localtime())
      g = open(public_dir + 'date.txt','w')
      g.write(date)
      g.close
      
    elif 'off' in request.args:
      print "camera off"
      f.write(str(0))
      send_email()
    f.close()
  return "camera toggle"

@app.route('/upload', methods=['GET'])
def upload_get():
  return """
  <!DOCTYPE html>
  <html>
  <body>

  <form  method="post" enctype="multipart/form-data">
      Select image to upload:
      <input type="file" name="uploaded_file" id="uploaded_file">
      <input type="submit" value="Upload Image" name="submit">
  </form>

  </body>
  </html>
  """

@app.route('/upload', methods=['POST'])
def upload_post():
  # Already take pictures
  # f = open('status','w')
  # f.write(str(0))
  # f.close()

  #request.environ['CONTENT_TYPE'] = 'application/something_Flask_ignores'
  print 'headers -------------'
  print request.headers
  print 'get_data -------------'
  print len(request.get_data())
  print 'args -------------'
  print request.args
  print 'files -------------'
  print request.files
  if request.files:
    f = request.files['uploaded_file']
    print 'filename -------------'
    print f.filename
    print 'uploaded_files -------------'
    print f
    f.save(public_dir + f.filename.split('/')[-1].split('#')[0] + ".jpg")
  print 'data -------------'
  print len(request.data)
  print 'form -------------'
  print request.form
  print 'encoding -------------'
  print request.content_encoding
  print request.content_length
  print request.content_type
  print 'host -------------'
  print request.host
  print '-------------'
  return "uploade OK"

def send_email():
  
  fromaddr = 'name@gmail.com'
  toaddrs  = ['to1@mail.com', 'to2@mail.com']

  username = 'name'
  password = 'password'
  
  # Create the message                                                                
  msg = MIMEText('Guess what? I just rode my two wheeler for the first time!\nWant to share this memorable moment with me and see how it went?\nFollow the link here:\n\nhttp://your.server:3030\n\nLove, XXX')
  msg.set_unixfrom('author')
  # msg['To'] = email.utils.formataddr((toaddrs,toaddrs))                              
  msg['From'] = email.utils.formataddr(('2Wheelz', fromaddr))
  msg['Subject'] = 'Message from 2Wheelz'

  # The actual mail send                                                              
  server = smtplib.SMTP('smtp.gmail.com:587')
  server.ehlo()
  server.starttls()
  server.login(username,password)
  server.sendmail(fromaddr, toaddrs, msg.as_string())
  server.quit()

if __name__ == '__main__':
  app.run(host='0.0.0.0', debug=True)

