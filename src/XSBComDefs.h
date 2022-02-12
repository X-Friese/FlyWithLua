/* 
 * Copyright (c) 2004, Ben Supnik and Chris Serio.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */


#ifndef XSB_COM_DEFS
#define XSB_COM_DEFS

/*
	XSQUAWKBOX INTERPLUGIN INTERFACES
	
	GENERAL NOTES:
	
	All commands are sent from you to XSquawkBox directly.  Notifications are sent
	from XSquawkBox to you only if you subscribe to their category.
	
	Most structures are passed by pointer in the param to the command.  Always fill
	in structsize with the number of bytes in the structure.  For any parameter
	you do not want to provide, pass NULL for a string, or -1 for an integer to use
	the last stored value from XSquawkBox.  To set a field to nothing, pass an empty 
	string.  All strings are null terminated 8-bit ASCII C-strings.
	
	SCRIPTING ACTIONS
	
	Many of the actions you can script come in two parts when a dialog box exists
	in the application:
	
	 1 A command to pose the dialog box.
	 2 A command to execute the commaand without a dialog box.
	 
	For example, for login, you can do a fully automated login or pose the dialog
	box.  You can use datarefs 
	
	NOTIFICATIONS
	
	Your plugin must ask XSquawkBox to receive notifications.  Notifications are messages
	with the high bit set that come from XSB as things happen.  You send subscribe and
	unsubscribe commands to XSB to control which notifications you will receive.  For 
	example, if your plugin uses private messages to tune Roger Wilco, you would 
	subscribe to text messages to receive copies of all incoming radio transmissions.
	All messages come in categories; you subscribe to a whole category of messages.
	
	DATAREFS
	
	Datarefs that are marked as strings are binary arrays of chars, making a null-terminated
	C-string.  Always write them at offset zero with a null, e.g.
	
	XPLMSetDatab(XPLMFindDataRef(XSB_CON_CALLSIGN), "BAW1384", 0, 8); // 7 chars + 1 null = 8
	
	Some datarefs are used only to hold data while messages are being passed.  The contents
	of these datarefs is undefined when these messages are not being sent out.
	
	A NOTE ABOUT FLIGHT PLANS
	
	Flight plans can be edited by three sources:
	
	 - The user typing in the flight plan dialog and clicking ok.
	 - A third party program editing the XSB prefs file.
	 - A third party plugin modifying datarefs.
	
	There is always the risk of overwriting each other when this happens.  If your plugin
	edits the flight plan datarefs after the prefs file is modified and then the dialog
	box is posed, the prefs file will win.  To prevent this, always either pose the 
	dialog box or send the flight plan immediately after editing it to make sure your
	changes are not lost.
	
	USAGE
	
	Using these datarefs and commands and notifications you can:
	
	 - Automate logging in and/or sending flight plans.
	 - Determine when audio is being played and/or open/close the mic if you must coordinate audio usage.
	 - Modify the current flight plan.	 
	 - Create new "dot" commands for scripting and aliases.
	 - Get text coming over the network and auto-tune Roger Wilco or TeamSpeak.	 
	
*/	

/**************************************************************************************************************
 * DATA REFS
 **************************************************************************************************************/

/*
 * The following datarefs are permanent and good at all times.  Most are writable.
 *
 */

#define XSB_VERS_NUMBER		"xsquawkbox/vers/number"		// int,  100 = 1.0
#define XSB_VERS_STRING		"xsquawkbox/vers/string"		// string, e.g. 1.0b4

/* Login dialog box datarefs.  Note that if the connection/login dialog box is showing, you must repose
 * it for the changes you made to become visible. */
#define	XSB_CON_CALLSIGN		"xsquawkbox/login/callsign"		// string
#define	XSB_CON_SERVER			"xsquawkbox/login/server"		// string (DNS address or IP as string
#define	XSB_CON_PORT			"xsquawkbox/login/port"			// int
#define	XSB_CON_PILOT_ID		"xsquawkbox/login/pilot_id"		// string
#define	XSB_CON_PASSWORD		"xsquawkbox/login/password"		// string
#define	XSB_CON_REALNAME		"xsquawkbox/login/realname"		// string
#define	XSB_CON_MODEL			"xsquawkbox/login/model"		// string
#define XSB_CON_STATUS			"xsquawkbox/login/status"		// int, 0 = disconnected, 1 = connecting, 2 = connected, NOT WRITABLE

/* The current flight plan.  Note that if you change it while the FP dialog is opened, the dialog will not
 * be changed until you repose that dialog box.  Also on VATSIM ATC may "own" the current FP.  In that case
 * (for example in flight or after ATC has amended it) changes to these data refs may be overwritten at various
 * times.  This is a VATSIM-specific issue. */
#define XSB_FP_FLIGHT_TYPE				"xsquawkbox/fp/flight_type"				// int, either I, V, D, or S
#define XSB_FP_TCAS_TYPE				"xsquawkbox/fp/tcas_type"				// int, T, H, B, L, or 0
#define XSB_FP_NAV_TYPE					"xsquawkbox/fp/nav_type"				// int, F, G, R, A, etc.
#define XSB_FP_SPEED					"xsquawkbox/fp/speed"					// int, knots
#define XSB_FP_DEPARTURE_AIRPORT		"xsquawkbox/fp/departure_airport"		// string, ICAO code
#define XSB_FP_DEPARTURE_TIME			"xsquawkbox/fp/departure_time"			// int, zulu time, e.g. 1340 = 13:40
#define XSB_FP_DEPARTURE_TIME_ACTUAL	"xsquawkbox/fp/departure_time_actual"	// int, zulu time
#define XSB_FP_CRUISE_ALTITUDE			"xsquawkbox/fp/cruise_altitude"			// string, either FLxxx or yyyyy
#define XSB_FP_ARRIVAL_AIRPORT			"xsquawkbox/fp/arrival_airport"			// string, ICAO code
#define XSB_FP_ENROUTE_HRS				"xsquawkbox/fp/enroute_hrs"				// int, hrs
#define XSB_FP_ENROUTE_MINS				"xsquawkbox/fp/enroute_mins"			// int, mins
#define XSB_FP_FUEL_HRS					"xsquawkbox/fp/fuel_hrs"				// int, hrs
#define XSB_FP_FUEL_MINS				"xsquawkbox/fp/fuel_mins"				// int, mins
#define XSB_FP_ALTERNATE_AIRPORT		"xsquawkbox/fp/alternate_airport"		// string, ICAO code
#define XSB_FP_REMARKS					"xsquawkbox/fp/remarks"					// string
#define XSB_FP_ROUTE					"xsquawkbox/fp/route"					// string

/* These datarefs contain the mic state (open = playing sound over network, closed = no sound) and 
 * enabled state.  (You can open or close it yourself or let XSB decide using the usual PTT keys. */
#define XSB_MIC_OPEN			"xsquawkbox/mic/open"					// int, 1 if the mic is open right now, NOT WRITABLE
#define XSB_MIC_ENABLED			"xsquawkbox/mic/enabled"				// int, 0 to mute mic, 1 to let XSB decide, 2 to force open



/*
 * The following datarefs are used to hold extra info about messages being sent; they are
 * only good while the message is being sent, i.e. read them immediately when you get
 * the notification message. 
 *
 */


/* This dataref stores the current METAR that is being processed during an XSB_MSG_METAR notification.
 * You cannot actually change the incoming weather by writing this. */
#define XSB_WEATHER_METAR		"xsquawkbox/weather/metar"				// string, the metar in question when MSG_METAR is sent, NOT WRITABLE

/* This data ref stores the string that you want to print on the screen.  Set this dataref, then send an XSB_CMD_USER_MSG
 * to XSquawkBox. */
#define XSB_INPUT_USER_MSG		"xsquawkbox/input/user_msg"				// string, what will be printed when CMD_USER_MSG is sent.

/* This data ref stores the string that was typed by the user that is going to XSquawkBox to be sent over the
 * network during an XSB_MSG_USER_COMMAND.  If you want to repress this string, set it to an empty string
 * while you process it. */
#define XSB_INPUT_STRING		"xsquawkbox/input/string"				// The currently being-handled user input command.

/* These data refs are used to store a transmission coming from the network to be shown to the user
 * while your plugin examines it in response to the XSB_MSG_TEXT notification. */
#define XSB_TEXT_MESSAGE		"xsquawkbox/text/msg"					// string, the current incoming message
#define XSB_TEXT_FROM			"xsquawkbox/text/from"					// string, who is it from
#define XSB_TEXT_FREQS			"xsquawkbox/text/frqs"					// int array, incoming freqs in khz (25 khz friendly), NOT WRITABLE
#define XSB_TEXT_USE			"xsquawkbox/text/use"					// int, whether XSB should show this message (defaults to 1)

/* These datarefs are used to query frequencies for pilots. */
#define XSB_ATC_CALLSIGN		"xsquawkbox/atc/callsign"				// String, the callsign you want to know about
#define XSB_ATC_FREQ			"xsquawkbox/atc/freq"					// Freq in khz they are on or 0 if unknown

/**************************************************************************************************************
 * XSQUAWKBOX COMMANDS
 **************************************************************************************************************/


/*
 * SUBSCRIBE AND UNSUBSCRIBE
 *
 * To receive any notifications from XSquawkBox, you must subscribe to them.  Do this
 * by sending a subscribe command to XSB.  The param is the union of bit fields for which
 * you want to start receiving notifications.
 * 
 * Each catagory of notifications has a separate bit field.  Flags set in the subscribe
 * command are _added_ to your total set of subscriptions.  Subscribing to an already
 * subscribed notification has no effect.
 *
 * Use the unsubscribe command to stop receiving notifications.  Unsubscribing from
 * a notification catagory that is not already subscribed has no effect.
 *
 */
 
/* Notification catagory flags */
#define XSB_NETWORK		0x00000001
#define XSB_AUDIO		0x00000002
#define XSB_WEATHER		0x00000004
#define XSB_TEXT		0x00000008

/* Add some notifications that you care about. */
#define	XSB_CMD_SUBSCRIBE	0x01000001 /* Param is bit flags for whom to subscribe to. */

/* Unsubscribe to somme notifications. */
#define XSB_CMD_UNSUBSCRIBE 0x01000002 /* Param is bit flags for whom to subscribe to. */

/* This command poses the user's login dialog box. */
#define XSB_CMD_USER_LOGIN	0x01000003

/* This command connects to the network without posing the login dialog box. */
#define XSB_CMD_CONNECT		0x01000004

/* This command disconnect's from the network. */
#define XSB_CMD_DISCONNECT	0x01000005


/* This command poses the flight plan dialog box. */
#define XSB_CMD_SHOW_FP		0x01000007

/* This command sends the FP to ATC. */
#define XSB_CMD_SEND_FP		0x01000008


/*
 * SENDING MESSASGES TO THE USER
 *
 * You can send a message to the user that appears as a 'red' system message as follows:
 * First set the XSB_INPUT_USER_MSG dataref to the string you want.
 *
 */
#define XSB_CMD_USER_MSG	0x01000009

/* Pass an ATC callsign into XSB_ATC_CALLSIGN, you'll get back a freq in khz in
 * XSB_ATC_FREQ */
#define XSB_CMD_LOOKUP_ATC	0x0100000A

/**************************************************************************************************************
 * NOTIFICATIONS SENT BY XSQUAWKBOX
 **************************************************************************************************************/


/* XSB_NETWORK - Network catagory notifications. */

/* This is sent whenever XSB is connected to the network. */
#define XSB_MSG_CONNECTED		0x80000001

/* This is sent whenever XSB is disconnected from the network.
 * WARNING: this may be sent even if you were not connected if
 * a connection attempt fails.  After connecting, you will get
 * one of the connected or disconnected messages when the 
 * connection timeout has passed. */
#define XSB_MSG_DISCONNECTED	0x80000002



/* XSB_AUDIO - Audio catagory notifications. */

/* This notification is sent whenever the existence of incoming audio
 * changes.  The first parameter is 1 for audio coming in and 0 for not. */
#define XSB_MSG_INCOMING_AUDIO	0x80000003 /* Param is 1 for audio, 0 for none */

/* This notification is sent as the mic is opened and closed. */
#define XSB_MSG_MIC_STATUS		0x80000004 /* Param is 1 for mic open, 0 for not */

/* This notification is sent whenever the user changes com frequencies.
 * The user may be tuning to a new controller or no controller; you actually
 * cannot tell from the PCSB protocol.  Once you see this message, stop old voice
 * services and wait to see a valid room to resume voice services. */
#define XSB_MSG_COM_CHANGE		0x80000005


/* XSB_WEATHER - Weather category notifications. */

/* This message is sent when a METAR is received (for any reason).
 * The METAR is in XSB_WEATHER_METAR */
#define XSB_MSG_METAR			0x80000006 



/* XSB_TEXT - text catagory notifications. */

/* This is sent whenever incoming text comes to XSquawkBox.
 * See the TEXT_ data refs for controlling it. */
#define XSB_MSG_TEXT			0x80000007

/* This is sent when the user types an input.  You can make your own dot-commands this way. */
/* See MSG_INPUT for the string.  Set it to empty to cause the string to not be used. */
#define XSB_MSG_USER_COMMAND	0x80000008

/* This message is sent when the 'about box' is displayed as text in the window.  You can
 * send message commands to add your own. */
#define XSB_MSG_ABOUT_BOX		0x80000009

#endif
