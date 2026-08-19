function init(name) {
	// Initialize the logging processor (for example, global variables),
	proc.initOutputFile("subsystemcontrolactions"+name)
	proc.writeLine("Subsystem Control Actions")
	proc.writeLine("")
	proc.writeLine("Simulation time [s.ms]; Subsystem; Value; Fault Ongoing (0=no 1=yes); Fault type; User")
}

function parse(timestamp, tag, sender, text, status) {
	// timestamp is a double value representing the time an event occurred
	// tag is a string identifying the type of event
	// sender is a string identifying the source of the event
	// text is a string with an additional payload for the event
	// status is an object containing the most relevant parameters of the system
	if (tag === "update" && sender !== "<system>") {
		var data = JSON.parse(text)
		var key = data["key"]
		var value = data["value"]		
		var faultOngoing = (status["fault"] === "" ? 0 : 1)
		proc.writeLine(timestamp+"; " + key + "; " + value + "; " + faultOngoing + "; " + status["fault"] + "; " + sender)
	}
}

function cleanup() {
	// Cleanup the environment, close the output file	
	proc.closeOutputFile()
}

