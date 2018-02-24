
from flask import Flask, render_template,redirect,url_for,jsonify, request, make_response,session,flash,logging
import os ,jwt ,datetime
from flask_bootstrap import Bootstrap
from functools import wraps
from flask_wtf import Form
from wtforms import StringField , PasswordField ,BooleanField
from wtforms.validators import InputRequired ,Email ,Length
from flask_sqlalchemy import SQLAlchemy
from werkzeug.security import generate_password_hash, check_password_hash
from flask_login import LoginManager , UserMixin , login_user , login_required,logout_user,current_user
import paho.mqtt.client as mqtt

from flask_sqlalchemy import SQLAlchemy


app = Flask(__name__)
mqttc=mqtt.Client()
mqttc.connect("localhost",1883,60)

app.secret_key = os.urandom(24)
app.config['SECRET_KEY'] = 'thisissecreat'
# app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite://///home/shivam/flaskprj/python_web_server/flaskdb.db'
# app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite://///C/Users/shivam/Desktop/python_web_server/flaskdb.db'
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:////home/vineet/Documents/vineet/demo/python_web_server/flaskdb.db'
# C:/Users/shivam/Desktop/python_web_server
Bootstrap(app)
db =SQLAlchemy(app)
login_manager = LoginManager()
login_manager.init_app(app)
login_manager.login_view = 'login'
# mqttc.loginoop_start()

# Create a dictionary called pins to store the pin number, name, and pin state:
pins = {
   4 : {'name' : 'SWITCH 3', 'board' : 'esp8266', 'topic' : 'esp8266/4', 'state' : 'False'},
   5 : {'name' : 'SWITCH 4', 'board' : 'esp8266', 'topic' : 'esp8266/5', 'state' : 'False'},
   16 : {'name' :'SWITCH 7','board' : 'esp8266', 'topic' : 'esp8266/16','state' : 'False'},
   0 : {'name' : 'SWITCH 1', 'board' : 'esp8266', 'topic' : 'esp8266/0', 'state' : 'False'},
   2 : {'name' : 'SWITCH 2', 'board' : 'esp8266', 'topic' : 'esp8266/2', 'state' : 'False'},
   12 :{'name' : 'SWITCH 5','board' : 'esp8266', 'topic' : 'esp8266/12','state' : 'False'},
   14 :{'name' : 'SWITCH 6','board' : 'esp8266', 'topic' : 'esp8266/14','state':  'False'}
   }
templateData = {'pins' : pins }






class User(UserMixin,db.Model):
	id = db.Column(db.Integer,primary_key =True)
	username = db.Column(db.String(15), unique =True)
	email = db.Column(db.String(80), unique =True)		
	password = db.Column(db.String(80))

@login_manager.user_loader
def load_user(user_id):
	return User.query.get(int(user_id))


class LoginForm(Form):
	username = StringField('username',validators=[InputRequired(),Length(min=4,max=15)])
	password = PasswordField('password',validators=[InputRequired(),Length(min=8,max=80)])
	remember = BooleanField('remember me')

class RegisterForm(Form):
	email = StringField('email',validators=[InputRequired(),Email(message = "Invalid email"), Length(max = 50)])	
	username = StringField('username',validators=[InputRequired(),Length(min=4,max=15)])
	password = PasswordField('password',validators=[InputRequired(),Length(min=8,max=80)])




@app.route('/')
def index():
	return render_template('index.html')






@app.route('/login', methods = ['GET' , 'POST'])
def login():
#	print (db)
	form = LoginForm()
	if form.validate_on_submit():
		user= User.query.filter_by(username=form.username.data).first()
		if user:
			if check_password_hash(user.password,form.password.data):
				session['logged_in'] =True
				session['username'] = form.username.data
				flash('Please Login First') 
				return render_template('dashboard.html',**templateData)
				
	flash('Please Try againg')
	return render_template('login.html', form=form )




def is_logged_in(f):
   @wraps(f)
   def wrap(*args, **kwargs):
       if 'logged_in' in session:
           return f(*args, **kwargs)
       else:
           flash('Unauthorized, Please Login','danger')
           flash('Alredy looded in') 
           return redirect(url_for('dashboard'))
   return wrap






@app.route('/signup', methods = ['GET' , 'POST'])
def signup():
	form = RegisterForm()
	if form.validate_on_submit():
		hashed_password = generate_password_hash(form.password.data,method='sha256')
		user= User.query.filter_by(username=form.username.data).first()
		if user:
			# if check_password_hash(user.password,form.password.data):
			app.logger.info('user alredy exixst')
			error = 'user alredy exixst'
			flash('user alredy exixst')
			return render_template('signup.html',error = error,form= form)
			
		else:
			new_user = User(username=form.username.data,email=form.email.data,password=hashed_password)
			db.session.add(new_user)
			db.session.commit()
			return render_template('login.html',form= form)
	
	return render_template('signup.html',error ="", form= form)






@app.route('/dashboard')
@is_logged_in
def dashboard():
	return render_template('dashboard.html',name=current_user.username)
	return render_template('index.html', form=form )



@app.route("/<board>/<changePin>/<action>")
@is_logged_in
def action(board, changePin, action):
   # Convert the pin from the URL into an integer:
   changePin = int(changePin)
   # Get the device name for the pin being changed:
   devicePin = pins[changePin]['name']
   # If the action part of the URL is "on," execute the code indented below:
   if action == "1" and board == 'esp8266':
      mqttc.publish(pins[changePin]['topic'],"1")
      pins[changePin]['state'] = 'True'

   if action == "0" and board == 'esp8266':
      mqttc.publish(pins[changePin]['topic'],"0")
      pins[changePin]['state'] = 'False'

   # Along with the pin dictionary, put the message into the template data dictionary:
   templateData = {      'pins' : pins   }
   return render_template('main1.html', **templateData)






@app.route('/logout')
@login_required
def logout():
	session.clear()
	logout_user()
	return redirect(url_for('index'))



if __name__ == '__main__':
   app.run(host = '192.168.43.97',port = 8080,debug = True)
