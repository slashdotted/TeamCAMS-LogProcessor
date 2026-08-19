var timestampAtRoundStart = -1 // Timestamp of the last start of a round
var loggedResponses = {} // Responses
var faultAtRoundStart = "" // Fault at the last start of a round
var usersConnectedAtRoundStart = [] // Users connected a the last start of a round
var lastTimestamp = 0 // Current timestamp
var responseTemplate = { "timestamp" : 0, "completed" : false, "responseTime" : -1 }

function init(name) {
	// Initialize the logging processor (for example, global variables),
	proc.initOutputFile("transmission_control"+name)
	proc.writeLine("Transmission  Control")
	proc.writeLine("")
	proc.writeLine("Simulation time [s.ms]; task completed (0=no 1=yes); Response time [ms]; Fault ongoing (0=no 1=yes); Fault type; User")
}

function startNextRound(timestamp, tag, sender, connectedUsers, status) {
	// Start next round
	usersConnectedAtRoundStart = []
	loggedResponses = {} // Reset responses
	for (i = 0; i < connectedUsers.length; i++) {
		var udata = JSON.parse(JSON.stringify(responseTemplate))
		udata["timestamp"] = timestamp
		loggedResponses[connectedUsers[i]] = udata
		usersConnectedAtRoundStart.push(connectedUsers[i])
	}
	faultAtRoundStart = status["fault"]
	timestampAtRoundStart = timestamp
}

function finishRound() {
	if (timestampAtRoundStart != -1) {
		for (i = 0; i < usersConnectedAtRoundStart.length; i++) {
			var user = usersConnectedAtRoundStart[i]
			var response = loggedResponses[user]
			proc.writeLine(response["timestamp"]  
				+ ";" + (response["completed"] ? 1 : 0)
				+ ";" + (response["completed"] ? response["responseTime"] : '')
				+ ";" + (faultAtRoundStart !== "" ? 1 : 0)
				+ ";" + faultAtRoundStart 
				+ ";" + user)
		}
		timestampAtRoundStart = -1
	}
}

function parse(timestamp, tag, sender, text, status) {
	// timestamp is a double value representing the time an event occurred
	// tag is a string identifying the type of event
	// sender is a string identifying the source of the event
	// text is a string with an additional payload for the event
	// status is an object containing the most relevant parameters of the system
	lastTimestamp = timestamp
	if (tag === "secondary:transmission.notify") {
		// Transmission check notification
		finishRound()
		startNextRound(timestamp, tag, sender, JSON.parse(text), status)
	} else if (tag === "secondary:transmission.response") {
		// Transmission check valid response
		var data = JSON.parse(text)
		loggedResponses[sender] = { "timestamp" : timestamp, "completed" : true, "responseTime" : data['responseTime'] }
	} else if (tag === "secondary:transmission.invalid") {
		// Transmission check invalid response
		proc.writeLine(timestampAtRoundStart 
				+ ";0; "
				+ ";" + (status["fault"] !== "" ? 1 : 0)
				+ ";" + status["fault"] 
				+ ";" + sender +";")
	}
	
}

function cleanup() {
	// Cleanup the environment, close the output file
	finishRound()
	proc.closeOutputFile()
}
