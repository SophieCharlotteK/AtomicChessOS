#include "StateMachine.h"


StateMachine::StateMachine()
{
	
}


StateMachine::SM_STATE StateMachine::determ_next_state(BackendConnector::PLAYER_STATUS _ps)
{
	return StateMachine::SM_STATE::UNKNOWN;
}

StateMachine::SM_STATE StateMachine::current_next_state(BackendConnector::PLAYER_STATUS _ps)
{
	return StateMachine::SM_STATE::UNKNOWN;
}

StateMachine::SM_STATE StateMachine::switch_to_next_state(BackendConnector::PLAYER_STATUS _ps)
{
	StateMachine::SM_STATE tmp = determ_next_state(_ps);
	current_state = tmp;
	return tmp;
}


