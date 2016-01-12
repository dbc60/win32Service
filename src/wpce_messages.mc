; // messages.mc for the Win32 Pipe Client Example

; // This is the header section.
SeverityNames =
(
    Success=0x0:STATUS_SEVERITY_SUCCESS
    Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
    Warning=0x2:STATUS_SEVERITY_WARNING
    Error=0x3:STATUS_SEVERITY_ERROR
)

LanguageNames =
(
    English=0x409:MSG00409
)


; // The following are the categories of events.
MessageIdTypedef=WORD

MessageId=0x1
SymbolicName=WPCE_SVC_MSG_CATEGORY
Language=English
Win32 Pipe Client Example Events
.


; // The following are the message definitions.
MessageIdTypedef=DWORD

MessageId=0x100
Severity=Informational
SymbolicName=WPCE_SVC_MSG_INITIALIZING
Language=English
The Win32 Pipe Client Example is initializing.
.

MessageId=+1
Severity=Error
SymbolicName=WPCE_SVC_MSG_INIT_LOGGING_FAILED
Language=English
The Win32 Pipe Client Example failed to initialize its trace-logging subsystem: %1.
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_INIT_LOGGING_SUCCESS
Language=English
The Win32 Pipe Client Example initialize its trace-logging subsystem: path(%1).
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_INITIALIZED
Language=English
The Win32 Pipe Client Example is initialized.
.

MessageId=+1
Severity=Informational
SymbolicName=WPCE_SVC_MSG_REGISTERING_SCM
Language=English
The Win32 Pipe Client Example is registering its service control handler with the Service Control Manager.
.

MessageId=+1
Severity=Informational
SymbolicName=WPCE_SVC_MSG_REGISTERED_SCM
Language=English
The Win32 Pipe Client Example registered with the Service Control Manager.
.

MessageId=+1
Severity=Informational
SymbolicName=WPCE_SVC_MSG_REPORTED_START_PENDING
Language=English
The Win32 Pipe Client Example reported start pending to the Service Control Manager.
.

MessageId=+1
Severity=Informational
SymbolicName=WPCE_SVC_MSG_REPORTED_STARTED
Language=English
The Win32 Pipe Client Example is running.
.

MessageId=+1
Severity=Error
SymbolicName=WPCE_SVC_MSG_SCM_REPORT_FAILURE
Language=English
The Win32 Pipe Client Example failed while reporting to the Service Control Manager.
.

MessageId=+1
Severity=Error
SymbolicName=WPCE_SVC_MSG_REGISTRATION_FAILURE
Language=English
The Win32 Pipe Client Example failed to register with the Service Control Manager. Error code: %1.
.

MessageId=+1
Severity=Error
SymbolicName=WPCE_SVC_MSG_INITIALIZATION_FAILURE
Language=English
The Win32 Pipe Client Example failed to initialize. Error code: %1.
.

MessageId=+1
Severity=Error
SymbolicName=WPCE_SVC_MSG_START_FAILURE
Language=English
The Win32 Pipe Client Example failed to start. Error code: %1.
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_PRESHUTDOWN
Language=English
The Win32 Pipe Client Example is preparing to stop because the server will shut down.
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_SHUTTING_DOWN
Language=English
The Win32 Pipe Client Example is stopping because the server is shutting down.
.

MessageId=+1
Severity=Informational
SymbolicName=WPCE_SVC_MSG_STOPPING
Language=English
The Win32 Pipe Client Example is stopping.
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_STOPPED
Language=English
The Win32 Pipe Client Example has stopped.
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_PAUSED
Language=English
The Win32 Pipe Client Example is paused.
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_CONTINUING
Language=English
The Win32 Pipe Client Example is continuing.
.

MessageId=+1
Severity=Informational
SymbolicName=WPCE_SVC_MSG_POWER_EVENT
Language=English
The Win32 Pipe Client Example detected a power event.
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_POWER_EVENT_AUTO_RESUME
Language=English
The Win32 Pipe Client Example is resuming from a previous power event.
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_POWER_EVENT_SUSPEND
Language=English
The Win32 Pipe Client Example is suspending service due to a power event.
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_UNINSTALL_SERVICE
Language=English
The Win32 Pipe Client Example is stopping and will be uninstalled.
.

MessageId=+1
Severity=Success
SymbolicName=WPCE_SVC_MSG_MONITORING
Language=English
The Win32 Pipe Client Example is monitoring changes.
.

MessageId=+1
Severity=Error
SymbolicName=WPCE_SVC_MSG_EVENT_SINK_FAILURE
Language=English
The Win32 Pipe Client Example failed to register event sink: %1.
.

MessageId=+1
Severity=Error
SymbolicName=WPCE_SVC_MSG_READING_SUPPORTED_DEVICES_FAILURE
Language=English
The Win32 Pipe Client Example failed to read and parse the list of supported devices.
.
