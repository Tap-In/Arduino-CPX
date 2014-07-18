require './cpx.rb'
require './doohickey.rb'

RED = 6
#c = CPX.new("http://50.16.114.126:8085/cpx","BEN","5551212");
c = CPX.new("http://localhost:8085/cpx","BEN","5551212");

puts c.sendCommand(c.listDevices())                 # list all the devices

#c.sendCommand(c.ping())

#c.sendCommand(c.digitalWrite("x",6,1) )              # on
#c.sendCommand(c.digitalWrite("x",6,0) )              # off


#
# Blink 10 times
#

commands = []
commands << c.digitalWrite("loop",6,0)
commands << c.delay(nil,250)
commands << c.digitalWrite(nil,6,1)
commands << c.delay(nil,250)
commands << c.jump(nil,"loop",10)
# c.sendCommands(commands);


#Fade once
puts c.sendCommand(c.call("fade","1"))

# Set to white
commands = []
commands << c.digitalWrite("loop",44,0)
commands << c.digitalWrite("loop",45,0)
commands << c.digitalWrite("loop",46,0)
c.sendCommands(commands);

#Intense red on pin 6
for i in 0..10
	puts i
	c.sendCommand(c.analogWrite(nil,6,i));
	sleep(1)
end

