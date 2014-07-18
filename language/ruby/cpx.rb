require 'json'
require 'uri'
require 'net/http'

#
# CPX Protocol in Ruby, Tap In Systems, Inc.
#
# This program is free software: you can redistribute it and/or modify it under the terms of the 
# GNU General Public License as published by the Free Software Foundation, either version 3 of the 
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with this program.  
# If not, see <http://www.gnu.org/licenses/>.

#
# Creates a control plan x object
#
# user = the username
# id = the device id
class CPX
	def initialize(host, user, id)
		@host = host;
		@user = user;
		@id = id;
		
		@OFF = 0
		@ON = 1
		
		@uri = URI(host);
		@http = Net::HTTP.new(@uri.host, @uri.port);
	end
	
#
# Send a single command
#
	def sendCommand(command)
		command.store(:user, @user)
		command.store(:id, @id)
		@data = {"map" => command}		

		req = Net::HTTP::Post.new @uri.path
		req.body = @data.to_json
		res = Net::HTTP.start(@uri.host, @uri.port) do |http|
 			http.request req
		end
		puts res.body
	end
	
#
# Sent multiple commands
#
	def sendCommands(commands)
		command = {"user" => @user,"id" => @id}
		command.store(:commands,commands);
		@data = {"map" => command}		

		req = Net::HTTP::Post.new @uri.path
		req.body = @data.to_json
		res = Net::HTTP.start(@uri.host, @uri.port) do |http|
 			http.request req
		end
		puts res.body
	end

#
# Notify a control plan
#	
	def notify(label,user,plan,args,endpoint,wait)
		@y = {"plan-user" => user, "plan" => plan, "args" => args, "endpoint" => endpoint, "wait" => wait}
		if label != nil
			@y.store(:label,label); 
		end
	end

#
# Trigger on hardware event
#	
	def trigger(label, pin, timeout)
		m = {"command" => "trigger",  "pin" => pin, "timeout" => timeout};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
#
# Sends a ping
#
	def ping
		@y = {"command" => "ping"}
	end
	
#
# Write to digital pin
# label = the label to use or nil
# pin = the hardware pin
# value = set to this value
#
	def digitalWrite(label,pin,value)
		m = {"command" => "digitalwrite",  "pin" => pin, "value" => value};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
#
# Read a digital pin
# label = the label to use, or nil
# pin = the pin to read from
#
	def digitalRead(label,pin)
		m = {"command" => "digitalread",  "pin" => pin};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
#
# Read analog value
# label = the label to use or nil
# pin = the pin to read from
#
	def analogRead(label,pin)
		m = {"command" => "analogread",  "pin" => pin};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
#
# Write to analog pin
# label = the label to use or nil
#
	def analogWrite(label,pin,value)
		m = {"command" => "analogwrite",  "pin" => pin, "value" => value};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
# 
# Set a pin mode
# label = the label or nil
# pin = the hardware pin
# value = 0 = in, 1 = out.
#
	def setPinMode(label,pin,value)
		m = {"command" => "setpinmode",  "pin" => pin, "value" => value};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
#
# Pause hardware
# label = the label to use, or nil
# value = the number of ms to pause.
#
	def delay(label,value)
		m = {"command" => "delay",  "value" => value};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
#
# List all devices for this user
#
	def listDevices() 
		@y = { "proxy-command" => "listdevices"};
	end
	
	def call(function,param)
		m = {"command" => "call",  "function" => function, "param" => param};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
#
# Send an HTTP POST or GET from the hardware
# label = the label to use, or nil
# website = the place to send the request
# oper = 'post' or 'get'
#
	def proxy(label,website,wepbage,oper)
		m = {"command" => "proxy",  "website" => website, "oper" => oper};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
#
# Jump to a label
# label = this commands label, or nil
# where = the label to jump to
# count = number of times jump is allowed (0 means forever)
#
	def jump(label,where, count)
		m = {"command" => "goto",  "where" => where};
		if label != nil
			m.store(:label,label) 
		end
		if count != 0
			m.store(:count, count)
		end
		@y = m
	end
	
#
# Allocate memory in the device
# label = the label or nil
# name = the symbol name
# size = the size in bytes to allocate
#
	def allocate(label,name,size)
		m = {"command" => "allocate",  "name" => name, "size" => size};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
	def setImage(label,start,name,values)
		m = {"command" => "start",  "name" => name, "values" => values};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
	def getImage(label,name,start,stop) 
		m = {"command" => "getimage",  "name" => name, "start" => start, "stop" => stop};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
	def getEEPROM(label,address)
		m = {"command" => "geteeprom",  "address" => address};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
	def setEEPROM(label,address,value)
		m = {"command" => "seteeprom",  "address" => address, "value" => value};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
	def callback(label) 
		m = {"command" => "send-callback"};
		if label != nil
			m.store(:label,label); 
		end
		@y = m
	end
	
	def call(name,param)
  		@y= {"command" => "call",  "function" => name, "param" => param};
	end
	
	def checkCallback() 
		@y = {"proxy-command" => "callback"};
	end
	
	def shift(command,name)
		command.store(:shift, name);
	end
	
	def print(command)
		puts command.to_json
	end
	
end

