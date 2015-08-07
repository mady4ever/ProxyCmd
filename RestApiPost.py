from flask import Flask
from flask import request
from subprocess import call

app = Flask(__name__)

app.debug = True
@app.route('/')
def index():
    return "Hello, World!"

@app.route('/ProxyCmd/Api/v1/CreateTunnel', methods=['POST'])
def create_task():
	porNumber = request.form['portNumer']
	haPort    = request.form['HAPort']		
	call(["./abc"])
    	return porNumber +  haPort 

if __name__ == '__main__':
    app.run(debug=True)
