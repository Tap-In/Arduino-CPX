
#
# Doohickey Controller, using CPX Protocol in Ruby, Tap In Systems, Inc.
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

require './doohickey.rb'

class Doohickey
	def initialize(cpx)
		@cpx = cp
		@RED = 6
		@GREEN = 4
		@RGB_GREEN = 46
		@RGB_RED = 44
		@redVal = 128
		@greenVal = 128
	end
	
	def setWhite()
		clist = []
		@redVal = 128
		@greenVal = 128
		clist << @cpx.analogWrite(nil,@RGB_RED, 255 - @redVal)
		clist << @cpx.analogWrite(nil,@RGB_GREEN, 255 - @greenVal)
		clist << @cpx.digitalWrite(nil,@RED, @cpx.OFF)
		clist << @cpx.digitalWrite(nil,@GREEN, @cpx.OFF)
		cpx.sendCommands(clist)
	end
	
	def doGreen(value) 
		newv = 128 + 128 * value;
		incr = 1;
		if (newv < @greenVal)
			incr = -1
		end
		clist = []
		if (incr == 1) 
			clist << @cpx.digitalWrite(nil,@RED, @cpx.OFF)
			clist << @cpx.digitalWrite(nil,@GREEN, @cpx.ON)
			clist << @cpx.delay(100)
			clist << @cpx.digitalWrite(nil,@GREEN, @cpx.OFF)
	    else 
			clist << @cpx.digitalWrite(nil,@GREEN, @cpx.OFF)
			clist << @cpx.digitalWrite(nil,@RED, @cpx.ON)
			clist << @cpx.delay(nil,100)
			clist << @cpx.digitalWrite(nil,@RED, @cpx.OFF)
		end
		@cpx.sendCommands(clist)
		sleep(100)
		for i in @greenVal..newv 
			clist = []
			@greenVal += incr
			@redVal -= incr
			if @greenVal > 255
				@greenVal = 255;
			end
			if @redVal < 0
				@redVal = 0
			end
			clist << @cpx.analogWrite(nil,@RGB_RED, 255 - @redVal)
			clist << @cpx.analogWrite(nil,@RGB_GREEN, 255 - @greenVal)
			@cpx.sendCommands(clist);
		end
	end
	
	
	def doRed(value)
		newv = 128 + 128 * value
		incr = 1;
		if newv < @redVal
			incr = -1;
		end
		
		clist = []
		if incr == -1
			clist << @cpx.digitalWrite(nil,@GREEN, @cpx.OFF)
			clist << @cpx.digitalWrite(nil,@RED, @cpx.ON)
			clist << @cpx.delay(nil,100)
			clist << cpx.digitalWrite(nil,@RED, @cpx.OFF)
		else
			clist << @cpx.digitalWrite(nil,@RED, @cpx.OFF)
			clist << @cpx.digitalWrite(nil,@GREEN, @cpx.ON)
			clist << @cpx.delay(nil,100)
			clist << @cpx.digitalWrite(nil,@GREEN, @cpx.OFF)
		end
		@cpx.sendCommands(clist)
		sleep(100);
		
		for i in @greenVal..newv 
			clist = []
			@greenVal -= value
			@redVal += value
			if @greenVal < 0
				@greenVal = 0
			end
			if @redVal > 255
				@redVal = 255
			end
			clist << @cpx.analogWrite(nil,@RGB_RED, 255 - @redVal)
			clist << @cpx.analogWrite(nil,@RGB_GREEN, 255 - @greenVal)
			@cpx.sendCommands(clist)
		end
	end
	
	def addGreen(value) 
		clist = []
		clist << @cpx.digitalWrite(nil,@RED, @cpx.OFF)
		clist << @cpx.digitalWrite(nil,@GREEN, @cpx.ON)
		clist << @cpx.delay(nil,100)
		clist << @cpx.digitalWrite(nil,@GREEN, @cpx.OFF)
		@greenVal += v
		@redVal -= v
		if @greenVal > 255
			@greenVal = 255
		end
		if @redVal < 0
			@redVal = 0
		end
		clist << @cpx.analogWrite(nil,@RGB_RED, 255 - @redVal)
		clist << @cpx.analogWrite(nil,@RGB_GREEN, 255 - @greenVal)
		@cpx.sendCommands(clist)
	end
	
	def addRed(value) 
		clist = []
		clist << @cpx.digitalWrite(nil,@GREEN, @cpx.OFF)
		clist << @cpx.digitalWrite(nil,@RED, @cpx.ON)
		clist << @cpx.delay(100)
		clist << @cpx.digitalWrite(nil,@RED, @cpx.OFF)

		@greenVal -= v
		@redVal += v
		if @greenVal < 0
			@greenVal = 0
		end
		if @redVal > 255
			@redVal = 255
		end
		clist << @cpx.analogWrite(nil,@RGB_RED, 255 - @redVal)
		clist << @cpx.analogWrite(nil,@RGB_GREEN, 255 - @greenVal)
		@cpx.sendCommands(clist)
	end
	
	def setRed(value)
		clist = []
		clist << @cpx.digitalWrite(nil,@RED, @cpx.OFF)
		clist << @cpx.digitalWrite(nil,@GREEN, @cpx.ON)
		clist << @cpx.delay(100)
		clist << @cpx.digitalWrite(nil,@GREEN, @cpx.OFF)
		
		@redVal = (255 * value).to_i       
		@greenVal = 0
		clist << @cpx.analogWrite(nil,@RGB_RED, 255 - @redVal)
		clist << @cpx.analogWrite(nil,@RGB_GREEN, 255 - @greenVal)
		@cpx.sendCommands(clist)
	end
	
	def setGreen(value) 
		clist = []
		clist << @cpx.digitalWrite(nil,@GREEN, @cpx.OFF)
		clist << @cpx.digitalWrite(nil,@RED, @cpx.ON);
		clist << @cpx.delay(nil,100)
		clist << @cpx.digitalWrite(nil,@RED, @cpx.OFF)
		
		@greenVal = (255 * value).to_i
		@redVal = 0;
		clist << @cpx.analogWrite(nil,@RGB_RED, 255 - @redVal)
		clist << @cpx.analogWrite(nil,@RGB_GREEN, 255 - greenVal)
		@cpx.sendCommands(clist)
	end
		def blink(n, delay) 
		clist = []
		value1 = @redVal
		value2 = @greenVal
		clist << @cpx.analogWrite(nil,"loop",@RGB_RED, @greenVal)
		clist << @cpx.analogWrite(nil,@RGB_GREEN, @redVal)
		if delay == 0
			clist << cpx.delay(nil,100)
		else
			clist << cpx.delay(nil,delay)
		end
		clist << cpx.analogWrite(nil,@RGB_RED, @redVal)
		clist << @cpx.analogWrite(nil,@RGB_GREEN, @greenVal)
		if delay == 0
			clist << @cpx.delay(nil,100)
		else
			clist << @cpx.delay(nil,delay)
		end
		if (n == 0)
			clist << @cpx.jump(nil,"loop")
		else
			clist << @cpx.jump(nil,"loop",n)
		end
		cpx.sendCommands(clist)
	end
	
	def fade(func,n)
	   cpx.sendCommand(call("fade",n))
	end

end

