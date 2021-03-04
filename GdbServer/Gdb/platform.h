#pragma once

#ifndef PC_HOSTED
#define PC_HOSTED 1
#endif

extern int running_status;

#define SET_RUN_STATE(state)	{running_status = (state);}
#define SET_IDLE_STATE(state)	{}
#define SET_ERROR_STATE(state)	{}