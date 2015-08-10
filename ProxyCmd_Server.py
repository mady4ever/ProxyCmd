
from flask import Flask
from flask import request
from subprocess import call
import os
import subprocess

app = Flask(__name__)

app.debug = True
@app.route('/')
def index():
    return "Hello, World!"

@app.route('/ProxyCmd/Api/v1/CreateTunnel', methods=['POST'])
def create_task():
	HA_ip = request.form.get('HA_ip')
	HA_port = request.form.get('HA_port')
	HA_username = request.form.get('HA_username')
	HA_password = request.form.get('HA_password')
	HM_ip = request.form.get('HM_ip')
	HM_port = request.form.get('HM_port')
	HM_username = request.form.get('HM_username')
	HM_password = request.form.get('HM_password')
	Switch_ip = request.form.get('Switch_ip')
	Switch_port = request.form.get('Switch_port')
	subprocess.Popen(["/home/yugesh/./ProxyCmd",HA_ip,HA_port,HA_username,HA_password,HM_ip,HM_port,HM_username,HM_password,Switch_ip,Switch_port])
	return "OK"

if __name__ == '__main__':
    app.run(host='0.0.0.0')
