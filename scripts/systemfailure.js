var lastTimestamp = 0 // Current timestamp
var lastFaultTimestamp = 0
var lastFault = ""
var lastDiagnostics = ""
var lastFake = 0
var lastAutomationFailure = 1

function init(name) {
	// Initialize the logging processor (for example, global variables),
	proc.initOutputFile("systemfailure"+name)
	proc.writeLine("System Failure")
	proc.writeLine("")
	proc.writeLine("Simulation time [s.ms]; Fault type; Repair type; Automation failure (1=no failure 2=miss 3=misdiagnosis 4=falsealarm); Correct diagnosis (0=no 1=yes); Diagnosis time (endrepair - startfailure); LOA Initiated; User")
}

function parse(timestamp, tag, sender, text, status) {
	// timestamp is a double value representing the time an event occurred
	// tag is a string identifying the type of event
	// sender is a string identifying the source of the event
	// text is a string with an additional payload for the event
	// status is an object containing the most relevant parameters of the system
	lastTimestamp = timestamp
	if (tag === "failure:start") {
		lastFaultTimestamp = lastTimestamp
		var data = JSON.parse(text)
		lastFault = data["fault"]
		lastDiagnostics = data["diagnostics"]
		lastFake = data["fake"]
		if (lastDiagnostics === "true") {
			lastAutomationFailure = 1
		} else if (lastDiagnostics === "miss") {
			lastAutomationFailure = 2
		} else if (lastDiagnostics != lastFault) {
			lastAutomationFailure = 3
		} else if (lastFake > 0) {
			lastAutomationFailure = 4
		}
	} else if (tag === "failure:repair-wrong-nofault") {
		// Repair in no fault situation
		var data = JSON.parse(text)
		proc.writeLine(lastTimestamp+"; ;" + data["fault"] + "; 1; 0; -1; false; "+ sender)
	} else if (tag === "failure:loa-repair-wrong-nofault") {
		// Repair initiated by LOA in no fault situation
		var data = JSON.parse(text)
		proc.writeLine(lastTimestamp+"; ;" + data["fault"] + "; 1; 0; -1; true; "+ sender)
	} else if (tag === "failure:repair-wrong") {
		// Wrong repair
		var data = JSON.parse(text)
		proc.writeLine(lastTimestamp+"; " + lastFault +"; " + data["fault"] + "; " + lastAutomationFailure + "; 0; -1; false; "+ sender)
	} else if (tag === "failure:loa-repair-wrong") {
		// Wrong repair initiated by LOA
		var data = JSON.parse(text)
		proc.writeLine(lastTimestamp+"; " + lastFault +"; " + data["fault"] + "; " + lastAutomationFailure + "; 0; -1; true; "+ sender)
	} else if (tag === "failure:repair-correct") {
		// Repair in no fault situation
		var data = JSON.parse(text)
		var diagnoseTime = lastTimestamp - lastFaultTimestamp
		proc.writeLine(lastTimestamp+"; " + lastFault +"; " + data["fault"] + "; " + lastAutomationFailure + "; 1; " + diagnoseTime + "; false; "+ sender)
	} else if (tag === "failure:loa-repair-correct") {
		// Repair in no fault situation
		var data = JSON.parse(text)
		var diagnoseTime = lastTimestamp - lastFaultTimestamp
		proc.writeLine(lastTimestamp+"; " + lastFault +"; " + data["fault"] + "; " + lastAutomationFailure + "; 1; " + diagnoseTime + "; true; "+ sender)
	} else if (tag === "failure:cleared") {
		proc.writeLine(lastTimestamp+"; " + lastFault +"; " + lastFault + "; " + lastAutomationFailure + "; -1; " + diagnoseTime + "; true; <cleared>")
		lastFaultTimestamp = 0
		lastFault = ""
		lastDiagnostics = ""
		lastFake = 0
		lastAutomationFailure = 1
	}
	
}

function cleanup() {
	// Cleanup the environment, close the output file
	proc.closeOutputFile()
}
