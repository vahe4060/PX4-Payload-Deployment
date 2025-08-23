/****************************************************************************
 *
 *   Copyright (c) 2022 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include "payload_deployer.h"


PayloadDeployer::PayloadDeployer()
	: ModuleBase<PayloadDeployer>()
	, ModuleParams(nullptr)
	, ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::lp_default)
{
}

/* idk for now */
int PayloadDeployer::task_spawn(int argc, char *argv[]) {
	PayloadDeployer *instance = new PayloadDeployer();
	if (instance) {
		_object.store(instance);
		_task_id = task_id_is_work_queue;
		return PX4_OK;
	}
	else {
		PX4_ERR("Alloc failed");
	}
	// Cleanup instance in memory and mark this module as invalid to run
	delete instance;
	_object.store(nullptr);
	_task_id = -1;

	return PX4_ERROR;
}

/* add a new payload */
bool PayloadDeployer::add(int argc, char *argv[]) {

	return false;
}

/* edit an existing payload by its index */
bool PayloadDeployer::edit(int argc, char *argv[]) {

	return false;
}

/* remove an existing payload by its index */
bool PayloadDeployer::remove(int argc, char *argv[]) {

	return false;
}

/* test servo of payload by its index*/
bool PayloadDeployer::test_servo(int argc, char *argv[]) {

	return false;
}

/* list added payloads */
bool PayloadDeployer::list() {

	return false;
}

int PayloadDeployer::custom_command(int argc, char *argv[]) {
	if (argc == 0)
		return print_usage();
	else if (strcmp(argv[0], "add") == 0)
		return _object.load()->add(argc + 1, argv + 1);
	else if (strcmp(argv[0], "edit") == 0)
		return _object.load()->edit(argc + 1, argv + 1);
	else if (strcmp(argv[0], "remove") == 0)
		return _object.load()->remove(argc + 1, argv + 1);
	else if (strcmp(argv[0], "test_servo") == 0)
		return _object.load()->test_servo(argc + 1, argv + 1);
	else if (strcmp(argv[0], "list") == 0)
		return _object.load()->list();
	return print_usage("Unrecognized command");
}

int PayloadDeployer::print_usage(const char *reason) {
	if (reason) {
		PX4_WARN("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
Handles payload deployment for each item based on its aerodynamic properties so that it reaches to
its destination (lat, lon) from specified drop altitude with smallest margin error.
Number of payloads is not limited, however each payload
must have its unique drop priority index (the smaller the sooner).

	add [index] [weight(kg)] [area_x(sq.m)] [area_y(sq.m)] [drag_coef] [alt(m)]
	    [lat(WGS84)] [lon(WGS84)] [pwm_id] [pwm_open_freq] [pwm_close_freq]
		example:
			add  1050  1.2  0.01  0.042  0.1  150  40.175885  44.612789  12  950  1450
	edit [index] [FIELD_NAME] [NEW_VALUE]
		example:
			edit  1050  weight  1.35
	test_servo [index]
		example:
			test_servo  1050
	remove [index]
		example:
			remove  1050
)DESCR_STR");
	PRINT_MODULE_USAGE_NAME("payload_deployer", "command");
	PRINT_MODULE_USAGE_COMMAND_DESCR("add [index ...]", "Add a new payload.");
	PRINT_MODULE_USAGE_COMMAND_DESCR("edit [index ...]", "Edit payload properties.");
	PRINT_MODULE_USAGE_COMMAND_DESCR("test_servo [index]", "Test servo open/close functionality.");
	PRINT_MODULE_USAGE_COMMAND_DESCR("remove [index]", "Remove the payload.");
	PRINT_MODULE_USAGE_COMMAND_DESCR("list", "List the payloads.");
	PRINT_MODULE_USAGE_DEFAULT_COMMANDS();
	return PX4_OK;
}

void PayloadDeployer::Run() {

}

/** @see ModuleBase::print_status() */
int PayloadDeployer::print_status() {
	return 0;
}

int payload_deployer_main(int argc, char *argv[]) {
	return PayloadDeployer::main(argc, argv);
}
