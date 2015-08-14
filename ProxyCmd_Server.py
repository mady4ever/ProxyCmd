from flask import request
from subprocess import call
import os
import subprocess

app = Flask(__name__)

app.debug = True
@app.route('/')
def index():
  	msg="For accessing CLI of Switch please send request with all credentials using this URI :\nhttp://172.16.75.11:5000/ProxyCmd/Api/v1/CreateTunnel  POST method"
	parameters="\nRequired parameters:\n1) HA_ip \n2) HA_port \n3) HA_username \n4) HA_password \n5) HM_ip \n6) HM_port \n7) HM_username \n8) HM_password \n9) Switch_ip \n10) Switch_port"
	return msg+parameters

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

@app.route('/ProxyCmd/Api/v1/DownloadLog', methods=['GET'])
def download_log():
	file_name="/home/yugesh/ProxyCmdLog.txt"
   	with open(file_name,'r') as f:
		body=f.read()
		response=make_response(body)
		response.headers["Content-Disposition"] = "attachment; filename="+file_name  	
		return response

if __name__ == '__main__':
    app.run(host='0.0.0.0')
