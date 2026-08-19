function init(name) {
	// Initialize the logging processor (for example, global variables), open
	// destination file
	proc.initOutputFile("sample_"+name)
}

function parse(timestamp, tag, sender, text, status) {
	// timestamp is a double value representing the time an event occurred
	// tag is a string identifying the type of event
	// sender is a string identifying the source of the event
	// text is a string with an additional payload for the event
	// status is an object containing the most relevant parameters of the system
}

function cleanup() {
	// Cleanup the environment, close the output file
	proc.closeOutputFile()
}

// TODO: Implementare sistema per scrivere nei file
// TODO: Gestire output in file diversi nella directory specificata
