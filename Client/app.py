from flask import Flask, render_template, request, redirect,flash
import xmlrpc.client
import secrets

app = Flask(__name__)

app.secret_key = secrets.token_hex(16)

USERNAME = ""
API_KEYS = []

@app.route('/')
def home():
    return render_template('login.html')

@app.route('/login', methods=['POST'])
def login():
    username = request.form['username']
    password = request.form['password']
    global USERNAME
    USERNAME = username
    return send_to_server(username, password)


@app.route('/generate_api_key', methods=['POST'])
def generate_api_key():
    username = USERNAME # Retrieve the username from the session or a hidden form field
    print("Username: ",username)
    with xmlrpc.client.ServerProxy("http://10.20.24.87:8000/RPC2") as proxy:
        new_api_key = proxy.generate_api_key(username)
        print(new_api_key)
        return render_template('api_keys.html', new_api_key=new_api_key, api_keys=API_KEYS)

def send_to_server(username, password):
    with xmlrpc.client.ServerProxy("http://10.20.24.87:8000/RPC2") as proxy:
        result = proxy.receive_credentials(username, password)
        if type(result) != list:
            flash("invalid credentials", "danger")
            return render_template('login.html')
        for i in range(len(result)):
            result[i] = result[i][:9]
        global API_KEYS
        API_KEYS = result
        print("Type: ",type(result))
        print("Got the result",result)
        
        if result != "-1":
            return render_template('api_keys.html', api_keys=result)

if __name__ == '__main__':
    app.run(debug=True)
