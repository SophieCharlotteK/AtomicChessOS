#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <cstdint>
#include <fstream>
#include <streambuf>
#include <regex>

#include <thread>
//#include <format> // FOR FORMAT STRING

// INCLUDE AND TYPEDEF FOR SYSTEM CLOCK
//USED FOR SYSTEM TICK IN MAIN LOOP
#include <ctime>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::_V2::system_clock::time_point TimePoint;
#define CHRONO_DURATION_MS(x,y) std::chrono::duration_cast<std::chrono::milliseconds>(x - y).count()


//INCLUDE SHARES RESSOURCES
#include "SHARED/guicommunicator/guicommunicator.h"
#include "SHARED/loguru-master/loguru.hpp"
//INCLUDE LOCAL RESSOURCES
#include "TMC5160.h"
#include "BackendConnector.h"
#include "ChessPiece.h"
#include "ConfigParser.h"
#include "ChessBoard.h"
#include "IOController.h"
#include "StateMachine.h"

//---------------------- CONFIG DEFINED --------------------------- //
#define CONFIG_FILE_PATH "./atccontrollerconfig.ini"


using namespace std;

//!Reads the system HWID File from the location
std::string readHWID(std::string _file)
{
	std::ifstream t(_file.c_str());
	std::string str;

	t.seekg(0, std::ios::end);   
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
}


int mainloop_running = 0;
void signal_callback_handler(int signum)
{
	printf("Caught signal %d\n", signum);
	mainloop_running = signum;
	//exit(signum);
}

std::string get_interface_mac_address(const string& _ifname) {
	ifstream iface("/sys/class/net/" + _ifname + "/address");
	//READ FILE
	std::string str((istreambuf_iterator<char>(iface)), istreambuf_iterator<char>());
	//CHECK LENGTH
	if(str.length() > 0) {
		//REPLACE ILLEGAL CHARAKTERS
		std::string hex = regex_replace(str, std::regex("[,:.;/\\\n\t\r ]"), "");
		return hex;
	}
	else {
		return "1nv4l1dm4c";
	}
} 

std::string read_file_to_string(const std::string& _path) {
	ifstream iface(_path);
	std::string str((istreambuf_iterator<char>(iface)), istreambuf_iterator<char>());
	if (str.length() > 0) {
		return str;
	}
	else {
		return "";
	}
} 




int main(int argc, char *argv[])
{
	
	

	
	
	
		
	
	
	
		//REGISTER SIGNAL HANDLER
		signal(SIGINT, signal_callback_handler);
	
	

	
	//SETUP LOGGER
	loguru::init(argc, argv);
	loguru::add_file("./log.log", loguru::Append, loguru::Verbosity_MAX);
	loguru::g_stderr_verbosity = 1;
	LOG_SCOPE_F(INFO, "ATC CONTROLLER STARTED");
	
	//READ CONFIG FILE
	LOG_SCOPE_F(INFO, "LOADING CONFIG FILE ./atccontrollerconfig.ini");
	ConfigParser::getInstance()->loadDefaults();     //LOAD (PUPULATE) ALL CONFIG ENTRIES WITH THE DEFAULT CONFIG
	//GENERATE A DEFAULT CONFIG FILE IN DEBUG MODE TO TEST THE CONFIG GENERATION
#ifdef DEBUG
	ConfigParser::getInstance()->createConfigFile(CONFIG_FILE_PATH, false);
#endif
	//OVERWRITE WITH EXISTSING CONFIG FILE SETTINGS
	if(!ConfigParser::getInstance()->loadConfigFile(CONFIG_FILE_PATH))
	{
		LOG_F(ERROR, "Failed to load atccontrollerconfig.ini");
		return 1;
	}
	LOG_F(INFO, "CONFIG FILE LOADED");	
	
	
	
	
	
	
	ChessBoard board;
	board.initBoard(false);
	board.test();
	
	

	
	while (1)
	{
		
	}
	
	
	/*
	
		
	//SARTING GUI COMMUNICATOR PROCESS
	LOG_F(INFO, "guicommunicator startig ipc thread");
	guicommunicator gui;
	gui.start_recieve_thread();
	
	//WAIT FOR GUI TO BECOME REACHABLE
	int gui_wait_counter = 0;
	//TRY X TIMES BEFORE RETURN AN ERROR
	const int GUI_WAIT_COUNTER_MAX = 100;
	while (!gui.check_guicommunicator_reachable())
	{
		gui_wait_counter++;
		if (gui_wait_counter > GUI_WAIT_COUNTER_MAX)
		{
			break;
		}
	}
	//
	if(gui_wait_counter > GUI_WAIT_COUNTER_MAX)
	{
		LOG_F(WARNING, "guicommunicator check_guicommunicator_reachable check failed");
		return 2;
	}
	
	
	gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::PROCESSING_SCREEN);
	IOController io;
	io.setTurnStateLight(IOController::TURN_STATE_LIGHT::TSL_PRECCESSING);
	//CHECK VERSION ON GUI SIDE
	if(gui.check_guicommunicator_version())
	{
		LOG_F(WARNING, "guicommunicator version check failed");
	}
	//DETERM THE HWID BY USING THE MAC ADDRESS OF THE OUTGOING INTERNFACE NAME
	std::string hwid = get_interface_mac_address(ConfigParser::getInstance()->get(ConfigParser::CFG_ENTRY::GENERAL_HWID_INTERFACE));
	gui.createEvent(guicommunicator::GUI_ELEMENT::INFOSCREEN_HWID_LABEL, guicommunicator::GUI_VALUE_TYPE::USER_INPUT_STRING, hwid);
	LOG_F(INFO, (const char*)hwid.c_str());
	
	
	//DISPLAY FIRMARE VERSION NUMBER
	std::string fwver = ConfigParser::getInstance()->get(ConfigParser::CFG_ENTRY::GENERAL_VERSION_FILE_PATH);
	std::string hwrev = ConfigParser::getInstance()->get(ConfigParser::CFG_ENTRY::GENERAL_HWREV_FILE_PATH);
	std::string bootpart = ConfigParser::getInstance()->get(ConfigParser::CFG_ENTRY::GENERAL_BOOT_PARTION_INFO_FILE_PATH);
	gui.createEvent(guicommunicator::GUI_ELEMENT::INFOSCREEN_VERSION, guicommunicator::GUI_VALUE_TYPE::USER_INPUT_STRING, fwver + "|" + hwrev + "|" + bootpart);
	LOG_F(INFO, (const char*)fwver.c_str());
	
	
	//INIT CHESSBOARD
	ChessBoard board;
	//INIT THE CHESS BOARD MECHANIC
	//=> HOME, SETUP COILS
	io.setTurnStateLight(IOController::TURN_STATE_LIGHT::TSL_PLAYER_WHITE_TURN);
	bool board_scan = true;
	if (gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_A_OK_CANCEL, "CHESS FIGURES PLACED IN PARKING POSITIONS?", 10000) != guicommunicator::GUI_MESSAGE_BOX_RESULT::MSGBOX_RES_OK)
	{
		board_scan = false;
	}
	else if (gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_A_OK_CANCEL, "CHESS FIGURES PLACED IN PARKING POSITIONS?", 10000) != guicommunicator::GUI_MESSAGE_BOX_RESULT::MSGBOX_RES_CANCEL)
	{
		board_scan = true;
	}
	io.setTurnStateLight(IOController::TURN_STATE_LIGHT::TSL_PRECCESSING);
	gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::PROCESSING_SCREEN);
	while (board.initBoard(board_scan) != ChessBoard::BOARD_ERROR::INIT_COMPLETE)
	{
		if (gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_A_OK_CANCEL, "BOARD_INIT_FAILED RETRY?", 10000) != guicommunicator::GUI_MESSAGE_BOX_RESULT::MSGBOX_RES_OK) {
			break;
		}
	}
	

	
	//CREATE GAME BACKEND INSTANCE
	BackendConnector gamebackend(ConfigParser::getInstance()->get(ConfigParser::CFG_ENTRY::NETWORK_BACKEND_URL), ConfigParser::getInstance()->get(ConfigParser::CFG_ENTRY::GENERAL_HWID_INTERFACE), hwid);
	//SET NEEDED SETTINGS
	int gamebackend_heartbeat_interval = 5;
	ConfigParser::getInstance()->getInt(ConfigParser::CFG_ENTRY::NETWORK_HEARTBEAT_INTERVAL_SECS, gamebackend_heartbeat_interval);
	gamebackend.setHearbeatCallInterval(gamebackend_heartbeat_interval);
	
	//NOW TRY TO CONNECT TO THE SERVER
	std::string ALTERNATIVE_BACKEND_URL[] = { "http://192.168.178.125:3000", "http://192.168.178.24:3000", "http://atomicchess.de:3000", "http://marcelochsendorf.com:3000", "http://marcelochsendorf.com:3001", "http://marcelochsendorf.com:3002", "http://prodevmo.com:3001", "http://prodevmo.com:3002", "http://127.0.0.1:3000" };	
	//CHECK IF GAMESERVER IS REACHABLE ELSE USE A OTHER PREDEFINED URL
	volatile int abu_counter = 0;
	volatile bool abu_result = true;
	while (!gamebackend.check_connection()) {
		gamebackend.set_backend_base_url(ALTERNATIVE_BACKEND_URL[abu_counter]);
		LOG_F(INFO, "gamebackend - change backendurl due prev not abariable");
		LOG_F(INFO, "%s", ALTERNATIVE_BACKEND_URL[abu_counter].c_str());
#ifdef DEBUG
		gui.show_error_message_on_gui("" + gamebackend.get_backend_base_url() + "");
#endif
		abu_counter++;
		//MARK SEARCH FAILED
		if(abu_counter >= 6)
		{
			abu_result = false;
			break;
		}
	}
	//UPDATE GUI THAT NETWORK IS ONLINE
	if(abu_result) {
		gui.createEvent(guicommunicator::GUI_ELEMENT::NETWORK_STATUS, guicommunicator::GUI_VALUE_TYPE::ONLINE);
		//SHOW MESSAGEBOX IF THE CURRENT URL IS A DIFFERENT THAN IN THE CONFIG
		if(gamebackend.get_backend_base_url() != ConfigParser::getInstance()->get(ConfigParser::CFG_ENTRY::NETWORK_BACKEND_URL))
		{
			if (gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_B_OK, "GAME_SERVER_URL_CHANGED:" + gamebackend.get_backend_base_url(), 1000) != guicommunicator::GUI_MESSAGE_BOX_RESULT::MSGBOX_RES_OK) {
				gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::PROCESSING_SCREEN);
			}
		}
	}
	else {
		//CONNECTION FAILED => EXIT
	   gui.createEvent(guicommunicator::GUI_ELEMENT::NETWORK_STATUS, guicommunicator::GUI_VALUE_TYPE::OFFLINE);
		gui.show_error_message_on_gui("Cant connect to game server. (ERR01) [" + gamebackend.get_backend_base_url() + "]");
		return 3;
	}
	
	

	
	
	//PERFORM A LOGOUT
	if(gamebackend.logout())
	{
		LOG_F(ERROR, "gamebackend - LOGIN");	
	}else
	{
		LOG_F(ERROR, "gamebackend - logout failed");
		gui.show_error_message_on_gui("LOGOUT FAILED");

	}
	
	gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::LOGIN_SCREEN);
	
	

	
	//INIT SYSTEM TIMER
	TimePoint t1 = Clock::now();
	TimePoint t2 = Clock::now();
	int SYSTEM_TICK_DELAY = 0;
	if (!ConfigParser::getInstance()->getInt(ConfigParser::CFG_ENTRY::GENERAL_SYSTEM_TICK_INTERVAL_MS, SYSTEM_TICK_DELAY)) {
		LOG_F(ERROR, "GENERAL_SYSTEM_TICK_INTERVAL_MS - config get entry  failed");
	}
	LOG_F(INFO, "SYSTEM_TICK_DELAY SET %i", SYSTEM_TICK_DELAY);
	
	//INIT SYSTEM STATE MACHINE
	StateMachine state_machiene;
	
	//DISCARD ALL GUI EVENTS
	gui.clearPreviousEvents();
	//ENTERING MIAN LOOP
	io.setTurnStateLight(IOController::TURN_STATE_LIGHT::TSL_IDLE);
	while (mainloop_running == 0)
	{
		
	
		t2 = Clock::now();
		//IF TIME DIFFERENCE IS BIG ENOUGHT => CALL THE TIMER EVENT
		if(CHRONO_DURATION_MS(t2, t1) > SYSTEM_TICK_DELAY)
		{
			t1 = Clock::now();
		

		
			//IF A VALID SESSION
			if(gamebackend.check_login_state())
			{
				//THE THE PLAYER STATUS
				//WHICH CONTAINS ALL INFORMATION ABOUT THE CURRENT STATE OF GAME / MATCHMAKING / GENERAL STATE OF THE CLIENT
				BackendConnector::PLAYER_STATUS ps = gamebackend.get_player_state();
				
				
				//BASIC ERROR HANDLING
				//HANDLING LOGOUT / INVALID SESSION
				if(ps.err == "err_session_key_sid_check_failed" || ps.err == "err_session_check_failed" || ps.err == "err_session_key_sid_check_failed") {
					if (gamebackend.stop_heartbeat_thread())
					{
						gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_B_OK, "LOGOUT_FAILED_HEARTBEAT_STOP", 4000);
						LOG_F(ERROR, "GOT LOGIN_FAILED_HEARTBEAT STOP THREAD");
					}
						
					gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::LOGIN_SCREEN);
					
				}else if(ps.err == "err_query_paramter_hwid_or_sid_or_not_set")
				{
					gui.show_error_message_on_gui("get_player_state - ERROR err_query_paramter_hwid_or_sid_or_not_set");
					LOG_F(ERROR, "GOT err_query_paramter_hwid_or_sid_or_not_set");
				}
				
				//FOR ALL OTHER EVENTS USE THE STATE MACHINE HANDLING CLASS
				//WHICH HANDLES THE GAME LOGIC STATES
				//THE CALL DETERMS THE CURRENT STATE
				StateMachine::SM_STATE current_state = state_machiene.determ_state(ps);
				StateMachine::SM_STATE previous_state = state_machiene.get_prev_state();
				//IF STATE SWITCHED
				if(current_state != previous_state)
				{
					//IN THIS SECIONT THE IMMEDIATES STATE WILL BE HANDLED
					//EG FROM NO_GAME_RUNNING TO GAME_RUNNING, THE SCREEN HAVE TO BE SWITCHED, ...
					
					
					//NOW THE NEW GAME HAS STARTED AND THE HARDWARE HAS TO INITILIZES TO THE GIVEN BOARD
					//AND MOVE THE CHESS FIGURES TO THE INIT BOARD GIVEN BY THE SERVER
					if(current_state == StateMachine::SM_STATE::SMS_GAME_RUNNING_WAITING_FOR_INITILIZEING) {
						//SHOW THE PROCESSING SCREEN
						gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::PROCESSING_SCREEN);
						io.setTurnStateLight(IOController::TURN_STATE_LIGHT::TSL_PRECCESSING);
						//NOW INIT THE GAME BOARD
						if(!ps.game_state.current_board_fen.empty()) {
							//LOAD THE FEN TO THE TARGET BOARD
							//IF RETURNS FALSE => THE FEN IS INVALID
							if(!board.boardFromFen(ps.game_state.current_board_fen, ChessBoard::BOARD_TPYE::TARGET_BOARD))
							{
								gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_B_OK, "INVALID BOARD FEN - CANCEL GAME", 4000);
								LOG_F(ERROR, ps.game_state.current_board_fen.c_str());
								gamebackend.set_player_state(BackendConnector::PLAYER_STATE::PS_IDLE);
								gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::MAIN_MENU_SCREEN);	
							}
								
							//NOW SYNC THE TWO BOARDS => THE TARGET CHESS POSITIONS WITH THE REAL WORLD MECHANICAL POSTIONS
							if(board.syncRealWithTargetBoard())
							{
								//BOARD SYNCED => UPDATE STATE ON SERVER THAT BOARD IS SYNCED
								gamebackend.set_player_setup_confirmation(BackendConnector::PLAYER_SETUP_STATE::PSP_READY);
								gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::GAME_SCREEN);	
							}else
							{
								//BOARD INIT FAILED => SHOW OPTION TO CONTIOUE
								if(gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_A_OK_CANCEL, "BOARD SETUP FAILED? CONTINUE", 4000) == guicommunicator::GUI_MESSAGE_BOX_RESULT::MSGBOX_RES_OK) {
									//SEND BOARD INIT COMPLETE CALL
									gamebackend.set_player_setup_confirmation(BackendConnector::PLAYER_SETUP_STATE::PSP_READY);
									gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::GAME_SCREEN);	
								}else
								{
									//RESET PLAYER STATE => ABORTS GAME
									gamebackend.set_player_state(BackendConnector::PLAYER_STATE::PS_IDLE);
									//GO BACK TO MAIN_MENU SCREEN
									gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::MAIN_MENU_SCREEN);	
								}
							}
						}
						
						
						//INIT FINÍSHED AND GAME RUNNING => SWITCH TO GAME SCREEN
					}else if(current_state == StateMachine::SM_STATE::SMS_GAME_RUNNING_INITILIZED) {
						gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::GAME_SCREEN);	
					
					}else if(current_state == StateMachine::SM_STATE::SMS_GAME_RUNNING_WAITING_FOR_OTHER_TURN) {
						gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::GAME_SCREEN);	
						if (!ps.game_state.current_board_fen.empty() && board.boardFromFen(ps.game_state.current_board_fen, ChessBoard::BOARD_TPYE::TARGET_BOARD) && board.syncRealWithTargetBoard())
						{
								
						}
						//UPDATE UI
						//UPDATE BOARD
					}else if(current_state == StateMachine::SM_STATE::SMS_GANE_RUNNIGN_WAITING_FOR_OWN_TURN) {
						gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::GAME_SCREEN);	
						//UPDATE BOARD TO CURRENT FEN
						if(!ps.game_state.current_board_fen.empty() && board.boardFromFen(ps.game_state.current_board_fen, ChessBoard::BOARD_TPYE::TARGET_BOARD) && board.syncRealWithTargetBoard()){
								
						}
							
						
						
						
						
						
						
						//TODO LIST MOVE ON UI
						//TODO MAKE MOVE ON BOARD
						//TODO CREATE QT MADE MOVE UI WITH ONE BUTTON
					
						//INIT FINÍSHED AND GAME RUNNING => SWITCH TO GAME SCREEN
						//UPDATE BOARD
					}else if(current_state == StateMachine::SM_STATE::SMS_GAME_ABORTED || current_state == StateMachine::SM_STATE::SMS_UNKNOWN)
					{
						gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::MAIN_MENU_SCREEN);	
					}
			
					
				
					
					
					
					//FINALLY SWITCH TO THE NEW STATE
					state_machiene.switch_to_next_state(ps);
					
					
					
					
					//THE PREVIOUS IF ARE ONLY FOR STATE TRANSITION LIKE FROM GAME_INIT TO GAME RUNNING
					//THE ELSE IS USED TO UPDATE THE GAME SCREEN WITH CHANGES (like the turn of the other player)
				}else
				{
					//TODO REFRESH THE GAME SCREEN 
				}
				
			}
		}
		
		
		//HANDLE UI EVENTS UI LOOP
		guicommunicator::GUI_EVENT ev = gui.get_gui_update_event();
		if (!ev.is_event_valid){continue;}
				
		gui.debug_event(ev, true);	

		
		//--------------------------------------------------------
		//----------------LOGIN BUTTON ---------------------------
		//--------------------------------------------------------
		if(ev.event == guicommunicator::GUI_ELEMENT::BEGIN_BTN && ev.type == guicommunicator::GUI_VALUE_TYPE::CLICKED) {
			//PERFORM A LOGIN AS HUMAN
			if(gamebackend.login(BackendConnector::PLAYER_TYPE::PT_HUMAN) && !gamebackend.get_session_id().empty())
			{
				//START HEARTBEAT THREAD
				if(gamebackend.start_heartbeat_thread()) {
					//SWITCH TO MAIN MENU
						gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::MAIN_MENU_SCREEN);
					//PLACE THE GOT SESSION ID ON THE INFO SCREEN
					gui.createEvent(guicommunicator::GUI_ELEMENT::INFOSCREEN_SESSIONID_LABEL, guicommunicator::GUI_VALUE_TYPE::USER_INPUT_STRING, gamebackend.get_session_id());
					//SHOW PLAYERNAME ON INFO SCREEN
					gui.createEvent(guicommunicator::GUI_ELEMENT::INFOSCREEN_VERSION, guicommunicator::GUI_VALUE_TYPE::USER_INPUT_STRING, "Playername: " + gamebackend.getPlayerProfile().friendly_name + " | " + gamebackend.getPlayerProfile().elo_rank_readable);
				}else {
					gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_B_OK, "LOGIN_FAILED_HEARTBEAT", 4000);
					LOG_F(ERROR, "GOT LOGIN_FAILED_HEARTBEAT START THREAD");
					gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::LOGIN_SCREEN);
				}
			
			}else {
				gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_B_OK, "LOGIN_FAILED", 4000);
				LOG_F(ERROR, "GOT LOGIN FAILED");
				gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::LOGIN_SCREEN);
			}
		}
		
		
		
		
		
		
		
		//--------------------------------------------------------
		//----------------BOARD INIT BUTTON-----------------------
		//--------------------------------------------------------
		if(ev.event == guicommunicator::GUI_ELEMENT::INIT_BTN && ev.type == guicommunicator::GUI_VALUE_TYPE::CLICKED) {
			gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::PROCESSING_SCREEN);
			if (board.initBoard(board_scan) != ChessBoard::BOARD_ERROR::INIT_COMPLETE)
			{
				gui.show_error_message_on_gui("board.initBoard() FAILED");
			}
			gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::SETTINGS_SCREEN);
		}
				
		
		
		//--------------------------------------------------------
		//----------------SCAN BOARD BTN--------------------------
		//--------------------------------------------------------
		if((ev.event == guicommunicator::GUI_ELEMENT::SCAN_BOARD_BTN || ev.event == guicommunicator::GUI_ELEMENT::DEBUG_FUNCTION_A) && ev.type == guicommunicator::GUI_VALUE_TYPE::CLICKED) {
			gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::PROCESSING_SCREEN);
			if (board.calibrate_home_pos() == ChessBoard::BOARD_ERROR::NO_ERROR)
			{
				gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_B_OK, "TABLE REACHED HOME POSITION", 10000);
			}
			else
			{
				gui.show_error_message_on_gui("board.initBoard() FAILED");
			}
			gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::SETTINGS_SCREEN);
		}
					
		
		
		//--------------------------------------------------------
		//----------------DEBUG - LOAD CONFIG BUTTON--------------
		//--------------------------------------------------------
		if(ev.event == guicommunicator::GUI_ELEMENT::DEBUG_FUNCTION_B && ev.type == guicommunicator::GUI_VALUE_TYPE::CLICKED) {
			gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::PROCESSING_SCREEN);
			ConfigParser::getInstance()->createConfigFile(CONFIG_FILE_PATH, true);
			gui.show_message_box(guicommunicator::GUI_MESSAGE_BOX_TYPE::MSGBOX_B_OK, "LOADED DEFAULT CONFIG", 10000);
			gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::SETTINGS_SCREEN);
		}
			
		
		//--------------------------------------------------------
		//----------------LOGOUT BUTTON --------------------------
		//--------------------------------------------------------	
		if(ev.event == guicommunicator::GUI_ELEMENT::LOGOUT_BTN && ev.type == guicommunicator::GUI_VALUE_TYPE::CLICKED) {
			gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::PROCESSING_SCREEN);
			//LOGOUT AND GOTO LOGIN SCREEN
			if(gamebackend.stop_heartbeat_thread() && gamebackend.logout()) {
				gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::LOGIN_SCREEN);
			}
		}
		
		
		//--------------------------------------------------------
		//----------------ENABLE MATCHMAKING BUTTON --------------
		//--------------------------------------------------------
		if(ev.event == guicommunicator::GUI_ELEMENT::MAINMENU_START_AI_MATCH_BTN && ev.type == guicommunicator::GUI_VALUE_TYPE::ENABLED) {
			//SET PLAYERSTATE TO OPEN FO A MATCH
			if(!gamebackend.set_player_state(BackendConnector::PLAYER_STATE::PS_SEARCHING)) {
				gui.show_error_message_on_gui("ENABLE MATCHMAKING FAILED");
			}
		}
		//--------------------------------------------------------
		//----------------DISBALE MATCHMAKING BUTTON --------------
		//--------------------------------------------------------
		if(ev.event == guicommunicator::GUI_ELEMENT::MAINMENU_START_AI_MATCH_BTN && ev.type == guicommunicator::GUI_VALUE_TYPE::DISBALED) {
			//SET PLAYERSTATE TO OPEN FO A MATCH
			if(!gamebackend.set_player_state(BackendConnector::PLAYER_STATE::PS_IDLE)) {
				gui.show_error_message_on_gui("DISBALE MATCHMAKING FAILED");
			}
		}	
			
		//--------------------------------------------------------
		//----------------DISBALE MATCHMAKING BUTTON --------------
		//--------------------------------------------------------
		if(ev.event == guicommunicator::GUI_ELEMENT::GAMESCREEN_ABORT_GAME && ev.type == guicommunicator::GUI_VALUE_TYPE::CLICKED) {
			//SET PLAYERSTATE TO OPEN FO A MATCH
			if(gamebackend.set_player_state(BackendConnector::PLAYER_STATE::PS_IDLE)) {
				gui.show_error_message_on_gui("GAME STOPPED");
				gui.createEvent(guicommunicator::GUI_ELEMENT::SWITCH_MENU, guicommunicator::GUI_VALUE_TYPE::MAIN_MENU_SCREEN);
			}
		}	
		
		
		
	}
	*/
	return 0;
}




